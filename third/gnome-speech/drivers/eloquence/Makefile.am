serverinfodir = $(libdir)/bonobo/servers
serverinfo_in_files = GNOME_Speech_SynthesisDriver_Eloquence.server.in
if BUILD_ELOQUENCE_DRIVER
bin_PROGRAMS = eloquence-synthesis-driver
serverinfo_DATA = $(serverinfo_in_files:.server.in=.server)
endif

INCLUDES = -I$(top_srcdir)          \
	-I$(top_builddir)        \
	-I$(top_srcdir)/gnome-speech\
	$(gnome_speech_CFLAGS) \
	$(WARN_CFLAGS) \
	-I$(eloquence_INCLUDEDIR)

LDADD = $(top_builddir)/gnome-speech/libgnomespeech.la 	\
	$(gnome_speech_LIBS)	\
	-L$(eloquence_LIBDIR)	\
	-leci	 \
	-lstdc++

eloquence_synthesis_driver_SOURCES = \
	eloquencesynthesisdriver.c \
	eloquencesynthesisdriver.h \
	eloquencespeaker.c \
	eloquencespeaker.h

EXTRA_DIST = GNOME_Speech_SynthesisDriver_Eloquence.server.in GNOME_Speech_SynthesisDriver_Eloquence.server 
CLEANFILES = $(serverinfo_DATA)
