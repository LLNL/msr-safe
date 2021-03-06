MSR-SAFE
========

[![Build Status](https://travis-ci.com/LLNL/msr-safe.svg?branch=main)](https://travis-ci.com/LLNL/msr-safe)

The msr-safe.ko module is comprised of the following source files:

    Makefile
    msr_entry.c         Original MSR driver with added calls to batch and
                        allowlist implementations.
    msr_batch.[ch]      MSR batching implementation
    msr_allowlist.[ch]  MSR allowlist implementation
    allowlists          Sample text allowlists that may be input to msr_safe

Kernel Build & Load
-------------------

Building and loading the msr-safe.ko module can be done with the commands
below. When no command line arguments are specified, the kernel will
dynamically assign major numbers to each device. A successful load of the
msr-safe kernel module will have `msr_batch` and `msr_allowlist` in
`/dev/cpu`, and will have an `msr_safe` present under each CPU directory in
`/dev/cpu/*`.

    $ git clone https://github.com/LLNL/msr-safe
    $ cd msr-safe
    $ make
    $ insmod msr-safe.ko

Kernel Load with Command Line Arguments
---------------------------------------

Alternatively, this module can be loaded with command line arguments. The
arguments specify the major device number you want to associate with a
particular device. When loading the kernel, you can specify 1 or all 3 of the
msr devices.

    $ insmod msr-safe.ko mdev_msr_safe=<#> \
                         mdev_msr_allowlist=<#> \
                         mdev_msr_batch=<#>

Configuration Notes After Install
---------------------------------

Setup permissions and group ownership for `/dev/cpu/msr_batch`,
`/dev/cpu/msr_allowlist`, and `/dev/cpu/*/msr_safe` as you like since the
allowlist will protect you from harm.

Sample allowlists for specific architectures are provided in
`allowlists/` directory. These are meant to be a starting point, and should
be used with caution. Each site may add to, remove from, or modify the write
masks in the allowlist depending on specific needs. See the Troubleshooting
section below for more information.

To configure the allowlist:

    cat allowlist/al_file > /dev/cpu/msr_allowlist

Where `al_file` can be determined as follows:

    printf 'al_%.2x%x\n' $(lscpu | grep "CPU family:" | awk -F: '{print $2}') $(lscpu | grep "Model:" | awk -F: '{print $2}')

To confirm successful allowlist configured:

    cat /dev/cpu/msr_allowlist

To enumerate the current allowlist (i.e., implies allowlist was loaded
successfully):

    cat < /dev/cpu/msr_allowlist

To remove allowlist (as root):

    echo > /dev/cpu/msr_allowlist

msrsave
-------

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

Troubleshooting
---------------

If you encounter errors attempting to read a particular MSR, it may be for
several reasons:

If you encounter a "Permission denied" error, likely the MSR was not exposed in
the current allowlist.

It is possible that the MSR you are attempting to read is not supported by your
CPU. You will likely see this if attempting to use the msrsave utility.  In
that case, you should see an error message like the following:

    Warning: Failed to read msr value ...

These messages are benign and should not interfere with msrsave's ability to
save and restore MSR values that are currently supported. If it is desired to
remove the warning messages, remove the corresponding entry from the allowlist.

A note on `CAP_SYS_RAWIO`
-------------------------

msr-safe relies on the Linux filesystem permissions to restrict access to the
allowlist, the batch device and the individual msr devices. The stock kernel
msr module does not have the allowlist mechanism, of course, but does
add another layer of protection: users/binaries accessing /dev/cpu/X/msr must
have the `CAP_SYS_RAWIO` capability. For a general explanation of the Linux
capability model see `man -s7 capabilities. For discussion of why this was
added see the Linux Weekly News article [The Trouble with
CAP_SYS_RAWIO](https://lwn.net/Articles/542327/).

If you are transitioning from using the stock Linux msr kernel module and
relying on `CAP_SYS_RAWIO`, please be aware that msr_safe does not perform
capability checks. Any user with sufficient file permissions can access the
device drivers.

Release
-------

msr-safe is released under the GPL v2.0 license. For more details, please
see the [LICENSE](https://github.com/LLNL/msr-safe/blob/main/LICENSE) and
[NOTICE](https://github.com/LLNL/msr-safe/blob/master/NOTICE) files.

SPDX-License-Identifier: GPL-2.0-only

`LLNL-CODE-807679`

License and LLNL release number have been corrected to match internal records.
