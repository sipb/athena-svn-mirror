SUBDIRS = po intl bonobo-conf monikers examples cedit idl macros doc tests


EXTRA_DIST = 			\
	xml-i18n-extract.in 	\
	xml-i18n-merge.in 	\
	xml-i18n-update.in 	\
	bonobo_confConf.sh.in   \
	bonobo-conf.spec

confexecdir = $(libdir)
confexec_DATA = bonobo_confConf.sh


bonobo_confConf.sh: $(top_srcdir)/bonobo_confConf.sh.in Makefile
	sed -e 's?\@INCLUDEDIR\@?$(includedir)?g' \
	    -e 's?\@LIBDIR\@?$(libdir)?g' \
	    -e 's?\@BONOBO_CONF_CFLAGS\@?$(BONOBO_CONF_CFLAGS)?g' \
	    -e 's?\@VERSION\@?$(VERSION)?g' \
	    -e 's?\@BONOBO_CONF_LIBS\@?$(BONOBO_CONF_LIBS)?g' \
	      < $(top_srcdir)/bonobo_confConf.sh.in > bonobo_confConf.tmp \
	  && mv bonobo_confConf.tmp bonobo_confConf.sh
