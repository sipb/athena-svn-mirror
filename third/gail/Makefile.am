# Process this file with automake to create Makefile.in.

SUBDIRS=libgail-util gail tests docs

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA= gail.pc

.PHONY: files

files:
	@for subdir in $(SUBDIRS); do\
	  files=`cd $$subdir; $(MAKE) files | grep -v "make\[[1-9]\]"`; \
	  for file in $$files; do \
	    echo $$subdir/$$file; \
	  done; \
	done

