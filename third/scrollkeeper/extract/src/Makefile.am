# copyright (C) 2001 Sun Microsystems, Inc.

INCLUDES = \
        -I$(top_srcdir)/libs \
        $(XML_CFLAGS) \
        $(XSLT_CFLAGS)

LDADD = \
        $(XML_LIBS) \
        $(XSLT_LIBS) \
        $(top_builddir)/libs/libscrollkeeper.la \
	@LTLIBINTL@

scrollkeeper_extract_SOURCES = \
        scrollkeeper-extract.c

bin_PROGRAMS=scrollkeeper-extract
