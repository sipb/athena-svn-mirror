INCLUDES =					\
	-I$(top_srcdir)                         \
	-I$(top_srcdir)/gal                     \
	-I$(top_srcdir)/gal/a11y		\
	$(EXTRA_GNOME_CFLAGS)			\
	$(GNOME_BONOBO_CFLAGS)			\
	$(GNOME_INCLUDEDIR)			\
	$(ICONV_CFLAGS)				\
        -DG_LOG_DOMAIN=\"e-text\"

noinst_LTLIBRARIES = libgal-a11y-etext.la

libgal_a11y_etext_la_SOURCES =			\
	gal-a11y-e-text-factory.c		\
	gal-a11y-e-text.c

libgal_a11y_etextincludedir = $(includedir)/gal-$(GAL_API_VERSION)/gal/a11y/e-text

libgal_a11y_etextinclude_HEADERS =		\
	gal-a11y-e-text-factory.h		\
	gal-a11y-e-text.h
