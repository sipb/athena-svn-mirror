serverinfodir = $(libdir)/bonobo/servers
serverinfo_in_files = GNOME_Speech_SynthesisDriver_Theta.server.in
if BUILD_THETA_DRIVER
bin_PROGRAMS = theta-synthesis-driver
serverinfo_DATA = $(serverinfo_in_files:.server.in=.server)
endif

INCLUDES = -I$(top_srcdir)          \
	-I$(top_builddir)           \
	-I$(top_srcdir)/gnome-speech\
	$(gnome_speech_CFLAGS)	    \
	$(WARN_CFLAGS)              \
	$(theta_INCLUDES)

LDADD = $(top_builddir)/gnome-speech/libgnomespeech.la $(gnome_speech_LIBS) $(theta_LIBS)

theta_synthesis_driver_SOURCES = \
	thetasynthesisdriver.c \
	thetasynthesisdriver.h \
	thetaspeaker.c \
	thetaspeaker.h

EXTRA_DIST = $(serverinfo_in_files)
CLEANFILES = $(serverinfo_DATA)
