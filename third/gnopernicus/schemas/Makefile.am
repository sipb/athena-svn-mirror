schemasdir	 = $(GCONF_SCHEMA_FILE_DIR)
schemas_DATA 	 =  brlmonitor.schemas	\
		    gnopernicus.schemas  \
		    remote.schemas
@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST = \
	$(schemas_DATA)			

if GCONF_SCHEMAS_INSTALL
install-data-local:
	        if test -z "$(DESTDIR)" ; then \
	                for p in $(schemas_DATA) ; do \
	                        GCONF_CONFIG_SOURCE=$(GCONF_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$$p ; \
	                done \
	        fi
endif

