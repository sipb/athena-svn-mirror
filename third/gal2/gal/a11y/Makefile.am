SUBDIRS = e-table e-text

INCLUDES =					\
	-I$(top_srcdir)                         \
	-I$(top_srcdir)/gal                     \
	$(EXTRA_GNOME_CFLAGS)			\
	$(GNOME_BONOBO_CFLAGS)			\
	$(GNOME_INCLUDEDIR)			\
	$(ICONV_CFLAGS)				\
        -DG_LOG_DOMAIN=\"gal-a11y\"

module_LTLIBRARIES = libgal-a11y-2.0.la

moduledir = $(libdir)/gtk-2.0/modules

libgal_a11y_2_0_la_LDFLAGS =    \
	-rpath $(moduledir) -module -avoid-version -no-undefined \
        @LDFLAGS@

libgal_a11y_2_0_la_SOURCES =	\
	gal-a11y-util.c		\
	init.c

libgal_a11y_2_0includedir = $(includedir)/gal-$(GAL_API_VERSION)/gal/a11y

libgal_a11y_2_0include_HEADERS =		\
	gal-a11y-util.h


libgal_a11y_2_0_la_LIBADD =		\
	e-table/libgal-a11y-etable.la	\
	e-text/libgal-a11y-etext.la	\
	$(top_builddir)/gal/libgal-2.0.la	\
	$(EXTRA_GNOME_LIBS)
