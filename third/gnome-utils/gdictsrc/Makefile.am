## Process this file with automake to produce Makefile.in

SUBDIRS = doc

INCLUDES = -DG_LOG_DOMAIN=\"gdict\"			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = gdict

gdict_SOURCES = \
	gdict-about.c gdict-about.h \
	gdict-pref.c gdict-pref.h \
        gdict-pref-dialog.c gdict-pref-dialog.h \
	gdict-app.c gdict-app.h \
	gdict-applet.c gdict-applet.h \
	gdict-defbox.c gdict-defbox.h \
	gdict-speller.c gdict-speller.h \
	dict.c dict.h \
	main.c

gdict_LDADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(GNOME_APPLETS_LIBS) $(GNOME_PRINT_LIBS) $(INTLLIBS)

EXTRA_DIST = \
	GDict.desktop \
	gdict.desktop \
	gdict.gnorba \
	gdict.png \
	$(top_srcdir)/app-docs.make

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = gdict.png

gdictgnorbadir  = $(sysconfdir)/CORBA/servers
gdictgnorba_DATA = gdict.gnorba

gdictappletdir  = $(datadir)/applets/Utility
gdictapplet_DATA = gdict.desktop

gdictappdir  = $(datadir)/gnome/apps/Utilities
gdictapp_DATA = GDict.desktop
