Summary: A collection of utilities and DSOs to handle compiled objects.
Name: elfutils
Version: 0.76
Release: 1
Copyright: GPL
Group: Development/Tools
URL: file://home/devel/drepper/
Source: elfutils-0.76.tar.gz
Requires: elfutils-libelf = %{version}-%{release}

# ExcludeArch: xxx

BuildRoot: %{_tmppath}/%{name}-root
BuildRequires: gcc >= 3.2

%define _gnu %{nil}
%define _programprefix eu-

%description
Elfutils is a collection of utilities, including ld (a linker),
nm (for listing symbols from object files), size (for listing the
section sizes of an object or archive file), strip (for discarding
symbols), readline (the see the raw ELF file structures), and elflint
(to check for well-formed ELF files).  Also included are numerous
helper libraries which implement DWARF, ELF, and machine-specific ELF
handling.

%package devel
Summary: Development libraries to handle compiled objects.
Group: Development/Tools
Requires: elfutils = %{version}-%{release}
Conflicts: libelf-devel

%description devel
The elfutils-devel package contains the libraries to create
applications for handling compiled objects.  libelf allows you to
access the internals of the ELF object file format, so you can see the
different sections of an ELF file.  libebl provides some higher-level
ELF access functionality.  libdwarf provides access to the DWARF
debugging information.  libasm provides a programmable assembler
interface.

%package libelf
Summary: Library to read and write ELF files.
Group: Development/Tools

%description libelf
The elfutils-libelf package provides a DSO which allows reading and
writing ELF files on a high level.  Third party programs depend on
this package to read internals of ELF files.  The programs of the
elfutils package use it also to generate new ELF files.

%prep
%setup -q
# %patch0 -p1 -b .jbj

%build
mkdir build-%{_target_platform}
cd build-%{_target_platform}
../configure \
  --prefix=%{_prefix} --exec-prefix=%{_exec_prefix} \
  --bindir=%{_bindir} --sbindir=%{_sbindir} --sysconfdir=%{_sysconfdir} \
  --datadir=%{_datadir} --includedir=%{_includedir} --libdir=%{_libdir} \
  --libexecdir=%{_libexecdir} --localstatedir=%{_localstatedir} \
  --sharedstatedir=%{_sharedstatedir} --mandir=%{_mandir} \
  --infodir=%{_infodir} --program-prefix=%{_programprefix} --enable-shared
cd ..

%install
rm -rf ${RPM_BUILD_ROOT}
mkdir -p ${RPM_BUILD_ROOT}%{_prefix}

cd build-%{_target_platform}
echo '======================== Testing =========================='
make check

%makeinstall

chmod +x ${RPM_BUILD_ROOT}%{_prefix}/%{_lib}/lib*.so*
chmod +x ${RPM_BUILD_ROOT}%{_prefix}/%{_lib}/elfutils/lib*.so*

cd ..

# XXX Nuke unpackaged files
{ cd ${RPM_BUILD_ROOT}
  rm -f .%{_bindir}/eu-ld
  rm -f .%{_includedir}/elfutils/libasm.h
  rm -f .%{_includedir}/elfutils/libdw.h
  rm -f .%{_includedir}/elfutils/libdwarf.h
  rm -f .%{_libdir}/libasm-%{version}.so
  rm -f .%{_libdir}/libasm.a
  rm -f .%{_libdir}/libdw-%{version}.so
  rm -f .%{_libdir}/libdw.a
  rm -f .%{_libdir}/libdwarf.a
}

%clean
rm -rf ${RPM_BUILD_ROOT}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
%doc README TODO libdwarf/AVAILABLE
%{_bindir}/eu-elflint
#%{_bindir}/eu-ld
%{_bindir}/eu-nm
%{_bindir}/eu-readelf
%{_bindir}/eu-size
%{_bindir}/eu-strip
#%{_libdir}/libasm-%{version}.so
%{_libdir}/libebl-%{version}.so
#%{_libdir}/libdw-%{version}.so
%{_libdir}/libdwarf-%{version}.so
#%{_libdir}/libasm*.so.*
%{_libdir}/libebl*.so.*
#%{_libdir}/libdw*.so.*
%{_libdir}/libdwarf*.so.*
%dir %{_libdir}/elfutils
%{_libdir}/elfutils/lib*.so

%files devel
%defattr(-,root,root)
%{_includedir}/dwarf.h
%{_includedir}/libelf.h
%{_includedir}/gelf.h
%{_includedir}/nlist.h
%dir %{_includedir}/elfutils
%{_includedir}/elfutils/elf-knowledge.h
%{_includedir}/elfutils/libebl.h
#%{_libdir}/libasm.a
%{_libdir}/libebl.a
%{_libdir}/libelf.a
#%{_libdir}/libdw.a
#%{_libdir}/libasm.so
%{_libdir}/libebl.so
%{_libdir}/libelf.so
#%{_libdir}/libdw.so
#%{_libdir}/libdwarf.so

%files libelf
%defattr(-,root,root)
%{_libdir}/libelf-%{version}.so
%{_libdir}/libelf*.so.*

%changelog
* Mon Dec  2 2002 Jeff Johnson <jbj@redhat.com> 0.64-2
- update to 0.64.

* Sun Dec 1 2002 Ulrich Drepper <drepper@redhat.com> 0.64
- split packages further into elfutils-libelf

* Sat Nov 30 2002 Jeff Johnson <jbj@redhat.com> 0.63-2
- update to 0.63.

* Fri Nov 29 2002 Ulrich Drepper <drepper@redhat.com> 0.62
- Adjust for dropping libtool

* Sun Nov 24 2002 Jeff Johnson <jbj@redhat.com> 0.59-2
- update to 0.59

* Thu Nov 14 2002 Jeff Johnson <jbj@redhat.com> 0.56-2
- update to 0.56

* Thu Nov  7 2002 Jeff Johnson <jbj@redhat.com> 0.54-2
- update to 0.54

* Sun Oct 27 2002 Jeff Johnson <jbj@redhat.com> 0.53-2
- update to 0.53
- drop x86_64 hack, ICE fixed in gcc-3.2-11.

* Sat Oct 26 2002 Jeff Johnson <jbj@redhat.com> 0.52-3
- get beehive to punch a rhpkg generated package.

* Wed Oct 23 2002 Jeff Johnson <jbj@redhat.com> 0.52-2
- build in 8.0.1.
- x86_64: avoid gcc-3.2 ICE on x86_64 for now.

* Tue Oct 22 2002 Ulrich Drepper <drepper@redhat.com> 0.52
- Add libelf-devel to conflicts for elfutils-devel

* Mon Oct 21 2002 Ulrich Drepper <drepper@redhat.com> 0.50
- Split into runtime and devel package

* Fri Oct 18 2002 Ulrich Drepper <drepper@redhat.com> 0.49
- integrate into official sources

* Wed Oct 16 2002 Jeff Johnson <jbj@redhat.com> 0.46-1
- Swaddle.
