%define		gst_minver	0.8.0
%define		gstp_minver	0.8.0
%define		gstreamer	gstreamer

Name: 		nautilus-media
Version: 	0.8.1
Release: 	1
Summary: 	A Nautilus media package with views and thumbnailers.

Group: 		Libraries/Multimedia
License: 	LGPL
URL:		http://www.gnome.org/
Packager:	Thomas Vander Stichele <thomas at apestaart dot org>
Source: 	%{name}-%{version}.tar.gz
BuildRoot: 	%{_tmppath}/%{name}-root

Requires: 	%{gstreamer} >= 0.8.0
Requires: 	%{gstreamer}-plugins >= 0.8.0
Requires:	GConf2

BuildRequires:	%{gstreamer}-devel >= 0.8.0
BuildRequires:	%{gstreamer}-plugins-devel >= 0.8.0

BuildRequires:  nautilus2-devel
BuildRequires:	eel2-devel

%description
This package contains a Nautilus view for audio using GStreamer, as well
as an audio property page and a video thumbnailer.

#%package -n nautilus-test-view
#Summary:	Nautilus test view, only for educational purposes.
#Group:		Libraries/Multimedia
#
#%description -n nautilus-test-view
#This package contains a Nautilus test view which doesn't do anything useful
#and is only meant for educational use.

%prep
%setup -q

%build
%configure

%install
rm -rf $RPM_BUILD_ROOT
export GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL=1

%makeinstall
unset GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL

%find_lang nautilus-media

# clean up unpackaged files
rm -f $RPM_BUILD_ROOT%{_libdir}/*.la
rm -f $RPM_BUILD_ROOT%{_libdir}/bonobo/*.la
rm -f $RPM_BUILD_ROOT%{_libdir}/bonobo/*.a

%clean
rm -rf $RPM_BUILD_ROOT

%post
export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
gconftool-2 --makefile-install-rule %{_sysconfdir}/gconf/schemas/gst-thumbnail.schemas > /dev/null

%files -f nautilus-media.lang
%defattr(-, root, root)
%doc AUTHORS COPYING README ChangeLog
%{_bindir}/gst-thumbnail
%{_libdir}/bonobo/servers/*
%{_libdir}/bonobo/libnautilus-audio-properties-view.so
%{_libexecdir}/nautilus-audio-view
%{_datadir}/pixmaps/%{name}
%{_datadir}/nautilus/glade/audio-properties-view.glade
%{_datadir}/gnome-2.0/ui/nautilus-audio-view-ui.xml
%{_sysconfdir}/gconf/schemas/gst-thumbnail.schemas

#%files -n nautilus-test-view
#%defattr(-, root, root)
#%{_libexecdir}/nautilus-test-view
#%{_datadir}/gnome-2.0/ui/nautilus-test-view-ui.xml

%changelog
* Tue Mar 30 2004 Thomas Vander Stichele <thomas at apestaart dot org>
- More cleanups

* Sun Feb 29 2004 Christian Schaller <Uraeus@gnome.org>
- Add versioning variable to build against versioned packages

* Wed Jan 22 2003 Thomas Vander Stichele <thomas at apestaart dot org>
- conditionalize test view package
- add gconf stuff for thumbnailers

* Fri Oct 25 2002 Thomas Vander Stichele <thomas@apestaart.org>
- initial spec file
