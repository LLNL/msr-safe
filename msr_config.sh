#!/bin/bash

usage() { echo "Usage: $0 [-r | -w] [-f foo.txt]" 1>&2; exit 1; }

if [ -z "$1" ] ||[  -z "$2" ] || [ -z "$3" ]
then
	usage;
fi

ARG1=$1;
ARG2=$2;
file=$3;

main() {
	if [ $ARG1 == "-r" ] && [ $ARG2 == "-f" ] && [ $file  != "" ]
	then
		echo "Reading into file $file";
		readMSRS;
		#testingPrintf;
	elif [ $ARG1 == "-w" ] && [ $ARG2 == "-f" ] && [ $file  != "" ]
	then
		echo "Writing from file $file";
		resetMSRS;
	else
		usage;
	fi
}

resetMSRS() {

# While loop to read in the MSR original configuration file
while read line
do
	array=($line) # This stores each line in an array

	# If a value was not read from one of the addresses, it will not write that MSR back
	if [ ${array[2]} != "0x" ] 
	then
		# Writing each MSR back to it's value from the configuration file. 
		echo "$(wrmsr -p${array[0]} ${array[1]} ${array[2]})";
	fi
done < $file

}

testingPrintf() {
	x=$(rdmsr 0x0C1)	
	x="0x${x}"
	printf "%3d 0x0C1 %17s description\n" 1 $x >> $file
}

readMSRS() {
threadID=0

for i in /dev/cpu/*/msr_safe
do
	# Store MSRS into file MSR_original_config.txt
	#							The data below can be found in msr-safe/msr-supplemental.h
	#
	#	MSR Name			Address		Mask in msr-safe
	#
	#	SMSR_PMC0,			0x0C1		0xFFFFFFFF, 0xFFFFFFFF 
	result=$(rdmsr -p$threadID 0x0C1)
	result="0x${result}"
	printf "%3d 0x0C1 %17s SMSR_PMC0: A counter register.\n" $threadID $result >> $file

	#	SMSR_PMC1,			0x0C2		0xFFFFFFFF, 0xFFFFFFFF 
	result=$(rdmsr -p$threadID 0x0C2)
	result="0x${result}"
	printf "%3d 0x0C2 %17s SMSR_PMC1: A counter register.\n" $threadID $result >> $file

	#	SMSR_PMC2,			0x0C3		0xFFFFFFFF, 0xFFFFFFFF 
	result=$(rdmsr -p$threadID 0x0C3)
	result="0x${result}"
	printf "%3d 0x0C3 %17s SMSR_PMC2: A counter register.\n" $threadID $result >> $file

	#	SMSR_PMC3,			0x0C4		0xFFFFFFFF, 0xFFFFFFFF 
	result=$(rdmsr -p$threadID 0x0C4)
	result="0x${result}"
	printf "%3d 0x0C4 %17s SMSR_PMC3: A counter register.\n" $threadID $result >> $file

	#	SMSR_PMC4,			0x0C5		0xFFFFFFFF, 0xFFFFFFFF 
	result=$(rdmsr -p$threadID 0x0C5)
	result="0x${result}"
	printf "%3d 0x0C5 %17s SMSR_PMC4: A counter register.\n" $threadID $result >> $file

	#	SMSR_PMC5,			0x0C6		0xFFFFFFFF, 0xFFFFFFFF 
	result=$(rdmsr -p$threadID 0x0C6)
	result="0x${result}"
	printf "%3d 0x0C6 %17s SMSR_PMC5: A counter register.\n" $threadID $result >> $file

	#	SMSR_PMC6,			0x0C7		0xFFFFFFFF, 0xFFFFFFFF 
	result=$(rdmsr -p$threadID 0x0C7)
	result="0x${result}"
	printf "%3d 0x0C7 %17s SMSR_PMC6: A counter register.\n" $threadID $result >> $file

	#	SMSR_PMC7,			0x0C8		0xFFFFFFFF, 0xFFFFFFFF 
	result=$(rdmsr -p$threadID 0x0C8)
	result="0x${result}"
	printf "%3d 0x0C8 %17s SMSR_PMC7: A counter register.\n" $threadID $result >> $file
	
	#	SMSR_PERFEVTSEL0		0x186		0xFFFFFFFF, 0x0   
	result=$(rdmsr -p$threadID 0x186)
	result="0x${result}"
	printf "%3d 0x186 %17s SMSR_PERFEVTSEL0: Controls matching performance counter.\n" $threadID $result >> $file        


	#	SMSR_PERFEVTSEL1,		0x187		0xFFFFFFFF, 0x0        
	result=$(rdmsr -p$threadID 0x187)
	result="0x${result}"
	printf "%3d 0x187 %17s SMSR_PERFEVTSEL1: Controls matching performance counter.\n" $threadID $result >> $file 

	#	SMSR_PERFEVTSEL2,		0x188		0xFFFFFFFF, 0x0        
	result=$(rdmsr -p$threadID 0x188)
	result="0x${result}"
	printf "%3d 0x188 %17s SMSR_PERFEVTSEL2: Controls matching performance counter.\n" $threadID $result >> $file

	#	SMSR_PERFEVTSEL3,		0x189		0xFFFFFFFF, 0x0        
	result=$(rdmsr -p$threadID 0x189)
	result="0x${result}"
	printf "%3d 0x189 %17s SMSR_PERFEVTSEL3: Controls matching performance counter.\n" $threadID $result >> $file

	#	SMSR_PERFEVTSEL4,		0x18A,	0xFFFFFFFF, 0x0        
	result=$(rdmsr -p$threadID 0x18A)
	result="0x${result}"
	printf "%3d 0x18A %17s SMSR_PERFEVTSEL4: Controls matching performance counter.\n" $threadID $result >> $file

	#	SMSR_PERFEVTSEL5,		0x18B,	0xFFFFFFFF, 0x0        
	result=$(rdmsr -p$threadID 0x18B)
	result="0x${result}"
	printf "%3d 0x18B %17s SMSR_PERFEVTSEL5: Controls matching performance counter.\n" $threadID $result >> $file

	#	SMSR_PERFEVTSEL6,		0x18C,	0xFFFFFFFF, 0x0        
	result=$(rdmsr -p$threadID 0x18C)
	result="0x${result}"
	printf "%3d 0x18C %17s SMSR_PERFEVTSEL6: Controls matching performance counter.\n" $threadID $result >> $file

	#	SMSR_PERFEVTSEL7,		0x18D,	0xFFFFFFFF, 0x0        
	result=$(rdmsr -p$threadID 0x18D)
	result="0x${result}"
	printf "%3d 0x18D %17s SMSR_PERFEVTSEL7: Controls matching performance counter.\n" $threadID $result >> $file
	
	#	SMSR_PERF_CTL,			0x199,	0x0,        0x00000001 
	result=$(rdmsr -p$threadID 0x199)
	result="0x${result}"
	printf "%3d 0x199 %17s SMSR_PERF_CTRL: Control for performance counters.\n" $threadID $result >> $file

	#	SMSR_CLOCK_MODULATION,		0x19A,	0x00000007, 0x0        
	result=$(rdmsr -p$threadID 0x19A)
	result="0x${result}"
	printf "%3d 0x19A %17s SMSR_CLOCK_MODULATION: Control for clock modulation.\n" $threadID $result >> $file

	#	SMSR_THERM_INTERRUPT,		0x19B,	0x01FFFF0F, 0x0        
	result=$(rdmsr -p$threadID 0x19B)
	result="0x${result}"
	printf "%3d 0x19B %17s SMSR_THERM_INTERRUPT: Control for thermal interrupts.\n" $threadID $result >> $file

	#	SMSR_THERM_STATUS,		0x19C,	0x00000AAA, 0x0        
	result=$(rdmsr -p$threadID 0x19C)
	result="0x${result}"
	printf "%3d 0x19C %17s SMSR_THERM_STATUS: Thermal status register.\n" $threadID $result >> $file

	#	SMSR_MISC_ENABLE,		0x1A0,	0x00010000, 0x00000040 
	result=$(rdmsr -p$threadID 0x1A0)
	result="0x${result}"
	printf "%3d 0x1A0 %17s SMSR_MISC_ENABLE: Various enables in this register.\n" $threadID $result >> $file

	#	SMSR_OFFCORE_RSP_0,		0x1A6,	0xFFFF8FFF, 0x0000003F 
	result=$(rdmsr -p$threadID 0x1A6)
	result="0x${result}"
	printf "%3d 0x1A6 %17s SMSR_OFFCORE_RSP_0: description here.\n" $threadID $result >> $file

	#	SMSR_OFFCORE_RSP_1,		0x1A7,	0xFFFF8FFF, 0x0000003F 
	result=$(rdmsr -p$threadID 0x1A7)
	result="0x${result}"
	printf "%3d 0x1A7 %17s SMSR_OFFCORE_RSP_1: description here.\n" $threadID $result >> $file
	
	#	SMSR_ENERGY_PERF_BIAS,		0x1B0,	0xF,        0x0        
	result=$(rdmsr -p$threadID 0x1B0)
	result="0x${result}"
	printf "%3d 0x1B0 %17s SMSR_ENERGY_PERF_BIAS: description here.\n" $threadID $result >> $file

	#	SMSR_PACKAGE_THERM_STATUS,	0x1B1,	0x00000555, 0x0        
	result=$(rdmsr -p$threadID 0x1B1)
	result="0x${result}"
	printf "%3d 0x1B1 %17s SMSR_PACKAGE_THERM_STATUS: Thermal status for package granularity.\n" $threadID $result >> $file

	#	SMSR_PACKAGE_THERM_INTERRUPT,	0x1B2,0x01FFFF07, 0x0        
	result=$(rdmsr -p$threadID 0x1B2)
	result="0x${result}"
	printf "%3d 0x1B2 %17s SMSR_PACKAGE_THERM_INTERRUPT: Thermal interrupt control for package granularity.\n" $threadID $result >> $file
	
	#	SMSR_FIXED_CTR0,		0x309,	0xFFFFFFFF, 0xFFFFFFFF 
	result=$(rdmsr -p$threadID 0x309)
	result="0x${result}"
	printf "%3d 0x309 %17s SMSR_FIXED_CTR0: Fixed counter 0.\n" $threadID $result >> $file

	#	SMSR_FIXED_CTR1,		0x30A,	0xFFFFFFFF, 0xFFFFFFFF 
	result=$(rdmsr -p$threadID 0x30A)
	result="0x${result}"
	printf "%3d 0x30A %17s SMSR_FIXED_CTR1: Fixed counter 1.\n" $threadID $result >> $file

	#	SMSR_FIXED_CTR2,		0x30B,	0xFFFFFFFF, 0xFFFFFFFF 
	result=$(rdmsr -p$threadID 0x30B)
	result="0x${result}"
	printf "%3d 0x30B %17s SMSR_FIXED_CTR2: Fixed counter 2.\n" $threadID $result >> $file

	#	SMSR_FIXED_CTR_CTRL,		0x38D,	0x00000BBB, 0x0        
	result=$(rdmsr -p$threadID 0x38D)
	result="0x${result}"
	printf "%3d 0x38D %17s SMSR_FIXED_CTR_CRL: Control for the fixed counters.\n" $threadID $result >> $file
	
	#	SMSR_PERF_GLOBAL_CTRL,		0x38F,	0x00000003, 0x00000007 
	result=$(rdmsr -p$threadID 0x38F)
	result="0x${result}"
	printf "%3d 0x38F %17s SMSR_PERF_GLOBAL_CTRL: Global control for performance counters.\n"  $threadID $result >> $file

	#	SMSR_PERF_GLOBAL_OVF_CTRL,	0x390,	0x00000003, 0xC0000007 
	result=$(rdmsr -p$threadID 0x390)
	result="0x${result}"
	printf "%3d 0x390 %17s SMSR_PERF_GLOBAL_OVFPCTRL: Global control for performance counter overflows.\n" $threadID $result >> $file

	#	SMSR_PEBS_ENABLE,		0x3F1,	0x0000000F, 0x0000000F 
	result=$(rdmsr -p$threadID 0x3F1)
	result="0x${result}"
	printf "%3d 0x3F1 %17s SMSR_PEBS_ENABLE: PEBS enable.\n" $threadID $result >> $file

	#	SMSR_PEBS_LD_LAT,		0x3F6,	0x0000FFFF, 0x0        
	result=$(rdmsr -p$threadID 0x3F6)
	result="0x${result}"
	printf "%3d 0x3F6 %17s SMSR_PEBS_LD_LAT: description here.\n" $threadID $result >> $file
	
	#	SMSR_PKG_POWER_LIMIT,		0x610,	0x00FFFFFF, 0x00FFFFFF 
	result=$(rdmsr -p$threadID 0x610)
	result="0x${result}"
	printf "%3d 0x610 %17s SMSR_PKG_POWER_LIMIT: Power capping can be done with this register.\n" $threadID $result >> $file

	#	SMSR_PP0_POWER_LIMIT,		0x638,	0x00FFFFFF, 0x0        
	result=$(rdmsr -p$threadID 0x638)
	result="0x${result}"
	printf "%3d 0x638 %17s SMSR_PP0_POWER_LIMIT: description here.\n" $threadID $result >> $file

	
	#	SMSR_TURBO_RATIO_LIMIT,	0x1AD, 0xFFFFFFFF, 0xFFFFFFFF 
	result=$(rdmsr -p$threadID 0x1AD)
	result="0x${result}"
	printf "%3d 0x1Ad %17s SMSR_TURBO_RATIO_LIMIT: description here.\n" $threadID $result >> $file

	#	SMSR_DRAM_POWER_LIMIT,		0x618,	0x00FFFFFF, 0x0        
	result=$(rdmsr -p$threadID 0x618)
	result="0x${result}"
	printf "%3d 0x618 %17s SMSR_DRAM_POWER_LIMIT: Power capping for DRAM can be done with this register.\n" $threadID $result >> $file
	threadID=$[$threadID +1];
done

}

main;
