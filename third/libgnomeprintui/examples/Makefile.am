if WITH_LIBGLADE
libglade_cflags=$(LIBGLADE_CFLAGS)
libglade_libs=$(LIBGLADE_LIBS)
glade_programs=example_10
CLEANFILES=*.bak *~ *.ps
else
CLEANFILES=example_10 *.bak *~ *.ps
glade_programs=
libglade_cflags=
libglade_libs=
endif

noinst_PROGRAMS= \
	example_01 \
	example_02 \
	example_03 \
	example_04 \
	example_05 \
	example_06 \
	example_08 \
	example_09 $(glade_programs) \
	example_11 \
	example_12

INCLUDES = \
	-I$(top_srcdir) \
	$(LIBGNOMEPRINTUI_CFLAGS) $(libglade_cflags)


if EXAMPLES_USE_LOCAL_LIBGNOMEPRINT
libgnomeprint_local=$(top_builddir)/../libgnomeprint/libgnomeprint/libgnomeprint-2-2.la
else
libgnomeprint_local=
endif

EXTRA_DIST = sample-image.png example_10.glade example_10.gladep README

## including ../../libngomeprint/libgnomeprint/libgnomeprint-2.la is not very robust
## but it allows us to not have to make install in libgnomeprint to test new stuff
print_libs = \
	$(top_builddir)/libgnomeprintui/libgnomeprintui-2-2.la \
	$(libgnomeprint_local) \
	$(LIBGNOMEPRINTUI_LIBS) $(libglade_libs)

example_01_SOURCES = example_01.c
example_02_SOURCES = example_02.c
example_03_SOURCES = example_03.c
example_04_SOURCES = example_04.c
example_05_SOURCES = example_05.c
example_06_SOURCES = example_06.c
example_08_SOURCES = example_08.c
example_09_SOURCES = example_09.c
example_11_SOURCES = example_11.c
example_12_SOURCES = example_12.c
if WITH_LIBGLADE
example_10_SOURCES = example_10.c
endif

example_01_LDADD = $(print_libs)
example_02_LDADD = $(print_libs)
example_03_LDADD = $(print_libs)
example_04_LDADD = $(print_libs)
example_05_LDADD = $(print_libs)
example_06_LDADD = $(print_libs)
example_08_LDADD = $(print_libs)
example_09_LDADD = $(print_libs)
example_11_LDADD = $(print_libs)
example_12_LDADD = $(print_libs)

if WITH_LIBGLADE
example_10_LDADD = $(print_libs)
endif
