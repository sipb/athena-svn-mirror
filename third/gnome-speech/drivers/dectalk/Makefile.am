serverinfodir = $(libdir)/bonobo/servers
serverinfo_in_files=GNOME_Speech_SynthesisDriver_Dectalk.server.in
if BUILD_DECTALK_DRIVER
bin_PROGRAMS = dectalk-synthesis-driver
serverinfo_DATA = $(serverinfo_in_files:.server.in=.server)
endif

INCLUDES = -I$(top_srcdir)          \
	-I$(top_builddir)        \
	-I$(top_srcdir)/gnome-speech\
	$(gnome_speech_CFLAGS) \
	$(WARN_CFLAGS) \
	$(dectalk_INCLUDES)

LDADD = $(top_builddir)/gnome-speech/libgnomespeech.la $(gnome_speech_LIBS) $(dectalk_LIBS)

dectalk_synthesis_driver_SOURCES = \
	dectalksynthesisdriver.c \
	dectalksynthesisdriver.h \
	dectalkspeaker.c \
	dectalkspeaker.h

EXTRA_DIST = $(serverinfo_in_files)
CLEANFILES = $(serverinfo_DATA)
