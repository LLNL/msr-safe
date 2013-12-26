#ifndef MSR_SUPPLEMENTAL_H
#define MSR_SUPPLEMENTAL_H

/* TLCC2 machines are based on Sandy Bridge Server processors, family 06 model 2D.*/


#define SMSR_NOWRITE  (0x0)
#define SMSR_NOREAD   (0x0)
#define SMSR_READALL  (0xFFFFFFFF)
#define SMSR_WRITEALL (0xFFFFFFFF)

#ifdef _USE_ARCH_062D

/* TODO:  Fixed performance counters, offcore response, uncore. */

/* 
 * References are to the September 2013 edition of the Intel documentation.
 *
 * Reserved bits are assumed to contain potentially sensitive information and are 
 * masked for both reading and writing.
 *
 * Values are for processor family 06 model 2D, "Intel Xeon Processor E5 Family
 * based on the Intel microarchitecture code name Sandy Bridge" (Table 35-1).
 *
 * Relevant tables are:
 * 	Page 35-102
 * 	Section 35.8 MSRs in Intel Processor Family Based on Intel Microarchitecture Sandy Bridge
 * 	Table 35-12 MSRs Suppored by Intel Processors based on Intel microarchitecture code name Sandy Bridge
 *
 * 	Page 35-121
 * 	Section 35.8.2 MSRs in Intel Xeon Processor E5 Family 
 * 	Table 35-14 Selected MSRs Supported by Intel Xeon Processors E5 Family
 *
 * 	Page 35-2
 * 	Section 35.1
 * 	Table 35-2 IA-32 Architectural MSRs
 *
 * Architectural information should only be used when indicated by tables 35-12 and 35-14.
 *
 *	IA32_TIME_STAMP_COUNTER		See section 17.13.  Restrict to RO.
 *	 Thread		RW (RO)		
 *
 *	IA32_PLATFORM_ID		Bits 52:50 are of interest.  
 *	 Package	RO		Read mask 0x7 << 50 = [0x0, 0x7 << 18]
 *	
 *	PMCn				
 *	 0-3 Thread	RW		No restricted bits.
 *	 4-7 Core	RW		
 *
 * 	MPERF/APERF			Restrict to RO.  See Section 14.2.		
 * 	 Thread		RW (RO)
 *
 *	PERFEVTSELn	RW		See Section 18.2.2.2 (Architectural Perforamnce Monitoring
 *					Version 3 Facilities).  Bits 63:32 are reserved.  Note that bit
 *                                      17 enables counting Ring 0 events; we may want to restrict this.
 *	
 *	PERF_STATUS	RO		See Section 14.1.1.  Table 35-12 contains a duplicate 
 *	 Package			entry for this MSR.  Interpreting both, bits 0-15 are
 *					the current performance value and 47:32 is the core 
 *					voltage: [37:32] * (float) 1/(2^13).  I have asked Intel
 *					for clarification.
 *	 
 *	PERF_CTL	RW (RMW)	Bits 15:0 are the target performance value and bit 
 *	 Thread				32 controls turbo mode (set high to disable).
 *					Section 14.1.1 states "Applications and performance
 *					tools are not expected to use either IA32_PERF_CTL
 *					or IA32_PERF_STATUS and should treat both as reserved",
 *					but Section 14.3.2.2 states "System software can 
 *					temporarily disengage opportunistic processor performance
 *					operation by setting bit 32 of the IA32_PERF_CTL (0199H),
 *					using a read-modify-write sequence on the MSR."  
 *
 *
 */

/*	    Name		       Address  Low	      High	    Low	   	   High
 *	    					Read	      Read	    Write	   Write
 *	    					Mask	      Mask	    Mask	   Mask
 */
























#define SMSR_ENTRIES \
SMSR_ENTRY( NO_SUCH_SMSR,		{0x000, 0x0,          0x0,          0x0,           0x0          }),\
SMSR_ENTRY( SMSR_TIME_STAMP_COUNTER,	{0x010,	SMSR_READALL, SMSR_READALL, SMSR_RO,       SMSR_NOWRITE }),\
SMSR_ENTRY( SMSR_PLATFORM_ID,		{0x017,	0x0,          (0x7 << 18),  SMSR_RO,       SMSR_NOWRITE }),\
SMSR_ENTRY( SMSR_PMC0,			{0x0C1,	SMSR_READALLd SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_PMC1,			{0x0C2,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_PMC2,			{0x0C3,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_PMC3,			{0x0C4,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_PMC4,			{0x0C5,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_PMC5,			{0x0C6,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_PMC6,			{0x0C7,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_PMC7,			{0x0C8,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_MPERF,			{0x0E7,	SMSR_READALL, SMSR_NOREAD,  SMSR_NOWRITE,  SMSR_NOWRITE }),\
SMSR_ENTRY( SMSR_APERF,			{0x0E8,	SMSR_READALL, SMSR_READALL, SMSR_NOWRITE,  SMSR_NOWRITE }),\
SMSR_ENTRY( SMSR_PERFEVTSEL0,		{0x186,	SMSR_READALL, SMSR_NOREAD,  SMSR_WRITEALL, SMSR_NOWRITE }),\
SMSR_ENTRY( SMSR_PERFEVTSEL1,		{0x187,	SMSR_READALL, SMSR_NOREAD,  SMSR_WRITEALL, SMSR_NOWRITE }),\
SMSR_ENTRY( SMSR_PERFEVTSEL2,		{0x188,	SMSR_READALL, SMSR_NOREAD,  SMSR_WRITEALL, SMSR_NOWRITE }),\
SMSR_ENTRY( SMSR_PERFEVTSEL3,		{0x189,	SMSR_READALL, SMSR_NOREAD,  SMSR_WRITEALL, SMSR_NOWRITE }),\
SMSR_ENTRY( SMSR_PERFEVTSEL4,		{0x18A,	SMSR_READALL, SMSR_NOREAD,  SMSR_WRITEALL, SMSR_NOWRITE }),\
SMSR_ENTRY( SMSR_PERFEVTSEL5,		{0x18B,	SMSR_READALL, SMSR_NOREAD,  SMSR_WRITEALL, SMSR_NOWRITE }),\
SMSR_ENTRY( SMSR_PERFEVTSEL6,		{0x18C,	SMSR_READALL, SMSR_NOREAD,  SMSR_WRITEALL, SMSR_NOWRITE }),\
SMSR_ENTRY( SMSR_PERFEVTSEL7,		{0x18D,	SMSR_READALL, SMSR_NOREAD,  SMSR_WRITEALL, SMSR_NOWRITE }),\
SMSR_ENTRY( SMSR_PERF_STATUS,		{0x198,	SMSR_READALL, SMSR_READALL, SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_PERF_CTL,		{0x199,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_CLOCK_MODULATION,	{0x19A,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_THERM_STATUS,		{0x19C,	SMSR_READALL, SMSR_READALL, SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_MISC_ENABLE,		{0x1A0,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_OFFCORE_RSP_0,		{0x1A6,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_OFFCORE_RSP_1,		{0x1A7,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_ENERGY_PERF_BIAS,	{0x1B0,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_PACKAGE_THERM_STATUS,	{0x1B1,	SMSR_READALL, SMSR_READALL, SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_FIXED_CTR0,		{0x309,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_FIXED_CTR1,		{0x30A,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_FIXED_CTR2,		{0x30A,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_PERF_CAPABILITIES,	{0x345,	SMSR_READALL, SMSR_READALL, SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_FIXED_CTR_CTRL,	{0x38D,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_PERF_GLOBAL_STATUS,	{0x38E,	SMSR_READALL, SMSR_READALL, SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_PERF_GLOBAL_CTRL,	{0x38F,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_PERF_GLOBAL_OVF_CTRL,	{0x390,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_PEBS_ENABLE,		{0x3F1,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_PEBS_LD_LAT,		{0x3F6,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_RAPL_POWER_UNIT,	{0x606,	SMSR_READALL, SMSR_READALL, SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_PKG_POWER_LIMIT,	{0x610,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_PKG_ENERGY_STATUS,	{0x611,	SMSR_READALL, SMSR_READALL, SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_PKG_POWER_INFO,	{0x612,	SMSR_READALL, SMSR_READALL, SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_PP0_POWER_LIMIT,	{0x638,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_PP0_ENERGY_STATUS,	{0x639,	SMSR_READALL, SMSR_READALL, SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_MSR_PKG_PERF_STATUS,	{0x613,	SMSR_READALL, SMSR_READALL, SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_DRAM_POWER_LIMIT,	{0x618,	SMSR_READALL, SMSR_READALL, SMSR_WRITEALL, SMSR_WRITEALL}),\
SMSR_ENTRY( SMSR_DRAM_ENERGY_STATUS,	{0x619,	SMSR_READALL, SMSR_READALL, SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_DRAM_PERF_STATUS,	{0x61B,	SMSR_READALL, SMSR_READALL, SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_DRAM_POWER_INFO,	{0x61C,	SMSR_READALL, SMSR_READALL, SMSR_RO,       SMSR_RO      }),\
SMSR_ENTRY( SMSR_LAST_ENTRY, 		{0x000, 0x0,          0x0,          0x0,           0x0          })

#endif //_USE_ARCH_062D

#endif /* MSR_SUPPLEMENTAL_H */
