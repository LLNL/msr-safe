%global rev         %(git rev-parse HEAD)
%global shortrev    %(r=%{rev}; echo ${r:0:12})

Name:       msr-safe
Version:    0
Release:    0.4.git%{shortrev}%{?dist}
License:    GPLv3+
Summary:    Allows safer access to model specific registers (MSRs)
Url:        https://github.com/LLNL/msr-safe
Group:      System Environment/Daemons
Source0:    https://github.com/LLNL/%{name}/archive/%{rev}.tar.gz#/%{name}-%{rev}.tar.gz
Source1:    msr-safe.service
Source2:    msr-safe.sysconfig
Source3:    10-msr-safe.rules
Source4:    msr-safe.sh
BuildRoot:  %{_tmppath}/%{name}-%{version}-build
BuildRequires:  %kernel_module_package_buildreqs
BuildRequires:  systemd

%kernel_module_package default

%description
Allows safer access to model specific registers (MSRs)

%prep
%autosetup -n %{name}-%{rev}

%build
for flavor in %flavors_to_build; do
    rm -rf obj/$flavor
    mkdir -p obj/$flavor
    cp -r msr* Makefile obj/$flavor
    %{__make} -C %{kernel_source $flavor} M=$PWD/obj/$flavor
done
%{__make} CPPFLAGS="-DVERSION=\\\"%{version}-%{release}\\\"" msrsave/msrsave

%install
%{__make} install DESTDIR=%{buildroot} prefix=%{_prefix} sbindir=%{_sbindir} mandir=%{_mandir}
install -d %{buildroot}/%{_datadir}/msr-safe/whitelists
install -m 0644 whitelists/* %{buildroot}/%{_datadir}/msr-safe/whitelists/
install -d %{buildroot}%{_unitdir}
install -m 0644 %{SOURCE1} %{buildroot}%{_unitdir}/msr-safe.service
install -d %{buildroot}/%{_sysconfdir}/sysconfig
install -m 0644 %{SOURCE2} %{buildroot}/%{_sysconfdir}/sysconfig/msr-safe
install -d %{buildroot}/%{_udevrulesdir}
install -m 0644 %{SOURCE3} %{buildroot}/%{_udevrulesdir}/10-msr-safe.rules
install -d %{buildroot}%{_sbindir}
install -m 0755 %{SOURCE4} %{buildroot}%{_sbindir}/msr-safe

export INSTALL_MOD_PATH=$RPM_BUILD_ROOT
export INSTALL_MOD_DIR=extra/%{name}
for flavor in %flavors_to_build ; do
        make -C %{kernel_source $flavor} modules_install \
                M=$PWD/obj/$flavor
done

%clean
rm -rf %{buildroot}

%pre
getent group msr >/dev/null || groupadd -r msr
exit 0

%post
/usr/bin/udevadm control --reload-rules
echo /lib/modules/%{latest_kernel}/extra/msr-safe/msr-safe.ko | weak-modules --add-modules
/usr/bin/systemctl daemon-reload >/dev/null 2>&1
/usr/bin/systemctl enable msr-safe >/dev/null 2>&1 || :

%preun
if [ $1 -eq 0 ] ; then
    /usr/bin/systemctl stop msr-safe >/dev/null 2>&1
    /usr/bin/systemctl disable msr-safe >/dev/null 2>&1
fi
echo /lib/modules/%{latest_kernel}/extra/msr-safe/msr-safe.ko | weak-modules --remove-modules

%postun
if [ "$1" -ge "1" ] ; then
   /usr/bin/systemctl try-restart msr-safe >/dev/null 2>&1 || :
fi

%files
%defattr(-,root,root,-)
%{_datadir}/msr-safe/whitelists/*
%{_unitdir}/msr-safe.service
%{_udevrulesdir}/10-msr-safe.rules
%config %{_sysconfdir}/sysconfig/msr-safe
%doc README
%{_sbindir}/msrsave
%{_sbindir}/msr-safe
%dir %{_mandir}/man1
%doc %{_mandir}/man1/msrsave.1.gz


%changelog
* Tue Aug 23 2017 Ben Allen <bsallen@alcf.anl.gov> 9240638-4
- Rev package version to 0.4
* Tue Jan 17 2017 Ben Allen <bsallen@alcf.anl.gov> 7bfb843-2
- Rev package version to 0.2
* Mon Dec 05 2016 Christopher M. Cantalupo <christopher.m.cantalupo@intel.com>
- Add msrsave application build and install.
* Wed Apr 20 2016 Ben Allen <bsallen@alcf.anl.gov> 7667118-1
- Initial release (v7667118)

