confexecdir = $(libdir)
confexec_DATA = cappletConf.sh

INCLUDES = 			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DVERSION=\""$(VERSION)"\"			\
	-DSETTINGS_DIR=\""$(sysdir)"\"			\
	-DPIXMAPS_DIR=\""$(appicondir)"\"

lib_LTLIBRARIES = libcapplet.la

libcapplet_la_SOURCES = \
	capplet-widget.c

include_HEADERS = capplet-widget.h

EXTRA_DIST = cappletConf.sh.in

#
# Create the cappletConf.sh file from the sh.in file
#
cappletConf.sh: cappletConf.sh.in Makefile
	sed -e 's?\@GNOME_LIBDIR\@?$(GNOME_LIBDIR)?g' \
	    -e 's?\@GNOMEUI_LIBS\@?$(GNOMEUI_LIBS)?g' \
	    -e 's?\@GNOME_INCLUDEDIR\@?$(GNOME_INCLUDEDIR)?g' \
	    -e 's?\@ORB_LIBS\@?$(ORB_LIBS)?g' \
	    < $(srcdir)/cappletConf.sh.in > cappletConf.tmp \
	  && mv cappletConf.tmp cappletConf.sh
