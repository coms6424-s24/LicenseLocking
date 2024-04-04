#!/bin/bash
TIMER=tsc
if [ -z "$1" ]
	then
		TIMER=$1
fi
echo $TIMER > /sys/devices/system/clocksource/clocksource0/current_clocksource
