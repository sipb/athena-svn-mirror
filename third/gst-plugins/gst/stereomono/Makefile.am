plugindir = $(libdir)/gstreamer-@GST_MAJORMINOR@

plugin_LTLIBRARIES = libgststereo2mono.la libgstmono2stereo.la libgststereosplit.la

libgststereo2mono_la_SOURCES = gststereo2mono.c
libgststereo2mono_la_CFLAGS = $(GST_CFLAGS)
libgststereo2mono_la_LIBADD =
libgststereo2mono_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

libgststereosplit_la_SOURCES = gststereosplit.c
libgststereosplit_la_CFLAGS = $(GST_CFLAGS)
libgststereosplit_la_LIBADD =
libgststereosplit_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

libgstmono2stereo_la_SOURCES = gstmono2stereo.c
libgstmono2stereo_la_CFLAGS = $(GST_CFLAGS)
libgstmono2stereo_la_LIBADD =
libgstmono2stereo_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

noinst_HEADERS = gststereo2mono.h gstmono2stereo.h
