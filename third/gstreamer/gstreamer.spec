Name: 		gstreamer
Version: 	0.5.2.3
Release: 	20030130_235633
Summary: 	GStreamer streaming media framework runtime.

Group: 		Libraries/Multimedia
License: 	LGPL
URL:		http://gstreamer.net/
Vendor:         GStreamer Backpackers Team <package@gstreamer.net>
Source: 	http://gstreamer.net/releases/%{version}/src/%{name}-%{version}.tar.gz
BuildRoot: 	%{_tmppath}/%{name}-%{version}-root

%define		majorminor	0.6
%define 	_glib2		2.0.1
%define 	_libxml2	2.4.0

Requires: 	glib2 >= %_glib2
Requires: 	libxml2 >= %_libxml2
Requires:	popt > 1.6
Prereq:		%{name}-tools >= %{version}
BuildRequires: 	glib2-devel >= %_glib2
BuildRequires: 	libxml2-devel >= %_libxml2
BuildRequires: 	bison
BuildRequires: 	flex
BuildRequires: 	gtk-doc >= 0.7
BuildRequires: 	gcc
BuildRequires: 	zlib-devel
BuildRequires:  popt > 1.6
Prereq:		/sbin/ldconfig

### documentation requirements
BuildRequires:  openjade
BuildRequires:  python2
BuildRequires:  docbook-style-dsssl docbook-dtd31-sgml
BuildRequires:	transfig xfig

%description
GStreamer is a streaming-media framework, based on graphs of filters which
operate on media data. Applications using this library can do anything
from real-time sound processing to playing videos, and just about anything
else media-related.  Its plugin-based architecture means that new data
types or processing capabilities can be added simply by installing new 
plugins.

%package devel
Summary: 	Libraries/include files for GStreamer streaming media framework.
Group: 		Development/Libraries

Requires: 	%{name} = %{version}-%{release}
Requires: 	glib2-devel >= %_glib2
Requires: 	libxml2-devel >= %_libxml2

%description devel
GStreamer is a streaming-media framework, based on graphs of filters which
operate on media data. Applications using this library can do anything
from real-time sound processing to playing videos, and just about anything
else media-related.  Its plugin-based architecture means that new data
types or processing capabilities can be added simply by installing new   
plugins.

This package contains the libraries and includes files necessary to develop
applications and plugins for GStreamer.

%package tools
Summary: 	tools for GStreamer streaming media framework.
Group: 		Libraries/Multimedia
Requires:	%{name}

%description tools
GStreamer is a streaming-media framework, based on graphs of filters which
operate on media data. Applications using this library can do anything
from real-time sound processing to playing videos, and just about anything
else media-related.  Its plugin-based architecture means that new data
types or processing capabilities can be added simply by installing new   
plugins.

This package contains the basic command-line tools used for GStreamer, like
gst-register and gst-launch.  It is split off to allow parallel-installability
in the future.

%prep
%setup

%build
CFLAGS="${CFLAGS:-%optflags}" ; export CFLAGS ; \
CXXFLAGS="${CXXFLAGS:-%optflags}" ; export CXXFLAGS ; \
FFLAGS="${FFLAGS:-%optflags}" ; export FFLAGS ; \
%{?__libtoolize:[ -f configure.in ] && %{__libtoolize} --copy --force} ; \
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
  --infodir=%{_infodir} \
  --enable-debug \
  --with-cachedir=%{_localstatedir}/cache/gstreamer-%{majorminor} \
  --disable-tests --disable-examples \
  --enable-docs-build --with-html-dir=$RPM_BUILD_ROOT%{_datadir}/gtk-doc/html

if [ "$SMP" != "" ]; then
  (make "MAKE=make -k -j $SMP"; exit 0)
else
  make
fi

%install  
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT
# adding devhelp stuff here for now, need to integrate better
# when devhelp allows it
mkdir -p $RPM_BUILD_ROOT/usr/share/devhelp/specs
cp $RPM_BUILD_DIR/%{name}-%{version}/docs/devhelp/*.devhelp $RPM_BUILD_ROOT/usr/share/devhelp/specs

%makeinstall
# Clean out files that should not be part of the rpm. 
# This is the recommended way of dealing with it for RH8
mkdir -p $RPM_BUILD_ROOT%{_localstatedir}/cache/gstreamer-%{majorminor}
rm -f $RPM_BUILD_ROOT%{_libdir}/%{name}-%{majorminor}/*.la
rm -f $RPM_BUILD_ROOT%{_libdir}/%{name}-%{majorminor}/*.a
rm -f $RPM_BUILD_ROOT%{_libdir}/*.a
rm -f $RPM_BUILD_ROOT%{_libdir}/*.la

%clean
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null

%post devel
# adding devhelp links to work around different base not working
mkdir -p %{_datadir}/devhelp/books
ln -sf %{_datadir}/gtk-doc/html/gstreamer %{_datadir}/devhelp/books
ln -sf %{_datadir}/gtk-doc/html/gstreamer-libs %{_datadir}/devhelp/books

%postun
/sbin/ldconfig

%files
%defattr(-, root, root)
%doc AUTHORS COPYING README TODO COPYING.LIB ABOUT-NLS REQUIREMENTS DOCBUILDING RELEASE 
%{_libdir}/libgstreamer-%{majorminor}.so.*
%{_libdir}/libgstcontrol-%{majorminor}.so.*
%dir %{_libdir}/gstreamer-%{majorminor}
%dir %{_localstatedir}/cache/gstreamer-%{majorminor}
%{_libdir}/gstreamer-%{majorminor}/libgstautoplugcache*.so*
%{_libdir}/gstreamer-%{majorminor}/libgstautoplugger*.so*
%{_libdir}/gstreamer-%{majorminor}/libgstbasicomega*.so*
%{_libdir}/gstreamer-%{majorminor}/libgstoptscheduler.so*
%{_libdir}/gstreamer-%{majorminor}/libgstoptomega*.so*
%{_libdir}/gstreamer-%{majorminor}/libgstbasicwingo*.so*
%{_libdir}/gstreamer-%{majorminor}/libgstoptwingoscheduler*.so
%{_libdir}/gstreamer-%{majorminor}/libgstelements*.so*
%{_libdir}/gstreamer-%{majorminor}/libgsttypes*.so*
%{_libdir}/gstreamer-%{majorminor}/libgststaticautoplug*.so*
%{_libdir}/gstreamer-%{majorminor}/libgstbytestream*.so*
%{_libdir}/gstreamer-%{majorminor}/libgstgetbits*.so*
%{_libdir}/gstreamer-%{majorminor}/libgstputbits*.so*
%{_libdir}/gstreamer-%{majorminor}/libgstspider*.so*
%{_libdir}/gstreamer-%{majorminor}/libgstindexers.so

%files tools
%{_bindir}/gst-complete
%{_bindir}/gst-compprep
%{_bindir}/gst-inspect
%{_bindir}/gst-launch
%{_bindir}/gst-md5sum
%{_bindir}/gst-register
%{_bindir}/gst-feedback
%{_bindir}/gst-xmllaunch
%{_mandir}/man1/gst-feedback.*
%{_mandir}/man1/gst-xmllaunch.*
%{_mandir}/man1/gst-complete.*
%{_mandir}/man1/gst-compprep.*
%{_mandir}/man1/gst-inspect.*
%{_mandir}/man1/gst-launch.*
%{_mandir}/man1/gst-md5sum.*
%{_mandir}/man1/gst-register.*


%files devel
%defattr(-, root, root)
%dir %{_includedir}/%{name}-%{majorminor}
%dir %{_includedir}/%{name}-%{majorminor}/gst
%{_includedir}/%{name}-%{majorminor}/gst/*.h
%dir %{_includedir}/%{name}-%{majorminor}/gst/control
%{_includedir}/%{name}-%{majorminor}/gst/control/*.h
%dir %{_includedir}/%{name}-%{majorminor}/gst/bytestream
%{_includedir}/%{name}-%{majorminor}/gst/bytestream/bytestream.h
%dir %{_includedir}/%{name}-%{majorminor}/gst/getbits
%{_includedir}/%{name}-%{majorminor}/gst/getbits/getbits.h
%dir %{_includedir}/%{name}-%{majorminor}/gst/putbits
%{_includedir}/%{name}-%{majorminor}/gst/putbits/putbits.h
# %{_libdir}/libgstreamer.a
%{_libdir}/libgstreamer-%{majorminor}.so
%{_libdir}/libgstcontrol-%{majorminor}.so
%{_libdir}/pkgconfig/gstreamer-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-control-%{majorminor}.pc
## we specify the API docs as regular files since %docs doesn't fail when
#  files aren't found anymore for RPM >= 4
#  we list all of the files we really need to trap incomplete doc builds
#  then we catch the rest with *, you can safely ignore the errors from this
## gstreamer API
%dir %{_datadir}/gtk-doc/html/%{name}-%{majorminor}
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/autopluggers.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/book1.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/element-types.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstautoplugfactory.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstautoplug.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstbin.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstclock.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstelement.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gst-index.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstobject.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstpad.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstpipeline.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstpluginfeature.html
# %{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstreamer-cothreads.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstreamer-gstbuffer.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstreamer-gstbufferpool.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstreamer-gstcaps.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstreamer-gstcpu.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstreamer-gstdata.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstreamer-gstevent.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstreamer-gst.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstreamer-gstinfo.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstreamer-gstparse.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstreamer-gstplugin.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstreamer-gstprops.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstreamer-gststaticautoplug.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstreamer-gststaticautoplugrender.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstreamer-gsttype.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstreamer-gstutils.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstreamer.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstschedulerfactory.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstscheduler.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstthread.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gsttypefactory.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/gstxml.html
%{_datadir}/gtk-doc/html/%{name}-%{majorminor}/index.sgml
## gstreamer-libs API
%dir %{_datadir}/gtk-doc/html/%{name}-libs-%{majorminor}
%{_datadir}/gtk-doc/html/%{name}-libs-%{majorminor}/book1.html
%{_datadir}/gtk-doc/html/%{name}-libs-%{majorminor}/%{name}-libs-gstcolorspace.html
%{_datadir}/gtk-doc/html/%{name}-libs-%{majorminor}/%{name}-libs-gstgetbits.html
%{_datadir}/gtk-doc/html/%{name}-libs-%{majorminor}/%{name}-libs.html
%{_datadir}/gtk-doc/html/%{name}-libs-%{majorminor}/index.sgml
## this catches all of the rest of the docs we might have forgotten
%{_datadir}/gtk-doc/html/*
%{_datadir}/devhelp/specs/%{name}-%{majorminor}.devhelp
%{_datadir}/devhelp/specs/%{name}-libs-%{majorminor}.devhelp


%changelog
* Tue Jan 28 2003 Thomas Vander Stichele <thomas at apestaart dot org>
- added gstreamer-control.pc file

* Sat Dec 07 2002 Thomas Vander Stichele <thomas at apestaart dot org>
- define majorminor and use it everywhere
- full parallel installability

* Tue Nov 05 2002 Christian Schaller <Uraeus@linuxrising.org>
- Add optwingo scheduler

* Sat Oct 12 2002 Christian Schaller <Uraeus@linuxrising.org>
- Updated to work better with default RH8 rpm
- Added missing unspeced files
- Removed .a and .la files from buildroot

* Sat Sep 21 2002 Thomas Vander Stichele <thomas@apestaart.org>
- added gst-md5sum

* Tue Sep 17 2002 Thomas Vander Stichele <thomas@apestaart.org>
- adding flex to buildrequires

* Fri Sep 13 2002 Christian F.K. Schaller <Uraeus@linuxrising.org>
- Fixed the schedulers after the renaming
* Sun Sep 08 2002 Thomas Vander Stichele <thomas@apestaart.org>
- added transfig to the BuildRequires:

* Sat Jun 22 2002 Thomas Vander Stichele <thomas@apestaart.org>
- moved header location

* Mon Jun 17 2002 Thomas Vander Stichele <thomas@apestaart.org>
- added popt
- removed .la

* Fri Jun 07 2002 Thomas Vander Stichele <thomas@apestaart.org>
- added release of gstreamer to req of gstreamer-devel
- changed location of API docs to be in gtk-doc like other gtk-doc stuff
- reordered SPEC file

* Mon Apr 29 2002 Thomas Vander Stichele <thomas@apestaart.org>
- moved html docs to gtk-doc standard directory

* Tue Mar 5 2002 Thomas Vander Stichele <thomas@apestaart.org>
- move version defines of glib2 and libxml2 to configure.ac
- add BuildRequires for these two libs

* Sun Mar 3 2002 Thomas Vander Stichele <thomas@apestaart.org>
- put html docs in canonical place, avoiding %doc erasure
- added devhelp support, current install of it is hackish

* Sat Mar 2 2002 Christian Schaller <Uraeus@linuxrising.org>
- Added documentation to build

* Mon Feb 11 2002 Thomas Vander Stichele <thomas@apestaart.org>
- added libgstbasicscheduler
- renamed libgst to libgstreamer

* Fri Jan 04 2002 Christian Schaller <Uraeus@linuxrising.org>
- Added configdir parameter as it seems the configdir gets weird otherwise

* Thu Jan 03 2002 Thomas Vander Stichele <thomas@apestaart.org>
- split off gstreamer-editor from core
- removed gstreamer-gnome-apps

* Sat Dec 29 2001 Rodney Dawes <dobey@free.fr>
- Cleaned up the spec file for the gstreamer core/plug-ins split
- Improve spec file

* Sat Dec 15 2001 Christian Schaller <Uraeus@linuxrising.org>
- Split of more plugins from the core and put them into their own modules
- Includes colorspace, xfree and wav
- Improved package Require lines
- Added mp3encode (lame based) to the SPEC

* Wed Dec 12 2001 Christian Schaller <Uraeus@linuxrising.org>
- Thomas merged mpeg plugins into one
* Sat Dec 08 2001 Christian Schaller <Uraeus@linuxrising.org>
- More minor cleanups including some fixed descriptions from Andrew Mitchell

* Fri Dec 07 2001 Christian Schaller <Uraeus@linuxrising.org>
- Added logging to the make statement

* Wed Dec 05 2001 Christian Schaller <Uraeus@linuxrising.org>
- Updated in preparation for 0.3.0 release

* Fri Jun 29 2001 Christian Schaller <Uraeus@linuxrising.org>
- Updated for 0.2.1 release
- Split out the GUI packages into their own RPM
- added new plugins (FLAC, festival, quicktime etc.)

* Sat Jun 09 2001 Christian Schaller <Uraeus@linuxrising.org>
- Visualisation plugins bundled out togheter
- Moved files sections up close to their respective descriptions

* Sat Jun 02 2001 Christian Schaller <Uraeus@linuxrising.org>
- Split the package into separate RPMS, 
  putting most plugins out by themselves.

* Fri Jun 01 2001 Christian Schaller <Uraeus@linuxrising.org>
- Updated with change suggestions from Dennis Bjorklund

* Tue Jan 09 2001 Erik Walthinsen <omega@cse.ogi.edu>
- updated to build -devel package as well

* Sun Jan 30 2000 Erik Walthinsen <omega@cse.ogi.edu>
- first draft of spec file
