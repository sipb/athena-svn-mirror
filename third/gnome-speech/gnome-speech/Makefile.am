lib_LTLIBRARIES=libgnomespeech.la

INCLUDES = -I$(top_srcdir)          \
	$(WARN_CFLAGS) 		 \
	-I$(top_builddir)        \
	$(gnome_speech_CFLAGS)

libgnomespeech_la_LDFLAGS = $(gnome_speech_LIBS) @LT_VERSION_INFO@

IDL_OUT = \
	GNOME_Speech.h \
	GNOME_Speech-stubs.c \
	GNOME_Speech-skels.c \
	GNOME_Speech-common.c

IDL = \
	$(top_srcdir)/idl/GNOME_Speech.idl

BUILT_SOURCES = $(IDL_OUT) GNOME_Speech-imodule.c
CLEANFILES=$(BUILT_SOURCES)

IDLFLAGS = -I$(BONOBO_ACTIVATION_IDL_DIR) \
	-I$(LIBBONOBO_IDL_DIR)         \
	-I$(top_srcdir)/idl            \
	--add-imodule

$(BUILT_SOURCES): $(IDL) $(ORBIT_IDL)
	$(ORBIT_IDL) -D__GNOME_SPEECH_COMPILATION__ $(IDLFLAGS) $(IDL)

orbittypelibdir = $(libdir)/orbit-2.0
orbittypelib_LTLIBRARIES = GNOME_Speech_module.la
GNOME_Speech_module_la_LDFLAGS = \
	-export-dynamic -module -avoid-version

GNOME_Speech_module_la_SOURCES = \
	GNOME_Speech-imodule.c

libgnomespeechdir=$(includedir)/gnome-speech-1.0/gnome-speech

libgnomespeech_HEADERS=\
	GNOME_Speech.h \
	speaker.h \
	gnome-speech.h

libgnomespeech_la_SOURCES = \
	$(IDL_OUT) \
	speaker.c

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
