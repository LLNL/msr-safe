/*
 * Copyright (c) 2016, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY LOG OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <limits.h>

#include "msrsave.h"

#ifndef VERSION
#define VERSION "0.0.0"
#endif

int main(int argc, char **argv)
{
    const char *usage =
"NAME\n"
"       msrsave - msr-safe save restore application\n"
"\n"
"SYNOPSIS\n"
"       SAVE MSR:\n"
"              msrsave outfile\n"
"\n"
"       RESTORE MSR:\n"
"              msrsave -r infile\n"
"\n"
"       PRINT VERSION OR HELP:\n"
"              msrsave --version | --help\n"
"\n"
"DESCRIPTION\n"
"       The  msrsave  application  is used save to a file the state of all MSR values\n"
"       that the current MSR whitelist enables to be written or restore the MSR  val‐\n"
"       ues  from a saved state file. The msrsave application also respects the stan‐\n"
"       dard --version and --help options for printing the msr-safe  package  version\n"
"       or a brief message about usage.\n"
"\n"
"OPTIONS\n"
"       --help\n"
"              Print brief summary of the command line usage information, then exit.\n"
"\n"
"       --version\n"
"              Print version of msr-safe package to standard output, then exit.\n"
"\n"
"       -r\n"
"              Restore the MSR values that are recorded in an existing MSR save state\n"
"              file.\n"
"\n"
"COPYRIGHT\n"
"       Copyright (C) 2016, Intel Corporation. All rights reserved.\n"
"\n"
"\n";

    if (argc > 1 &&
        strncmp(argv[1], "--version", strlen("--version") + 1) == 0) {
        printf("%s\n", VERSION);
        printf("\nCopyright (C) 2016, Intel Corporation. All rights reserved.\n\n");
        return 0;
    }
    if (argc > 1 && (
            strncmp(argv[1], "--help", strlen("--help") + 1) == 0 ||
            strncmp(argv[1], "-h", strlen("-h") + 1) == 0)) {
        printf(usage, argv[0]);
        return 0;
    }

    int err = 0;
    int do_restore = 0;
    int opt = 0;

    while (!err && (opt = getopt(argc, argv, "r")) != -1) {
        switch (opt) {
            case 'r':
                do_restore = 1;
                break;
            default:
                fprintf(stderr, "Error: Unknown parameter \"%c\"\n\n", opt);
                fprintf(stderr, usage, argv[0]);
                err = EINVAL;
                break;
        }
    }

    if (!err && optind == argc) {
        fprintf(stderr, "Error: No file name specified.\n\n", argv[0]);
        fprintf(stderr, usage, argv[0]);
        err = EINVAL;
    }

    if (!err) {
        const char *file_name = argv[optind];
        const char *msr_path = "/dev/cpu/%d/msr";
        const char *msr_whitelist_path = "/dev/cpu/msr_whitelist";
        int num_cpu = sysconf(_SC_NPROCESSORS_ONLN);
        if (do_restore) {
            err = msr_restore(file_name, msr_whitelist_path, msr_path, num_cpu);
        }
        else {
            err = msr_save(file_name, msr_whitelist_path, msr_path, num_cpu);
        }
    }

    return err;
}

