# Process this file with automake to create Makefile.in.

SUBDIRS = idl magnifier test po cursors

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA= gnome-mag-1.0.pc

EXTRA_DIST=gnome-mag-1.0.pc.in \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in


.PHONY: files

files:
	@for subdir in $(SUBDIRS); do\
	  files=`cd $$subdir; $(MAKE) files | grep -v "make\[[1-9]\]"`; \
	  for file in $$files; do \
	    echo $$subdir/$$file; \
	  done; \
	done

