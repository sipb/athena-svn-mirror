SUBDIRS = 
NULL =

lib_LIBRARIES = libammonite-gtk.a

libammonite_gtkincludedir = $(includedir)/libtrilobite

libammonite_gtkinclude_HEADERS = \
	ammonite-login-dialog.h	\
	libammonite-gtk.h		\
	$(NULL)

libammonite_gtk_a_LIBADD = \
	$(top_builddir)/libammonite/libammonite.a	\
	$(NULL)


libammonite_gtk_a_SOURCES = \
	ammonite-login-dialog.c 	\
	nautilus-caption-table.c 	\
	nautilus-caption-table.h	\
	nautilus-gtk-macros.h		\
	libammonite-gtk.c		\
	copy-paste.c			\
	copy-paste.h			\
	$(NULL)

INCLUDES = \
	-I$(top_srcdir)/libammonite \
	$(GCONF_CFLAGS)	\
	$(OAF_CFLAGS)	\
	$(GLIB_CFLAGS)	\
	$(GNOMEUI_CFLAGS) \
	$(WERROR)	\
	-DDATADIR=\""$(datadir)"\" \
	$(NULL)
