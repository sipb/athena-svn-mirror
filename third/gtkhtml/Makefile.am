if BONOBO
DIR_COMPONENTS = components
else
DIR_COMPONENTS =
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gtkhtml-@GTKHTML_RELEASE@.pc

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
	gtkhtml.spec.in				\
	gtkhtml.spec                            \
	COPYING.LIB				\
	BUGS					\
	xml-i18n-extract.in			\
	xml-i18n-update.in			\
	xml-i18n-merge.in
