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

## State

So far, this has been tested on my (Andrei's) laptop runing Ubuntu Linux and a random GCP instance.
The fingerprint was mostly stable on both machines, with no false positive matches observed.
