
zoneinfodatadir = $(privdatadir)/zoneinfo

zoneinfodata_DATA = zones.tab

DIRS = \
	Africa			\
	America			\
	America/Indiana		\
	America/Kentucky	\
	America/North_Dakota	\
	Antarctica		\
	Arctic			\
	Asia			\
	Atlantic		\
	Australia		\
	Europe			\
	Indian			\
	Pacific


install-data-local:
	@$(NORMAL_INSTALL)
	for dir in $(DIRS); do \
	  $(mkinstalldirs) $(DESTDIR)$(zoneinfodatadir)/$$dir; \
	  for tzfile in $(srcdir)/$$dir/*.ics; do \
	    $(INSTALL_DATA) $$tzfile $(DESTDIR)$(zoneinfodatadir)/$$dir; \
	  done \
	done

uninstall-local:
	@$(NORMAL_UNINSTALL)
	for dir in $(DIRS); do \
	  for tzpath in  $(srcdir)/$$dir/*.ics; do \
	    tzfile=`basename $$tzpath`; \
	    rm -f  $(DESTDIR)$(zoneinfodatadir)/$$dir/$$tzfile; \
	  done \
	done

dist-hook:
	for dir in $(DIRS); do \
	  mkdir $(distdir)/$$dir; \
	  for tzfile in $(srcdir)/$$dir/*.ics; do \
	    cp -p $$tzfile $(distdir)/$$dir; \
	  done \
	done

EXTRA_DIST = $(zoneinfodata_DATA)
