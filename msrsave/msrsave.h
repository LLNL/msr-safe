/*
 * Copyright (c) 2015, 2016, Intel Corporation
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef MSRSAVE_H_INCLUDE
#define MSRSAVE_H_INCLUDE

#include <stdio.h>

int msr_save(const char *out_path,
             const char *allowlist_path,
             const char *msr_path,
             int num_cpu,
             FILE *output_log,
             FILE *error_log);

int msr_restore(const char *in_path,
                const char *allowlist_path,
                const char *msr_path,
                int num_cpu,
                FILE *output_log,
                FILE *error_log);

#endif
