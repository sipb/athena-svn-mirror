if LIBGPA
LIBGPA_TEST_CFLAGS = -I$(top_srcdir)/libgpa
LIBGPA_TEST_LIBS = $(top_builddir)/libgpa/libgpa.la
else
LIBGPA_TEST_CFLAGS = 
LIBGPA_TEST_LIBS =
endif

CPPFLAGS = @CPPFLAGS@ \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" 	\
	-DBINDIR=\""$(bindir)"\"	\
	-DDATADIR=\""$(datadir)"\" \
	-DFONTMAPDIR_STATIC=\""$(datadir)/gnome/fonts"\" \
	-DFONTMAPDIR_DYNAMIC=\""$(sysconfdir)/gnome/fonts"\"

INCLUDES = \
	$(PRINT_INCLUDEDIR) \
	-I$(top_srcdir) 	\
	-I$(top_builddir)	\
	$(LIBGPA_TEST_CFLAGS)			

bin_PROGRAMS = gnome-font-install

gnome_font_install_SOURCES = \
	gnome-font-install.c \
	gf-pfb.c gf-pfb.h \
	gf-fontmap.c gf-fontmap.h \
	parseAFM.c parseAFM.h

gnome_font_install_LDADD = \
	$(PRINT_TEST_LIBS) \
	$(LIBGPA_TEST_LIBS) \
	$(INTLLIBS)

EXTRA_DIST = README.installer README.binaries README

readmedir = $(datadir)/fonts
readme_DATA = README.installer README.binaries

install-data-local: gnome-font-install
	$(PERL) $(top_srcdir)/run-gnome-font-install ./gnome-font-install $(datadir) $(top_srcdir) $(sysconfdir)
