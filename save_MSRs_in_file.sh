#!/bin/bash

# Author: Kathleen Shoga

# Currently storing output in MSR_original_config.txt
# If you would like a date on your file, please use:
#
 file="MSR_original_config_$(date +%Y_%m_%d).txt";
# 

#file="MSR_original_config.txt"

# Store MSRS into file MSR_original_config.txt
#							The data below can be found in msr-safe/msr-supplemental.h
#								
#    Address   Execute Read	File to Save in		Which MSR 			Address		Mask	
echo "0x0C1 $(rdmsr 0x0C1)" >> 	$file		 	#SMSR_PMC0,			0x0C1,	0xFFFFFFFF, 0xFFFFFFFF 
echo "0x0C2 $(rdmsr 0x0C2)" >> 	$file			#SMSR_PMC1,			0x0C2,	0xFFFFFFFF, 0xFFFFFFFF 
echo "0x0C3 $(rdmsr 0x0C3)" >> 	$file			#SMSR_PMC2,			0x0C3,	0xFFFFFFFF, 0xFFFFFFFF 
echo "0x0C4 $(rdmsr 0x0C4)" >> 	$file			#SMSR_PMC3,			0x0C4,	0xFFFFFFFF, 0xFFFFFFFF 
echo "0x0C5 $(rdmsr 0x0C5)" >> 	$file			#SMSR_PMC4,			0x0C5,	0xFFFFFFFF, 0xFFFFFFFF 
echo "0x0C6 $(rdmsr 0x0C6)" >> 	$file			#SMSR_PMC5,			0x0C6,	0xFFFFFFFF, 0xFFFFFFFF 
echo "0x0C7 $(rdmsr 0x0C7)" >> 	$file			#SMSR_PMC6,			0x0C7,	0xFFFFFFFF, 0xFFFFFFFF 
echo "0x0C8 $(rdmsr 0x0C8)" >> 	$file			#SMSR_PMC7,			0x0C8,	0xFFFFFFFF, 0xFFFFFFFF 

echo "0x186 $(rdmsr 0x186)" >> 	$file			#SMSR_PERFEVTSEL0,		0x186,	0xFFFFFFFF, 0x0        
echo "0x187 $(rdmsr 0x187)" >> 	$file			#SMSR_PERFEVTSEL1,		0x187,	0xFFFFFFFF, 0x0        
echo "0x188 $(rdmsr 0x188)" >> 	$file			#SMSR_PERFEVTSEL2,		0x188,	0xFFFFFFFF, 0x0        
echo "0x189 $(rdmsr 0x189)" >> 	$file			#SMSR_PERFEVTSEL3,		0x189,	0xFFFFFFFF, 0x0        
echo "0x18A $(rdmsr 0x18A)" >> 	$file			#SMSR_PERFEVTSEL4,		0x18A,	0xFFFFFFFF, 0x0        
echo "0x18B $(rdmsr 0x18B)" >> 	$file			#SMSR_PERFEVTSEL5,		0x18B,	0xFFFFFFFF, 0x0        
echo "0x18C $(rdmsr 0x18C)" >> 	$file			#SMSR_PERFEVTSEL6,		0x18C,	0xFFFFFFFF, 0x0        
echo "0x18D $(rdmsr 0x18D)" >> 	$file			#SMSR_PERFEVTSEL7,		0x18D,	0xFFFFFFFF, 0x0        

echo "0x199 $(rdmsr 0x199)" >> 	$file			#SMSR_PERF_CTL,			0x199,	0x0,        0x00000001 
echo "0x19A $(rdmsr 0x19A)" >> 	$file			#SMSR_CLOCK_MODULATION,		0x19A,	0x00000007, 0x0        
echo "0x19B $(rdmsr 0x19B)" >> 	$file			#SMSR_THERM_INTERRUPT,		0x19B,	0x01FFFF0F, 0x0        
echo "0x19C $(rdmsr 0x19C)" >> 	$file			#SMSR_THERM_STATUS,		0x19C,	0x00000AAA, 0x0        
echo "0x1A0 $(rdmsr 0x1A0)" >> 	$file			#SMSR_MISC_ENABLE,		0x1A0,	0x00010000, 0x00000040 
echo "0x1A2 $(rdmsr 0x1A2)" >> 	$file			#SMSR_TEMPERATURE_TARGET,	0x1A2,	       0x0, 0x0        

echo "0x1A6 $(rdmsr 0x1A6)" >> 	$file			#SMSR_OFFCORE_RSP_0,		0x1A6,	0xFFFF8FFF, 0x0000003F 
echo "0x1A7 $(rdmsr 0x1A7)" >> 	$file			#SMSR_OFFCORE_RSP_1,		0x1A7,	0xFFFF8FFF, 0x0000003F 

echo "0x1B0 $(rdmsr 0x1B0)" >> 	$file			#SMSR_ENERGY_PERF_BIAS,		0x1B0,	0xF,        0x0        
echo "0x1B1 $(rdmsr 0x1B1)" >> 	$file			#SMSR_PACKAGE_THERM_STATUS,	0x1B1,	0x00000555, 0x0        
echo "0x1B2 $(rdmsr 0x1B2)" >> 	$file			#SMSR_PACKAGE_THERM_INTERRUPT,	0x1B2,0x01FFFF07, 0x0        

echo "0x309 $(rdmsr 0x309)" >> 	$file			#SMSR_FIXED_CTR0,		0x309,	0xFFFFFFFF, 0xFFFFFFFF 
echo "0x30A $(rdmsr 0x30A)" >> 	$file			#SMSR_FIXED_CTR1,		0x30A,	0xFFFFFFFF, 0xFFFFFFFF 
echo "0x30B $(rdmsr 0x30B)" >> 	$file			#SMSR_FIXED_CTR2,		0x30B,	0xFFFFFFFF, 0xFFFFFFFF 
echo "0x38D $(rdmsr 0x38D)" >> 	$file			#SMSR_FIXED_CTR_CTRL,		0x38D,	0x00000BBB, 0x0        

echo "0x38E $(rdmsr 0x38E)" >> 	$file			#SMSR_PERF_GLOBAL_STATUS,	0x38E,	0x00000003, 0xC0000007 
echo "0x38F $(rdmsr 0x38F)" >> 	$file			#SMSR_PERF_GLOBAL_CTRL,		0x38F,	0x00000003, 0x00000007 
echo "0x390 $(rdmsr 0x390)" >> 	$file			#SMSR_PERF_GLOBAL_OVF_CTRL,	0x390,	0x00000003, 0xC0000007 
echo "0x3F1 $(rdmsr 0x3F1)" >> 	$file			#SMSR_PEBS_ENABLE,		0x3F1,	0x0000000F, 0x0000000F 
echo "0x3F6 $(rdmsr 0x3F6)" >> 	$file			#SMSR_PEBS_LD_LAT,		0x3F6,	0x0000FFFF, 0x0        

echo "0x610 $(rdmsr 0x610)" >> 	$file			#SMSR_PKG_POWER_LIMIT,		0x610,	0x00FFFFFF, 0x00FFFFFF 
echo "0x638 $(rdmsr 0x638)" >> 	$file			#SMSR_PP0_POWER_LIMIT,		0x638,	0x00FFFFFF, 0x0        

echo "0x1AD $(rdmsr 0x1AD)" >> 	$file			#SMSR_TURBO_RATIO_LIMIT,	0x1AD, 0xFFFFFFFF, 0xFFFFFFFF 
echo "0x618 $(rdmsr 0x618)" >> 	$file			#SMSR_DRAM_POWER_LIMIT,		0x618,	0x00FFFFFF, 0x0        

