
bin_PROGRAMS = xalf-capplet

xalf_capplet_SOURCES = xalf-capplet.c

CFLAGS = $(GNOME_CC_CFLAGS)

xalf_capplet_LDFLAGS = $(GNOME_CC_LIBS)

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DGNOMESYSCONFDIR=\""$(sysconfdir)"\"

EXTRA_DIST = \
	settings.desktop Xalf.directory hourglass-big.png hourglass-small.png 

ccenterdir = $(datadir)/control-center
Xalfdir = $(ccenterdir)/Xalf

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datadir)/pixmaps
	$(INSTALL_DATA) $(srcdir)/hourglass-big.png $(DESTDIR)$(datadir)/pixmaps/hourglass-big.png
	$(INSTALL_DATA) $(srcdir)/hourglass-small.png $(DESTDIR)$(datadir)/pixmaps/hourglass-small.png	
	$(mkinstalldirs) $(DESTDIR)$(Xalfdir)
	$(INSTALL_DATA) $(srcdir)/Xalf.directory $(DESTDIR)$(Xalfdir)/.directory
	$(INSTALL_DATA) $(srcdir)/settings.desktop $(DESTDIR)$(Xalfdir)/settings.desktop 
	$(mkinstalldirs) $(DESTDIR)$(datadir)/gnome/apps/Settings/Xalf
	$(INSTALL_DATA) $(srcdir)/settings.desktop $(DESTDIR)$(datadir)/gnome/apps/Settings/Xalf/settings.desktop 










