SUBDIRS = e-table e-text

INCLUDES =					\
	-I$(top_srcdir)                         \
	-I$(top_srcdir)/gal                     \
	$(EXTRA_GNOME_CFLAGS)			\
	$(GNOME_BONOBO_CFLAGS)			\
	$(GNOME_INCLUDEDIR)			\
	$(ICONV_CFLAGS)				\
	-DGTK_DISABLE_DEPRECATED=1		\
	-DGDK_DISABLE_DEPRECATED=1 		\
	-DG_DISABLE_DEPRECATED=1		\
	-DGNOME_DISABLE_DEPRECATED=1       	\
	-DG_LOG_DOMAIN=\"gal-a11y\"


lib_LTLIBRARIES = libgal-a11y-2.2.la

libgal_a11y_2_2_la_LDFLAGS =    \
	-version-info $(GAL_CURRENT):$(GAL_REVISION):$(GAL_AGE) 

libgal_a11y_2_2_la_SOURCES =	\
	gal-a11y-util.c

libgal_a11y_2_2includedir = $(includedir)/gal-$(GAL_API_VERSION)/gal/a11y

libgal_a11y_2_2include_HEADERS =		\
	gal-a11y-util.h			\
	gal-a11y-factory.h


libgal_a11y_2_2_la_LIBADD =		\
	e-text/libgal-a11y-etext.la	\
	e-table/libgal-a11y-etable.la	\
	$(EXTRA_GNOME_LIBS)
