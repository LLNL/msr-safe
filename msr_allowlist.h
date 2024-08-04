// Copyright 2011-2021 Lawrence Livermore National Security, LLC and other
// msr-safe Project Developers. See the top-level COPYRIGHT file for
// details.
//
// SPDX-License-Identifier: GPL-2.0-only

/*
 * Internal declarations for x86 MSR allowlist implementation functions.
 *
 * Thank you to everyone who has contributed and helped with this project:
 *
 * Kathleen Shoga, Peter Bailey, Trent D'Hooge, Jim Foraker, David Lowenthal
 * Tapasya Patki, Barry Rountree, Kendrick Shaw, Marty McFadden
 */

#ifndef _ARCH_X68_KERNEL_MSR_ALLOWLIST_HEADER_INCLUDE
#define _ARCH_X68_KERNEL_MSR_ALLOWLIST_HEADER_INCLUDE

#include <linux/types.h>

int msr_allowlist_init(int *majordev);

void msr_allowlist_cleanup(int majordev);

int msr_allowlist_exists(void);

int msr_allowlist_maskexists(loff_t reg);

u64 msr_allowlist_readmask(loff_t reg);

u64 msr_allowlist_writemask(loff_t reg);

#endif
