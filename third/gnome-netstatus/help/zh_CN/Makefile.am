docname = gnome-netstatus
lang = zh_CN
omffile = gnome-netstatus-zh_CN.omf
entities = legal.xml

dest_figdir = figures
src_figdir  = $(datadir)/icons/hicolor/48x48/apps
figures =                               \
	gnome-netstatus-disconn.png     \
	gnome-netstatus-idle.png        \
	gnome-netstatus-rx.png          \
	gnome-netstatus-tx.png          \
	gnome-netstatus-txrx.png        \
	gnome-netstatus-error.png

install-data-local: install-icon-symlinks

install-icon-symlinks:
	$(mkinstalldirs) $(DESTDIR)$(docdir)/$(dest_figdir); \
	cd $(DESTDIR)$(docdir)/$(dest_figdir); \
	for file in $(figures); do \
	  rm -f $$file && $(LN_S) $(src_figdir)/$$file $$file; \
	done

uninstall-local: uninstall-icon-symlinks

uninstall-icon-symlinks:
	for file in $(figures); do \
	  rm -f $(DESTDIR)$(docdir)/$(dest_figdir)/$$file; \
	done; \
	rmdir $(DESTDIR)$(docdir)/$(dest_figdir)

include $(top_srcdir)/xmldocs.make
dist-hook: app-dist-hook
