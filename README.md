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

On one single machine, the fingerprint, measured on the same operating system (Ubuntu 22.04.3 LTS), is stable
across measurements performed at random times within 14 days of fingerprint generation. The fingerprint stability 
is preserved between battery/charging modes, frequency-affecting power modes (performance, balanced, power saving)
and system load variations. Running the fingerprinting routine with CPU affinity under light load
```
$ ./taskset_test.sh --fp=./fingerprints/fingerprint_Andrei_noload --test_count=1000 --stress=0
cpu 0: 4/1000 failed
cpu 1: 4/1000 failed
cpu 2: 3/1000 failed
cpu 3: 4/1000 failed
cpu 4: 2/1000 failed
cpu 5: 4/1000 failed
cpu 6: 6/1000 failed
cpu 7: 2/1000 failed
cpu 8: 0/1000 failed
cpu 9: 2/1000 failed
cpu 10: 0/1000 failed
cpu 11: 5/1000 failed
```
reveals a large probability (99.5%) of success, and running under heavy load
```
$ ./taskset_test.sh --fp=./fingerprints/fingerprint_Andrei_noload --test_count=1000 --stress=1
<TODO>
```
reveals ...

GCP testing will be included here soon.
