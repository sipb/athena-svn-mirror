# copyright (C) 2000 Sun Microsystems, Inc.

@INTLTOOL_XML_RULE@

EXTRA_DIST=scrollkeeper_cl.xml.in
LIB_PATH=$(top_builddir)/libs/.libs

TRANSLATED_LOCALES = C ca da de el es fr hu it ja ko nb nl no pl pt_BR ru sk sl sv tr uk, zh_TW

all: scrollkeeper_cl.xml
	$(top_builddir)/buildtools/scrollkeeper-tree-separate scrollkeeper_cl.xml $(srcdir) 

clean:
	rm -f scrollkeeper_cl.xml
	for locale in $(TRANSLATED_LOCALES); do \
		rm -rf $$locale; \
        done


install-data-local:
	for locale in $(TRANSLATED_LOCALES); do \
		$(mkinstalldirs) $(DESTDIR)$(datadir)/scrollkeeper/Templates/$$locale; \
		$(INSTALL_DATA) $(srcdir)/$$locale/scrollkeeper_cl.xml \
			$(DESTDIR)$(datadir)/scrollkeeper/Templates/$$locale; \
	done

uninstall-local:
	rm -rf $(DESTDIR)$(datadir)/scrollkeeper/Templates

