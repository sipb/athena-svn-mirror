Summary: A library for manipulating GIF format image files.
Name: libungif
Version: 4.1.0
Release: 1
Copyright: MIT
Group: Libraries
Source0: ftp://prtr-13.ucsc.edu/pub/libungif/%{name}-%{version}.tar.gz
URL: http://prtr-13.ucsc.edu/~badger/software/libungif/
BuildRoot: /var/tmp/libungif-root
Prefix: /usr

Provides: libgif.so.4 libgif.so giflib
Obsoletes: giflib libungif-rhcn

%description
The libungif package contains a shared library of functions for loading
and saving GIF format image files.  The libungif library can load any
GIF file, but it will save GIFs only in uncompressed format (i.e., it
won't use the patented LZW compression used to save "normal" compressed
GIF files).

Install the libungif package if you need to manipulate GIF files.  You
should also install the libungif-progs package.

%package devel
Summary: Development tools for programs which will use the libungif library. 
Group: Development/Libraries
%description devel
This package contains the static libraries, header files and documentation
necessary for development of programs that will use the libungif library
to load and save GIF format image files.

You should install this package if you need to develop programs which
will use the libungif library functions for loading and saving GIF format
image files.  You'll also need to install the libungif package.

%package progs
Summary: Programs for converting and transforming gif images.
Group: Applications/Graphics
%description progs
The libungif-progs package contains various programs for manipulating
GIF format image files.

Install this package if you need to manipulate GIF format image files.
You'll also need to install the libungif package.

%prep
%setup

%build
libtoolize --copy --force
CFLAGS="${RPM_OPT_FLAGS}" ./configure --prefix=/usr
make all

%install
rm -rf ${RPM_BUILD_ROOT}
make DESTDIR="${RPM_BUILD_ROOT}" install-strip

{
    cd ${RPM_BUILD_ROOT}/usr/lib
    chmod +x libungif.so.%{version}
    ln -sf libungif.so.%{version} ./libgif.so.%{version}
    ln -sf libgif.so.%{version} ./libgif.so.4
    ln -sf libgif.so.4 libgif.so
    ln -sf libungif.a libgif.a
}

%clean
rm -rf ${RPM_BUILD_ROOT}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(- root root)
%doc COPYING README UNCOMPRESSED_GIF NEWS ONEWS
%doc ChangeLog TODO BUGS AUTHORS
/usr/lib/lib*.so.*

%files devel
%defattr(- root root)
%doc doc/*
%doc util/giffiltr.c
%doc util/gifspnge.c
/usr/lib/lib*.a
/usr/lib/lib*.so
/usr/lib/lib*.la
/usr/include/gif_lib.h

%files progs
%attr(0755 root root) /usr/bin/*

%changelog
* Tue Jan 19 1999 Toshio Kuratomi <badger@prtr-13.ucsc.edu>
- Upgrade to version 4.1
  + Fix a few minor memory leaks in error conditions.
  + Add a new function EGifOpen( void *userData, OutputFunc writeFunc) that
    allows user specified gif writing function.
- Merge spec file from libungif-3.1-5

* Mon Dec 14 1998 Toshio Kuratomi <badger@prtr-13.ucsc.edu>
- Upgrade to version 4.0 (Fixes rather nasty behaviour when dealing with
  Extensions.)

* Fri Oct 9 1998 Toshio Kuratomi <badger@prtr-13.ucsc.edu>
- Upgrade to version 3.1.1 (Fixes some segfaults and memory leaks when dealing
  with extensions and colourmaps.)

* Mon Sep 7 1998 Toshio Kuratomi <badger@prtr-13.ucsc.edu>
- Upgrade to version 3.1.0 (which incorporates the patches in 3.0-4)
- Updated Source: and URL: to reflect change in directories/pages.

* Tue May 26 1998 Dick Porter <dick@cymru.net>
- Fixed some "warning: cast to pointer from integer of different size" on Alpha

* Tue May 5 1998 Marc Ewing <marc@redhat.com>
- Made it obsolete giflib and provide libgif.so and giflib (previous
  giflib packages were built incorrectly and packages built against
  it require libgif.so but work fine with this package)
- cleaned buildroot
- Removed Toshio as packager so he doesn't get yelled at when Red Hat
  breaks it :-)

* Fri Apr 24 1998 Toshio Kuratomi <badger@prtr-13.ucsc.edu>
- Initial revision of libungif, a giflib derivative modified to not use LZW
  compression.
