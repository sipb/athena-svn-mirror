## Process this file with automake to produce Makefile.in

CFLAGS=-g ${LIBGLADE_CFLAGS}

INCLUDES = \
	-DGNOMELOCALEDIR=\""$(gnomeutilslocaledir)"\"	\
        -DGLADEDIR=\""$(gladedir)"\"           	\
        -DDATADIR=\""$(datadir)"\"		\
	$(GNOME_UTILS_CFLAGS)

bin_PROGRAMS = gfloppy

gladedir = $(pkgdatadir)/glade
glade_DATA = gfloppy2.glade

gfloppy-strings.c: gfloppy2.glade
	libglade-xgettext -c -o $(srcdir)/gfloppy-strings.c $(srcdir)/gfloppy2.glade

gfloppy_SOURCES = \
	main.c \
	gfloppy-config.c \
	gfloppy-config.h \
	gfloppy.c \
	progress.c \
	fdformat.c \
	badblocks.c

gfloppy_LDADD = $(GNOME_UTILS_LIBS) ${LIBGLADE_LIBS} -lext2fs

gfloppy_LDFLAGS = -export-dynamic

man_MANS = gfloppy.1

EXTRA_DIST = 		     \
	gfloppy2.glade		\
	gfloppy.h		\
	progress.h		\
	fdformat.h		\
	badblocks.h             \
	gfloppy.1

update-po: gfloppy-strings.c
	cd ../po && make update-po
