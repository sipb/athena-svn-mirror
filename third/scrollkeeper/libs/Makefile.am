lib_LTLIBRARIES=libscrollkeeper.la

INCLUDES = \
        $(XML_CFLAGS) \
	$(XSLT_CFLAGS) \
	$(HOST_TYPE) \
	-DSCROLLKEEPER_STATEDIR=\""$(localstatedir)"\"

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
	
# Ideally, BUILT_SOURCES should be used here. However, that variable does not
# quite work until automake-1.7 is used, so we do it manually for now.
built_sources = scrollkeeper.h

$(OBJECTS) $(libscrollkeeper_la_OBJECTS): $(built_sources)

libscrollkeeper_la_LIBADD = $(XML_LIBS) $(XSLT_LIBS)

scrollkeeper.h : Makefile $(srcdir)/scrollkeeper.h.in
	sed -e 's,@localstatedir\@,$(localstatedir),g' \
	    -e 's,@prefix\@,$(prefix),g' \
	    $(srcdir)/scrollkeeper.h.in > scrollkeeper.h

EXTRA_DIST = scrollkeeper.h.in

CLEANFILES = $(built_sources)
