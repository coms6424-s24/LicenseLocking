#include <cstdio>
#include <string>
#include <unistd.h>
#include <vector>

#include "fp_funcs.h"
#include "utils.h"

// scale the size of the timed instances
constexpr int scaling_factor = 30;

/*
 * Adapted from the Mozilla UNIX implementation of the
 * function RNG_SystemRNG (page 8, section 5).
 * (mozilla-central/security/nss/lib/freebl/unix_rand.c)
 * This should closely correspond to what the JavaScript
 * CryptoFP implementation was timing in the paper.
 *
 * NOTE: fingerprinting via this method is unstable,
 * supposedly because of the open() and read() syscalls,
 * which add too much variability to the timestamps. As
 * a consequence, the mode of the measurements is
 * meaningless.
 */
size_t RNG_SystemRNG(void *dest, size_t maxLen) {
  FILE *file;
  int fd;
  int bytes;
  size_t fileBytes = 0;
  unsigned char *buffer = (unsigned char *)dest;

  file = fopen("/dev/urandom", "r");
  if (file == NULL)
    return 0;
  /* Read from the underlying file descriptor directly to bypass stdio
   * buffering and avoid reading more bytes than we need from /dev/urandom.
   * NOTE: we can't use fread with unbuffered I/O because fread may return
   * EOF in unbuffered I/O mode on Android.
   */
  fd = fileno(file);
  while (maxLen > fileBytes && fd != -1) {
    bytes = maxLen - fileBytes;
    bytes = read(fd, buffer, bytes);
    if (bytes <= 0)
      break;
    fileBytes += bytes;
    buffer += bytes;
  }
  fclose(file);
  if (fileBytes != maxLen)
    fileBytes = 0;
  return fileBytes;
}
std::function<size_t(size_t)> make_rand() {
  // similar to 'unsigned char buffer[scaling_factor * n];'
  void *buffer = malloc(scaling_factor * n); // life is too short to free memory
  return
      [buffer](size_t j) { return RNG_SystemRNG(buffer, scaling_factor * j); };
}

/*
 * std::hash (page 5, section 3.3.3)
 */
std::function<size_t(size_t)> make_hash() {
  std::vector<std::string> seeds(n + 1);
  for (int j = 1; j <= n; j++)
    seeds[j] = std::string(scaling_factor * j, 'c');
  struct std::hash<std::string> H {};

  return [seeds, H](size_t j) { return H(seeds[j]); };
}

std::function<size_t(size_t)> make_fp_func(const Method &method) {
  if (method == HASH)
    return make_hash();
  else if (method == RAND)
    return make_rand();
  else
    exit(1);
}
