SUBDIRS = gpaui

lib_LTLIBRARIES = libgnomeprintui-2.la

INCLUDES = \
	-I$(top_srcdir) \
	$(LIBGNOMEPRINTUI_CFLAGS) \
	-DG_LOG_DOMAIN=\"libgnomeprintui\" \
	-DVERSION=\"$(VERSION)\" \
	-DWE_ARE_LIBGNOMEPRINT_UI \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" 	\
	-DGNOME_PRINT_LIBDIR=\"$(libdir)/gnome-print\" \
	-DBINDIR=\""$(bindir)"\"	\
	-DDATADIR=\""$(datadir)"\" 


libgnomeprintui_2_la_LIBADD = \
	gpaui/libgpaui.la \
	$(LIBGNOMEPRINTUI_LIBS)

libgnomeprintui_2_la_SOURCES = \
	gnome-print-i18n.c gnome-print-i18n.h \
	gnome-print-preview.c gnome-print-preview-private.h \
	gnome-canvas-hacktext.c gnome-canvas-hacktext.h \
	gnome-print-master-preview.c \
	gnome-font-dialog.c \
	gnome-printer-dialog.c \
	gnome-print-copies.c \
	gnome-print-dialog.c \
	gnome-print-paper-selector.c

libgnomeprintuiincdir = $(includedir)/libgnomeprintui-2.0/libgnomeprintui

libgnomeprintuiinc_HEADERS = \
	gnome-print-preview.h \
	gnome-print-master-preview.h \
	gnome-font-dialog.h \
	gnome-printer-dialog.h \
	gnome-print-copies.h \
	gnome-print-dialog.h \
	gnome-print-paper-selector.h

EXTRA_DIST = libgnomeprintui-2.0.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgnomeprintui-2.0.pc
