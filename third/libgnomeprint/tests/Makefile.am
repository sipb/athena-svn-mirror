CFLAGS=$(WARN_CFLAGS)

noinst_PROGRAMS = generate
## testchema

INCLUDES = \
	-I$(top_srcdir) \
	$(LIBGNOMEPRINT_CFLAGS)	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DBINDIR=\""$(bindir)"\" \
	-DDATADIR=\""$(datadir)"\"

print_libs = \
	$(top_builddir)/libgnomeprint/libgnomeprint-2.la \
	$(LIBGNOMEPRINT_LIBS) \
	-lpopt \
	$(INTLLIBS)

generate_SOURCES = generate.c
generate_LDFLAGS =
generate_LDADD = $(print_libs)

EXTRA_DIST = quit.txt

## testchema_SOURCES = testchema.c
## testchema_LDFLAGS =
## testchema_DEPENDENCIES = $(DEPS)
## testchema_LDADD = $(print_libs)
