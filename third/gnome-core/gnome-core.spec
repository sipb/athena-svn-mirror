# Note that this is NOT a relocatable package
%define ver      	1.4.0.4
%define RELEASE		1
%define rel      	%{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}
%define localstatedir   /var/lib

Summary:         The core programs for the GNOME GUI desktop environment.
Name: 		 gnome-core
Version: 	 %ver
Release: 	 %rel
Copyright: 	 LGPL
Group: 		 System Environment/Base
Source:          ftp://ftp.gnome.org/pub/sources/gnome-core/gnome-core-%{ver}.tar.gz
BuildRoot: 	 /var/tmp/%{name}-%{version}-root
URL: 		 http://www.gnome.org
Prereq: 	 /sbin/install-info
Requires:        gtk+ >= 1.2.5, gdk-pixbuf >= 0.7.0
Requires:        libglade >= 0.14, libxml
Requires:        gnome-libs >= 1.0.59
Requires:        ORBit >= 0.5.0
Requires:        control-center >= 1.4.0
BuildRequires:   gtk+-devel >= 1.2.5, libxml-devel
BuildRequires:   gdk-pixbuf-devel >= 0.7.0
BuildRequires:   libglade-devel >= 0.14
BuildRequires:   scrollkeeper >= 0.1.4
BuildRequires:   gnome-libs-devel >= 1.0.59
BuildRequires:   ORBit-devel >= 0.5.0
BuildRequires:   control-center-devel >= 1.4.0


%description
GNOME (GNU Network Object Model Environment) is a user-friendly
set of applications and desktop tools to be used in conjunction with a
window manager for the X Window System.  GNOME is similar in purpose and
scope to CDE and KDE, but GNOME is based completely on free
software.  The gnome-core package includes the basic programs and
libraries that are needed to install GNOME.

You should install the gnome-core package if you would like to use the
GNOME desktop environment.  You'll also need to install the gnome-libs
package.  If you would like to develop GNOME applications, you'll also
need to install gnome-libs-devel.  If you want to use linuxconf with a
GNOME front end, you'll also need to install the gnome-linuxconf package.

%package devel
Summary:        GNOME core libraries, includes, and more.
Group: 		Development/Libraries
Requires: 	gnome-core
PreReq: 	/sbin/install-info

%description devel
Panel libraries and header files for creating GNOME panels.

%prep
%setup -q

%build
./configure --disable-gtkhtml-help --prefix=%{_prefix} \
    --bindir=%{_bindir} --mandir=%{_mandir} \
    --localstatedir=%{localstatedir} --libdir=%{_libdir} \
    --datadir=%{_datadir} --includedir=%{_includedir} \
    --sysconfdir=%{_sysconfdir}

CFLAGS="$RPM_OPT_FLAGS" make


%install
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

make prefix=$RPM_BUILD_ROOT%{_prefix} bindir=$RPM_BUILD_ROOT%{_bindir} \
    mandir=$RPM_BUILD_ROOT%{_mandir} libdir=$RPM_BUILD_ROOT%{_libdir} \
    localstatedir=$RPM_BUILD_ROOT%{localstatedir} \
    datadir=$RPM_BUILD_ROOT%{_datadir} \
    includedir=$RPM_BUILD_ROOT%{_includedir} \
    sysconfdir=$RPM_BUILD_ROOT%{_sysconfdir} install

 
%clean
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig
if which scrollkeeper-update>/dev/null 2>&1; then scrollkeeper-update; fi

%postun
/sbin/ldconfig
if which scrollkeeper-update>/dev/null 2>&1; then scrollkeeper-update; fi

%files
%doc AUTHORS COPYING ChangeLog NEWS README
%defattr (-, root, root)
%{_bindir}/*
%{_sysconfdir}/CORBA/servers/*
%{_sysconfdir}/sound/events/*
%{_datadir}/locale/*/*/*
%{_mandir}/man1/*
%{_mandir}/man5/*
%{_libdir}/*.so.*
%{_datadir}/applets/*
%{_datadir}/control-center/*
%{_datadir}/gnome/*
%{_datadir}/gnome-about/*
%{_datadir}/gnome-terminal/*
%{_datadir}/mc/*
%{_datadir}/omf/*
%{_datadir}/pixmaps/*

%files devel
%{_includedir}/*
%{_libdir}/*a
%{_libdir}/*so
%{_libdir}/*sh
%{_datadir}/idl/*


%changelog
* Thu Apr 02 2001 Gregory Leblanc <gleblanc@cu-portland.edu>
- Fixed %files section to include some directories as well as their contents.
- Updated to do the scrollkeeper ditty.

* Wed Mar 28 2001 Gregory Leblanc <gleblanc@cu-portland.edu>
- integrate configure.in and gnome-core.spec.in for version numbers of
  dependancies.  This should help to slow bit-rot in this spec file.

* Fri Mar 23 2001 Gregory Leblanc <gleblanc@cu-portland.edu>
- re-wrote the %files section from scratch, added a couple of
  dependancies.

* Wed Feb 21 2001 Gregory Leblanc <gleblanc@cu-portland.edu>
- updated, fixed macros, removed hard-coded paths.

* Sat Feb 26 2000 Gregory McLean <gregm@comstar.net>
- Updated to 1.1.4
- Autogenerate the %files section.

* Sat Oct 16 1999 Gregory McLean <gregm@comstar.net>
- Updated to 1.0.50
- Sorted the language specific stuff out.

* Sun Oct 03 1999 Gregory McLean <gregm@comstar.net>
- updated to 1.0.50
- Overhauled the %files section.

* Sat Nov 21 1998 Pablo Saratxaga <srtxg@chanae.alphanet.ch>

- Cleaned %files section
- added spanish and french translations for rpm

* Wed Sep 23 1998 Michael Fulbright <msf@redhat.com>
- Built 0.30 release

* Fri Mar 13 1998 Marc Ewing <marc@redhat.com>
- Integrate into gnome-core CVS source tree
