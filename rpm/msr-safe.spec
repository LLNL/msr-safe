%global rev         %(git rev-parse HEAD)
%global shortrev    %(r=%{rev}; echo ${r:0:12})

Name:       msr-safe
Version:    0
Release:    0.1.git%{shortrev}%{?dist}
License:    GPLv3+
Summary:    Allows safer access to model specific registers (MSRs)
Url:        https://github.com/LLNL/msr-safe
Group:      System Environment/Daemons
Source0:    https://github.com/LLNL/%{name}/archive/%{rev}.tar.gz#/%{name}-%{rev}.tar.gz
Source1:    msr-safe.service
Source2:    msr-safe.sysconfig
Source3:    10-msr-safe.rules
BuildRoot:  %{_tmppath}/%{name}-%{version}-build
BuildRequires:  %kernel_module_package_buildreqs

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
    make -C %{kernel_source $flavor} M=$PWD/obj/$flavor
done
make msrsave

%install
make install DESTDIR=%{buildroot} prefix=%{_prefix} sbindir=%{_sbindir} mandir=%{_mandir}
install -d %{buildroot}/%{_datadir}/msr-safe/whitelists
install -m 0644 whitelists/* %{buildroot}/%{_datadir}/msr-safe/whitelists/
install -d %{buildroot}%{_unitdir}
install -m 0644 %{SOURCE1} %{buildroot}%{_unitdir}/msr-safe.service
install -d %{buildroot}/%{_sysconfdir}/sysconfig
install -m 0644 %{SOURCE2} %{buildroot}/%{_sysconfdir}/sysconfig/msr-safe
install -d %{buildroot}/%{_udevrulesdir}
install -m 0644 %{SOURCE3} %{buildroot}/%{_udevrulesdir}/10-msr-safe.rules

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
%dir %{_mandir}/man1
%doc %{_mandir}/man1/msrsave.1.gz


%changelog
* Mon Dec 05 2016 Christopher M. Cantalupo <christopher.m.cantalupo@intel.com>
- Add msrsave application build and install.
* Wed Apr 20 2016 Ben Allen <bsallen@alcf.anl.gov> 7667118-1
- Initial release (v7667118)

