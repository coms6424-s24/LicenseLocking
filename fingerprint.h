#ifndef __FINGERPRINT_H__
#define __FINGERPRINT_H__

#include <functional>
#include <stddef.h>
#include <string>

#include "utils.h"

typedef std::array<std::array<long long, m>, n> fingerprint;
constexpr double threshold = 0.5;

/*
 * fingerprints 'fp_func' according to the m, n
 * parameters set in 'utils.h'. Returns the fp
 * array and, if the out string is not empty, 
 * prints the fingerprint in the file 'out'.
 */
fingerprint make_fingerprint(const std::function<size_t(size_t)>& fp_func, const std::string& out = "");

/*
 * reads and returns a fingerprint from 'in'.
 */
fingerprint read_fingerprint(const std::string& in);

/*
 * checks if fingerprints A and B match. This
 * is implemented according to section 3.3.2
 * and Figure 2 of the original paper.
 */
bool match(fingerprint A, fingerprint B);

#endif
