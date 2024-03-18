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

On my machine, the fingerprint, measured on the same operating system (Ubuntu 22.04.3 LTS), is stable
across multiple (e.g. 20) measurements performed at random times within 14 days of fingerprint generation.
The fingerprint stability is preserved between battery/charging modes and the three power modes (performance,
balanced, power saving) provided by Ubuntu.

GCP testing is currently limited to one GCP instance, with measurements at a short time horizon (i.e. minutes).
The fingerprint is generally stable, although few runs lead to false negatives. No false positives observed.
