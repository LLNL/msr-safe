// table_35_25.h
#ifndef __TABLE_35_25_H
#define __TABLE_35_25_H

/* Table 35-25 : Intel Xeon Processor E5 Family
 * (Based on Intel Microarchitecture Code Name Haswell)
 *
 * CPUID signature 06_3F
 *
 */
#define WL_TABLE_35_25 \
SMSR_ENTRY( SMSR_TURBO_RATIO_LIMIT1,    0x1AE, 0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_TURBO_RATIO_LIMIT2,    0x1AF, 0xFFFFFFFF, 0xFFFFFFFF),

#endif //__TABLE_35_25_H
