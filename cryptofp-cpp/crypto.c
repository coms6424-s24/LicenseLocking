#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cifer/innerprod/simple/ddh.h>
#include <gmp.h>

void* mpz_serialize(size_t* n, const mpz_t op)
{
    return mpz_export(NULL, n, 1, 1, 1, 0, op);
}

void mpz_deserialize(mpz_t op, void* addr, size_t n)
{
    mpz_import(op, n, 1, 1, 1, 0, addr);
}

void* cfe_ddh_serialize(size_t* n, cfe_ddh* s)
{
    size_t bound_n, g_n, p_n, q_n;
    void* bound = mpz_serialize(&bound_n, s->bound);
    void* g = mpz_serialize(&g_n, s->g);
    void* p = mpz_serialize(&p_n, s->p);
    void* q = mpz_serialize(&q_n, s->q);

    *n = 5 * sizeof(size_t) + bound_n + g_n + p_n + q_n;
    char* buf = (char*)malloc(*n);
    char* dest = buf;
    // write l
    memcpy(dest, &s->l, sizeof(size_t));
    dest += sizeof(size_t);
    // write bound
    memcpy(dest, &bound_n, sizeof(size_t));
    dest += sizeof(size_t);
    memcpy(dest, bound, bound_n);
    dest += bound_n;
    // write g
    memcpy(dest, &g_n, sizeof(size_t));
    dest += sizeof(size_t);
    memcpy(dest, g, g_n);
    dest += g_n;
    // write p
    memcpy(dest, &p_n, sizeof(size_t));
    dest += sizeof(size_t);
    memcpy(dest, p, p_n);
    dest += p_n;
    // write q
    memcpy(dest, &q_n, sizeof(size_t));
    dest += sizeof(size_t);
    memcpy(dest, q, q_n);
    dest += q_n;

    return buf;
}

void cfe_ddh_deserialize(cfe_ddh* s, void* addr)
{
    char* src = addr;
    size_t bound_n, g_n, p_n, q_n;
    // read l
    memcpy(&s->l, src, sizeof(size_t));
    src += sizeof(size_t);
    // read bound
    memcpy(&bound_n, src, sizeof(size_t));
    src += sizeof(size_t);
    void* bound = malloc(bound_n);
    memcpy(bound, src, bound_n);
    src += bound_n;
    // read g
    memcpy(&g_n, src, sizeof(size_t));
    src += sizeof(size_t);
    void* g = malloc(g_n);
    memcpy(g, src, g_n);
    src += g_n;
    // read p
    memcpy(&p_n, src, sizeof(size_t));
    src += sizeof(size_t);
    void* p = malloc(p_n);
    memcpy(p, src, p_n);
    src += p_n;
    // read q
    memcpy(&q_n, src, sizeof(size_t));
    src += sizeof(size_t);
    void* q = malloc(q_n);
    memcpy(q, src, q_n);
    src += q_n;

    mpz_inits(s->bound, s->g, s->p, s->q, NULL);
    mpz_deserialize(s->bound, bound, bound_n);
    mpz_deserialize(s->g, g, g_n);
    mpz_deserialize(s->p, p, p_n);
    mpz_deserialize(s->q, q, q_n);

    free(bound);
    free(g);
    free(p);
    free(q);
    free(addr);
}

void* cfe_vec_serialize(size_t* n, cfe_vec* v)
{
    *n = sizeof(size_t);

    size_t vec_n[v->size];
    void* vec[v->size];
    for (int i = 0; i < v->size; i++) {
        vec[i] = mpz_serialize(&vec_n[i], v->vec[i]);
        *n += sizeof(size_t) + vec_n[i];
    }

    char* buf = (char*)malloc(*n);
    char* dest = buf;
    // write size
    memcpy(dest, &v->size, sizeof(size_t));
    dest += sizeof(size_t);
    // write vec
    for (int i = 0; i < v->size; i++) {
        memcpy(dest, &vec_n[i], sizeof(size_t));
        dest += sizeof(size_t);
        memcpy(dest, vec[i], vec_n[i]);
        dest += vec_n[i];
    }

    return buf;
}

void cfe_vec_deserialize(cfe_vec* v, void* addr)
{
    char* src = addr;
    // read size
    memcpy(&v->size, src, sizeof(size_t));
    src += sizeof(size_t);
    cfe_vec_init(v, v->size);
    // read vec
    for (int i = 0; i < v->size; i++) {
        size_t vec_n;
        memcpy(&vec_n, src, sizeof(size_t));
        src += sizeof(size_t);
        void* vec = malloc(vec_n);
        memcpy(vec, src, vec_n);
        src += vec_n;

        mpz_deserialize(v->vec[i], vec, vec_n);
        free(vec);
    }
    free(addr);
}

void init_scheme(cfe_ddh* s, size_t len)
{
    // 2 << 16 upper bound for input vector coordinates
    mpz_t bound;
    mpz_inits(bound, NULL);
    mpz_set_ui(bound, 2);
    mpz_pow_ui(bound, bound, 16);
    // bit length of prime modulus p
    int modulus_len = 1024;

    cfe_ddh_init(s, len, modulus_len, bound);
}

cfe_vec encrypt(cfe_ddh* scheme, cfe_vec* ciphertext, cfe_vec* x)
{
    // Copy DDH scheme
    cfe_ddh encryptor;
    cfe_ddh_copy(&encryptor, scheme);

    // Initialize and generate a pair of master secret key
    // and master public key for scheme s
    cfe_vec msk, mpk;
    cfe_ddh_master_keys_init(&msk, &mpk, &encryptor);
    cfe_ddh_generate_master_keys(&msk, &mpk, &encryptor);

    // Encrypt x using master public key
    cfe_ddh_ciphertext_init(ciphertext, &encryptor);
    cfe_ddh_encrypt(ciphertext, &encryptor, x, &mpk);

    // Return master secret key
    return msk;
}

void decrypt(void* scheme_addr,
    mpz_t xy,
    cfe_vec* y,
    void* x_encrypted_addr,
    void* msk_addr)
{
    // Deserialize DDH scheme
    cfe_ddh decryptor;
    cfe_ddh_deserialize(&decryptor, scheme_addr);
    // Deserialize ciphertext
    cfe_vec x_encrypted;
    cfe_vec_deserialize(&x_encrypted, x_encrypted_addr);
    // Deserialize master secret key
    cfe_vec msk;
    cfe_vec_deserialize(&msk, msk_addr);

    // derive functional encryption key for y
    mpz_t fe_key;
    mpz_init(fe_key);
    cfe_ddh_derive_fe_key(fe_key, &decryptor, &msk, y);

    // decrypt to obtain the result: inner product of x and y
    mpz_init(xy);
    cfe_ddh_decrypt(xy, &decryptor, &x_encrypted, fe_key, y);
}

void cfe_vec_init_C(cfe_vec* v, size_t size)
{
    cfe_vec_init(v, size);
}
void cfe_vec_set_C(cfe_vec* v, mpz_t el, size_t i)
{
    cfe_vec_set(v, el, i);
}
