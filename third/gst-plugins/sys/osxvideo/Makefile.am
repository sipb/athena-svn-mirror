OBJC=gcc
AM_OBJCFLAGS=$(CFLAGS) $(GST_CFLAGS)

plugin_LTLIBRARIES = libgstosxvideosink.la

libgstosxvideosink_la_SOURCES =  osxvideosink.m cocoawindow.m 
libgstosxvideosink_la_CFLAGS = $(GST_CFLAGS) 
libgstosxvideosink_la_LIBADD = $(top_builddir)/gst-libs/gst/libgstinterfaces-$(GST_MAJORMINOR).la
LIBS = -Wl,-framework -Wl,Cocoa -Wl,-framework -Wl,QuickTime -Wl,-framework -Wl,OpenGL
libgstosxvideosink_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS) -framework Cocoa -framework QuickTime -framework OpenGL

noinst_HEADERS = osxvideosink.h cocoawindow.h
