

INCLUDES =					\
	-I$(top_srcdir)                         \
	-I$(top_srcdir)/gal                     \
	$(EXTRA_GNOME_CFLAGS)			\
	$(GNOME_BONOBO_CFLAGS)			\
	$(GNOME_VFS_CFLAGS)			\
	$(GNOME_INCLUDEDIR)			\
	-DETABLE_GLADEDIR=\"$(gladedir)\"	\
        -DG_LOG_DOMAIN=\"e-table\"

LDFLAGS = `gnome-config --libs gdk_pixbuf`
LDADD =	$(top_builddir)/gal/libgal.la \
	$(EXTRA_GNOME_LIBS) $(ICONV_LIBS)

if GAL_HAVE_VFS
vfs_programs = # test-tree-2
endif

noinst_PROGRAMS = \
	test-table-1 test-tree-1 $(vfs_programs) test-tree-3 test-shortcut-bar test-define-views test-unicode

test_table_1_LDADD =	$(LDADD)
test_table_1_LDFLAGS = $(LDFLAGS)

test_tree_1_LDFLAGS = $(LDFLAGS)
test_tree_1_LDADD = $(LDADD)

test_shortcut_bar_LDFLAGS = $(LDFLAGS)
test_shortcut_bar_LDADD = $(LDADD)

if GAL_HAVE_VFS
#test_tree_2_LDFLAGS = $(LDFLAGS)
#test_tree_2_LDADD = $(LDADD)		\
#	$(GNOME_VFS_LIBS)
endif

test_tree_3_LDFLAGS = $(LDFLAGS)
test_tree_3_LDADD = $(LDADD) 

test_shortcut_bar_SOURCES = 	\
	test-shortcut-bar.c

test_define_views_SOURCES = 	\
	test-define-views.c

test_define_views_LDFLAGS = $(LDFLAGS)
test_define_views_LDADD   = $(LDADD)

test_unicode_LDFLAGS = $(LDFLAGS)
test_unicode_LDADD   = $(LDADD)























