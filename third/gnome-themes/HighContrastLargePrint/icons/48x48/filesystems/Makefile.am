NULL=

icondir=	$(datadir)/icons/HighContrastLargePrint/48x48/filesystems/

icon_DATA=	\
		gnome-fs-client.png \
		gnome-fs-desktop.png \
                gnome-fs-directory-accept.icon \
                gnome-fs-directory-accept.png \
                gnome-fs-directory.icon\
                gnome-fs-directory.png \
                gnome-fs-executable.png \
                gnome-fs-home.png \
                gnome-fs-network.png \
                gnome-fs-regular.png \
                gnome-fs-share.png \
                gnome-fs-trash-empty.png \
                gnome-fs-trash-full.png \
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
