# copyright (C) 2001 Sun Microsystems, Inc.

INCLUDES = \
        -I$(top_srcdir)/libs \
        $(XML_CFLAGS)
	
LDADD = \
        $(XML_LIBS) \
	$(XSLT_LIBS) \
	$(top_builddir)/libs/libscrollkeeper.la \
	@LTLIBINTL@

noinst_PROGRAMS = \
	scrollkeeper-tree-separate

scrollkeeper_tree_separate_SOURCES = \
	separate.c
