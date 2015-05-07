// table_35_19.h
#ifndef __TABLE_35_19_H
#define __TABLE_35_19_H
/* Table 35-19 : Intel Xeon Processor E5 v2 Product Family
 * (Based on Ivy Bridge-EP Microarchitecture)
 *
 * CPUID signature06_3EH
 *
 */
#define WL_TABLE_35_19 \
SMSR_ENTRY( SMSR_DRAM_POWER_LIMIT,      0x618,  0x00FFFFFF, 0x0       ),\
SMSR_ENTRY( SMSR_DRAM_ENERGY_STATUS,    0x619,  0x0,        0x0       ),\
SMSR_ENTRY( SMSR_DRAM_PERF_STATUS,      0x61B,  0x0,        0x0       ),\
SMSR_ENTRY( SMSR_DRAM_POWER_INFO,       0x61C,  0x0,        0x0       ),

#endif //__TABLE_35_19_H
