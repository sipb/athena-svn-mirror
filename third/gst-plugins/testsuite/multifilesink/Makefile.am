testprogs = fakesrc_test lame_test oggvorbis_test oggtheora_test wavenc_test multifilesrc_test

noinst_PROGRAMS = $(testprogs)

fakesrc_test_SOURCES = fakesrc_test.c
lame_test_SOURCES = lame_test.c
oggvorbis_test_SOURCES = oggvorbis_test.c
oggtheora_test_SOURCES = oggtheora_test.c
wavenc_test_SOURCES = wavenc_test.c
multifilesrc_test_SOURCES = multifilesrc_test.c

# we have nothing but apps here, we can do this safely
LIBS = $(GST_LIBS)
AM_CFLAGS = $(GST_CFLAGS)
