SUBDIRS = po gal tests docs art

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gal-2.0.pc

EXTRA_DIST = 			\
	gal-2.0.pc.in		\
	gal.spec.in		\
	MAINTAINERS		\
	AUTHORS			\
	iconv-detect.c		\
	intltool-extract.in	\
	intltool-merge.in	\
	intltool-update.in

dist-hook: gal.spec
	cp gal.spec $(distdir)

CLEANFILES = gal-2.0.pc 
