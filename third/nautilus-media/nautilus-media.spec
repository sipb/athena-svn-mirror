Name: 		nautilus-media
Version: 	0.2.0
Release: 	1
Summary: 	A Nautilus media package with views and thumbnailers.

Group: 		Libraries/Multimedia
License: 	LGPL
URL:		http://www.gnome.org/
Packager:	Thomas Vander Stichele <thomas at apestaart dot org>
Source: 	%{name}-%{version}.tar.gz
BuildRoot: 	%{_tmppath}/%{name}-root

# spec file trickery, please don't look
%{expand:%%define buildforrh7 %(A=$(awk '{print $5}' /etc/redhat-release); if [ "$A" = 7.2 -o "$A" = 7.3 ]; then echo 1; else echo 0; fi)}
%{expand:%%define buildforrh8 %(A=$(awk '{print $5}' /etc/redhat-release); if [ "$A" = 8.0 -o "$A" = 8.1 ]; then echo 1; else echo 0; fi)}

Requires: 	gstreamer >= 0.5.2
Requires: 	gstreamer-plugins >= 0.5.2
Requires: 	gstreamer-audio-effects >= 0.5.2
Requires: 	gstreamer-audio-formats >= 0.5.2
Requires:	gstreamer-vorbis
Requires:	gstreamer-GConf
Requires:	gstreamer-plugins
Requires:	GConf2

BuildRequires:	gstreamer-devel >= 0.5.2
BuildRequires:	gstreamer-plugins-devel >= 0.5.2

%if %buildforrh8
%{echo: Building for Red Hat 8.x}
BuildRequires:	nautilus
%endif

%if %buildforrh7
%{echo: Building for Red Hat 7.x}
BuildRequires:  nautilus2-devel
%endif
BuildRequires:	eel2-devel

%description
This package contains a Nautilus view for audio using GStreamer.

#%package -n nautilus-test-view
#Summary:	Nautilus test view, only for educational purposes.
#Group:		Libraries/Multimedia
#
#%description -n nautilus-test-view
#This package contains a Nautilus test view which doesn't do anything useful
#and is only meant for educational use.

%prep
%setup -n %{name}-%{version}
%build
CFLAGS="${CFLAGS:-%optflags}" ; export CFLAGS ; \
CXXFLAGS="${CXXFLAGS:-%optflags}" ; export CXXFLAGS ; \
FFLAGS="${FFLAGS:-%optflags}" ; export FFLAGS ; \
## not doing the libtoolize thing because we don't really need it
## note that we have configure.in because of intltoolize needing it
## and thus libtoolize gets triggered
## %{?__libtoolize:[ -f configure.in ] && %{__libtoolize} --copy --force} ; \
./configure \
  --prefix=%{_prefix} \
  --exec-prefix=%{_exec_prefix} \
  --bindir=%{_bindir} \
  --sbindir=%{_sbindir} \
  --sysconfdir=%{_sysconfdir} \
  --datadir=%{_datadir} \
  --includedir=%{_includedir} \
  --libdir=%{_libdir} \
  --libexecdir=%{_libexecdir} \
  --localstatedir=%{_localstatedir} \
  --sharedstatedir=%{_sharedstatedir} \
  --mandir=%{_mandir} \
  --infodir=%{_infodir}

%install
export GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL=1

%makeinstall
unset GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL

# clean up unpackaged files
rm -f $RPM_BUILD_ROOT%{_libdir}/*.la
rm -f $RPM_BUILD_ROOT%{_libdir}/bonobo/*.la
rm -f $RPM_BUILD_ROOT%{_libdir}/bonobo/*.a

%clean
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

%post
export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
gconftool-2 --makefile-install-rule %{_sysconfdir}/gconf/schemas/gst-thumbnail.schemas > /dev/null

%files
%defattr(-, root, root)
%doc AUTHORS COPYING README ChangeLog
%{_bindir}/gst-thumbnail
%{_libdir}/bonobo/servers/*
%{_libdir}/bonobo/libnautilus-audio-properties-view.so
%{_libexecdir}/nautilus-audio-view
%{_datadir}/pixmaps/%{name}
%{_datadir}/nautilus/glade/audio-properties-view.glade
%{_datadir}/gnome-2.0/ui/nautilus-audio-view-ui.xml
%{_datadir}/locale/*/LC_MESSAGES/nautilus-media.mo
%{_sysconfdir}/gconf/schemas/gst-thumbnail.schemas
#%{_libdir}/libgstmedia-info.a
#%{_libdir}/libgstmedia-info.so.0.0.0

#%files -n nautilus-test-view
#%defattr(-, root, root)
#%{_libexecdir}/nautilus-test-view
#%{_datadir}/gnome-2.0/ui/nautilus-test-view-ui.xml

%changelog
* Wed Jan 22 2003 Thomas Vander Stichele <thomas at apestaart dot org>
- conditionalize test view package
- add gconf stuff for thumbnailers

* Fri Oct 25 2002 Thomas Vander Stichele <thomas@apestaart.org>
- initial spec file
