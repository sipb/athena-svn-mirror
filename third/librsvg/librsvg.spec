%define __libtoolize :
%define __spec_install_post /usr/lib/rpm/brp-compress
Name:             librsvg2
Summary:          An SVG library based on libart.
Version:          2.1.2
Release:          1
License:          LGPL
Group:            System Environment/Libraries
Source:           librsvg-%{version}.tar.gz
BuildRoot:        %{_tmppath}/%{name}-%{version}-root
BuildRequires:    pkgconfig >= 0.8
Requires:         gtk2 >= 1.3.7
Requires:         glib2 >= 2.0.0
Requires:         libart_lgpl >= 2.3.10
Requires:         libxml2 >= 2.4.7
Requires:         pango >= 1.0.0
BuildRequires:    gtk2-devel >= 1.3.7
BuildRequires:    glib2-devel >= 2.0.0
BuildRequires:    libart_lgpl-devel >= 2.3.10
BuildRequires:    libxml2-devel >= 2.4.7
BuildRequires:    pango-devel >= 1.0.0


%description
An SVG library based on libart.


%package devel
Summary:          Libraries and include files for developing with librsvg.
Group:            Development/Libraries
Requires:         %{name} = %{version}
Requires:         pkgconfig >= 0.8
Requires:         gtk2 >= 1.3.7
Requires:         gtk2-devel >= 1.3.7
Requires:         glib2 >= 2.0.0
Requires:         glib2-devel >= 2.0.0
Requires:         libart_lgpl >= 2.3.10
Requires:         libart_lgpl-devel >= 2.3.10
Requires:         libxml2 >= 2.4.7
Requires:         libxml2-devel >= 2.4.7
Requires:         pango >= 1.0.0
Requires:         pango-devel >= 1.0.0


%description devel
This package provides the necessary development libraries and include
files to allow you to develop with librsvg.

%package -n rsvg-gtk
Summary:        Gtk+ 2.0 theme engine for SVG based themes
Group:          System Environment/Libraries

%description -n rsvg-gtk
This package installs a GTK+ 2.0 theme engine that uses SVG images. It is based
on the gdkpixbuf engine.

%prep
%setup -q -n librsvg-%{version}

%build
%configure
make

%install
rm -rf $RPM_BUILD_ROOT

%makeinstall
# Clean out files that should not be part of the rpm.
# This is the recommended way of dealing with it for RH8
rm -f $RPM_BUILD_ROOT%{_libdir}/gtk-2.0/2.0.0/engines/*.la
rm -f $RPM_BUILD_ROOT%{_libdir}/gtk-2.0/2.0.0/engines//*.a
rm -f $RPM_BUILD_ROOT%{_libdir}/*.a
rm -f $RPM_BUILD_ROOT%{_libdir}/*.la


%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files

%defattr(-, root, root)
%doc AUTHORS COPYING COPYING.LIB ChangeLog NEWS README
%{_libdir}/*.so.*
%{_bindir}/rsvg
%{_sysconfdir}/gtk-2.0/gdk-pixbuf.loaders

%files devel
%defattr(-, root, root)
%{_libdir}/*.so
%{_includedir}/librsvg-2/librsvg
%{_libdir}/pkgconfig/librsvg-2.0.pc

%files -n rsvg-gtk
%defattr(-, root, root)
%{_libdir}/gtk-2.0/2.0.0/engines/*.so
# %{_datadir}/themes/bubble/gtk-2.0/*
# %{_datadir}/themes/bubble/README

%changelog
* Thu Oct 22 2002 Christian Schaller <Uraeus@linuxrising.org>
- Disabled building of example theme (as done in gtk-engines)

* Mon Oct 21 2002 Christian Schaller <Uraeus@linuxrising.org>
- Fixes for RH 8 
- Adding gtk theme engine
- adding gdk-loader

* Tue Mar 05 2002 Chris Chabot <chabotc@reviewboard.com>
- Deps
- Formatting
- converted to .spec.in

* Sat Jan 19 2002 Chris Chabot <chabotc@reviewboard.com>
- Imported into gnome 2.0 alpha, set Requirements accordingly
- Bumped version to 1.1.1
- Minor cleanups

* Wed Jan  2 2002 Havoc Pennington <hp@redhat.com>
- new CVS snap 1.1.0.91
- remove automake/autoconf calls

* Mon Nov 26 2001 Havoc Pennington <hp@redhat.com>
- convert to librsvg2 RPM

* Tue Oct 23 2001 Havoc Pennington <hp@redhat.com>
- 1.0.2

* Fri Jul 27 2001 Alexander Larsson <alexl@redhat.com>
- Add a patch that moves the includes to librsvg-1/librsvg
- in preparation for a later librsvg 2 library.

* Tue Jul 24 2001 Havoc Pennington <hp@redhat.com>
- build requires gnome-libs-devel, #49509

* Thu Jul 19 2001 Havoc Pennington <hp@redhat.com>
- own /usr/include/librsvg

* Wed Jul 18 2001 Akira TAGOH <tagoh@redhat.com> 1.0.0-4
- fixed the linefeed problem in multibyte environment. (Bug#49310)

* Mon Jul 09 2001 Havoc Pennington <hp@redhat.com>
- put .la file back in package

* Fri Jul  6 2001 Trond Eivind Glomsr�d <teg@redhat.com>
- Put changelog at the end
- Move .so files to devel subpackage
- Don't mess with ld.so.conf
- Don't use %%{prefix}, this isn't a relocatable package
- Don't define a bad docdir
- Add BuildRequires
- Use %%{_tmppath}
- Don't define name, version etc. on top of the file (why do so many do that?)
- s/Copyright/License/

* Wed May  9 2001 Jonathan Blandford <jrb@redhat.com>
- Put into Red Hat Build system

* Tue Oct 10 2000 Robin Slomkowski <rslomkow@eazel.com>
- removed obsoletes from sub packages and added mozilla and trilobite
subpackages

* Wed Apr 26 2000 Ramiro Estrugo <ramiro@eazel.com>
- created this thing

