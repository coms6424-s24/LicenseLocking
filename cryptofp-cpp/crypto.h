#include <cifer/innerprod/simple/ddh.h>
#include <gmp.h>

// for testing purposes
constexpr bool ENCRYPT = true;

extern "C" {
/*
 * Initialize DDH encryption scheme
 */
void init_scheme(cfe_ddh* s, size_t len);

/*
 * Configures a DDH scheme, generates master public/secret
 * keys, encrypts x with the public key (within ciphertext)
 * and returns the secret key.
 *
 * NOTE: the role of public/secret keys is switched between
 * the paper and this application, since this makes more
 * sense for our application. To my understanding, these two
 * keys should be interchangeable since the secret key is
 * meant to ONLY decode the inner product <x, y> and not x
 * itself. This should allow the user to take dot products
 * locally.
 */
cfe_vec encrypt(cfe_ddh* scheme,
    cfe_vec* ciphertext,
    cfe_vec* x);

/*
 * Given serialized representations of the scheme, x's
 * encryption and the master secret key, it deserializes
 * them to their appropriate data types and decripts the
 * inner product <x, y>.
 */
void decrypt(void* scheme_addr,
    mpz_t xy,
    cfe_vec* y,
    void* x_encrypted_addr,
    void* msk_addr);

/*
 * Serialization and deserialization functions for
 * fingerprint storage.
 */
void* cfe_ddh_serialize(size_t* n, cfe_ddh* s);
void cfe_ddh_deserialize(cfe_ddh* s, void* addr);
void* cfe_vec_serialize(size_t* n, cfe_vec* v);
void cfe_vec_deserialize(cfe_vec* v, void* addr);

/*
 * Reexpose CiFEr functions with C linkage to fix C/C++
 * linker errors. I don't know what I'm doing.
 */
void cfe_vec_init_C(cfe_vec* v, size_t size);
void cfe_vec_set_C(cfe_vec* v, mpz_t el, size_t i);
}
