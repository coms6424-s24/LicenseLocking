#include <algorithm>
#include <cassert>
#include <immintrin.h>
#include <map>
#include <time.h>

#include <cassert>

#include "fingerprint.h"
#include "prng.h"
#include "utils.h"

/*
 * The 'clock_gettime' function seems to most closely
 * resemble the Windows datetimeapi.h 'GetDateFormat'
 * (presumably used in the paper, page 2, section 1);
 * this reads the WALL CLOCK, not the CPU clock
 */
int _clock_gettime(clockid_t clk_id, struct timespec *res) {
  _mm_mfence();
  int retval = clock_gettime(clk_id, res);
  _mm_mfence();
  return retval;
}

/*
 * Reads the Timestamp Counter (TSC); this should be
 * the "CPU clock" mentioned in the paper
 */
inline uint64_t rdtsc() {
  unsigned long a, d;
  asm volatile("mfence");
  asm volatile("rdtsc" : "=a"(a), "=d"(d));
  asm volatile("mfence");
  return a | ((uint64_t)d << 32);
}

/*
 * This type represents the raw fingerprint type defined in the
 * paper (i.e. 2-dimensiona array of clock-related measurements)
 */
typedef std::array<std::array<long long, m>, n> fingerprint;

/*
 * This type stores every row of the original (raw) fingerprint
 * as an std::map, for easy identification of the mode (and, if
 * needed, for easy set-membership checks; however, I might
 * deprecate this soon).
 */
typedef std::vector<std::map<long long, int>> fingerprint_map;

/*
 * to_map : fingerprint -> fingerprint_map
 */
fingerprint_map to_map(const fingerprint& F) {
  fingerprint_map M (n);
  for (int i = 0; i < n; i++)
    for (const long long& measurement : F[i])
      M[i][measurement]++;
  return M;
}

long long mode(const std::map<long long, int>& m) {
  auto x = std::max_element(m.begin(), m.end(),
                            [](const std::pair<long long, int> &p1,
                               const std::pair<long long, int> &p2) {
                              return p1.second < p2.second;
                            });
  return x->first;
}

/*
 * to_hash : fingerprint -> fingerprint_hash
 */
fingerprint_hash to_hash(const fingerprint& F) {
  /* 
   * Fingerprint matching, in the original paper, is posed
   * in terms of set membership (i.e. mode(fp1[i]) in fp2[i]
   * and mode(fp2[i]) in fp1[i]). In this implementation, we 
   * express set membership as a dot product:
   *     1) For all i = 1...n, hash the measurements fp[i]
   *        to some range [0, FINGERPRINT_HASH_LINE]. The
   *        hash is selected randomly from a m-independent hash
   *        family and need not be cryptographically safe.
   *     2) All cells containing a measurement contain value
   *        1, and the cell containing the mode is set to
   *        MODE_WEIGHT.
   *     3) If we dot product 2 hash tables, we have 3 types
   *        of non-zero contributions:
   *            a) 1 * 1 -> false collision, at most m per
   *               hash line, n * m overall
   *            b) 1 * MODE_WEIGHT -> true collision, 1 mode
   *               is in the opposite hash table (assuming
   *               perfect hashing; assumption discussed later)
   *            c) MODE_WEIGHT * MODE_WEIGHT -> two true 
   *               collisions, both modes in the opposite hashes
   * MODE_WEIGHT is set so that the three types of collisions
   * have different orders of magnitude and can be distinguished
   * from each other.
   */
  assert(MODE_WEIGHT > m * n);
  /* 
   * Seed PRNG with deterministic seed (for reproducible
   * hash function instantiations across multiple fingerprints.
   * This could be randomized and stored in a file in the
   * future if desired.
   */
  long long seed = 8009488488;
  prng_seed_bytes(&seed, sizeof(seed));
  /* 
   * [Carter, Wegman] set p to be a prime greater than any
   * measurement. Assuming all measurements are an order of
   * magnitude shorter than the  within the window of one 
   * context switch (0.1 ms = 1e8 ns), this p should be fine.
   */
  long long p = 100000007;

  fingerprint_hash H (n * FINGERPRINT_HASH_LINE);
  fingerprint_map M = to_map(F);
  for (int i = 0; i < n; i++) {
	// [Carter, Wegman], m-wise independent hash function:
	// Evaluate a polynomial with random coefficients in
	// Zp (given the large p chosen above), then map that
	// to the desired range
	long long c[m];
	for(int i = 0; i < m; i++) {
		prng_get_bytes(c + i, sizeof(long long));
		c[i] %= p;
	}
	auto hash_func = [&c, &p](long long x) {
		long long acc = 0;
		x %= p;
		// Horner's Method of evaluating polynomials
		for(int i = 0; i < m; i++)
			acc = (acc * x + c[i]) % p;
		return acc >= 0 ? acc : acc + p;
	};
	
	// add regular elements with weight 1 and modes with a
	// predefined, large weight (discussed above)
	for(int j = 0; j < m; j++) {
		long long x = F[i][j];
		long long hx = hash_func(x) % FINGERPRINT_HASH_LINE;
		H[i * FINGERPRINT_HASH_LINE + hx] = 1;
	}
    long long md = mode(M[i]);
	long long hm = hash_func(md) % FINGERPRINT_HASH_LINE;
	H[i * FINGERPRINT_HASH_LINE + hm] = MODE_WEIGHT;
  }

  return H;
}

/*
 * Generates the fingerprint (by timing fp_func repeatedly) and returns
 * it as a fingerprint_hash; if non-empty, out is the filename where the
 * fingerprint is dumped.
 */
fingerprint_hash make_hash(const std::function<size_t(size_t)>& fp_func,
                   		   const std::string &out) {
  fingerprint F;
  for (int i = 1; i <= m; i++) {
    for (int j = 1; j <= n; j++) {
	  struct timespec startTime, endTime;
      uint64_t startTSC, endTSC;
      _clock_gettime(CLOCK_REALTIME, &startTime);
      fp_func(j);
      _clock_gettime(CLOCK_REALTIME, &endTime);

      startTSC = rdtsc();
      fp_func(j);
      endTSC = rdtsc();

	  /* Use quotient version when set tsc as clock source,
	   * Use non-quotient version when set HPET as clock source.
	   */
	  long long logTime;
	  if (CLOCKSOURCE == TSC)
        logTime = sensitivity * (endTime.tv_nsec - startTime.tv_nsec) /
                      (endTSC - startTSC);
	  else if (CLOCKSOURCE == HPET)
	    logTime = endTime.tv_nsec - startTime.tv_nsec;
      F[j - 1][i - 1] = logTime;
    }
  }

  fingerprint_hash H = to_hash(F);
  if (out.empty())
    return H;

  FILE *fout = fopen(out.c_str(), "w");
  for(int i = 0; i < n * FINGERPRINT_HASH_LINE; i++)
	fprintf(fout, "%lld ", H[i]);
  fclose(fout);
  return H;
}

fingerprint_hash read_hash(const std::string &in) {
  fingerprint_hash H (n * FINGERPRINT_HASH_LINE);
  FILE *fin = fopen(in.c_str(), "r");
  for(int i = 0; i < n * FINGERPRINT_HASH_LINE; i++)
    fscanf(fin, "%lld", &H[i]);
  fclose(fin);
  return H;
}

bool match_hash(const fingerprint_hash& myH, const fingerprint_hash& baseH) {
	long long dp = 0LL;
	for(int i = 0; i < n * FINGERPRINT_HASH_LINE; i++)
		dp += myH[i] * baseH[i];
	// distinguish between the three types of collisions
	long long count = 2LL * (dp / (MODE_WEIGHT * MODE_WEIGHT)) + 
                      1LL * (dp % (MODE_WEIGHT * MODE_WEIGHT) / MODE_WEIGHT) + 
                      0LL * (dp % MODE_WEIGHT);
	printf("%lld/%lld\n", count, 2LL * n);
	return count >= threshold * 2LL * n;
}
