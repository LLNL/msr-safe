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

#define ENTRY1    SMSR_ENTRY( NO_SUCH_SMSR,		0x000, 0x0,        0x0       ),
#define ENTRY_END SMSR_ENTRY( SMSR_LAST_ENTRY, 		0x000, 0x0,        0x0       )
#define DUMMY ENTRY1

// Each table_*.h file defines one WL_TABLE.

//#include "table_35_02.h" usued. 
#include "./whitelists/table_35_17.h"
#include "./whitelists/table_35_20.h"
#include "./whitelists/table_35_23.h"
#include "./whitelists/table_35_15.h"
#include "./whitelists/table_35_18.h"
//#include "table_35_21.h" conflict; talk to Kathleen.
#include "./whitelists/table_35_24.h"
#include "./whitelists/table_35_16.h"
#include "./whitelists/table_35_19.h"
#include "./whitelists/table_35_22.h"


#ifdef _USE_ARCH_063F
#define _USE_TABLE_35_24
#define SMSR_063F ENTRY1 ENTRY_END
//Table35_24 does not have a WL_ defined
//INCOMPLETE because for next gen Haswell
#endif // _USE_ARCH_063F

#ifdef _USE_ARCH_0646		//Haswell
// Temporarily removing WL_TABLE_35_21 because matches PEFVEVTSEL in WL_TABLE_35_15
#define SMSR_0646 WL_TABLE_35_15 WL_TABLE_35_16 WL_TABLE_35_18 WL_TABLE_35_22 WL_TABLE_35_23
#endif // _USE_ARCH_0646

#ifdef _USE_ARCH_0645		//Haswell
// Temporarily removing WL_TABLE_35_21 because matches PEFVEVTSEL in WL_TABLE_35_15
#define SMSR_0645 WL_TABLE_35_15 WL_TABLE_35_16 WL_TABLE_35_18 WL_TABLE_38_23
#endif // _USE_ARCH_0645

#ifdef _USE_ARCH_063C		//Haswell
// Temporarily removing WL_TABLE_35_21 because matches PEFVEVTSEL in WL_TABLE_35_15
#define SMSR_063C WL_TABLE_35_15 WL_TABLE_35_16 WL_TABLE_35_18 WL_TABLE_38_23
#endif // _USE_ARCH_063C

#ifdef _USE_ARCH_063E		//Ivy Bridge
#define SMSR_063E WL_TABLE_35_19 WL_TABLE_35_20 WL_TABLE_35_15
#endif // _USE_ARCH_063E

#ifdef _USE_ARCH_062D		//Sandy Bridge 
#define SMSR_062D WL_TABLE_35_17 WL_TABLE_35_15
#endif // _USE_ARCH_062D

#ifdef _USE_ARCH_062A		//Sandy Bridge 
#define SMSR_062A WL_TABLE_35_16 WL_TABLE_35_15
#endif // _USE_ARCH_062A

#ifdef _USE_ARCH_EMPTY  	//Default (unknown)
#define SMSR_EMPTY ENTRY1 ENTRY_END
#endif

#endif /* MSR_SUPPLEMENTAL_H */
