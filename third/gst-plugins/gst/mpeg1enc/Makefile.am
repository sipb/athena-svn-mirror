plugindir = $(libdir)/gstreamer-@GST_MAJORMINOR@

plugin_LTLIBRARIES = libgstmpeg1encoder.la

libgstmpeg1encoder_la_SOURCES = chendct.c     \
                          codec.c     \
                          huffman.c \
                          io.c \
                          marker.c \
                          me.c \
                          mem.c \
                          mpeg.c \
                          putbits.c \
                          stat.c \
                          stream.c \
                          transform.c \
                          gstmpeg1encoder.c

if HAVE_CPU_I386
ARCHCFLAGS = -O3 -mcpu=i486 -fschedule-insns2 $(FOMIT_FRAME_POINTER) -funroll-all-loops -finline-functions -ffast-math -DNDEBUG
else
ARCHCFLAGS = -O3 -fschedule-insns2 $(FOMIT_FRAME_POINTER) -funroll-all-loops -finline-functions -ffast-math -DNDEBUG
endif

libgstmpeg1encoder_la_CFLAGS = $(ARCHCFLAGS) $(GST_CFLAGS)
libgstmpeg1encoder_la_LIBADD =
libgstmpeg1encoder_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

noinst_HEADERS = csize.h \
                 ctables.h \
                 dct.h \
                 globals.h \
                 gstmpeg1encoder.h \
                 huffman.h \
                 marker.h \
                 mem.h \
                 param.h \
                 prototypes.h \
                 putbits.h \
                 qtables.h \
                 size.h \
                 stream.h \
                 system.h \
                 tables.h

EXTRA_DIST = CHANGES README SETUP PORTABILITY
