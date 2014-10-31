# Copyright (c) 2011, 2012, 2013, 2014 by Lawrence Livermore National Security, LLC. LLNL-CODE-645430 
# Produced at the Lawrence Livermore National Laboratory.
# Written by Kathleen Shoga and Barry Rountree (shoga1|rountree@llnl.gov).
# All rights reserved.
#
# This file is part of msr-safe.
#
# msr-safe is free software: you can redistribute it and/or 
# modify it under the terms of the GNU Lesser General Public 
# License as published by the Free Software Foundation, either 
# version 3 of the License, or (at your option) any
# later version.
#
# msr-safe is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public 
# License along
# with msr-safe. If not, see <http://www.gnu.org/licenses/>.
#
# This material is based upon work supported by the U.S. Department
# of Energy's Lawrence Livermore National Laboratory. Office of 
# Science, under Award number DE-AC52-07NA27344.
#
# Based off of work from qib
#

VERSION=$(shell grep Version META | sed -e "s/Version:[ \t]*\(.*\)/\1/")
RELEASE=test
KERNVER=$(shell uname -r)
KERNDIR=/usr/src/kernels/$(KERNVER)

default:
	make RELEASE=$(VERSION)-$(RELEASE)$(KERNVER) -C $(KERNDIR) M=`pwd`

clean:
	make -C /usr/src/kernels/`uname -r` M=`pwd` clean
	rm -f *.tgz

dist: clean
	mkdir -p /tmp/msr-safe-$(VERSION); \
	sed -e 's/#VERSION#/'$(VERSION)'/g' msr-safe.spec.in > /tmp/msr-safe-$(VERSION)/msr-safe.spec ; \
	cp -a * /tmp/msr-safe-$(VERSION); \
	sed -e 's/#VERSION#/'$(VERSION)'/g' msr-safe.spec.in > msr-safe.spec ; \
	pushd /tmp; \
	rm -f ./msr-safe-$(VERSION)/msr-safe.spec.in ./msr-safe-$(VERSION)/cscope*; \
	tar czf msr-safe-$(VERSION).tgz ./msr-safe-$(VERSION); \
	popd; \
	mv /tmp/msr-safe-$(VERSION).tgz .

install:
	mkdir -p $(RPM_BUILD_ROOT)/lib/modules/$(KERNVER)/updates
	install msr_safe.ko $(RPM_BUILD_ROOT)/lib/modules/$(KERNVER)/updates
	mkdir -p $(RPM_BUILD_ROOT)/usr/sbin
	install msrx $(RPM_BUILD_ROOT)/usr/sbin
