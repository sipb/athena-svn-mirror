SUBDIRS = src doc

EXTRA_DIST = \
	README COPYING AUTHORS INSTALL \
	gnome-sound-recorder.desktop.in \
	pixmaps/gnome_mixer.xpm \
	$(pixmap_DATA)		\
	$(Icon_DATA)		\
	gnome-grecord.png	\
	gnome-sound-recorder.schemas.in	\
	gsr.xml

@INTLTOOL_DESKTOP_RULE@
@INTLTOOL_SCHEMAS_RULE@

Applicationsdir = $(datadir)/applications
Applications_in_files = gnome-sound-recorder.desktop.in 
Applications_DATA = $(Applications_in_files:.desktop.in=.desktop)


Icondir = $(datadir)/pixmaps
Icon_DATA = gnome-grecord.png

pixmapdir = $(datadir)/pixmaps/gnome-media/gnome-sound-recorder
pixmap_DATA = media-record.png		\
	      media-play.png		\
	      media-stop.png

uidir = $(datadir)/gnome-sound-recorder/ui
ui_DATA = gsr.xml

schemadir=$(GCONF_SCHEMA_FILE_DIR)
schema_DATA=gnome-sound-recorder.schemas

CLEANFILES=$(Applications_DATA) $(schema_DATA)
install-schemas:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schema_DATA) || true

if GCONF_SCHEMAS_INSTALL
install-data-local: install-schemas
else
install-data-local:
endif

