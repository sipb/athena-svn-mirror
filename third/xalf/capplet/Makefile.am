
bin_PROGRAMS = xalf-capplet

xalf_capplet_SOURCES = xalf-capplet.c getopt.c getopt1.c

CFLAGS = $(GNOME_CC_CFLAGS)

xalf_capplet_LDADD = $(GNOME_CC_LIBS) @GTK_LIBS@

INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DGNOMESYSCONFDIR=\""$(sysconfdir)"\"		\
	@GTK_CFLAGS@ 

EXTRA_DIST = \
	xalf-settings.desktop hourglass-big.png hourglass-small.png 

ccenterdir = $(datadir)/control-center
Xalfdir = $(ccenterdir)/Desktop

install-data-local:
	$(mkinstalldirs) $(datadir)/pixmaps
	$(INSTALL_DATA) $(srcdir)/hourglass-big.png $(datadir)/pixmaps/hourglass-big.png
	$(INSTALL_DATA) $(srcdir)/hourglass-small.png $(datadir)/pixmaps/hourglass-small.png	
	$(mkinstalldirs) $(Xalfdir)
	$(INSTALL_DATA) $(srcdir)/xalf-settings.desktop $(Xalfdir)/xalf-settings.desktop 
	$(mkinstalldirs) $(datadir)/gnome/apps/Settings/Desktop
	$(INSTALL_DATA) $(srcdir)/xalf-settings.desktop $(datadir)/gnome/apps/Settings/Desktop/xalf-settings.desktop 

