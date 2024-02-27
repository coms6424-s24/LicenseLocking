#include <immintrin.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

// scale number of random bytes generated
constexpr int scaling_factor = 1;

/*
 * The 'clock_gettime' function seems to most closely
 * resemble the Windows datetimeapi.h 'GetDateFormat' 
 * (presumably used in the paper, page 2, section 1); 
 * this reads the WALL CLOCK, not the CPU clock
 */
int _clock_gettime(clockid_t clk_id, struct timespec *res) {
	_mm_mfence();
	int retval = clock_gettime(CLOCK_REALTIME, res);
	_mm_mfence();
	return retval;
}

/*
 * Adapted from the Mozilla UNIX implementation of the
 * function RNG_SystemRNG (page 8, section 5).
 * (mozilla-central/security/nss/lib/freebl/unix_rand.c)
 * This should closely correspond to what the JavaScript
 * CryptoFP implementation was timing in the paper.
 */
size_t RNG_SystemRNG(void *dest, size_t maxLen) {
    FILE *file;
    int fd;
    int bytes;
    size_t fileBytes = 0;
    unsigned char *buffer = (unsigned char *) dest;

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

int main() {
	int n = 1000, m = 50; // page 5, section 3.3.3
	long long fp[n][m];
	unsigned char buffer[scaling_factor * n];
	for(int i = 1; i <= m; i++) {
		for(int j = 1; j <= n; j++) {	
			struct timespec startTime, endTime;
			_clock_gettime(CLOCK_REALTIME, &startTime);
			RNG_SystemRNG(buffer, scaling_factor * j);
			_clock_gettime(CLOCK_REALTIME, &endTime);
			long long logTime = endTime.tv_nsec - startTime.tv_nsec;
			fp[j - 1][i - 1] = logTime;
		}
	}
	FILE *fout = fopen("fingerprint", "w");
	for(int j = 0; j < n; j++) {
		for(int i = 0; i < m; i++)
			fprintf(fout, "%lld ", fp[j][i]);
		fprintf(fout, "\n");
	}
	fclose(fout);
	return 0;
}
