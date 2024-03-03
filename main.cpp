#include <functional>
#include <stdio.h>

#include "fingerprint.h"
#include "fp_funcs.h"
#include "utils.h"

// which function is used for fingerprinting
constexpr Method method = HASH;

int main() {
	std::function<size_t(size_t)> fp_func = make_fp_func(method);
	std::array<std::array<long long, m>, n> fp = fingerprint(fp_func, "fingerprint");
	return 0;
}
