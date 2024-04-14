#include <cstdio>
#include <cstring>
#include <functional>
#include <unistd.h>

#include "crypto.h"
#include "fingerprint.h"
#include "fp_funcs.h"
#include "utils.h"

// which function is used for fingerprinting
constexpr Method method = HASH;

int main(int argc, char** argv)
{
    // Assign task to CPU0
    // set_cpu(0);

    if (argc == 2 && strcmp(argv[1], "-cmp")) {
        // Compute fingerprint
        std::function<size_t(size_t)> fp_func = make_fp_func(method);
        fingerprint_hash H = make_hash(fp_func);
        // Write (encrypted) fingerprint
        if (ENCRYPT)
            write_encrypted_hash(H, argv[1]);
        else
            write_hash(H, argv[1]);
    } else if (argc == 3 && strcmp(argv[1], "-cmp") && !strcmp(argv[2], "-cmp")) {
        // Compute fingerprint
        std::function<size_t(size_t)> fp_func = make_fp_func(method);
        fingerprint_hash Hmine = make_hash(fp_func);
        // Compute dot-product similarity of (encrypted) fingerprints
        long long dp;
        if (ENCRYPT)
            dp = dot_hash_encrypted(Hmine, argv[1]);
        else
            dp = dot_hash(Hmine, read_hash(argv[1]));
        // Check for match
        if (match(dp)) {
            printf("fingerprint match\n");
            return 0;
        } else {
            printf("no match\n");
            return 1;
        }
    } else {
        printf("usage: ./main <fingerprint_filename> [-cmp]\n");
        exit(1);
    }
    return 0;
}
