// table_35_17.h
#ifndef __TABLE_35_17_H
#define __TABLE_35_17_H

/* Table 35-17 : Intel Xeon Processor E5 Family
 * (Based on Intel Microarchitecture Code Name Sandy Bridge)
 *
 * CPUID signature 06_2D
 *
 */
#define WH_TABLE_35_17 \
SMSR_ENTRY( SMSR_TURBO_RATIO_LIMIT,     0x1AD, 0xFFFFFFFF, 0xFFFFFFFF),\
\
SMSR_ENTRY( SMSR_MSR_PKG_PERF_STATUS,   0x613,  0x0,        0x0       ),\
SMSR_ENTRY( SMSR_DRAM_POWER_LIMIT,      0x618,  0x00FFFFFF, 0x0       ),\
SMSR_ENTRY( SMSR_DRAM_ENERGY_STATUS,    0x619,  0x0,        0x0       ),\
SMSR_ENTRY( SMSR_DRAM_PERF_STATUS,      0x61B,  0x0,        0x0       ),\
SMSR_ENTRY( SMSR_DRAM_POWER_INFO,       0x61C,  0x0,        0x0       ),

#endif //__TABLE_35_17_H
