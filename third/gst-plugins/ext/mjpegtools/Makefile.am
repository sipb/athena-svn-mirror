plugindir = $(libdir)/gstreamer-@GST_MAJORMINOR@

plugin_LTLIBRARIES = libgstjpegmmxdec.la libgstjpegmmxenc.la

libgstjpegmmxdec_la_SOURCES = gstjpegmmxdec.c
libgstjpegmmxdec_la_CFLAGS = $(GST_CFLAGS) $(MJPEGTOOLS_CFLAGS)
libgstjpegmmxdec_la_LIBADD = $(MJPEGTOOLS_LIBS) -llavjpeg -lmjpegutils
libgstjpegmmxdec_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

libgstjpegmmxenc_la_SOURCES = gstjpegmmxenc.c
libgstjpegmmxenc_la_CFLAGS = $(GST_CFLAGS) $(MJPEGTOOLS_CFLAGS)
libgstjpegmmxenc_la_LIBADD = $(MJPEGTOOLS_LIBS) -llavjpeg -lmjpegutils
libgstjpegmmxenc_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

noinst_HEADERS = gstjpegmmxdec.h gstjpegmmxenc.h
