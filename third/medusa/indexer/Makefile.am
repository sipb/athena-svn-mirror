NULL = 

bin_PROGRAMS = medusa-indexd

medusa_indexd_SOURCES = \
	medusa-file-index-daemon.c \
	$(NULL)


INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_builddir)/libmedusa-internal \
	-I$(top_builddir)/libmedusa \
	$(GLIB_CFLAGS) \
	$(XML_CFLAGS) \
	$(VFS_CFLAGS) \
	$(NULL)

medusa_indexd_LDADD = \
	$(top_builddir)/libmedusa/libmedusa.la \
	$(top_builddir)/libmedusa-internal/libmedusa-index.la \
	$(VFS_LIBS) \
	$(GLIB_LIBS) \
	$(GNOME_LIBS) \
	$(OAF_LIBS) \
	$(XML_LIBS) \
	$(NULL)
