%define prefix /usr

Summary: The G App library.
Name: gal
Version: 1.99.10
Release: 1
License: LGPL
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
* Tue Apr 14 2003 Rui Miguel Silva Seabra <rms@1407.org>
- the spec works again
- s/Copyright/License/ this _is_ the correct tag

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

%find_lang %{name}-%{version}

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files -f %{name}-%{version}.lang
%defattr(0644, root, root, 0755)
%doc README AUTHORS COPYING ChangeLog NEWS
%{_datadir}/gal-2.0/%{version}/glade/*.glade
%{_datadir}/gal-2.0/%{version}/pixmaps/categories/*.png
%{_datadir}/gal-2.0/html
%{_libdir}/libgal-2.0.so*
%{_libdir}/gtk-2.0/modules/libgal-a11y-2.0.so
#%{prefix}/lib/galConf.sh

%files devel
%defattr(0644, root, root, 0755)
#%{prefix}/lib/libgal.so
%{_includedir}/gal-2.0
%{_libdir}/libgal-2.0.a
%{_libdir}/libgal-2.0.la
%{_libdir}/gtk-2.0/modules/libgal-a11y-2.0.a
%{_libdir}/gtk-2.0/modules/libgal-a11y-2.0.la
%{_libdir}/pkgconfig/gal-2.0.pc