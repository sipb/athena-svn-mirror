## Process this file with automake to produce Makefile.in

# The name of the module, e.g. 'glib'.
DOC_MODULE=libgnomeprintui

# The top-level SGML file. Change it if you want.
DOC_MAIN_SGML_FILE=$(DOC_MODULE)-docs.sgml

# The directory containing the source code. Relative to $(srcdir).
# gtk-doc will search all .c & .h files beneath here for inline comments
# documenting functions and macros.
DOC_SOURCE_DIR=$(top_srcdir)/libgnomeprintui

# Extra options to supply to gtkdoc-scan.
SCAN_OPTIONS=--deprecated-guards="GNOME_DISABLE_DEPRECATED"

# Extra options to supply to gtkdoc-mkdb.
MKDB_OPTIONS=

# Extra options to supply to gtkdoc-fixref.
FIXXREF_OPTIONS=

# Used for dependencies.
HFILE_GLOB=$(top_srcdir)/libgnomeprintui/*.h $(top_srcdir)/libgnomeprintui/*/*.h
CFILE_GLOB=$(top_srcdir)/libgnomeprintui/*.c $(top_srcdir)/libgnomeprintui/*/*.c

# Header files to ignore when scanning.
IGNORE_HFILES= \
	gnome-canvas-hacktext.h \
	gnome-print-i18n.h \
	gnome-print-pixbuf.h \
	gnome-print-preview-icons.h \
	gnome-print-preview-private.h \
	add-printer-dialog.h \
	config-dialog.h \
	gpa-widget.h \
	media-orientation.h \
	printer-selector.h \
	settings-selector.h \
	transport-selector.h

# Images to copy into HTML directory.
HTML_IMAGES =

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE).
content_files =

# Other files to distribute.
extra_files =

# CFLAGS and LDFLAGS for compiling scan program. Only needed if your app/lib
# contains GtkObjects/GObjects and you want to document signals and properties.
GTKDOC_CFLAGS = -DWE_ARE_LIBGNOMEPRINT_INTERNALS -I$(top_builddir) -I$(top_srcdir) $(LIBGNOMEPRINTUI_CFLAGS) $(CFLAGS)
GTKDOC_LIBS = $(top_builddir)/libgnomeprintui/libgnomeprintui-2-2.la $(LDFLAGS)

# _CC and _LD are already defined in gtk-doc.make
# GTKDOC_CC=$(LIBTOOL) --mode=compile $(CC)
# GTKDOC_LD=$(LIBTOOL) --mode=link $(CC)

# If you need to override some of the declarations, place them in this file
# and uncomment this line.
#DOC_OVERRIDES = $(DOC_MODULE)-overrides.txt
DOC_OVERRIDES =

include $(top_srcdir)/gtk-doc.make
