Audiodir = $(datadir)/applications
pixmapdir = $(datadir)/pixmaps

EXTRA_DIST = README TODO NEWS gnome-volume-control.desktop.in gnome-mixer.png \
	device_labels_linux.sed device_labels_linux.h \
	device_labels_alsa.sed device_labels_alsa.h	\
	gnome-volume-control.schemas

INCLUDES = \
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
	$(MEDIA_CFLAGS) 				\
	-DDATADIR=\""$(datadir)"\"			\
	-DGNOME_ICONDIR=\""$(pixmapdir)"\"

SUBDIRS = doc

if BUILD_GMIX
bin_PROGRAMS = gnome-volume-control

gnome_volume_control_SOURCES =	\
	gmix.c gmix.h prefs.c prefs.h

gnome_volume_control_LDADD = \
	$(MEDIA_LIBS) $(INTLLIBS) $(GMIX_LIBS)
endif

@INTLTOOL_DESKTOP_RULE@
@INTLTOOL_SCHEMAS_RULE@

Audio_in_files = gnome-volume-control.desktop.in
Audio_DATA = $(Audio_in_files:.desktop.in=.desktop)


pixmap_DATA = gnome-mixer.png

if BUILD_GMIX
schemadir=$(sysconfdir)/gconf/schemas
schema_DATA=gnome-volume-control.schemas

install-schemas:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)

install-data-local: install-schemas
endif
