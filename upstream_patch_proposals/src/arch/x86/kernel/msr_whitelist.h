/*
 * Internal declarations for x86 MSR whitelist implementation functions.
 *
 * Copyright (c) 2015, Lawrence Livermore National Security, LLC.
 * Produced at the Lawrence Livermore National Laboratory
 * All rights reserved.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License v.2.
 *
 * Thank you to everyone who has contributed and helped with this project:
 *
 * Kathleen Shoga
 * Peter Bailey
 * Trent D'Hooge
 * Jim Foraker
 * David Lowenthal
 * Tapasya Patki
 * Barry Rountree
 * Marty McFadden
 *
 * Special thanks to Kendrick Shaw at Case Western Reserve University for
 * his initial suggestion to explore MSRs.
 *
 * Latest Updates from: Marty McFadden, mcfadden8@llnl.gov
 */
#ifndef _ARCH_X68_KERNEL_MSR_WHITELIST_H
#define _ARCH_X68_KERNEL_MSR_WHITELIST_H 1

#include <linux/types.h>

int msr_whitelist_init(void);
int msr_whitelist_cleanup(void);
int msr_whitelist_maskexists(loff_t reg);
u64 msr_whitelist_writemask(loff_t reg);

#endif /* _ARCH_X68_KERNEL_MSR_WHITELIST_H */
