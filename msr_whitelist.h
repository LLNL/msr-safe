// Copyright 2011-2020 Lawrence Livermore National Security, LLC and other
// msr-safe Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: LGPL-2.1-only

/*
 * Internal declarations for x86 MSR whitelist implementation functions.
 *
 * Thank you to everyone who has contributed and helped with this project:
 *
 * Kathleen Shoga, Peter Bailey, Trent D'Hooge, Jim Foraker, David Lowenthal
 * Tapasya Patki, Barry Rountree, Kendrick Shaw, Marty McFadden
 */

#ifndef _ARCH_X68_KERNEL_MSR_WHITELIST_HEADER_INCLUDE
#define _ARCH_X68_KERNEL_MSR_WHITELIST_HEADER_INCLUDE

#include <linux/types.h>

int msr_whitelist_init(int *majordev);

void msr_whitelist_cleanup(int majordev);

int msr_whitelist_maskexists(loff_t reg);

u64 msr_whitelist_writemask(loff_t reg);

#endif
