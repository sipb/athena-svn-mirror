# Process this file with automake to create Makefile.in.

SUBDIRS = gail-gnome test

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA= libgail-gnome.pc

EXTRA_DIST=libgail-gnome.pc.in

.PHONY: files

files:
	@for subdir in $(SUBDIRS); do\
	  files=`cd $$subdir; $(MAKE) files | grep -v "make\[[1-9]\]"`; \
	  for file in $$files; do \
	    echo $$subdir/$$file; \
	  done; \
	done

