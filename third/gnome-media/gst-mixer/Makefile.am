Desktopdir = $(datadir)/applications
SUBDIRS = pixmaps src doc

@INTLTOOL_DESKTOP_RULE@
Desktop_in_files = gnome-volume-control.desktop.in
Desktop_DATA = $(Desktop_in_files:.desktop.in=.desktop)

EXTRA_DIST = $(Desktop_in_files)
CLEANFILES = $(Desktop_DATA)
