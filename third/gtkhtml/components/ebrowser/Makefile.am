
bin_PROGRAMS = ebrowser

INCLUDES = \
	-I$(top_srcdir)/src	\
	$(GNOME_INCLUDEDIR)	\
	-I$(top_srcdir)/components

CFLAGS += $(EXTRA_GNOME_CFLAGS) $(GCONF_CFLAGS)

EBROWSER_ENGINE_FILES = 		\
	ebrowser-history.c		\
	ebrowser-widget.c 		\
	e-cache.c 			\
	e-cache.h			\
	e-http-client.c 		\
	e-http-client.h 		\
	eloader.c 			\
	eloader.h 			\
	eloader-file.c 			\
	eloader-file.h 			\
	eloader-http.c 			\
	eloader-http.h			\
	eloader-moniker.c 		\
	eloader-moniker.h		\
	ebrowser-history.h		\
	ebrowser-widget.h

ebrowser_SOURCES = \
	$(EBROWSER_ENGINE_FILES) \
	ebrowser-stream.c ebrowser-stream.h \
	ebrowser.c ebrowser.h ebrowser-ui.c ebrowser-ui.h

LDADD = \
	$(top_builddir)/src/libgtkhtml-1.1.la \
	$(EXTRA_GNOME_LIBS) \
	$(GHTTP_LIBS)


noinst_PROGRAMS = test test-ebrowser test-history

test_SOURCES = \
	$(EBROWSER_ENGINE_FILES) \
	test.c test.h

test_ebrowser_SOURCES = \
	$(EBROWSER_ENGINE_FILES) \
	test-ebrowser.c test-ebrowser.h

test_history_SOURCES = \
	test-history.c ebrowser-history.c

oafdir = $(datadir)/oaf

oaf_in_files = GNOME_GtkHTML_EBrowser.oaf.in
oaf_DATA = $(oaf_in_files:.oaf.in=.oaf)

@XML_I18N_MERGE_OAF_RULE@

EXTRA_DIST = \
	$(oaf_in_files)	\
	$(oaf_DATA)	\
	ChangeLog
