#include <immintrin.h>
#include <map>
#include <time.h>

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

fingerprint make_fingerprint(const std::function<size_t(size_t)>& fp_func, const std::string& out) {
	fingerprint fp;
	for(int i = 1; i <= m; i++) {
		for(int j = 1; j <= n; j++) {	
			struct timespec startTime, endTime;
			_clock_gettime(CLOCK_REALTIME, &startTime);
			fp_func(j);
			_clock_gettime(CLOCK_REALTIME, &endTime);
			long long logTime = endTime.tv_nsec - startTime.tv_nsec;
			fp[j - 1][i - 1] = logTime;
		}
	}

	if(out.empty())
		return fp;

	FILE *fout = fopen(out.c_str(), "w");
	for(int j = 0; j < n; j++) {
		for(int i = 0; i < m; i++)
			fprintf(fout, "%lld ", fp[j][i]);
		fprintf(fout, "\n");
	}
	fclose(fout);
	return fp;
}

fingerprint read_fingerprint(const std::string& in) {
	fingerprint fp;
	FILE *fin = fopen(in.c_str(), "r");
	for(int i = 0; i < n; i++)
		for(int j = 0; j < m; j++)
			fscanf(fin, "%lld", &fp[i][j]);
	fclose(fin);
	return fp;
}


typedef std::array<std::map<long long, int>, n> m_fingerprint;
m_fingerprint to_map(fingerprint fp) {
	m_fingerprint m_fp;
	for(int i = 0; i < n; i++)
		for(const long long& measurement: fp[i])
			m_fp[i][measurement]++;
	return m_fp;
}

long long mode(std::map<long long, int> m) {
	auto x = std::max_element(
					m.begin(), m.end(),
    				[](const std::pair<long long, int>& p1, const std::pair<long long, int>& p2) {
        				return p1.second < p2.second; 
					});
	return x->first;
}

bool match(fingerprint myfp, fingerprint base) {
	m_fingerprint m_myfp = to_map(myfp);
	m_fingerprint m_base = to_map(base);
	
	int count = 0;
	for(int i = 0; i < n; i++) {
		long long x = mode(m_myfp[i]);
		count += m_base[i].find(x) != m_base[i].end();
		long long y = mode(m_base[i]);
		count += m_myfp[i].find(y) != m_base[i].end();
	}
	printf("%d/%d\n", count, 2 * n);
	return count >= threshold * 2 * n;
}

