SUBDIRS = pixmaps lib src po sounds man xdap_data help

@INTLTOOL_DESKTOP_RULE@
@INTLTOOL_SCHEMAS_RULE@

DESKTOP_IN_FILES=gnomemeeting.desktop.in
DESKTOP_FILES=$(DESKTOP_IN_FILES:.desktop.in=.desktop)

SCHEMAS_FILE = gnomemeeting.schemas

EXTRA_DIST = $(DESKTOP_IN_FILES) $(DESKTOP_FILES) intltool-extract.in intltool-merge.in intltool-update.in FAQ $(SCHEMAS_FILE) src/gnomemeeting.server
Applicationsdir = $(datadir)/applications
Applications_DATA = $(DESKTOP_FILES)

schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_DATA = $(SCHEMAS_FILE)

install-schemas: gnomemeeting.schemas
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) gconftool-2 --makefile-install-rule $(SCHEMAS_FILE) >&1 > /dev/null
	gconftool-2 --shutdown

if GCONF_SCHEMAS_INSTALL
install-data-local: install-schemas
else
install-data-local:
endif
