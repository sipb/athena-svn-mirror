if BONOBO
DIR_COMPONENTS = components
else
DIR_COMPONENTS =
endif

confexecdir = $(libdir)
confexec_DATA = gtkhtmlConf.sh

gtkhtmlConf.sh: gtkhtmlConf.sh.in Makefile
	sed -e 's?\@GTKHTML_LIBDIR\@?$(GTKHTML_LIBDIR)?g' \
	    -e 's?\@GTKHTML_INCLUDEDIR\@?$(GTKHTML_INCLUDEDIR)?g' \
	    -e 's?\@GTKHTML_DATADIR\@?$(GTKHTML_DATADIR)?g' \
	    -e 's?\@VERSION\@?$(VERSION)?g' \
	    -e 's?\@GTKHTML_LIBS\@?$(GTKHTML_LIBS)?g' \
	      < $(srcdir)/gtkhtmlConf.sh.in > gtkhtmlConf.tmp \
	  && mv gtkhtmlConf.tmp gtkhtmlConf.sh

SUBDIRS =					\
	intl					\
	macros					\
	src					\
	capplet					\
	doc					\
	art					\
	$(DIR_COMPONENTS)			\
	po

EXTRA_DIST =					\
	html-component.gnorba			\
	gtkhtml.spec.in				\
	gtkhtml.spec                            \
	COPYING.LIB				\
	BUGS					\
	gtkhtmlConf.sh.in			\
	xml-i18n-extract.in			\
	xml-i18n-update.in			\
	xml-i18n-merge.in

if BONOBO
gnorbadir = $(sysconfdir)/CORBA/servers
gnorba_DATA = html-component.gnorba
endif
