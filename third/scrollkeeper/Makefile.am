# copyright (C) 2000 Sun Microsystems, Inc.

SUBDIRS = libuuid libs buildtools config cl extract scripts man intl po doc

EXTRA_DIST = scrollkeeper.spec.in COPYING-DOCS TODO intltool-extract.in intltool-merge.in intltool-update.in scrollkeeper-spec.txt

scrollkeeperdocdir = $(datadir)/doc/scrollkeeper-$(VERSION)
scrollkeeperdoc_DATA = COPYING-DOCS
scrollkeeper_localstate_dir = $(localstatedir)/lib/scrollkeeper

dist-hook: scrollkeeper.spec
	cp scrollkeeper.spec $(distdir)

install-data-hook:
	# Build/rebuild the catalog
	rm -rf $(DESTDIR)$(scrollkeeper_localstate_dir)
	$(mkinstalldirs) $(DESTDIR)$(scrollkeeper_localstate_dir)
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/log
	echo "`date +\"%b %d %X\"` Installing ScrollKeeper `scrollkeeper-config --version`..." >> $(DESTDIR)$(localstatedir)/log/scrollkeeper.log
	$(DESTDIR)$(bindir)/scrollkeeper-rebuilddb -q -p $(DESTDIR)$(scrollkeeper_localstate_dir)

uninstall-local:
	# Remove generated files
	-rm -rf $(DESTDIR)$(scrollkeeper_localstate_dir)
	-rm -f $(DESTDIR)$(localstatedir)/log/scrollkeeper.log
	-rm -f $(DESTDIR)$(localstatedir)/log/scrollkeeper.log.1

