SUBDIRS = images access doc
INCLUDES =					\
	-I$(top_srcdir)				\
	$(GNOME_INCLUDEDIR)			\
	-I$(top_builddir)			\
	-I$(top_srcdir)/cddb-slave2		\
	$(MEDIA_CFLAGS)				\
	$(GAILUTIL_CFLAGS)			\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
	-DDATADIR=\""$(datadir)"\"		\
	-DGNOME_ICONDIR=\""$(datadir)/pixmaps"\"	\
	-DTHEME_DIR=\""$(datadir)/pixmaps/gnome-cd/themes"\"

Audiodir = $(datadir)/applications
Pixmapdir = $(datadir)/pixmaps
EXTRA_DIST = \
	HACKING \
	solaris-cdrom.c		\
	solaris-cdrom.h		\
	bsd-cdrom.c		\
	bsd-cdrom.h		\
	linux-cdrom.c		\
	linux-cdrom.h		\
	gnome-cd.png gnome-cd.desktop.in gnome-cd.schemas.in

### intermediate libtool library generated from IDL source
IDLS = $(top_srcdir)/cddb-slave2/GNOME_Media_CDDBSlave2.idl

IDL_GENERATED =				\
	GNOME_Media_CDDBSlave2.h	\
	GNOME_Media_CDDBSlave2-common.c	\
	GNOME_Media_CDDBSlave2-skels.c	\
	GNOME_Media_CDDBSlave2-stubs.c

$(IDL_GENERATED): $(IDLS)
	$(ORBIT_IDL) -I $(BONOBO_IDLDIR) -I $(BONOBO_ACT_IDLDIR)	\
	-I $(srcdir) -I $(datadir)/idl	$(IDLS)

noinst_LTLIBRARIES = libgnomemedia-cddbslave2.la
libgnomemedia_cddbslave2_la_SOURCES = $(IDL_GENERATED)

if CDROM_HOST_BSD
cd_sources = bsd-cdrom.c bsd-cdrom.h
endif
if CDROM_HOST_LINUX
cd_sources = linux-cdrom.c linux-cdrom.h
endif
if CDROM_HOST_SOLARIS
cd_sources = solaris-cdrom.c solaris-cdrom.h
endif

bin_PROGRAMS = gnome-cd
gnome_cd_SOURCES =		\
	eggtrayicon.c		\
	eggtrayicon.h		\
	gnome-cd.c		\
	gnome-cd.h		\
	gnome-cd-type.h		\
	callbacks.c		\
	callbacks.h		\
	cddb.c			\
	cddb.h			\
	cdrom.c			\
	cdrom.h			\
	display.c		\
	display.h		\
	$(cd_sources)		\
	preferences.c		\
	preferences.h		\
	cd-selection.c		\
	cd-selection.h		\
	theme.c

ACCESS_LT_LIB = $(top_builddir)/gnome-cd/access/libgnomecdaccess.la

gnome_cd_LDADD = $(MEDIA_LIBS)		\
		 $(top_builddir)/cddb-slave2/libcddb-slave2.la	\
		 libgnomemedia-cddbslave2.la \
		 $(INTLLIBS)		\
		 $(GAILUTIL_LIBS)	\
		 $(ACCESS_LT_LIB)

@INTLTOOL_DESKTOP_RULE@
@INTLTOOL_SCHEMAS_RULE@

Audio_in_files = gnome-cd.desktop.in
Audio_DATA = $(Audio_in_files:.desktop.in=.desktop)
Pixmap_DATA = gnome-cd.png

schemadir=$(GCONF_SCHEMA_FILE_DIR)
schema_DATA=gnome-cd.schemas

install-schemas:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/gnome-cd/gnome-cd.schemas || true

if GCONF_SCHEMAS_INSTALL
install-data-local: install-schemas
else
install-data-local:
endif

CLEANFILES = $(Audio_DATA) $(schema_DATA)

CDROM_HOST = gst-cdparanoia
