plugindir = $(libdir)/gstreamer-@GST_MAJORMINOR@

plugin_LTLIBRARIES = libgstrtp.la

libgstrtp_la_SOURCES = gstrtp.c gstrtpsend.c gstrtprecv.c gstrtp-common.c rtp-packet.c rtp.c rtcp-packet.c md5c.c net.c

libgstrtp_la_CFLAGS = $(GST_CFLAGS)
libgstrtp_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

noinst_HEADERS = gstrtpsend.h gstrtprecv.h gstrtp-common.h rtcp-packet.h rtp.h md5.h rtp-audio.h rtp-packet.h
