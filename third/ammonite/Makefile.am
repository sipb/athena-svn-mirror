NULL =

SUBDIRS = libammonite libammonite-gtk src doc idl data

confexecdir = $(libdir)
confexec_DATA = ammoniteConf.sh

ammoniteConf.sh: $(top_srcdir)/ammoniteConf.sh.in Makefile
	sed -e 's?\@AMMONITE_LIBDIR\@?$(AMMONITE_LIBDIR)?g' \
	    -e 's?\@AMMONITE_INCLUDEDIR\@?$(AMMONITE_INCLUDEDIR)?g' \
	    -e 's?\@AMMONITE_VERSION\@?$(AMMONITE_VERSION)?g' \
	    -e 's?\@AMMONITE_LIBS\@?$(AMMONITE_LIBS)?g' \
	      < $(top_srcdir)/ammoniteConf.sh.in > ammoniteConf.tmp \
	  && mv ammoniteConf.tmp ammoniteConf.sh	

CLEANFILES=ammoniteConf.sh

EXTRA_DIST = \
	NEWS \
	README \
	AUTHORS \
	HACKING \
	ammoniteConf.sh.in\
	acconfig.h\
	$(NULL)
