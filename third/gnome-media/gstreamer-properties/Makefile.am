SUBDIRS = help
bin_PROGRAMS = gstreamer-properties

gstreamer_properties_LDADD = $(GSTPROPS_LIBS)
gstreamer_properties_SOURCES = \
	gstreamer-properties.c gstreamer-properties-structs.h \
	pipeline-tests.h pipeline-tests.c \
	pipeline-constants.c
	
@INTLTOOL_DESKTOP_RULE@

iconsdir   = $(datadir)/gstreamer-properties/icons
icons_DATA = gstreamer-properties.png

desktop_iconsdir = $(datadir)/pixmaps
desktop_icons_DATA = gstreamer-properties.png

gladedir   = $(datadir)/gstreamer-properties/glade
glade_DATA = gstreamer-properties.glade 

desktopdir = $(datadir)/control-center-2.0/capplets
Desktop_in_files = gstreamer-properties.desktop.in
desktop_DATA = $(Desktop_in_files:.desktop.in=.desktop)

INCLUDES   = \
	$(GSTPROPS_CFLAGS) \
	-DGSTPROPS_GLADEDIR=\""$(gladedir)"\" \
	-DGSTPROPS_ICONDIR=\""$(iconsdir)"\" \
	-DGNOMELOCALEDIR="${prefix}/${DATADIRNAME}/locale" \
	-DG_LOG_DOMAIN="gstreamer-properties"

CLEANFILES = $(GSTPROPS_CLEANFILES) $(desktop_DATA)
EXTRA_DIST = $(icons_DATA) $(glade_DATA) $(Desktop_in_files)
