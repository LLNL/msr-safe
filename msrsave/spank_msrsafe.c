/*
 * Copyright (c) 2018, Intel Corporation
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

/*
 * The msr-safe Linux kernel module enables user access to read and
 * write capabilities for a restricted set of allowed Model
 * Specific Registers (MSRs) on x86 platforms.  The purpose of this
 * slurm plugin is to ensure that MSRs modified within a user's slurm
 * job allocation are reset to their original state before the compute
 * node is returned to the pool available to other users of the
 * system.  The msr-safe kernel module is targeting HPC systems that
 * enforce single user occupancy per compute node, and is not
 * appropriate for systems where compute nodes are shared between
 * users.  The modifications that one user makes to allowed
 * registers may impact subsequent users of the processor if not
 * restored.
 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>

#include "slurm/spank.h"

#include "msrsave.h"

#define SLURM_SPANK_MSRSAFE_BUFFER_SIZE 1024

SPANK_PLUGIN(msr-safe, 1);

int slurm_spank_init(spank_t spank_ctx, int argc, char **argv);
int slurm_spank_slurmd_init(spank_t spank_ctx, int argc, char **argv);
int slurm_spank_job_prolog(spank_t spank_ctx, int argc, char **argv);
int slurm_spank_job_epilog(spank_t spank_ctx, int argc, char **argv);

static int slurm_spank_msrsafe_read_log(FILE *log_fid);

#ifdef SLURM_SPANK_MSRSAVE_TEST
/* If test is defined then print to standard output rather
   than slurm log. */

#include <stdio.h>
#define slurm_info printf

int main(int argc, char **argv)
{
    spank_t spank_ctx;
    printf("SAVE SCRIPT:\n");
    slurm_spank_job_prolog(spank_ctx, 0, NULL);
    printf("\n\nRESTORE SCRIPT:\n");
    slurm_spank_job_epilog(spank_ctx, 0, NULL);
    printf("\n\n");
    return 0;
}
/* END TEST PROGRAM */

#endif

static int slurm_spank_msrsafe_read_log(FILE *log_fid)
{
    char buffer[SLURM_SPANK_MSRSAFE_BUFFER_SIZE];
    size_t buffer_pos = 0;
    int err = 0;
    int character = 0;

    if (log_fid == NULL) {
        err = 1;
    }
    else {
        do {
            character = fgetc(log_fid);
            if (character == '\n' ||
                character == EOF ||
                buffer_pos == SLURM_SPANK_MSRSAFE_BUFFER_SIZE - 1) {
                buffer[buffer_pos] = '\0';
                slurm_info("%s", buffer);
                buffer_pos = 0;
            }
            else {
                buffer[buffer_pos] = character;
                ++buffer_pos;
            }
        } while (character != EOF);
    }
    return err;
}

#ifndef SLURM_SPANK_MSRSAVE_FILE_PREFIX
#define SLURM_SPANK_MSRSAVE_FILE_PREFIX "/var/run/slurm-msrsave"
#endif

int slurm_spank_init(spank_t spank_ctx, int argc, char **argv)
{
    slurm_info("Loaded msrsave restore plugin.");
    return 0;
}

int slurm_spank_slurmd_init(spank_t spank_ctx, int argc, char **argv)
{
    slurm_info("Loaded msrsave restore plugin.");
}

int slurm_spank_job_prolog(spank_t spank_ctx, int argc, char **argv)
{
    slurm_info("Running msr-safe plugin to save register values.");

    int err = 0;
    FILE *out_log = NULL;
    char out_log_name[NAME_MAX * 2];
    char msrsave_file[NAME_MAX * 2];
    const char *allowlist_path = "/dev/cpu/msr_allowlist";
    const char *msr_path = "/dev/cpu/%d/msr_safe";
    int num_cpu = sysconf(_SC_NPROCESSORS_ONLN);
    char hostname[NAME_MAX];
    hostname[NAME_MAX - 1] = '\0';
    err = gethostname(hostname, NAME_MAX - 1);
    if (err) {
        slurm_info("gethostname failed.");
    }
    if (!err) {
        snprintf(out_log_name, NAME_MAX * 2, "/tmp/slurm-msrsave-outlog-%s.XXXXXXXXXX", hostname);
	err = mkstemp(out_log_name);
	if (err) {
	    slurm_info("failed to create msrsave output log");
	}
    }
    if (!err) {
        out_log = fopen(out_log_name, "w+");
	if (out_log == NULL) {
	    slurm_info("failed to open %s for writing", out_log_name);
	}
    }
    if (!err) {
        snprintf(msrsave_file, NAME_MAX * 2, "%s-%s", SLURM_SPANK_MSRSAVE_FILE_PREFIX, hostname);
        err = msr_save(msrsave_file, allowlist_path, msr_path, num_cpu, out_log, out_log);
        if (err) {
            slurm_info("msr_save failed:");
	}
        rewind(out_log);
        slurm_spank_msrsafe_read_log(out_log);
    }
    if (!err) {
        slurm_info("Completed msr-safe plugin to save register values.");
    }
    if (out_log) {
        fclose(out_log);
        unlink(out_log_name);
    }
    return err;
}

int slurm_spank_job_epilog(spank_t spank_ctx, int argc, char **argv)
{
    slurm_info("Running msr-safe plugin to restore register values.");
    int err = 0;
    FILE *out_log = NULL;
    char out_log_name[NAME_MAX * 2];
    char msrsave_file[NAME_MAX * 2];
    const char *allowlist_path = "/dev/cpu/msr_allowlist";
    const char *msr_path = "/dev/cpu/%d/msr_safe";
    int num_cpu = sysconf(_SC_NPROCESSORS_ONLN);
    char hostname[NAME_MAX];
    hostname[NAME_MAX - 1] = '\0';
    err = gethostname(hostname, NAME_MAX - 1);
    if (err) {
        slurm_info("gethostname failed.");
    }
    if (!err) {
        snprintf(out_log_name, NAME_MAX * 2, "/tmp/slurm-msrsave-outlog-%s.XXXXXXXXXX", hostname);
	err = mkstemp(out_log_name);
	if (err) {
	    slurm_info("failed to create msrsave output log");
	}
    }
    if (!err) {
        out_log = fopen(out_log_name, "w+");
	if (out_log == NULL) {
	    slurm_info("failed to open %s for writing", out_log_name);
	}
    }
    if (!err) {
        snprintf(msrsave_file, NAME_MAX * 2, "%s-%s", SLURM_SPANK_MSRSAVE_FILE_PREFIX, hostname);
        err = msr_restore(msrsave_file, allowlist_path, msr_path, num_cpu, out_log, out_log);
        if (err) {
            slurm_info("msr_restore failed:");
	}
        rewind(out_log);
        slurm_spank_msrsafe_read_log(out_log);
    }
    if (!err) {
        slurm_info("Completed msr-safe plugin to restore register values.");
    }
    if (out_log) {
        fclose(out_log);
        unlink(out_log_name);
    }
    return err;
}

#undef SLURM_SPANK_MSRSAVE_FILE_PREFIX
