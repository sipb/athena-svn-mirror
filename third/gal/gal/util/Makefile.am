INCLUDES =					\
	$(EXTRA_GNOME_CFLAGS)			\
	$(GNOME_INCLUDEDIR)			\
	$(ICONV_CFLAGS)				\
	-I$(top_srcdir)                         \
	-I$(top_builddir)                       \
        -DG_LOG_DOMAIN=\"widgets\"

noinst_LTLIBRARIES = libutil.la

libutil_la_SOURCES =	\
	e-bit-array.c	\
	e-cache.c	\
	e-iconv.c	\
	e-sorter-array.c	\
	e-sorter.c	\
	e-text-event-processor-emacs-like.c	\
	e-text-event-processor.c		\
	e-unicode-i18n.c	\
	e-util.c	\
	e-xml-utils.c

libgalincludedir = $(includedir)/gal/util

libgalinclude_HEADERS =	\
	e-bit-array.h	\
	e-cache.h	\
	e-iconv.h	\
	e-i18n.h	\
	e-sorter-array.h	\
	e-sorter.h	\
	e-text-event-processor-emacs-like.h	\
	e-text-event-processor-types.h		\
	e-text-event-processor.h		\
	e-unicode-i18n.h	\
	e-util.h	\
	e-xml-utils.h
