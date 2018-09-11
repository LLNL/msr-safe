/*
 * Copyright (c) 2018, Intel Corporation
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

/*
 * The msr-safe Linux kernel module enables user access to read and
 * write capabilities for a restricted set of whitelisted Model
 * Specific Registers (MSRs) on x86 platforms.  The purpose of this
 * slurm plugin is to ensure that MSRs modified within a user’s slurm
 * job allocation are reset to their original state before the compute
 * node is returned to the pool available to other users of the
 * system.  The msr-safe kernel module is targeting HPC systems that
 * enforce single user occupancy per compute node, and is not
 * appropriate for systems where compute nodes are shared between
 * users.  The modifications that one user makes to whitelisted
 * registers may impact subsequent users of the processor if not
 * restored.
 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#include "slurm/spank.h"

#define SLURM_SPANK_MSRSAFE_BUFFER_SIZE 1024

SPANK_PLUGIN(msr-safe, 1);

int slurm_spank_job_prolog(spank_t spank_ctx, int argc, char **argv);
int slurm_spank_job_epilog(spank_t spank_ctx, int argc, char **argv);

static int slurm_spank_msrsafe_system(const char *cmd);
static void slurm_spank_msrsafe_popen_complete(int signum);
static int slurm_spank_msrsafe_popen(const char *cmd, FILE **fid);

static volatile unsigned g_is_popen_complete = 0;
static struct sigaction g_popen_complete_signal_action;

static void slurm_spank_msrsafe_popen_complete(int signum)
{
    if (signum == SIGCHLD) {
        g_is_popen_complete = 1;
    }
}

static int slurm_spank_msrsafe_popen(const char *cmd, FILE **fid)
{
    int err = 0;
    *fid = NULL;

    struct sigaction save_action;
    g_popen_complete_signal_action.sa_handler = slurm_spank_msrsafe_popen_complete;
    sigemptyset(&g_popen_complete_signal_action.sa_mask);
    g_popen_complete_signal_action.sa_flags = 0;
    err = sigaction(SIGCHLD, &g_popen_complete_signal_action, &save_action);
    if (!err) {
        *fid = popen(cmd, "r");
        while (*fid && !g_is_popen_complete) {

        }
        g_is_popen_complete = 0;
        sigaction(SIGCHLD, &save_action, NULL);
    }
    if (!err && *fid == NULL) {
        err = errno ? errno : -1;
    }
    return err;
}

#ifndef SLURM_SPANK_MSRSAVE_TEST
/* Do not compile slurm_spank_msrsafe_system if testing since requires
   linking to slurm library for slurm_error() API and the function is
   not executed by the test program. */

static int slurm_spank_msrsafe_system(const char *cmd)
{
    const size_t buffer_size = SLURM_SPANK_MSRSAFE_BUFFER_SIZE - 1;
    char buffer[SLURM_SPANK_MSRSAFE_BUFFER_SIZE];
    FILE *fid = NULL;
    int err = slurm_spank_msrsafe_popen(cmd, &fid);
    if (!err) {
        size_t num_read = 0;
        do {
            num_read = fread(buffer, sizeof(*buffer), buffer_size, fid);
            buffer[num_read] = '\0';
            if (num_read) {
                slurm_info("%s", buffer);
            }
        } while (num_read == buffer_size);
        err = pclose(fid);
    }
    return err;
}

#else /* BEGIN TEST PROGRAM */
/* If test is defined then print the scripts to standard output rather
   than executing them. */

#include <stdio.h>
#define slurm_spank_msrsafe_system printf

int main(int argc, char **argv)
{
    spank_t spank_ctx;
    const char *test_cmd = "ls --version";
    printf("SAVE SCRIPT:\n");
    slurm_spank_job_prolog(spank_ctx, 0, NULL);
    printf("\n\nRESTORE SCRIPT:\n");
    slurm_spank_job_epilog(spank_ctx, 0, NULL);
    printf("\n\n");
    FILE *fid;
    char buffer[4096] = {0};
    int err = slurm_spank_msrsafe_popen(test_cmd, &fid);
    printf("CALLING \"%s\":\n", test_cmd);
    fread(buffer, sizeof(char), 4096, fid);
    printf("%s", buffer);
    return 0;
}

#endif /* END TEST PROGRAM */

#ifndef SLURM_SPANK_MSRSAVE_FILE_PREFIX
#define SLURM_SPANK_MSRSAVE_FILE_PREFIX "/var/run/slurm-msrsave"
#endif

int slurm_spank_job_prolog(spank_t spank_ctx, int argc, char **argv)
{
    const char *save_script = "if [ -e /dev/cpu/msr_whitelist ]; then "
                              "tmp_file=$(mktemp " SLURM_SPANK_MSRSAVE_FILE_PREFIX "-$(hostname -s).XXXXXXXXXX) && "
                              "/usr/sbin/msrsave $tmp_file 2>&1; "
                              "fi";
    return slurm_spank_msrsafe_system(save_script);
}

int slurm_spank_job_epilog(spank_t spank_ctx, int argc, char **argv)
{
    const char *restore_script = "if [ -e /dev/cpu/msr_whitelist ]; then "
                                 "tmp_files=$(ls -t " SLURM_SPANK_MSRSAVE_FILE_PREFIX "-$(hostname -s).*) && "
                                 "tmp_file=$(echo $tmp_files | head -n1) && "
                                 "/usr/sbin/msrsave -r $tmp_file 2>&1 && "
                                 "rm $tmp_file 2>&1; "
                                 "fi";

    return slurm_spank_msrsafe_system(restore_script);
}

#undef SLURM_SPANK_MSRSAVE_FILE_PREFIX
