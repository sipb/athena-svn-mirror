serverinfodir = $(libdir)/bonobo/servers
serverinfo_in_files = GNOME_Speech_SynthesisDriver_Viavoice.server.in
if BUILD_VIAVOICE_DRIVER
bin_PROGRAMS = viavoice-synthesis-driver
serverinfo_DATA = $(serverinfo_in_files:.server.in=.server)
endif

INCLUDES = -I$(top_srcdir)          \
	-I$(top_builddir) \
	-I$(top_srcdir)/gnome-speech \
	$(viavoice_INCLUDES) \
	$(WARN_CFLAGS) \
	$(gnome_speech_CFLAGS)

LDADD = $(top_builddir)/gnome-speech/libgnomespeech.la	\
	$(gnome_speech_LIBS) \
	$(viavoice_LIBS)


viavoice_synthesis_driver_SOURCES = \
	viavoicesynthesisdriver.c \
	viavoicesynthesisdriver.h \
	viavoicespeaker.c \
	viavoicespeaker.h

EXTRA_DIST = $(serverinfo_in_files)
