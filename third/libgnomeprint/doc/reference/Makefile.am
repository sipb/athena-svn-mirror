## Process this file with automake to produce Makefile.in

# The name of the module, e.g. 'glib'.
DOC_MODULE=libgnomeprint

# The top-level SGML file. Change it if you want.
DOC_MAIN_SGML_FILE=$(DOC_MODULE)-docs.sgml

# The directory containing the source code. Relative to $(srcdir).
# gtk-doc will search all .c & .h files beneath here for inline comments
# documenting functions and macros.
DOC_SOURCE_DIR=$(top_builddir)/libgnomeprint

# Extra options to supply to gtkdoc-scan.
SCAN_OPTIONS=--deprecated-guards="GNOME_DISABLE_DEPRECATED"

# Extra options to supply to gtkdoc-mkdb.
MKDB_OPTIONS=--sgml-mode

# Extra options to supply to gtkdoc-fixref.
FIXXREF_OPTIONS=

# Used for dependencies.
HFILE_GLOB=$(top_srcdir)/libgnomeprint/*.h $(top_srcdir)/libgnomeprint/*/*.h
CFILE_GLOB=$(top_srcdir)/libgnomeprint/*.c $(top_srcdir)/libgnomeprint/*/*.c

# Header files to ignore when scanning.
IGNORE_HFILES=				\
	art_rgb_affine_private.h	\
	art_rgba_rgba_affine.h 		\
	art_rgba_svp.h			\
	\
	gp-fontmap.h			\
	gnome-font-family.h		\
	gp-truetype-utils.h		\
	gnome-font-private.h		\
	\
	gnome-print-bonobo-client.h	\
	gnome-print-meta.h		\
	gnome-print-master-preview.h	\
	gnome-print-multipage.h		\
	gnome-print-ps2.h		\
	gnome-print-rbuf.h		\
	gnome-print-rgbp.h		\
	gnome-print-transport.h		\
	gnome-print-i18n.h		\
	gnome-print-encode.h		\
	\
	gp-gc.h				\
	gp-gc-private.h			\
	\
	gnome-print-private.h		\
	gnome-print-job-private.h	\
	gnome-print-rbuf-private.h	\
	gnome-glyphlist-private.h	\
	gnome-pgl-private.h		\
	gnome-print-encode-private.h	\
	\
	ttsubset/ttcr.h			\
	ttsubset/sft.h			\
	\
	gpa-*.h

# Images to copy into HTML directory.
HTML_IMAGES =

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE).
content_files = \
	building.sgml \
	compiling.sgml \
	resources.sgml

# Other files to distribute.
extra_files =

# CFLAGS and LDFLAGS for compiling scan program. Only needed if your app/lib
# contains GtkObjects/GObjects and you want to document signals and properties.
GTKDOC_CFLAGS = -I$(top_builddir) -I$(top_srcdir) $(LIBGNOMEPRINT_CFLAGS) $(CFLAGS)
GTKDOC_LIBS = $(top_builddir)/libgnomeprint/libgnomeprint-2.la $(LDFLAGS)

# If you need to override some of the declarations, place them in this file
# and uncomment this line.
#DOC_OVERRIDES = $(DOC_MODULE)-overrides.txt
DOC_OVERRIDES =

include $(top_srcdir)/gtk-doc.make
