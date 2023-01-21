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

obj-m += msr-safe.o
msr-safe-objs := msr_entry.o msr_allowlist.o msr-smp.o msr_batch.o msr_version.o

all: msrsave/msrsave
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f msrsave/msrsave.o msrsave/msrsave msrsave/msrsave_test msrsave/spank_msrsafe.o msrsave/libspank_msrsafe.so

check: msrsave/msrsave_test
	msrsave/msrsave_test

# current msr-safe/msrsave version
CURRENT_VERSION := -DVERSION=\"1.6.0\"

msrsave/msrsave.o: msrsave/msrsave.c msrsave/msrsave.h
	$(CC) $(CFLAGS) $(CURRENT_VERSION) -fPIC -c msrsave/msrsave.c -o $@

msrsave/msrsave_main.o: msrsave/msrsave_main.c msrsave/msrsave.h
	$(CC) $(CFLAGS) $(CURRENT_VERSION) -fPIC -c msrsave/msrsave_main.c -o $@

msrsave/msrsave: msrsave/msrsave_main.o msrsave/msrsave.o
	$(CC) $(CFLAGS) $(CURRENT_VERSION) $^ -o $@

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

.SUFFIXES: .c .o
.PHONY: all clean install spank install-spank
