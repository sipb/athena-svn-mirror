plugindir = $(libdir)/gstreamer-@GST_MAJORMINOR@

plugin_LTLIBRARIES = libgstmpeg2enc.la

if HAVE_CPU_I386
ASM_SRCS =     bdist1_sse.s       \
	       bdist1_mmx.s       \
	       bdist2_mmx.s       \
	       dist1_00_mmx.s     \
	       dist1_01_mmx.s     \
	       dist1_10_mmx.s     \
	       dist1_11_mmx.s     \
	       dist2_mmx.s     	  \
	       variance_mmx.s     \
	       dist1_00_sse.s     \
	       dist1_01_sse.s     \
	       dist1_10_sse.s     \
	       dist1_11_sse.s     \
	       predict_mmx.s 	  \
	       quant_mmx.s 	  \
	       fdct_mmx.s 	  \
	       transfrm_mmx.s 	  \
	       predict_sse.s 
else
ASM_SRCS = 
endif

libgstmpeg2enc_la_SOURCES = conform.c     	\
                          fdctref.c     \
                          fdctdata.c     \
                          motion.c 	\
                          predict.c 	\
                          puthdr.c 	\
                          putmpg.c 	\
                          putpic.c 	\
                          putseq.c 	\
                          putvlc.c 	\
                          quantize.c 	\
                          ratectl.c 	\
                          readpic.c 	\
                          stats.c 	\
                          tables.c 	\
                          transfrm.c 	\
                          mpeg2enc.c 	\
                          gstmpeg2enc.c \
			  $(ASM_SRCS)

if HAVE_CPU_I386
ARCHCFLAGS = -mcpu=i486
else
ARCHCFLAGS = 
endif

libgstmpeg2enc_la_CFLAGS = -O3 $(ARCHCFLAGS) -fschedule-insns2 $(FOMIT_FRAME_POINTER) -finline-functions -ffast-math $(GST_CFLAGS)
libgstmpeg2enc_la_LIBADD =
libgstmpeg2enc_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

noinst_HEADERS = mpeg2enc.h \
		 gstmpeg2enc.h \
                 vlc.h       

EXTRA_DIST = CHANGES TODO
