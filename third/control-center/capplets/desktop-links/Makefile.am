ccenterdir = $(datadir)/control-center
Multimediadir	= $(ccenterdir)/Multimedia
Desktopdir	= $(ccenterdir)/Desktop
UIOptionsdir    = $(ccenterdir)/UIOptions
Peripheralsdir  = $(ccenterdir)/Peripherals

paneldir        = $(datadir)/gnome/apps/Settings
Multimediadirp	= $(paneldir)/Multimedia
Desktopdirp	= $(paneldir)/Desktop
UIOptionsdirp   = $(paneldir)/UIOptions
Peripheralsdirp = $(paneldir)/Peripherals
Directory_files = \
	Gnome.directory		\
	Desktop.directory	\
	Peripherals.directory   \
	Multimedia.directory	\
	UIOptions.directory

EXTRA_DIST = \
	Gnome.directory		\
	Desktop.directory	\
	Peripherals.directory   \
	Multimedia.directory	\
	UIOptions.directory

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(Desktopdir)
	$(mkinstalldirs) $(DESTDIR)$(Multimediadir)
	$(mkinstalldirs) $(DESTDIR)$(UIOptionsdir)
	$(mkinstalldirs) $(DESTDIR)$(Peripheralsdir)
	$(INSTALL_DATA) $(srcdir)/Gnome.directory $(DESTDIR)$(ccenterdir)/.directory
	$(INSTALL_DATA) $(srcdir)/Multimedia.directory $(DESTDIR)$(Multimediadir)/.directory
	$(INSTALL_DATA) $(srcdir)/Desktop.directory $(DESTDIR)$(Desktopdir)/.directory
	$(INSTALL_DATA) $(srcdir)/UIOptions.directory $(DESTDIR)$(UIOptionsdir)/.directory
	$(INSTALL_DATA) $(srcdir)/Peripherals.directory $(DESTDIR)$(Peripheralsdir)/.directory
	$(mkinstalldirs) $(DESTDIR)$(Desktopdirp)
	$(mkinstalldirs) $(DESTDIR)$(Multimediadirp)
	$(mkinstalldirs) $(DESTDIR)$(UIOptionsdirp)
	$(mkinstalldirs) $(DESTDIR)$(Peripheralsdirp)
	$(INSTALL_DATA) $(srcdir)/Multimedia.directory $(DESTDIR)$(Multimediadirp)/.directory
	$(INSTALL_DATA) $(srcdir)/Desktop.directory $(DESTDIR)$(Desktopdirp)/.directory
	$(INSTALL_DATA) $(srcdir)/UIOptions.directory $(DESTDIR)$(UIOptionsdirp)/.directory
	$(INSTALL_DATA) $(srcdir)/Peripherals.directory $(DESTDIR)$(Peripheralsdirp)/.directory
