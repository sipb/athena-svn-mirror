INCLUDES = \
	-I$(srcdir)/..					\
	-I$(top_srcdir)/lib				\
	$(EXTRA_GNOME_CFLAGS)

LDADD = \
	$(EXTRA_GNOME_LIBS)				\
	../libgpdfshell.a				\
	$(top_builddir)/lib/recent-files/librecent.a

UNIT_TESTS = \
	test-uri-input

noinst_PROGRAMS = $(UNIT_TESTS)

TESTS = $(UNIT_TESTS)

TESTS_ENVIRONMENT = function RUNTST { $$1; RC="$$?"; if test $$RC -ne 0; then echo "test: Test failed: $$1"; exit; fi; }; RUNTST
