// Copyright 2011-2021 Lawrence Livermore National Security, LLC and other
// msr-safe Project Developers. See the top-level COPYRIGHT file for
// details.
//
// SPDX-License-Identifier: GPL-2.0-only

/*
 * Internal declarations for x86 MSR version implementation functions.
 *
 * Thank you to everyone who has contributed and helped with this project:
 *
 * Kathleen Shoga, Peter Bailey, Trent D'Hooge, Jim Foraker, David Lowenthal
 * Tapasya Patki, Barry Rountree, Kendrick Shaw, Marty McFadden
 */

#ifndef _ARCH_X68_KERNEL_MSR_VERSION_HEADER_INCLUDE
#define _ARCH_X68_KERNEL_MSR_VERSION_HEADER_INCLUDE

#include <linux/types.h>

int msr_version_init(int *majordev);

void msr_version_cleanup(int majordev);

#endif
