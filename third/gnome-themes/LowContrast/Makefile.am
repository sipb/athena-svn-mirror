
@INTLTOOL_THEME_RULE@


themedir = $(datadir)/themes/LowContrast
theme_in_files = index.theme.in
theme_DATA = $(theme_in_files:.theme.in=.theme)

SUBDIRS= gtk-2.0 icons

EXTRA_DIST= index.theme index.theme.in
DISTCLEANFILES= index.theme

if CORE_THEMES_ONLY
install: install-recursive
	rm -f $(DESTDIR)$(themedir)/index.theme~
	mv $(DESTDIR)$(themedir)/index.theme $(DESTDIR)$(themedir)/index.theme.disabled
uninstall: uninstall-recursive
	rm -f $(DESTDIR)$(themedir)/index.theme.disabled
endif
