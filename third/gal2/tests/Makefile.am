

INCLUDES =					\
	-I$(top_srcdir)                         \
	-I$(top_srcdir)/gal                     \
	$(EXTRA_GNOME_CFLAGS)			\
	$(GNOME_BONOBO_CFLAGS)			\
	$(GNOME_INCLUDEDIR)			\
	-DETABLE_GLADEDIR=\"$(gladedir)\"	\
        -DG_LOG_DOMAIN=\"e-table\"

LDADD =	$(top_builddir)/gal/libgal-2.2.la \
	$(EXTRA_GNOME_LIBS) $(ICONV_LIBS)

noinst_PROGRAMS = \
	test-completion \
	test-table-1 \
	test-text \
	test-tree-1 test-tree-3 \
	test-shortcut-bar \
	test-define-views

test_shortcut_bar_SOURCES = 	\
	test-shortcut-bar.c

test_define_views_SOURCES = 	\
	test-define-views.c
