#ifndef __FINGERPRINT_H__
#define __FINGERPRINT_H__

#include <functional>
#include <stddef.h>
#include <string>

#include "utils.h"

/*
 * fingerprints 'fp_func' according to the m, n
 * parameters set in 'utils.h'. Returns the fp
 * array and, if the out string is not empty, 
 * prints the fingerprint in the file 'out'.
 */
std::array<std::array<long long, m>, n> fingerprint(const std::function<size_t(size_t)>& fp_func, const std::string& out);

#endif
