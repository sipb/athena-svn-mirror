# copyright (C) 2000 Sun Microsystems, Inc.

SUBDIRS = m4 libuuid intl libs buildtools config cl extract scripts man po doc

EXTRA_DIST = 			\
	config.rpath		\
	scrollkeeper.spec.in	\
	intltool-extract.in	\
	intltool-merge.in	\
	intltool-update.in	\
	intltool-modules/XML/Parser/Style/OrigTree.pm	\
	scrollkeeper-spec.txt

scrollkeeperdocdir = $(datadir)/doc/scrollkeeper-$(VERSION)
scrollkeeperdoc_DATA = 		\
	AUTHORS			\
	ChangeLog		\
	COPYING.DOC		\
	COPYING			\
	NEWS			\
	README			\
	scrollkeeper-spec.txt	\
	TODO

scrollkeeper_localstate_dir = $(localstatedir)/@SCROLLKEEPER_PARTIAL_DB_DIR@

dist-hook: scrollkeeper.spec
	cp scrollkeeper.spec $(distdir)

# Build/rebuild the catalog
install-data-hook:
	rm -rf $(DESTDIR)$(scrollkeeper_localstate_dir)
	$(mkinstalldirs) $(DESTDIR)$(scrollkeeper_localstate_dir)
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/log
	PATH="$(PATH):$(DESTDIR)$(bindir)" ; \
		echo "`date +\"%b %d %X\"` Installing ScrollKeeper `scrollkeeper-config --version`..." >> $(DESTDIR)$(localstatedir)/log/scrollkeeper.log
	PATH="$(PATH):$(DESTDIR)$(bindir)" ; \
		$(DESTDIR)$(bindir)/scrollkeeper-rebuilddb -q -p $(DESTDIR)$(scrollkeeper_localstate_dir)

# Remove generated files
uninstall-local:
	-rm -rf $(DESTDIR)$(scrollkeeper_localstate_dir)
	-rm -f $(DESTDIR)$(localstatedir)/log/scrollkeeper.log
	-rm -f $(DESTDIR)$(localstatedir)/log/scrollkeeper.log.1

ACLOCAL_AMFLAGS = -I m4

# It is wrong to have scrollkeeper.pot here, but some other bug means its presence is causing distcheck to fail. :-(
DISTCLEANFILES =	 	\
	intltool-extract	\
	intltool-merge		\
	intltool-update		\
	po/scrollkeeper.pot
