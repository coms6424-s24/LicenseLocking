# LicenseLocking

## Paper

https://www.s3.eurecom.fr/docs/ccs18_iskander.pdf

## Example
```
$ make
$ ./main
usage: ./main <fingerprint_filename> [-cmp]
$ ./main fingerprints/fingerprint_Andrei
$ ./main fingerprints/fingerprint_Andrei -cmp
1773/2000
fingerprint match
```

## Testing

On my (Andrei's) machine, the fingerprint, measured on the same operating system (Ubuntu 22.04.3 LTS), is stable
across a small number (e.g. 20) of measurements performed at random times within 14 days of fingerprint generation.
The fingerprint stability is preserved between battery/charging modes and the three power modes (performance,
balanced, power saving) provided by Ubuntu. Running the fingerprinting routine with CPU affinity under light load
```
$ ./taskset_test.sh
cpu 0: 2/1000 failed
cpu 1: 3/1000 failed
cpu 2: 4/1000 failed
cpu 3: 3/1000 failed
cpu 4: 2/1000 failed
cpu 5: 1/1000 failed
cpu 6: 9/1000 failed
cpu 7: 11/1000 failed
cpu 8: 12/1000 failed
cpu 9: 10/1000 failed
cpu 10: 7/1000 failed
cpu 11: 6/1000 failed
```
reveals a small probability of false negatives occurring. We also note that failures are not uniform among different 
cores.

GCP testing is currently limited to one GCP instance, with measurements at a short time horizon (i.e. minutes).
The fingerprint is generally stable, although few runs lead to false negatives. No false positives observed.
