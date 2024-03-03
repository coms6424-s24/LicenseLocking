#ifndef __FP_FUNCS_H__
#define __FP_FUNCS_H__

#include <functional>
#include <stddef.h>

enum Method {RAND, HASH};

/*
 * Creates a function to be fingerprinted
 */
std::function<size_t(size_t)> make_fp_func(const Method& method);

#endif
