# WHAT YOU NEED TO CHANGE: Change all the filenames to refer to your
# component.  Also add whatever libraries, include paths, etc you want,
# and change the G_LOG_DOMAIN.

INCLUDES =						\
	-DG_DISABLE_DEPRECATED				\
	-DGTK_DISABLE_DEPRECATED			\
	-DGNOME_DISABLE_DEPRECATED			\
	-DG_LOG_DOMAIN=\"Nautilus-Audio-Properties\"	\
	-DDATADIR=\"$(datadir)\"			\
	-I$(top_srcdir)					\
	-I$(top_builddir)				\
	$(DISABLE_DEPRECATED_CFLAGS)			\
	$(NAUTILUS_CFLAGS)				\
	$(GST_CFLAGS)

# a bunch of test programs
noinst_PROGRAMS = gnome-test

gnome_test_SOURCES =					\
	audio-properties-view.h          		\
	audio-properties-view.c          		\
	i18n-support.h					\
	gnome-test.c

gnome_test_LDADD = ../media-info/libgstmedia-info.la \
		   $(GST_GCONF_LIBS) $(GNOME_LIBS) $(GST_LIBS)

# works around .o files being created with and without libtool
gnome_test_CFLAGS = $(AM_CFLAGS)

bonobodir = $(libdir)/bonobo
bonobo_LTLIBRARIES = libnautilus-audio-properties-view.la

libnautilus_audio_properties_view_la_SOURCES =		\
	audio-properties-view.h          		\
	audio-properties-view.c          		\
	nautilus-audio-properties-view.h          	\
	nautilus-audio-properties-view.c

libnautilus_audio_properties_view_la_LIBADD =		\
	$(NAUTILUS_LIBS) 				\
	$(GST_LIBS) $(GST_GCONF_LIBS) 				\
	../media-info/libgstmedia-info.la

libnautilus_audio_properties_view_la_LDFLAGS = -module -avoid-version
# works around .o files being created with and without libtool
libnautilus_audio_properties_view_la_CFLAGS = $(AM_CFLAGS)

gladedir = $(datadir)/nautilus/glade
glade_DATA = audio-properties-view.glade

server_in_files = Nautilus_View_audio_properties.server.in.in

serverdir = $(libdir)/bonobo/servers
server_DATA = $(server_in_files:.server.in.in=.server)
@INTLTOOL_SERVER_RULE@

EXTRA_DIST = $(server_in_files) $(glade_DATA)
CLEANFILES = $(server_DATA) $(server_DATA).in
