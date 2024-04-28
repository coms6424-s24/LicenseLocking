#ifndef __FINGERPRINT_H__
#define __FINGERPRINT_H__

#include <functional>
#include <stddef.h>
#include <string>

#include "utils.h"

enum Clocksource { TSC,
    HPET };
constexpr Clocksource CLOCKSOURCE = HPET;

constexpr long long MODE_WEIGHT = (1 << 11); // must be larger than 2 * n
constexpr int FINGERPRINT_HASH_LINE = 10 * m;
typedef std::vector<long long> fingerprint_hash;

/*
 * similarity threshold for fingerprint match or comparison
 */
constexpr double threshold = 0.5;
/*
 * additional fingerprint entropy can be introduced by increasing
 * the 'sensitivity' parameter. As a consequence, single machine
 * measurements also become more unreliable.
 */
constexpr long long sensitivity = 1000;

/*
 * fingerprints 'fp_func' according to the m, n parameters set in
 * 'utils.h'. Returns the hashed fingerprint array (see fingerprint.cpp
 * for why hashing is used).
 */
fingerprint_hash make_hash(const std::function<size_t(size_t)>& fp_func);

/*
 * reads and returns a fingerprint_hash from 'in'.
 */
fingerprint_hash read_hash(const std::string& in);
/*
 * writes a fingerprint_hash to 'out'.
 */
void write_hash(const fingerprint_hash& H, const std::string& out);
void write_encrypted_hash(const fingerprint_hash& H, const std::string& out);

/*
 * Computes the dot product of two hashed fingerprints. This relates to
 * section 3.3.2 and Figure 2 of the original paper. However, notably,
 * the similarity of two hashed fingerprints is expressed in terms of
 * the dot product of their hashes.
 */
long long dot_hash(const fingerprint_hash& myH, const fingerprint_hash& baseH);
long long dot_hash_encrypted(const fingerprint_hash& myH, const std::string& baseHpath);

/*
 * Checks if a dot product corresponds to a match.
 */
bool match(long long dp);

#endif
