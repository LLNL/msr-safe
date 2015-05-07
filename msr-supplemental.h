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

//ArchDefines------------------------------------------------------------------------------

#ifdef _USE_ARCH_063F
#define _USE_TABLE_35_24
#define SMSR_063F ENTRY1 ENTRY_END
//Table35_24 does not have a WL_ defined
//INCOMPLETE because for next gen Haswell
#endif // _USE_ARCH_063F

#ifdef _USE_ARCH_0646		//Haswell
#include "whitelist/table_35_23.h"
#include "whitelist/table_35_22.h"
#include "whitelist/table_35_21.h"
#include "whitelist/table_35_18.h"
#include "whitelist/table_35_16.h"
#include "whitelist/table_35_15.h"
// Temporarily removing WL_TABLE_35_21 because matches PEFVEVTSEL in WH_TABLE_35_15
#define SMSR_0646 WH_TABLE_35_15 WH_TABLE_35_16 WH_TABLE_35_18 WH_TABLE_35_22 WH_TABLE_35_23
#endif // _USE_ARCH_0646

#ifdef _USE_ARCH_0645		//Haswell
#include "whitelist/table_35_23.h"
#include "whitelist/table_35_21.h"
#include "whitelist/table_35_18.h"
#include "whitelist/table_35_16h"
#include "whitelist/table_35_15.h"
#endif // _USE_ARCH_0645

#ifdef _USE_ARCH_063C		//Haswell
#include "whitelist/table_35_23.h"
#include "whitelist/table_35_21.h"
#include "whitelist/table_35_18.h"
#include "whitelist/table_35_16.h"
#include "whitelist/table_35_15.h"
#endif // _USE_ARCH_063C

#ifdef _USE_ARCH_063E		//Ivy Bridge
#include "whitelist/table_02_12.h"
#include "whitelist/table_35_20.h"
#include "whitelist/table_35_19.h"
#include "whitelist/table_35_15.h"
#endif // _USE_ARCH_063E

#ifdef _USE_ARCH_IvyBridge	//Ivy Bridge
#include "whitelist/table_35_18.h"
#include "whitelist/table_35_16.h"
#include "whitelist/table_35_15.h"
#endif // _USE_IvyBridge

#ifdef _USE_ARCH_062D		//Sandy Bridge 
#include "whitelist/table_35_17.h"
#include "whitelist/table_35_15.h"
//#include "whitelist/table_35_02.h"
#endif // _USE_ARCH_062D

#ifdef _USE_ARCH_062A		//Sandy Bridge 
#include "whitelist/table_35_16.h"
#include "whitelist/table_35_15.h"
//#include "whitelist/table_35_02.h"
#endif // _USE_ARCH_062A

#ifdef _USE_ARCH_EMPTY  	//Default (unknown)
#define SMSR_EMPTY ENTRY1 ENTRY_END
#endif








#ifdef _USE_ARCH_0645		//Haswell
#define SMSR_0645 WH_TABLE_35_16 WH_TABLE_35_15
// Temporarily removing WL_TABLE_35_21 because matches PEFVEVTSEL
// in WH_TABLE_35_15
//Table35_23 does not have a TMP defined
//Table35_18 does not have a TMP defined
#endif // _USE_ARCH_0645

#ifdef _USE_ARCH_063C		//Haswell
#define SMSR_063C WH_TABLE_35_16 WH_TABLE_35_15
// Temporarily removing WL_TABLE_35_21 because matches PEFVEVTSEL
// in WH_TABLE_35_15
//Table35_23 does not have a TMP defined
//Table35_18 does not have a TMP defined
#endif // _USE_ARCH_063C

#ifdef _USE_ARCH_063E		//Ivy Bridge
#define SMSR_063E WL_TABLE_35_19 WH_TABLE_35_15
//Table35_20 does not have a TMP defined
#endif // _USE_ARCH_063E

#ifdef _USE_ARCH_IvyBridge	//Ivy Bridge
#define SMSR_IvyBridge WH_TABLE_35_16 WH_TABLE_35_15
//Table35_18 does not have a TMP defined
#endif // _USE_IvyBridge

#ifdef _USE_ARCH_062D		//Sandy Bridge 
#define SMSR_062D WH_TABLE_35_17 WH_TABLE_35_15
//Taking out architectural table
//#define SMSR_ENTRIES WH_TABLE_35_17 WH_TABLE_35_15 WL_TABLE_35_2 ENTRY_END
#endif // _USE_ARCH_062D

#ifdef _USE_ARCH_062A		//Sandy Bridge 
#define SMSR_062A WH_TABLE_35_16 WH_TABLE_35_15
//Taking out architectural table
//#define SMSR_ENTRIES WH_TABLE_35_16 WH_TABLE_35_15 WL_TABLE_35_2 ENTRY_END
#endif // _USE_ARCH_062A
#endif /* MSR_SUPPLEMENTAL_H */
