NULL=

icondir=	$(datadir)/icons/HighContrastLargePrint/48x48/devices/

icon_DATA=	\
		gnome-dev-cdrom.png \
		gnome-dev-floppy.png \
		gnome-dev-harddisk.png \
		gnome-dev-keyboard.png \
		gnome-dev-mouse-optical.png \
		gnome-dev-printer.png \
		$(NULL)

EXTRA_DIST=	$(icon_DATA) \
		symlinks.dat

install: install-am
	$(SHELL) $(top_srcdir)/common/mkiconlinks.sh $(srcdir)/symlinks.dat $(DESTDIR)$(icondir)

uninstall-symlinks:
	list=`ls $(DESTDIR)$(icondir)`; for p in $$list; do \
	if test -h $(DESTDIR)$(icondir)/$$p; then \
		rm -f $(DESTDIR)$(icondir)/$$p; \
	fi; \
	done

uninstall-am: uninstall-iconDATA uninstall-symlinks
