if BUILD_FREETTS_DRIVER
lib_LTLIBRARIES=libfreettsdriver.la

if BUILD_FREETTS_DRIVER_LINUX
INCLUDES = \
	$(WARN_CFLAGS)		     \
	-I$(JAVA_HOME)/include 	     \
	-I$(JAVA_HOME)/include/linux \
	$(gnome_speech_CFLAGS)
endif
if BUILD_FREETTS_DRIVER_SOLARIS
INCLUDES = \
	-I $(JAVA_HOME)/include \
	-I $(JAVA_HOME)/include/solaris \
	$(gnome_speech_CFLAGS)
endif

AM_LDFLAGS = \
	$(gnome_speech_LIBS) \
	@LT_VERSION_INFO@

freettsdriver.h:
	$(JAVAH) \
	-classpath ../java:$(GNOME_SPEECH_CLASSPATH):$(FREETTS_CLASSPATH) \
	-o freettsdriver.h \
	org.GNOME.Speech.FreeTTSSynthesisDriver

CLEANFILES = freettsdriver.h

libfreettsdriver_la_SOURCES = \
	freettsdriver.c
$(libfreettsdriver_la_SOURCES): freettsdriver.h
endif
