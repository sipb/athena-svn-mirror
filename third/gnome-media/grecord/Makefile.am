SUBDIRS = src doc

EXTRA_DIST = \
	gnome-sound-recorder.desktop.in README COPYING AUTHORS INSTALL \
	pixmaps/gnome_mixer.xpm \
	$(pixmap_DATA)		\
	$(Icon_DATA)		\
	gnome-grecord.png	\
	gnome-sound-recorder.schemas.in

@INTLTOOL_DESKTOP_RULE@
@INTLTOOL_SCHEMAS_RULE@

Applicationsdir = $(datadir)/applications
Applications_in_files = gnome-sound-recorder.desktop.in 
Applications_DATA = $(Applications_in_files:.desktop.in=.desktop)


Icondir = $(datadir)/pixmaps
Icon_DATA = gnome-grecord.png

pixmapdir = $(datadir)/pixmaps/gnome-media/gnome-sound-recorder
pixmap_DATA = media-rec.png		\
	      media-play.png		\
	      media-stop.png

schemadir=$(sysconfdir)/gconf/schemas
schema_DATA=gnome-sound-recorder.schemas

install-schemas:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schema_DATA)

install-data-local: install-schemas

