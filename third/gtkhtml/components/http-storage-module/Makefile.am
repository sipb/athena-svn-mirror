PLUGIN_DIR=$(libdir)/bonobo/plugin

INCLUDES =                                              \
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
	-DPLUGIN_DIR=\""$(PLUGIN_DIR)"\"		\
        -I$(includedir)                                 \
	$(EXTRA_GNOME_CFLAGS)

common_ldflags = -avoid-version

bonobo_plugindir = $(PLUGIN_DIR)
bonobo_plugin_LTLIBRARIES = libstorage_http.la

#
# HTTP storage module
#
libstorage_http_la_SOURCES =	\
	bonobo-storage-http.c	\
	bonobo-storage-http.h	\
	bonobo-stream-http.c	\
	bonobo-stream-http.h

libstorage_http_la_LDFLAGS =	\
	$(common_ldflags)	\
	$(GHTTP_LIBS)
