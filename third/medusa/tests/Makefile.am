NULL =

noinst_PROGRAMS = text-index-test index-service-test queue-test test-file-indexer versioned-file-test search-method-test

INCLUDES = \
	-I$(top_srcdir)/libmedusa-internal \
	-I$(top_srcdir)/libmedusa \
	$(VFS_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GTK_CFLAGS) \
	$(NULL)

text_index_test_SOURCES = \
	medusa-text-index-test.c

index_service_test_SOURCES = \
	medusa-index-service-test.c

queue_test_SOURCES = \
	medusa-queue-test.c

test_file_indexer_SOURCES = \
	test-file-indexer.c

versioned_file_test_SOURCES = \
	medusa-versioned-file-test.c 

search_method_test_SOURCES = \
	search-method-test.c

LDADD = \
	$(top_builddir)/libmedusa-internal/libmedusa-index.la \
	$(top_builddir)/libmedusa/libmedusa.la \
	$(VFS_LIBS) \
	$(GLIB_LIBS) \
	$(GTK_LIBS) \
	$(XML_LIBS) \
	$(NULL)
