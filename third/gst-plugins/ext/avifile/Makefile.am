plugindir = $(libdir)/gstreamer-@GST_MAJORMINOR@

plugin_LTLIBRARIES = libgstwincodec.la

libgstwincodec_la_SOURCES = gstwindec.cc gstwincodec.cc gstwinenc.cc

libgstwincodec_la_CXXFLAGS = $(GST_CFLAGS) $(AVIFILE_CXXFLAGS)
### we add ldstc++ because it'll be used by C apps
libgstwincodec_la_LIBADD = $(AVIFILE_LDFLAGS) -lstdc++
libgstwincodec_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

noinst_HEADERS = gstwindec.h gstwinenc.h
