entry_figs =				\
	figures/entry-blank.png		\
	figures/entry-bool.png		\
	figures/entry-list.png		\
	figures/entry-number.png	\
	figures/entry-schema.png	\
	figures/entry-string.png

all: $(entry_figs)
$(entry_figs) : figures/% : ../../data/icons/%
	cp $< $@

figs = figures/gconf_editor_anno_window.png $(entry_figs)
figdir = figures
docname = gconf-editor
lang = C
omffile = gconf-editor-C.omf
entities = legal.xml
include $(top_srcdir)/xmldocs.make
dist-hook: app-dist-hook

