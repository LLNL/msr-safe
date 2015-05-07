#ifndef MSR_SUPPLEMENTAL_H
#define MSR_SUPPLEMENTAL_H

/* TLCC2 machines are based on Sandy Bridge Server processors, family 06 model 2D.*/
/* Catalyst cluster based on 06_3E Ivy Bridge. */

// Uncore performance counters listed in			
//   Intel Xeon Processor E5 v2	and E7 v2 Product Families	
//   Uncore Performance Monitoring Reference Manual	
//   Reference number 329468-002
//   February 2014		

#define SMSR_NOWRITE  (0x0)
#define SMSR_NOREAD   (0x0)
#define SMSR_READALL  (0xFFFFFFFF)
#define SMSR_WRITEALL (0xFFFFFFFF)

#define ENTRY1 SMSR_ENTRY( NO_SUCH_SMSR,		0x000, 0x0,        0x0       ),

//ArchDefines------------------------------------------------------------------------------

#ifdef _USE_ARCH_063F
#define _USE_TABLE_35_24
//INCOMPLETE because for next gen Haswell
#endif // _USE_ARCH_063F

#ifdef _USE_ARCH_0646		//Haswell
#define _USE_TABLE_35_23
#define _USE_TABLE_35_22
#define _USE_TABLE_35_21
#define _USE_TABLE_35_18
#define _USE_TABLE_35_16
#define _USE_TABLE_35_15
#endif // _USE_ARCH_0646

#ifdef _USE_ARCH_0645		//Haswell
#define _USE_TABLE_35_23
#define _USE_TABLE_35_21
#define _USE_TABLE_35_18
#define _USE_TABLE_35_16
#define _USE_TABLE_35_15
#endif // _USE_ARCH_0645

#ifdef _USE_ARCH_063C		//Haswell
#define _USE_TABLE_35_23
#define _USE_TABLE_35_21
#define _USE_TABLE_35_18
#define _USE_TABLE_35_16
#define _USE_TABLE_35_15
#endif // _USE_ARCH_063C

#ifdef _USE_ARCH_063E		//Ivy Bridge
#define _USE_TABLE_35_20
#define _USE_TABLE_35_19
#define _USE_TABLE_35_15
#endif // _USE_ARCH_063E

#ifdef _USE_ARCH_IvyBridge	//Ivy Bridge
#define _USE_TABLE_35_18
#define _USE_TABLE_35_16
#define _USE_TABLE_35_15
#endif // _USE_IvyBridge

#ifdef _USE_ARCH_062D		//Sandy Bridge 
#define _USE_TABLE_35_17
#define _USE_TABLE_35_15
//#define _USE_TABLE_35_2
#endif // _USE_ARCH_062D

#ifdef _USE_ARCH_062A		//Sandy Bridge 
#define _USE_TABLE_35_16
#define _USE_TABLE_35_15
//#define _USE_TABLE_35_2
#endif // _USE_ARCH_062A

//The format for the following is:
/*	    Name		       Address  Low   	    High
 *	    					Write	    Write
 *	    					Mask	    Mask        */

//Haswell------------------------------------------------------------------------------

#ifdef _USE_TABLE_35_24
#include "whitelist/table_35_24.h"
#endif //_USE_TABLE_35_24

//------------------------------------------------------------------------------

#ifdef _USE_TABLE_35_23
#include "whitelist/table_35_23.h"
#endif //_USE_TABLE_35_23

//------------------------------------------------------------------------------

#ifdef _USE_TABLE_35_22
#include "whitelist/table_35_22.h"
#endif //_USE_TABLE_35_22

//------------------------------------------------------------------------------

#ifdef _USE_TABLE_35_21
#include "whitelist/table_35_21.h"
#endif //_USE_TABLE_35_21

//IvyBridge------------------------------------------------------------------------------

#ifdef _USE_TABLE_35_20
#include "whitelist/table_35_20.h"
#endif //_USE_TABLE_35_20

//------------------------------------------------------------------------------

#ifdef _USE_TABLE_35_19
#include "whitelist/table_35_19.h"
#endif //_USE_TABLE_35_19

//------------------------------------------------------------------------------

#ifdef _USE_TABLE_35_18
#include "whitelist/table_35_18.h"
#endif //_USE_TABLE_35_18

//SandyBridge------------------------------------------------------------------------------

#ifdef _USE_TABLE_35_17
#include "whitelist/table_35_17.h"
#endif //_USE_TABLE_35_17

//------------------------------------------------------------------------------

#ifdef _USE_TABLE_35_16
#include "whitelist/table_35_16.h"
#endif //_USE_TABLE_35_16

//------------------------------------------------------------------------------

#ifdef _USE_TABLE_35_15
/* Table 35-15 : Sandy Bridge
 *
 * CPUID signature 06_2DH and 06_2AH
 * Additional MSRs that are specific to 06_2AH are in
 * Table 35-15. 
 *
 */
#define TMP1 \
SMSR_ENTRY( SMSR_TIME_STAMP_COUNTER,	0x010,	0x0,        0x0       ),\
SMSR_ENTRY( SMSR_PLATFORM_ID,		0x017,	0x0,        0x0       ),\
SMSR_ENTRY( SMSR_PMC0,			0x0C1,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_PMC1,			0x0C2,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_PMC2,			0x0C3,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_PMC3,			0x0C4,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_PMC4,			0x0C5,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_PMC5,			0x0C6,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_PMC6,			0x0C7,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_PMC7,			0x0C8,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_MPERF,			0x0E7,	0x0,        0x0       ),\
SMSR_ENTRY( SMSR_APERF,			0x0E8,	0x0,        0x0       ),\
SMSR_ENTRY( SMSR_PERFEVTSEL0,		0x186,	0xFFFFFFFF, 0x0       ),\
SMSR_ENTRY( SMSR_PERFEVTSEL1,		0x187,	0xFFFFFFFF, 0x0       ),\
SMSR_ENTRY( SMSR_PERFEVTSEL2,		0x188,	0xFFFFFFFF, 0x0       ),\
SMSR_ENTRY( SMSR_PERFEVTSEL3,		0x189,	0xFFFFFFFF, 0x0       ),\
SMSR_ENTRY( SMSR_PERFEVTSEL4,		0x18A,	0xFFFFFFFF, 0x0       ),\
SMSR_ENTRY( SMSR_PERFEVTSEL5,		0x18B,	0xFFFFFFFF, 0x0       ),\
SMSR_ENTRY( SMSR_PERFEVTSEL6,		0x18C,	0xFFFFFFFF, 0x0       ),\
SMSR_ENTRY( SMSR_PERFEVTSEL7,		0x18D,	0xFFFFFFFF, 0x0       ),\
SMSR_ENTRY( SMSR_PERF_STATUS,		0x198,	0x0,        0x0       ),\
SMSR_ENTRY( SMSR_PERF_CTL,		0x199,	0x0,        0x0       ),\
SMSR_ENTRY( SMSR_CLOCK_MODULATION,	0x19A,	0x00000007, 0x0       ),\
SMSR_ENTRY( SMSR_THERM_INTERRUPT,	0x19B,	0x01FFFF0F, 0x0       ),\
SMSR_ENTRY( SMSR_THERM_STATUS,		0x19C,	0x00000AAA, 0x0       ),\
SMSR_ENTRY( SMSR_MISC_ENABLE,		0x1A0,	0x0       , 0x0       ),\
\
SMSR_ENTRY( SMSR_TEMPERATURE_TARGET,	0x1A2,	0x0,        0x0       ),\
SMSR_ENTRY( SMSR_OFFCORE_RSP_0,		0x1A6,	0xFFFF8FFF, 0x0000003F),\
SMSR_ENTRY( SMSR_OFFCORE_RSP_1,		0x1A7,	0xFFFF8FFF, 0x0000003F),\
\
SMSR_ENTRY( SMSR_ENERGY_PERF_BIAS,	0x1B0,	0xF,        0x0       ),\
SMSR_ENTRY( SMSR_PACKAGE_THERM_STATUS,	0x1B1,	0x00000555, 0x0       ),\
SMSR_ENTRY( SMSR_PACKAGE_THERM_INTERRUPT,0x1B2,0x01FFFF07, 0x0       ),\
SMSR_ENTRY( SMSR_FIXED_CTR0,		0x309,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_FIXED_CTR1,		0x30A,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_FIXED_CTR2,		0x30B,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_PERF_CAPABILITIES,	0x345,	0x0,        0x0       ),\
SMSR_ENTRY( SMSR_FIXED_CTR_CTRL,	0x38D,	0x00000BBB, 0x0       ),\
SMSR_ENTRY( SMSR_PERF_GLOBAL_STATUS,	0x38E,	0x0       , 0x0       ),\
SMSR_ENTRY( SMSR_PERF_GLOBAL_CTRL,	0x38F,	0x00000003, 0x00000007),\
SMSR_ENTRY( SMSR_PERF_GLOBAL_OVF_CTRL,	0x390,	0x00000003, 0xC0000007),\
SMSR_ENTRY( SMSR_PEBS_ENABLE,		0x3F1,	0x0000000F, 0x0000000F),\
\
SMSR_ENTRY( SMSR_PEBS_LD_LAT,		0x3F6,	0x0000FFFF, 0x0       ),\
SMSR_ENTRY( SMSR_RAPL_POWER_UNIT,	0x606,	0x0,        0x0       ),\
SMSR_ENTRY( SMSR_PKG_POWER_LIMIT,	0x610,	0x00FFFFFF, 0x00FFFFFF),\
SMSR_ENTRY( SMSR_PKG_ENERGY_STATUS,	0x611,	0x0,        0x0       ),\
SMSR_ENTRY( SMSR_PKG_POWER_INFO,	0x614,	0x0,        0x0       ),\
SMSR_ENTRY( SMSR_PP0_POWER_LIMIT,	0x638,	0x00FFFFFF, 0x0       ),\
SMSR_ENTRY( SMSR_PP0_ENERGY_STATUS,	0x639,	0x0,        0x0       ),\

#endif	//Table 35-15

//Architectural------------------------------------------------------------------------------

#ifdef _USE_TABLE_35_2 		
/* Table 35-2 Architectural MSRs		
 *
 * 
 */

#define TMP0 \
SMSR_ENTRY( SMSR_TIME_STAMP_COUNTER,	0x010,	0x0,        0x0       ),\
SMSR_ENTRY( SMSR_PLATFORM_ID,		0x017,	0x0,        0x0       ),\
SMSR_ENTRY( SMSR_PMC0,			0x0C1,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_PMC1,			0x0C2,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_PMC2,			0x0C3,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_PMC3,			0x0C4,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_PMC4,			0x0C5,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_PMC5,			0x0C6,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_PMC6,			0x0C7,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_PMC7,			0x0C8,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_MPERF,			0x0E7,	0x0,        0x0       ),\
SMSR_ENTRY( SMSR_APERF,			0x0E8,	0x0,        0x0       ),\
SMSR_ENTRY( SMSR_PERFEVTSEL0,		0x186,	0xFFFFFFFF, 0x0       ),\
SMSR_ENTRY( SMSR_PERFEVTSEL1,		0x187,	0xFFFFFFFF, 0x0       ),\
SMSR_ENTRY( SMSR_PERFEVTSEL2,		0x188,	0xFFFFFFFF, 0x0       ),\
SMSR_ENTRY( SMSR_PERFEVTSEL3,		0x189,	0xFFFFFFFF, 0x0       ),\
SMSR_ENTRY( SMSR_PERFEVTSEL4,		0x18A,	0xFFFFFFFF, 0x0       ),\
SMSR_ENTRY( SMSR_PERFEVTSEL5,		0x18B,	0xFFFFFFFF, 0x0       ),\
SMSR_ENTRY( SMSR_PERFEVTSEL6,		0x18C,	0xFFFFFFFF, 0x0       ),\
SMSR_ENTRY( SMSR_PERFEVTSEL7,		0x18D,	0xFFFFFFFF, 0x0       ),\
SMSR_ENTRY( SMSR_PERF_STATUS,		0x198,	0x0,        0x0       ),\
SMSR_ENTRY( SMSR_PERF_CTL,		0x199,	0x0,        0x00000001),\
SMSR_ENTRY( SMSR_CLOCK_MODULATION,	0x19A,	0x00000007, 0x0       ),\
SMSR_ENTRY( SMSR_THERM_INTERRUPT,	0x19B,	0x01FFFF0F, 0x0       ),\
SMSR_ENTRY( SMSR_THERM_STATUS,		0x19C,	0x00000AAA, 0x0       ),\
SMSR_ENTRY( SMSR_MISC_ENABLE,		0x1A0,	0x0       , 0x0       ),\
SMSR_ENTRY( SMSR_ENERGY_PERF_BIAS,	0x1B0,	0xF,        0x0       ),\
SMSR_ENTRY( SMSR_PACKAGE_THERM_STATUS,	0x1B1,	0x00000555, 0x0       ),\
SMSR_ENTRY( SMSR_PACKAGE_THERM_INTERRUPT,0x1B2,0x01FFFF07, 0x0       ),\
SMSR_ENTRY( SMSR_FIXED_CTR0,		0x309,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_FIXED_CTR1,		0x30A,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_FIXED_CTR2,		0x30B,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_PERF_CAPABILITIES,	0x345,	0x0,        0x0       ),\
SMSR_ENTRY( SMSR_FIXED_CTR_CTRL,	0x38D,	0x00000BBB, 0x0       ),\
SMSR_ENTRY( SMSR_PERF_GLOBAL_STATUS,	0x38E,	0x0       , 0x0       ),\
SMSR_ENTRY( SMSR_PERF_GLOBAL_CTRL,	0x38F,	0x00000003, 0x00000007),\
SMSR_ENTRY( SMSR_PERF_GLOBAL_OVF_CTRL,	0x390,	0x00000003, 0xC0000007),\
SMSR_ENTRY( SMSR_PEBS_ENABLE,		0x3F1,	0x0000000F, 0x0000000F),

#endif 	//Table 35-2

//------------------------------------------------------------------------------

#ifdef _USE_E5v2_E7v2_UNCORE
/* Table 35-15 : Sandy Bridge
 *
 * CPUID signature 06_2DH and 06_2AH
 * Additional MSRs that are specific to 06_2AH are in
 * Table 35-15. 
 *
 */
#define TMP1 \


SMSR_ENTRY( SMSR_C0_MSR_PMON_BOX_CTL,	0x0D04,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C1_MSR_PMON_BOX_CTL,	0x0D24,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C2_MSR_PMON_BOX_CTL,	0x0D44,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C3_MSR_PMON_BOX_CTL,	0x0D64,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C4_MSR_PMON_BOX_CTL,	0x0D84,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C5_MSR_PMON_BOX_CTL,	0x0DA4,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C6_MSR_PMON_BOX_CTL,	0x0DC4,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C7_MSR_PMON_BOX_CTL,	0x0DE4,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C0_MSR_PMON_BOX_FILTER,0x0D14,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C1_MSR_PMON_BOX_FILTER,0x0D34,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C2_MSR_PMON_BOX_FILTER,0x0D54,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C3_MSR_PMON_BOX_FILTER,0x0D74,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C4_MSR_PMON_BOX_FILTER,0x0D94,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C5_MSR_PMON_BOX_FILTER,0x0DB4,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C6_MSR_PMON_BOX_FILTER,0x0DD4,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C7_MSR_PMON_BOX_FILTER,0x0DF4,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C0_MSR_PMON_CTL0,	0x0D10,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C1_MSR_PMON_CTL0,	0x0D30,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C2_MSR_PMON_CTL0,	0x0D50,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C3_MSR_PMON_CTL0,	0x0D70,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C4_MSR_PMON_CTL0,	0x0D90,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C5_MSR_PMON_CTL0,	0x0DB0,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C6_MSR_PMON_CTL0,	0x0DD0,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C7_MSR_PMON_CTL0,	0x0DF0,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C0_MSR_PMON_CTL1,	0x0D11,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C1_MSR_PMON_CTL1,	0x0D31,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C2_MSR_PMON_CTL1,	0x0D51,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C3_MSR_PMON_CTL1,	0x0D71,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C4_MSR_PMON_CTL1,	0x0D91,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C5_MSR_PMON_CTL1,	0x0DB1,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C6_MSR_PMON_CTL1,	0x0DD1,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C7_MSR_PMON_CTL1,	0x0DF1,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C0_MSR_PMON_CTR0,	0x0D16,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C1_MSR_PMON_CTR0,	0x0D36,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C2_MSR_PMON_CTR0,	0x0D56,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C3_MSR_PMON_CTR0,	0x0D76,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C4_MSR_PMON_CTR0,	0x0D96,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C5_MSR_PMON_CTR0,	0x0DB6,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C6_MSR_PMON_CTR0,	0x0DD6,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C7_MSR_PMON_CTR0,	0x0DF6,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C0_MSR_PMON_CTR1,	0x0D17,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C1_MSR_PMON_CTR1,	0x0D37,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C2_MSR_PMON_CTR1,	0x0D57,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C3_MSR_PMON_CTR1,	0x0D77,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C4_MSR_PMON_CTR1,	0x0D97,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C5_MSR_PMON_CTR1,	0x0DB7,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C6_MSR_PMON_CTR1,	0x0DD7,	0xFFFFFFFF, 0xFFFFFFFF),\
SMSR_ENTRY( SMSR_C7_MSR_PMON_CTR1,	0x0DF7,	0xFFFFFFFF, 0xFFFFFFFF),
#endif //_USE_E5v2_E7v2_UNCORE

#define ENTRY_END SMSR_ENTRY( SMSR_LAST_ENTRY, 		0x000, 0x0,        0x0       )

//To use if architecture not recognized
#ifdef _USE_ARCH_EMPTY
#define SMSR_EMPTY ENTRY1 ENTRY_END
#endif

#ifdef _USE_ARCH_063F
#define SMSR_063F ENTRY1 ENTRY_END
//Table35_24 does not have a TMP defined
//INCOMPLETE because for next gen Haswell
#endif // _USE_ARCH_063F

#ifdef _USE_ARCH_0646		//Haswell
#define SMSR_0646 WH_TABLE_35_16 TMP1
// Temporarily removing WL_TABLE_35_21 because matches PEFVEVTSEL
// in TMP1
//Table35_23 does not have a TMP defined
//Table35_22 does not have a TMP defined
//Table35_18 does not have a TMP defined
#endif // _USE_ARCH_0646

#ifdef _USE_ARCH_0645		//Haswell
#define SMSR_0645 WH_TABLE_35_16 TMP1
// Temporarily removing WL_TABLE_35_21 because matches PEFVEVTSEL
// in TMP1
//Table35_23 does not have a TMP defined
//Table35_18 does not have a TMP defined
#endif // _USE_ARCH_0645

#ifdef _USE_ARCH_063C		//Haswell
#define SMSR_063C WH_TABLE_35_16 TMP1
// Temporarily removing WL_TABLE_35_21 because matches PEFVEVTSEL
// in TMP1
//Table35_23 does not have a TMP defined
//Table35_18 does not have a TMP defined
#endif // _USE_ARCH_063C

#ifdef _USE_ARCH_063E		//Ivy Bridge
#define SMSR_063E WL_TABLE_35_19 TMP1
//Table35_20 does not have a TMP defined
#endif // _USE_ARCH_063E

#ifdef _USE_ARCH_IvyBridge	//Ivy Bridge
#define SMSR_IvyBridge WH_TABLE_35_16 TMP1
//Table35_18 does not have a TMP defined
#endif // _USE_IvyBridge

#ifdef _USE_ARCH_062D		//Sandy Bridge 
#define SMSR_062D WH_TABLE_35_17 TMP1
//Taking out architectural table
//#define SMSR_ENTRIES WH_TABLE_35_17 TMP1 TMP0 ENTRY_END
#endif // _USE_ARCH_062D

#ifdef _USE_ARCH_062A		//Sandy Bridge 
#define SMSR_062A WH_TABLE_35_16 TMP1
//Taking out architectural table
//#define SMSR_ENTRIES WH_TABLE_35_16 TMP1 TMP0 ENTRY_END
#endif // _USE_ARCH_062A
#endif /* MSR_SUPPLEMENTAL_H */
