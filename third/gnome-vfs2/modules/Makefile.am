NULL =
SUBDIRS = extfs vfolder

INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_builddir)			\
	$(MODULES_XML_GCONF_CFLAGS)		\
	$(MODULES_FILE_CFLAGS) 			\
	$(LIBEFS_CFLAGS)			\
	$(VFS_CFLAGS)				\
	-D_FILE_OFFSET_BITS=64			\
	-D_BSD_SOURCE				\
	-D_GNU_SOURCE				\
	-D_LARGEFILE64_SOURCE			\
	-D_POSIX_PTHREAD_SEMANTICS		\
	-D_REENTRANT				\
	-DG_DISABLE_DEPRECATED			\
	-DDATADIR=\"$(datadir)\"		\
	-DPREFIX=\"$(prefix)\"			\
	-DLIBDIR=\"$(libdir)\"			\
	-DSYSCONFDIR=\"$(sysconfdir)\"		\
	-DG_LOG_DOMAIN=\"gnome-vfs-modules\"	\
	$(NULL)

EXTRA_DIST =					\
	default-modules.conf			\
	cdda-module.conf			\
	cdemenu-module.conf			\
	ssl-modules.conf

###  Module setup

if HAVE_CDDA
CDDA_LTLIBS = libcdda.la
else
CDDA_LTLIBS =
endif

if BUILD_CDEMENU_MODULE
CDEMENU_LTLIBS = libcdemenu-desktop.la
else
CDEMENU_LTLIBS =
endif

module_flags = -export_dynamic -avoid-version -module
modulesdir = $(libdir)/gnome-vfs-2.0/modules

modules_LTLIBRARIES =				\
	$(CDDA_LTLIBS)				\
	libextfs.la				\
	libfile.la				\
	libnntp.la				\
	libvfs-test.la				\
	$(HTTP_LTLIBS)				\
	libbzip2.la				\
	libgzip.la				\
	libftp.la				\
	libvfs-pipe.la				\
	libvfs-translate.la			\
	$(CDEMENU_LTLIBS)                       \
	libssh.la				\
	libtar.la				\
	$(NULL)

# Not currently supported
#	libnfs.la

modulesconfdir=$(sysconfdir)/gnome-vfs-2.0/modules
if HAVE_CDDA
CDDA_CONF = cdda-module.conf
endif
if HAVE_SSL
SSL_CONF = ssl-modules.conf
endif
if BUILD_CDEMENU_MODULE
CDE_CONF = cdemenu-module.conf
endif
modulesconf_DATA = default-modules.conf $(CDDA_CONF) $(SSL_CONF) $(CDE_CONF)

###  `cdda' method

libcdda_la_SOURCES = \
	cdda-method.c 				\
	cdda-cddb.c 				\
	cdda-cddb.h				\
	cdda-cdrom-extensions.h

libcdda_la_LDFLAGS = $(module_flags)
libcdda_la_LIBADD = -lcdda_paranoia -lcdda_interface ../libgnomevfs/libgnomevfs-2.la

###  `extfs' method

libextfs_la_SOURCES = extfs-method.c
libextfs_la_LDFLAGS = $(module_flags)
libextfs_la_LIBADD = ../libgnomevfs/libgnomevfs-2.la

###  `file' method

libfile_la_SOURCES = file-method.c fstype.c
libfile_la_LDFLAGS = $(module_flags)
libfile_la_LIBADD = $(FAM_LIBS) $(MODULES_FILE_LIBS) ../libgnomevfs/libgnomevfs-2.la

###  `nntp' method

libnntp_la_SOURCES = nntp-method.c nntp-method.h
libnntp_la_LDFLAGS = $(module_flags)
libnntp_la_LIBADD = ../libgnomevfs/libgnomevfs-2.la

###  `cdemenu-desktop' method

libcdemenu_desktop_la_SOURCES = cdemenu-desktop-method.c
libcdemenu_desktop_la_LDFLAGS = $(module_flags)
libcdemenu_desktop_la_LIBADD = ../libgnomevfs/libgnomevfs-2.la

###  `ssh' method

libssh_la_SOURCES = ssh-method.c
libssh_la_LDFLAGS = $(module_flags)
libssh_la_LIBADD = ../libgnomevfs/libgnomevfs-2.la

###  `test' method

libvfs_test_la_SOURCES = test-method.c
libvfs_test_la_LDFLAGS = $(module_flags)
libvfs_test_la_LIBADD = $(MODULES_XML_LIBS) ../libgnomevfs/libgnomevfs-2.la

### `gzip' method

libgzip_la_SOURCES = gzip-method.c
libgzip_la_LDFLAGS = $(module_flags)
libgzip_la_LIBADD = -lz ../libgnomevfs/libgnomevfs-2.la

### `bzip2' method

libbzip2_la_SOURCES = bzip2-method.c
libbzip2_la_LDFLAGS = $(module_flags)
libbzip2_la_LIBADD = $(BZ2_LIBS) ../libgnomevfs/libgnomevfs-2.la

### `ftp' method

libftp_la_SOURCES = ftp-method.c
libftp_la_LDFLAGS = $(module_flags)
libftp_la_LIBADD = ../libgnomevfs/libgnomevfs-2.la

### `nfs' method

#libnfs_la_SOURCES = 				\
#	nfs-method.c				\
#	nfs-method.h				\
#	nfs-method_mount_xdr.c			\
#	nfs-method_nfs_prot_xdr.c		\
#	nfs-method_mount.h			\
#	nfs-method_nfs_prot.h
#libnfs_la_LDFLAGS = $(module_flags)

### `http' method

HTTP_LTLIBS = libhttp.la

libhttp_la_SOURCES =				\
	http-method.c				\
	http-method.h				\
	http-authn.c				\
	http-authn.h				\
	http-cache.c				\
	http-cache.h				\
	$(NULL)
libhttp_la_LDFLAGS = $(module_flags)
libhttp_la_LIBADD  = $(MODULES_GCONF_LIBS) ../libgnomevfs/libgnomevfs-2.la

libvfs_pipe_la_SOURCES = pipe-method.c
libvfs_pipe_la_LDFLAGS = $(module_flags)
libvfs_pipe_la_LIBADD  = $(MODULES_LIBS) ../libgnomevfs/libgnomevfs-2.la

libvfs_translate_la_SOURCES = translate-method.c
libvfs_translate_la_LDFLAGS = $(module_flags)
libvfs_translate_la_LIBADD  = $(MODULES_LIBS) ../libgnomevfs/libgnomevfs-2.la

libtar_la_SOURCES = tar-method.c tarpet.h 
libtar_la_LDFLAGS = $(module_flags)
libtar_la_LIBADD  = $(MODULES_LIBS) ../libgnomevfs/libgnomevfs-2.la

#if HAVE_LIBEFS
#vefsmoduledir = $(modulesdir)
#vefsmodule_LTLIBRARIES = libvefs.la
#
#libvefs_la_SOURCES =	\
#	efs-method.c
#libvefs_la_LIBADD = $(LIBEFS_LIBS) $(MODULES_LIBS)
#endif

