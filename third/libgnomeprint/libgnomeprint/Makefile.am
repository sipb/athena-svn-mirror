SUBDIRS = gpa drivers transports

temp_sources = \
	art_rgb_affine_private.h \
	art_rgba_rgba_affine.c \
	art_rgba_rgba_affine.h \
	art_rgba_svp.c \
	art_rgba_svp.h

font_sources = \
	gp-fontmap.c gp-fontmap.h \
	gnome-font.c \
	gnome-font-face.c \
	gnome-rfont.c \
	gnome-font-family.c gnome-font-family.h \
	gp-truetype-utils.c gp-truetype-utils.h \
	gp-tt-t1.c gp-tt-t1.h

font_headers = \
	gnome-font.h \
	gnome-font-face.h \
	gnome-rfont.h

font_private_headers = \
	gnome-font-private.h 

nox_sources = \
	gnome-print-private.h		\
	gnome-print.c 			\
	gnome-print-bonobo.h   gnome-print-bonobo-client.h \
	gnome-print-stdapi.c \
	gnome-print-config.c \
	gnome-print-meta.c 	    gnome-print-meta.h \
	gnome-print-master.c    gnome-print-master-private.h \
	gnome-print-multipage.c gnome-print-multipage.h \
	gnome-print-ps2.c       gnome-print-ps2.h \
	gnome-print-rbuf.c      gnome-print-rbuf.h \
	gnome-print-rbuf-private.h	\
	gnome-print-frgba.c     gnome-print-frgba.h \
	gnome-print-rgbp.c      gnome-print-rgbp.h \
	gnome-print-transport.c gnome-print-transport.h \
	gnome-print-i18n.c      gnome-print-i18n.h \
	gp-path.c			       	\
	gp-path.h			       	\
	gp-gc.c                       \
	gnome-glyphlist.c		  	\
	gnome-glyphlist-private.h  		\
	gnome-pgl.c gnome-pgl-private.h \
	gnome-print-encode.c gnome-print-encode.h \
	gnome-print-encode-private.h		\
	libgnomeprint-marshal-main.c		\
	gnome-print-bonobo-client.c		\
	gnome-print-bonobo.c \
	gnome-print-unit.c \
	gnome-print-paper.c

nox_headers = \
	gnome-print-config.h \
	gnome-print.h \
	gp-gc.h \
	gnome-print-master.h \
	gnome-glyphlist.h \
	gnome-pgl.h \
	gnome-print-paper.h \
	gnome-print-unit.h

nox_private_headers = \
	gnome-print-private.h \
	gp-gc-private.h

lib_LTLIBRARIES = libgnomeprint-2.la

AM_CPPFLAGS = "-DGNOME_PRINT_LIBDIR=\"$(libdir)/gnome-print-2.0\""

#LDFLAGS = -version-info $(GNOMEPRINT_CURRENT):$(GNOMEPRINT_REVISION):$(GNOMEPRINT_AGE)

INCLUDES = \
	-I$(top_srcdir) \
	$(LIBGNOMEPRINT_CFLAGS) \
	-DG_LOG_DOMAIN=\"GnomePrint\" \
	-DWE_ARE_LIBGNOMEPRINT_UI \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DBINDIR=\""$(bindir)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DFONTMAPDIR_STATIC=\"$(FONTMAPDIR_STATIC)\" \
	-DFONTMAPDIR_DYNAMIC=\"$(FONTMAPDIR_DYNAMIC)\"

libgnomeprint_2_la_LIBADD = \
	gpa/libgpa.la \
	$(LIBGNOMEPRINT_LIBS)

libgnomeprint_2_la_SOURCES = \
	$(nox_sources) \
	$(font_sources) \
	$(temp_sources)

$(libgnomeprint_2_la_OBJECTS): $(marshal_sources)

libgnomeprintincdir = $(includedir)/libgnomeprint-2.0/libgnomeprint
libgnomeprintinc_HEADERS = \
	$(nox_headers) \
	$(font_headers)

libgnomeprintprivateincdir = $(libgnomeprintincdir)/private
libgnomeprintprivateinc_HEADERS = \
	$(nox_private_headers) \
	$(font_private_headers)

EXTRA_DIST = libgnomeprint-2.0.pc.in libgnomeprint-marshal.list

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgnomeprint-2.0.pc

marshal_sources = \
	libgnomeprint-marshal.c		\
	libgnomeprint-marshal.h

libgnomeprint-marshal.h: libgnomeprint-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=libgnomeprint_marshal > $@
libgnomeprint-marshal.c: libgnomeprint-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --body --prefix=libgnomeprint_marshal > $@

CLEANFILES = $(marshal_sources)
DONT_DIST_SOURCE = $(marshal_sources)

dist-hook:
	for file in $(DONT_DIST_SOURCE) ; do \
	    rm -f $(distdir)/$$file ; \
	done
