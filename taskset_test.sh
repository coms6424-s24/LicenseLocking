#!/bin/bash

ncpus=$(nproc --all)
test_count=1000
for (( c=0; c<$ncpus; c++ ))
do
	fail_count=0
	for (( n=1; n<=$test_count; n++ ))
	do
		taskset -c $c ./main fingerprints/fingerprint_Andrei -cmp 1> /dev/null
		ret=$?
		if [ $ret != "0" ] 
		then
			fail_count=$((fail_count+1))
		fi
	done
	printf 'cpu %d: %d/%d failed\n' "$c" "$fail_count" "$test_count"
done
