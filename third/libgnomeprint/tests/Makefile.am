SUBDIRS = files output tools

CFLAGS=$(WARN_CFLAGS)

noinst_PROGRAMS = generate gpa-test fonts

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libgnomeprint/gpa \
	$(LIBGNOMEPRINT_CFLAGS)

print_libs = \
	$(top_builddir)/libgnomeprint/libgnomeprint-2-2.la \
	$(LIBGNOMEPRINT_LIBS) \
	-lpopt

generate_SOURCES = generate.c
generate_LDFLAGS =
generate_LDADD = $(print_libs)

EXTRA_DIST = quit.txt run-test.pl quit.txt

gpa_test_SOURCES = gpa-test.c
gpa_test_LDFLAGS =
gpa_test_DEPENDENCIES = $(DEPS)
gpa_test_LDADD = $(print_libs)

fonts_SOURCES = fonts.c
fonts_LDFLAGS =
fonts_LDADD = $(print_libs)
