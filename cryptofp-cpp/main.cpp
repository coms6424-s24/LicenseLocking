#include <cstdio>
#include <cstring>
#include <functional>
#include <unistd.h>

#include "fingerprint.h"
#include "fp_funcs.h"
#include "utils.h"

// which function is used for fingerprinting
constexpr Method method = HASH;

int main(int argc, char **argv) {
  // Assign task to CPU0
  // set_cpu(0);
  if (argc == 2 && strcmp(argv[1], "-cmp")) {
    std::function<size_t(size_t)> fp_func = make_fp_func(method);
    make_hash(fp_func, argv[1]);
  } else if (argc == 3 && strcmp(argv[1], "-cmp") && !strcmp(argv[2], "-cmp")) {
	std::function<size_t(size_t)> fp_func = make_fp_func(method);
	fingerprint_hash Hmine = make_hash(fp_func);
    fingerprint_hash Hbase = read_hash(argv[1]);

    if (match_hash(Hmine, Hbase)) {
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
