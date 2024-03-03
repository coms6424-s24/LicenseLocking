#include <functional>
#include <cstdio>
#include <cstring>

#include "fingerprint.h"
#include "fp_funcs.h"
#include "utils.h"

#include <map>

// which function is used for fingerprinting
constexpr Method method = HASH;

typedef std::array<std::map<long long, int>, n> m_fingerprint;
constexpr double threshold = 0.5;

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
	printf("%d %d\n", count, 2 * n);
	return count >= threshold * 2 * n;
}

int main(int argc, char** argv) {
	if (argc == 2 && strcmp(argv[1], "-cmp")) {
		std::function<size_t(size_t)> fp_func = make_fp_func(method);
		make_fingerprint(fp_func, argv[1]);
	}
	else if (argc == 3 && strcmp(argv[1], "-cmp") && !strcmp(argv[2], "-cmp")){
		std::function<size_t(size_t)> fp_func = make_fp_func(method);
		fingerprint myfp = make_fingerprint(fp_func);
		fingerprint base = read_fingerprint(argv[1]);
		if(match(myfp, base))
			printf("fingerprint match\n");
		else
			printf("no match\n");
	}
	else {
		printf("usage: ./main <fingerprint_filename> [-cmp]\n");
		exit(1);
	}	
	return 0;
}
