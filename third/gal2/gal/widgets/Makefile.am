noinst_LTLIBRARIES = libwidgets.la

libwidgets_la_SOURCES = 			\
	color-group.c				\
	color-palette.c				\
	e-canvas-background.c			\
	e-canvas-utils.c			\
	e-canvas-vbox.c				\
	e-canvas.c				\
	e-categories-master-list-array.c	\
	e-categories-master-list-combo.c	\
	e-categories-master-list-dialog-model.c	\
	e-categories-master-list-dialog.c	\
	e-categories-master-list-option-menu.c	\
	e-categories-master-list.c		\
	e-categories.c				\
	e-colors.c				\
	e-cursors.c				\
	e-file-selection.c			\
	e-gui-utils.c				\
	e-hsv-utils.c				\
	e-option-menu.c				\
	e-popup-menu.c				\
	e-printable.c				\
	e-reflow-model.c			\
	e-reflow.c				\
	e-selection-model-array.c		\
	e-selection-model-simple.c		\
	e-selection-model.c			\
	e-unicode.c				\
	gal-combo-box.c				\
	gal-combo-stack.c			\
	gal-combo-text.c			\
	widget-color-combo.c			\
	widget-pixmap-combo.c

libwidgets_la_LDFLAGS = 			\
	$(ICONV_LIBS)
gladedir = $(datadir)/gal-$(GAL_API_VERSION)/$(VERSION)/glade
glade_DATA = 					\
	gal-categories.glade			\
	e-categories-master-list-dialog.glade

imagesdir = $(datadir)/gal-$(GAL_API_VERSION)/$(VERSION)/pixmaps/categories

libwidgetsincludedir = $(includedir)/gal-$(GAL_API_VERSION)/gal/widgets

libwidgetsinclude_HEADERS = 			\
	color-group.h				\
	color-palette.h				\
	e-canvas-background.h			\
	e-canvas-utils.h			\
	e-canvas-vbox.h				\
	e-canvas.h				\
	e-categories-master-list-array.h	\
	e-categories-master-list-combo.h	\
	e-categories-master-list-dialog-model.h	\
	e-categories-master-list-dialog.h	\
	e-categories-master-list-option-menu.h	\
	e-categories-master-list.h		\
	e-categories.h				\
	e-colors.h				\
	e-cursors.h				\
	e-file-selection.h			\
	e-gui-utils.h				\
	e-hsv-utils.h				\
	e-option-menu.h				\
	e-popup-menu.h				\
	e-printable.h				\
	e-reflow-model.h			\
	e-reflow.h				\
	e-selection-model-array.h		\
	e-selection-model-simple.h		\
	e-selection-model.h			\
	e-unicode.h				\
	gal-combo-box.h				\
	gal-combo-stack.h			\
	gal-combo-text.h			\
	widget-color-combo.h			\
	widget-pixmap-combo.h

INCLUDES = 				\
	-I$(top_srcdir) 		\
	$(EXTRA_GNOME_CFLAGS) 		\
	$(ICONV_CFLAGS)			\
	-DGAL_GLADEDIR=\"$(gladedir)\"	\
	-DGAL_IMAGESDIR=\"$(imagesdir)\"

noinst_PROGRAMS = test-color

test_color_SOURCES = \
	test-color.c

test_color_LDADD = \
	libwidgets.la \
	$(top_builddir)/gal/util/libutil.la \
	$(EXTRA_GNOME_LIBS)

dist-hook:
	-mkdir $(distdir)/pixmaps
	cp $(srcdir)/pixmaps/*.xpm $(distdir)/pixmaps

EXTRA_DIST = \
	$(glade_DATA)
