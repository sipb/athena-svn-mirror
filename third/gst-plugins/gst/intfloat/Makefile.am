plugindir = $(libdir)/gstreamer-@GST_MAJORMINOR@

plugin_LTLIBRARIES = libgstintfloat.la

libgstintfloat_la_SOURCES = gstintfloatconvert.c gstint2float.c gstfloat2int.c
libgstintfloat_la_CFLAGS = $(GST_CFLAGS)
libgstintfloat_la_LIBADD =
libgstintfloat_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

noinst_HEADERS = gstint2float.h gstfloat2int.h
