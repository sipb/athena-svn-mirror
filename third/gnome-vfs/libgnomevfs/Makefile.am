NULL = 

INCLUDES =					\
	-DGNOME_VFS_CONFDIR=\"$(sysconfdir)\"	\
	-DGNOME_VFS_DATADIR=\"$(datadir)\"	\
	-DGNOME_VFS_PREFIX=\"$(prefix)\"	\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/intl			\
	-I$(top_builddir)			\
	-I$(top_builddir)/intl			\
	-I$(top_builddir)/libgnomevfs		\
	$(OAF_CFLAGS)				\
	$(GCONF_CFLAGS) 			\
	$(GNOME_INCLUDEDIR)			\
	$(VFS_CFLAGS) $(WERROR)			\
	-D_GNU_SOURCE				\
	-D_REENTRANT				\
	-D_POSIX_PTHREAD_SEMANTICS		\
	-DLIBDIR=\"$(libdir)\"			\
	-DSYSCONFDIR=\"$(sysconfdir)\"		\
	-DG_LOG_DOMAIN=\"GnomeVFS\"		\
	$(NULL)

lib_LTLIBRARIES = libgnomevfs.la

EXTRA_DIST =					\
	getdelim.c

libgnomevfs_la_LIBADD =				\
	$(GNOME_LIBDIR)				\
	$(GNOME_LIBS)				\
	$(OAF_LIBS)	                        \
	$(GCONF_LIBS)	                        \
	$(GLIB_LIBS)				\
	$(NULL)

libgnomevfs_la_CFLAGS =				\
	$(GNOME_CFLAGS)				\
	$(OAF_CFLAGS)	                        \
	$(GCONF_CFLAGS)	                        \
	$(GLIB_CFLAGS)				\
	$(NULL)

libgnomevfsincludedir = $(includedir)/libgnomevfs

libgnomevfsinclude_HEADERS =			\
	gnome-vfs-async-ops.h			\
	gnome-vfs-constants.h			\
	gnome-vfs-directory.h			\
	gnome-vfs-directory-filter.h		\
	gnome-vfs-directory-list.h		\
	gnome-vfs-file-info.h			\
	gnome-vfs-file-size.h			\
	gnome-vfs-find-directory.h		\
	gnome-vfs-init.h			\
	gnome-vfs-xfer.h			\
	gnome-vfs-ops.h				\
	gnome-vfs-process.h			\
	gnome-vfs-result.h			\
	gnome-vfs-types.h			\
	gnome-vfs-uri.h				\
	gnome-vfs-utils.h			\
	gnome-vfs.h				\
	gnome-vfs-backend.h			\
	gnome-vfs-cancellation.h		\
	gnome-vfs-configuration.h		\
	gnome-vfs-context.h			\
	gnome-vfs-handle.h			\
	gnome-vfs-iobuf.h			\
	gnome-vfs-inet-connection.h		\
	gnome-vfs-list-sort.h			\
	gnome-vfs-messages.h			\
	gnome-vfs-method.h			\
	gnome-vfs-mime.h			\
	gnome-vfs-mime-handlers.h		\
	gnome-vfs-mime-info.h			\
	gnome-vfs-mime-magic.h			\
	gnome-vfs-mime-monitor.h		\
	gnome-vfs-mime-sniff-buffer.h		\
	gnome-vfs-mime-sniff-buffer-private.h	\
	gnome-vfs-application-registry.h	\
	gnome-vfs-module.h			\
	gnome-vfs-module-shared.h		\
	gnome-vfs-parse-ls.h			\
	gnome-vfs-cancellable-ops.h		\
	gnome-vfs-private-types.h		\
	gnome-vfs-private.h			\
	gnome-vfs-private-utils.h		\
	gnome-vfs-regexp-filter.h		\
	gnome-vfs-seekable.h			\
	gnome-vfs-shellpattern-filter.h

libgnomevfs_la_SOURCES =			\
	fnmatch.h				\
	gnome-vfs-backend.c			\
	gnome-vfs-cancellation.c		\
	gnome-vfs-configuration.c		\
	gnome-vfs-context.c			\
	gnome-vfs-directory.c			\
	gnome-vfs-directory-filter.c		\
	gnome-vfs-directory-list.c		\
	gnome-vfs-handle.c			\
	gnome-vfs-file-info.c			\
	gnome-vfs-find-directory.c		\
	gnome-vfs-init.c			\
	gnome-vfs-inet-connection.c		\
	gnome-vfs-iobuf.c			\
	gnome-vfs-list-sort.c			\
	gnome-vfs-messages.c			\
	gnome-vfs-method.c			\
	gnome-vfs-mime.c			\
	gnome-vfs-mime-handlers.c               \
	gnome-vfs-mime-info.c			\
	gnome-vfs-mime-magic.c			\
	gnome-vfs-mime-monitor.c		\
	gnome-vfs-mime-sniff-buffer.c		\
	gnome-vfs-application-registry.c	\
	gnome-vfs-module-shared.c		\
	gnome-vfs-xfer.c			\
	gnome-vfs-ops.c				\
	gnome-vfs-parse-ls.c			\
	gnome-vfs-cancellable-ops.c	 	\
	gnome-vfs-private-utils.c		\
	gnome-vfs-process.c			\
	gnome-vfs-result.c			\
	gnome-vfs-regexp-filter.c		\
	gnome-vfs-seekable.c			\
	gnome-vfs-shellpattern-filter.c		\
	gnome-vfs-uri.c				\
	gnome-vfs-utils.c			\
	fnmatch.c

noinst_HEADERS =				\
	gnome-vfs-mime-private.h

# Platform specific includes
libgnomevfsplatofrmincludedir = $(libdir)/vfs/include
libgnomevfsplatofrminclude_HEADERS =			\
	gnome-vfs-file-size.h
