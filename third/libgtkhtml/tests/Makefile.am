INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libgtkhtml \
	$(GTKHTML2_CFLAGS) \
	$(GNOME_VFS_CFLAGS) \
	-DG_LOG_DOMAIN=\"GtkHtmlTest\" \
	-DGTKHTML_SAMPLES_DIRECTORY=\"$(top_srcdir)/tests/samples\"

DGNOMELOCALEDIR=\"$(datadir)/locale\" \
	-I../intl -I$(top_srcdir)/intl

if HAVE_GNOME_VFS
TESTGTKHTML = testgtkhtml
else
TESTGTKHTML =
endif

noinst_PROGRAMS = simple dom-test $(TESTGTKHTML)

simple_SOURCES = 		\
	simple.c

simple_LDADD = 			\
	$(top_builddir)/libgtkhtml/libgtkhtml-2.la \
	$(GTKHTML2_LIBS)

dom_test_SOURCES = 		\
	dom-test-node-menu.c	\
	dom-test-node-menu.h	\
	dom-test-pixmaps.h	\
	dom-test-tree-model.c	\
	dom-test-tree-model.h	\
	dom-test-window.c 	\
	dom-test-window.h 	\
	dom-test.c

dom_test_LDADD = 			\
	$(top_builddir)/libgtkhtml/libgtkhtml-2.la \
	$(GTKHTML2_LIBS)

testgtkhtml_SOURCES = 		\
	debug.c			\
	debug.h			\
	browser-window.c	\
	browser-window.h	\
	prop-editor.c		\
	prop-editor.h		\
	main.c

testgtkhtml_LDADD = \
	$(top_builddir)/libgtkhtml/libgtkhtml-2.la \
	$(GTKHTML2_LIBS) $(GNOME_VFS_LIBS)

