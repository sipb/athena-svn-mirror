# Basic setup

gladedir = $(datadir)/gtkhtml
glade_DATA = 	gtkhtml-editor-properties.glade

INCLUDES =						\
        -DGNOMEDATADIR=\""$(datadir)"\"		\
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(top_srcdir)/src				\
	-I$(srcdir)					\
	-I.						\
        $(GNOME_INCLUDEDIR)				\
	$(EXTRA_GNOME_CFLAGS)				\
	$(GTK_CFLAGS)					\
	-DGLADE_DATADIR=\"$(gladedir)\"

CFLAGS +=  $(GCONF_CFLAGS) $(PSPELL_CFLAGS) -DICONDIR=\"$(datadir)/gtkhtml/icons\"

# CORBA stuff

IDLS = \
	Editor.idl

SPELL_IDLS = \
	Spell.idl

IDL_GENERATED = \
	Editor.h		\
	Editor-common.c	\
	Editor-skels.c	\
	Editor-stubs.c

SPELL_IDL_GENERATED = \
	Spell.h			\
	Spell-common.c		\
	Spell-skels.c		\
	Spell-stubs.c

Editor-common.o: Editor.h
Spell-common.o: Spell.h

$(IDL_GENERATED): $(IDLS)
	$(ORBIT_IDL) -I$(datadir)/idl `$(GNOME_CONFIG) --cflags idl`	\
		-I$(srcdir) $(srcdir)/Editor.idl

$(SPELL_IDL_GENERATED): $(SPELL_IDLS)
	$(ORBIT_IDL) -I$(datadir)/idl `$(GNOME_CONFIG) --cflags idl`	\
		-I$(srcdir) $(srcdir)/Spell.idl

LDFLAGS += -dynamic

LDADD =	\
	$(top_builddir)/src/libgtkhtml.la	\
	$(EXTRA_GNOME_LIBS)			\
	$(GNOMEUI_LIBS)				\
	$(GNOME_LIBS)				\
	$(GCONF_LIBS)

# -lpspell-modules -lpspell_aspell -laspell -lpspell -lstdc++
#	-lccmalloc -ldl

# The Editor Control

GTKHTML_EDITOR_SOURCES_COMMON =			\
	$(IDL_GENERATED)			\
	$(SPELL_IDL_GENERATED)			\
	body.h					\
	body.c					\
	cell.h					\
	cell.c					\
	control-data.c				\
	control-data.h				\
	dialog.c				\
	dialog.h				\
	editor-control-factory.c		\
	engine.h				\
	engine.c				\
	image.c					\
	image.h					\
	link.c					\
	link.h					\
	menubar.c				\
	menubar.h				\
	paragraph.c				\
	paragraph.h				\
	persist-file-impl.c			\
	persist-file-impl.h			\
	persist-stream-impl.c			\
	persist-stream-impl.h			\
	popup.c					\
	popup.h					\
	properties.c				\
	properties.h				\
	replace.c				\
	replace.h				\
	html-stream-mem.h			\
	html-stream-mem.c			\
	resolver-progressive-impl.c		\
	resolver-progressive-impl.h		\
	rule.c					\
	rule.h					\
	search.c				\
	search.h				\
	spell.c					\
	spell.h					\
	table.c					\
	table.h					\
	template.c				\
	template.h				\
	text.c					\
	text.h					\
	toolbar.c				\
	toolbar.h				\
	utils.c					\
	utils.h

if EDITOR_SHLIB
lib_LTLIBRARIES = libgnome-gtkhtml-editor.la

libgnome_gtkhtml_editor_la_SOURCES =		\
	$(GTKHTML_EDITOR_SOURCES_COMMON)
PRGS =
else
PRGS = gnome-gtkhtml-editor
gnome_gtkhtml_editor_SOURCES =			\
	$(GTKHTML_EDITOR_SOURCES_COMMON)
endif

bin_PROGRAMS = $(PRGS)

oaf_in_in_files = GNOME_GtkHTML_Editor.oaf.in.in
oaf_in_files = GNOME_GtkHTML_Editor.oaf.in
oaf_DATA = $(oaf_in_files:.oaf.in=.oaf)

oafdir = $(datadir)/oaf

@XML_I18N_MERGE_OAF_RULE@

GNOME_GtkHTML_Editor.xml: GNOME_GtkHTML_Editor.xml.in Makefile
	sed -e 's?\@ICONSDIR\@?$(GTKHTML_DATADIR)/gtkhtml/icons?g' \
	      < $(srcdir)/GNOME_GtkHTML_Editor.xml.in > GNOME_GtkHTML_Editor.xml.tmp \
	  && mv GNOME_GtkHTML_Editor.xml.tmp GNOME_GtkHTML_Editor.xml

uidir = $(datadir)/gnome/ui
ui_DATA = GNOME_GtkHTML_Editor.xml

# The Test program

noinst_PROGRAMS =				\
	test-editor

test_editor_SOURCES =				\
	$(IDL_GENERATED)			\
	resolver.c				\
	resolver.h				\
	test-html-editor-control.c

# Extra stuff to distribute.

EXTRA_DIST =					\
	$(IDLS)					\
	$(SPELL_IDLS)				\
	$(oaf_in_files)				\
	$(oaf_DATA)				\
	ChangeLog				\
	tt.xpm					\
	$(ui_DATA)				\
	GNOME_GtkHTML_Editor.xml.in		\
	gtkhtml-editor-properties.glade

idldir = $(datadir)/gtkhtml
idl_DATA = $(IDLS)

CLEANFILES += $(IDL_GENERATED) $(SPELL_IDL_GENERATED)
BUILT_SOURCE = $(IDL_GENERATED) $(SPELL_IDL_GENERATED)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
