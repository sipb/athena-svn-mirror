INCLUDES = \
	-I$(top_srcdir)						\
	$(GPDF_DEPRECATED_CFLAGS)				\
	$(EXTRA_GNOME_CFLAGS)

EGG_FILES =				\
        egg-recent-model.c		\
        egg-recent-model.h		\
        egg-recent-item.c		\
        egg-recent-item.h		\
        egg-recent-view.c		\
        egg-recent-view.h		\
        egg-recent-view-bonobo.c	\
        egg-recent-view-bonobo.h	\
        egg-recent-view-gtk.c		\
        egg-recent-view-gtk.h		\
        egg-recent-util.c		\
	egg-recent-util.h

noinst_LIBRARIES = librecent.a

librecent_a_SOURCES = $(EGG_FILES)

EXTRA_DIST =				\
	config.h			\
	update-from-egg.sh

EGG_DIR=$(srcdir)/../../../libegg/libegg/recent-files
regenerate-built-sources:
	EGGFILES="$(EGG_FILES)" EGGDIR="$(EGG_DIR)" $(srcdir)/update-from-egg.sh

