INCLUDES = $(THEME_ENGINE_CFLAGS) -DG_LOG_DOMAIN=\"HcEngine\"

enginedir = $(libdir)/gtk-2.0/$(GTK_VERSION)/engines

engine_LTLIBRARIES = libhcengine.la

libhcengine_la_SOURCES = \
	hc-main.c\
	hc-rc-style.c	\
	hc-rc-style.h	\
	hc-style.c	\
	hc-style.h

libhcengine_la_LDFLAGS= -module -avoid-version
libhcengine_la_LIBADD = $(THEME_ENGINE_LIBS)
