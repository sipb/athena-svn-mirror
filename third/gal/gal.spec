%define prefix /usr

Summary: The G App library.
Name: gal
Version: 0.18
Release: 1
Copyright: LGPL
Group: System Environment/Libraries
Source: ftp://ftp.gnome.org/pub/GNOME/unstable/sources/gal/gal-%{version}.tar.gz
BuildRoot: /var/tmp/gal-%{PACKAGE_VERSION}-root

%description
Reuseable GNOME library functions.

%package devel
Summary: Libraries and include files for the G App library.
Group: Development/Libraries

%description devel
The gal-devel package includes the static libraries and header files for the
gal package.

Install gal-devel if you want to develop programs which will use gal.

%changelog
* Thu Mar 15 2001 Matthew Wilson <msw@redhat.com>
- the -devel package must own the /usr/include/gal directory and all
  its contents

* Fri Oct 10 2000 John Gotts <jgotts@linuxsavvy.com>
- Created spec file.

%prep
%setup

%build
./configure --prefix=%{prefix} --sysconfdir=/etc --enable-static --enable-shared
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{prefix}

make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%doc README AUTHORS COPYING ChangeLog NEWS
%{prefix}/share/etable/%{version}/glade/*.glade
%{prefix}/lib/libgal.so.*
%{prefix}/lib/galConf.sh

%files devel
%defattr(-, root, root)
%{prefix}/lib/libgal.so
%{prefix}/lib/libgal.a
%{prefix}/lib/libgal.la
%{prefix}/include/gal
