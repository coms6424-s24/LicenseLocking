#ifndef __FINGERPRINT_H__
#define __FINGERPRINT_H__

#include <functional>
#include <stddef.h>
#include <string>

#include "utils.h"

enum Clocksource { TSC, HPET };
constexpr Clocksource CLOCKSOURCE = TSC;

constexpr long long MODE_WEIGHT = (1 << 16); // must be larger than m * n
constexpr int FINGERPRINT_HASH_LINE = 10 * m;
typedef std::vector<long long> fingerprint_hash;

/*
 * similarity threshold for fingerprint match
 * or comparison
 */
constexpr double threshold = 0.5;
/*
 * additional fingerprint entropy can be
 * introduced by increasing the 'sensitivity'
 * parameter. As a consequence, single machine
 * measurements also become more unreliable.
 */
constexpr long long sensitivity = 1000;

/*
 * fingerprints 'fp_func' according to the m, n
 * parameters set in 'utils.h'. Returns the fp
 * array and, if the out string is not empty,
 * prints the fingerprint in the file 'out'.
 */
fingerprint_hash make_hash(const std::function<size_t(size_t)> &fp_func,
                           const std::string &out = "");

/*
 * reads and returns a fingerprint from 'in'.
 */
fingerprint_hash read_hash(const std::string &in);

/*
 * checks if fingerprints A and B match. This
 * is implemented according to section 3.3.2
 * and Figure 2 of the original paper.
 */
bool match_hash(const fingerprint_hash& A, const fingerprint_hash& B);

#endif
