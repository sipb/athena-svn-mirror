Audiodir = $(datadir)/applications

pixmapdir = $(datadir)/pixmaps

INCLUDES = \
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	$(VUMETER_CFLAGS)			\
	-DGNOME_ICONDIR=\""$(pixmapdir)"\"

bin_PROGRAMS = vumeter

vumeter_SOURCES = \
	gtkled.c	\
	gtkled.h	\
	gtkledbar.c	\
	gtkledbar.h	\
	main.c

vumeter_LDADD = \
	$(VUMETER_LIBS) $(INTLLIBS)

BITMAPS = gnome-vumeter.png gnome-reclevel.png

pixmap_DATA = $(BITMAPS)

EXTRA_DIST = $(BITMAPS) vumeter.desktop.in reclevel.desktop.in README TODO

@INTLTOOL_DESKTOP_RULE@
Audio_in_files = vumeter.desktop.in reclevel.desktop.in
Audio_DATA = $(Audio_in_files:.desktop.in=.desktop)

CLEANFILES = $(Audio_DATA)
