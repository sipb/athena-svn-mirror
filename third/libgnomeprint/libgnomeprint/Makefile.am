SUBDIRS = gpa ttsubset . transports modules

temp_sources = \
	art_rgb_affine_private.h \
	art_rgba_rgba_affine.c \
	art_rgba_rgba_affine.h \
	art_rgba_svp.c \
	art_rgba_svp.h

font_sources = \
	gnome-fontmap.c \
	gnome-font.c \
	gnome-font-face.c \
	gnome-rfont.c \
	gnome-font-family.c gnome-font-family.h

font_headers = \
	gnome-font.h \
	gnome-font-face.h \
	gnome-rfont.h

font_private_headers = \
	gnome-font-private.h \
	gnome-fontmap.h

nox_sources = \
	gnome-print-private.h	\
	gnome-print.c 			\
	gnome-print-stdapi.c \
	gnome-print-config.c    gnome-print-config.h gnome-print-config-private.h \
	gnome-print-meta.c      gnome-print-meta.h \
	gnome-print-job.c       gnome-print-job-private.h \
	gnome-print-multipage.c gnome-print-multipage.h \
	gnome-print-ps2.c       gnome-print-ps2.h \
	gnome-print-pdf.c       gnome-print-pdf.h 	gnome-print-pdf-private.h \
	gnome-print-pdf-t1.c	\
	gnome-print-pdf-tt.c	\
	gnome-print-pango.c	\
	\
	gnome-print-module.h \
	\
	gnome-print-rbuf.c      gnome-print-rbuf.h \
	gnome-print-frgba.c     gnome-print-frgba.h \
	gnome-print-rgbp.c      gnome-print-rgbp.h \
	gnome-print-transport.c gnome-print-transport.h \
	gnome-print-i18n.c      gnome-print-i18n.h \
	gp-path.c			       	\
	gp-path.h			       	\
	gp-gc.c                       \
	gnome-glyphlist.c		  	\
	gnome-pgl.c gnome-pgl-private.h \
	gnome-print-encode.c gnome-print-encode.h \
	libgnomeprint-enum-types.c		\
	gnome-print-unit.c \
	gnome-print-paper.c

nox_headers = \
	gnome-print-config.h \
	gnome-print.h \
	gp-gc.h \
	gnome-print-job.h \
	gnome-print-pango.h	\
	gnome-glyphlist.h \
	gnome-pgl.h \
	gnome-print-paper.h \
	gnome-print-unit.h

nox_private_headers = \
	gnome-glyphlist-private.h \
	gnome-print-private.h \
	gnome-print-config-private.h \
	gp-gc-private.h

lib_LTLIBRARIES = libgnomeprint-2-2.la

AM_LDFLAGS = -version-info $(GNOMEPRINT_CURRENT):$(GNOMEPRINT_REVISION):$(GNOMEPRINT_AGE)

INCLUDES = \
	-I$(top_srcdir) \
	$(LIBGNOMEPRINT_CFLAGS) \
	-DG_LOG_DOMAIN=\"GnomePrint\" \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DBINDIR=\""$(bindir)"\" \
	-DDATADIR=\""$(datadir)"\" \
        -DGNOME_PRINT_MODULES_DIR=\""$(GNOME_PRINT_MODULES_DIR)"\"

marshal_sources = \
	libgnomeprint-marshal.c		\
	libgnomeprint-marshal.h

enum_sources = \
	libgnomeprint-enum-types.c	\
	libgnomeprint-enum-types.h

$(libgnomeprint_2_2_la_OBJECTS): $(marshal_sources) $(enum_sources)

libgnomeprint_2_2_la_LIBADD = \
	gpa/libgpa.la \
	ttsubset/libttsubset.la \
	$(LIBGNOMEPRINT_LIBS)

libgnomeprint_2_2_la_LDFLAGS = \
	-no-undefined

libgnomeprint_2_2_la_SOURCES = \
	$(nox_sources) \
	$(font_sources) \
	$(temp_sources)

libgnomeprintincdir = $(includedir)/libgnomeprint-2.2/libgnomeprint
libgnomeprintinc_HEADERS = \
	$(nox_headers) \
	$(font_headers) \
	libgnomeprint-enum-types.h

libgnomeprintprivateincdir = $(libgnomeprintincdir)/private
libgnomeprintprivateinc_HEADERS = \
	$(nox_private_headers) \
	$(font_private_headers)

EXTRA_DIST = \
	libgnomeprint-2.2.pc.in \
	libgnomeprint-2.2-uninstalled.pc.in \
	libgnomeprint-marshal.list \
	gnome-print-svg.c	\
	gnome-print-svg.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgnomeprint-2.2.pc

CLEANFILES = \
	$(marshal_sources) \
	$(enum_sources) \
	s-enum-types-h s-enum-types-c

DONT_DIST_SOURCE = $(marshal_sources) $(enum_sources)

libgnomeprint-marshal.h: libgnomeprint-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=libgnomeprint_marshal > $@

libgnomeprint-marshal.c: libgnomeprint-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --body --prefix=libgnomeprint_marshal > $@

dist-hook:
	for file in $(DONT_DIST_SOURCE) ; do \
	    rm -f $(distdir)/$$file ; \
	done


libgnomeprint-enum-types.h: s-enum-types-h
	@true

s-enum-types-h: $(nox_headers) Makefile
	( cd $(srcdir) && glib-mkenums \
			--fhead "#ifndef __LIBGNOMEPRINT_ENUM_TYPES_H__\n#define __LIBGNOMEPRINT_ENUM_TYPES_H__\n\n#include <glib-object.h>\n\nG_BEGIN_DECLS\n" \
			--fprod "/* enumerations from \"@filename@\" */\n" \
			--vhead "GType @enum_name@_get_type (void);\n#define GNOME_TYPE_PRINT_@ENUMSHORT@ (@enum_name@_get_type())\n" 	\
			--ftail "G_END_DECLS\n\n#endif /* __LIBGNOMEPRINT_ENUM_TYPES_H__ */" \
		$(nox_headers) ) > tmp-libgnomeprint-enum-types.h \
	&& (cmp -s tmp-libgnomeprint-enum-types.h libgnomeprint-enum-types.h || cp tmp-libgnomeprint-enum-types.h libgnomeprint-enum-types.h ) \
	&& rm -f tmp-libgnomeprint-enum-types.h	\
	&& echo timestamp > $(@F)

libgnomeprint-enum-types.c: s-enum-types-c
	@true

s-enum-types-c: $(nox_headers) Makefile
	( cd $(srcdir) && glib-mkenums \
			--fhead "#include \"libgnomeprint-enum-types.h\"\n#include \"gnome-print.h\"" \
			--fprod "\n/* enumerations from \"@filename@\" */" \
			--vhead "GType\n@enum_name@_get_type (void)\n{\n  static GType etype = 0;\n  if (etype == 0) {\n    static const G@Type@Value values[] = {" 	\
			--vprod "      { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
			--vtail "      { 0, NULL, NULL }\n    };\n    etype = g_@type@_register_static (\"@EnumName@\", values);\n  }\n  return etype;\n}\n" \
		$(nox_headers) ) > tmp-libgnomeprint-enum-types.c \
	&& (cmp -s tmp-libgnomeprint-enum-types.c libgnomeprint-enum-types.c || cp tmp-libgnomeprint-enum-types.c libgnomeprint-enum-types.c ) \
	&& rm -f tmp-libgnomeprint-enum-types.c \
	&& echo timestamp > $(@F)
