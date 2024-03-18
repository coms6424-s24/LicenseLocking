#include <cstdio>
#include <cstring>
#include <functional>

#include "fingerprint.h"
#include "fp_funcs.h"
#include "utils.h"

// which function is used for fingerprinting
constexpr Method method = HASH;

int main(int argc, char **argv) {
  if (argc == 2 && strcmp(argv[1], "-cmp")) {
    std::function<size_t(size_t)> fp_func = make_fp_func(method);
    make_fingerprint(fp_func, argv[1]);
  } else if (argc == 3 && strcmp(argv[1], "-cmp") && !strcmp(argv[2], "-cmp")) {
    std::function<size_t(size_t)> fp_func = make_fp_func(method);
    fingerprint myfp = make_fingerprint(fp_func);
    fingerprint base = read_fingerprint(argv[1]);
    if (match(myfp, base)) {
      printf("fingerprint match\n");
	  return 0;
	}
    else {
      printf("no match\n");
	  return 1;
	}
  } else {
    printf("usage: ./main <fingerprint_filename> [-cmp]\n");
    exit(1);
  }
  return 0;
}
