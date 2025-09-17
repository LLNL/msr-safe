# Copyright 2011-2021 Lawrence Livermore National Security, LLC and other
# msr-safe Project Developers. See the top-level COPYRIGHT file for
# details.
#
# SPDX-License-Identifier: GPL-2.0-only

# Uncomment following line(s) to enable debug output from different
# portions of kernel module.
#
#CFLAGS_msr_allowlist.o := -DDEBUG
#CFLAGS_msr_entry.o := -DDEBUG
#CFLAGS_msr_batch.o := -DDEBUG
#CFLAGS_msr-smp.o := -DDEBUG

KERNELVER ?= $(shell uname -r)

obj-m += msr-safe.o
msr-safe-objs := msr_entry.o msr_allowlist.o msr-smp.o msr_batch.o msr_version.o

all: module msrsave/msrsave
	make -C /lib/modules/$(KERNELVER)/build M=$(PWD) modules

module:
	make -C /lib/modules/$(KERNELVER)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(KERNELVER)/build M=$(PWD) clean
	rm -f msrsave/msrsave.o msrsave/msrsave msrsave/msrsave_test msrsave/spank_msrsafe.o msrsave/libspank_msrsafe.so

check: msrsave/msrsave_test
	msrsave/msrsave_test

msrsave/msrsave.o: msrsave/msrsave.c msrsave/msrsave.h
	$(CC) $(CFLAGS) -fPIC -c msrsave/msrsave.c -o $@

msrsave/msrsave_main.o: msrsave/msrsave_main.c msrsave/msrsave.h
	$(CC) $(CFLAGS) -fPIC -c msrsave/msrsave_main.c -o $@

msrsave/msrsave: msrsave/msrsave_main.o msrsave/msrsave.o
	$(CC) $(CFLAGS) $^ -o $@

msrsave/msrsave_test.o: msrsave/msrsave_test.c msrsave/msrsave.h

msrsave/msrsave_test: msrsave/msrsave_test.o msrsave/msrsave.o

SLURM_CFLAGS ?= -I/usr/include -fPIC -shared
SLURM_LDFLAGS ?= -L/usr/lib64 -lslurm -shared
spank: msrsave/libspank_msrsafe.so

msrsave/spank_msrsafe.o: msrsave/spank_msrsafe.c
	$(CC) $(CFLAGS) $(SLURM_CFLAGS) -c $^ -o $@

msrsave/libspank_msrsafe.so: msrsave/spank_msrsafe.o msrsave/msrsave.o
	$(CC) $(LDFLAGS) $(SLURM_LDFLAGS) $^ -o $@

INSTALL ?= install
prefix ?= $(HOME)/build
exec_prefix ?= $(prefix)
sbindir ?= $(exec_prefix)/sbin
libdir ?= $(exec_preffix)/lib
datarootdir ?= $(prefix)/share
mandir ?= $(datarootdir)/man
man1dir ?= $(mandir)/man1

install: msrsave/msrsave msrsave/msrsave.1
	$(INSTALL) -d $(DESTDIR)/$(sbindir)
	$(INSTALL) msrsave/msrsave $(DESTDIR)/$(sbindir)
	$(INSTALL) -d $(DESTDIR)/$(man1dir)
	$(INSTALL) -m 644 msrsave/msrsave.1 $(DESTDIR)/$(man1dir)

install-spank: spank
	$(INSTALL) -d $(DESTDIR)/$(libdir)/slurm
	$(INSTALL) msrsave/libspank_msrsafe.so $(DESTDIR)/$(libdir)/slurm/libspank_msrsafe.so

# The current spack package ignore this Makefile for building the
# msr-safe.ko kernel module, as it is building against an arbitrary
# version of the linux kernel and thus $(shell uname -r) is not useful.
#
# Installation relies on the spack package setting DESTDIR to the
# msr-safe package prefix spec variable.
#
# Later iterations of the spack package might also build and install
# msrsave.  That will likely require a reworking of this Makefile.
# Prefer single-source-of-truth in that case.
spack-install:
	$(INSTALL) -d $(DESTDIR)/lib/modules
	$(INSTALL) msr-safe.ko $(DESTDIR)/lib/modules
	$(INSTALL) -d $(DESTDIR)/include
	$(INSTALL) msr_safe.h $(DESTDIR)/include

.SUFFIXES: .c .o
.PHONY: all module clean install spank install-spank
