SUBDIRS=20x20 48x48 24x24 72x72 192x192 36x36 12x12 96x96 
THEME_IN_FILES=index.theme.in
 
%.theme:   %.theme.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

icondir = $(datadir)/icons/Crux

icon_DATA = index.theme
 
EXTRA_DIST=index.theme.in index.theme
