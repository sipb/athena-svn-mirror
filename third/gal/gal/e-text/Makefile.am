INCLUDES =					\
	-I$(top_srcdir)				\
	$(GNOME_INCLUDEDIR)			\
	$(ICONV_CFLAGS)				\
	-DG_LOG_DOMAIN=\"e-text\"		\
	$(EXTRA_GNOME_CFLAGS)

noinst_LTLIBRARIES = libetext.la

# WARNING
# WARNING Be sure to add any files here into the parent Makefile too.
# WARNING
libetext_la_SOURCES = 		\
	e-entry.c				\
	e-completion-match.c			\
	e-completion.c				\
	e-completion-view.c			\
	e-table-text-model.c			\
	e-text-model-repos.c			\
	e-text-model.c				\
	e-text-model-uri.c                      \
	e-text.c

libetextincludedir = $(includedir)/gal/e-text

libetextinclude_HEADERS = 			\
	e-entry.h				\
	e-completion-match.h			\
	e-completion.h				\
	e-completion-view.h			\
	e-table-text-model.h			\
	e-text-model-repos.h			\
	e-text-model.h				\
	e-text-model-uri.h			\
	e-text.h

noinst_PROGRAMS =	\
	e-completion-test
#	e-text-model-test \
#
#	e-text-test	\
#	e-entry-test	\



#e_text_test_SOURCES =	\
#	 e-text-test.c

#e_text_test_LDADD =				\
#	 libetext.la				\
#	 $(EXTRA_GNOME_LIBS)			\
#	 $(top_builddir)/gal/widgets/libwidgets.la \
#	 $(top_builddir)/gal/unicode/libunicode.la \
#	 $(top_builddir)/gal/util/libutil.la

#e_entry_test_SOURCES =	\
#	 e-entry-test.c

#e_entry_test_LDADD =				\
#	 libetext.la				\
#	 $(EXTRA_GNOME_LIBS)			\
#	 $(top_builddir)/gal/e-table/libetable.la \
#	 $(top_builddir)/gal/widgets/libwidgets.la \
#	 $(top_builddir)/gal/unicode/libunicode.la \
#	 $(top_builddir)/gal/util/libutil.la

#e_text_model_test_SOURCES = \
#	e-text-model-test.c

#e_text_model_test_LDADD = \
#	libetext.la 		\
#	$(EXTRA_GNOME_LIBS)	\
#	 $(top_builddir)/gal/widgets/libwidgets.la \
#	 $(top_builddir)/gal/unicode/libunicode.la \
#	 $(top_builddir)/gal/util/libutil.la

e_completion_test_SOURCES = \
	e-completion-test.c

e_completion_test_LDADD = \
	libetext.la 		\
	$(top_builddir)/gal/e-table/libetable.la \
	$(EXTRA_GNOME_LIBS)	\
	$(ICONV_LIBS)		\
	$(top_builddir)/gal/widgets/libwidgets.la \
	$(top_builddir)/gal/unicode/libunicode.la \
	$(top_builddir)/gal/util/libutil.la
