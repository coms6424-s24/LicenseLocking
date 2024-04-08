#include <algorithm>
#include <immintrin.h>
#include <map>
#include <time.h>

#include <cassert>

#include "fingerprint.h"
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

typedef std::array<std::array<long long, m>, n> fingerprint;

typedef std::vector<std::map<long long, int>> fingerprint_map;

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

fingerprint_hash to_hash(const fingerprint& F, Target t) {
  fingerprint_hash H (2 * n * FINGERPRINT_HASH_LINE);
  fingerprint_map M = to_map(F);
  for (int i = 0; i < n; i++) {
    long long md = mode(M[i]);
	long long hm = (md % FINGERPRINT_HASH_LINE + FINGERPRINT_HASH_LINE) % FINGERPRINT_HASH_LINE;
	H[(i * 2 + t) * FINGERPRINT_HASH_LINE + hm] = 1;
	for(int j = 0; j < m; j++) {
		long long x = F[i][j];
		long long hx = (x % FINGERPRINT_HASH_LINE + FINGERPRINT_HASH_LINE) % FINGERPRINT_HASH_LINE;
		H[(i * 2 + 1 - t) * FINGERPRINT_HASH_LINE + hx] = 1;
	}
  }

  return H;
}


fingerprint_hash make_hash(const std::function<size_t(size_t)>& fp_func,
                   		   const Target& t, const std::string &out) {
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
      /*
       * CryptoFP is based on the "identification of readily
       * available functions that, when repeated a sufficient
       * number of times, can be used to amplify the small
       * differences between different clocks" (page 2, section 1).
       *
       * Two clocks available from user space are the TSC
       * (Timestamp Counter), which should be the clock "used
       * by the CPU to execute instructions", and the wall clock
       * which is different, has nanosecond resolution, and is
       * colocated with the CPU clock. Both of those clocks should
	   * run at a fixed frequency, independent of the CPU freq.
       *
       * However, no reliable nanosecond-resolution clock is
       * made available from the TSC (which is measured in ticks)
       * and the wall clock is only shown in nanoseconds (not
       * in ticks). So, the TSC and wall clock are measured in
       * different units, and conversion between them may be
       * unreliable.
	   * IDEA: use TICKS_PER_SEC (?)
       *
       * My idea is, instead of trying to get the "difference"
       * between the clocks, like mentioned in the paper, to get
       * the quotient of those two measurements. This should
       * cancel out any dynamic frequency factor or influence of
       * temperature (section 3.3.4), which affects both of them
       * equally. 
	   * NOTE: for some reason, the order in which the measurements
       * are made matters. By this, I mean that code like 

      	_clock_gettime(CLOCK_REALTIME, &startTime);
      	startTSC = rdtsc();
	    fp_func(j);
      	endTSC = rdtsc();
      	_clock_gettime(CLOCK_REALTIME, &endTime);

       * will produce fingerprints dependent on the system load.
	   * This is unexpected, since this order should only contribute
	   * with a multiplicative factor to the quitoent. For this
	   * reason, we currently take two measurements and time them
	   * independently.
       *
       * Indeed, this leads to reliable fingerprinting on a
       * single machine, even with different power settings,
       * which was NOT the case when using only one of the two
       * measurements (in that case, a lower frequency leads
       * to a proportional change in the fingerprint, which
       * compromises any similarity).
       */

	  /* Use quotient version when set tsc as clock source,
	   * Use non-quotient version when set HPET as clock source.
	   */
      long long logTime = sensitivity * (endTime.tv_nsec - startTime.tv_nsec) /
                      (endTSC - startTSC);
      F[j - 1][i - 1] = logTime;
    }
  }

  fingerprint_hash H = to_hash(F, t);
  if (out.empty())
    return H;

  FILE *fout = fopen(out.c_str(), "w");
  for(int i = 0; i < 2 * n * FINGERPRINT_HASH_LINE; i++)
	fprintf(fout, "%lld ", H[i]);
  fclose(fout);
  return H;
}

fingerprint_hash read_hash(const std::string &in) {
  fingerprint_hash H (2 * n * FINGERPRINT_HASH_LINE);
  FILE *fin = fopen(in.c_str(), "r");
  for(int i = 0; i < 2 * n * FINGERPRINT_HASH_LINE; i++)
    fscanf(fin, "%lld", &H[i]);
  fclose(fin);
  return H;
}

bool match_hash(const fingerprint_hash& myH, const fingerprint_hash& baseH) {
	int count = 0;
	for(int i = 0; i < 2 * n * FINGERPRINT_HASH_LINE; i++)
		count += myH[i] * baseH[i];
	printf("%d/%d\n", count, 2 * n);
	return count >= threshold * 2 * n;
}
