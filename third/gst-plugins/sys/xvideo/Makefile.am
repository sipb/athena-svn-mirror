plugindir = $(libdir)/gstreamer-@GST_MAJORMINOR@

plugin_LTLIBRARIES = libgstxvideosink.la

# -lXv must link against -lXext (XFree86 4.1.0)
## FIXME: check these flags
## if HAVE_LIBXV
## GSTXVLIBS = -lXv -lXext
## else
## GSTXVLIBS =
## endif

#CFLAGS += -O3 -ffast-math -finline-functions -L/usr/X11R6/lib

libgstxvideosink_la_SOURCES =  xvideosink.c gstxwindow.c gstximage.c gstxvimage.c 
libgstxvideosink_la_CFLAGS = $(GST_CFLAGS)
libgstxvideosink_la_LIBADD = $(X_LIBS) $(XVIDEO_LIBS) 
libgstxvideosink_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

noinst_HEADERS = xvideosink.h gstxwindow.h gstximage.h gstxvimage.h gstimage.h

# FIXME: do we still need this test ? move it somewhere else ?
# check_PROGRAMS = imagetest
# 
# imagetest_SOURCES = imagetest.c gstxwindow.c gstximage.c gstxvimage.c
# imagetest_CFLAGS = `gnome-config --cflags gnome gnomeui`
# imagetest_LDFLAGS = $(shell gnome-config --libs gnome gnomeui) 
# imagetest_LDADD = $(GLIB_LIBS) $(GTK_LIBS) $(GSTXVLIBS) $(top_srcdir)/gst/libgst.la
