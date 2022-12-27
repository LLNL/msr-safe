/* This example assumes the user has the following permissions:
 *
 * write        /dev/cpu/msr_allowlist
 * read/write   /dev/cpu/<cpu_number>/msr_safe
 * read         /dev/cpu/msr_batch
 *
 * Typically, only the administrator will have write permissions
 * on the allowlist.
 *
 * Production code should have more robust error handling than
 * what is shown here.
 *
 * This example should be able to run successfully on an x86
 * processor from the past ten years or so.
 *
 */


#include <stdio.h>      // printf(3)
#include <assert.h>     // assert(3)
#include <fcntl.h>      // open(2)
#include <unistd.h>     // write(2), pwrite(2), pread(2)
#include <string.h>     // strlen(3), memset(3)
#include <stdint.h>     // uint8_t
#include <inttypes.h>   // PRIu8
#include <stdlib.h>     // exit(3)
#include <sys/ioctl.h>  // ioctl(2)

#include "../msr_safe.h"   // batch data structs

#define MSR_MPERF 0xE7

char const *const allowlist = "0xE7 0xFFFFFFFFFFFFFFFF\n";  // MPERF

static uint8_t const nCPUs = 32;

void set_allowlist()
{
    int fd = open("/dev/cpu/msr_allowlist", O_WRONLY);
    assert(-1 != fd);
    ssize_t nbytes = write(fd, allowlist, strlen(allowlist));
    assert(strlen(allowlist) == nbytes);
    close(fd);
}

void measure_serial_latency()
{
    int fd[nCPUs], rc;
    char filename[255];
    uint64_t data[nCPUs];
    memset(data, 0, sizeof(uint64_t)*nCPUs);

    // Open each of the msr_safe devices (one per CPU)
    for (uint8_t i = 0; i < nCPUs; i++)
    {
        rc = snprintf(filename, 254, "/dev/cpu/%"PRIu8"/msr_safe", i);
        assert(-1 != rc);
        fd[i] = open(filename, O_RDWR);
        assert(-1 != fd[i]);
    }
    // Write 0 to each MPERF register
    for (uint8_t i = 0; i < nCPUs; i++)
    {
        rc = pwrite(fd[i], &data[i], sizeof(uint64_t), MSR_MPERF);
        assert(8 == rc);
    }

    // Read each MPERF register
    for (uint8_t i = 0; i < nCPUs; i++)
    {
        pread(fd[i], &data[i], sizeof(uint64_t), MSR_MPERF);
        assert(8 == rc);
    }

    // Show results
    printf("Serial cycles from first write to last read:"
           "%"PRIu64" (on %"PRIu8" CPUs)\n",
           data[nCPUs - 1], nCPUs);
}

void measure_batch_latency()
{
    struct msr_batch_array rbatch, wbatch;
    struct msr_batch_op r_ops[nCPUs], w_ops[nCPUs];
    int fd, rc;

    fd = open("/dev/cpu/msr_batch", O_RDONLY);
    assert(-1 != fd);

    for (uint8_t i = 0; i < nCPUs; i++)
    {
        r_ops[i].cpu = w_ops[i].cpu = i;
        r_ops[i].isrdmsr = 1;
        w_ops[i].isrdmsr = 0;
        r_ops[i].msr = w_ops[i].msr = MSR_MPERF;
        w_ops[i].msrdata = 0;
    }
    rbatch.numops = wbatch.numops = nCPUs;
    rbatch.ops = r_ops;
    wbatch.ops = w_ops;

    rc = ioctl(fd, X86_IOC_MSR_BATCH, &wbatch);
    assert(-1 != rc);
    rc = ioctl(fd, X86_IOC_MSR_BATCH, &rbatch);
    assert(-1 != rc);

    printf("Batch cycles from first write to last read:"
           "%llu (on %"PRIu8" CPUs)\n",
           r_ops[nCPUs - 1].msrdata, nCPUs);
}

int main()
{
    set_allowlist();
    measure_serial_latency();
    measure_batch_latency();
    return 0;
}
