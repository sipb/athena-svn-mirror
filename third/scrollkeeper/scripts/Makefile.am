bin_SCRIPTS=scrollkeeper-config scrollkeeper-get-content-list scrollkeeper-get-toc-from-id scrollkeeper-get-toc-from-docpath scrollkeeper-get-index-from-docpath scrollkeeper-get-extended-content-list scrollkeeper-rebuilddb

CLEANFILES=scrollkeeper-config scrollkeeper-get-content-list scrollkeeper-get-toc-from-id scrollkeeper-get-toc-from-docpath scrollkeeper-get-index-from-docpath scrollkeeper-get-extended-content-list scrollkeeper-rebuilddb

INCLUDES = \
	-I$(top_srcdir)/libs \
	-I$(top_srcdir)/libuuid \
	$(XML_CFLAGS)

LDADD = \
	$(top_builddir)/libuuid/libuuid.la \
	@LTLIBINTL@

bin_PROGRAMS = scrollkeeper-gen-seriesid

scrollkeeper_gen_seriesid_SOURCES = \
	gen-docuid.c
