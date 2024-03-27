#include <algorithm>
#include <immintrin.h>
#include <map>
#include <time.h>
#include <numeric>

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
  int retval = clock_gettime(CLOCK_REALTIME, res);
  _mm_mfence();
  return retval;
}
uint64_t clock_nsec() {
	struct timespec T;
	_clock_gettime(CLOCK_REALTIME, &T);
	return T.tv_sec * 1000000000 + T.tv_nsec;
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

// https://github.com/travisdowns/avx-turbo/blob/master/tsc-support.cpp
constexpr uint64_t DELTA = 1000000;
double sample() {
	uint64_t startCLK, endCLK;
    uint64_t startTSC, endTSC;
    startCLK = clock_nsec();
    startTSC = rdtsc();
    while (startCLK + DELTA > (endCLK = clock_nsec()))
		;
    endTSC = rdtsc();
    return 1.0 * (endCLK - startCLK) / (endTSC - startTSC);
}

constexpr size_t SAMPLES = 101;
double ns_per_tick() {
    std::array<double, SAMPLES * 2> samples;
    for (size_t s = 0; s < SAMPLES * 2; s++)
        samples[s] = sample();

    // throw out the first half of samples as a warmup
	std::sort(samples.begin() + SAMPLES, samples.end());
    // average the middle quintile
    double sum = std::accumulate(samples.begin() + SAMPLES + 2 * SAMPLES / 5, 
								 samples.begin() + SAMPLES + 3 * SAMPLES / 5, 
								 0.0);
    return sum / (SAMPLES / 5);
}

fingerprint make_fingerprint(const std::function<size_t(size_t)> &fp_func,
                             const std::string &out) {
  fingerprint fp;
  double mult = ns_per_tick();
  for (int i = 1; i <= m; i++) {
    for (int j = 1; j <= n; j++) {
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

      /*struct timespec startTime, endTime;
      uint64_t startTSC, endTSC;
      _clock_gettime(CLOCK_REALTIME, &startTime);
      fp_func(j);
      _clock_gettime(CLOCK_REALTIME, &endTime);

      startTSC = rdtsc();
      fp_func(j);
      endTSC = rdtsc();*/
	  uint64_t startCLK, endCLK;
	  uint64_t startTSC, endTSC;

	  startCLK = clock_nsec();
	  startTSC = rdtsc();
      fp_func(j);
	  endCLK = clock_nsec();
	  endTSC = rdtsc();

      long long logTime = sensitivity * 
						  ((endCLK - startCLK) - mult * (endTSC - startTSC));
      fp[j - 1][i - 1] = logTime;
    }
  }

  if (out.empty())
    return fp;

  FILE *fout = fopen(out.c_str(), "w");
  for (int j = 0; j < n; j++) {
    for (int i = 0; i < m; i++)
      fprintf(fout, "%lld ", fp[j][i]);
    fprintf(fout, "\n");
  }
  fclose(fout);
  return fp;
}

fingerprint read_fingerprint(const std::string &in) {
  fingerprint fp;
  FILE *fin = fopen(in.c_str(), "r");
  for (int i = 0; i < n; i++)
    for (int j = 0; j < m; j++)
      fscanf(fin, "%lld", &fp[i][j]);
  fclose(fin);
  return fp;
}

typedef std::array<std::map<long long, int>, n> m_fingerprint;
m_fingerprint to_map(fingerprint fp) {
  m_fingerprint m_fp;
  for (int i = 0; i < n; i++)
    for (const long long &measurement : fp[i])
      m_fp[i][measurement]++;
  return m_fp;
}

long long mode(std::map<long long, int> m) {
  auto x = std::max_element(m.begin(), m.end(),
                            [](const std::pair<long long, int> &p1,
                               const std::pair<long long, int> &p2) {
                              return p1.second < p2.second;
                            });
  return x->first;
}

bool match(fingerprint myfp, fingerprint base) {
  m_fingerprint m_myfp = to_map(myfp);
  m_fingerprint m_base = to_map(base);

  int count = 0;
  for (int i = 0; i < n; i++) {
    long long x = mode(m_myfp[i]);
    count += m_base[i].find(x) != m_base[i].end();
    long long y = mode(m_base[i]);
    count += m_myfp[i].find(y) != m_myfp[i].end();
  }
  printf("%d/%d\n", count, 2 * n);
  return count >= threshold * 2 * n;
}
