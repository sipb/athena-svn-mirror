NULL=

INCLUDES = -I$(top_srcdir) -I$(srcdir) -I$(includedir) $(GNOME_INCLUDEDIR) \
	$(EXTRA_GNOME_CFLAGS) \
	$(IMLIB_CFLAGS)	\
	-DG_LOG_DOMAIN=\"gtkhtml\" $(SOUP_CFLAGS) \
	-DSRCDIR=\"$(srcdir)\" \
	-DPREFIX=\"$(prefix)\"

CFLAGS +=  $(GCONF_CFLAGS) -DICONDIR=\"$(GTKHTML_DATADIR)/icons\"

lib_LTLIBRARIES = libgtkhtml-1.1.la

if SOUP
soup_programs = testgtkhtml
endif
noinst_PROGRAMS = $(soup_programs) gtest

libgtkhtmldir = $(includedir)/$(GTKHTML_RELEASE_STRING)/gtkhtml/

# FIXME not all the stuff should be installed!

libgtkhtml_HEADERS =				\
	gtkhtml-compat.h			\
	gtkhtml-embedded.h			\
	gtkhtml-propmanager.h			\
	gtkhtml-enums.h				\
	gtkhtml-im.h				\
	gtkhtml-private.h			\
	gtkhtml-properties.h			\
	gtkhtml-search.h			\
	gtkhtml-stream.h			\
	gtkhtml-types.h				\
	gtkhtml.h				\
	gtkhtmldebug.h				\
	gtkhtmlfontstyle.h			\
	htmlanchor.h				\
	htmlbutton.h				\
	htmlcheckbox.h				\
	htmlclue.h				\
	htmlcluealigned.h			\
	htmlclueflow.h				\
	htmlclueh.h				\
	htmlcluev.h				\
	htmlcolor.h				\
	htmlcolorset.h				\
	htmlcursor.h				\
	htmldrawqueue.h				\
	htmlembedded.h				\
	htmlengine-edit.h			\
	htmlengine-edit-clueflowstyle.h		\
	htmlengine-edit-cursor.h		\
	htmlengine-edit-cut-and-paste.h         \
	htmlengine-edit-fontstyle.h		\
	htmlengine-edit-images.h		\
	htmlengine-edit-movement.h		\
	htmlengine-edit-rule.h			\
	htmlengine-edit-selection-updater.h	\
	htmlengine-edit-table.h			\
	htmlengine-edit-tablecell.h		\
	htmlengine-edit-text.h			\
	htmlengine-print.h			\
	htmlengine-save.h			\
	htmlengine-search.h			\
	htmlengine.h				\
	htmlentity.h				\
	htmlenums.h				\
	htmlenumutils.h				\
	htmlfontmanager.h			\
	htmlform.h				\
	htmlgdkpainter.h			\
	htmlplainpainter.h			\
	htmlhidden.h				\
	htmlimage.h				\
	htmlimageinput.h			\
	htmlinterval.h				\
	htmllinktext.h				\
	htmllist.h				\
	htmlmap.h				\
	htmlobject.h				\
	htmlpainter.h				\
	htmlprinter.h				\
	htmlradio.h				\
	htmlrule.h				\
	htmlsearch.h				\
	htmlreplace.h				\
	htmlselect.h				\
	htmlselection.h				\
	htmlsettings.h				\
	htmlshape.h				\
	htmlstack.h				\
	htmlstringtokenizer.h			\
	htmltable.h				\
	htmltablepriv.h				\
	htmltablecell.h				\
	htmltext.h				\
	htmltextarea.h				\
	htmltextinput.h				\
	htmltextslave.h				\
	htmltokenizer.h				\
	htmltype.h				\
	htmltypes.h				\
	htmlundo.h				\
	htmlundo-action.h			\
	htmlvspace.h				\
	gtkhtml-keybinding.h			\
	htmliframe.h				\
	htmlframe.h				\
	htmlframeset.h				\
	htmlengine-edit-table.h			\
	htmlhspace.h				\
	$(NULL)

libgtkhtml_1_1_la_SOURCES =				\
	gtkhtml-embedded.c			\
	gtkhtml-propmanager.c			\
	gtkhtml-im.c				\
	gtkhtml-properties.c			\
	gtkhtml-search.c			\
	gtkhtml-stream.c			\
	gtkhtml.c				\
	gtkhtmldebug.c				\
	gtkhtmlfontstyle.c			\
	htmlanchor.c				\
	htmlbutton.c				\
	htmlcheckbox.c				\
	htmlclue.c				\
	htmlcluealigned.c			\
	htmlclueflow.c				\
	htmlclueh.c				\
	htmlcluev.c				\
	htmlcolor.c				\
	htmlcolorset.c				\
	htmlcursor.c				\
	htmldrawqueue.c				\
	htmlembedded.c				\
	htmlengine-edit.c			\
	htmlengine-edit-clueflowstyle.c		\
	htmlengine-edit-cursor.c		\
	htmlengine-edit-cut-and-paste.c		\
	htmlengine-edit-fontstyle.c		\
	htmlengine-edit-images.c		\
	htmlengine-edit-movement.c		\
	htmlengine-edit-rule.c			\
	htmlengine-edit-selection-updater.c	\
	htmlengine-edit-table.c			\
	htmlengine-edit-tablecell.c		\
	htmlengine-edit-text.c			\
	htmlengine-print.c			\
	htmlengine-save.c			\
	htmlengine-search.c			\
	htmlengine.c				\
	htmlentity.c				\
	htmlenumutils.c				\
	htmlfontmanager.c			\
	htmlform.c				\
	htmlgdkpainter.c			\
	htmlplainpainter.c			\
	htmlhidden.c				\
	htmlimage.c				\
	htmlimageinput.c			\
	htmlinterval.c				\
	htmllinktext.c				\
	htmllist.c				\
	htmlmap.c				\
	htmlobject.c				\
	htmlpainter.c				\
	htmlprinter.c				\
	htmlradio.c				\
	htmlrule.c				\
	htmlsearch.c				\
	htmlreplace.c				\
	htmlselect.c				\
	htmlselection.c				\
	htmlsettings.c				\
	htmlshape.c				\
	htmlstack.c				\
	htmlstringtokenizer.c			\
	htmltable.c				\
	htmltablecell.c				\
	htmltext.c				\
	htmltextarea.c				\
	htmltextinput.c				\
	htmltextslave.c				\
	htmltokenizer.c				\
	htmltype.c				\
	htmlundo.c				\
	htmlundo-action.c			\
	htmlvspace.c				\
	htmliframe.c				\
	htmlframe.c				\
	htmlframeset.c				\
	$(NULL)
#	htmlhspace.c				\
#	htmlelement.c				\
#	html-component.c			\
#	htmlengine-edit-table.c			\

libgtkhtml_1_1_la_LDFLAGS = -version-info $(GTKHTML_CURRENT):$(GTKHTML_REVISION):$(GTKHTML_AGE)

libgtkhtml_1_1_la_LIBADD = $(EXTRA_GNOME_LIBS) $(GCONF_LIBS)

# The test program.

testgtkhtml_SOURCES = 		\
	testgtkhtml.c		\
	htmlurl.h		\
	htmlurl.c

# testgtkhtml_LDFLAGS = -static
testgtkhtml_LDADD =				\
	libgtkhtml-@GTKHTML_RELEASE@.la		\
	$(GNOMEUI_LIBS)				\
	$(GNOME_LIBDIR)				\
	$(EXTRA_GNOME_LIBS)                     \
	$(SOUP_LIBS)				\
	$(GCONF_LIBS)
#	-lccmalloc -ldl
#

gtest_SOURCES = \
	test.c
gtest_LDADD =					\
	libgtkhtml-@GTKHTML_RELEASE@.la		\
	$(GNOMEUI_LIBS)				\
	$(GNOME_LIBDIR)				\
	$(GCONF_LIBS)				\
	$(EXTRA_GNOME_LIBS)                     

dist-hook:
	mkdir $(distdir)/tests
	cp -f $(srcdir)/tests/*.html $(distdir)/tests
	cp -f $(srcdir)/tests/*.jpg $(distdir)/tests

EXTRA_DIST = \
	keybindingsrc.emacs \
	keybindingsrc.xemacs \
	keybindingsrc.ms

keybindingsdir = $(GTKHTML_DATADIR)
keybindings_DATA = \
	keybindingsrc.emacs \
	keybindingsrc.xemacs \
	keybindingsrc.ms
