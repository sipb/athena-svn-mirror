## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.6

# The name of the module.
DOC_MODULE=gok

# The top-level SGML file.
DOC_MAIN_SGML_FILE=$(DOC_MODULE)-docs.sgml

# The directory containing the source code (if it contains documentation).
DOC_SOURCE_DIR=../../gok

# Used for dependencies
HFILE_GLOB =
CFILE_GLOB =

# CFLAGS and LDFLAGS for compiling scan program. Only needed
# if $(DOC_MODULE).types is non-empty.
INCLUDES =                      \
        -I$(top_srcdir)         \
        -I$(top_builddir)       \
        $(DEP_CFLAGS)

# gtkdoc-mkdb related varaibles
MKDB_OPTIONS =
content_files =

HTML_IMAGES =

include $(top_srcdir)/gtk-doc.make
