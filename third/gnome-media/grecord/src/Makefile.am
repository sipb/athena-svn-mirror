## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-I$(top_srcdir)/intl \
	$(MEDIA_CFLAGS)		\
	-DDATADIR=\""$(datadir)"\"		\
	-DGNOME_ICONDIR=\""$(datadir)/pixmaps"\"

bin_PROGRAMS = gnome-sound-recorder

gnome_sound_recorder_SOURCES = \
	main.c \
	gui.c gui.h \
	grec.c grec.h \
	preferences.c preferences.h \
	sound.c sound.h \
	prog.h

gnome_sound_recorder_LDADD = $(MEDIA_LIBS) $(INTLLIBS)

