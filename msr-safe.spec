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
# Based off work from qib
#

%if 0%{?require_kver:1}
%define kdir /usr/src/kernels/%{require_kver}
%define kver %{require_kver}
BuildRequires: kernel-devel = %{kver}
%else
%define kdir %((echo X; ls -1d /usr/src/kernels/*)|tail -1)
%define kver %(basename %{kdir})
BuildRequires: kernel-devel
%endif
%if 0%{!?build_src_rpm:1}
%define relext _%(echo %{kver}|sed -e 's/-/_/g')
%endif

Name: msr-safe
Version: #VERSION#
Release: test
Source: msr-safe-%{version}.tgz

Summary: Safe MSR driver
Packager: Jim Foraker <foraker1@llnl.gov>
License: GPLv2 or BSD
Group: System Environment/Base
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Url: http://computation.llnl.gov/
%define arch %(uname -p)
Requires: kernel = %(echo %{kver} | sed -e 's/\(.*\)\.%{arch}/\1/g')

%description
A sanitized version of the MSR kernel driver.

%prep
%setup -q

%build
make VERSION=%version KERNVER=%kver KERNDIR=%kdir RELEASE=%release

%install
make VERSION=%version KERNVER=%kver KERNDIR=%kdir RELEASE=%release DESTDIR=${RPM_BUILD_ROOT} install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/lib/modules/*
/usr/sbin/msrx

%post
depmod -a %{kver}

