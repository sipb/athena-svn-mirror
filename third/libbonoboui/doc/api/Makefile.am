INCLUDES = -I$(top_srcdir) -I$(top_builddir)

## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = 1.6

SUBDIRS = figures

# The name of the module, e.g. 'glib'.
DOC_MODULE=libbonoboui

# The top-level SGML file. Change it if you want.
DOC_MAIN_SGML_FILE=$(DOC_MODULE)-docs.sgml

# The directory containing the source code. Relative to $(srcdir).
# gtk-doc will search all .c & .h files beneath here for inline comments
# documenting functions and macros.
DOC_SOURCE_DIR=../../bonobo

# Extra options to supply to gtkdoc-scan.
SCAN_OPTIONS=--deprecated-guards="BONOBO_UI_DISABLE_DEPRECATED"

# Extra options to supply to gtkdoc-mkdb.
MKDB_OPTIONS=--sgml-mode --output-format=xml

# Extra options to supply to gtkdoc-fixref.
FIXXREF_OPTIONS=

# Used for dependencies.
HFILE_GLOB=
CFILE_GLOB=

# Header files to ignore when scanning.
IGNORE_HFILES= bonobo-client-site.h		\
	bonobo-control-internal.h		\
	bonobo-dock-band.h			\
	bonobo-dock-item-grip.h			\
	bonobo-dock-item.h			\
	bonobo-dock-layout.h			\
	bonobo-dock.h				\
	bonobo-embeddable-factory.h		\
	bonobo-item-container.h			\
	bonobo-item-handler.h			\
	bonobo-transient.h			\
	bonobo-ui-config-widget.h		\
	bonobo-ui-engine-private.h		\
	bonobo-ui-marshal.h			\
	bonobo-ui-node-private.h		\
	bonobo-ui-private.h			\
	bonobo-ui-xml.h				\
	bonobo-ui-sync-keys.h			\
	bonobo-ui-sync-menu.h			\
	bonobo-ui-sync-status.h			\
	bonobo-ui-sync-toolbar.h		\
	bonobo-ui-sync-status.h			\
	bonobo-ui-toolbar-button-item.h		\
	bonobo-ui-toolbar-control-item.h	\
	bonobo-ui-toolbar-icon.h		\
	bonobo-ui-toolbar-item.h		\
	bonobo-ui-toolbar-popup-item.h		\
	bonobo-ui-toolbar-separator-item.h	\
	bonobo-ui-toolbar.h			\
	bonobo-ui-toolbar-toggle-button-item.h	\
	bonobo-canvas-item.h			\
	bonobo-canvas-component.h		\
	bonobo-ui-type-builtins.h


# Images to copy into HTML directory.
HTML_IMAGES = figures/control.png figures/control-frame.png

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE).
content_files = ui-faq.xml

# Other files to distribute.
extra_files =


# CFLAGS and LDFLAGS for compiling scan program. Only needed
# if $(DOC_MODULE).types is non-empty.
GTKDOC_CFLAGS =					\
	@CFLAGS@				\
	-I$(top_srcdir)				\
	-DBONOBO_UI_INTERNAL			\
	$(BONOBOUI_CFLAGS)

GTKDOC_LIBS = $(top_builddir)/bonobo/libbonoboui-2.la $(BONOBOUI_LIBS)


# If you need to override some of the declarations, place them in the
# $(DOC_MODULE)-overrides.txt file and uncomment the second line here.
DOC_OVERRIDES =
#DOC_OVERRIDES = $(DOC_MODULE)-overrides.txt


# include common portion ...
include $(top_srcdir)/gtk-doc.make

