Name: 		gstreamer-plugins
Version: 	0.8.4
Release: 	1
Summary: 	GStreamer Streaming-media framework plug-ins.

%define 	majorminor	0.8

Group: 		Applications/Multimedia
License: 	LGPL
URL:		http://gstreamer.net/
Vendor:         GStreamer Backpackers Team <package@gstreamer.net>
Source:         http://gstreamer.freedesktop.org/src/gst-plugins/gst-plugins-%{version}.tar.gz
BuildRoot: 	%{_tmppath}/%{name}-%{version}-root

%define         _glib2          1.3.12

Requires:       glib2 >= %_glib2
BuildRequires:  glib2-devel >= %_glib2
Requires: 	gstreamer >= 0.8.1
BuildRequires: 	gstreamer-devel >= 0.8.1
BuildRequires:	gstreamer-tools >= 0.8.1

Requires:      arts >= 1.1.4
BuildRequires: arts-devel >= 1.1.4
BuildRequires: gcc-c++
Requires:      audiofile >= 0.2.1
BuildRequires: audiofile-devel >= 0.2.1
Requires:      cdparanoia-libs >= alpha9.7
BuildRequires: cdparanoia-devel >= alpha9.7
Requires:      esound >= 0.2.8
BuildRequires: esound-devel >= 0.2.8
Obsoletes:     gstreamer-esd

Provides:		gstreamer-audiosrc
Provides:		gstreamer-audiosink
Requires:      flac >= 1.0.3
BuildRequires: flac-devel >= 1.0.3
Requires: 	GConf2
BuildRequires: 	GConf2-devel
Requires:      gnome-vfs2 > 1.9.4.00
BuildRequires: gnome-vfs2-devel > 1.9.4.00
Requires:      Hermes >= 1.3.0
BuildRequires: Hermes-devel >= 1.3.0
Requires:      libjpeg
BuildRequires: libjpeg-devel
Requires:      libpng >= 1.2.0
BuildRequires: libpng-devel >= 1.2.0
Requires:      mikmod
BuildRequires: mikmod
BuildRequires: glibc-devel
Requires:        pango
BuildRequires:   pango-devel
Requires:      libraw1394
BuildRequires: libraw1394-devel
Requires:      SDL >= 1.2.0
BuildRequires: SDL-devel >= 1.2.0
#SDL-devel should require XFree86-devel because it links to it
#only it doesn't seem to do that currently
BuildRequires: 	XFree86-devel
Requires:	speex
BuildRequires:	speex-devel
Requires:	gtk2
BuildRequires:	gtk2-devel
Requires:      libogg >= 1.0
Requires:      libvorbis >= 1.0
BuildRequires: libogg-devel >= 1.0
BuildRequires: libvorbis-devel >= 1.0
Requires: 	XFree86-libs
BuildRequires: XFree86-devel
# Snapshot plugin
Requires:	libpng

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
%configure \
  --with-gdk-pixbuf-loader-dir=$RPM_BUILD_ROOT%{_libdir}/gtk-2.0/2.2.0/loaders \
  --enable-debug \
  --enable-DEBUG 

make %{?_smp_mflags}
                                                                                
%install
rm -rf $RPM_BUILD_ROOT

export GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL=1
%makeinstall
unset GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL
                                                                                
# Clean out files that should not be part of the rpm.
rm -f $RPM_BUILD_ROOT%{_libdir}/gstreamer-%{majorminor}/*.la
rm -f $RPM_BUILD_ROOT%{_libdir}/gstreamer-%{majorminor}/*.a
rm -f $RPM_BUILD_ROOT%{_libdir}/*.a
rm -f $RPM_BUILD_ROOT%{_libdir}/*.la
rm -f $RPM_BUILD_ROOT%{_libdir}/gstreamer-%{majorminor}/libgstgdkpixbuf.so

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%doc AUTHORS COPYING README REQUIREMENTS

# helper programs
%{_bindir}/gst-launch-ext-%{majorminor}
%{_bindir}/gst-visualise-%{majorminor}
%{_mandir}/man1/gst-launch-ext-%{majorminor}.*
%{_mandir}/man1/gst-visualise-%{majorminor}*

# schema files
%{_sysconfdir}/gconf/schemas/gstreamer-%{majorminor}.schemas

# libraries
%{_libdir}/libgstplay-%{majorminor}.so.*
%{_libdir}/libgstinterfaces-%{majorminor}.so.*
%{_libdir}/libgstgconf-%{majorminor}.so.*

# plugin helper libraries
%{_libdir}/gstreamer-%{majorminor}/libgstaudio.so
%{_libdir}/gstreamer-%{majorminor}/libgstidct.so
%{_libdir}/gstreamer-%{majorminor}/libgstriff.so
%{_libdir}/gstreamer-%{majorminor}/libgstvideo.so
%{_libdir}/gstreamer-%{majorminor}/libgstxwindowlistener.so

# non-core plugins without external dependencies
%{_libdir}/gstreamer-%{majorminor}/libgstac3parse.so
%{_libdir}/gstreamer-%{majorminor}/libgstadder.so
%{_libdir}/gstreamer-%{majorminor}/libgstalaw.so
%{_libdir}/gstreamer-%{majorminor}/libgstasf.so
%{_libdir}/gstreamer-%{majorminor}/libgstaudioconvert.so
%{_libdir}/gstreamer-%{majorminor}/libgstaudiofilter.so
%{_libdir}/gstreamer-%{majorminor}/libgstaudioscale.so
%{_libdir}/gstreamer-%{majorminor}/libgstauparse.so
%{_libdir}/gstreamer-%{majorminor}/libgstavi.so
%{_libdir}/gstreamer-%{majorminor}/libgstcdplayer.so
%{_libdir}/gstreamer-%{majorminor}/libgstcdxaparse.so
%{_libdir}/gstreamer-%{majorminor}/libgstchart.so
%{_libdir}/gstreamer-%{majorminor}/libgstcolorspace.so
%{_libdir}/gstreamer-%{majorminor}/libgstcutter.so
%{_libdir}/gstreamer-%{majorminor}/libgstdebug.so
%{_libdir}/gstreamer-%{majorminor}/libgstdeinterlace.so
%{_libdir}/gstreamer-%{majorminor}/libgstefence.so
%{_libdir}/gstreamer-%{majorminor}/libgsteffectv.so
%{_libdir}/gstreamer-%{majorminor}/libgstfestival.so
%{_libdir}/gstreamer-%{majorminor}/libgstffmpegcolorspace.so
%{_libdir}/gstreamer-%{majorminor}/libgstfilter.so
%{_libdir}/gstreamer-%{majorminor}/libgstflxdec.so
%{_libdir}/gstreamer-%{majorminor}/libgstgamma.so
%{_libdir}/gstreamer-%{majorminor}/libgstgoom.so
%{_libdir}/gstreamer-%{majorminor}/libgstinterleave.so
%{_libdir}/gstreamer-%{majorminor}/libgstlevel.so
%{_libdir}/gstreamer-%{majorminor}/libgstmatroska.so
%{_libdir}/gstreamer-%{majorminor}/libgstmedian.so
%{_libdir}/gstreamer-%{majorminor}/libgstmixmatrix.so
%{_libdir}/gstreamer-%{majorminor}/libgstmodplug.so
%{_libdir}/gstreamer-%{majorminor}/libgstmonoscope.so
%{_libdir}/gstreamer-%{majorminor}/libgstmp1videoparse.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpeg1systemencode.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpegaudio.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpegaudioparse.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpegstream.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpeg2subt.so
%{_libdir}/gstreamer-%{majorminor}/libgstmulaw.so
%{_libdir}/gstreamer-%{majorminor}/libgstnavigationtest.so
%{_libdir}/gstreamer-%{majorminor}/libgstoverlay.so
%{_libdir}/gstreamer-%{majorminor}/libgstpassthrough.so
%{_libdir}/gstreamer-%{majorminor}/libgstplayondemand.so
%ifarch %{ix86}
%{_libdir}/gstreamer-%{majorminor}/libgstqcam.so
%endif
%{_libdir}/gstreamer-%{majorminor}/libgstresample.so
%{_libdir}/gstreamer-%{majorminor}/libgstrmdemux.so
%{_libdir}/gstreamer-%{majorminor}/libgstrtjpeg.so
%{_libdir}/gstreamer-%{majorminor}/libgstrtp.so
%{_libdir}/gstreamer-%{majorminor}/libgstqtdemux.so
%{_libdir}/gstreamer-%{majorminor}/libgstsilence.so
%{_libdir}/gstreamer-%{majorminor}/libgstsinesrc.so
%{_libdir}/gstreamer-%{majorminor}/libgstsmooth.so
%{_libdir}/gstreamer-%{majorminor}/libgstsmpte.so
%{_libdir}/gstreamer-%{majorminor}/libgstspectrum.so
%{_libdir}/gstreamer-%{majorminor}/libgstspeed.so
%{_libdir}/gstreamer-%{majorminor}/libgststereo.so
%{_libdir}/gstreamer-%{majorminor}/libgstswitch.so
%{_libdir}/gstreamer-%{majorminor}/libgstsynaesthesia.so
%{_libdir}/gstreamer-%{majorminor}/libgsttagedit.so
%{_libdir}/gstreamer-%{majorminor}/libgsttcp.so
%{_libdir}/gstreamer-%{majorminor}/libgsttypefindfunctions.so
%{_libdir}/gstreamer-%{majorminor}/libgstudp.so
%{_libdir}/gstreamer-%{majorminor}/libgstvbidec.so
%{_libdir}/gstreamer-%{majorminor}/libgstvcdsrc.so
%{_libdir}/gstreamer-%{majorminor}/libgstvideobalance.so
%{_libdir}/gstreamer-%{majorminor}/libgstvideocrop.so
%{_libdir}/gstreamer-%{majorminor}/libgstvideodrop.so
%{_libdir}/gstreamer-%{majorminor}/libgstvideofilter.so
%{_libdir}/gstreamer-%{majorminor}/libgstvideoflip.so
%{_libdir}/gstreamer-%{majorminor}/libgstvideoscale.so
%{_libdir}/gstreamer-%{majorminor}/libgstvideotestsrc.so
%{_libdir}/gstreamer-%{majorminor}/libgstvolenv.so
%{_libdir}/gstreamer-%{majorminor}/libgstvolume.so
%{_libdir}/gstreamer-%{majorminor}/libgstwavenc.so
%{_libdir}/gstreamer-%{majorminor}/libgstwavparse.so
%{_libdir}/gstreamer-%{majorminor}/libgsty4menc.so

# gstreamer-plugins with external dependencies but in the main package
%{_libdir}/gstreamer-%{majorminor}/libgstarts.so
%{_libdir}/gstreamer-%{majorminor}/libgstartsdsink.so
%{_libdir}/gstreamer-%{majorminor}/libgstaudiofile.so
%{_libdir}/gstreamer-%{majorminor}/libgstcdparanoia.so
%{_libdir}/gstreamer-%{majorminor}/libgstesd.so
%{_libdir}/gstreamer-%{majorminor}/libgstflac.so
%{_libdir}/gstreamer-%{majorminor}/libgstgnomevfs.so
%{_libdir}/gstreamer-%{majorminor}/libgsthermescolorspace.so
%{_libdir}/gstreamer-%{majorminor}/libgstsmoothwave.so
%{_libdir}/gstreamer-%{majorminor}/libgstjpeg.so
%{_libdir}/gstreamer-%{majorminor}/libgstmikmod.so
%{_libdir}/gstreamer-%{majorminor}/libgstsdlvideosink.so
%{_libdir}/gstreamer-%{majorminor}/libgstvorbis.so
%{_libdir}/gstreamer-%{majorminor}/libgstogg.so
%{_libdir}/gstreamer-%{majorminor}/libgstpng.so
%{_libdir}/gstreamer-%{majorminor}/libgstossaudio.so
%{_libdir}/gstreamer-%{majorminor}/libgstvideo4linux.so
%{_libdir}/gstreamer-%{majorminor}/libgst1394.so
# Snapshot plugin uses libpng
%{_libdir}/gstreamer-%{majorminor}/libgstsnapshot.so
%{_libdir}/gstreamer-%{majorminor}/libgsttextoverlay.so
%{_libdir}/gstreamer-%{majorminor}/libgsttimeoverlay.so
%{_libdir}/gstreamer-%{majorminor}/libgstspeex.so
%{_libdir}/gstreamer-%{majorminor}/libgstcacasink.so
%{_libdir}/gstreamer-%{majorminor}/libgstximagesink.so
%{_libdir}/gstreamer-%{majorminor}/libgstxvimagesink.so
# %{_libdir}/gstreamer-%{majorminor}/libgstgdkpixbuf.so
# Docs
%{_datadir}/locale

%post
export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
gconftool-2 --makefile-install-rule %{_sysconfdir}/gconf/schemas/gstreamer-%{majorminor}.schemas > /dev/null
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null

%package -n gstreamer-plugins-devel
Summary: 	GStreamer Plugin Library Headers.
Group: 		Development/Libraries
Requires: 	gstreamer-plugins = %{version}
Provides:	gstreamer-play-devel = %{version}

%description -n gstreamer-plugins-devel
GStreamer support libraries header files.

%files -n gstreamer-plugins-devel
%defattr(-, root, root)
# plugin helper library headers
%{_includedir}/gstreamer-%{majorminor}/gst/audio/audio.h
%{_includedir}/gstreamer-%{majorminor}/gst/audio/audioclock.h
%{_includedir}/gstreamer-%{majorminor}/gst/audio/gstaudiofilter.h
%{_includedir}/gstreamer-%{majorminor}/gst/floatcast/floatcast.h
%{_includedir}/gstreamer-%{majorminor}/gst/idct/idct.h
%{_includedir}/gstreamer-%{majorminor}/gst/resample/resample.h
%{_includedir}/gstreamer-%{majorminor}/gst/riff/riff-ids.h
%{_includedir}/gstreamer-%{majorminor}/gst/riff/riff-media.h
%{_includedir}/gstreamer-%{majorminor}/gst/riff/riff-read.h
%{_includedir}/gstreamer-%{majorminor}/gst/video/video.h
%{_includedir}/gstreamer-%{majorminor}/gst/video/videosink.h
# plugin interface headers
%{_includedir}/gstreamer-%{majorminor}/gst/mixer/mixer.h
%{_includedir}/gstreamer-%{majorminor}/gst/mixer/mixertrack.h
%{_includedir}/gstreamer-%{majorminor}/gst/mixer/mixer-enumtypes.h
%{_includedir}/gstreamer-%{majorminor}/gst/navigation/navigation.h
%{_includedir}/gstreamer-%{majorminor}/gst/colorbalance/colorbalance.h
%{_includedir}/gstreamer-%{majorminor}/gst/colorbalance/colorbalancechannel.h
%{_includedir}/gstreamer-%{majorminor}/gst/colorbalance/colorbalance-enumtypes.h
%{_includedir}/gstreamer-%{majorminor}/gst/propertyprobe/propertyprobe.h
%{_includedir}/gstreamer-%{majorminor}/gst/tuner/tuner.h
%{_includedir}/gstreamer-%{majorminor}/gst/tuner/tunerchannel.h
%{_includedir}/gstreamer-%{majorminor}/gst/tuner/tunernorm.h
%{_includedir}/gstreamer-%{majorminor}/gst/tuner/tuner-enumtypes.h
%{_includedir}/gstreamer-%{majorminor}/gst/xoverlay/xoverlay.h
%{_includedir}/gstreamer-%{majorminor}/gst/xwindowlistener/xwindowlistener.h
# library headers
%{_includedir}/gstreamer-%{majorminor}/gst/gconf/gconf.h
%{_includedir}/gstreamer-%{majorminor}/gst/media-info/media-info.h
%{_includedir}/gstreamer-%{majorminor}/gst/play/play.h
%{_includedir}/gstreamer-%{majorminor}/gst/play/play-enumtypes.h
%{_includedir}/gstreamer-%{majorminor}/gst/tag/tag.h
# pkg-config files
%{_libdir}/pkgconfig/gstreamer-gconf-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-interfaces-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-libs-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-media-info-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-play-%{majorminor}.pc
%{_libdir}/pkgconfig/gstreamer-plugins-%{majorminor}.pc
# .so files
%{_libdir}/libgstgconf-%{majorminor}.so
%{_libdir}/libgstmedia-info-%{majorminor}.so*
%{_libdir}/libgstplay-%{majorminor}.so

# Here are packages not in the base plugins package but not dependant
# on an external lib

# Here are all the packages depending on external libs #

### A52DEC ###
%package -n gstreamer-a52dec
Summary:       GStreamer VOB decoder plug-in.
Group:         Applications/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      a52dec >= 0.7.3
BuildRequires: a52dec-devel >= 0.7.3

%description -n gstreamer-a52dec
Plug-in for decoding of VOB files.

%files -n gstreamer-a52dec
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgsta52dec.so

%post -n gstreamer-a52dec
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null

%postun -n gstreamer-a52dec
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null

### AALIB ###
%package -n gstreamer-aalib
Summary:	GStreamer plug-in for ASCII art output.
Group:		Applications/Multimedia
Requires:	gstreamer-plugins = %{version}
Requires:	aalib >= 1.3
BuildRequires:	aalib-devel >= 1.3

%description -n gstreamer-aalib
Plug-in for viewing video in ASCII art using aalib library.

%files -n gstreamer-aalib
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstaasink.so

%post -n gstreamer-aalib
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null

%postun -n gstreamer-aalib
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null
### ALSA ###
%package -n gstreamer-alsa
Summary:  GStreamer plug-ins for the ALSA sound system.
Group:    Applications/Multimedia
Requires: gstreamer-plugins = %{version}

Provides:	gstreamer-audiosrc
Provides:	gstreamer-audiosink

%description -n gstreamer-alsa
Input and output plug-in for the ALSA soundcard driver system. 
This plug-in depends on Alsa 0.9.x or higher.

%files -n gstreamer-alsa
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstalsa.so

%post -n gstreamer-alsa
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-alsa
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### DVDNAV ###
%package -n gstreamer-libdvdnav
Summary:       GStreamer plug-in for DVD playback.
Group:         Applications/Multimedia
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
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-libdvdnav
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### DVDREAD ###
%package -n gstreamer-libdvdread
Summary:       GStreamer plug-in for DVD playback.
Group:         Applications/Multimedia
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
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-libdvdread
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

## DXR3 ###
#%package -n gstreamer-dxr3
#Summary:       GStreamer plug-in for playback using dxr3 card.
#Group:         Applications/Multimedia
#Requires:      gstreamer-plugins = %{version}
#Requires:      em8300 >= 0.12.0
#BuildRequires: em8300-devel >= 0.12.0
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
#%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null
#
#%postun -n gstreamer-dxr3
#%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### FAAC ###
#%package -n gstreamer-faac
#Summary:GStreamer plug-ins for AAC audio playback.
#Group:         Applications/Multimedia
#Requires:      gstreamer-plugins = %{version}
#Requires:      faac >= 1.23
#BuildRequires: faac-devel >= 1.23
#
#%description -n gstreamer-faac
#Plug-ins for playing AAC audio
#
#%files -n gstreamer-faac
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgstfaac.so
#%post -n  gstreamer-faac
#%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null
#
#%postun -n  gstreamer-faac
#%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### FAAD ###
%package -n gstreamer-faad
Summary:GStreamer plug-ins for AAC audio playback.
Group:         Applications/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      faad2 >= 2.0
BuildRequires: faad2-devel >= 2.0

%description -n gstreamer-faad
Plug-ins for playing AAC audio

%files -n gstreamer-faad
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstfaad.so
%post -n  gstreamer-faad
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n  gstreamer-faad
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### GSM ###
%package -n gstreamer-gsm
Summary:       GStreamer plug-in for GSM lossy audio format.
Group:         Applications/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      gsm >= 1.0.10
BuildRequires: gsm-devel >= 1.0.10

%description -n gstreamer-gsm
Output plug-in for GStreamer to convert to GSM lossy audio format.

%files -n gstreamer-gsm
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstgsm.so

%post -n gstreamer-gsm
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-gsm
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

#### JACK AUDIO CONNECTION KIT ###
#%package -n gstreamer-jack
#Summary:  GStreamer plug-in for the Jack Sound Server.
#Group:    Applications/Multimedia
#Requires: gstreamer-plugins = %{version}
#Requires: jack-audio-connection-kit >= 0.28.0
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
#%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null
#
#%postun -n gstreamer-jack
#%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### LIBCACA ###
%package -n gstreamer-libcaca
Summary:        GStreamer plug-in for libcaca ASCII art output.
Group:          Applications/Multimedia
Requires:       gstreamer-plugins = %{version}
BuildRequires:  libcaca-devel >= 0.7

%description -n gstreamer-libcaca
Plug-in for viewing video in ASCII art using libcaca library.

%files -n gstreamer-libcaca
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstcacasink.so

%post -n gstreamer-libcaca
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null

%postun -n gstreamer-libcaca
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### LADSPA ###
%package -n gstreamer-ladspa
Summary:       GStreamer wrapper for LADSPA plug-ins.
Group:         Applications/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      ladspa
BuildRequires: ladspa-devel

%description -n gstreamer-ladspa
Plug-in which wraps LADSPA plug-ins for use by GStreamer applications.
We suggest you also get the cmt package of ladspa plug-ins
and steve harris s swh-plugins package.

%files -n gstreamer-ladspa
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstladspa.so

%post -n gstreamer-ladspa
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-ladspa
%{_bindir}/gst-register-%{majorminor}  > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### LAME ###
%package -n gstreamer-lame
Summary:       GStreamer plug-in encoding mp3 songs using lame.
Group:         Applications/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      lame >= 3.89
BuildRequires: lame-devel >= 3.89

%description -n gstreamer-lame
Plug-in for encoding mp3 with lame under GStreamer.

%files -n gstreamer-lame
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstlame.so

%post -n gstreamer-lame
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-lame
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null

### LIBDV ###
%package -n gstreamer-dv
Summary:       GStreamer DV plug-in.
Group:         Applications/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      libdv >= 0.9.5
BuildRequires: libdv-devel >= 0.9.5

%description -n gstreamer-dv
Plug-in for digital video support using libdv.

%files -n gstreamer-dv
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstdvdec.so

%post -n gstreamer-dv
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-dv
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null

### LIBFAME ###
%package -n gstreamer-libfame
Summary:       GStreamer plug-in to encode MPEG1/MPEG4 video.
Group:         Applications/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      libfame >= 0.9.0 
BuildRequires: libfame-devel >= 0.9.0 

%description -n gstreamer-libfame
Plug-in for encoding MPEG1/MPEG4 video using libfame.

%files -n gstreamer-libfame
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstlibfame.so

%post -n gstreamer-libfame
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-libfame
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null

### MAD ###
%package -n gstreamer-mad  
Summary:       GStreamer plug-in using MAD for mp3 decoding.
Group:         Applications/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      libmad >= 0.13.0
BuildRequires: libmad-devel >= 0.13.0
Requires:	     libid3tag >= 0.15.0
BuildRequires: libid3tag-devel >= 0.15.0

%description -n gstreamer-mad
Plug-in for playback of mp3 songs using the very good MAD library.

%files -n gstreamer-mad
%defattr(-, root, root)  
%{_libdir}/gstreamer-%{majorminor}/libgstmad.so

%post -n gstreamer-mad
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null

%postun -n gstreamer-mad
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null

### MPEG2DEC ###
%package -n gstreamer-mpeg
Summary:GStreamer plug-ins for MPEG video playback and encoding.
Group:         Applications/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      mpeg2dec >= 0.3.1
BuildRequires: mpeg2dec-devel >= 0.3.1
Obsoletes:     gstreamer-mpeg1
Obsoletes:     gstreamer-mpeg2
Obsoletes:     gstreamer-mpeg2dec

%description -n gstreamer-mpeg
Plug-ins for playing and encoding MPEG video.

%files -n gstreamer-mpeg
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstmpeg2dec.so
%post -n  gstreamer-mpeg
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n  gstreamer-mpeg
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

#### NETWORK AUDIO SYSTEM  ###
#%package -n gstreamer-nas
#Summary:  GStreamer plug-in for the Network Audio System.
#Group:    Applications/Multimedia
#Requires: gstreamer-plugins = %{version}
#Requires: libnas2 >= 1.6
#
#%description -n gstreamer-nas
#Plug-in for the Network Audio System sound server.
#
#%files -n gstreamer-nas
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgstnassink.so
#
#%post -n gstreamer-nas
#%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
#### %{_bindir}/gst-compprep > /dev/null 2> /dev/null
#
#%postun -n gstreamer-nas
#%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null

### SIDPLAY ###
%package -n gstreamer-sid
Summary:       GStreamer Sid C64 music plug-in.
Group:         Applications/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      libsidplay >= 1.36.0
BuildRequires: libsidplay-devel >= 1.36.0

%description -n gstreamer-sid
Plug-in for playback of C64 SID format music files.

%files -n gstreamer-sid
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstsid.so

%post -n gstreamer-sid
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-sid
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### SHOUT ###
%package -n gstreamer-icecast
Summary:       GStreamer Icecast plug-in using libshout.
Group:         Applications/Multimedia
Requires:      gstreamer-plugins = %{version}
Requires:      libshout >= 1.0.5
BuildRequires: libshout-devel >= 1.0.5

%description -n gstreamer-icecast
Plug-in for broadcasting audio to the Icecast server.

%files -n gstreamer-icecast
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstshout.so

%post -n gstreamer-icecast
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-icecast
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### SWFDEC FLASH PLUGIN ###
%package -n gstreamer-swfdec
Summary:  GStreamer Flash redering plug-in.
Group:    Applications/Multimedia
Requires: gstreamer-plugins = %{version}
Requires: swfdec >= 0.1.2

%description -n gstreamer-swfdec
Plug-in for rendering Flash animations using swfdec library

%files -n gstreamer-swfdec
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstswfdec.so

%post -n gstreamer-swfdec
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null

%postun -n gstreamer-swfdec
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null

### VIDEO 4 LINUX 2 ###
@USE_V4L2_TRUE@%package -n gstreamer-v4l2
@USE_V4L2_TRUE@Summary:       GStreamer Video for Linux 2 plug-in.
@USE_V4L2_TRUE@Group:         Applications/Multimedia
@USE_V4L2_TRUE@Requires:      gstreamer-plugins = %{version}
@USE_V4L2_TRUE@BuildRequires: glibc-devel
@USE_V4L2_TRUE@
@USE_V4L2_TRUE@%description -n gstreamer-v4l2
@USE_V4L2_TRUE@Plug-in for accessing Video for Linux devices.
@USE_V4L2_TRUE@
@USE_V4L2_TRUE@%files -n gstreamer-v4l2
@USE_V4L2_TRUE@%defattr(-, root, root)
@USE_V4L2_TRUE@%{_libdir}/gstreamer-%{majorminor}/libgstvideo4linux2.so
@USE_V4L2_TRUE@
@USE_V4L2_TRUE@%post -n gstreamer-v4l2
@USE_V4L2_TRUE@%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
@USE_V4L2_TRUE@### %{_bindir}/gst-compprep > /dev/null 2> /dev/null
@USE_V4L2_TRUE@
@USE_V4L2_TRUE@%postun -n gstreamer-v4l2
@USE_V4L2_TRUE@%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
@USE_V4L2_TRUE@### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

### XVID ###
%package -n gstreamer-xvid
Summary:       GStreamer XVID plug-in.
Group:         Applications/Multimedia
Requires:      gstreamer-plugins = %{version}
BuildRequires: glibc-devel

%description -n gstreamer-xvid
Plug-in for decoding XVID files.

%files -n gstreamer-xvid
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstxvid.so

%post -n gstreamer-xvid
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null

%postun -n gstreamer-xvid
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null
### %{_bindir}/gst-compprep > /dev/null 2> /dev/null


%changelog
* Mon Mar 15 2004 Thomas Vander Stichele <thomas at apestaart dot org>
- put back media-info
- add ffmpegcolorspace plugin

* Sun Mar 07 2004 Christian Schaller <Uraeus@gnome.org>
- Remove rm commands for media-info stuff
- Add libdir/*
                                                                                
* Thu Mar 04 2004 Christian Schaller <Uraeus@gnome.org>
- Add missing gconf schema install in %post

* Tue Mar 02 2004 Thomas Vander Stichele <thomas at apestaart dot org>
- Libraries/Multimedia doesn't exist, remove it

* Tue Mar 02 2004 Thomas Vander Stichele <thomas at apestaart dot org>
- added speex plugin.

* Mon Mar 01 2004 Thomas Vander Stichele <thomas at apestaart dot org>
- Cleaned up the mess.  Could we PLEASE keep this sort of organized and
- alphabetic for easy lookup ?

* Fri Feb 13 2004 Christian Schaller <Uraeus@gnome.org>
- Added latest new headers

* Wed Jan 21 2004 Christian Schaller <Uraeus@gnome.org>
- added NAS plugin
- added i18n locale dir

* Fri Jan 16 2004 Christian Schaller <uraeus@gnome.org>
- added libcaca plugin
- added libgstcolorspace - fixed name of libgsthermescolorspace

* Wed Jan 14 2004 Christian Schaller <uraeus@gnome.org>
- Add gamma plugin
- Have the pixbuf plugin deleted for now

* Wed Dec 18 2003 Christian Schaller <Uraeus@gnome.org>
- remove gsttagediting.h as it is gone
- replace it with gst/tag/tag.h

* Sun Nov 23 2003 Christian Schaller <Uraeus@gnome.org>
- Update spec file for latest changes
- add faad plugin

* Thu Oct 16 2003 Christian Schaller <Uraeus@gnome.org>
- Add new colorbalance and tuner and xoverlay stuff
- Change name of kde-audio-devel to arts-devel

* Sat Sep 27 2003 Christian Schaller <Uraeus@gnome.org>
- Add majorminor to man page names
- add navigation lib to package

* Tue Sep 11 2003 Christian Schaller <Uraeus@gnome.org>
- Add -%{majorminor} to each instance of gst-register

* Tue Aug 19 2003 Christian Schaller <Uraeus@Gnome.org>
- Add new plugins

* Sat Jul 12 2003 Thomas Vander Stichele <thomas at apestaart dot org>
- move gst/ mpeg plugins to base package
- remove hermes conditional from snapshot
- remove one instance of resample plugin
- fix up silly versioned plugins efence and rmdemux

* Sat Jul 05 2003 Christian Schaller <Uraeus@gnome.org>
- Major overhaul of SPEC file to make it compatible with what Red Hat ships
  as default
- Probably a little less sexy, but cross-distro SPEC files are a myth anyway
  so making it convenient for RH users wins out
- Keeping conditionals even with new re-org so that developers building the
  RPMS don't need everything installed
- Add bunch of obsoletes to ease migration from earlier official GStreamer RPMS
- Remove plugins that doesn't exist anymore

* Sun Mar 02 2003 Christian Schaller <Uraeus@gnome.org>
- Remove USE_RTP statement from RTP plugin
- Move RTP plugin to no-deps section

* Sat Mar 01 2003 Christian Schaller <Uraeus@gnome.org>
- Remove videosink from SPEC
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
- add gst-register-%{majorminor} calls everywhere again since auto-reregister doesn't work
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
- removed all gst-register-%{majorminor} calls since this should be done automatically now

* Thu Jul 04 2002 Thomas Vander Stichele <thomas@apestaart.org>
- fix issue with SDL package
- make all packages STRICTLY require the right version to avoid
  ABI issues
- make gst-plugins obsolete gst-plugin-libs
- also send output of gst-register-%{majorminor} to /dev/null to lower the noise

* Wed Jul 03 2002 Thomas Vander Stichele <thomas@apestaart.org>
- require glibc-devel instead of glibc-kernheaders since the latter is only
  since 7.3 and glibc-devel pulls in the right package anyway

* Sun Jun 23 2002 Thomas Vander Stichele <thomas@apestaart.org>
- changed header location of plug-in libs

* Mon Jun 17 2002 Thomas Vander Stichele <thomas@apestaart.org>
- major cleanups
- adding gst-register-%{majorminor} on postun everywhere
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
