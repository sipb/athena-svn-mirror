NULL =
SUBDIRS = extfs

INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/libgnomevfs		\
	-I$(top_builddir)/libgnomevfs		\
	-I$(top_srcdir)/intl			\
	-I$(top_builddir)/intl			\
	$(GLIB_CFLAGS)				\
	$(XML_CFLAGS)	 			\
	$(LIBEFS_CFLAGS)			\
	$(GCONF_CFLAGS)				\
	$(GTK_CFLAGS)				\
	$(VFS_CFLAGS) $(WERROR)			\
	-D_GNU_SOURCE				\
	-D_REENTRANT				\
	-D_POSIX_PTHREAD_SEMANTICS		\
	-DPREFIX=\"$(prefix)\"

EXTRA_DIST =					\
	default-modules.conf			\
	efs-method.c                            \
	efs-method.h



###  Module setup

module_flags = -export_dynamic -avoid-version
modulesdir = $(libdir)/vfs/modules

modules_LTLIBRARIES =				\
	libextfs.la				\
	libfile.la				\
	libvfs-test.la				\
	libhttp.la				\
	libbzip2.la				\
	libgzip.la				\
	libftp.la				\
	libvfs-pipe.la				\
	libvfs-translate.la			\
	$(NULL)

# Not currently supported
#	libnfs.la				\
#	libcdda-method.la			\

modulesconfdir=$(sysconfdir)/vfs/modules
modulesconf_DATA =				\
	default-modules.conf


###  `cdda' method
#libcdda_la_SOURCES = 				\
#	cdda-method.c 				\
#	cdda-method.h				\
#	cdda-cddb.c 				\
#	cdda-cddb.h

#libcdda_la_LDFLAGS = $(module_flags)
#libcdda_la_LIBADD = -L/gnome/lib -lcdda_paranoia -lcdda_interface

###  `extfs' method

libextfs_la_SOURCES = 				\
	extfs-method.c 				\
	extfs-method.h

libextfs_la_LDFLAGS = $(module_flags)


###  `file' method

libfile_la_SOURCES = 				\
	file-method.c 				\
	file-method.h

libfile_la_LDFLAGS = $(module_flags)


###  `test' method

libvfs_test_la_SOURCES =			\
	test-method.c				
libvfs_test_la_LIBADD = $(XML_LIBS)

### `gzip' method

libgzip_la_SOURCES = 				\
	gzip-method.c 				\
	gzip-method.h
libgzip_la_LDFLAGS = $(module_flags)
libgzip_la_LIBADD = -lz


### `bzip2' method

libbzip2_la_SOURCES =				\
	bzip2-method.c				\
	bzip2-method.h
libbzip2_la_LDFLAGS = $(module_flags)
libbzip2_la_LIBADD = -lbz2


### `ftp' method

libftp_la_SOURCES = 				\
	ftp-method.c				\
	ftp-method.h

### `gconf' method

gconfmoduledir = $(modulesdir)
gconfmodule_LTLIBRARIES = libgconf.la

libgconf_la_SOURCES =                           \
	gconf-method.c                          \
        gconf-method.h
libgconf_la_LIBADD = $(GCONF_LIBS) $(GTK_LIBS)

libgconf_la_LDFLAGS = $(module_flags)

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

libhttp_la_SOURCES =				\
	http-method.c				\
	http-method.h
libhttp_la_LDFLAGS = $(module_flags)
libhttp_la_LIBADD  = $(GLIB_LIBS) $(XML_LIBS) $(GCONF_LIBS) $(GTK_LIBS)

libvfs_pipe_la_SOURCES =			\
	pipe-method.c				\
	pipe-method.h
libvfs_pipe_la_LDFLAGS = $(module_flags)
libvfs_pipe_la_LIBADD  = $(GLIB_LIBS)

libvfs_translate_la_SOURCES =			\
	translate-method.c
libvfs_translate_la_LDFLAGS = $(module_flags)
libvfs_translate_la_LIBADD  = $(GLIB_LIBS)

#if HAVE_LIBEFS
#vefsmoduledir = $(modulesdir)
#vefsmodule_LTLIBRARIES = libvefs.la
#
#libvefs_la_SOURCES =	\
#	efs-method.c	\
#        efs-method.h
#libvefs_la_LIBADD = $(LIBEFS_LIBS) $(GTK_LIBS)
#endif
