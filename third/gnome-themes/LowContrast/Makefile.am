
@INTLTOOL_THEME_RULE@


themedir = $(datadir)/themes/LowContrast
theme_in_files = index.theme.in
theme_DATA = $(theme_in_files:.theme.in=.theme)

SUBDIRS= gtk-2.0 icons

EXTRA_DIST= index.theme index.theme.in
DISTCLEANFILES= index.theme

if CORE_THEMES_ONLY
install: install-recursive
	mv $(datadir)/themes/LowContrast/index.theme $(datadir)/themes/LowContrast/index.theme~
endif
