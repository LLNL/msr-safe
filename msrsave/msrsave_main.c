/*
 * Copyright (c) 2016, Intel Corporation
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "msrsave.h"
#include "../msr_version.h"

int main(int argc, char **argv)
{
    const char *usage = "NAME\n"
                        "       msrsave - msr-safe save and restore application\n"
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
                        "       The  msrsave  application  is  used  to save to a file the  state of all write\n"
                        "       accessible MSR values defined in the current MSR allowlist or restore the MSR\n"
                        "       values from a saved state file. The msrsave application  also respects the\n"
                        "       standard --version  and  --help  options  for  printing  the msr-safe pack-\n"
                        "       age version or a brief message about usage.\n"
                        "\n"
                        "OPTIONS\n"
                        "       --help\n"
                        "              Print brief summary of the command line usage information, then exit.\n"
                        "\n"
                        "       --version\n"
                        "              Print version of msr-safe package to standard output, then exit.\n"
                        "\n"
                        "       -r\n"
                        "              Restore the MSR values that are recorded in an existing MSR saved state\n"
                        "              file.\n"
                        "\n"
                        "COPYRIGHT\n"
                        "       Copyright (C) 2016, Intel Corporation. All rights reserved.\n"
                        "\n"
                        "\n";

    if (argc > 1 && strncmp(argv[1], "--version", strlen("--version") + 1) == 0)
    {
        printf("%s\n", MSR_SAFE_VERSION_STR);
        printf("\nCopyright (C) 2016, Intel Corporation. All rights reserved.\n\n");
        return 0;
    }
    if (argc > 1 && (strncmp(argv[1], "--help", strlen("--help") + 1) == 0 || strncmp(argv[1], "-h", strlen("-h") + 1) == 0))
    {
        printf(usage, argv[0]);
        return 0;
    }

    int err = 0;
    int do_restore = 0;
    int opt = 0;

    while (!err && (opt = getopt(argc, argv, "r")) != -1)
    {
        switch (opt)
        {
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

    if (!err && optind == argc)
    {
        fprintf(stderr, "Error: No file name specified.\n\n");
        fprintf(stderr, usage, argv[0]);
        err = EINVAL;
    }

    if (!err)
    {
        const char *file_name = argv[optind];
        const char *msr_path = "/dev/cpu/%d/msr_safe";
        const char *msr_allowlist_path = "/dev/cpu/msr_allowlist";
        int num_cpu = sysconf(_SC_NPROCESSORS_ONLN);
        if (do_restore)
        {
            err = msr_restore(file_name, msr_allowlist_path, msr_path, num_cpu, stdout, stderr);
        }
        else
        {
            err = msr_save(file_name, msr_allowlist_path, msr_path, num_cpu, stdout, stderr);
        }
    }

    return err;
}
