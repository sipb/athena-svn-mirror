plugindir = $(libdir)/gstreamer-@GST_MAJORMINOR@

plugin_LTLIBRARIES = libgstvideosink.la

libgstvideosink_la_SOURCES = gstvideosink.c gstxwindow.c gstximage.c gstxvimage.c
libgstvideosink_la_CFLAGS = $(GST_CFLAGS)
libgstvideosink_la_LIBADD = $(X_LIBS) $(XVIDEO_LIBS) 
libgstvideosink_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

noinst_HEADERS = gstvideosink.h
