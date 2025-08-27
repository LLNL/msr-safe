# NAME

msr_safe - kernel module implementing access-control lists for  model-specific registers

# SYNOPSIS

**/dev/cpu/\<cpuid\>/msr_safe**
**/dev/cpu/msr_batch**
**/dev/cpu/msr_allowlist**
**/dev/cpu/msr_version**
**msr-save**

# OVERVIEW
msr_safe provides controlled userspace access to model-specific registers
(MSRs).
It allows system administrators to give
register-level read access and bit-level write access to trusted users in
production environments.  This access is useful where kernel
drivers have not caught up with new processor features, or performance
constraints requires batch access across dozens or hundreds of registers.

# SETUP
Building the kernel module requires linux kernel headers.  Best practice
for production environments requires creation of *msr-user* and *msr-admin*
groups.  Members of the former can read and write MSRs using either the
per-CPU interface or the batch interface, subject to the restrictions
specified in the allowlist. Members of the latter can also change the
contents of the allowlist.
```
git clone https://github.com/LLNL/msr-safe
cd msr-safe
make
sudo insmod ./msr-safe.ko
sudo chmod g+rw /dev/cpu/*/msr_safe /dev/cpu/msr_*
sudo chgrp msr-user /dev/cpu/*/msr_safe /dev/cpu/msr_batch /dev/cpu/msr_safe_version
sudo chgrp msr-admin /dev/cpu/msr_allowlist
```

msr_safe uses dynamically allocated major device numbers.  These can conflict
with devices that use hard-coded numbers.  To work around this, major device
numbers can be specified during module load.

```
sudo insmod msr-safe.ko \
                [ mdev_msr_safe=<#> ] \
                [ mdev_msr_allowlist=<#> ] \
                [ mdev_msr_batch=<#> ] \
                [ mdev_msr_version=<#> ] 
```

Use **rmmod(8)** to unload msr-safe.

```
sudo rmmod msr-safe
```

# DKMS

Copy or clone the repo into /usr/src as the specific directory `msr-safe-git`

```console
$ sudo git clone https://github.com/LLNL/msr-safe /usr/src/msr-safe-git
```

Than add it to DKMS:

```console
$ sudo dkms add -m msr-safe -v git
```

Try to build and install it:

```console
$ sudo dkms build -m msr-safe -v git
$ sudo dkms install -m msr-safe -v git
```

Remove it with:

```console
$ sudo dkms remove -m msr-safe -v git --all
```

# Udev

Rules to adjust permissions on the device files and restore default allowlist:

```console
$ cat <<EOR | sudo tee /etc/udev/rules.d/99-msr_safe.rules >/dev/null
# Rules to adjust group and permission for all device files.
SUBSYSTEM=="msr_safe",         ACTION=="add", GROUP="msr-user",  MODE="0660"
SUBSYSTEM=="msr_batch",        ACTION=="add", GROUP="msr-user",  MODE="0660"
SUBSYSTEM=="msr_safe_version", ACTION=="add", GROUP="msr-user",  MODE="0440"
SUBSYSTEM=="msr_allowlist",    ACTION=="add", GROUP="msr-admin", MODE="0660"

# Restore default allowlist
SUBSYSTEM=="msr_allowlist", ACTION=="add", RUN+="/bin/sh -c 'cat /etc/msr-safe/default_allowlist >/dev/cpu/msr_allowlist'"
EOR
```

# DESCRIPTION
## /dev/cpu/msr_allowlist  

Contains a list of model specific registers and their
writemasks.  Supports **read(2)**, **write(2)** and **open(2)**.  Any MSR
access using **msr_safe** or **msr_batch** is checked against this list.  An MSR
can be read if its address is present in the list.  An MSR can only be
written if its address is present in the list and there is at least one bit
writable as indicated by the write mask.  For example,
the following entry marks the MSR at address 0x10 (the time stamp counter) as read-only,
as the write mask is 0.

```
0x00000010 0x0000000000000000 # "MSR_TIME_STAMP_COUNTER"
```

This entry allows MSR_PERF_CTL (at address 0x199) to be read, but only the
bottom sixteen bits are writeable.
```
0x00000199 0x000000000000ffff # "MSR_PERF_CTL"
```

It is up to the system administrator to create appropriate per-architecture,
per-user allowlists.  The "safety" of a particular MRS depends on the totality
of the environment.  The msr-safe repo provides sample allowlists that have
been useful in other installations; they may or may not be appropriate for
yours.

To see the existing allowlist:
```
cat /dev/cpu/msr_allowlist
```
The output will look something like:
```
# MSR      Write mask
0x00000010 0x0000000000000000
0x00000017 0x0000000000000000
0x000000C1 0x0000000000000000
...
```
Comments are not preserved.

To install a new allowlist:
```
cat <new_allowlist> > /dev/cpu/msr_allowlist
```

Writing, appending, or modifying a loaded allowlist discards the existing
allowlist.

Parsing a
new allowlist is done in two passes.  If an error occurs during the first pass
the existing allowlist is undisturbed.  If an error occurs during the second
pass the allowlist is reset to be empty.  In practice, the most common
second-phase error is the discovery of a duplicate allowlist entry.  See
**ERRORS** for details.

## /dev/cpu/\<cpuid\>/msr_safe  

Per logical-cpu interface for model-specific registers.  Supports **llseek(2)**,
**read(2)**, **write(2)**, and **open(2)**.  Reads or writes a single MSR at a
time.  To access multiple MSRs and/or MSRs across multiple logical CPUs, use
**/dev/cpu/msr_batch**.

The most common approach is to use **pread(2)** and **pwrite(2)**, as these
combine the seek operation with reading and writing.  Alternatively, the
device supports __SEEK_SET__ and __SEEK_CUR__ parameters to **llseek(2)**, but
not __SEEK_END__.  Both reads and and writes must be exactly 8 bytes.


## /dev/cpu/msr_batch  

Batch interface for MSR access.  Only supports **ioctl(2)**, with the first
parameter being the file descriptor, the second parameter being
__X86_IOC_MSR_BATCH__ (defined in __msr_safe.h__), and the third parameter
being a pointer to a __struct msr_batch_array__.

```
struct msr_batch_array
{
    __u32 numops;             // In: # of operations in operations array
    __u32 version;            // In: MSR_SAFE_VERSION_u32 (see msr_version.h)
    struct msr_batch_op *ops; // In: Array[numops] of operations
};
```

The maximum __numops__ is system-dependent, but 30k operations is not
unheard-of.

Starting in version 2.0.0, the __version__ field will be
compared to the version of the loaded kernel module with a mismatch
resulting in an error.  Earlier versions do not check this field.

Each op is contained in a __struct msr_batch_op__:

```
struct msr_batch_op
{
    __u16 cpu;     // In: CPU to execute {rd/wr}msr instruction
    __u16 isrdmsr; // In: 0=wrmsr, non-zero=rdmsr
    __s32 err;     // Out: set if error occurred with this operation
    __u32 msr;     // In: MSR address
    __u64 msrdata; // In/Out: Data to write or data that was read
    __u64 wmask;   // Out: Write mask applied to wrmsr
};
```

The __cpu__ uses the same numbering found in __/dev/cpu/\<cpuid\>__.  A zero
value for __isrdmsr__ indicates a write operation, any other value indicates a
read operation.  __err__ is populated by the kernel if there is an error on a
particular operation, and will be one of __ENXIO__ (the virtual CPU does not
exist or is offline), __EACCES__ (the requested MSR was not found in the
allowlist), or __EROFS__ (a write operation was attempted on an MSR with a write
mask of 0).

__msr__ is the address of the model-specific register.  __msrdata__ is the
value that will be written to or read from the MSR, respectively.
Finally, the __wmask__ records the writemask for the MSR provided in the
allowlist.

## /dev/cpu/msr_safe_version

Starting with version 1.6, this device contains the loaded version of msr-safe.

# RETURN VALUES

On success, calls to **write(2)** and **read(2)** return the number of bytes
written or read, which in the case of **/dev/cpu/\<cpu\>/msr_safe** will be 8
(as only a single register per call may be written to or read from).
**llseek(2)** returns the new file offset.  **open(2)** returns the new file
descriptor.  **ioctl(2)** returns 0.

On error, All of the following system calls will return -1 and set __errno__ to
the appropriate value.  The errors listed below are specific to msr_safe.  The
man pages for the individual system calls describe additional errors that may
occur.

# ERRORS

## /dev/cpu/msr_allowlist

### **write(2)**

__E2BIG__
**\<count\>** exceeds MAX_WLIST_BSIZE (defined as (128 * 1024) + 1)

__EILSEQ__
Unexpected EOF.

__EINVAL__
Address or writemask caused parsing error.

__EFAULT__
Kernel **copy_from_user()** failed.

__ENOMEM__
Kernel unable to allocate memory to hold the raw or parsed allowlist.

__ENOMSG__
No valid allowlist entries found.

__ENOTUNIQ__
Duplicate allowlist entries found.

__ERANGE__
Address or writemask is too large for an unsigned long long.

### **read(2)**

__E2BIG__
The **read(2)** **\<count\>** parameter was less than 60 bytes.

__EFAULT__
Kernel **copy_from_user()** failed.


### **llseek(2)**

__EINVAL__
The **\<whence\>** parameter was
neither __SEEK_CUR__ nor __SEEK_SET__, e.g., __SEEK_END__.

## /dev/cpu/\<cpuid\>/msr_safe

### **read(2)**

__EACCESS__
The MSR requested is not in the allowlist.

__EBUSY__
Requested virtual CPU is (temporarily?) locked.

__EFAULT__
Kernel **copy_to_user()** failed.

__EIO__
A general protection fault occurred.  See the description for
__EIO__ errors in the **/dev/cpu/msr_batch** section below.

__EINVAL__
Number of bytes requested to read is something other than 8.

__ENXIO__
Requested virtual CPU does not exist or is offline.


### **write(2)**

__EACCESS__
The MSR requested is not in the allowlist.

__EBUSY__
Requested virtual CPU is (temporarily?) locked.

__EFAULT__
Kernel **copy_from_user()** failed.

__EIO__
A general protection fault occurred.  See the description for
__EIO__ errors in the **/dev/cpu/msr_batch** section below.

__EINVAL__
Number of bytes requested to read is something other than 8.

__ENXIO__
Requested virtual CPU does not exist or is offline.

### **open(2)**

__EIO__
Model-specific registers not supported on this virtual CPU.

__ENXIO__
Requested virtual CPU does not exist or is offline.

### /dev/cpu/msr_batch

### **ioctl(2)**

All of the operations in the batch will be executed.  Each operation may result
in an __EIO__, __ENXIO__, __EACCES__, or __EROFS__ error, which will be
recorded in the __msr_batch_op__ struct.  If any operation caused an error, the
first such error becomes the return value for **ioctl(2)**.

__E2BIG__
Kernel unable to allocate memory to hold the array of operations.

__EACCES__
An individual operation requested an MSR that is not present in the allowlist.

__EBADF__
The __msr_batch__ file was not opened for reading.

__EFAULT__
Kernel **copy_from_user()** or **copy_to_user()** failed.

__EINVAL__
Number of requested batch operations is <=0.

__EIO__
A general protection fault occurred.  On Intel processors this
can be caused by a) attempting to access an MSR outside of ring 0, b)
attempting to access a non-existent or reserved MSR address, c) writing 1-bits
to a reserved area of an MSR, d) writing a non-canonical address to MSRs that
take memory addresses, or e) writing to MSR bits that are marked as read-only.

__ENOMEM__
Kernel unable to allocate memory to hold the results of __zalloc_cpumask_var()__.

__ENOTTY__
Invalid ioctl command.  As of this writing the only ioctl command
supported on this device is __X86_IOC_MSR_BATCH__, defined in __msr_safe.h__.

__ENXIO__
An individual operation requested a virtual CPU does not exist or is offline.

__EROFS__
An individual operation requested a write to a read-only MSR.


### **open(2)**

There are no msr_safe-specific error conditions.

# ALLOWLIST CONTRIBUTIONS

The contribs/ folder contains allowlists shared by 
our external collaborators, organized by site-name.

# msr-save

The msrsave utility provides a mechanism for saving and restoring MSR values
based on entries in the allowlist. To restore MSR values, the register must
have an appropriate writemask.

Modification of MSRs that are marked as safe in the allowlist may impact
subsequent users on a shared HPC system. It is important the resource manager
on such a system use the msrsave utility to save and restore MSR values between
allocating compute nodes to users. An example of this has been implemented for
the SLURM resource manager as a SPANK plugin. This plugin can be built with the
"make spank" target and installed with the "make install-spank" target. This
uses the SLURM SPANK infrastructure to make a popen(3) call to the msrsave
command line utility in the job epilogue and prologue.

The version of msrsave (and msr-safe) can be modified by updating the following
compiler flag:

    -DVERSION=\"MAJOR.MINOR.PATCH\"

The msrsave version can be queried with:

    msrsave --version

# Security

## Model-specific registers

The safety of a particular model-specific register depends on the system
environment.  The sample allowlists provided were developed for non-classified
high performance computing systems where only a single non-privileged user at a
time can access a given compute node.  These lists should be re-evaluated for
use in other environments, particularly multi-user environments.

## Filesystems permissions

msr-safe is designed to support multiple classes of users, each of which would
have their own group and allowlist.  **Best practice is to unload and reload the
msr-safe kernel module when changing device ownership or permissions.**  If
this is not done, a lower-privileged user can open **/dev/cpu/msr_batch** and
retain the file descriptor until the permissions (and allowlist) are changed
to allow higher-privileged users to run and the allowlist remains readable by
the less-privileged user, the less-privileged user can continue using their
original file descriptor with the higher-privileged allowlist.

# FAQ

## Can I append or modify an allowlist in place?
No. Each **write(2)** call discards the previous allowlist.

## What happens if an allowlist is changed during an **ioctl(2)** call?
The kernel records all of the relevant writemasks in the __struct
msr_batch_op__ prior to executing the ops.  If the allowlist is changed
during a call, the new allowlist will be applied to subsequent calls.

## How many operations can fit into one batch?
Determining the formula to provide an upper bound is almost certainly
more trouble than it's worth, but we have easily gotten 30k entries
in a single batch on production machines.

## What happens if a CPU is taken offline or brought back online?
We haven't had a good reason to wire up hotplugging.  If the collection
of online CPUs changes, it's best to unload and reload the msr-safe kernel
module.

## What happens if a CPU is taken offline and a user still has an open file descriptor for that device?
The kernel checks to see if a CPU is online.  Attempts to access MSRs using
that file descriptor should generate and error.

## Can the batch API be extended to do other operations such as polling?
It can and it has.  If you need this functionality please let us know.
The code is brittle enough that we don't use it in production, but we
are happy to share.



# EXAMPLE CODE
```
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
```

# Release

msr-safe is released under the GPL v2.0 license. For more details, please
see the [LICENSE](https://github.com/LLNL/msr-safe/blob/main/LICENSE) and
[NOTICE](https://github.com/LLNL/msr-safe/blob/master/NOTICE) files.

SPDX-License-Identifier: GPL-2.0-only

`LLNL-CODE-807679`

License and LLNL release number have been corrected to match internal records.
