SUBDIRS = gal po intl tests docs art

confexecdir = $(libdir)
confexec_DATA = galConf.sh 

galConf.sh: $(top_srcdir)/galConf.sh.in Makefile
	sed -e 's?\@GAL_LIBDIR\@?$(GAL_LIBDIR)?g' \
	    -e 's?\@GAL_INCLUDEDIR\@?$(GAL_INCLUDEDIR)?g' \
	    -e 's?\@VERSION\@?$(VERSION)?g' \
	    -e 's?\@GAL_LIBS\@?$(GAL_LIBS)?g' \
	      < $(top_srcdir)/galConf.sh.in > galConf.tmp \
	  && mv galConf.tmp galConf.sh

EXTRA_DIST = 			\
	gal.spec.in		\
	galConf.sh.in		\
	MAINTAINERS		\
	AUTHORS			\
	xml-i18n-extract.in	\
	xml-i18n-merge.in	\
	xml-i18n-update.in

dist-hook: gal.spec
	cp gal.spec $(distdir)

CLEANFILES = galConf.sh 
