plugindir = $(libdir)/gstreamer-@GST_MAJORMINOR@

plugin_LTLIBRARIES = libgstmpegaudio.la

libgstmpegaudio_la_SOURCES = \
	common.c          \
	encode.c          \
	gstmpegaudio.c    \
	musicin.c         \
	psy.c             \
	subs.c            \
	tables.c            \
	tonal.c

if HAVE_CPU_I386
ARCHCFLAGS = -O2 -mcpu=i486 $(FOMIT_FRAME_POINTER) -finline-functions -ffast-math
else
ARCHCFLAGS = -O2 $(FOMIT_FRAME_POINTER) -finline-functions -ffast-math
endif

libgstmpegaudio_la_CFLAGS = $(ARCHCFLAGS) $(GST_CFLAGS)
libgstmpegaudio_la_LIBADD =
libgstmpegaudio_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

noinst_HEADERS = gstmpegaudio.h common.h  encoder.h musicin.h table_alloc.h \
                 table_enwindow.h table_absthr.h table_cb.h table_th.h

EXTRA_DIST = README.crh
