bin_SCRIPTS=scrollkeeper-config scrollkeeper-get-content-list scrollkeeper-get-toc-from-id scrollkeeper-get-toc-from-docpath scrollkeeper-get-index-from-docpath scrollkeeper-get-extended-content-list scrollkeeper-rebuilddb

CLEANFILES=scrollkeeper-config scrollkeeper-get-content-list scrollkeeper-get-toc-from-id scrollkeeper-get-toc-from-docpath scrollkeeper-get-index-from-docpath scrollkeeper-get-extended-content-list scrollkeeper-rebuilddb

INCLUDES = \
	-I$(top_srcdir)/libuuid

LDADD = \
	$(top_builddir)/libuuid/libuuid.la

bin_PROGRAMS = scrollkeeper-gen-seriesid

scrollkeeper_gen_seriesid_SOURCES = \
	gen-docuid.c
