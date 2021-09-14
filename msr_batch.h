// Copyright 2011-2021 Lawrence Livermore National Security, LLC and other
// msr-safe Project Developers. See the top-level COPYRIGHT file for
// details.
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef MSR_BATCH_HEADER_INCLUDE
#define MSR_BATCH_HEADER_INCLUDE

void msrbatch_cleanup(int majordev);

int msrbatch_init(int *majordev);

#endif
