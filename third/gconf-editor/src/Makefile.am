INCLUDES = \
	-DLOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DGCONF_EDITOR_IMAGEDIR=\""$(datadir)/pixmaps/gconf-editor"\" \
	-DIMAGEDIR=\""$(datadir)/pixmaps"\" \
	-I$(includedir)		\
	-DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED \
	-DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED \
	-DGNOME_DISABLE_DEPRECATED -DGNOME_VFS_DISABLE_DEPRECATED \
	$(GCONF_EDITOR_CFLAGS)	

bin_PROGRAMS = gconf-editor

gconf_editor_built_headers = gconf-marshal.h
gconf_editor_built_cfiles = gconf-marshal.c

gconf_editor_SOURCES = \
	gconf-bookmarks.c		\
	gconf-bookmarks.h		\
	gconf-bookmarks-dialog.c	\
	gconf-bookmarks-dialog.h	\
	gconf-cell-renderer.c		\
	gconf-cell-renderer.h 		\
	gconf-editor-application.c	\
	gconf-editor-application.h	\
	gconf-editor-window.c		\
	gconf-editor-window.h		\
	gconf-key-editor.c		\
	gconf-key-editor.h		\
	gconf-list-model.c		\
	gconf-list-model.h		\
	gconf-message-dialog.c		\
	gconf-message-dialog.h		\
	gconf-stock-icons.c		\
	gconf-stock-icons.h		\
	gconf-tree-model.c		\
	gconf-tree-model.h		\
	gconf-util.c			\
	gconf-util.h			\
	main.c				\
	$(gconf_editor_built_headers)	\
	$(gconf_editor_built_cfiles)

## we just punt on built sources, and check them into cvs, manually updating 
## when required.
regenerate-built-sources:
	echo "Building gconf-marshal.h" \
	&& @GLIB_GENMARSHAL@ --prefix=gconf_marshal gconf-marshal.list --header > gconf-marshal.h \
	&& echo "Building gconf-marshal.c" \
	&& echo "#include <gconf-marshal.h>" > gconf-marshal.c \
        && @GLIB_GENMARSHAL@ --prefix=gconf_marshal gconf-marshal.list --body >> gconf-marshal.c


gconf_editor_LDADD = \
	$(GCONF_EDITOR_LIBS)

EXTRA_DIST = \
	gconf-marshal.list
