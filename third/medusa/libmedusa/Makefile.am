NULL=

INCLUDES =\
	-I$(top_srcdir) \
	$(VFS_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(XML_CFLAGS) \
	$(WERROR) \
	$(NULL)

libmedusaincludedir=$(includedir)/libmedusa

libmedusainclude_HEADERS=		\
	medusa-file-info-utilities.h	\
	medusa-index-progress.h		\
	medusa-index-service.h          \
	medusa-index-service-private.h  \
	medusa-indexed-search.h         \
	medusa-search-service.h 	\
	medusa-search-service-private.h \
	medusa-service-private.h        \
	medusa-unindexed-search.h       \
	medusa-unsearched-locations.h	\
	medusa-utils.h			\
	$(NULL)

lib_LTLIBRARIES = libmedusa.la

libmedusa_la_SOURCES = \
	medusa-file-info-utilities.c	\
	medusa-index-progress.c 	\
	medusa-index-service.c          \
	medusa-indexed-search.c         \
	medusa-search-service.c         \
	medusa-service-private.c        \
	medusa-unindexed-search.c       \
	medusa-unsearched-locations.c	\
	medusa-utils.c			\
	$(NULL)


