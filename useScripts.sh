#!/bin/bash

file="MSR_original_config_$(date +%Y_%m_%d).txt";
COUNTER=1;

while [ $COUNTER -lt 900 ]
do
	echo "$(./reset_MSRs_from_file.sh)";
	echo "$(./save_MSRs_in_file.sh)";
	echo "$( date +%T )" >> results.txt;
	echo "$(diff Original.txt $file)" >> results.txt;

	echo "$(rm $file)";
	sleep 1;

	COUNTER=$[$COUNTER +1];

done

