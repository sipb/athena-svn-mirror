bin_PROGRAMS = festival-synthesis-driver
serverinfodir = $(libdir)/bonobo/servers
serverinfo_DATA = GNOME_Speech_SynthesisDriver_Festival.server

INCLUDES = -I$(top_srcdir)          \
	-I$(top_builddir)        \
	-I$(top_srcdir)/gnome-speech\
	$(gnome_speech_CFLAGS) \
	$(WARN_CFLAGS)

LDADD = $(top_builddir)/gnome-speech/libgnomespeech.la $(gnome_speech_LIBS)

festival_synthesis_driver_SOURCES = \
	festivalsynthesisdriver.c \
	festivalsynthesisdriver.h \
	festivalspeaker.c \
	festivalspeaker.h

EXTRA_DIST = GNOME_Speech_SynthesisDriver_Festival.server.in $(serverinfo_DATA)
CLEANFILES = $(serverinfo_DATA)
