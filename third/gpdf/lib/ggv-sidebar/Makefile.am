INCLUDES =						\
	-I$(top_srcdir)					\
	-DGNOMEICONDIR=\""$(datadir)/pixmaps"\"		\
	$(EXTRA_GNOME_CFLAGS)				\
	$(GPDF_DEPRECATED_CFLAGS)


GGV_FILES_WITH_PATHS =		\
	bonobo/ggv-document.c	\
	bonobo/ggv-document.h	\
	bonobo/ggv-sidebar.c	\
	bonobo/ggv-sidebar.h	\
	src/gtkchecklist.c	\
	src/gtkchecklist.h

# $(notdir $(GGV_FILES_WITH_PATHS)) confuses automake
GGV_FILES =			\
	ggv-document.c		\
	ggv-document.h		\
	ggv-sidebar.c		\
	ggv-sidebar.h		\
	gtkchecklist.c		\
	gtkchecklist.h

noinst_LIBRARIES = libggv-sidebar.a

libggv_sidebar_a_SOURCES =	\
	ggv-document.c		\
	ggv-document.h

#	ggvutils.c		\
#	ggvutils.h		\
#	ggv-prefs.h		\
#	$(GGV_FILES)

test:
	@echo $(libggv_sidebar_a_SOURCES)

EXTRA_DIST =			\
	config.h		\
	update-from-ggv.sh

GGV_DIR=$(srcdir)/../../../ggv
regenerate-built-sources:
	GGVFILES="$(GGV_FILES_WITH_PATHS)" GGVDIR="$(GGV_DIR)" $(srcdir)/update-from-ggv.sh
