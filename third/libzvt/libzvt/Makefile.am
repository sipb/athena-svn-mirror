ptyhelperdir = $(libexecdir)/libzvt-2.0

noinst_PROGRAMS = zterm test-utmp
ptyhelper_PROGRAMS = gnome-pty-helper
lib_LTLIBRARIES = libzvt-2.0.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libzvt-2.0.pc

INCLUDES = \
	-I$(top_srcdir) 				\
	$(ZVT_CFLAGS)					\
        -DGNOMEZVTLIBDIR=\""$(libdir)"\" 		\
        -DGNOMEZVTDATADIR=\""$(datadir)"\" 		\
        -DGNOMEZVTPIXMAPDIR=\""$(datadir)/pixmaps"\"	\
        -DGNOMEZVTBINDIR=\""$(bindir)"\" 		\
        -DGNOMEZVTLOCALSTATEDIR=\""$(localstatedir)"\" 	\
        -DGNOMEZVTLOCALEDIR=\""$(gnomelocaledir)"\" 	\
	-DGTK_VERSION=\""$(GTK_VERSION)"\"		\
	-DG_LOG_DOMAIN=\"ZVT\"				\
	-DPTY_HELPER_DIR=\""$(ptyhelperdir)"\"		\
	-DG_DISABLE_DEPRECATED		\
	-DGDK_DISABLE_DEPRECATED	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED \
	-DGTK_DISABLE_DEPRECATED

libzvt_2_includedir = $(includedir)/libzvt-2.0/libzvt

libzvt_2_include_HEADERS = \
	libzvt.h lists.h background.h vt.h vtx.h

libzvt_2_0_la_SOURCES = \
	libzvt.h \
	libzvt-init.c \
	gnome-login-support.h	\
	gnome-login-support.c	\
	lists.h			\
	lists.c			\
	subshell.h		\
	subshell.c		\
	subshell-includes.h	\
	update.c		\
	vt.c			\
	vt.h			\
	vtx.h			\
	background.c		\
	background.h		\
	zvtterm.c		\
	zvt-marshal.c		\
	zvt-marshal.h		\
	zvt-accessible.c	\
	zvt-accessible.h	\
	zvt-accessible-factory.c	\
	zvt-accessible-factory.h

libzvt_2_0_la_LIBADD = $(ZVT_LIBS)

libzvt_2_0_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

$(srcdir)/zvt-marshal.h: Makefile.am zvt-marshal.list
	cd $(srcdir) \
	&& glib-genmarshal --prefix=zvt_marshal zvt-marshal.list --header > xgen-gmh \
	&& (cmp -s xgen-gmh zvt-marshal.h || cp xgen-gmh zvt-marshal.h) \
	&& rm -f xgen-gmh xgen-gmh~

$(srcdir)/zvt-marshal.c: Makefile.am zvt-marshal.list
	cd $(srcdir) \
	&& echo "#include \"zvt-marshal.h\"" > xgen-gmc \
	&& glib-genmarshal --prefix=zvt_marshal zvt-marshal.list --body >> xgen-gmc \
	&& (cmp -s xgen-gmc zvt-marshal.c || cp xgen-gmc zvt-marshal.c) \
	&& rm -f xgen-gmc xgen-gmc~

BUILT_SOURCES = 	\
	zvt-marshal.h	\
	zvt-marshal.c

gnome_pty_helper_SOURCES=   \
	gnome-pty-helper.c    \
        gnome-login-support.c \
        gnome-utmp.c

gnome_pty_helper_LDADD= $(UTIL_LIBS)

# Since we do gnome-pty-helper manually, include the relevant sources here
EXTRA_DIST = TODO BUGS README	\
	gnome-pty.h gnome-pty-helper.c gnome-utmp.c zvt-marshal.list zvt-marshal.c

zterm_SOURCES	= zterm.c
zterm_INCLUDES	= $(INCLUDES)

zterm_LDADD	= 			\
	libzvt-2.0.la			\
	$(ZVT_LIBS) $(UTIL_LIBS)

test_utmp_SOURCES =			\
	gnome-utmp.c			\
	test-utmp.c

test_utmp_LDADD =

install-exec-local: install-ptyhelperPROGRAMS
	-file=$(DESTDIR)$(ptyhelperdir)/gnome-pty-helper; \
	chown @PTY_HELPER_UID@ $$file && chgrp @PTY_HELPER_GID@ $$file && chmod u+s $$file
