plugindir = $(libdir)/gstreamer-@GST_MAJORMINOR@

plugin_LTLIBRARIES = libgstvumeter.la

libgstvumeter_la_SOURCES = gstvumeter.c
libgstvumeter_la_CFLAGS = $(GST_CFLAGS)
libgstvumeter_la_LIBADD =
libgstvumeter_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

noinst_HEADERS = gstvumeter.h

EXTRA_DIST = README
