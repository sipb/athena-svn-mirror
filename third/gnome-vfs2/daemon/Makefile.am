libexec_PROGRAMS=gnome-vfs-daemon

INCLUDES= \
	$(LIBGNOMEVFS_CFLAGS)			\
	$(LIBGNOMEVFSDAEMON_CFLAGS)		\
	$(VFS_CFLAGS)				\
	$(MONIKERS_CFLAGS)                      \
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
	-DGNOMEVFS_LOCALEDIR=\"$(prefix)/@DATADIRNAME@/locale\" \
	-DSYSCONFDIR=\"$(sysconfdir)\"		\
	-DG_LOG_DOMAIN=\"libgnomevfs\"		\
	-I$(top_srcdir)				\
	-I$(top_builddir)			\
	-I$(top_srcdir)/libgnomevfs             \
	-I$(top_builddir)/libgnomevfs           \
	$(NULL)
gnome_vfs_daemon_LDADD=$(MONIKERS_LIBS) $(LIBGNOMEVFSDAEMON_LIBS) $(top_builddir)/libgnomevfs/libgnomevfs-2.la $(top_builddir)/libgnomevfs/libgnomevfsdaemon-2.la

gnome_vfs_daemon_SOURCES = \
	gnome-vfs-daemon.c \
	gnome-vfs-daemon.h \
	gnome-vfs-async-daemon.c \
	gnome-vfs-async-daemon.h \
	gnome-vfs-daemon-dir-handle.h \
	gnome-vfs-daemon-dir-handle.c \
	gnome-vfs-daemon-handle.h \
	gnome-vfs-daemon-handle.c \
	$(top_builddir)/libgnomevfs/GNOME_VFS_Daemon.h		\
	$(top_builddir)/libgnomevfs/GNOME_VFS_Daemon-common.c	\
	$(top_builddir)/libgnomevfs/GNOME_VFS_Daemon-stubs.c	\
	$(top_builddir)/libgnomevfs/GNOME_VFS_Daemon-skels.c

# Hack for now
BONOBO_ACTIVATION_IDL_DIR=`pkg-config --variable=idldir bonobo-activation-2.0`

idls = GNOME_VFS_Daemon.idl
idl_flags = -I $(BONOBO_ACTIVATION_IDL_DIR)

$(CORBA_SOURCE): $(idls) $(ORBIT_IDL)
	$(ORBIT_IDL) $(idl_flags) $(idls)

serverfile = GNOME_VFS_Daemon.server.in
serverdir = $(libdir)/bonobo/servers

install-data-hook :
	$(mkinstalldirs) $(DESTDIR)$(serverdir)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $(srcdir)/$(serverfile) > $(DESTDIR)$(serverdir)/$(serverfile:.in=)


EXTRA_DIST = $(serverfile)
