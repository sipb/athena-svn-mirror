Name: 		gstreamer-plugins
Version: 	0.6.0
Release: 	1
Summary: 	GStreamer Streaming-media framework plug-ins.

%define 	majorminor	0.6
#%define 	prefix  /usr
#%define 	sysconfdir /etc
#Docdir: 	%{prefix}/share/doc
#Prefix: 	%prefix

Group: 		Libraries/Multimedia
License: 	LGPL
URL:		http://gstreamer.net/
Vendor:         GStreamer Backpackers Team <package@gstreamer.net>
Source:         http://gstreamer.net/releases/%{version}/src/gst-plugins-%{version}.tar.gz
BuildRoot: 	%{_tmppath}/%{name}-%{version}-root

%define         _glib2          1.3.12

Requires:       glib2 >= %_glib2
BuildRequires:  glib2-devel >= %_glib2
Requires: 	gstreamer >= 0.5.1
BuildRequires: 	nasm => 0.90
BuildRequires: 	gstreamer-devel >= 0.5.1
BuildRequires:	gstreamer-tools >= 0.5.1
Obsoletes:	gstreamer-plugin-libs

%description
GStreamer is a streaming-media framework, based on graphs of filters which
operate on media data. Applications using this library can do anything
from real-time sound processing to playing videos, and just about anything
else media-related.  Its plugin-based architecture means that new data
types or processing capabilities can be added simply by installing new
plug-ins.

%prep
%setup -n gst-plugins-%{version}
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
  --enable-DEBUG 

if [ "$SMP" != "" ]; then
  (make "MAKE=make -k -j $SMP"; exit 0)
  make 2>&1 | tee make.log
else
  make 2>&1 | tee make.log
fi

%install
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT
export GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL=1
make prefix=%{?buildroot:%{buildroot}}%{_prefix} \
     exec_prefix=%{?buildroot:%{buildroot}}%{_exec_prefix} \
     bindir=%{?buildroot:%{buildroot}}%{_bindir} \
     sbindir=%{?buildroot:%{buildroot}}%{_sbindir} \
     sysconfdir=%{?buildroot:%{buildroot}}%{_sysconfdir} \
     datadir=%{?buildroot:%{buildroot}}%{_datadir} \
     includedir=%{?buildroot:%{buildroot}}%{_includedir} \
     libdir=%{?buildroot:%{buildroot}}%{_libdir} \
     libexecdir=%{?buildroot:%{buildroot}}%{_libexecdir} \
     localstatedir=%{?buildroot:%{buildroot}}%{_localstatedir} \
     sharedstatedir=%{?buildroot:%{buildroot}}%{_sharedstatedir} \
     mandir=%{?buildroot:%{buildroot}}%{_mandir} \
     infodir=%{?buildroot:%{buildroot}}%{_infodir} \
  install
unset GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL

# Clean out files that should not be part of the rpm.
# This is the recommended way of dealing with it for RH8
rm -f $RPM_BUILD_ROOT%{_libdir}/gstreamer-%{majorminor}/*.la
rm -f $RPM_BUILD_ROOT%{_libdir}/gstreamer-%{majorminor}/*.a
rm -f $RPM_BUILD_ROOT%{_libdir}/*.a
rm -f $RPM_BUILD_ROOT%{_libdir}/*.la
rm -f $RPM_BUILD_ROOT%{_includedir}/gstreamer-%{majorminor}/gst/media-info/media-info.h
rm -f $RPM_BUILD_ROOT%{_libdir}/libgstmedia-info*.so.0.0.0


%clean
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%doc AUTHORS COPYING README RELEASE REQUIREMENTS
%{_bindir}/gst-launch-ext
%{_bindir}/gst-visualise
%{_mandir}/man1/gst-launch-ext.*
%{_mandir}/man1/gst-visualise.1.*
%{_libdir}/gstreamer-%{majorminor}/libgstaudioscale.so
%{_libdir}/gstreamer-%{majorminor}/libgstaudio.so
%{_libdir}/gstreamer-%{majorminor}/libgstidct.so
%{_libdir}/gstreamer-%{majorminor}/libgstresample.so
%{_libdir}/gstreamer-%{majorminor}/libgstriff.so
%{_libdir}/gstreamer-%{majorminor}/libgstvideo.so

%package -n gstreamer-plugins-devel
Summary: 	GStreamer Plugin Library Headers.
Group: 		Development/Libraries
Requires: 	gstreamer-plugins = %{version}
Provides:	gstreamer-play-devel = %{version}
Provides:	gstreamer-gconf-devel = %{version}

%description -n gstreamer-plugins-devel
GStreamer support libraries header files.

%files -n gstreamer-plugins-devel
%defattr(-, root, root)
%{_includedir}/gstreamer-%{majorminor}/gst/gconf/gconf.h
%{_includedir}/gstreamer-%{majorminor}/gst/play/play.h
%{_includedir}/gstreamer-%{majorminor}/gst/audio/audio.h
%{_includedir}/gstreamer-%{majorminor}/gst/floatcast/floatcast.h
%{_includedir}/gstreamer-%{majorminor}/gst/idct/idct.h
%{_includedir}/gstreamer-%{majorminor}/gst/resample/resample.h
%{_includedir}/gstreamer-%{majorminor}/gst/riff/riff.h
%{_includedir}/gstreamer-%{majorminor}/gst/video/video.h
%{_datadir}/aclocal/gst-element-check-%{majorminor}.m4
%{_libdir}/pkgconfig/gstreamer-libs-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-play-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-gconf-%{majorminor}.pc
%{_libdir}/libgstgconf-%{majorminor}.so
%{_libdir}/libgstplay-%{majorminor}.so

# Here are all the packages depending on external libs #

### A52DEC ###
%package -n gstreamer-a52dec
Summary:       GStreamer VOB decoder plug-in.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      a52dec >= 0.7.3
BuildRequires: a52dec-devel >= 0.7.3

%description -n gstreamer-a52dec
Plug-in for decoding of VOB files.

%files -n gstreamer-a52dec
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgsta52dec.so
%{_libdir}/gstreamer-%{majorminor}/libgstac3parse.so

%post -n gstreamer-a52dec
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-a52dec
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### AALIB ###
%package -n gstreamer-aalib
Summary:	GStreamer plug-in for Ascii-art output.
Group:		Libraries/Multimedia
Requires:	gstreamer-plugins = %{version}
Requires:	aalib >= 1.3
BuildRequires:	aalib-devel >= 1.3

%description -n gstreamer-aalib
Plug-in for viewing video in Ascii-art using aalib library.

%files -n gstreamer-aalib
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstaasink.so

%post -n gstreamer-aalib
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-aalib
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### ALSA ###
#%package -n gstreamer-alsa
#Summary:  GStreamer plug-ins for the ALSA sound system.
#Group:    Libraries/Multimedia
#Requires: gstreamer-plugins = %{version}
#
#Provides:	gstreamer-audiosrc
#Provides:	gstreamer-audiosink
#
#%description -n gstreamer-alsa
#Input and output plug-in for the ALSA soundcard driver system. 
#This plug-in depends on Alsa 0.9.x or higher.
#
#%files -n gstreamer-alsa
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgstalsa.so
### #%{_mandir}/man1/gstalsa*
#
#%post -n gstreamer-alsa
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null
#
#%postun -n gstreamer-alsa
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### ARTS WRAPPER ###
%package -n gstreamer-arts
Summary:       GStreamer arts wrapper plug-in.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      kdelibs-sound >= 2
BuildRequires: kdelibs-sound-devel >= 2
BuildRequires: gcc-c++

%description -n gstreamer-arts
This plug-in wraps arts plug-ins.

%files -n gstreamer-arts
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstarts.so

%post -n gstreamer-arts
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-arts
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### ARTSD SOUND SERVER ###
%package -n gstreamer-artsd
Summary:  GStreamer artsd output plug-in.
Group:    Libraries/Multimedia
Requires: gstreamer-plugins = %{version}

%description -n gstreamer-artsd
Plug-in for outputting to artsd sound server.

%files -n gstreamer-artsd
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstartsdsink.so

%post -n gstreamer-artsd
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-artsd
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### SWFDEC FLASH PLUGIN ###
#%package -n gstreamer-swfdec
#Summary:  GStreamer Flash redering plug-in.
#Group:    Libraries/Multimedia
#Requires: gstreamer-plugins = %{version}
#Requires: swfdec => 0.1.2
#
#%description -n gstreamer-swfdec
#Plug-in for rendering Flash animations using swfdec library
#
#%files -n gstreamer-swfdec
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgstswfdec.so
#
#%post -n gstreamer-swfdec
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null
#
#%postun -n gstreamer-swfdec
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null


### AUDIOFILE ###
#%package -n gstreamer-audiofile
#Summary:       GStreamer plug-in for audiofile support.
#Group:         Libraries/Multimedia
#Requires:      gstreamer-plugins = %{version}
#Requires:      audiofile >= 0.2.1
#BuildRequires: audiofile-devel >= 0.2.1
#
#%description -n gstreamer-audiofile
#Plug-in for supporting reading and writing of all files supported by audiofile.
#
#%files -n gstreamer-audiofile
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgstaudiofile.so
#
#%post -n  gstreamer-audiofile
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null
#
#%postun -n  gstreamer-audiofile
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### AVI ###
%package -n gstreamer-avi
Summary:       GStreamer plug-in for AVI movie playback.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      gstreamer-colorspace = %{version}

%description -n gstreamer-avi
Plug-ins for playback of AVI format media files.

%files -n gstreamer-avi
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstavidemux.so
%{_libdir}/gstreamer-%{majorminor}/libgstavimux.so

%post -n gstreamer-avi
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-avi
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### AVIFILE ###
%package -n gstreamer-windec
Summary:       GStreamer plug-in for Windows DLL loading
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      avifile
BuildRequires: avifile-devel

%description -n gstreamer-windec
Plug-ins for playback for loading window DLL files. 
Needed for playback of some  AVI format media files.

%files -n gstreamer-windec
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstwincodec.so

%post -n gstreamer-windec
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-windec
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### CDPARANOIA ###
%package -n gstreamer-cdparanoia
Summary:       GStreamer plug-in for CD audio input using CDParanoia IV.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      cdparanoia-libs >= alpha9.7
BuildRequires: cdparanoia-devel >= alpha9.7

%description -n gstreamer-cdparanoia
Plug-in for ripping audio tracks using cdparanoia under GStreamer.

%files -n gstreamer-cdparanoia
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstcdparanoia.so

%post -n gstreamer-cdparanoia
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-cdparanoia
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### DVDREAD ###
%package -n gstreamer-libdvdread
Summary:       GStreamer plug-in for DVD playback.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      libdvdread >= 0.9.0
BuildRequires: libdvdread-devel >= 0.9.0
Obsoletes:     gstreamer-libdvd

%description -n gstreamer-libdvdread
Plug-in for reading DVDs using libdvdread under GStreamer.

%files -n gstreamer-libdvdread
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstdvdreadsrc.so

%post -n gstreamer-libdvdread
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-libdvdread
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### DVDNAV ###
%package -n gstreamer-libdvdnav
Summary:       GStreamer plug-in for DVD playback.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      libdvdnav >= 0.1.3
BuildRequires: libdvdnav-devel >= 0.1.3
Obsoletes:     gstreamer-libdvd

%description -n gstreamer-libdvdnav
Plug-in for reading DVDs using libdvdnav  under GStreamer.

%files -n gstreamer-libdvdnav
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstdvdnavsrc.so

%post -n gstreamer-libdvdnav
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-libdvdnav
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

## DXR3 ###
#%package -n gstreamer-dxr3
#Summary:       GStreamer plug-in for playback using dxr3 card.
#Group:         Libraries/Multimedia
#Requires:      gstreamer-plugins = %{version}
#Requires:      em8300 => 0.12.0
#BuildRequires: em8300-devel => 0.12.0
#
#%description -n gstreamer-dxr3
#Plug-in supporting DVD playback using cards
#with the dxr3 chipset like Hollywood Plus
#and Creative Labs DVD cards.
#
#%files -n gstreamer-dxr3
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgstdxr3.so
#
#%post -n gstreamer-dxr3
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null
#
#%postun -n gstreamer-dxr3
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### ESD ###
%package -n gstreamer-esound
Summary:       GStreamer plug-in for ESD sound output.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      esound >= 0.2.8
BuildRequires: esound-devel >= 0.2.8
Obsoletes:     gstreamer-esd

Provides:		gstreamer-audiosrc
Provides:		gstreamer-audiosink

%description -n gstreamer-esound
Output and monitoring plug-ins for GStreamer using ESound.

%files -n gstreamer-esound
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstesdmon.so
%{_libdir}/gstreamer-%{majorminor}/libgstesdsink.so

%post -n gstreamer-esound
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-esound
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### FLAC ###
%package -n gstreamer-flac
Summary:       GStreamer plug-in for FLAC lossless audio.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      flac >= 1.0.3
BuildRequires: flac-devel >= 1.0.3

%description -n gstreamer-flac
Plug-in for the free FLAC lossless audio format.

%files -n gstreamer-flac
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstflac.so

%post -n gstreamer-flac
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-flac
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### GNOME VFS 2 ###
%package -n gstreamer-gnomevfs
Summary:       GStreamer plug-ins for Gnome-VFS input and output.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      gnome-vfs2 > 1.9.4.00
BuildRequires: gnome-vfs2-devel > 1.9.4.00

%description -n gstreamer-gnomevfs
Plug-ins for reading and writing through GNOME VFS.

%files -n gstreamer-gnomevfs
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstgnomevfssrc.so
%{_libdir}/gstreamer-%{majorminor}/libgstgnomevfssink.so

%post -n gstreamer-gnomevfs
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-gnomevfs
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### GSM ###
#%package -n gstreamer-gsm
#Summary:       GStreamer plug-in for GSM lossy audio format.
#Group:         Libraries/Multimedia
#Requires:      gstreamer-plugins = %{version}
#Requires:      gsm >= 1.0.10
#BuildRequires: gsm-devel >= 1.0.10
#
#%description -n gstreamer-gsm
#Output plug-in for GStreamer to convert to GSM lossy audio format.
#
#%files -n gstreamer-gsm
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgstgsm.so
#
#%post -n gstreamer-gsm
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null
#
#%postun -n gstreamer-gsm
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### HERMES ###
%package -n gstreamer-colorspace
Summary:       GStreamer colorspace conversion plug-in.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      Hermes => 1.3.0
BuildRequires: Hermes-devel => 1.3.0
%description -n gstreamer-colorspace
Colorspace plug-in based on Hermes library.

%files -n gstreamer-colorspace
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstcolorspace.so

%post -n gstreamer-colorspace
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-colorspace
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### HTTP ###
#%package -n gstreamer-httpsrc
#Summary:       GStreamer plug-in for http using libghttp.
#Group:         Libraries/Multimedia
#Requires:      gstreamer-plugins = %{version}
#Requires:      libghttp => 1.0.9
#BuildRequires: libghttp-devel => 1.0.9
#
#%description -n gstreamer-httpsrc
#Plug-in supporting the http protocol based 
#on the libghttp library.
#
#%files -n gstreamer-httpsrc
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgsthttpsrc.so
#
#%post -n gstreamer-httpsrc
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null
#
#%postun -n gstreamer-httpsrc
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

#### JACK AUDIO CONNECTION KIT ###
#%package -n gstreamer-jack
#Summary:  GStreamer plug-in for the Jack Sound Server.
#Group:    Libraries/Multimedia
#Requires: gstreamer-plugins = %{version}
#Requires: jack-audio-connection-kit => 0.28.0
#
#Provides:	gstreamer-audiosrc
#Provides:	gstreamer-audiosink
#
#%description -n gstreamer-jack
#Plug-in for the JACK professional sound server.
#
#%files -n gstreamer-jack
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgstjack.so
#
#%post -n gstreamer-jack
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null
#
#%postun -n gstreamer-jack
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### JPEG ###
%package -n gstreamer-jpeg
Summary:       GStreamer plug-in for JPEG images.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      libjpeg
BuildRequires: libjpeg-devel

%description -n gstreamer-jpeg
Output plug-in for GStreamer using libjpeg.

%files -n gstreamer-jpeg
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstjpeg.so

%post -n gstreamer-jpeg
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-jpeg
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### LADSPA ###
%package -n gstreamer-ladspa
Summary:       GStreamer wrapper for LADSPA plug-ins.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      ladspa
BuildRequires: ladspa-devel

%description -n gstreamer-ladspa
Plug-in which wraps LADSPA plug-ins for use by GStreamer applications.
We suggest you also get the cmt package of ladspa plug-ins
and steve harris's swh-plugins package.

%files -n gstreamer-ladspa
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstladspa.so

%post -n gstreamer-ladspa
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-ladspa
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### LAME ###
%package -n gstreamer-lame
Summary:       GStreamer plug-in encoding mp3 songs using lame.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      lame >= 3.89
BuildRequires: lame-devel >= 3.89

%description -n gstreamer-lame
Plug-in for encoding mp3 with lame under GStreamer.

%files -n gstreamer-lame
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstlame.so

%post -n gstreamer-lame
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-lame
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### LIBDV ###
%package -n gstreamer-dv
Summary:       GStreamer DV plug-in.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      libdv >= 0.9.5
BuildRequires: libdv-devel >= 0.9.5

%description -n gstreamer-dv
Plug-in for digital video support using libdv.

%files -n gstreamer-dv
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstdvdec.so

%post -n gstreamer-dv
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-dv
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null


### LIBFAME ###
%package -n gstreamer-libfame
Summary:       GStreamer plug-in to encode MPEG1/MPEG4 video.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      libfame >= 0.9.0 
BuildRequires: libfame-devel >= 0.9.0 

%description -n gstreamer-libfame
Plug-in for encoding MPEG1/MPEG4 video using libfame.

%files -n gstreamer-libfame
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstlibfame.so

%post -n gstreamer-libfame
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-libfame
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### LIBPNG ###
%package -n gstreamer-libpng
Summary:       GStreamer plug-in to encode png images
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      libpng >= 1.2.0
BuildRequires: libpng-devel >= 1.2.0

%description -n gstreamer-libpng
Plug-in for encoding png images.

%files -n gstreamer-libpng
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstpng.so

%post -n gstreamer-libpng
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-libpng
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### MAD ###
%package -n gstreamer-mad  
Summary:       GStreamer plug-in using MAD for mp3 decoding.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      gstreamer-audio-formats
Requires:      mad >= 0.13.0
BuildRequires: mad-devel >= 0.13.0

%description -n gstreamer-mad
Plug-in for playback of mp3 songs using the very good MAD library.

%files -n gstreamer-mad
%defattr(-, root, root)  
%{_libdir}/gstreamer-%{majorminor}/libgstmad.so

%post -n gstreamer-mad
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-mad
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### MIKMOD ###
%package -n gstreamer-mikmod
Summary:       GStreamer Mikmod plug-in.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      mikmod
BuildRequires: mikmod

%description -n gstreamer-mikmod
Plug-in for playback of module files supported by mikmod under GStreamer.

%files -n gstreamer-mikmod
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstmikmod.so

%post -n gstreamer-mikmod
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-mikmod
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### MJPEGTOOLS ###
#%package -n gstreamer-jpegmmx
#Summary:       GStreamer mjpegtools plug-in for mmx jpeg.
#Group:         Libraries/Multimedia
#Requires:      gstreamer-plugins = %{version}
#Requires:      mjpegtools
#BuildRequires: mjpegtools-devel
#
#%description -n gstreamer-jpegmmx
#mjpegtools-based encoding and decoding plug-in.
#
#%files -n gstreamer-jpegmmx
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgstjpegmmxenc.so
#%{_libdir}/gstreamer-%{majorminor}/libgstjpegmmxdec.so
#
#%post -n gstreamer-jpegmmx
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null
#
#%postun -n gstreamer-jpegmmx
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### MPEG2DEC ###
%package -n gstreamer-mpeg
Summary:GStreamer plug-ins for MPEG video playback and encoding.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      mpeg2dec => 0.3.1
BuildRequires: mpeg2dec-devel => 0.3.1
Obsoletes:     gstreamer-mpeg1
Obsoletes:     gstreamer-mpeg2
Obsoletes:     gstreamer-mpeg2dec

%description -n gstreamer-mpeg
Plug-ins for playing and encoding MPEG video.

%files -n gstreamer-mpeg
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstmpeg1types.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpeg1encoder.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpeg1systemencode.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpegaudio.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpegaudioparse.so
%{_libdir}/gstreamer-%{majorminor}/libgstmp1videoparse.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpegstream.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpeg2enc.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpeg2dec.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpeg2subt.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpeg2types.so

%post -n  gstreamer-mpeg
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n  gstreamer-mpeg
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### OPENQUICKTIME ###
#%package -n gstreamer-openquicktime
#Summary:       GStreamer OpenQuicktime video Plug-in.
#Group:         Libraries/Multimedia
#Requires:      gstreamer-plugins = %{version}
#Requires:      openquicktime => 1.0
#BuildRequires: openquicktime-devel => 1.0
#
#%description -n gstreamer-openquicktime
#Plug-in which uses the OpenQuicktime library
#from 3ivx to play Quicktime movies.
#(http://openquicktime.sourceforge.net/)
#
#%files -n gstreamer-openquicktime
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgstopenquicktimedemux.so
#%{_libdir}/gstreamer-%{majorminor}/libgstopenquicktimetypes.so
#%{_libdir}/gstreamer-%{majorminor}/libgstopenquicktimedecoder.so
#
#%post -n gstreamer-openquicktime
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null
#
#%postun -n gstreamer-openquicktime
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### OSS ###
%package -n gstreamer-oss
Summary:       GStreamer plug-ins for input and output using OSS.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
BuildRequires: glibc-devel

Provides:		gstreamer-audiosrc
Provides:		gstreamer-audiosink

%description -n gstreamer-oss 
Plug-ins for output and input to the OpenSoundSytem audio
drivers found in the Linux kernels or commercially available
from OpenSound.

%files -n gstreamer-oss
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstossaudio.so
# %{_libdir}/gstreamer-%{majorminor}/libgstosshelper*

%post -n gstreamer-oss
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-oss
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### RAW1394 ###
#%package -n gstreamer-raw1394
#Summary:       GStreamer raw1394 Firewire plug-in.
#Group:         Libraries/Multimedia
#Requires:      gstreamer-plugins = %{version}
#Requires:      libraw1394
#BuildRequires: libraw1394-devel
#
#%description -n gstreamer-raw1394
#Plug-in for digital video support using raw1394.
#
#%files -n gstreamer-raw1394
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgst1394.so
#
#%post -n gstreamer-raw1394
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null
#
#%postun -n gstreamer-raw1394
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### RTP ###
#%package -n gstreamer-rtp
#Summary:  GStreamer RTP plug-in.
#Group:    Libraries/Multimedia
#Requires: gstreamer-plugins = %{version}
#Requires: librtp >= 0.1
#
#%description -n gstreamer-rtp
#Library for transfering data with the RTP protocol.
#
#%files -n gstreamer-rtp
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgstrtp.so
#
#%post -n gstreamer-rtp
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null
#
#%postun -n gstreamer-rtp
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### SIDPLAY ###
#%package -n gstreamer-sid
#Summary:       GStreamer Sid C64 music plug-in.
#Group:         Libraries/Multimedia
#Requires:      gstreamer-plugins = %{version}
#Requires:      libsidplay => 1.36.0
#BuildRequires: libsidplay-devel => 1.36.0
#
#%description -n gstreamer-sid
#Plug-in for playback of C64 SID format music files.
#
#%files -n gstreamer-sid
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgstsid.so
#
#%post -n gstreamer-sid
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null
#
#%postun -n gstreamer-sid
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### SDL ###
%package -n gstreamer-SDL
Summary:       GStreamer plug-in for outputting video to SDL.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      SDL >= 1.2.0
BuildRequires: SDL-devel >= 1.2.0
#SDL-devel should require XFree86-devel because it links to it
#only it doesn't seem to do that currently
BuildRequires: 	XFree86-devel
#it used to be called gstreamer-sdl
Obsoletes: 	gstreamer-sdl

%description -n gstreamer-SDL
Plug-in for sending video output to the Simple Direct Media architecture.
(http://www.libsdl.org). Useful for full-screen playback.

%files -n gstreamer-SDL
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstsdlvideosink.so

%post -n gstreamer-SDL
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-SDL
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### SHOUT ###
%package -n gstreamer-icecast
Summary:       GStreamer Icecast plug-in using libshout.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      libshout >= 1.0.5
BuildRequires: libshout-devel >= 1.0.5

%description -n gstreamer-icecast
Plug-in for broadcasting audio to the Icecast server.

%files -n gstreamer-icecast
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstshout.so

%post -n gstreamer-icecast
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-icecast
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### VORBIS ###
%package -n gstreamer-vorbis
Summary:       GStreamer plug-in for encoding and decoding Ogg Vorbis audio files.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      libogg >= 1.0
Requires:      libvorbis >= 1.0
BuildRequires: libogg-devel >= 1.0
BuildRequires: libvorbis-devel >= 1.0

%description -n gstreamer-vorbis
Plug-ins for creating and playing Ogg Vorbis audio files.

%files -n gstreamer-vorbis  
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstvorbis.so

%post -n gstreamer-vorbis
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-vorbis
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### VIDEO 4 LINUX ###
%package -n gstreamer-v4l
Summary:       GStreamer Video for Linux plug-in.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
BuildRequires: glibc-devel

%description -n gstreamer-v4l
Plug-in for accessing Video for Linux devices.

%files -n gstreamer-v4l
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstv4lelement.so
%{_libdir}/gstreamer-%{majorminor}/libgstv4lsrc.so
%{_libdir}/gstreamer-%{majorminor}/libgstv4lmjpegsrc.so
%{_libdir}/gstreamer-%{majorminor}/libgstv4lmjpegsink.so

%post -n gstreamer-v4l
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-v4l
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### VIDEO 4 LINUX 2 ###
#%package -n gstreamer-v4l2
#Summary:       GStreamer Video for Linux 2 plug-in.
#Group:         Libraries/Multimedia
#Requires:      gstreamer-plugins = %{version}
#BuildRequires: glibc-devel
#
#%description -n gstreamer-v4l2
#Plug-in for accessing Video for Linux devices.
#
#%files -n gstreamer-v4l2
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgstv4l2element.so
#%{_libdir}/gstreamer-%{majorminor}/libgstv4l2src.so
#
#%post -n gstreamer-v4l2
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null
#
#%postun -n gstreamer-v4l2
#%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null


### XVIDEO ###
%package -n gstreamer-xvideosink
Summary: 	GStreamer XFree output plug-in
Group: 	Libraries/Multimedia
Requires: 	gstreamer-plugins = %{version}
Requires: 	XFree86-libs
BuildRequires: XFree86-devel
%description -n gstreamer-xvideosink
XFree86 video sink

%files -n gstreamer-xvideosink
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstxvideosink.so

%post -n gstreamer-xvideosink
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-xvideosink
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%package -n gstreamer-videosink
Summary:       GStreamer Video Sink
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      XFree86-libs
BuildRequires: XFree86-devel

%description -n gstreamer-videosink
Plug-in for X playback.

%files -n gstreamer-videosink
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstvideosink.so

%post -n gstreamer-videosink
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-videosink
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### packages without external dependencies ###

### audio-effects ###
%package -n gstreamer-audio-effects
Summary: 	GStreamer audio effects plug-in.
Group: 		Libraries/Multimedia
Requires: 	gstreamer-plugins = %{version}
Obsoletes:	gstreamer-misc

%description -n gstreamer-audio-effects
Plug-in with various audio effects including resampling, 
sine wave generation, silence generation, channel mixing, stream mixing,
integer to float conversion, LAW conversion and level detection plug-ins.

%files -n gstreamer-audio-effects
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstresample.so
%{_libdir}/gstreamer-%{majorminor}/libgstsinesrc.so
%{_libdir}/gstreamer-%{majorminor}/libgstsilence.so
%{_libdir}/gstreamer-%{majorminor}/libgststereo.so
%{_libdir}/gstreamer-%{majorminor}/libgststereo2mono.so
%{_libdir}/gstreamer-%{majorminor}/libgstvolume.so
%{_libdir}/gstreamer-%{majorminor}/libgstvolenv.so
%{_libdir}/gstreamer-%{majorminor}/libgstplayondemand.so
%{_libdir}/gstreamer-%{majorminor}/libgstspeed.so
%{_libdir}/gstreamer-%{majorminor}/libgststereosplit.so
%{_libdir}/gstreamer-%{majorminor}/libgstadder.so
%{_libdir}/gstreamer-%{majorminor}/libgstalaw.so
%{_libdir}/gstreamer-%{majorminor}/libgstintfloat.so
%{_libdir}/gstreamer-%{majorminor}/libgstlevel.so
%{_libdir}/gstreamer-%{majorminor}/libgstmono2stereo.so
%{_libdir}/gstreamer-%{majorminor}/libgstmulaw.so
%{_libdir}/gstreamer-%{majorminor}/libgstpassthrough.so
# %{_libdir}/gstreamer-%{majorminor}/libgstfloatcast.so
%{_libdir}/gstreamer-%{majorminor}/libgstcutter.so
%{_libdir}/gstreamer-%{majorminor}/libgstfilter.so
%{_libdir}/gstreamer-%{majorminor}/libmixmatrix.so
%{_libdir}/gstreamer-%{majorminor}/libgstoneton.so

%post -n gstreamer-audio-effects
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-audio-effects
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### audio-formats ###
%package -n gstreamer-audio-formats
Summary: 	GStreamer audio format plug-ins.
Group: 		Libraries/Multimedia
Requires: 	gstreamer-plugins = %{version}
BuildRequires: 	gcc-c++

%description -n gstreamer-audio-formats
Plug-in for playback of wav, au and mod audio files as well as mp3 type.

%files -n gstreamer-audio-formats
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstwavparse.so
%{_libdir}/gstreamer-%{majorminor}/libgstauparse.so
%{_libdir}/gstreamer-%{majorminor}/libgstmp3types.so
%{_libdir}/gstreamer-%{majorminor}/libgstmodplug.so
%{_libdir}/gstreamer-%{majorminor}/libgstwavenc.so

%post -n gstreamer-audio-formats
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-audio-formats
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### festival ###
%package -n gstreamer-festival
Summary: 	GStreamer plug-in for text-to-speech support using a festival server.
Group: 		Libraries/Multimedia
Requires: 	gstreamer-plugins = %{version}

%description -n gstreamer-festival
Plug-in for text-to-speech using the festival server.

%files -n gstreamer-festival
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstfestival.so

%post -n gstreamer-festival
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-festival
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### ffmpeg ###
%package -n gstreamer-ffmpeg
Summary: 	GStreamer plug-in for included ffmpeg libavcodec/format library.
Group: 		Libraries/Multimedia
Requires: 	gstreamer-plugins = %{version}
%description -n gstreamer-ffmpeg
Plug-in for ffmpeg library.

%files -n gstreamer-ffmpeg
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstffmpeg.so
%{_libdir}/gstreamer-%{majorminor}/libgstffmpegall.so

%post -n gstreamer-ffmpeg
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-ffmpeg
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### flx ###
%package -n gstreamer-flx
Summary: 	GStreamer plug-in for FLI/FLX animation format.
Group: 		Libraries/Multimedia
Requires: 	gstreamer-plugins = %{version}
Requires: 	gstreamer-colorspace = %{version}
%description -n gstreamer-flx
Plug-in for playing FLI/FLX animations under GStreamer.

%files -n gstreamer-flx
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstflxdec.so

%post -n gstreamer-flx
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-flx
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### qcam ###
%package -n gstreamer-qcam
Summary: 	GStreamer QuickCam plug-in.
Group: 		Libraries/Multimedia
Requires: 	gstreamer-plugins = %{version}

%description -n gstreamer-qcam
Plug-in for accessing a Quickcam video source.

%files -n gstreamer-qcam
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstqcam.so

%post -n gstreamer-qcam
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-qcam
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### udp ###
%package -n gstreamer-udp
Summary: 	GStreamer plug-ins for UDP tranport.
Group: 		Libraries/Multimedia
Requires: 	gstreamer-plugins = %{version}

%description -n gstreamer-udp
Plug-ins for UDP transport under GStreamer.

%files -n gstreamer-udp
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstudp.so

%post -n gstreamer-udp
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-udp
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### vcd ###
%package -n gstreamer-vcd
Summary: 	GStreamer Video CD plug-in.
Group: 		Libraries/Multimedia
Requires: 	gstreamer-plugins = %{version}

%description -n gstreamer-vcd
Video CD parsing and playback plug-in for GStreamer.

%files -n gstreamer-vcd
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstvcdsrc.so
%{_libdir}/gstreamer-%{majorminor}/libgstcdxaparse.so

%post -n gstreamer-vcd
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-vcd
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### video-effects ###
%package -n gstreamer-video-effects
Summary: 	GStreamer video effects plug-in.
Group: 		Libraries/Multimedia
Requires: 	gstreamer-plugins = %{version}
Obsoletes:	gstreamer-deinterlace
Obsoletes:	gstreamer-misc

%description -n gstreamer-video-effects
Plug-in with various video effects including deinterlacing and effecTV
plug-ins.

%files -n gstreamer-video-effects
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgsteffectv.so
%{_libdir}/gstreamer-%{majorminor}/libgstdeinterlace.so
%{_libdir}/gstreamer-%{majorminor}/libgstmedian.so
%{_libdir}/gstreamer-%{majorminor}/libgstrtjpeg.so
%{_libdir}/gstreamer-%{majorminor}/libgstvideocrop.so
%{_libdir}/gstreamer-%{majorminor}/libgstvideoscale.so
%{_libdir}/gstreamer-%{majorminor}/libgstsmpte.so
%{_libdir}/gstreamer-%{majorminor}/libgstvbidec.so

%post -n gstreamer-video-effects
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-video-effects
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### visualisation ###
%package -n gstreamer-visualisation
Summary: 	GStreamer visualisations plug-ins.
Group: 		Libraries/Multimedia
Requires: 	gstreamer-plugins = %{version}

%description -n gstreamer-visualisation
Various plug-ins for visual effects to use with audio.
This includes smoothwave, spectrum, goom, chart, monoscope, synaesthesia
and vumeter.

%files -n gstreamer-visualisation
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstsmooth.so
%{_libdir}/gstreamer-%{majorminor}/libgstspectrum.so
%{_libdir}/gstreamer-%{majorminor}/libgstvumeter.so
%{_libdir}/gstreamer-%{majorminor}/libgstgoom.so
%{_libdir}/gstreamer-%{majorminor}/libgstchart.so
%{_libdir}/gstreamer-%{majorminor}/libgstmonoscope.so
%{_libdir}/gstreamer-%{majorminor}/libgstsynaesthesia.so

%post -n gstreamer-visualisation
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-visualisation
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### yuv4mjpeg ###
%package -n gstreamer-yuv4mjpeg
Summary: 	GStreamer plug-in for YUV to MJPEG conversion.
Group: 		Libraries/Multimedia
Requires: 	gstreamer-plugins = %{version}
Obsoletes: 	gstreamer-lavencode

%description -n gstreamer-yuv4mjpeg
It takes YUV video frames and adds a header in front of it so it can be 
processed with the lavtools from mjpegtools.

%files -n gstreamer-yuv4mjpeg
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgsty4menc.so

%post -n gstreamer-yuv4mjpeg
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-yuv4mjpeg
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

# cdplayer
%package -n gstreamer-cdplayer
Summary:        GStreamer plug-in playing audio cds	
Group:          Libraries/Multimedia
Requires:       gstreamer-plugins = %{version}

%description -n gstreamer-cdplayer
Lets you get sound from audio cd's using GStreamer

%files -n gstreamer-cdplayer
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstcdplayer.so

%post -n gstreamer-cdplayer
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-cdplayer
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

# Videotest plugin
%package -n gstreamer-videotest
Summary:        GStreamer plug-in for generating a video test streamer
Group:          Libraries/Multimedia
Requires:       gstreamer-plugins = %{version}

%description -n gstreamer-videotest
This plugin provides a videotest plugin. This plugin can be used to generate a videostream for testing other plugins.

%files -n gstreamer-videotest
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstvideotestsrc.so

%post -n gstreamer-videotest
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-videotest
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

# Snapshot plugin
%package -n gstreamer-snapshot
Summary:        GStreamer plug-in for grabbing images from videostreams
Group:          Libraries/Multimedia
Requires:       gstreamer-plugins = %{version}

%description -n gstreamer-snapshot
This plugin grabs images from videostreams and saves them as PNG format images.
Requires:	libpng
Requires:	gstreamer-colorspace = %{version}
BuildRequires:	libpng-devel

%files -n gstreamer-snapshot
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstsnapshot.so

%post -n gstreamer-snapshot
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-snapshot
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

# Dependency free Quicktime demuxer
%package -n gstreamer-quicktime
Summary:       GStreamer Quicktime demuxer video Plug-in.
Group:         Libraries/Multimedia
Requires:      gstreamer-plugins = %{version}

%description -n gstreamer-quicktime
Plug-in for demuxing Quicktime movies

%files -n gstreamer-quicktime
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstqtdemux.so

%post -n gstreamer-quicktime
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-quicktime
%{_bindir}/gst-register --gst-mask=0 > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

# package supporting GConf
%package -n gstreamer-GConf
Summary: 	GStreamer GConf schemas.
Group: 		Libraries/Multimedia
Requires: 	gstreamer-plugins = %{version}
Requires: 	GConf2
BuildRequires: 	GConf2-devel

%description -n gstreamer-GConf
Installation of GStreamer GConf schemas.
These set usable defaults used by all GStreamer-enabled Gnome applications.

%files -n gstreamer-GConf
%defattr(-, root, root)
%{_sysconfdir}/gconf/schemas/gstreamer.schemas
%{_libdir}/libgstgconf-%{majorminor}.so.*

%post -n gstreamer-GConf
export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
gconftool-2 --makefile-install-rule %{_sysconfdir}/gconf/schemas/gstreamer.schemas > /dev/null

# play library #
%package -n gstreamer-play
Summary: 	GStreamer play library.
Group: 		Libraries/Multimedia
Requires: 	gstreamer-plugins = %{version}

%description -n gstreamer-play
This package contains a basic audio and video playback library.

%files -n gstreamer-play
%defattr(-, root, root)
%{_libdir}/libgstplay-%{majorminor}.so.*

%post -n gstreamer-play
/sbin/ldconfig

%postun -n gstreamer-play
/sbin/ldconfig

%changelog
* Wed Jan 29 2003 Thomas Vander Stichele <thomas at apestaart dot org>
- undo gstreamer-videosink provides since they obviously clash

* Mon Jan 27 2003 Thomas Vander Stichele <thomas at apestaart dot org>
- added gconf-devel virtual provide in gstreamer-plugins-devel, as well
  as .pc files

* Thu Jan 23 2003 Thomas Vander Stichele <thomas at apestaart dot org>
- various fixes
- make video output packages provide gstreamer-videosink

* Thu Jan 23 2003 Thomas Vander Stichele <thomas at apestaart dot org>
- split out ffmpeg stuff to separate plugin

* Fri Dec 27 2002 Thomas Vander Stichele <thomas at apestaart dot org>
- add virtual provides for audio sources and sinks

* Sun Dec 15 2002 Christian Schaller <Uraeus@linuxrising.org>
- Update mpeg2dec REQ to be 0.3.1

* Tue Dec 10 2002 Thomas Vander Stichele <thomas at apestaart dot org>
- only install schema once
- move out devel lib stuff to -devel package

* Sun Dec 08 2002 Thomas Vander Stichele <thomas at apestaart dot org>
- fix location of libgstpng
- changes for parallel installability

* Thu Nov 28 2002 Christian Schaller <Uraeus@linuxrising.org>
- Put in libgstpng plugin
- rm the libgstmedia-info stuff until thomas think they are ready

* Fri Nov 01 2002 Thomas Vander Stichele <thomas at apestaart dot org>
- don't use compprep until ABI issues can be fixed

* Wed Oct 30 2002 Thomas Vander Stichele <thomas at apestaart dot org>
- added smpte plugin
- split out dvdnavread package
- fixed snapshot deps and added hermes conditionals

* Tue Oct 29 2002 Thomas Vander Stichele <thomas at apestaart dot org>
- added -play package, libs, and .pc files

* Thu Oct 24 2002 Christian Schaller <Uraeus@linuxrising.org>
- Added wavenc to audio formats package

* Sat Oct 20 2002 Christian Scchaller <Uraeus@linuxrising.org>
- Removed all .la files
- added separate non-openquicktime demuxer plugin
- added snapshot plugin
- added videotest plugin
- Split avi plugin out to avi and windec plugins since aviplugin do not depend on avifile
- Added cdplayer plugin

* Fri Sep 20 2002 Thomas Vander Stichele <thomas@apestaart.org>
- added gst-compprep calls

* Wed Sep 18 2002 Thomas Vander Stichele <thomas@apestaart.org>
- add gst-register calls everywhere again since auto-reregister doesn't work
- added gstreamer-audio-formats to mad's requires since it needs the typefind
  to work properly

* Mon Sep 9 2002 Christian Schaller <Uraeus@linuxrising.org>
- Added v4l2 plugin
* Thu Aug 27 2002 Christian Schaller <Uraeus@linuxrising.org>
- Fixed USE_DV_TRUE to USE_LIBDV_TRUE
- Added Gconf and floatcast headers to gstreamer-plugins-devel package
- Added mixmatrix plugin to audio-effects package

* Thu Jul 11 2002 Thomas Vander Stichele <thomas@apestaart.org>
- fixed oss package to buildrequire instead of require glibc headers

* Mon Jul 08 2002 Thomas Vander Stichele <thomas@apestaart.org>
- fixed -devel package group

* Fri Jul 05 2002 Thomas Vander Stichele <thomas@apestaart.org>
- release 0.4.0 !
- added gstreamer-libs.pc
- removed all gst-register calls since this should be done automatically now

* Thu Jul 04 2002 Thomas Vander Stichele <thomas@apestaart.org>
- fix issue with SDL package
- make all packages STRICTLY require the right version to avoid
  ABI issues
- make gst-plugins obsolete gst-plugin-libs
- also send output of gst-register to /dev/null to lower the noise

* Wed Jul 03 2002 Thomas Vander Stichele <thomas@apestaart.org>
- require glibc-devel instead of glibc-kernheaders since the latter is only
  since 7.3 and glibc-devel pulls in the right package anyway

* Sun Jun 23 2002 Thomas Vander Stichele <thomas@apestaart.org>
- changed header location of plug-in libs

* Mon Jun 17 2002 Thomas Vander Stichele <thomas@apestaart.org>
- major cleanups
- adding gst-register on postun everywhere
- remove ldconfig since we don't actually install libs in system dirs
- removed misc package
- added video-effects
- dot every Summary
- uniformify all descriptions a little

* Thu Jun 06 2002 Thomas Vander Stichele <thomas@apestaart.org>
- various BuildRequires: additions

* Tue Jun 04 2002 Thomas Vander Stichele <thomas@apestaart.org>
- added USE_LIBADSPA_TRUE bits to ladspa package

* Mon Jun 03 2002 Thomas Vander Stichele <thomas@apestaart.org>
- Added libfame package

* Mon May 12 2002 Christian Fredrik Kalager Schaller <Uraeus@linuxrising.org>
- Added jack, dxr3, http packages
- Added visualisation plug-ins, effecttv and synaesthesia
- Created devel package
- Removed gstreamer-plugins-libs package (moved it into gstreamer-plugins)
- Replaced prefix/dirname with _macros

* Mon May 06 2002 Thomas Vander Stichele <thomas@apestaart.org>
- added gstreamer-GConf package

* Wed Mar 13 2002 Thomas Vander Stichele <thomas@apestaart.org>
- added more BuildRequires and Requires
- rearranged some plug-ins
- added changelog ;)
