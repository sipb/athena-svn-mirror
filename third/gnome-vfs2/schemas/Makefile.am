schemadir   = $(GCONF_SCHEMA_FILE_DIR)
schema_in_files = \
	system_http_proxy.schemas.in \
	system_smb.schemas.in \
	system_dns_sd.schemas.in \
	desktop_default_applications.schemas.in \
	desktop_gnome_url_handlers.schemas.in

schema_DATA = $(schema_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

# don't do this if we are building in eg. rpm
if GCONF_SCHEMAS_INSTALL
install-data-local:
	if test -z "$(DESTDIR)" ; then \
		for p in $(schema_DATA) ; do \
			GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $$p; \
		done \
	fi
else
install-data-local:
endif

EXTRA_DIST = $(schema_in_files) $(schema_DATA)

DISTCLEANFILES = $(schema_DATA)
