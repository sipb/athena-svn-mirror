Summary:         The core programs for the GNOME GUI desktop environment.
Name:            gnome-panel
Version:         2.0.11
Release:         1_CVS_1
License:         LGPL
Group:           System Environment/Base
Source:          %{name}-%{version}.tar.gz
BuildRoot:       %{_tmppath}/%{name}-%{version}-root
URL:             http://www.gnome.org
Requires:        ORBit2 >= 2.4.0
Requires:        gtk2 >= 2.0.3
Requires:        libgnomeui >= @LIBGNOMEUI_REQUIRED@
Requires:        libwnck >= 0.13
Requires:        gnome-desktop >= @LIBGNOME_DESKTOP_REQUIRED@
Requires:        libglade2 >= 2.0.0
Requires:        gnome-vfs2 >= 1.9.16
BuildRequires:   pkgconfig >= 0.8
BuildRequires:   ORBit2-devel >= 2.4.0
BuildRequires:   gtk2-devel >= 2.0.3
BuildRequires:   libgnomeui-devel >= @LIBGNOMEUI_REQUIRED@
BuildRequires:   libwnck-devel >= 0.13
BuildRequires:   libglade2-devel >= 2.0.0
BuildRequires:   gnome-vfs2-devel >= 1.9.16
BuildRequires:   gnome-desktop-devel >= @LIBGNOME_DESKTOP_REQUIRED@

%description
GNOME (GNU Network Object Model Environment) is a user-friendly
set of applications and desktop tools to be used in conjunction with a
window manager for the X Window System.  GNOME is similar in purpose and
scope to CDE and KDE, but GNOME is based completely on free
software.  The gnome-core package includes the basic programs and
libraries that are needed to install GNOME.

The GNOME panel packages provides the gnome panel, menu's and some
basic applets for the panel.

%package devel
Summary:    GNOME panel libraries, includes, and more.
Group:      Development/Libraries
Requires:   %name = %version
Requires:   pkgconfig >= 0.8
Requires:   ORBit2-devel >= 2.4.0
Requires:   gtk2-devel >= 2.0.3
Requires:   libgnomeui-devel >= @LIBGNOMEUI_REQUIRED@
Requires:   gnome-desktop-devel >= @LIBGNOME_DESKTOP_REQUIRED@
Requires:   libwnck-devel >= 0.13
Requires:   libglade2-devel >= 2.0.0
Requires:   gnome-vfs2-devel >= 1.9.16


%description devel
Panel libraries and header files for creating GNOME panels.

%prep
%setup -q

%build
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT

export GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL=1
%makeinstall
unset GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL

%find_lang %name

%clean
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig
export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
for SCHEMA in %{_sysconfdir}/gconf/schemas/{mailcheck,pager,panel-global-config,panel-per-panel-config,fish,tasklist}.schemas; do
gconftool-2 --makefile-install-rule $SCHEMA > /dev/null 2>&1
done

%postun
/sbin/ldconfig

%files -f %name.lang
%defattr (-, root, root)
%doc AUTHORS COPYING ChangeLog NEWS README
%{_bindir}/*
%config %{_sysconfdir}/gconf/schemas/*
%doc %{_mandir}/man1/*
%{_libdir}/*.so.*
%{_libdir}/bonobo/servers
%{_datadir}/gnome/capplets
%{_datadir}/gnome/hints
%doc %{_datadir}/gnome/help
%{_datadir}/gnome/panel
%{_datadir}/gnome-panel/glade
%{_datadir}/idl/*
%{_datadir}/omf/*
%{_datadir}/pixmaps/*
%dir %{_datadir}/gnome

%files devel
%defattr (-, root, root)
%{_includedir}/panel-2.0
%{_libdir}/*a
%{_libdir}/*.so
%{_libdir}/pkgconfig/*


%changelog
* Tues Mar 12 2002 <glynn.foster@sun.com>
- fix up gconf schema install

* Mon Mar  4 2002  <gleblanc@linuxweasel.com>
- made into a proper .spec.in, using the magic version numbers and such

* Mon Feb 18 2002  <gleblanc@linuxweasel.com>
- flagged man pages as documentation

* Mon Feb 18 2002 Gregory Leblanc <gleblanc@linuxweasel.com> 
- remove extra tab from header
- added defattr to devel package
- moved defattr to make sure that it owns the random package docs
- moved the idl files into the main package, as when perl bindings arrive, they'll want to use them at run-time
- made it not own the omf dir
- moved the line for the GNOME help stuff back into the section with the rest of the regular files
- changed name of find_lang's output file
- removed some whitespace from the install section
- removed some tabs from the devel package headers
- make release number funky, so that people know it's a snapshot
- use auto* version
- group all BuildRequires together

* Fri Feb 15 2002 Chris Chabot <chabotc@reviewboard.com>
- initial spec file
- cleaned up header
- moved gnome/help to doc section

