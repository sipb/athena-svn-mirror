# librarydir = $(libdir)

noinst_LTLIBRARIES = libgstmedia-info.la

libgstmedia_info_la_SOURCES = media-info.c media-info-priv.c

noinst_HEADERS = media-info.h media-info-priv.h

libgstmedia_info_la_CFLAGS = $(GST_CFLAGS)
libgstmedia_info_la_LIBADD = $(GST_LIBS)

noinst_PROGRAMS = media-info-test

media_info_test_SOURCES = media-info-test.c
media_info_test_CFLAGS = $(GST_CFLAGS)
media_info_test_LDADD = $(GST_LIBS)
media_info_test_LDADD += libgstmedia-info.la
