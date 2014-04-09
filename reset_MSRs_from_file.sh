#!/bin/bash

# Author: Kathleen Shoga

# While loop to read in the MSR original configuration file
while read line
do
	array=($line) # This stores each line in an array

	# If a value was not read from one of the addresses, it will not write that MSR back
	if [ ${array[1]} ] 
	then
		# Writing each MSR back to it's value from the configuration file. 
		echo "wrmsr ${array[0]} ${array[1]}";
	fi
done < MSR_original_config.txt
