plugindir = $(libdir)/gstreamer-@GST_MAJORMINOR@

plugin_LTLIBRARIES = libgstmpeg2types.la libgstmpeg1types.la

libgstmpeg2types_la_SOURCES = gstmpeg2types.c
libgstmpeg2types_la_CFLAGS = -O2 $(FOMIT_FRAME_POINTER) -funroll-all-loops -finline-functions -ffast-math $(GST_CFLAGS)
libgstmpeg2types_la_LIBADD =
libgstmpeg2types_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

libgstmpeg1types_la_SOURCES = gstmpeg1types.c
libgstmpeg1types_la_CFLAGS = $(GST_CFLAGS)
libgstmpeg1types_la_LIBADD =
libgstmpeg1types_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
