## Process this file with automake to produce Makefile.in


SUBDIRS = doc src


gfloppyappdir = $(datadir)/applications
gfloppyapp_in_files = gfloppy.desktop.in
gfloppyapp_DATA = $(gfloppyapp_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@
@INTLTOOL_KEYS_RULE@

schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_file = gfloppy.schemas.in
schemas_DATA = $(schemas_in_file:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST = \
	$(gfloppyapp_in_files) \
	$(gfloppyapp_DATA) \
	$(schemas_in_file)

install-data-local: gfloppy.desktop install-misc-local install-gconf-schemas

install-misc-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/pixmaps; then \
	  $(mkinstalldirs) $(DESTDIR)@PACKAGE_PIXMAPS_DIR@; \
	  for pixmap in $(srcdir)/pixmaps/*; do \
	    if test -f $$pixmap; then \
	      $(INSTALL_DATA) $$pixmap $(DESTDIR)@PACKAGE_PIXMAPS_DIR@; \
	    fi \
	  done \
	fi

if GCONF_SCHEMAS_INSTALL
install-gconf-schemas:
	if test -z "$(DESTDIR)" ; then \
		for p in $(schemas_DATA) ; do \
			GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/gfloppy/$$p ; \
			done \
	fi
else
install-gconf-schemas:
endif

dist-hook:
	if test -d pixmaps; then \
	  mkdir $(distdir)/pixmaps; \
	  for pixmap in pixmaps/*; do \
	    if test -f $$pixmap; then \
	      cp -p $$pixmap $(distdir)/pixmaps; \
	    fi \
	  done \
	fi
	cd $(distdir) ; rm -f $(CLEANFILES)

CLEANFILES = gfloppy.desktop
