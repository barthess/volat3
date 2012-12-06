#!/bin/bash

COUNTER=10164
LIMIT=5000

while [  $LIMIT -le $COUNTER ]; do
	echo The counter is $COUNTER
	echo "$COUNTER,480,272,80,8,10,14,3,4,3" > /tmp/test.txt
	let COUNTER=COUNTER-1
	sleep 0
done
