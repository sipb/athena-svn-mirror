Utilitiesdir = $(datadir)/applications
schemasdir   = $(GCONF_SCHEMA_FILE_DIR)

SUBDIRS=icons

Utilities_in_files = gconf-editor.desktop.in 
Utilities_DATA = $(Utilities_in_files:.desktop.in=.desktop)
schemas_in_files = gconf-editor.schemas.in
schemas_DATA   = $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	$(Utilities_in_files) \
	$(Utilities_DATA)
if GCONF_SCHEMAS_INSTALL
install-data-local:
	if test -z "$(DESTDIR)" ; then \
		for p in $(schemas_DATA) ; do \
			GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $$p ; \
		done \
	fi
else
install-data-local:
endif

