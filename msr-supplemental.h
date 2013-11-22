#ifndef MSR_SUPPLEMENTAL_H
#define MSR_SUPPLEMENTAL_H

/* TLCC2 machines are based on Sandy Bridge Server processors, family
 * 06 model 2D.  
 *
 * MSRs taken from section 35.8 table 35-12
 */
#ifdef _USE_ARCH_062D

#define SMSR_TIME_STAMP_COUNTER		0x010	//ro
#define SMSR_PLATFORM_ID		0x017	//ro
#define SMSR_FEATURE_CONTROL		0x034	//RW
#define SMSR_PMC0			0x0C1	//RW
#define SMSR_PMC1			0x0C2	//RW
#define SMSR_PMC2			0x0C3	//RW
#define SMSR_PMC3			0x0C4	//RW
#define SMSR_PMC4			0x0C5	//RW
#define SMSR_PMC5			0x0C6	//RW
#define SMSR_PMC6			0x0C7	//RW
#define SMSR_PMC7			0x0C8	//RW
#define SMSR_PLATFORM_INFO		0x0CE	//RW
#define SMSR_PKG_CST_CONFIG_CONTROL	0x0E2	//RW
#define SMSR_MPERF			0x0E7	//ro
#define SMSR_APERF			0x0E8	//ro
#define SMSR_PERFEVTSEL0		0x186	//RW
#define SMSR_PERFEVTSEL1		0x187	//RW
#define SMSR_PERFEVTSEL2		0x188	//RW
#define SMSR_PERFEVTSEL3		0x189	//RW
#define SMSR_PERFEVTSEL4		0x18A	//RW
#define SMSR_PERFEVTSEL5		0x18B	//RW
#define SMSR_PERFEVTSEL6		0x18C	//RW
#define SMSR_PERFEVTSEL7		0x18D	//RW
#define SMSR_PERF_STATUS		0x198	//RW
#define SMSR_PERF_CTL			0x199	//RW
#define SMSR_CLOCK_MODULATION		0x19A	//RW
#define SMSR_THERM_INTERRUPT		0x19B	//RW
#define SMSR_THERM_STATUS		0x19C	//RW
#define SMSR_MISC_ENABLE		0x1A0	//RW
#define SMSR_TEMPERATURE_TARGET		0x1A2	//RW
#define SMSR_OFFCORE_RSP_0		0x1A6	//RW
#define SMSR_OFFCORE_RSP_1		0x1A7	//RW
#define SMSR_MISC_PWR_MGMT		0x1AA	//RW
#define SMSR_ENERGY_PERF_BIAS		0x1B0	//RW
#define SMSR_PACKAGE_THERM_STATUS	0x1B1	//RW
#define SMSR_PACKAGE_THERM_INTERRUPT
#define SMSR_
#define SMSR_
#define SMSR_
#define SMSR_
#define SMSR_
#define SMSR_
#define SMSR_
#endif

#endif /* MSR_SUPPLEMENTAL_H */
