INCLUDES =					\
	$(EXTRA_GNOME_CFLAGS)			\
	$(GNOME_INCLUDEDIR)			\
	$(ICONV_CFLAGS)				\
	-I$(top_srcdir)                         \
	-I$(top_builddir)                       \
	-DG_LOG_DOMAIN=\"widgets\"		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"

noinst_LTLIBRARIES = libutil.la

libutil_la_SOURCES =	\
	e-bit-array.c	\
	e-cache.c	\
	e-i18n.h	\
	e-iconv.c	\
	e-sorter-array.c	\
	e-sorter.c	\
	e-text-event-processor-emacs-like.c	\
	e-text-event-processor.c		\
	e-util.c	\
	e-xml-utils.c


libgalincludedir = $(includedir)/gal-$(GAL_API_VERSION)/gal/util

libgalinclude_HEADERS =	\
	e-bit-array.h	\
	e-cache.h	\
	e-iconv.h	\
	e-marshal.h	\
	e-sorter-array.h	\
	e-sorter.h	\
	e-text-event-processor-emacs-like.h	\
	e-text-event-processor-types.h		\
	e-text-event-processor.h		\
	e-util.h	\
	e-xml-utils.h

$(libutil_la_OBJECTS) : $(marshal_sources)

marshal_sources = \
	e-marshal.c \
	e-marshal.h

e-marshal.h: e-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=e_marshal > $@
e-marshal.c: e-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --body --prefix=e_marshal > $@

EXTRA_DIST = e-marshal.list
CLEANFILES = $(marshal_sources)

DONT_DIST_SOURCE = $(marshal_sources)

dist-hook:
	for file in $(DONT_DIST_SOURCE) ; do \
	    rm -f $(distdir)/$$file ; \
	done
