INCLUDES =					\
	-I$(top_srcdir)				\
	$(EXTRA_GNOME_CFLAGS)			\
	$(ICONV_CFLAGS)				\
	-DG_LOG_DOMAIN=\"shortcut-bar\"

noinst_LTLIBRARIES = libshortcut-bar.la

libshortcut_bar_la_SOURCES = 	\
	e-group-bar.c		\
	e-icon-bar-bg-item.c	\
	e-icon-bar.c		\
	e-shortcut-bar.c	\
	e-shortcut-model.c	\
	e-vscrolled-bar.c

libshortcut_barincludedir = $(includedir)/gal-$(GAL_API_VERSION)/gal/shortcut-bar

libshortcut_barinclude_HEADERS = 	\
	e-group-bar.h		\
	e-icon-bar-bg-item.h	\
	e-icon-bar.h		\
	e-shortcut-bar.h	\
	e-shortcut-model.h	\
	e-vscrolled-bar.h
