noinst_LTLIBRARIES = libgpa.la

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libgnomeprint \
	$(LIBGNOMEPRINT_CFLAGS) \
	-DBINDIR=\""$(bindir)"\" \
	-DDATADIR=\""$(datadir)"\" \
     -DGPA_MODULES_DIR=\""$(GNOME_PRINT_MODULES_DIR)"\" \
     -DGPA_DATA_DIR=\""$(GNOME_PRINT_DATA_DIR)"\" \
	\
	-DG_DISABLE_DEPRECATED				\
	-DGDK_DISABLE_DEPRECATED				\
	-DGTK_DISABLE_DEPRECATED				\
	-DGDK_PIXBUF_DISABLE_DEPRECATED		\
	-DGNOME_DISABLE_DEPRECATED

libgpa_la_SOURCES = \
	gpa-node.c      gpa-node.h gpa-node-private.h \
	gpa-list.c      \
	gpa-reference.c \
	\
	gpa-model.c   \
	gpa-option.c  \
	gpa-printer.c \
	\
	gpa-root.c     \
	gpa-config.c   \
	gpa-key.c      \
	gpa-settings.c \
	\
	gpa-utils.c    gpa-utils.h

libgpaincdir = $(includedir)/libgnomeprint-2.2/libgnomeprint/private

libgpainc_HEADERS = \
	gpa-node.h \
	gpa-utils.h \
	gpa-printer.h \
	gpa-key.h \
	gpa-option.h \
	gpa-node-private.h \
	gpa-list.h \
	gpa-model.h \
	gpa-settings.h \
	gpa-reference.h \
	gpa-root.h \
	gpa-config.h