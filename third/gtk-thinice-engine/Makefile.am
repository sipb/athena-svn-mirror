SUBDIRS = Theme

INCLUDES = $(THINICE_CFLAGS)

enginedir = $(libdir)/gtk-2.0/$(GTK_VERSION)/engines

engine_LTLIBRARIES = libthinice.la

libthinice_la_SOURCES = 	\
	thinice_main.c		\
	thinice_rc_style.c	\
	thinice_rc_style.h	\
	thinice_style.h	        \
	thinice_theme_draw.c

libthinice_la_LDFLAGS = -module -avoid-version

libthinice_la_LIBADD =  $(THINICE_LIBS)

EXTRA_DIST = autogen.sh gtk-thinice-engine-2.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gtk-thinice-engine-2.pc
