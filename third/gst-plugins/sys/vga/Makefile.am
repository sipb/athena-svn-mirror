plugindir = $(libdir)/gstreamer-@GST_MAJORMINOR@

plugin_LTLIBRARIES = libvgavideosink.la

libvgavideosink_la_SOURCES = vgavideosink.c

noinst_HEADERS = vgavideosink.h

## FIXME: maybe we should define VGA_CFLAGS and VGA_LIBS

libvgavideosink_la_CFLAGS = $(GST_CFLAGS)
libvgavideosink_la_LIBADD =
libvgavideosink_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS) -lvgagl -lvga
