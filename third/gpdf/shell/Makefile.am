SUBDIRS = . tests

INCLUDES =							\
	-I$(top_srcdir)						\
	-I$(top_srcdir)/lib					\
	-DDATADIR=\""$(datadir)"\"				\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
	-DGNOMEICONDIR=\""$(datadir)/pixmaps"\"			\
	$(EXTRA_GNOME_CFLAGS)					\
	$(GPDF_DEPRECATED_CFLAGS)

bin_PROGRAMS = gpdf

gpdf_SOURCES =							\
	gpdf.c							\
	gpdf-window.h

gpdf_LDADD =							\
	$(EXTRA_GNOME_LIBS)					\
	libgpdfshell.a						\
	../lib/recent-files/librecent.a

noinst_LIBRARIES = libgpdfshell.a

libgpdfshell_a_SOURCES =		\
	eel-string.c			\
	eel-string.h			\
	eel-vfs-extensions.c		\
	eel-vfs-extensions.h		\
	gpdf-uri-input.c		\
	gpdf-uri-input.h		\
	gpdf-recent-facade.c		\
	gpdf-recent-facade.h		\
	gpdf-recent-view-toolitem.c	\
	gpdf-recent-view-toolitem.h	\
	mock-recent-facade.c

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = gpdf-window-ui.xml

EXTRA_DIST = $(ui_DATA)