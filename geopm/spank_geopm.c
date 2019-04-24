/*
 * Copyright (c) 2019, Intel Corporation
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

#include "limits.h"
#include "math.h"

#include "slurm/spank.h"

#include "geopm_agent.h"
#include "geopm_pio.h"
#include "geopm_error.h"

SPANK_PLUGIN(geopm, 1);

int slurm_spank_init(spank_t spank_ctx, int argc, char **argv);
int slurm_spank_slurmd_init(spank_t spank_ctx, int argc, char **argv);
int slurm_spank_job_prolog(spank_t spank_ctx, int argc, char **argv);
int slurm_spank_job_epilog(spank_t spank_ctx, int argc, char **argv);

int slurm_spank_init(spank_t spank_ctx, int argc, char **argv)
{
    slurm_info("Loaded geopm plugin.");
    return 0;
}

int slurm_spank_slurmd_init(spank_t spank_ctx, int argc, char **argv)
{
    slurm_info("Loaded geopm plugin in slurmd.");
    return 0;
}

int slurm_spank_job_prolog(spank_t spank_ctx, int argc, char **argv)
{
    slurm_info("Running geopm plugin prolog.");

    int err = geopm_pio_save_control();
    if (!err) {
        err = geopm_agent_enforce_policy();
    }
    if (err) {
        char err_msg[NAME_MAX];
        geopm_error_message(err, err_msg, NAME_MAX);
        slurm_info("geopm plugin prolog failure:");
        slurm_info(err_msg);
    }
    else {
        slurm_info("Completed geopm plugin prolog.");
    }
    return err;
}

int slurm_spank_job_epilog(spank_t spank_ctx, int argc, char **argv)
{
    slurm_info("Running geopm plugin epilog.");
    int err = geopm_pio_restore_control();
    if (err) {
        char err_msg[NAME_MAX];
        geopm_error_message(err, err_msg, NAME_MAX);
        slurm_info("geopm plugin epilog failure:");
        slurm_info(err_msg);
    }
    else {
        slurm_info("Completed geopm plugin epilog.");
    }
    return err;
}
