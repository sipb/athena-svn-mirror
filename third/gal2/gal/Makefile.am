nonui_subdirs = util
ui_subdirs    = widgets e-table e-text menus shortcut-bar a11y

SUBDIRS = $(nonui_subdirs) $(ui_subdirs) 

lib_LTLIBRARIES = libgal-2.2.la

libgal_2_2_la_LDFLAGS = -version-info $(GAL_CURRENT):$(GAL_REVISION):$(GAL_AGE) \
		    $(ICONV_LIBS)
libgal_2_2_la_SOURCES = bogus_automake_hack.c

libgal_2_2_la_LIBADD = util/libutil.la \
		   widgets/libwidgets.la \
		   e-table/libetable.la \
		   e-text/libetext.la \
		   menus/libgalmenus.la \
		   shortcut-bar/libshortcut-bar.la \
		   a11y/libgal-a11y-2.2.la		\
		   $(EXTRA_GNOME_LIBS)
