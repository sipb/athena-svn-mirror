SUBDIRS = Theme
INCLUDES = $(GTK_CFLAGS)

enginedir = $(libdir)/gtk-2.0/$(GTK_VERSION)/engines

# replace the "metal" prefix with whatever you're using
engine_LTLIBRARIES = libmetal.la

libmetal_la_SOURCES = 		\
	metal_rc_style.c	\
	metal_rc_style.h	\
	metal_style.c		\
	metal_style.h		\
	metal_theme_main.c

libmetal_la_LDFLAGS = -module -avoid-version
libmetal_la_LIBADD = # $(GTK_LIBS)

