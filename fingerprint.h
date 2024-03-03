#ifndef __FINGERPRINT_H__
#define __FINGERPRINT_H__

#include <functional>
#include <stddef.h>
#include <string>

#include "utils.h"

typedef std::array<std::array<long long, m>, n> fingerprint;

/*
 * fingerprints 'fp_func' according to the m, n
 * parameters set in 'utils.h'. Returns the fp
 * array and, if the out string is not empty, 
 * prints the fingerprint in the file 'out'.
 */
fingerprint make_fingerprint(const std::function<size_t(size_t)>& fp_func, const std::string& out = "");

fingerprint read_fingerprint(const std::string& in);

#endif
