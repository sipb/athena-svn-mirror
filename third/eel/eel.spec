%define __spec_install_post /usr/lib/rpm/brp-compress
Name:             eel2
Summary:          Eazel Extensions Library.
Version:          2.1.5
Release:          1
License:          GPL
Group:            System Environment/Libraries
Source:           eel-%{version}.tar.gz
Source2:          fixed-ltmain.sh
URL:              http://nautilus.eazel.com/
BuildRoot:        %{_tmppath}/%{name}-%{version}-root
BuildRequires:    pkgconfig >= 0.8
Requires:         GConf2 >= 1.1.11
Requires:         gtk2 >= 2.1.0
Requires:         glib2 >= 2
Requires:         gnome-vfs2 >= 1.9
Requires:         libart_lgpl >= 2.3.8
Requires:         libgnome >= 2.0
Requires:         libgnomeui >= 2.0
Requires:         libxml2 >= 2.4.7
Prereq:           GConf2
BuildRequires:    GConf2-devel >= 1.1.11
BuildRequires:    gtk2-devel >= 2.1.0
BuildRequires:    glib2-devel >= 2
BuildRequires:    gnome-vfs2-devel >= 1.9
BuildRequires:    libart_lgpl-devel >= 2.3.8
BuildRequires:    libgnome-devel >= 2.0
BuildRequires:    libgnomeui-devel >= 2.0
BuildRequires:    libxml2-devel >= 2.4.7


%description
Eazel Extensions Library is a collection of widgets and functions for
use with GNOME.

%package devel
Summary:          Libraries and include files for developing with Eel.
Group:            Development/Libraries
Requires:         %{name} = %{version}
Requires:         pkgconfig >= 0.8
Requires:         GConf2 >= 1.1.11
Requires:         GConf2-devel >= 1.1.11
Requires:         gtk2 >= 2.1.0
Requires:         gtk2-devel >= 2.1.0
Requires:         glib2 >= 2
Requires:         glib2-devel >= 2
Requires:         gnome-vfs2 >= 1.9
Requires:         gnome-vfs2-devel >= 1.9
Requires:         libart_lgpl >= 2.3.8
Requires:         libart_lgpl-devel >= 2.3.8
Requires:         libgnome >= 2.0
Requires:         libgnome-devel >= 2.0
Requires:         libgnomeui >= 2.0
Requires:         libgnomeui-devel >= 2.0
Requires:         libxml2 >= 2.4.7
Requires:         libxml2-devel >= 2.4.7


%description devel
This package provides the necessary development libraries and include
files to allow you to develop with Eel.

%prep
%setup -q -n eel-%{version}

%build
rm ltmain.sh && cp %{SOURCE2} ltmain.sh
%configure

make

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall

%find_lang eel-2.0

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files -f eel-2.0.lang

%defattr(-,root,root)
%doc AUTHORS COPYING COPYING.LIB ChangeLog NEWS README
%{_libdir}/*.so*

%files devel
%defattr(-,root,root)
%{_libdir}/*.so
%{_libdir}/*a
%{_libdir}/pkgconfig
%{_includedir}/eel-2

%changelog
* Tue Mar 05 2002 Chris Chabot <chabotc@reviewboard.com>
- Fixed last small format items
- Converted to .spec.in
- Added deps

* Mon Feb 04 2002 Roy-Magne Mo <rmo@sunnmore.net>
- Fixed lang

* Sun Jan 20 2002 Chris Chabot <chabotc@reviewboard.com>
- Various cleanups
- moved build path from hard coded to _tmppath

* Sat Jan 19 2002 Chris Chabot <chabotc@reviewboard.com>
- Bumped version to 1.1.2

* Mon Nov 26 2001 Havoc Pennington <hp@redhat.com>
- Eel version 2 package created

* Tue Oct 23 2001 Havoc Pennington <hp@redhat.com>
- 1.0.2

* Wed Aug 29 2001 Alex Larsson <alexl@redhat.com>
- Added new font with cyrrilic glyphs from
- ftp://ftp.gnome.ru/fonts/urw/
- This closes #52772

* Mon Aug 27 2001 Alex Larsson <alexl@redhat.com> 1.0.1-18
- Add patch to fix #52348

* Thu Aug 23 2001 Havoc Pennington <hp@redhat.com>
- Applied patch from CVS to try fixing #51965

* Wed Aug 22 2001 Havoc Pennington <hp@redhat.com>
- Applied patch to handle multibyte chars in
eel_string_ellipsize - hopefully fixes #51710

* Fri Aug 17 2001 Alexander Larsson <alexl@redhat.com> 1.0.1-15
- Fixed the default font patch. It crashed on 64bit arch.

* Tue Aug 14 2001 Alexander Larsson <alexl@redhat.com> 1.0.1-13
- Fixed EelScalableFont to not keep reloading fonts
- all the time.

* Fri Aug  3 2001 Owen Taylor <otaylor@redhat.com>
- Fix problems with EelImageChooser widget and Japanese

* Fri Jul 27 2001 Alexander Larsson <alexl@redhat.com>
- Get some fixes from CVS head, one that segfaulted ia64.
- This also moves the include file into a eel-1 dir, so that
- it can later coexist with eel 2.0.

* Tue Jul 24 2001 Owen Taylor <otaylor@redhat.com>
- Fixes for efficiency of background drawing

* Tue Jul 24 2001 Akira TAGOH <tagoh@redhat.com> 1.0.1-7
- fixed typo in patch. oops.

* Mon Jul 23 2001 Akira TAGOH <tagoh@redhat.com> 1.0.1-6
- fixed choose the default font with every locale.

* Wed Jul 18 2001 Havoc Pennington <hp@redhat.com>
- own some directories we didn't before

* Sun Jul 08 2001 Tim Powers <timp@redhat.com>
- cleaned up files list so that the defattr is doing something
sensible and not leaving out the docs
- moved changelog to the end of the specfile

* Fri Jul 06 2001 Alexander Larsson <alla@redhat.com>
- Removed docdir and cleaned up specfile a bit.

* Fri Jul 06 2001 Alexander Larsson <alla@redhat.com>
- Updated to 1.0.1

* Wed May 09 2001 Jonathan Blandford <jrb@redhat.com>
- Add to Red Hat build system

* Wed Apr 04 2000 Ramiro Estrugo <ramiro@eazel.com>
- created this thing
