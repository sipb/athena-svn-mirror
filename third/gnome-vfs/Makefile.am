if USING_OAF
SUBDIRS_OAF=libgnomevfs-corba
else
SUBDIRS_OAF=
endif

if PLATFORM_GNOME_2
SUBDIRS_GNOME=
else
SUBDIRS_GNOME=test
endif

SUBDIRS =					\
	libgnomevfs				\
	libgnomevfs-pthread			\
	$(SUBDIRS_OAF)				\
	idl					\
	modules					\
	data					\
	man					\
	devel-docs				\
	$(SUBDIRS_GNOME)			\
	po					\
	macros


EXTRA_DIST =					\
	HACKING					\
	TODO					\
	autogen.sh				\
	gnome-vfs.spec				\
	vfsConf.sh.in				\
	vfscorbaConf.sh.in			\
	vfspthreadConf.sh.in			\
	xml-i18n-extract.in			\
	xml-i18n-merge.in			\
	xml-i18n-update.in			\
	gnome-vfs-config.in

bin_SCRIPTS = $(top_builddir)/gnome-vfs-config

confexecdir=$(libdir)
confexec_DATA =					\
	vfsConf.sh				\
	vfscorbaConf.sh				\
	vfspthreadConf.sh

## [The following is shamelessly copied from gnome-print.]

## We create `vfsConf.sh', `vfscorbaConf.sh' and `vfspthreadConf.sh' here and
## not from configure because we want to get the paths expanded correctly.
## Macros like srcdir are given the value NONE in configure if the user doesn't
## specify them (this is an autoconf feature, not a bug).

vfsConf.sh: vfsConf.sh.in Makefile
	sed -e 's?\@VFS_LIBDIR\@?$(VFS_LIBDIR)?g' \
	    -e 's?\@VFS_INCLUDEDIR\@?$(VFS_INCLUDEDIR)?g' \
	    -e 's?\@VERSION\@?$(VERSION)?g' \
	    -e 's?\@VFS_LIBS\@?$(VFS_LIBS)?g' \
	      < $(srcdir)/vfsConf.sh.in > vfsConf.tmp \
	  && mv vfsConf.tmp vfsConf.sh

vfscorbaConf.sh: vfscorbaConf.sh.in Makefile
	sed -e 's?\@VFSCORBA_LIBDIR\@?$(VFSCORBA_LIBDIR)?g' \
	    -e 's?\@VFSCORBA_INCLUDEDIR\@?$(VFSCORBA_INCLUDEDIR)?g' \
	    -e 's?\@VERSION\@?$(VERSION)?g' \
	    -e 's?\@VFSCORBA_LIBS\@?$(VFSCORBA_LIBS)?g' \
	      < $(srcdir)/vfscorbaConf.sh.in > vfscorbaConf.tmp \
	  && mv vfscorbaConf.tmp vfscorbaConf.sh

vfspthreadConf.sh: vfspthreadConf.sh.in Makefile
	sed -e 's?\@VFSPTHREAD_LIBDIR\@?$(VFSPTHREAD_LIBDIR)?g' \
	    -e 's?\@VFSPTHREAD_INCLUDEDIR\@?$(VFSPTHREAD_INCLUDEDIR)?g' \
	    -e 's?\@VERSION\@?$(VERSION)?g' \
	    -e 's?\@VFSPTHREAD_LIBS\@?$(VFSPTHREAD_LIBS)?g' \
	      < $(srcdir)/vfspthreadConf.sh.in > vfspthreadConf.tmp \
	  && mv vfspthreadConf.tmp vfspthreadConf.sh

gnome-vfs-config: gnome-vfs-config.in Makefile
	sed -e 's?\@LIBDIR\@?$(libdir)?g' \
	    -e 's?\@VFS_LIBDIR\@?$(VFS_LIBDIR)?g' \
	    -e 's?\@VFS_INCLUDEDIR\@?$(VFS_INCLUDEDIR)?g' \
	    -e 's?\@VERSION\@?$(VERSION)?g' \
	    -e 's?\@VFS_LIBS\@?$(VFS_LIBS)?g' \
	      < $(srcdir)/gnome-vfs-config.in > gnome-vfs-config.tmp \
	  && mv gnome-vfs-config.tmp gnome-vfs-config && \
		chmod 755 gnome-vfs-config


