NULL=

INCLUDES = \
	-I$(top_srcdir) \
	$(EEL_CFLAGS) \
	-DVERSION="\"$(VERSION)\"" \
	-DGNOMELOCALEDIR=\""$(prefix)/${DATADIRNAME}/locale"\"	\
	-DDATADIR=\""$(datadir)"\" \
	-DG_DISABLE_DEPRECATED \
	-DGDK_DISABLE_DEPRECATED \
	$(NULL)

# Disable this for now due to gtk 2.4 deprecation
#	-DGTK_DISABLE_DEPRECATED \
#	-DGNOME_DISABLE_DEPRECATED

LDADD = \
	$(top_builddir)/eel/libeel-2.la \
	$(EEL_LIBS) \
	$(NULL)

noinst_PROGRAMS = \
	test-eel-background \
	test-eel-ellipsizing	\
	test-eel-editable-label	\
	test-eel-gtk-style \
	test-eel-image-scrolled \
	test-eel-image-table \
	test-eel-labeled-image \
	test-eel-pixbuf-tile \
	test-eel-pixbuf-scale \
	$(NULL)

test_eel_background_SOURCES = test-eel-background.c
test_eel_ellipsizing_SOURCES = test-eel-ellipsizing.c
test_eel_gtk_style_SOURCES = test-eel-gtk-style.c test.c
test_eel_image_scrolled_SOURCES = test-eel-image-scrolled.c test.c test.h
test_eel_image_table_SOURCES = test-eel-image-table.c test.c
test_eel_labeled_image_SOURCES = test-eel-labeled-image.c test.c test.h
test_eel_pixbuf_tile_SOURCES = test-eel-pixbuf-tile.c test.c test.h
test_eel_pixbuf_scale_SOURCES = test-eel-pixbuf-scale.c test.c test.h

