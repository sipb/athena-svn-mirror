NULL = 

INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_builddir)			\
	$(LIBGNOMEVFS_CFLAGS)			\
	$(VFS_CFLAGS)				\
	-D_FILE_OFFSET_BITS=64			\
	-D_BSD_SOURCE				\
	-D_GNU_SOURCE				\
	-D_LARGEFILE64_SOURCE                   \
	-D_POSIX_PTHREAD_SEMANTICS		\
	-D_REENTRANT				\
	-DG_DISABLE_DEPRECATED			\
	-DDATADIR=\"$(datadir)\"		\
	-DLIBDIR=\"$(libdir)\"			\
	-DPREFIX=\"$(prefix)\"			\
	-DSYSCONFDIR=\"$(sysconfdir)\"		\
	-DG_LOG_DOMAIN=\"libgnomevfs\"		\
	$(NULL)

# %.c %.h: %.gob
#	$(top_srcdir)/gob/src/gob2 $<
#
# GOB_SRCLIST = 					\
#	gnome-vfs-metadata.c			\
#	gnome-vfs-metadata.h			\
#	gnome-vfs-metadata-private.h		\
#	$(NULL)
# GOB_FILES =					\
#	gnome-vfs-metadata.gob			\
#	$(NULL)

lib_LTLIBRARIES = libgnomevfs-2.la

libgnomevfs_2_la_LIBADD =			\
	$(LIBGNOMEVFS_LIBS)			\
	$(NULL)

libgnomevfsincludedir = $(includedir)/gnome-vfs-2.0/libgnomevfs

libgnomevfsmoduleincludedir = $(includedir)/gnome-vfs-module-2.0/libgnomevfs

# mime-sniff-buffers and cancellabe-ops are used by modules
noinst_HEADERS =				\
	gnome-vfs-async-job-map.h		\
	gnome-vfs-backend.h			\
	gnome-vfs-cancellable-ops.h		\
	gnome-vfs-configuration.h		\
	gnome-vfs-handle-private.h		\
	gnome-vfs-i18n.h			\
	gnome-vfs-job-queue.h			\
	gnome-vfs-job-slave.h			\
	gnome-vfs-job.h				\
	gnome-vfs-mime-magic.h			\
	gnome-vfs-mime-private.h		\
	gnome-vfs-mime-sniff-buffer.h		\
	gnome-vfs-mime-sniff-buffer-private.h	\
	gnome-vfs-module-callback-private.h	\
	gnome-vfs-monitor-private.h		\
	gnome-vfs-private-utils.h		\
	gnome-vfs-private.h			\
	gnome-vfs-process.h			\
	gnome-vfs-ssl-private.h			\
	gnome-vfs-thread-pool.h			\
	$(NULL)
#	gnome-vfs-metadata-private.h		\

libgnomevfsmoduleinclude_HEADERS =		\
	gnome-vfs-cancellable-ops.h		\
	gnome-vfs-inet-connection.h		\
	gnome-vfs-method.h			\
	gnome-vfs-mime.h			\
	gnome-vfs-mime-info.h			\
	gnome-vfs-module-callback-module-api.h	\
	gnome-vfs-module-shared.h		\
	gnome-vfs-module.h			\
	gnome-vfs-parse-ls.h			\
	gnome-vfs-ssl.h				\
	gnome-vfs-transform.h			\
	gnome-vfs-socket-buffer.h		\
	gnome-vfs-socket.h			\
	$(NULL)


libgnomevfsinclude_HEADERS =			\
	gnome-vfs-application-registry.h	\
	gnome-vfs-async-ops.h			\
	gnome-vfs-cancellation.h		\
	gnome-vfs-context.h			\
	gnome-vfs-directory.h			\
	gnome-vfs-file-info.h			\
	gnome-vfs-file-size.h			\
	gnome-vfs-find-directory.h		\
	gnome-vfs-handle.h			\
	gnome-vfs-init.h			\
	gnome-vfs-job-limit.h			\
	gnome-vfs-mime-utils.h			\
	gnome-vfs-mime-handlers.h		\
	gnome-vfs-mime-monitor.h		\
	gnome-vfs-module-callback.h		\
	gnome-vfs-monitor.h			\
	gnome-vfs-ops.h				\
	gnome-vfs-result.h			\
	gnome-vfs-standard-callbacks.h		\
	gnome-vfs-types.h			\
	gnome-vfs-uri.h				\
	gnome-vfs-utils.h			\
	gnome-vfs-xfer.h			\
	gnome-vfs.h				\
	$(NULL)
#	gnome-vfs-metadata.h			\

libgnomevfs_2_la_SOURCES =			\
	gnome-vfs-application-registry.c	\
	gnome-vfs-async-job-map.c		\
	gnome-vfs-async-ops.c			\
	gnome-vfs-cancellable-ops.c	 	\
	gnome-vfs-cancellation.c		\
	gnome-vfs-configuration.c		\
	gnome-vfs-context.c			\
	gnome-vfs-directory.c			\
	gnome-vfs-open-fd.c			\
	gnome-vfs-file-info.c			\
	gnome-vfs-find-directory.c		\
	gnome-vfs-handle.c			\
	gnome-vfs-i18n.c			\
	gnome-vfs-inet-connection.c		\
	gnome-vfs-init.c			\
	gnome-vfs-job-queue.c			\
	gnome-vfs-job-slave.c			\
	gnome-vfs-job.c				\
	gnome-vfs-method.c			\
	gnome-vfs-mime-handlers.c		\
	gnome-vfs-mime-info.c			\
	gnome-vfs-mime-magic.c			\
	gnome-vfs-mime-monitor.c		\
	gnome-vfs-mime-sniff-buffer.c		\
	gnome-vfs-mime.c			\
	gnome-vfs-module-callback-module-api.c	\
	gnome-vfs-module-callback-private.c	\
	gnome-vfs-module-callback.c		\
	gnome-vfs-module-shared.c		\
	gnome-vfs-module.c			\
	gnome-vfs-monitor.c			\
	gnome-vfs-ops.c				\
	gnome-vfs-parse-ls.c			\
	gnome-vfs-private-utils.c		\
	gnome-vfs-private.c			\
	gnome-vfs-process.c			\
	gnome-vfs-result.c			\
	gnome-vfs-socket-buffer.c		\
	gnome-vfs-socket.c			\
	gnome-vfs-ssl.c				\
	gnome-vfs-thread-pool.c			\
	gnome-vfs-transform.c			\
	gnome-vfs-uri.c				\
	gnome-vfs-utils.c			\
	gnome-vfs-xfer.c			\
	$(NULL)

#	$(GOB_SRCLIST)				\
#	$(GOB_FILES)				\
# BUILT_SOURCES = $(GOB_SRCLIST)
# CLEANFILES = $(BUILT_SOURCES)

# Platform specific includes
libgnomevfsplatformincludedir = $(libdir)/gnome-vfs-2.0/include
libgnomevfsplatforminclude_HEADERS =		\
	gnome-vfs-file-size.h

# TESTS = check-headers.pl
EXTRA_DIST = check-headers.pl getdelim.c

