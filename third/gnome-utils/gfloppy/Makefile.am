## Process this file with automake to produce Makefile.in


SUBDIRS = doc src


gfloppyappdir = $(datadir)/applications
gfloppyapp_in_files = gfloppy.desktop.in
gfloppyapp_DATA = $(gfloppyapp_in_files:.desktop.in=.desktop)

Mimedir = $(datadir)/mime-info

@INTLTOOL_DESKTOP_RULE@
@INTLTOOL_KEYS_RULE@

EXTRA_DIST = \
	$(gfloppyapp_in_files) \
	$(gfloppyapp_DATA) \
	gfloppy.keys \
	gfloppy.keys.in \
	gfloppy.mime

install-data-local: gfloppy.desktop gfloppy.keys
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/pixmaps; then \
	  $(mkinstalldirs) $(DESTDIR)@PACKAGE_PIXMAPS_DIR@; \
	  for pixmap in $(srcdir)/pixmaps/*; do \
	    if test -f $$pixmap; then \
	      $(INSTALL_DATA) $$pixmap $(DESTDIR)@PACKAGE_PIXMAPS_DIR@; \
	    fi \
	  done \
	fi

	$(mkinstalldirs) $(DESTDIR)$(Mimedir)
	$(INSTALL_DATA) $(srcdir)/gfloppy.mime $(DESTDIR)$(Mimedir)/
	$(INSTALL_DATA) $(srcdir)/gfloppy.keys $(DESTDIR)$(Mimedir)/

dist-hook:
	if test -d pixmaps; then \
	  mkdir $(distdir)/pixmaps; \
	  for pixmap in pixmaps/*; do \
	    if test -f $$pixmap; then \
	      cp -p $$pixmap $(distdir)/pixmaps; \
	    fi \
	  done \
	fi

