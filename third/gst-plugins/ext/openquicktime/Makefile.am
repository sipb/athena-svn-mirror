plugindir = $(libdir)/gstreamer-@GST_MAJORMINOR@

plugin_LTLIBRARIES = libgstopenquicktimedecoder.la libgstopenquicktimedemux.la libgstopenquicktimetypes.la

libgstopenquicktimedecoder_la_SOURCES = gstopenquicktimedecoder.c
libgstopenquicktimedecoder_la_CFLAGS = $(GST_CFLAGS)
libgstopenquicktimedecoder_la_LIBADD = $(OPENQUICKTIME_LIBS)
libgstopenquicktimedecoder_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

libgstopenquicktimedemux_la_SOURCES = gstopenquicktimedemux.c
libgstopenquicktimedemux_la_CFLAGS = $(GST_CFLAGS)
libgstopenquicktimedemux_la_LIBADD = $(OPENQUICKTIME_LIBS)
libgstopenquicktimedemux_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

libgstopenquicktimetypes_la_SOURCES = gstopenquicktimetypes.c
libgstopenquicktimetypes_la_CFLAGS = $(GST_CFLAGS)
libgstopenquicktimetypes_la_LIBADD =
libgstopenquicktimetypes_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

noinst_HEADERS = gstopenquicktimedecoder.h gstopenquicktimedemux.h gstopenquicktimetypes.h

#CFLAGS += -O $(FOMIT_FRAME_POINTER) -march=i686 -ffast-math -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -fomit-frame-pointer -malign-loops=2 -malign-jumps=2 -malign-functions=2
