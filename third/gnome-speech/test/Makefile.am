bin_PROGRAMS = test-speech

INCLUDES = -I$(top_srcdir)          \
	-I$(top_builddir)\
	-I$(top_srcdir)/gnome-speech\
	$(gnome_speech_CFLAGS)

LDADD = $(top_builddir)/gnome-speech/libgnomespeech.la \
	$(gnome_speech_LIBS)\
	@LT_VERSION_INFO@

test_speech_SOURCES = \
	test-speech.c \
	callback.c

EXTRA_DIST = callback.h
