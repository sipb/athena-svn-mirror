
if HAVE_ALSA
ALSA_CFLAGS = -DHAVE_ALSA
ALSA_files = volume-alsa.c volume-alsa.h volume-alsa-private.h
ALSA_gob_files = volume-alsa.gob
endif

if HAVE_OSS
OSS_CFLAGS = -DHAVE_OSS
OSS_files = volume-oss.c volume-oss.h volume-oss-private.h
OSS_gob_files = volume-oss.gob
endif

if HAVE_FB
FB_CFLAGS = -DHAVE_FB
FB_files = fb-level.c fb-level.h fb-level-private.h
FB_gob_files = fb-level.gob
endif

INCLUDES =						\
	$(EXTRA_GNOME_CFLAGS)				\
	$(ALSA_CFLAGS) $(OSS_CFLAGS) $(FB_CFLAGS)	\
	-DACME_DATA=\""$(datadir)/acme/"\"		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DWNCK_I_KNOW_THIS_IS_UNSTABLE			\
	-DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED	\
	-DGTK_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED

bin_PROGRAMS = acme acme-properties

BUILT_SOURCES =                                         \
	volume-alsa.c volume-alsa.h volume-alsa-private.h \
	volume-oss.c volume-oss.h volume-oss-private.h	\
	g-volume.c g-volume.h g-volume-private.h	\
	fb-level.c fb-level.h fb-level-private.h

BUILT_files_unconditional =				\
	g-volume.c g-volume.h g-volume-private.h

acme_SOURCES =						\
	$(ALSA_gob_files) $(OSS_gob_files)		\
	$(FB_gob_files)					\
	g-volume.gob					\
	$(ALSA_files) $(OSS_files) $(FB_files)		\
	$(BUILT_files_unconditional)			\
	eggtrayicon.h eggtrayicon.c			\
	acme.c acme.h

acme_LDFLAGS =						\
	-export-dynamic					\
	$(EXTRA_GNOME_LIBS)				\
	$(ALSA_LIBS)					\
	$(INTLLIBS)

acme_properties_SOURCES =				\
	acme-properties.c				\
	eggcellrendererkeys.c				\
	eggcellrendererkeys.h				\
	eggaccelerators.h				\
	eggaccelerators.c

%.c %.h %-private.h: %.gob
	@GOB2@ --always-private-header $<

CLEANFILES = $(BUILT_SOURCES) *.bak *.gladep $(desktop_DATA) $(schema_DATA)

eggcellrendererkeys.c: regenerate-built-sources
eggcellrendererkeys.h: regenerate-built-sources
eggaccelerators.h: regenerate-built-sources
eggaccelerators.c: regenerate-built-sources
eggtrayicon.h: regenerate-built-sources
eggtrayicon.c: regenerate-built-sources

EGGFILES1 = eggcellrendererkeys.c eggcellrendererkeys.h eggaccelerators.h eggaccelerators.c
EGGFILES2 = eggtrayicon.h eggtrayicon.c
EGGDIR1=$(srcdir)/../../libegg/libegg/treeviewutils/
EGGDIR2=$(srcdir)/../../libegg/libegg/tray/
regenerate-built-sources:
	EGGFILES="$(EGGFILES1)" EGGDIR="$(EGGDIR1)" $(srcdir)/update-from-egg.sh || true
	EGGFILES="$(EGGFILES2)" EGGDIR="$(EGGDIR2)" $(srcdir)/update-from-egg.sh || true

acme_properties_LDFLAGS = 			\
	$(EXTRA_GNOME_LIBS)			\
	$(INTLLIBS)

EXTRA_DIST =			\
	$(Data_DATA)		\
	$(schema_in_files)	\
	$(BUILT_SOURCES)	\
	$(EGGFILES1)		\
	$(EGGFILES2)		\
	$(desktop_in_files)	\
	fb-level.gob g-volume.gob volume-alsa.gob volume-oss.gob

Datadir = $(datadir)/acme/
Data_DATA = 				\
	acme.glade			\
	acme-properties.glade		\
	gnome-speakernotes-muted.png	\
	gnome-speakernotes.png		\
	acme-brightness.png		\
	acme-eject.png			\
	acme-48.png			\
	acme-16.png

schema_in_files = acme.schemas.in
schemadir = $(sysconfdir)/gconf/schemas
schema_DATA = $(schema_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

desktop_in_files = acme.desktop.in
desktopdir = $(datadir)/control-center-2.0/capplets
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

install-data-local:
	-GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)
