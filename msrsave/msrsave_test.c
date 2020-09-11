/*
 * Copyright (c) 2016, Intel Corporation
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "msrsave.h"

void msrsave_test_mock_msr(void *buffer, size_t buffer_size, const char *path_format, int num_cpu);
void msrsave_test_check_msr(uint64_t *buffer, size_t num_check, const char *path_format, int num_cpu);

void msrsave_test_mock_msr(void *buffer, size_t buffer_size, const char *path_format, int num_cpu)
{
    /* Create mock msr files for each CPU */
    int i;
    char this_path[NAME_MAX] = {};
    for (i = 0; i < num_cpu; ++i)
    {
        snprintf(this_path, NAME_MAX, path_format, i);
        FILE *fid = fopen(this_path, "w");
        assert(fid != 0);
        fwrite(buffer, 1, buffer_size, fid);
        fclose(fid);
    }
}

void msrsave_test_check_msr(uint64_t *check_val, size_t num_check, const char *path_format, int num_cpu)
{
    /* Check that values in the file match expected */
    int i, j;
    char this_path[NAME_MAX] = {};
    uint64_t *read_val = malloc(num_check * sizeof(uint64_t));
    assert(read_val != NULL);
    for (i = 0; i < num_cpu; ++i)
    {
        snprintf(this_path, NAME_MAX, path_format, i);
        FILE *fid = fopen(this_path, "r");
        assert(fid != 0);
        fread(read_val, sizeof(uint64_t), num_check, fid);
        fclose(fid);
        for (j = 0; j < num_check; ++j)
        {
            assert(check_val[j] == read_val[j]);
        }
    }
    free(read_val);
}

int main(int argc, char **argv)
{
    int err = 0;
    const uint64_t allowlist_off[] = {0x0000000000000000ULL,
                                      0x0000000000000008ULL,
                                      0x0000000000000010ULL,
                                      0x0000000000000018ULL,
                                      0x0000000000000020ULL,
                                      0x0000000000000028ULL,
                                      0x0000000000000030ULL,
                                      0x0000000000000038ULL,
                                      0x0000000000000040ULL,
                                      0x0000000000000048ULL,
                                      0x0000000000000050ULL,
                                      0x0000000000000058ULL,
                                      0x0000000000000060ULL,
                                      0x0000000000000068ULL,
                                      0x0000000000000070ULL,
                                      0x0000000000000078ULL,
                                      0x0000000000000080ULL,
                                      0x0000000000000088ULL,
                                      0x0000000000000090ULL,
                                      0x0000000000000098ULL,
                                      0x1000000000000100ULL  /* negative offset should be skipped */
    };

    const uint64_t allowlist_mask[] = {0x8000000000000000ULL,
                                           0x8000000000000000ULL,
                                           0x8000000000000000ULL,
                                           0x8000000000000000ULL,
                                           0x8000000000000000ULL,
                                           0x8000000000000000ULL,
                                           0x8000000000000000ULL,
                                           0x8000000000000000ULL,
                                           0x8000000000000000ULL,
                                           0x8000000000000000ULL,
                                           0x8000000000000000ULL,
                                           0x8000000000000000ULL,
                                           0x8000000000000000ULL,
                                           0x8000000000000000ULL,
                                           0x8000000000000000ULL,
                                           0x8000000000000000ULL,
                                           0x8000000000000000ULL,
                                           0x8000000000000000ULL,
                                           0x8000000000000000ULL,
                                           0xFFFFFFFFFFFFFFFFULL,
                                           0x8000000000000000ULL};

    enum {NUM_MSR = sizeof(allowlist_off) / sizeof(uint64_t)};
    assert(NUM_MSR == sizeof(allowlist_mask) / sizeof(uint64_t));
    const char *test_save_path = "msrsave_test_store";
    const char *test_allowlist_path = "msrsave_test_allowlist";
    const char *test_msr_path = "msrsave_test_msr.%d";
    const char *allowlist_format = "0x%.8zX 0x%.16zX\n";
    const int num_cpu = 10;
    int i;

    /* Create a mock white list from the data in the constants above. */
    FILE *fid = fopen(test_allowlist_path, "w");
    assert(fid != NULL);
    for (i = 0; i < NUM_MSR; ++i)
    {
        fprintf(fid, allowlist_format, allowlist_off[i], allowlist_mask[i]);
    }
    fclose(fid);

    uint64_t lval = 0x0;
    uint64_t hval = 0xDEADBEEF;
    uint64_t msr_val[NUM_MSR];

    /* Create mock msr data */
    for (i = 0; i < NUM_MSR; ++i)
    {
        lval = i;
        msr_val[i] = lval | (hval << 32);
    }

    msrsave_test_mock_msr(msr_val, sizeof(msr_val), test_msr_path, num_cpu);

    const char *log_file_name = "msrsave_test.log";
    FILE *log_file = fopen(log_file_name, "w");
    /* Save the current state to a file */
    err = msr_save(test_save_path, test_allowlist_path, test_msr_path, num_cpu, log_file, log_file);
    assert(err == 0);

    /* Overwrite the mock msr files with new data */
    /* This modifies both bit 63 and bit 62 from the original value
       however only bit 63 is covered by the write mask and will be restored. */
    hval = 0x1EADBEEF;
    for (i = 0; i < NUM_MSR; ++i)
    {
        lval = NUM_MSR - i;
        msr_val[i] = lval | (hval << 32);
    }

    msrsave_test_mock_msr(msr_val, sizeof(msr_val), test_msr_path, num_cpu);

    /* Restore to the original values */
    err = msr_restore(test_save_path, test_allowlist_path, test_msr_path, num_cpu, log_file, log_file);
    assert(err == 0);

    /* Check that the values that are writable have been restored. */
    /* Check that the values that are not writable have been unaltered. */
    hval = 0x9EADBEEF;
    for (i = 0; i < NUM_MSR - 1; ++i)
    {
        if (allowlist_mask[i] & 0xFFFFFFFF) {
            msr_val[i] = 0xDEADBEEF00000000 | i;
        }
        else {
            lval = NUM_MSR - i;
            msr_val[i] = lval | (hval << 32);
        }
    }

    msrsave_test_check_msr(msr_val, sizeof(msr_val) / sizeof(uint64_t), test_msr_path, num_cpu);

    char this_path[NAME_MAX] = {};
    for (i = 0; i < num_cpu; ++i)
    {
        snprintf(this_path, NAME_MAX, test_msr_path, i);
        unlink(this_path);
    }
    unlink(test_allowlist_path);
    unlink(test_save_path);
    fclose(log_file);
    if (!err) {
        unlink(log_file_name);
    }
    return err;
}
