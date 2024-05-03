#!/bin/bash

# https://unix.stackexchange.com/questions/129391/passing-named-arguments-to-shell-scripts
cntfp=0
cnttc=0
cntst=0
cntel=0
while [ $# -gt 0 ]; do
	case "$1" in
		--fp=*)
			fp="${1#*=}"
			cntfp=$((cntfp+1))
			;;
		--test_count=*)
			test_count="${1#*=}"
			cnttc=$((cnttc+1))
			;;
		--stress=*)
			stress="${1#*=}"
			cntst=$((cntst+1))
			;;
		*)
			cntel=$((cntel+1))
			;;
	esac
	shift
done

if [ $cntfp != "1" ] || [ $cnttc != "1" ] || [ $cntst != "1" ] || [ $cntel != "0" ]
then
	printf "usage: taskset_stress_test --fp=<fingerprint-path> --test_count=<number-of-iterations> --stress=<0/1>\n"
	exit 1
fi

ncpus=$(nproc)

if [ $stress != "0" ]
then
	pids=""
	trap 'for p in $pids; do kill $p; done' 0
	for (( i=0; i<$ncpus; i++ )); do while : ; do : ; done & pids="$pids $!"; done
fi

for (( c=0; c<$ncpus; c++ ))
do
	fail_count=0
	for (( n=1; n<=$test_count; n++ ))
	do
		taskset -c $c ./main $fp -cmp 1> /dev/null
		ret=$?
		if [ $ret != "0" ] 
		then
			fail_count=$((fail_count+1))
		fi
	done
	printf 'cpu %d: %d/%d failed\n' "$c" "$fail_count" "$test_count"
done
