Utilitiesdir = $(datadir)/applications

SUBDIRS=icons

Utilities_in_files = gconf-editor.desktop.in 
Utilities_DATA = $(Utilities_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	$(Utilities_in_files) \
	$(Utilities_DATA)

