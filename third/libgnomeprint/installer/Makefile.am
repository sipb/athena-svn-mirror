bin_PROGRAMS = libgnomeprint-2.0-font-install

INCLUDES = \
	-I$(top_srcdir) \
	$(LIBGNOMEPRINT_CFLAGS) \
	-DDATADIR=\""$(datadir)"\" \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DLIBDIR=\""$(libdir)"\" \
	-DFONTMAPDIR_STATIC=\"$(FONTMAPDIR_STATIC)\" \
	-DFONTMAPDIR_DYNAMIC=\"$(FONTMAPDIR_DYNAMIC)\"

libgnomeprint_2_0_font_install_SOURCES = \
	gnome-font-install.c \
	gf-pfb.c gf-pfb.h \
	gf-ttf.c gf-ttf.h \
	gf-fontmap.c gf-fontmap.h \
	parseAFM.c parseAFM.h

libgnomeprint_2_0_font_install_LDADD = \
	$(LIBGNOMEPRINT_LIBS) \
	$(INTLLIBS) -lpopt

EXTRA_DIST = README.installer

readmedir = $(datadir)/fonts
readme_DATA = README.installer

if INSTALL_FONTS
INSTALL_COMMAND = ./libgnomeprint-2.0-font-install \
		--debug \
		--smart \
		--refresh \
		--static \
		--aliases=$(FONTMAPDIR_STATIC)/adobe-urw.font \
		--aliases=$(top_srcdir)/fonts/adobe-urw.font \
		--target=$(FONTMAPDIR_STATIC)/gnome-print.fontmap
else
INSTALL_COMMAND =
endif

install-data-local: libgnomeprint-2.0-font-install
	$(INSTALL_COMMAND)
