INCLUDES = \
	-I$(top_srcdir) 				\
	-I$(top_builddir)				\
	$(WARN_CFLAGS)					\
	$(LIBGNOMECANVAS_CFLAGS)			\
        -DGNOMECANVASLIBDIR=\""$(libdir)"\" 		\
        -DGNOMECANVASDATADIR=\""$(datadir)"\" 		\
        -DGNOMECANVASPIXMAPDIR=\""$(datadir)/pixmaps"\"	\
        -DGNOMECANVASBINDIR=\""$(bindir)"\" 		\
        -DGNOMECANVASLOCALSTATEDIR=\""$(localstatedir)"\" 	\
        -DGNOMECANVASLOCALEDIR=\""$(gnomelocaledir)"\" 	\
	-DGTK_VERSION=\""$(GTK_VERSION)"\"		\
	-DG_LOG_DOMAIN=\"GnomeCanvas\"			\
	-DVERSION=\"$(VERSION)\"

LDADD =	\
	$(top_builddir)/libgnomecanvas/libgnomecanvas-2.la \
	$(INTLLIBS)

noinst_PROGRAMS = canvas_demo

canvas_demo_SOURCES =		\
	canvas.c		\
	canvas-primitives.c	\
	canvas-arrowhead.c	\
	canvas-features.c	\
	canvas-fifteen.c	\
	canvas-rich-text.c	\
	canvas-curve.c		\
	canvas_demo.h

EXTRA_DIST =			\
	toroid.png



