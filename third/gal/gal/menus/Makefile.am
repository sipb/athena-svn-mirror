gladedir = $(datadir)/gal/$(VERSION)/glade
glade_DATA = 					\
	gal-define-views.glade			\
	gal-view-new-dialog.glade

INCLUDES =					\
	-I$(top_srcdir)                         \
	-I$(top_srcdir)/gal                     \
	$(EXTRA_GNOME_CFLAGS)			\
	$(GNOME_INCLUDEDIR)			\
	$(ICONV_CFLAGS)				\
	-DGAL_GLADEDIR=\"$(gladedir)\"		\
        -DG_LOG_DOMAIN=\"gal\"

noinst_LTLIBRARIES = libgalmenus.la

# WARNING
# WARNING Be sure to add any files here into the parent Makefile too.
# WARNING
libgalmenus_la_SOURCES = 			\
	gal-define-views-dialog.c		\
	gal-define-views-model.c		\
	gal-view-collection.c			\
	gal-view-etable.c			\
	gal-view-factory-etable.c		\
	gal-view-factory.c			\
	gal-view-new-dialog.c			\
	gal-view.c

libgalmenusincludedir = $(includedir)/gal/menus

libgalmenusinclude_HEADERS = 			\
	gal-define-views-dialog.h		\
	gal-define-views-model.h		\
	gal-view-collection.h			\
	gal-view-etable.h			\
	gal-view-factory-etable.h		\
	gal-view-factory.h			\
	gal-view-new-dialog.h			\
	gal-view.h

icons =

EXTRA_DIST =			\
	 $(icons)		\
	 $(glade_DATA)
