SUBDIRS = help

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-I$(top_builddir)/panel \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DVERSION=\""$(VERSION)"\"			\
	@PIXBUF_CFLAGS@

bin_PROGRAMS = deskguide_applet

deskguide_applet_SOURCES = gstc.c gwmh.c gwmthumbnail.c gwmdesktop.c gwmtaskview.c deskguide_applet.c

deskguide_applet_LDADD = ../../panel/libpanel_applet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS)        \
		     $(GNOMEUI_LIBS) $(GNORBA_LIBS)     \
		     @PIXBUF_LIBS@			\
		     $(INTLLIBS)

EXTRA_DIST = \
	gstc.h \
	gwmh.h \
	gwmdesktop.h \
	gwmtaskview.h \
	gwmthumbnail.h \
	deskguide_applet.h \
	deskguide_applet.gnorba \
	deskguide_applet.desktop \
	gnome-deskguide-splash.png \
	gnome-deskguide.png

sysdir = $(datadir)/applets/Utility
sys_DATA = deskguide_applet.desktop

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-deskguide-splash.png gnome-deskguide.png

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = deskguide_applet.gnorba 
