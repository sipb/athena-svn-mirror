#
# Note that this is NOT a relocatable package
# $Id: gconf.spec,v 1.1.1.2 2003-01-29 20:00:20 ghudson Exp $
#
%define ver      2.2.0
%define rel      1
%define	prefix   %{_prefix}
%define name	 GConf
%define sysconfdir	/etc

Summary: Gnome Config System
Name: %name
Version: %ver
Release: %rel
Copyright: LGPL
Group: System Environment/Base
Source: ftp://ftp.gnome.org/pub/GNOME/unstable/sources/GConf/GConf-%{ver}.tar.gz
BuildRoot: /var/tmp/gconf-root
Packager: Eskil Heyn Olsen <eskil@eazel.com>
URL: http://www.gnome.org/
Prereq: /sbin/install-info
Prefix: %{prefix}
Docdir: %{prefix}/doc
Requires: glib >= 1.2.0
Requires: oaf >= 0.3.0
Requires: gtk+ >= 1.2.0
Requires: ORBit >= 0.5.0
Requires: libxml >= 1.8.0

%description
GConf is the GNOME Configuration database system.

GNOME is the GNU Network Object Model Environment.  That's a fancy
name but really GNOME is a nice GUI desktop environment.  It makes
using your computer easy, powerful, and easy to configure.

%package devel
Summary: Gnome Config System development package
Group: Development/Libraries
Requires: %name = %{ver}
Requires: ORBit-devel
Requires: glib-devel
Requires: oaf-devel
Requires: gtk+-devel
PreReq: /sbin/install-info

%description devel
GConf development package. Contains files needed for doing
development using GConf.

%changelog

* Sat Oct 13 2001 Ross Golder <ross@golder.org>
- Updated to reflect installation path changes

* Sun Jun 11 2000  Eskil Heyn Olsen <deity@eazel.com>
- Created the .spec file

%prep
%setup

%build
# Needed for snapshot releases.
if [ ! -f configure ]; then
  CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh --prefix=%prefix --sysconfdir=%{sysconfdir}
else
  CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%prefix --sysconfdir=%{sysconfdir}
fi

if [ "$SMP" != "" ]; then
  (make "MAKE=make -k -j $SMP"; exit 0)
  make
else
  make
fi

%install
rm -rf $RPM_BUILD_ROOT
make prefix=$RPM_BUILD_ROOT%{prefix} sysconfdir=$RPM_BUILD_ROOT%{sysconfdir} install

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-, root, root)
%doc AUTHORS COPYING ChangeLog NEWS README
%config(noreplace) %{sysconfdir}/gconf/2/path
%config(noreplace) %{sysconfdir}/gconf/schemas/*.schemas
%dir	%{sysconfdir}/gconf/gconf.xml.defaults
%dir	%{sysconfdir}/gconf/gconf.xml.mandatory
%{prefix}/libexec/gconfd-2
%{prefix}/bin/gconftool
%{prefix}/bin/gconftool-2
%{prefix}/lib/lib*.so.*
%{prefix}/lib/GConf/2/*.so
%{prefix}/lib/pkgconfig/*.pc
%{prefix}/share/locale/*/LC_MESSAGES/*.mo
%{prefix}/share/gconf/2


## /etc/gconf/schemas/desktop.schemas is notably missing;
## it will be shared between versions of GConf, preventing
## simulataneous installation, so maybe should be in 
## a different (minuscule) package.

%files devel
%defattr(-, root, root)
%{prefix}/lib/*.a
%{prefix}/lib/*.la
%{prefix}/lib/*.so
#%{prefix}/lib/GConf/2/*.a
#%{prefix}/lib/GConf/2/*.la
%{prefix}/include/gconf/2/gconf/*.h
%{prefix}/share/aclocal/*.m4