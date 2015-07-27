/*
 * Internal declarations for x86 MSR whitelist implementation functions.
 *
 * Thank you to everyone who has contributed and helped with this project:
 *
 * Kathleen Shoga, Peter Bailey, Trent D'Hooge, Jim Foraker, David Lowenthal
 * Tapasya Patki, Barry Rountree, Kendrick Shaw, Marty McFadden
 */
#ifndef _ARCH_X68_KERNEL_MSR_WHITELIST_H
#define _ARCH_X68_KERNEL_MSR_WHITELIST_H 1

#include <linux/types.h>

int msr_whitelist_init(void);
int msr_whitelist_cleanup(void);
int msr_whitelist_exists(void);
int msr_whitelist_maskexists(loff_t reg);
u64 msr_whitelist_readmask(loff_t reg);
u64 msr_whitelist_writemask(loff_t reg);

#endif /* _ARCH_X68_KERNEL_MSR_WHITELIST_H */
