INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../libcapplet		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DGNOMESYSCONFDIR=\""$(sysconfdir)"\"		\
	$(XML_CFLAGS)

bin_PROGRAMS = gnome-edit-properties-capplet

gnome_edit_properties_capplet_SOURCES = gnome-edit-properties.c

gnome_edit_properties_capplet_LDADD = ../../libcapplet/libcapplet.la \
		     $(GNOME_LIBS) $(ORBIT_LIBS) \
		     $(GNOMEUI_LIBS) $(GNOME_LIBDIR) $(INTLLIBS) \
		     $(GNOME_XML_LIB)


EXTRA_DIST = \
	gnome-edit-properties.desktop

sysdir = $(datadir)/control-center/
sys_DATA = gnome-edit-properties.desktop

install-data-local:
	$(INSTALL_DATA) $(srcdir)/gnome-edit-properties.desktop $(DESTDIR)$(datadir)/gnome/apps/Settings/gnome-edit-properties.desktop
