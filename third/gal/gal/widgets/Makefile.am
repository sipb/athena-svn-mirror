noinst_LTLIBRARIES = libwidgets.la

# WARNING
# WARNING Be sure to add any files here into the parent Makefile too.
# WARNING
libwidgets_la_SOURCES = 			\
	color-group.c				\
	color-palette.c				\
	e-canvas-utils.c			\
	e-canvas-vbox.c				\
	e-canvas.c				\
	e-categories-master-list-array.c	\
	e-categories-master-list-combo.c	\
	e-categories-master-list-dialog-model.c	\
	e-categories-master-list-dialog.c	\
	e-categories-master-list.c		\
	e-categories.c				\
	e-colors.c				\
	e-cursors.c				\
	e-font.c				\
	e-gui-utils.c				\
	e-hscrollbar.c				\
	e-hsv-utils.c				\
	e-popup-menu.c				\
	e-printable.c				\
	e-reflow-model.c			\
	e-reflow.c				\
	e-scroll-frame.c			\
	e-selection-model-array.c		\
	e-selection-model-simple.c		\
	e-selection-model.c			\
	e-unicode.c				\
	e-vscrollbar.c				\
	gtk-combo-box.c				\
	gtk-combo-stack.c			\
	gtk-combo-text.c			\
	widget-color-combo.c			\
	widget-pixmap-combo.c

libwidgets_la_LDFLAGS = 			\
	$(ICONV_LIBS)
gladedir = $(datadir)/gal/$(VERSION)/glade
glade_DATA = 					\
	gal-categories.glade			\
	e-categories-master-list-dialog.glade

imagesdir = $(datadir)/pixmaps/gal/categories

libwidgetsincludedir = $(includedir)/gal/widgets

libwidgetsinclude_HEADERS = 			\
	color-group.h				\
	color-palette.h				\
	e-canvas-utils.h			\
	e-canvas-vbox.h				\
	e-canvas.h				\
	e-categories-master-list-array.h	\
	e-categories-master-list-combo.h	\
	e-categories-master-list-dialog-model.h	\
	e-categories-master-list-dialog.h	\
	e-categories-master-list.h		\
	e-categories.h				\
	e-colors.h				\
	e-cursors.h				\
	e-font.h				\
	e-gui-utils.h				\
	e-hscrollbar.h				\
	e-hsv-utils.h				\
	e-popup-menu.h				\
	e-printable.h				\
	e-reflow-model.h			\
	e-reflow.h				\
	e-scroll-frame.h			\
	e-selection-model-array.h		\
	e-selection-model-simple.h		\
	e-selection-model.h			\
	e-unicode.h				\
	e-vscrollbar.h				\
	gtk-combo-box.h				\
	gtk-combo-stack.h			\
	gtk-combo-text.h			\
	widget-color-combo.h			\
	widget-pixmap-combo.h

INCLUDES = 				\
	-I$(top_srcdir) 		\
	$(EXTRA_GNOME_CFLAGS) 		\
	$(ICONV_CFLAGS)			\
	-DGAL_GLADEDIR=\"$(gladedir)\"	\
	-DGAL_IMAGESDIR=\"$(imagesdir)\"

noinst_PROGRAMS = test-color test-e-font test-font-loading

test_color_SOURCES = \
	test-color.c

test_color_LDADD = \
	$(EXTRA_GNOME_LIBS) \
	$(top_builddir)/gal/widgets/libwidgets.la \
	$(top_builddir)/gal/util/libutil.la

test_e_font_SOURCES = \
	test-e-font.c test-e-font.h

test_e_font_LDADD = \
	$(EXTRA_GNOME_LIBS) \
	$(ICONV_LIBS)	\
	$(top_builddir)/gal/widgets/libwidgets.la \
	$(top_builddir)/gal/unicode/libunicode.la \
	$(top_builddir)/gal/util/libutil.la

test_font_loading_SOURCES = \
	test-font-loading.c

test_font_loading_LDADD = \
	$(EXTRA_GNOME_LIBS) \
	$(ICONV_LIBS)	\
	$(top_builddir)/gal/widgets/libwidgets.la \
	$(top_builddir)/gal/unicode/libunicode.la \
	$(top_builddir)/gal/util/libutil.la

dist-hook:
	-mkdir $(distdir)/pixmaps
	cp $(srcdir)/pixmaps/*.xpm $(distdir)/pixmaps

EXTRA_DIST = \
	$(glade_DATA)
