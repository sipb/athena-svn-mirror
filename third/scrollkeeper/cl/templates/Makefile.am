# copyright (C) 2000 Sun Microsystems, Inc.

@INTLTOOL_XML_RULE@

EXTRA_DIST=scrollkeeper_cl.xml.in
LIB_PATH=$(top_builddir)/libs/.libs

TRANSLATED_LOCALES = C am az be ca cs da de el es fr hu it ja kn ko nl no pl pt_BR ro ru sk sl sr sr@Latn sv tr uk vi zh_CN zh_TW

all: scrollkeeper_cl.xml
	$(top_builddir)/buildtools/scrollkeeper-tree-separate scrollkeeper_cl.xml $(top_builddir)/cl/templates

clean-local:
	rm -f scrollkeeper_cl.xml
	for locale in $(TRANSLATED_LOCALES); do \
		rm -rf $$locale; \
        done

install-data-local:
	for locale in $(TRANSLATED_LOCALES); do \
		$(mkinstalldirs) $(DESTDIR)$(datadir)/scrollkeeper/Templates/$$locale; \
		$(INSTALL_DATA) $(top_builddir)/cl/templates/$$locale/scrollkeeper_cl.xml \
			$(DESTDIR)$(datadir)/scrollkeeper/Templates/$$locale; \
	done

uninstall-local:
	rm -rf $(DESTDIR)$(datadir)/scrollkeeper/Templates

