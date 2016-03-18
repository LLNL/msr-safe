#!/bin/bash
echo "MSR.KO pread/pwrite test"
uname -a
lscpu

for msr in 0x17a 0xe2 0xce
do
	for cpu in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
	do
		for i in 1 2 3
		do
			sudo ./msrko.x -src $cpu -target $cpu -msr $msr # -i 10
		done
	done
done
