#
# Note that this is NOT a relocatable package
# $Id: medusa.spec,v 1.1.1.1 2001-01-16 15:30:54 ghudson Exp $
#
%define name	 medusa
%define ver      0.2.2
%define RELEASE  1	 
%define rel      %{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}
%define prefix   /usr
%define sysconfdir /etc

Summary: Medusa, the search and indexing package for use with Eazel's Nautilus.
Name: %name
Version: %ver
Release: %rel
Copyright: LGPL
Vendor:	Eazel Inc.
Distribution:	Eazel PR2
Group: System Environment/Base
Source: ftp://ftp.gnome.org/pub/GNOME/unstable/sources/medusa/medusa-%{ver}.tar.gz
BuildRoot: /var/tmp/medusa
URL: http://www.gnome.org
Prereq: /sbin/install-info
Prefix: %{prefix}
Docdir: %{prefix}/doc
Requires: glib >= 1.2.0
Requires: gnome-vfs >= 0.1

%description
Medusa, the GNOME search/indexing package.

%package devel
Summary:        Libraries and include files for developing nautilus components
Group:          Development/Libraries
Requires:       %name = %{PACKAGE_VERSION}
Obsoletes:      %{name}-devel

%description devel
This package provides the necessary development libraries and include 
files to allow you to develop medusa components.

%changelog
* Sun Jun 11 2000  Eskil Heyn Olsen <deity@eazel.com>
- Created the .spec file

%prep
%setup

%build
# Needed for snapshot releases.
if [ ! -f configure ]; then
	CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh --enable-more-warnings --prefix=%prefix --sysconfdir=$RPM_BUILD_ROOT/etc
else
	CFLAGS="$RPM_OPT_FLAGS" ./configure --enable-more-warnings --prefix=%prefix --sysconfdir=$RPM_BUILD_ROOT/etc
fi
make -k check

%install
rm -rf $RPM_BUILD_ROOT
make -k prefix=$RPM_BUILD_ROOT%{prefix} install

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr (0755, bin, bin)
%config %{sysconfdir}/cron.daily/medusa.cron

%defattr(-, bin, bin)
%config %{sysconfdir}/vfs/modules/*.conf
%{prefix}/bin/medusa-config
%{prefix}/bin/medusa-indexd
%{prefix}/bin/medusa-searchd
%{prefix}/bin/msearch
%{prefix}/lib/*.0
%{prefix}/lib/*.so
%{prefix}/lib/vfs/modules/*.so
%{prefix}/share/medusa/file-index-stoplist

%doc AUTHORS COPYING ChangeLog NEWS README

%files devel
%defattr(-, bin, bin)
%{prefix}/include/libmedusa/*.h
%{prefix}/lib/*.la
%{prefix}/lib/vfs/modules/*.la