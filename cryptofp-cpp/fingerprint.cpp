#include <algorithm>
#include <cassert>
#include <cmath>
#include <ctime>
#include <immintrin.h>
#include <map>

#include "PRNG/prng.h"
#include "crypto.h"
#include "fingerprint.h"
#include "utils.h"

/*
 * The 'clock_gettime' function seems to most closely
 * resemble the Windows datetimeapi.h 'GetDateFormat'
 * (presumably used in the paper, page 2, section 1);
 * this reads the WALL CLOCK, not the CPU clock
 */
int _clock_gettime(clockid_t clk_id, struct timespec* res)
{
    _mm_mfence();
    int retval = clock_gettime(clk_id, res);
    _mm_mfence();
    return retval;
}

/*
 * Reads the Timestamp Counter (TSC); this should be
 * the "CPU clock" mentioned in the paper
 */
inline uint64_t rdtsc()
{
    unsigned long a, d;
    asm volatile("mfence");
    asm volatile("rdtsc"
                 : "=a"(a), "=d"(d));
    asm volatile("mfence");
    return a | ((uint64_t)d << 32);
}

/*
 * This type represents the raw fingerprint type defined in the
 * paper (i.e. 2-dimensiona array of clock-related measurements)
 */
typedef std::array<std::array<long long, m>, n> fingerprint;

/*
 * This type stores every row of the original (raw) fingerprint
 * as an std::map, for easy identification of the mode (and, if
 * needed, for easy set-membership checks; however, I might
 * deprecate this soon).
 */
typedef std::vector<std::map<long long, int>> fingerprint_map;

/*
 * to_map : fingerprint -> fingerprint_map
 */
fingerprint_map to_map(const fingerprint& F)
{
    fingerprint_map M(n);
    for (int i = 0; i < n; i++)
        for (const long long& measurement : F[i])
            M[i][measurement]++;
    return M;
}

/*
 * Pads each row of a fingerprint with random values until each
 * row has exactly m distinct elements; helps with correcting
 * the collision bias.
 */
void pad(fingerprint_map& M)
{
    for (int i = 0; i < n; i++) {
        long long dummy = 0;
        while (M[i].size() < m) {
            if (M[i].find(dummy) == M[i].end())
                M[i][dummy] = 1;
            dummy++;
        }
    }
}

long long mode(const std::map<long long, int>& m)
{
    auto x = std::max_element(m.begin(), m.end(),
        [](const std::pair<long long, int>& p1,
            const std::pair<long long, int>& p2) {
            return p1.second < p2.second;
        });
    return x->first;
}

/*
 * to_hash : fingerprint -> fingerprint_hash
 */
fingerprint_hash to_hash(const fingerprint& F)
{
    /*
     * Fingerprint matching, in the original paper, is posed
     * in terms of set membership (i.e. mode(fp1[i]) in fp2[i]
     * and mode(fp2[i]) in fp1[i]). In this implementation, we
     * express set membership as a dot product:
     *     1) For all i = 1...n, hash the measurements fp[i]
     *        to some range [0, FINGERPRINT_HASH_LINE]. The
     *        hash is selected randomly from a m-independent hash
     *        family and need not be cryptographically safe.
     *     2) All cells containing a measurement contain value
     *        1, and the cell containing the mode is set to
     *        MODE_WEIGHT.
     *     3) If we dot product 2 hash tables, we have 3 types
     *        of non-zero contributions:
     *            a) 1 * 1 -> false collision, at most m per
     *               hash line, n * m overall
     *            b) 1 * MODE_WEIGHT -> true collision, 1 mode
     *               is in the opposite hash table (assuming
     *               perfect hashing; assumption discussed later)
     *            c) MODE_WEIGHT * MODE_WEIGHT -> two true
     *               collisions, both modes in the opposite hashes
     * MODE_WEIGHT is set so that the three types of collisions
     * have different orders of magnitude and can be distinguished
     * from each other.
     */
    assert(MODE_WEIGHT > 2 * n);
    /*
     * Seed PRNG with deterministic seed (for reproducible
     * hash function instantiations across multiple fingerprints.
     * This could be randomized and stored in a file in the
     * future if desired.
     */
    long long seed = 8009488488;
    prng_seed_bytes(&seed, sizeof(seed));
    /*
     * [Carter, Wegman] set p to be a prime greater than any
     * measurement. Assuming all measurements are an order of
     * magnitude shorter than the  within the window of one
     * context switch (0.1 ms = 1e8 ns), this p should be fine.
     */
    long long p = 100000007;

    fingerprint_hash H(n * FINGERPRINT_HASH_LINE);
    fingerprint_map M = to_map(F);
    pad(M);
    for (int i = 0; i < n; i++) {
        // [Carter, Wegman], m-wise independent hash function:
        // Evaluate a polynomial with random coefficients in
        // Zp (given the large p chosen above), then map that
        // to the desired range
        long long c[m];
        for (int i = 0; i < m; i++) {
            prng_get_bytes(c + i, sizeof(long long));
            c[i] %= p;
        }
        auto hash_func = [&c, &p](long long x) {
            long long acc = 0;
            x %= p;
            // Horner's Method of evaluating polynomials
            for (int i = 0; i < m; i++)
                acc = (acc * x + c[i]) % p;
            return acc >= 0 ? acc : acc + p;
        };

        // add regular elements with weight 1 and modes with a
        // predefined, large weight (discussed above)
        for (int j = 0; j < m; j++) {
            long long x = F[i][j];
            long long hx = hash_func(x) % FINGERPRINT_HASH_LINE;
            H[i * FINGERPRINT_HASH_LINE + hx] = 1;
        }
        long long md = mode(M[i]);
        long long hm = hash_func(md) % FINGERPRINT_HASH_LINE;
        H[i * FINGERPRINT_HASH_LINE + hm] = MODE_WEIGHT;
    }

    return H;
}

/*
 * Generates the fingerprint (by timing fp_func repeatedly) and returns
 * it as a fingerprint_hash.
 */
fingerprint_hash make_hash(const std::function<size_t(size_t)>& fp_func)
{
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

            /* Use quotient version when set tsc as clock source,
             * Use non-quotient version when set HPET as clock source.
             */
            long long logTime;
            if (CLOCKSOURCE == TSC)
                logTime = sensitivity * (endTime.tv_nsec - startTime.tv_nsec) / (endTSC - startTSC);
            else if (CLOCKSOURCE == HPET)
                logTime = endTime.tv_nsec - startTime.tv_nsec;
            F[j - 1][i - 1] = logTime;
        }
    }

    fingerprint_hash H = to_hash(F);
    return H;
}

fingerprint_hash read_hash(const std::string& in)
{
    fingerprint_hash H(n * FINGERPRINT_HASH_LINE);
    FILE* fin = fopen(in.c_str(), "r");
    for (int i = 0; i < n * FINGERPRINT_HASH_LINE; i++)
        fscanf(fin, "%lld", &H[i]);
    fclose(fin);
    return H;
}

void write_hash(const fingerprint_hash& H, const std::string& out)
{
    FILE* fout = fopen(out.c_str(), "w");
    for (int i = 0; i < n * FINGERPRINT_HASH_LINE; i++)
        fprintf(fout, "%lld ", H[i]);
    fclose(fout);
}

void write_encrypted_hash(const fingerprint_hash& H, const std::string& out)
{
    // Encrypt fingerprint
    size_t s_n, h_encrypted_n, msk_n;
    void *s_addr, *h_encrypted_addr, *msk_addr;
    {
        // length of input vectors
        size_t len = n * FINGERPRINT_HASH_LINE;
        // Configure new instance of the DDH encryption scheme
        cfe_ddh s;
        init_scheme(&s, len);

        // initialize fingerprint H as a cfe_vec h
        cfe_vec h;
        cfe_vec_init_C(&h, len);
        mpz_t el;
        mpz_init(el);
        for (size_t i = 0; i < len; i++) {
            mpz_set_ui(el, H[i]);
            cfe_vec_set_C(&h, el, i);
        }
        // encrypt vector h
        cfe_vec h_encrypted;
        cfe_vec msk = encrypt(&s, &h_encrypted, &h);

        // serialize data
        s_addr = cfe_ddh_serialize(&s_n, &s);
        h_encrypted_addr = cfe_vec_serialize(&h_encrypted_n, &h_encrypted);
        msk_addr = cfe_vec_serialize(&msk_n, &msk);
    }

    // Write encrypted fingerprint
    FILE* fout = fopen(out.c_str(), "wb");
    fwrite(&s_n, 1, sizeof(size_t), fout);
    fwrite(s_addr, 1, s_n, fout);
    fwrite(&h_encrypted_n, 1, sizeof(size_t), fout);
    fwrite(h_encrypted_addr, 1, h_encrypted_n, fout);
    fwrite(&msk_n, 1, sizeof(size_t), fout);
    fwrite(msk_addr, 1, msk_n, fout);
    fclose(fout);
}

long long dot_hash(const fingerprint_hash& myH, const fingerprint_hash& baseH)
{
    long long dp = 0;
    for (int i = 0; i < n * FINGERPRINT_HASH_LINE; i++) {
        dp += myH[i] * baseH[i];
    }
    return dp;
}

long long dot_hash_encrypted(const fingerprint_hash& myH, const std::string& baseHpath)
{
    // Read encrypted fingerprint
    size_t s_n, h_encrypted_n, msk_n;
    void *s_addr, *h_encrypted_addr, *msk_addr;
    {
        FILE* fin = fopen(baseHpath.c_str(), "rb");
        fread(&s_n, 1, sizeof(size_t), fin);
        s_addr = malloc(s_n);
        fread(s_addr, 1, s_n, fin);
        fread(&h_encrypted_n, 1, sizeof(size_t), fin);
        h_encrypted_addr = malloc(h_encrypted_n);
        fread(h_encrypted_addr, 1, h_encrypted_n, fin);
        fread(&msk_n, 1, sizeof(size_t), fin);
        msk_addr = malloc(msk_n);
        fread(msk_addr, 1, msk_n, fin);
        fclose(fin);
    }
    // Compute dot-product between old and new fingerprints

    // length of input vectors
    size_t len = n * FINGERPRINT_HASH_LINE;
    // initialize new fingerprint as cfe vector y
    cfe_vec y;
    cfe_vec_init_C(&y, len);
    mpz_t el;
    mpz_init(el);
    for (size_t i = 0; i < len; i++) {
        mpz_set_ui(el, myH[i]);
        cfe_vec_set_C(&y, el, i);
    }

    mpz_t xy;
    decrypt(s_addr, xy, &y, h_encrypted_addr, msk_addr);
    return mpz_get_ui(xy);
}

bool match(long long dp)
{
    // distinguish between the three types of collisions
    long long count = 2LL * (dp / (MODE_WEIGHT * MODE_WEIGHT)) + 1LL * (dp % (MODE_WEIGHT * MODE_WEIGHT) / MODE_WEIGHT) + 0LL * (dp % MODE_WEIGHT);
    /* count includes false collisions from hashing to a small domain
     * a true  positive set-membership result is always counted
     * a false positive set-membership occurs with probability
     *
     *          fp = 1 - (1 - 1/FINGERPRINT_HASH_LINE)^m
     *
     * (by m-wise independence of hash family and map padding)
     * So, if X is the real match rate,
     *
     *          E[count] = X + (2 * n - X) * fp
     *        Var[count] = (2 * n - X) * fp * (1 - fp)
     *
     * (by properties of the binomial distribution). So, an unbiased
     * estimator of X is
     *
     *         Xhat := (count - 2 * n * fp) / (1 - fp)
     * 		 E[Xhat] = X
     *     Var[Xhat] = (2 * n - X) * fp / (1 - fp) <= 2 * n * fp / (1 - fp)
     *
     * For reference, if n = 1000 and m = 50, fp = 0.095, var = 210, std = 14.5.
     * So, with high probability,
     *
     *         X - 3 * std <= Xhat <= X + 3 * std
     *         X - 43.5    <= Xhat <= X + 43.5
     *
     * Which is pretty good :)
     */
    double fp = 1.0 - pow(1.0 - 1.0 / FINGERPRINT_HASH_LINE, m);
    long long Xhat = (count - 2 * n * fp) / (1 - fp);
    long long std = (long long)sqrt(2 * n * fp / (1 - fp));
    printf("%lld (+%lld -%lld)/%lld\n", Xhat, 3 * std, 3 * std + (m * n / MODE_WEIGHT), 2LL * n);
    return Xhat >= threshold * 2LL * n;
}
