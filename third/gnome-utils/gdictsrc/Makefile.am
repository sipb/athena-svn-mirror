## Process this file with automake to produce Makefile.in

SUBDIRS = help

INCLUDES = -DG_LOG_DOMAIN=\"gdict\"			\
	-DDATADIR=\""$(datadir)"\"			\
	-DLIBDIR=\""$(libdir)"\"			\
	-DSYSCONFDIR=\""$(sysconfdir)"\"		\
	-DPREFIX=\""$(prefix)"\"			\
	-DGNOMELOCALEDIR=\""$(gnomeutilslocaledir)"\"	\
	$(APPLET_CFLAGS) $(GNOME_UTILS_CFLAGS) 

bin_PROGRAMS = gnome-dictionary

libexec_PROGRAMS = gdict-applet

gnome_dictionary_SOURCES = \
	gdict-about.c gdict-about.h \
	gdict-pref.c gdict-pref.h \
        gdict-pref-dialog.c gdict-pref-dialog.h \
	gdict-app.c gdict-app.h \
	gdict-defbox.c gdict-defbox.h \
	gdict-speller.c gdict-speller.h \
	dict.c dict.h \
	main.c

gnome_dictionary_LDADD = $(GNOME_UTILS_LIBS)

gdict_applet_SOURCES = \
	gdict-applet.c gdict-applet.h \
	dict.c dict.h \
	gdict-app.c gdict-app.h \
	gdict-pref.c gdict-pref.h \
	gdict-pref-dialog.c gdict-pref-dialog.h \
	gdict-defbox.c gdict-defbox.h \
	gdict-speller.c gdict-speller.h \
	gdict-about.c gdict-about.h 

gdict_applet_LDADD = $(GNOME_UTILS_LIBS) $(APPLET_LIBS)

man_MANS = gnome-dictionary.1

gdictappdir  = $(datadir)/applications
gdictapp_in_files = gnome-dictionary.desktop.in
gdictapp_DATA = $(gdictapp_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

uidir		= $(datadir)/gnome-2.0/ui
ui_DATA		= GNOME_GDictApplet.xml

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_GDictApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

@INTLTOOL_SERVER_RULE@

schemadir   = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_file = gdict.schemas.in
schema_DATA = $(schemas_in_file:.schemas.in=.schemas)
schema_source_data = $(foreach file, $(schema_DATA), $(srcdir)/$(file))
@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST = \
	GNOME_GDictApplet.server.in.in \
	$(gdictapp_in_files) \
	$(gdictapp_DATA) \
	$(man_MANS) \
	$(schema_DATA) \
	$(ui_DATA)

CLEANFILES = $(server_in_files) \
	$(server_DATA) \
	$(gdictapp_DATA)

if GCONF_SCHEMAS_INSTALL
install-data-local:
	if test -z "$(DESTDIR)" ; then \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schema_source_data) ; \
	fi
else
install-data-local:
endif
