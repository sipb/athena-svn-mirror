NULL=

INCLUDES = 				\
	-I$(top_srcdir)			\
	-I$(srcdir)			\
	-I$(includedir)			\
	$(GTKHTML_CFLAGS)		\
	$(SOUP_CFLAGS)			\
	-DG_LOG_DOMAIN=\"gtkhtml\"	\
	-DSRCDIR=\"$(srcdir)\"		\
	-DPREFIX=\"$(prefix)\"

AM_CFLAGS = 								\
	-DICONDIR=\"$(datadir)/gtkhtml-@GTKHTML_API_VERSION@/icons\"	\
	-DGDK_DISABLE_DEPRECATED=1					\
	-DG_DISABLE_DEPRECATED=1					\
	-DGNOME_DISABLE_DEPRECATED=1
#	-DGTK_DISABLE_DEPRECATED=1

lib_LTLIBRARIES = libgtkhtml-3.1.la

libgtkhtmldir = $(includedir)/libgtkhtml-@GTKHTML_API_VERSION@/gtkhtml

# FIXME not all the stuff should be installed!

libgtkhtml_HEADERS =				\
	gtkhtml-compat.h			\
	gtkhtml-embedded.h			\
	gtkhtml-enums.h				\
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
	htmllist.h				\
	htmlmap.h				\
	htmlmarshal.h				\
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
	htmlstyle.h				\
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
	gtkhtml-keybinding.h			\
	htmliframe.h				\
	htmlframe.h				\
	htmlframeset.h				\
	htmlengine-edit-table.h			\
	$(NULL)

libgtkhtml_3_1_la_SOURCES =			\
	gtkhtml-embedded.c			\
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
	htmllist.c				\
	htmlmap.c				\
	htmlmarshal.c				\
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
	htmlstyle.c				\
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
	htmliframe.c				\
	htmlframe.c				\
	htmlframeset.c				\
	$(NULL)

libgtkhtml_3_1_la_LDFLAGS = -version-info $(GTKHTML_CURRENT):$(GTKHTML_REVISION):$(GTKHTML_AGE)
libgtkhtml_3_1_la_LIBADD = $(GTKHTML_LIBS) ../a11y/libgtkhtml-a11y.la $(GAIL_LIBS)

dist-hook:
	mkdir $(distdir)/tests
	cp -f $(srcdir)/tests/*.html $(distdir)/tests
	cp -f $(srcdir)/tests/*.jpg $(distdir)/tests

if HAVE_SOUP
test_programs = testgtkhtml
endif
noinst_PROGRAMS = $(test_programs) gtest

testgtkhtml_SOURCES = 		\
	testgtkhtml.c		\
	htmlurl.h		\
	htmlurl.c
#testgtkhtml_LDFLAGS = -static
testgtkhtml_LDADD =					\
	libgtkhtml-@GTKHTML_API_VERSION@.la			\
	$(SOUP_LIBS)

gtest_SOURCES = \
	test.c
gtest_LDFLAGS =
gtest_LDADD =	\
	libgtkhtml-@GTKHTML_API_VERSION@.la

pkgconfig_in_files = libgtkhtml.pc.in
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(pkgconfig_in_files:.pc.in=.pc)

keybindingsdir = $(datadir)/gtkhtml-@GTKHTML_API_VERSION@
keybindings_DATA =		\
	keybindingsrc.emacs

EXTRA_DIST =			\
	htmlclosures.list	\
	$(keybindings_DATA)	\
	$(NULL)

install-data-hook:
	mv $(DESTDIR)$(libdir)/pkgconfig/libgtkhtml.pc $(DESTDIR)$(libdir)/pkgconfig/libgtkhtml-@GTKHTML_API_VERSION@.pc

uninstall-hook:
	rm -f $(DESTDIR)$(libdir)/pkgconfig/libgtkhtml-@GTKHTML_API_VERSION@.pc	

htmlmarshal.h:	htmlclosures.list
	glib-genmarshal --header --prefix=html_g_cclosure_marshal $< > $@

htmlmarshal.c:	htmlclosures.list
	( echo '#include "htmlmarshal.h"' > $@ &&				\
	glib-genmarshal --body --prefix=html_g_cclosure_marshal $< >> $@ ) || rm -f $@
