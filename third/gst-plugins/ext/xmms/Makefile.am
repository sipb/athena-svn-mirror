plugindir = $(libdir)/gstreamer-@GST_MAJORMINOR@

plugin_LTLIBRARIES = libgstxmms.la

libgstxmms_la_SOURCES = gstxmms.c pluginenum.c gstxmmsinput.c gstxmmseffect.c
## FIXME: we need to use stuff from the AM_PATH_XMMS macro
## but let's do this later
libgstxmms_la_CFLAGS = $(GST_CFLAGS) $(XMMS_CFLAGS) -DPLUGIN_DIR="\"`xmms-config --plugin-dir`\"" -DPLUGINSUBS="\"Output/\",\"Input/\",\"Effect/\",\"General/\",\"Visualization/\"" `xmms-config --cflags`
libgstxmms_la_LIBADD = $(XMMS_LIBS)
libgstxmms_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

noinst_HEADERS = effect.h  general.h  gstxmms.h  gstxmmseffect.h  gstxmmsinput.h  input.h  main.h  output.h  pluginenum.h  visualization.h  xmms.h

if HAVE_GTK
noinst_PROGRAMS = demo-mpg123-stereo
endif

demo_mpg123_stereo_SOURCES = demo-mpg123-stereo.c
demo_mpg123_stereo_CFLAGS  = $(GST_CFLAGS) $(GTK_CFLAGS)
demo_mpg123_stereo_LDFLAGS = $(GST_LIBS) $(GTK_LIBS)
