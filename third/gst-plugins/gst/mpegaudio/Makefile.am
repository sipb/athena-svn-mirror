
plugin_LTLIBRARIES = libgstmpegaudio.la

libgstmpegaudio_la_SOURCES = \
	common.c          \
	encode.c          \
	gstmpegaudio.c    \
	musicin.c         \
	psy.c             \
	putbits.c         \
	subs.c            \
	tables.c            \
	tonal.c

libgstmpegaudio_la_CFLAGS = $(GST_CFLAGS)
libgstmpegaudio_la_LIBADD =
libgstmpegaudio_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

noinst_HEADERS = gstmpegaudio.h common.h  encoder.h musicin.h table_alloc.h \
                 table_enwindow.h table_absthr.h table_cb.h table_th.h \
		 putbits.h

EXTRA_DIST = README.crh
