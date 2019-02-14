# Copyright (c) 2011-2017 by Lawrence Livermore National Security, LLC.
# LLNL-CODE-645430
#
# Produced at Lawrence Livermore National Laboratory.
# Written by  Marty McFadden, mcfadden8@llnl.gov
#             Kathleen Shoga, shoga1@llnl.gov
#             Barry Rountree, rountree@llnl.gov
#
# All rights reserved.
#
# This file is part of msr-safe.
#
# msr-safe is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.
#
# msr-safe is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with msr-safe. If not, see <http://www.gnu.org/licenses/>.
#
# This material is based upon work supported by the U.S. Department of
# Energy's Lawrence Livermore National Laboratory. Office of Science, under
# Award number DE-AC52-07NA27344.

# Uncomment following line(s) to enable debug output from different
# portions of kernel module.
#
#CFLAGS_msr_whitelist.o := -DDEBUG
#CFLAGS_msr_entry.o := -DDEBUG
#CFLAGS_msr_batch.o := -DDEBUG
#CFLAGS_msr-smp.o := -DDEBUG

obj-m += msr-safe.o
msr-safe-objs := msr_entry.o msr_whitelist.o msr-smp.o msr_batch.o

all: msrsave/msrsave
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f msrsave/msrsave.o msrsave/msrsave msrsave/msrsave_test msrsave/spank_msrsafe.o msrsave/libspank_msrsafe.so

check: msrsave/msrsave_test
	msrsave/msrsave_test

msrsave/msrsave.o: msrsave/msrsave.c msrsave/msrsave.h
	$(CC) $(CFLAGS) -fPIC -shared -c msrsave/msrsave.c -o $@

msrsave/msrsave_main.o: msrsave/msrsave_main.c msrsave/msrsave.h

msrsave/msrsave: msrsave/msrsave_main.o msrsave/msrsave.o

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
