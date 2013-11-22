#ifndef MSR_SUPPLEMENTAL_H
#define MSR_SUPPLEMENTAL_H

#define MSR_RAPL_DOMAIN_BASE(domain) MSR_RAPL_BASE_ ## domain

#define MSR_RAPL_POWER_LIMIT(domain) MSR_RAPL_DOMAIN_BASE(domain)
#define MSR_RAPL_ENERGY_STATUS(domain) (MSR_RAPL_DOMAIN_BASE(domain) + 1)
#define MSR_RAPL_POLICY(domain) (MSR_RAPL_DOMAIN_BASE(domain) + 2)
#define MSR_RAPL_PERF_STATUS(domain) (MSR_RAPL_DOMAIN_BASE(domain) + 3)
#define MSR_RAPL_POWER_INFO(domain) (MSR_RAPL_DOMAIN_BASE(domain) + 4)

#define MSR_RAPL_BASE_PKG	0x610
#define MSR_RAPL_BASE_DRAM	0x618
#define MSR_RAPL_BASE_PP0	0x638
#define MSR_RAPL_BASE_PP1	0x640

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
#endif

#endif /* MSR_SUPPLEMENTAL_H */
