#!/bin/bash
for cpu in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24
do
	for i in 1 2 3 4 5
	do
		sudo ./msrko.x -src $cpu -target $cpu
	done
done
