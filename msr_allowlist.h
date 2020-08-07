/*
 * Copyright (c) 2011-2017 by Lawrence Livermore National Security, LLC.
 * LLNL-CODE-645430
 *
 * Produced at Lawrence Livermore National Laboratory.
 * Written by  Marty McFadden, mcfadden8@llnl.gov
 *             Kathleen Shoga, shoga1@llnl.gov
 *             Barry Rountree rountree@llnl.gov
 *
 * All rights reserved.
 *
 * This file is part of msr-safe.
 *
 * msr-safe is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * msr-safe is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with msr-safe. If not, see <http://www.gnu.org/licenses/>.
 *
 * This material is based upon work supported by the U.S. Department of
 * Energy's Lawrence Livermore National Laboratory. Office of Science, under
 * Award number DE-AC52-07NA27344.
 */

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

int msr_allowlist_cleanup(int majordev);

int msr_allowlist_exists(void);

int msr_allowlist_maskexists(loff_t reg);

u64 msr_allowlist_readmask(loff_t reg);

u64 msr_allowlist_writemask(loff_t reg);

#endif
