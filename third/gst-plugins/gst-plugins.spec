# This SPEC file is created in a way that tries to solve various demands. 
# First of all it tries to create packages that will easily replace both the 
# Fedora Core default packages and also replace the extra rpms provided 
# by Fedora.us or freshrpms.net.
# At the same time they will only include plugins for which you have the needed
# packages installed at the time you run autogen.sh. This means that if you
# are not careful you might end up with less plugins than what the standard 
# packages provide, which in turn means things might stop working for you. 
# So make sure you have an idea of what you do before creating RPMS using this 
# SPEC file.

%define         register        %{_bindir}/gst-register-%{majorminor} > /dev/null 2>&1 || :
%define         gst_minver      0.7.6
%define         gstp_minver     0.7.6

Name: 		gstreamer-plugins
Version: 	0.8.7
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
Requires: 	gstreamer >= %{gst_minver}
BuildRequires: 	gstreamer-devel >= %{gst_minver}
BuildRequires:	gstreamer-tools >= %{gst_minver}
BuildRequires:  gcc-c++
BuildRequires:  XFree86-devel

#Requires:      arts >= 1.1.4
#BuildRequires: arts-devel >= 1.1.4
#BuildRequires: gcc-c++
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
# @USE_RAW1394_TRUE@Requires:      libraw1394
# @USE_RAW1394_TRUE@BuildRequires: libraw1394-devel
Requires:      SDL >= 1.2.0
BuildRequires: SDL-devel >= 1.2.0
#SDL-devel should require XFree86-devel because it links to it
#only it doesn't seem to do that currently
BuildRequires: 	XFree86-devel
Requires:	speex
BuildRequires:	libspeex-devel
Requires:	gtk2
BuildRequires:	gtk2-devel
Requires:      libogg >= 1.0
Requires:      libvorbis >= 1.0
BuildRequires: libogg-devel >= 1.0
BuildRequires: libvorbis-devel >= 1.0
Requires: 	XFree86-libs
BuildRequires: XFree86-devel
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
%{_libdir}/gstreamer-%{majorminor}/libgstapetag.so
%{_libdir}/gstreamer-%{majorminor}/libgsttta.so
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
%{_libdir}/gstreamer-%{majorminor}/libgstalpha.so
%{_libdir}/gstreamer-%{majorminor}/libgstalphacolor.so
%{_libdir}/gstreamer-%{majorminor}/libgstaudiorate.so
%{_libdir}/gstreamer-%{majorminor}/libgstdecodebin.so
%{_libdir}/gstreamer-%{majorminor}/libgstmultifilesink.so
%{_libdir}/gstreamer-%{majorminor}/libgstmultipart.so
%{_libdir}/gstreamer-%{majorminor}/libgstplaybin.so
%{_libdir}/gstreamer-%{majorminor}/libgstvideobox.so
%{_libdir}/gstreamer-%{majorminor}/libgstvideomixer.so
%{_libdir}/gstreamer-%{majorminor}/libgstvideorate.so
%{_libdir}/gstreamer-%{majorminor}/libgsttheora.so
%{_libdir}/gstreamer-%{majorminor}/libgstmng.so
%{_libdir}/gstreamer-%{majorminor}/libgstequalizer.so

# gstreamer-plugins with external dependencies but in the main package
#%{_libdir}/gstreamer-%{majorminor}/libgstarts.so
#%{_libdir}/gstreamer-%{majorminor}/libgstartsdsink.so
%{_libdir}/gstreamer-%{majorminor}/libgstaudiofile.so
%{_libdir}/gstreamer-%{majorminor}/libgstcdparanoia.so
%{_libdir}/gstreamer-%{majorminor}/libgstesd.so
#%{_libdir}/gstreamer-%{majorminor}/libpolypaudio.so
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
#@USE_RAW1394_TRUE@%{_libdir}/gstreamer-%{majorminor}/libgst1394.so
# Snapshot plugin uses libpng
%{_libdir}/gstreamer-%{majorminor}/libgstsnapshot.so
%{_libdir}/gstreamer-%{majorminor}/libgsttextoverlay.so
%{_libdir}/gstreamer-%{majorminor}/libgsttimeoverlay.so
%{_libdir}/gstreamer-%{majorminor}/libgstspeex.so
#%{_libdir}/gstreamer-%{majorminor}/libgstcacasink.so
%{_libdir}/gstreamer-%{majorminor}/libgstximagesink.so
%{_libdir}/gstreamer-%{majorminor}/libgstxvimagesink.so
#%{_libdir}/gstreamer-%{majorminor}/libgstsndfile.so
%{_libdir}/gstreamer-%{majorminor}/libgsttrm.so

# Docs
%{_datadir}/locale
%{_datadir}/gtk-doc/html

%post
export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
gconftool-2 --makefile-install-rule %{_sysconfdir}/gconf/schemas/gstreamer-%{majorminor}.schemas > /dev/null
%{_bindir}/gst-register-%{majorminor} > /dev/null 2> /dev/null

%package audio
Summary:        Additional audio plugins for GStreamer
Group:          Applications/Multimedia
                                                                                
BuildRequires:  libsidplay-devel >= 1.36.0
#BuildRequires:  libshout-devel <= 2.0
# #BuildRequires: libshout-devel >= 2.0
BuildRequires:  ladspa-devel
                                                                                
Requires:       gstreamer-plugins >= %{gstp_minver}
Requires(pre):  %{_bindir}/gst-register-%{majorminor}
Requires(post): %{_bindir}/gst-register-%{majorminor}

Provides:       gstreamer-ladspa = %{version}-%{release}
Provides:       gstreamer-sid = %{version}-%{release}
#Provides:       gstreamer-shout = %{version}-%{release}
                                                                                
%description audio
This package contains additional audio plugins for GStreamer, including
- codec for sid (C64)
- a shout element to stream to icecast servers
- a ladspa elements wrapping LADSPA plugins
# - a shout 2 element

%files audio
%defattr(-, root, root, -)
%{_libdir}/gstreamer-%{majorminor}/libgstladspa.so
%{_libdir}/gstreamer-%{majorminor}/libgstsid.so
#%{_libdir}/gstreamer-%{majorminor}/libgstshout.so
# #%{_libdir}/gstreamer-%{majorminor}/libgstshout2.so

%post audio
%{register}
%postun audio
%{register}

%package extra-audio
Summary:        Extra audio plugins for GStreamer
Group:          Applications/Multimedia
                                                                                
#BuildRequires:  faad2-devel >= 2.0
BuildRequires:  gsm-devel >= 1.0.10
BuildRequires:  lame-devel >= 3.89
BuildRequires:  libid3tag-devel >= 0.15.0
BuildRequires:  libmad-devel >= 0.15.0
                                                                                
Requires:       gstreamer-plugins >= %{gstp_minver}
Requires(pre):  %{_bindir}/gst-register-%{majorminor}
Requires(post): %{_bindir}/gst-register-%{majorminor}
                                                                                
#Provides:      gstreamer-faad = %{version}-%{release}
Provides:       gstreamer-gsm = %{version}-%{release}
Provides:      gstreamer-lame = %{version}-%{release}
Provides:       gstreamer-mad = %{version}-%{release}
                                                                                
%description extra-audio
This package contains extra audio plugins for GStreamer, including
- gsm decoding
- faad2 decoding
- mad mp3 decoding
- lame mp3 encoding
                                                                                
%post extra-audio
%{register}
%postun extra-audio
%{register}
                                                                                
%files extra-audio
%defattr(-, root, root, -)
#%{_libdir}/gstreamer-%{majorminor}/libgstfaad.so
%{_libdir}/gstreamer-%{majorminor}/libgstgsm.so
%{_libdir}/gstreamer-%{majorminor}/libgstlame.so
%{_libdir}/gstreamer-%{majorminor}/libgstmad.so

%package extra-dvd
Summary:        DVD plugins for GStreamer
Group:          Applications/Multimedia
                                                                                
BuildRequires:  a52dec-devel >= 0.7.3
BuildRequires:  libdvdnav-devel >= 0.1.3
BuildRequires:  libdvdread-devel >= 0.9.0
                                                                                
Requires:       gstreamer-plugins >= %{gstp_minver}
Requires:       gstreamer-plugins-extra-video >= %{gstp_minver}
Requires(pre):  %{_bindir}/gst-register-%{majorminor}
Requires(post): %{_bindir}/gst-register-%{majorminor}
                                                                                
Provides:       gstreamer-dvd = %{version}-%{release}
Provides:       gstreamer-       = %{version}-%{release}
Provides:       gstreamer-dvdnavsrc = %{version}-%{release}
Provides:       gstreamer-dvdreadsrc = %{version}-%{release}
                                                                                
%description extra-dvd
This package contains dvd plugins for GStreamer, including

- libdvdread
      decoding
                                                                                
%post extra-dvd
%{register}
%postun extra-dvd
%{register}
                                                                                
%files extra-dvd
%defattr(-, root, root, -)
%{_libdir}/gstreamer-%{majorminor}/libgsta52dec.so
%{_libdir}/gstreamer-%{majorminor}/libgstdvdnavsrc.so
%{_libdir}/gstreamer-%{majorminor}/libgstdvdreadsrc.so

%package video
Summary:        Additional video plugins for GStreamer
Group:          Applications/Multimedia
                                                                                
BuildRequires:  aalib-devel >= 1.3
                                                                                
Requires:       gstreamer-plugins >= %{gstp_minver}
Requires(pre):  %{_bindir}/gst-register-%{majorminor}
Requires(post): %{_bindir}/gst-register-%{majorminor}
                                                                                
Provides:       gstreamer-aasink = %{version}-%{release}
                                                                                
%description video
This package contains additional video plugins for GStreamer, including
- an output sink based on aalib (ASCII art output)
- an element for decoding dv streams using libdv
- an output sink based on cacalib (color ASCII art output)
- A Dirac video format decoder
- An output sink based on OpenGL

%files video
%defattr(-, root, root, -)
%{_libdir}/gstreamer-%{majorminor}/libgstaasink.so
%{_libdir}/gstreamer-%{majorminor}/libgstdvdec.so
#%{_libdir}/gstreamer-%{majorminor}/libgstcacasink.so
#%{_libdir}/gstreamer-%{majorminor}/libgstdirac.so
%{_libdir}/gstreamer-%{majorminor}/libgstglimagesink.so

%post video
%{register}
%postun video
%{register}

%package extra-video
Summary:        Extra video plugins for GStreamer
Group:          Applications/Multimedia
                                                                                
BuildRequires:  libfame-devel >= 0.9.0
BuildRequires:  mpeg2dec-devel >= 0.4.0
BuildRequires:  swfdec-devel
                                                                                
Requires:       gstreamer-plugins >= %{gstp_minver}
Requires:       gstreamer-plugins-extra-audio >= %{gstp_minver}
Requires(pre):  %{_bindir}/gst-register-%{majorminor}
Requires(post): %{_bindir}/gst-register-%{majorminor}
                                                                                
Provides:       gstreamer-libfame = %{version}-%{release}
Provides:       gstreamer-mpeg2dec = %{version}-%{release}
Provides:       gstreamer-swfdec = %{version}-%{release}
                                                                                
%description extra-video
This package contains extra video plugins for GStreamer, including
- libfame MPEG video encoding
- mpeg2dec MPEG-2 decoding
- swfdec Flash decoding
                                                                                
%post extra-video
%{register}
%postun extra-video
%{register}
                                                                                
%files extra-video
%defattr(-, root, root, -)
%{_libdir}/gstreamer-%{majorminor}/libgstlibfame.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpeg2dec.so
%{_libdir}/gstreamer-%{majorminor}/libgstmp1videoparse.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpeg1systemencode.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpeg2subt.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpegaudio.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpegaudioparse.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpegstream.so
%{_libdir}/gstreamer-%{majorminor}/libgstswfdec.so

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
%{_includedir}/gstreamer-%{majorminor}/gst/audio/multichannel-enumtypes.h
%{_includedir}/gstreamer-%{majorminor}/gst/audio/multichannel.h
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
%{_includedir}/gstreamer-%{majorminor}/gst/mixer/mixeroptions.h

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
%{_libdir}/libgstinterfaces-%{majorminor}.so

# Here are packages not in the base plugins package but not dependant
# on an external lib

# Here are all the packages depending on external libs #

### ALSA ###
%package -n gstreamer-plugins-alsa
Summary:  GStreamer plug-ins for the ALSA sound system.
Group:    Applications/Multimedia
Requires: gstreamer-plugins = %{version}
Obsoletes:gstreamer-alsa

Provides:	gstreamer-audiosrc
Provides:	gstreamer-audiosink

%description -n gstreamer-plugins-alsa
Input and output plug-in for the ALSA soundcard driver system. 
This plug-in depends on Alsa 0.9.x or higher.

%files -n gstreamer-plugins-alsa
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstalsa.so

%post -n gstreamer-plugins-alsa
%{register}
%postun -n gstreamer-plugins-alsa
%{register}

## DXR3 ###
#%package -n gstreamer-plugins-dxr3
#Summary:       GStreamer plug-in for playback using dxr3 card.
#Group:         Applications/Multimedia
#Requires:      gstreamer-plugins = %{version}
#Requires:      em8300 >= 0.12.0
#BuildRequires: em8300-devel >= 0.12.0
#Obsoletes:     gstreamer-dxr3
#
#%description -n gstreamer-plugins-dxr3
#Plug-in supporting DVD playback using cards
#with the dxr3 chipset like Hollywood Plus
#and Creative Labs DVD cards.
#
#%files -n gstreamer-plugins-dxr3
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgstdxr3.so
#
#%post -n gstreamer-plugins-dxr3
#%{register}
#
#%postun -n gstreamer-plugins-dxr3
#%{register}

### FAAC ###
#%package -n gstreamer-plugins-faac
#Summary:GStreamer plug-ins for AAC audio playback.
#Group:         Applications/Multimedia
#Requires:      gstreamer-plugins = %{version}
#Requires:      faac >= 1.23
#BuildRequires: faac-devel >= 1.23
#Obsoletes:     gstreamer-faac
#
#%description -n gstreamer-plugins-faac
#Plug-ins for playing AAC audio
#
#%files -n gstreamer-plugins-faac
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgstfaac.so
#%post -n  gstreamer-faac
#%{register}
#
#%postun -n  gstreamer-plugins-faac
#%{register}

#### JACK AUDIO CONNECTION KIT ###
#%package -n gstreamer-plugins-jack
#Summary:  GStreamer plug-in for the Jack Sound Server.
#Group:    Applications/Multimedia
#Requires: gstreamer-plugins = %{version}
#Requires: jack-audio-connection-kit >= 0.28.0
#
#Provides:	gstreamer-audiosrc
#Provides:	gstreamer-audiosink
#Obsoletes:	gstreamer-jack
#
#%description -n gstreamer-plugins-jack
#Plug-in for the JACK professional sound server.
#
#%files -n gstreamer-plugins-jack
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgstjack.so
#
#%post -n gstreamer-plugins-jack
#%{register}
#
#%postun -n gstreamer-plugins-jack
#%{register}

#### NETWORK AUDIO SYSTEM  ###
#%package -n gstreamer-plugins-nas
#Summary:  GStreamer plug-in for the Network Audio System.
#Group:    Applications/Multimedia
#Requires: gstreamer-plugins = %{version}
#Requires: libnas2 >= 1.6
#Obsolotes:gstreamer-nas
#
#%description -n gstreamer-plugins-nas
#Plug-in for the Network Audio System sound server.
#
#%files -n gstreamer-plugins-nas
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgstnassink.so
#
#%post -n gstreamer-plugins-nas
#%{register}
#
#%postun -n gstreamer-plugins-nas
#%{register}

#### MMS Protocol support ####
#%package -n gstreamer-plugins-mms
#Summary:  GStreamer plug-in for MMS protocol support 
#Group:    Applications/Multimedia
#Requires: gstreamer-plugins = %{version}
#Requires: libmms >= 0.1
#Obsoletes:gstreamer-mms
#
#%description -n gstreamer-plugins-mms
#Plug-in for the MMS protocol used by Microsoft
#
#%files -n gstreamer-plugins-mms
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgstmms.so
#
#%post -n gstreamer-plugins-mms
#%{register}
#
#%postun -n gstreamer-plugins-mms
#%{register}

### VIDEO 4 LINUX 2 ###
#%package -n gstreamer-plugins-v4l2
#Summary:       GStreamer Video for Linux 2 plug-in.
#Group:         Applications/Multimedia
#Requires:      gstreamer-plugins = %{version}
#BuildRequires: glibc-devel
#Obsoletes:	  gstreamer-v4l2
#
#%description -n gstreamer-plugins-v4l2
#Plug-in for accessing Video for Linux devices.
#
#%files -n gstreamer-plugins-v4l2
#%defattr(-, root, root)
#%{_libdir}/gstreamer-%{majorminor}/libgstvideo4linux2.so
#
#%post -n gstreamer-plugins-v4l2
#%{register}
#
#%postun -n gstreamer-plugins-v4l2
#%{register}

### XVID ###
%package -n gstreamer-plugins-xvid
Summary:       GStreamer XVID plug-in.
Group:         Applications/Multimedia
Requires:      gstreamer-plugins = %{version}
BuildRequires: glibc-devel
Obsoletes:     gstreamer-xvid

%description -n gstreamer-plugins-xvid
Plug-in for decoding XVID files.

%files -n gstreamer-plugins-xvid
%defattr(-, root, root)
%{_libdir}/gstreamer-%{majorminor}/libgstxvid.so

%post -n gstreamer-plugins-xvid
%{register}

%postun -n gstreamer-plugins-xvid
%{register}


%changelog
* Wed Dec 22 2004 Christian Schaller <christian at fluendo dot com>
- Add -plugins- to plugin names

* Thu Dec 9  2004 Christian Schaller <christian a fluendo dot com>
- Add the mms plugin

* Wed Oct 06 2004 Christian Schaller <christian at fluendo dot com>
- Add Wim's new mng decoder plugin
- add shout2 plugin for Zaheer, hope it is correctly done :)

* Wed Sep 29 2004 Christian Schaller <uraeus at gnome dot org>
- Fix USE statement for V4L2

* Thu Sep 28 2004 Christian Schaller <uraeus at gnome dot org>
- Remove kio plugin (as it was broken)

* Wed Sep 21 2004 Christian Schaller <uraeus at gnome dot org>
- Reorganize SPEC to fit better with fedora.us and freshrpms.net packages
- Make sure gstinterfaces.so is in the package
- Add all new plugins

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
