Configuration Space Register Safe Access Driver (CSR_SAFE)
==========================================================

### **Please note this project is in early development and is not fully tested.**

The `csr-safe.ko module` is comprised of the following source files:

    Makefile
    csr_safe.h 
    csr_entry.c         Main driver allowing userspace access using an ioctl
    csr_whitelist.[ch]  Puts the 'safe' in csr-safe

Version
-------
Version 0.0 Alpha


Kernel Build & Install
----------------------

Installation is as easy as:

    $ cd csr-safe/
    $ make
    $ sudo insmod csr-safe.ko

Configuration Notes Following Install
-------------------------------------

After that just load the appropriate CSR whitelist:

    $ cat whitelists/csr_wl_file > /dev/cpu/csr_whitelist

It is up to the administrator to set the permissions of `csr_safe` and
`csr_whitelist`. Keep in mind that this driver can potentially be used to access
`/dev/mem` and CSRs can be dangerous if modified incorrectly. We suggest that
only administrators have access to `/dev/cpu/csr_whitelist`.
