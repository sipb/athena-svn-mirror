plugindir = $(libdir)/gstreamer-@GST_MAJORMINOR@

plugin_LTLIBRARIES = libgsthttpsrc.la

libgsthttpsrc_la_SOURCES = gsthttpsrc.c
libgsthttpsrc_la_CFLAGS = $(GST_CFLAGS)
libgsthttpsrc_la_LIBADD = $(HTTP_LIBS)
libgsthttpsrc_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

noinst_HEADERS = gsthttpsrc.h
