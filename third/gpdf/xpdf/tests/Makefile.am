INCLUDES = \
	-I$(srcdir)/..					\
	-I$(top_srcdir)					\
	-I$(top_srcdir)/goo				\
	-I$(top_srcdir)/lib				\
	-I$(top_srcdir)/lib/ggv-sidebar			\
	-D TESTFILES_DIR=\"$(top_builddir)/test-files\"	\
	$(EXTRA_GNOME_CFLAGS)

LDADD = \
	$(EXTRA_GNOME_LIBS)					\
	../libgpdf.a						\
	../libxpdf.a						\
	$(top_builddir)/fofi/libfofi.a				\
	$(top_builddir)/goo/libGoo.a				\
	$(top_builddir)/lib/ggv-sidebar/libggv-sidebar.a	\
	$(libpaper_LIBS)

if HAVE_PDFTEX
TESTS_NEEDING_PDFTEX =					\
	test-gpdf-view					\
	test-links					\
	test-pdf-loading
else
TESTS_NEEDING_PDFTEX =
endif

UNIT_TESTS = \
	test-gpdf-control				\
	test-page-setup					\
	$(TESTS_NEEDING_PDFTEX)

noinst_PROGRAMS = $(UNIT_TESTS)

test_gpdf_control_SOURCES = test-gpdf-control.cc

test_gpdf_view_SOURCES = test-gpdf-view.cc

test_links_SOURCES = test-links.cc

test_page_setup_SOURCES = test-page-setup.cc

test_pdf_loading_SOURCES = test-pdf-loading.cc

TESTS = $(UNIT_TESTS)

TESTS_ENVIRONMENT = function RUNTST { $$1; RC="$$?"; if test $$RC -ne 0; then echo "test: Test failed: $$1"; exit; fi; }; RUNTST
