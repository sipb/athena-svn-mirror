bin_PROGRAMS = gstreamer-properties

gstreamer_properties_LDADD = $(GSTPROPS_LIBS)
gstreamer_properties_SOURCES = \
	gstreamer-properties.c gstreamer-properties-structs.h \
	pipeline-tests.h pipeline-tests.c \
	pipeline-constants.c
	
@INTLTOOL_DESKTOP_RULE@

iconsdir   = $(GSTPROPS_ICONS_DIR)
icons_DATA = gstreamer-properties.png

desktop_iconsdir = $(datadir)/pixmaps
desktop_icons_DATA = gstreamer-properties.png

gladedir   = $(GSTPROPS_GLADE_DIR)
glade_DATA = gstreamer-properties.glade 

#desktopdir = $(GNOMECC_DESKTOP_DIR)
#Desktop_in_files = gstreamer-properties.desktop.in
#desktop_DATA = $(Desktop_in_files:.desktop.in=.desktop)

INCLUDES   = $(GSTPROPS_CFLAGS)
AM_CFLAGS  = -DG_LOG_DOMAIN="gstreamer-properties" \
             -DGNOMELOCALEDIR="${prefix}/${DATADIRNAME}/locale"

CLEANFILES = $(GSTPROPS_CLEANFILES)
EXTRA_DIST = $(icons_DATA) $(glade_DATA) # $(Desktop_in_files)
