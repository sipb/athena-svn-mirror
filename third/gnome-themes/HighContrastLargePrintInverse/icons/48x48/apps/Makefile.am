NULL=

icondir=	$(datadir)/icons/HighContrastLargePrintInverse/48x48/apps/

icon_DATA=	\
                accessibility-directory.png \
                advanced-directory.png \
		ax-applet.png \
                charpick.png \
		display-capplet.png \
		evolution-1.4.png \
		file-manager.png \
                file-roller.png \
                gcalctool.png \
		gdict.png \
                gedit-icon.png \
                gkb.png \
		gnome-application-generic.png \
		gnome-folder-generic.png \
                gnome-applications.png \
                gnome-audio2.png \
                gnome-calc3.png \
                gnome-ccmime.png \
                gnome-ccscreensaver.png \
                gnome-cdplayer-icon.png \
                gnome-cd.png \
                gnome-character-map.png \
                gnome-clock.png \
                gnome-devel.png \
                gnome-eog.png \
                gnome-ghex.png  \
                gnome-ghostview.png \
                gnome-globe.png \
                gnome-graphics.png \
                gnome-grecord.png \
                gnome-help.png  \
                gnome-home.png \
                gnome-joystick.png \
                gnome-log.png \
		gnome-logo-icon-transparent.png \
                gnome-mailcheck.png \
		gnome-main-menu.png \
                gnome-mini-commander.png \
		gnome-mixer.png \
                gnome-mixer-applet.png \
                gnome-mixer.png \
                gnome-multimedia.png \
                gnome-panel.png \
                gnome-session.png \
                gnome-settings-accessibility-keyboard.png \
                gnome-settings-accessibility-technologies.png \
                gnome-settings-background.png \
                gnome-settings-default-applications.png \
                gnome-settings-font.png \
                gnome-settings-keybindings.png \
                gnome-settings.png \
                gnome-settings-sound.png \
                gnome-settings-theme.png \
		gnome-settings-ui-behavior.png \
                gnome-starthere.png \
                gnome-system.png \
                gnome-terminal.png \
                gnome-util.png \
                gnome-window-manager.png \
                gnome-windows.png \
                gnome-workspace.png \
                gucharmap.png \
		keyboard.png \
                launcher-program.png \
		mozilla-icon.png \
                perfmeter.png \
		star.png \
                window-capplet.png \
		$(NULL)

EXTRA_DIST=	$(icon_DATA) \
		symlinks.dat

install: install-am
	$(SHELL) $(top_srcdir)/common/mkiconlinks.sh $(srcdir)/symlinks.dat $(DESTDIR)$(icondir)
 
uninstall-symlinks:
	list=`ls $(DESTDIR)$(icondir)`; for p in $$list; do \
		if test -h $(DESTDIR)$(icondir)/$$p; then \
			rm -f $(DESTDIR)$(icondir)/$$p; \
		fi; \
	done
uninstall-am: uninstall-iconDATA uninstall-symlinks
