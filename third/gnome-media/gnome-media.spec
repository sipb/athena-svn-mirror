%define glib2_version 2.0.3
%define pango_version 1.0.99
%define gtk2_version 2.0.5
%define libgnomeui_version 2.0.0
%define gail_version 0.17-2
%define desktop_file_utils_version 0.2.90

%define gettext_package gnome-media-2.0

Summary:        GNOME media programs.
Name:           gnome-media
Version:        2.2.0
Release:        9
Copyright:      GPL
Group:          Applications/Multimedia
Source:         ftp://ftp.gnome.org/pub/GNOME/sources/pre-gnome2/gnome-media/gnome-media-%{version}.tar.gz
Prereq:         scrollkeeper >= 0.1.4
BuildPrereq:    scrollkeeper intltool
BuildRoot:      %{_tmppath}/%{name}-%{PACKAGE_VERSION}-root
Obsoletes:      gnome
URL:            http://www.gnome.org

BuildRequires:  glib2-devel >= %{glib2_version}
BuildRequires:  pango-devel >= %{pango_version}
BuildRequires:  gtk2-devel >= %{gtk2_version}
BuildRequires:  libgnomeui-devel >= %{libgnomeui_version}
BuildRequires:  gail-devel >= %{gail_version}
BuildRequires:  Xft
BuildRequires:  fontconfig
BuildRequires:  desktop-file-utils >= %{desktop_file_utils_version}
BuildRequires:  /usr/bin/automake-1.4
Requires:	gstreamer >= 0.4.2
Requires:	scrollkeeper >= 0.3.8
%description
GNOME (GNU Network Object Model Environment) is a user-friendly set of
GUI applications and desktop tools to be used in conjunction with a
window manager for the X Window System. The gnome-media package will
install media features like the GNOME CD player.

Install gnome-media if you want to use GNOME's multimedia
capabilities.

%prep
%setup

%build
%configure
make

%install
rm -rf $RPM_BUILD_ROOT

export GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL=1
%makeinstall
unset GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL

# Clean out files that should not be part of the rpm.
# This is the recommended way of dealing with it for RH8
rm -f $RPM_BUILD_ROOT%{_libdir}/*.a
rm -f $RPM_BUILD_ROOT%{_libdir}/*.la
rm -rf $RPM_BUILD_ROOT/var/scrollkeeper/*
rm -f $RPM_BUILD_ROOT/%{_datadir}/pixmaps/gnome-cd/*.png

%find_lang %{gettext_package}

%clean
rm -rf $RPM_BUILD_ROOT

%post
scrollkeeper-update -q
export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
SCHEMAS="CDDB-Slave2.schemas gnome-volume-control.schemas gnome-cd.schemas gnome-sound-recorder.schemas"
for S in $SCHEMAS; do
  gconftool-2 --makefile-install-rule %{_sysconfdir}/gconf/schemas/$S > /dev/null
done
/sbin/ldconfig

%postun
scrollkeeper-update
/sbin/ldconfig
/bin/true ## for rpmlint, -p requires absolute path and is just dumb

%files -f %{gettext_package}.lang
%defattr(-, root, root)
%doc AUTHORS COPYING ChangeLog NEWS README
%{_prefix}/libexec/*
%{_datadir}/idl/GNOME_Media_CDDBSlave2.idl
%{_datadir}/applications/gnome-cd.desktop
%{_datadir}/applications/gnome-sound-recorder.desktop
%{_datadir}/applications/gnome-volume-control.desktop
%{_datadir}/applications/reclevel.desktop
%{_datadir}/applications/vumeter.desktop
%{_datadir}/pixmaps/gnome-cd.png
%{_datadir}/pixmaps/gnome-cd/themes/lcd/*.png
%{_datadir}/pixmaps/gnome-cd/themes/lcd/lcd.theme
%{_datadir}/pixmaps/gnome-cd/themes/media/*.png
%{_datadir}/pixmaps/gnome-cd/themes/media/media.theme
%{_datadir}/pixmaps/gnome-cd/themes/red-lcd/*.png
%{_datadir}/pixmaps/gnome-cd/themes/red-lcd/red-lcd.theme
%{_datadir}/pixmaps/gnome-grecord.png
%{_datadir}/pixmaps/gnome-mixer.png
%{_datadir}/pixmaps/gnome-reclevel.png
%{_datadir}/pixmaps/gnome-vumeter.png
%{_datadir}/pixmaps/gnome-media
%{_datadir}/omf/gnome-media
%{_datadir}/gnome/help/gnome-cd
%{_datadir}/gnome/help/gnome-volume-control
%{_datadir}/gnome/help/grecord
%{_datadir}/gnome-sound-recorder/ui/gsr.xml
%{_datadir}/control-center-2.0/capplets/cddb-slave.desktop
%{_libdir}/*.so.*
%{_libdir}/bonobo/servers/GNOME_Media_CDDBSlave2.server
%{_bindir}/cddb-slave2-properties
%{_bindir}/gnome-cd
%{_bindir}/gnome-sound-recorder
%{_bindir}/gnome-volume-control
%{_bindir}/vumeter
%{_sysconfdir}/gconf/schemas/*.schemas

# devel, if we had a devel
%{_includedir}/*
%{_libdir}/*.so

%changelog
* Wed Nov 06 2002 Christian Schaller <Uraeus@gnome.org>
- Clean up files listing
- Add some RPM pre-req
- Add -q to scrollkeeper command

* Wed Oct 23 2002 Christian Schaller <Uraeus@gnome.org>
- Update for use in CVS package
- I remove all the stuff installed into /var/scrollkeeper this is probably a bugwhich I have no idea how to fix
