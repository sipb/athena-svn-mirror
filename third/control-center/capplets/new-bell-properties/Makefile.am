Applicationsdir = $(datadir)/control-center/Desktop
Applications_DATA = \
	bell-properties.desktop

Gladedir = $(datadir)/control-center-data
Glade_DATA = \
	bell-properties.glade

glade_msgs = \
	bell-properties.glade.h

EXTRA_DIST = $(Applications_DATA) $(Glade_DATA) $(glade_msgs)

INCLUDES = \
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\" 	\
	-DGNOME_ICONDIR=\""${prefix}/share/pixmaps"\"	\
	-DG_LOG_DOMAIN=\"bell-properties\"	\
	-DGLADE_DATADIR=\""$(Gladedir)"\"		\
	$(GNOME_INCLUDEDIR)				\
	$(XML_CFLAGS)					\
	$(LIBGLADE_CFLAGS)				\
	-I$(top_srcdir)/				\
	-I$(top_srcdir)/intl				\
	-I$(top_srcdir)/libcapplet

bin_PROGRAMS = bell-properties-capplet

bell_properties_capplet_SOURCES = \
	prefs-widget.c prefs-widget.h \
	preferences.c preferences.h \
	main.c

bell_properties_capplet_LDADD = \
	$(GNOME_LIBDIR)					\
	$(GNOMEUI_LIBS)					\
	$(ORBIT_LIBS)					\
	$(GNOME_XML_LIB)				\
	$(LIBGLADE_LIBS)				\
	-lgdk_pixbuf					\
	$(INTLLIBS)					\
	../../libcapplet/libcapplet.la
