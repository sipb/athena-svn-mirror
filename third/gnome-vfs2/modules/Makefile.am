NULL =
SUBDIRS = vfolder

INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_builddir)			\
	-I$(top_srcdir)/imported/neon		\
	$(MODULES_XML_GCONF_CFLAGS)		\
	$(MODULES_FILE_CFLAGS) 			\
	$(HOWL_CFLAGS)				\
	$(LIBEFS_CFLAGS)			\
	$(SAMBA_CFLAGS)				\
	$(GSSAPI_CFLAGS)			\
	$(VFS_CFLAGS)				\
	-D_FILE_OFFSET_BITS=64			\
	-D_BSD_SOURCE				\
	-D_GNU_SOURCE				\
	-D_LARGEFILE64_SOURCE			\
	-D_POSIX_PTHREAD_SEMANTICS		\
	-D_REENTRANT				\
	-DSSH_PROGRAM=\"$(SSH_PROGRAM)\"	\
	-DG_DISABLE_DEPRECATED			\
	-DDATADIR=\"$(datadir)\"		\
	-DPREFIX=\"$(prefix)\"			\
	-DLIBDIR=\"$(libdir)\"			\
	-DSYSCONFDIR=\"$(sysconfdir)\"		\
	-DG_LOG_DOMAIN=\"gnome-vfs-modules\"	\
	$(NULL)

EXTRA_DIST =					\
	$(HTTP_NEON_SOURCES)			\
	$(HTTP_OLD_SOURCES)			\
	default-modules.conf			\
	smb-module.conf				\
	cdda-module.conf			\
	cdemenu-module.conf			\
	ssl-modules.conf

###  Module setup

if HAVE_CDDA
CDDA_LTLIBS = libcdda.la
else
CDDA_LTLIBS =
endif

if HAVE_SAMBA
SMB_LTLIBS = libsmb.la
else
SMB_LTLIBS = 
endif


if BUILD_CDEMENU_MODULE
CDEMENU_LTLIBS = libcdemenu-desktop.la
else
CDEMENU_LTLIBS =
endif

module_flags = -export_dynamic -avoid-version -module -no-undefined
modulesdir = $(libdir)/gnome-vfs-2.0/modules

modules_LTLIBRARIES =				\
	$(CDDA_LTLIBS)				\
	libcomputer.la				\
	libdns-sd.la				\
	libfile.la				\
	libnetwork.la				\
	libnntp.la				\
	libvfs-test.la				\
	$(HTTP_LTLIBS)				\
	libbzip2.la				\
	libgzip.la				\
	libftp.la				\
	libvfs-pipe.la				\
	libvfs-translate.la			\
	$(CDEMENU_LTLIBS)                       \
	$(SMB_LTLIBS)	                        \
	libsftp.la				\
	libtar.la				\
	$(NULL)

# Not currently supported
#	libnfs.la

modulesconfdir=$(sysconfdir)/gnome-vfs-2.0/modules
if HAVE_CDDA
CDDA_CONF = cdda-module.conf
endif
if HAVE_SAMBA
SMB_CONF = smb-module.conf
endif
if HAVE_SSL
SSL_CONF = ssl-modules.conf
endif
if BUILD_CDEMENU_MODULE
CDE_CONF = cdemenu-module.conf
endif
modulesconf_DATA = default-modules.conf $(CDDA_CONF) $(SMB_CONF) $(SSL_CONF) $(CDE_CONF)

###  `cdda' method

libcdda_la_SOURCES = \
	cdda-method.c 				\
	cdda-cddb.c 				\
	cdda-cddb.h				\
	cdda-cdrom-extensions.h

libcdda_la_LDFLAGS = $(module_flags)
libcdda_la_LIBADD = $(MODULES_LIBS) $(CDDA_LIBS) ../libgnomevfs/libgnomevfs-2.la

###  `computer' method

libcomputer_la_SOURCES = computer-method.c
libcomputer_la_LDFLAGS = $(module_flags)
libcomputer_la_LIBADD = $(MODULES_FILE_LIBS) ../libgnomevfs/libgnomevfs-2.la

###  `dns-sd' method

libdns_sd_la_SOURCES = dns-sd-method.c
libdns_sd_la_LDFLAGS = $(module_flags)
libdns_sd_la_LIBADD = $(MODULES_LIBS) $(HOWL_LIBS) ../libgnomevfs/libgnomevfs-2.la

###  `file' method

libfile_la_SOURCES = file-method.c fstype.c
libfile_la_LDFLAGS = $(module_flags)
libfile_la_LIBADD = $(FAM_LIBS) $(MODULES_FILE_LIBS) ../libgnomevfs/libgnomevfs-2.la

###  `network' method

libnetwork_la_SOURCES = network-method.c
libnetwork_la_LDFLAGS = $(module_flags)
libnetwork_la_LIBADD = $(MODULES_FILE_LIBS) ../libgnomevfs/libgnomevfs-2.la

###  `nntp' method

libnntp_la_SOURCES = nntp-method.c nntp-method.h
libnntp_la_LDFLAGS = $(module_flags)
libnntp_la_LIBADD = $(MODULES_LIBS) ../libgnomevfs/libgnomevfs-2.la

###  `cdemenu-desktop' method

libcdemenu_desktop_la_SOURCES = cdemenu-desktop-method.c
libcdemenu_desktop_la_LDFLAGS = $(module_flags)
libcdemenu_desktop_la_LIBADD = $(MODULES_LIBS) ../libgnomevfs/libgnomevfs-2.la

### `smb' method

libsmb_la_SOURCES = smb-method.c
libsmb_la_LDFLAGS = $(module_flags)
libsmb_la_LIBADD = $(SAMBA_LIBS) $(MODULES_LIBS) ../libgnomevfs/libgnomevfs-2.la 

###  `sftp' method

libsftp_la_SOURCES = sftp-method.c pty-open.c sftp.h pty-open.h
libsftp_la_LDFLAGS = $(module_flags)
libsftp_la_LIBADD = $(MODULES_LIBS) ../libgnomevfs/libgnomevfs-2.la

###  `test' method

libvfs_test_la_SOURCES = test-method.c
libvfs_test_la_LDFLAGS = $(module_flags)
libvfs_test_la_LIBADD = $(MODULES_XML_LIBS) ../libgnomevfs/libgnomevfs-2.la

### `gzip' method

libgzip_la_SOURCES = gzip-method.c
libgzip_la_LDFLAGS = $(module_flags)
libgzip_la_LIBADD = $(MODULES_LIBS) -lz ../libgnomevfs/libgnomevfs-2.la

### `bzip2' method

libbzip2_la_SOURCES = bzip2-method.c
libbzip2_la_LDFLAGS = $(module_flags)
libbzip2_la_LIBADD = $(MODULES_LIBS) $(BZ2_LIBS) ../libgnomevfs/libgnomevfs-2.la

### `ftp' method

libftp_la_SOURCES = ftp-method.c
libftp_la_LDFLAGS = $(module_flags)
libftp_la_LIBADD = $(MODULES_LIBS) $(GSSAPI_LIBS) ../libgnomevfs/libgnomevfs-2.la

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

## The old module
HTTP_OLD_SOURCES = 				\
	http-method.c				\
	http-method.h				\
	http-authn.c				\
	http-authn.h				\
	http-cache.c				\
	http-cache.h				

HTTP_OLD_LIBADD =				\
	$(MODULES_GCONF_LIBS) 		\
	$(MODULES_XML_LIBS) 		\
	../libgnomevfs/libgnomevfs-2.la

## The neon module
HTTP_NEON_SOURCES = 			\
	http-neon-method.c		\
	http-proxy.h			\
	http-proxy.c

HTTP_NEON_LIBADD =				\
		$(MODULES_GCONF_LIBS) 	\
	../imported/neon/libneon.la   \
	../libgnomevfs/libgnomevfs-2.la

if USE_HTTP_NEON
HTTP_SOURCES = $(HTTP_NEON_SOURCES)
HTTP_LIBS = $(HTTP_NEON_LIBADD)
else
HTTP_SOURCES = $(HTTP_OLD_SOURCES)
HTTP_LIBS = $(HTTP_OLD_LIBADD)
endif

libhttp_la_SOURCES =			\
	$(HTTP_SOURCES)			\
	$(NULL)

libhttp_la_LDFLAGS = $(module_flags)
libhttp_la_LIBADD  = $(HTTP_LIBS)

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

