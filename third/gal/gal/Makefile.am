nonui_subdirs = util unicode
ui_subdirs    = widgets e-paned e-table e-text menus shortcut-bar

SUBDIRS = $(nonui_subdirs) $(ui_subdirs) 

lib_LTLIBRARIES = libgal.la

libgal_la_LDFLAGS = -version-info $(GAL_CURRENT):$(GAL_REVISION):$(GAL_AGE) \
		    $(ICONV_LIBS) $(GTK_LIBS)
libgal_la_SOURCES = bogus_automake_hack.c

libgal_la_LIBADD = util/libutil.la \
		   unicode/libunicode.la \
		   widgets/libwidgets.la \
		   e-paned/libepaned.la \
		   e-table/libetable.la \
		   e-text/libetext.la \
		   menus/libgalmenus.la \
		   shortcut-bar/libshortcut-bar.la
