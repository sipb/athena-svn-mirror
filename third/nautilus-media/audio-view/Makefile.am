# WHAT YOU NEED TO CHANGE: Change all the filenames to refer to your
# component.  Also add whatever libraries, include paths, etc you want,
# and change the G_LOG_DOMAIN.

INCLUDES =						\
	-DG_DISABLE_DEPRECATED				\
	-DGTK_DISABLE_DEPRECATED			\
	-DGNOME_DISABLE_DEPRECATED			\
	-DG_LOG_DOMAIN=\"Nautilus-Audio\"		\
	-DDATADIR=\"$(datadir)\"			\
	-I$(top_srcdir)					\
	-I$(top_builddir)				\
	$(DISABLE_DEPRECATED_CFLAGS)			\
	$(NAUTILUS_CFLAGS)				\
	$(GST_CFLAGS)

LIBS = $(GST_LIBS)

# a bunch of test programs
noinst_PROGRAMS = bonobo-test gnome-test audio-play-test

bonobo_test_SOURCES =					\
	bonobo-test.c

bonobo_test_LDADD = $(NAUTILUS_LIBS)

gnome_test_SOURCES =					\
	audio-play.h          				\
	audio-play.c          				\
	audio-view.h          				\
	audio-view.c          				\
	i18n-support.h					\
	gnome-test.c

gnome_test_LDADD = ../media-info/libgstmedia-info.la \
		   $(GST_GCONF_LIBS) $(GNOME_LIBS)

audio_play_test_SOURCES =				\
	audio-play.h          				\
	audio-play.c          				\
	audio-play-test.c

audio_play_test_LDADD =	$(GST_GCONF_LIBS)

libexec_PROGRAMS = nautilus-audio-view

# the main view
nautilus_audio_view_SOURCES =				\
	audio-play.h          				\
	audio-play.c          				\
	audio-view.h          				\
	audio-view.c          				\
	nautilus-audio-view.h          			\
	nautilus-audio-view.c          			\
	main.c

nautilus_audio_view_LDADD = $(GST_GCONF_LIBS) $(NAUTILUS_LIBS) \
                   ../media-info/libgstmedia-info.la

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = nautilus-audio-view-ui.xml

server_in_files = Nautilus_View_audio.server.in.in

serverdir = $(libdir)/bonobo/servers

# this next line is Makefile magic that derives the contents of server_DATA
# by taking the value of $server_in_files 
# and replacing .server.in.in with .server
server_DATA = $(server_in_files:.server.in.in=.server)

@INTLTOOL_SERVER_RULE@

EXTRA_DIST = $(server_in_files) $(ui_DATA) 
CLEANFILES = $(server_DATA) $(server_DATA).in
