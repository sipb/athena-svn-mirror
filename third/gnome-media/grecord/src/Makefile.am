## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-I$(top_srcdir) \
	-I$(top_srcdir)/intl \
	$(GSR_CFLAGS)		\
	-DDATADIR=\""$(datadir)"\"		\
	-DGSR_DATADIR=\""$(datadir)/gnome-sound-recorder"\"	\
	-DGNOME_ICONDIR=\""$(datadir)/pixmaps"\"

bin_PROGRAMS = gnome-sound-recorder

#gnome_sound_recorder_SOURCES = \
#	main.c \
#	gui.c gui.h \
#	grec.c grec.h \
#	preferences.c preferences.h \
#	sound.c sound.h \
#	prog.h

gnome_sound_recorder_SOURCES = \
	gnome-recorder.c	\
	gnome-recorder.h	\
	gsr-window.c		\
	gsr-window.h

gnome_sound_recorder_LDADD = $(GSR_LIBS) $(INTLLIBS) $(top_builddir)/profiles/libgnome-media-profiles.la


