# Stolen from qib
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

%post
depmod -a %{kver}

