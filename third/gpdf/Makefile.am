SUBDIRS = goo lib test-files fofi splash xpdf shell po help

WIN32_EXTRA =

intltool_extra = intltool-extract.in intltool-merge.in intltool-update.in

EXTRA_DIST = \
	ChangeLog README README.GPdf CHANGES 		\
	vms_make.com aconf2.h $(WIN32_EXTRA)		\
	gpdf.desktop.in gpdf.keys.in gpdf.applications	\
	gnome-pdf.png $(intltool_extra)			\
	$(schema_DATA).in				\
	gpdf.spec.in

Applicationsdir = $(datadir)/applications
Applications_in_files = gpdf.desktop.in
Applications_DATA = $(Applications_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

Mimeappsdir = $(datadir)/application-registry
Mimeapps_DATA = gpdf.applications

Mimedir = $(datadir)/mime-info
Mime_in_files = gpdf.keys.in
Mime_DATA = $(Mime_in_files:.keys.in=.keys)
@INTLTOOL_KEYS_RULE@

appicon_DATA = gnome-pdf.png
appicondir = $(datadir)/pixmaps

schemadir = $(GCONF_SCHEMA_FILE_DIR)
schema_DATA = gpdf.schemas
@INTLTOOL_SCHEMAS_RULE@

if GCONF_SCHEMAS_INSTALL
install-data-local:
	for p in $(schema_DATA) ; do \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/$$p; \
	done
else
install-data-local:
endif

DISTCHECK_CONFIGURE_FLAGS = --disable-schemas-install

ACLOCAL_AMFLAGS = -I $(datadir)/aclocal -I $(datadir)/aclocal/gnome2-macros

CLEANFILES =			\
	$(schema_DATA) 		\
	gpdf.desktop		\
	gpdf.keys

DISTCLEANFILES =		\
	intltool-extract	\
	intltool-merge		\
	intltool-update		\
	po/.intltool-merge-cache

distuninstallcheck_listfiles = find . -type f -print | grep -v '^\./var/scrollkeeper' | grep -v 'omf' | grep -v 'figures'
