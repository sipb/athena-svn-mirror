lib_LTLIBRARIES=libscrollkeeper.la

INCLUDES = \
        $(XML_CFLAGS) \
	$(XSLT_CFLAGS) \
	$(HOST_TYPE)

libscrollkeeper_la_SOURCES = \
	scrollkeeper.h \
	error.c \
	file.c \
	i18n.c \
	merge.c \
	extract.c \
	install.c \
	database.c \
	uninstall.c \
	update-url.c

libscrollkeeper_la_LIBADD = $(XML_LIBS) $(XSLT_LIBS)
