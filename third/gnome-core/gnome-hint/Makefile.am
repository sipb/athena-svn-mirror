INCLUDES = 			\
	-I$(includedir)		\
	-I$(top_srcdir)/intl	\
	-I$(top_builddir)/intl	\
	$(GNOME_INCLUDEDIR)	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale\""

LDADD = \
	$(GNOME_LIBDIR)					\
	$(GNOMEUI_LIBS)					\
	$(GNOME_XML_LIB)				\
	$(INTLLIBS)

bin_PROGRAMS = gnome-hint gnome-hint-properties-capplet

gnome_hint_SOURCES = 		\
	gnome-hint.c

gnome_hint_properties_capplet_SOURCES = gnome-hint-properties.c

gnome_hint_properties_capplet_LDADD = \
	$(GNOME_LIBDIR)		\
	$(GNOME_CAPPLET_LIBS)	\
	$(INTLLIBS)

utildir = $(datadir)/gnome/apps/Utilities
util_DATA = gnome-hint.desktop

EXTRA_DIST = \
	gnome-hint-properties.desktop \
	gnome-hint.png \
	gnome-hint.desktop

sysdir = $(datadir)/control-center/Session/
sys_DATA = gnome-hint-properties.desktop

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = gnome-hint.png

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datadir)/gnome/apps/Settings/Session/
	$(INSTALL_DATA) $(srcdir)/gnome-hint-properties.desktop $(DESTDIR)$(datadir)/gnome/apps/Settings/Session/gnome-hint-properties.desktop

