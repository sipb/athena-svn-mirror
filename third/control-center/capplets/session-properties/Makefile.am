INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../libcapplet		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DGNOMESYSCONFDIR=\""$(sysconfdir)"\"		\
	$(XML_CFLAGS)

# if SESSION
bin_PROGRAMS = session-properties-capplet
# endif

session_properties_capplet_SOURCES = \
	chooser.c \
	session-properties.c \
	session-properties.h \
	startup-programs.c \
	session.h \
	gsm-protocol.c \
	gsm-protocol.h \
	gsm-client-list.c \
	gsm-client-list.h \
	gsm-client-row.c \
	gsm-client-row.h \
	gsm-client-editor.c \
	gsm-client-editor.h

session_properties_capplet_LDADD = ../../libcapplet/libcapplet.la \
		     $(GNOME_LIBS) $(ORBIT_LIBS) \
		     $(GNOMEUI_LIBS) $(GNOME_LIBDIR) $(INTLLIBS) \
		     $(GNOME_XML_LIB)


EXTRA_DIST = \
	session-properties.desktop \
	gnome-session.png

sysdir = $(datadir)/control-center/Session

# if SESSION
sys_DATA = session-properties.desktop
# endif

settingsdir = $(datadir)/gnome/apps/Settings/Session

# if SESSION
settings_DATA = session-properties.desktop
# endif

pixmapsdir = $(datadir)/pixmaps

# if SESSION
pixmaps_DATA = gnome-session.png
# endif
