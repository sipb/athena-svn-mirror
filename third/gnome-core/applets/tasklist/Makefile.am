SUBDIRS = help

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../panel				\
	-I$(srcdir)/../desk-guide			\
	-I$(top_builddir)/panel \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	@PIXBUF_CFLAGS@

bin_PROGRAMS = tasklist_applet
#lib_LTLIBRARIES = libtasklist_applet.la

tasklist_applet_SOURCES = 	\
	pixmaps.h		\
	tasklist_applet.c 	\
	tasklist_applet.h	\
	tasklist_config.c 	\
	tasklist_icon.c 	\
	tasklist_menu.c 	\
	tasklist_properties.c 	\
	gstc.c 		\
	gwmh.c


#libtasklist_applet_la_SOURCES =\
#	pixmaps.h 		\
#	tasklist_applet.h 	\
#	tasklist_applet.c 	\
#	tasklist_config.c 	\
#	tasklist_icon.c 	\
#	tasklist_menu.c 	\
#	tasklist_properties.c 	\
#	gstc.c 			\
#	gwmh.c

#libtasklist_applet_la_LIBADD =				\

tasklist_applet_LDADD = 			\
	../../panel/libpanel_applet.la 		\
	$(GNOME_LIBDIR) $(ORB_LIBS)		\
	$(GNOMEUI_LIBS) $(GNORBA_LIBS)		\
	$(INTLLIBS) @PIXBUF_LIBS@

EXTRA_DIST = 			\
	tasklist_applet.gnorba 	\
	tasklist_applet.desktop	\
	unknown.xpm		\
	gnome-tasklist.png

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-tasklist.png

sysdir = $(datadir)/applets/Utility
sys_DATA = tasklist_applet.desktop 

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = tasklist_applet.gnorba 

$(srcdir)/gstc.c: $(srcdir)/../desk-guide/gstc.c
	cp -f $(srcdir)/../desk-guide/gstc.c $(srcdir)

$(srcdir)/gwmh.c: $(srcdir)/../desk-guide/gwmh.c
	cp -f $(srcdir)/../desk-guide/gwmh.c $(srcdir)

CLEANFILES += gwmh.c gstc.c
