
bin_PROGRAMS = xalf-capplet

xalf_capplet_SOURCES = xalf-capplet.c

CFLAGS = $(GNOME_CC_CFLAGS) -Wall

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
	$(INSTALL_DATA) $(srcdir)/hourglass-big.png $(datadir)/pixmaps/hourglass-big.png
	$(INSTALL_DATA) $(srcdir)/hourglass-small.png $(datadir)/pixmaps/hourglass-small.png	
	$(mkinstalldirs) $(Xalfdir)
	$(INSTALL_DATA) $(srcdir)/Xalf.directory $(Xalfdir)/.directory
	$(INSTALL_DATA) $(srcdir)/settings.desktop $(Xalfdir)/settings.desktop 
	$(mkinstalldirs) $(datadir)/gnome/apps/Settings/Xalf
	$(INSTALL_DATA) $(srcdir)/settings.desktop $(datadir)/gnome/apps/Settings/Xalf/settings.desktop 










