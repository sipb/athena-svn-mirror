SUBDIRS = po gal tests docs art

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gal-2.2.pc

EXTRA_DIST = 			\
	gal-2.2.pc.in		\
	gal.spec.in		\
	MAINTAINERS		\
	AUTHORS			\
	iconv-detect.c		\
	intltool-extract.in	\
	intltool-merge.in	\
	intltool-update.in

DISTCLEANFILES = intltool-extract intltool-merge intltool-update iconv-detect.h

dist-hook: gal.spec
	cp gal.spec $(distdir)

CLEANFILES = gal-2.2.pc 
