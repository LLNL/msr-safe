// Copyright 2011-2020 Lawrence Livermore National Security, LLC and other
// msr-safe Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: LGPL-2.1-only

#ifndef MSR_BATCH_HEADER_INCLUDE
#define MSR_BATCH_HEADER_INCLUDE

void msr_batch_cleanup(int majordev);

int msr_batch_init(int *majordev);

#endif
