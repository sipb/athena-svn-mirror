INCLUDES = $(THEME_ENGINE_CFLAGS) -DG_LOG_DOMAIN=\"SmoothEngine\"

AM_CFLAGS = $(THEME_ENGINE_CFLAGS) -DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED \
-DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED

enginedir = $(libdir)/gtk-2.0/$(GTK_BINARY_VERSION)/engines
engine_LTLIBRARIES = libsmooth.la

libsmooth_la_SOURCES = \
	utils/draw_grips.c	\
	utils/draw_grips.h	\
	utils/draw_fill.h	\
	utils/draw_fill.c	\
	utils/draw_arrows.c	\
	utils/draw_arrows.h	\
        utils/misc_functions.c	\
        utils/misc_functions.h	\
	smooth_gtk2_engine.c	\
	smooth_rc_style.c	\
	smooth_rc_style.h	\
	smooth_style.c	\
	smooth_style.h

libsmooth_la_LDFLAGS= -module -avoid-version
libsmooth_la_LIBADD = $(THEME_ENGINE_LIBS)

#EXTRA_DIST = AUTHORS README
