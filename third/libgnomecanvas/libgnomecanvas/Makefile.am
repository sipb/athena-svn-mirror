INCLUDES = \
	-I$(top_srcdir) 				\
	-I$(top_builddir)				\
	$(WARN_CFLAGS) 					\
	$(LIBGNOMECANVAS_CFLAGS)			\
        -DGNOMECANVASLIBDIR=\""$(libdir)"\" 		\
        -DGNOMECANVASDATADIR=\""$(datadir)"\" 		\
        -DGNOMECANVASPIXMAPDIR=\""$(datadir)/pixmaps"\"	\
        -DGNOMECANVASBINDIR=\""$(bindir)"\" 		\
        -DGNOMECANVASLOCALSTATEDIR=\""$(localstatedir)"\" 	\
        -DGNOMECANVASLOCALEDIR=\""$(gnomelocaledir)"\" 	\
	-DG_LOG_DOMAIN=\"GnomeCanvas\"			\
	-DVERSION=\"$(VERSION)\"

lib_LTLIBRARIES = libgnomecanvas-2.la

libgnomecanvas_2_la_LIBADD = \
	$(LIBGNOMECANVAS_LIBS)

libgnomecanvasincludedir = $(includedir)/libgnomecanvas-2.0/libgnomecanvas

libgnomecanvasinclude_HEADERS =			\
	gnome-canvas-shape.h			\
	gnome-canvas-bpath.h			\
	gnome-canvas-clipgroup.h		\
	gnome-canvas-line.h			\
	gnome-canvas-pixbuf.h			\
	gnome-canvas-polygon.h			\
	gnome-canvas-rect-ellipse.h		\
	gnome-canvas-text.h			\
	gnome-canvas-rich-text.h		\
	gnome-canvas-util.h			\
	gnome-canvas-widget.h			\
	gnome-canvas-path-def.h			\
	gnome-canvas.h				\
	libgnomecanvas.h

libgnomecanvas_2_la_SOURCES =			\
	$(libgnomecanvasinclude_HEADERS)	\
	gnome-canvas-shape.c			\
	gnome-canvas-shape-private.h		\
	gnome-canvas-bpath.c			\
	gnome-canvas-path-def.c			\
	gnome-canvas-clipgroup.c		\
	gnome-canvas-i18n.h			\
	gnome-canvas-line.c			\
	gnome-canvas-pixbuf.c			\
	gnome-canvas-polygon.c			\
	gnome-canvas-rect-ellipse.c		\
	gnome-canvas-text.c			\
	gnome-canvas-rich-text.c		\
	gnome-canvas-util.c			\
	gnome-canvas-widget.c			\
	gnome-canvas.c				\
	libgnomecanvastypes.c

$(libgnomecanvas_2_la_OBJECTS): $(marshal_sources)

marshal_sources =				\
	gnome-canvas-marshal.c			\
	gnome-canvas-marshal.h

BUILT_SOURCES =					\
	$(marshal_sources)

gnome-canvas-marshal.h: gnome-canvas-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=gnome_canvas_marshal > $@
gnome-canvas-marshal.c: gnome-canvas-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --body --prefix=gnome_canvas_marshal > $@

EXTRA_DIST = \
	gnome-canvas-marshal.list	\
	libgnomecanvas-2.0.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgnomecanvas-2.0.pc

DONT_DIST_SOURCE = $(marshal_sources)

dist-hook:
	for file in $(DONT_DIST_SOURCE) ; do \
	    rm -f $(distdir)/$$file ; \
	done
