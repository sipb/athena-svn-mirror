# Note that this is NOT a relocatable package
%define ver      1.9.8.1
%define  RELEASE 1
%define  rel     %{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}
%define prefix   /usr

Summary: An image loading and rendering library for X11R6
Name: imlib
Version: %ver
Release: %rel
Copyright: LGPL
Group: System Environment/Libraries
Source: ftp://ftp.labs.redhat.com/pub/imlib/imlib-%{ver}.tar.gz
Obsoletes: Imlib
BuildRoot: /var/tmp/imlib-%{PACKAGE_VERSION}-root
URL: http://www.labs.redhat.com/imlib/
Requires: libpng 
Requires: libtiff 
Requires: libjpeg
Requires: zlib 
Requires: libgr-progs 
Requires: gtk+ >= 1.2
Requires: libungif

Docdir: %{prefix}/doc

%description
Imlib is a display depth-independent image loading and rendering library.
Imlib is designed to simplify and speed up the process of loading images
and obtaining X Window System drawables.  Imlib provides many simple
manipulation routines which can be used for common operations.  

Install imlib if you need an image loading and rendering library for X11R6.
You may also want to install the imlib-cfgeditor package, which will help
you configure Imlib.

%package devel
Summary: Development tools for Imlib applications.
Group: Development/Libraries
Requires: imlib = %{PACKAGE_VERSION}
Requires: libpng-devel
Requires: libtiff-devel
Requires: libjpeg-devel
Requires: zlib-devel
Requires: libgr-devel
Requires: gtk+-devel
Requires: libungif-devel
Requires: XFree86-devel
Obsoletes: Imlib

%description devel
The header files, static libraries and documentation needed for
developing Imlib applications.  Imlib is an image loading and rendering
library for X11R6.

Install the imlib-devel package if you want to develop Imlib applications.
You'll also need to install the imlib and imlib_cfgeditor packages.

%package cfgeditor
Summary: A configuration editor for the Imlib library.
Group: System Environment/Libraries
Requires: imlib = %{PACKAGE_VERSION}

%description cfgeditor
The imlib-cfgeditor package contains the imlib_config program, which you
can use to configure the Imlib image loading and rendering library.
imlib_config can be used to control how Imlib uses color and handles
gamma corrections, etc.

If you're installing the imlib package, you should also install
imlib_cfgeditor.

%prep
%setup -q

%build
if [ ! -f configure ]; then
  CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh --prefix=%prefix --sysconfdir=/etc
else
  CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%prefix --sysconfdir=/etc
fi

if [ "$SMP" != "" ]; then
  make -j$SMP "MAKE=make -j$SMP"
else
  make
fi
###########################################################################

%install
rm -rf $RPM_BUILD_ROOT
make prefix=$RPM_BUILD_ROOT%{prefix} sysconfdir=$RPM_BUILD_ROOT/etc install

strip $RPM_BUILD_ROOT%{prefix}/bin/imlib_config

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
%doc README AUTHORS ChangeLog NEWS
%attr(755,root,root) %{prefix}/lib/lib*.so.*
%config /etc/*
%{prefix}/lib/libimlib-*.so

%files cfgeditor
%defattr(-,root,root)
%{prefix}/bin/imlib_config

%files devel
%defattr(-,root,root)
%doc doc/*.gif doc/*.html
%{prefix}/bin/imlib-config
%{prefix}/lib/libImlib.so
%{prefix}/lib/libgdk_imlib.so
%{prefix}/lib/*a
%{prefix}/include/*
%{prefix}/share/aclocal/*


%changelog
* Tue Aug 31 1999 Elliot Lee <sopwith@redhat.com>
- Updates from the RHL 6.0 package.

* Mon Jan 11 1999 Carsten Haitzler <raster@redhat.com>
- up to 1.9.0

* Wed Sep 23 1998 Carsten Haitzler <raster@redhat.com>
- up to 1.8.1

* Tue Sep 22 1998 Cristian Gafton <gafton@redhat.com>
- yet another build for today (%defattr and %attr in the files lists)
- devel docs are back on the spec file

* Tue Sep 22 1998 Carsten Haitzler <raster@redhat.com>
- Added minor patch for ps saving code.

* Mon Sep 21 1998 Cristian Gafton <gafton@redhat.com>
- updated to version 1.8

* Fri Sep 11 1998 Cristian Gafton <gafton@redhat.com>
- take out imlib_config from devel package

* Wed Sep 9 1998 Michael Fulbright <msf@redhat.com>
- upgraded to 1.7
- changed name so it will persist if user later install devel imlib
- added subpackage for imlib_config

* Fri Apr 3 1998 Michael K. Johnson <johnsonm@redhat.com>
- fixed typo

* Fri Mar 13 1998 Marc Ewing <marc@redhat.com>
- Added -k, Obsoletes
- Integrate into CVS source tree
