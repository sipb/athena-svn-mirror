SUBDIRS = gpaui

lib_LTLIBRARIES = libgnomeprintui-2-2.la

INCLUDES = \
	-I$(top_srcdir)			\
	$(LIBGNOMEPRINTUI_CFLAGS)	\
	-DG_LOG_DOMAIN=\"libgnomeprintui\"	\
	-DVERSION=\"$(VERSION)\"	\
	-DWE_ARE_LIBGNOMEPRINT_INTERNALS	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DGNOME_PRINT_LIBDIR=\"$(libdir)/gnome-print\"	\
	-DBINDIR=\""$(bindir)"\"	\
	-DDATADIR=\""$(datadir)"\"	\
	\
	-DG_DISABLE_DEPRECATED		\
	-DGDK_DISABLE_DEPRECATED	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED	



libgnomeprintui_2_2_la_LIBADD = \
	gpaui/libgpaui.la \
	$(LIBGNOMEPRINTUI_LIBS)

libgnomeprintui_2_2_la_LDFLAGS = \
	-no-undefined \
	-version-info $(GNOMEPRINT_CURRENT):$(GNOMEPRINT_REVISION):$(GNOMEPRINT_AGE)

BUILT_SOURCES = gnome-printui-marshal.c  gnome-printui-marshal.h

libgnomeprintui_2_2_la_SOURCES = \
	gnome-print-i18n.c           gnome-print-i18n.h			\
	gnome-print-preview.c        gnome-print-preview-private.h	\
	gnome-canvas-hacktext.c      gnome-canvas-hacktext.h		\
	gnome-print-job-preview.c	\
	gnome-printer-selector.c     gnome-printer-selector.h		\
	gnome-print-copies.c         gnome-print-copies.h		\
	gnome-print-dialog.c         gnome-print-dialog-private.h	\
	gnome-print-config-dialog.c  gnome-print-config-dialog-private.h \
        gnome-print-widget.c         gnome-print-widget.h           \
	gnome-font-dialog.c		\
	gnome-print-paper-preview.c  gnome-print-paper-preview.h	\
	gnome-print-unit-selector.c	\
	gnome-print-paper-selector.c	\
	$(BUILT_SOURCES)


libgnomeprintuiincdir = $(includedir)/libgnomeprintui-2.2/libgnomeprintui

libgnomeprintuiinc_HEADERS = \
	gnome-print-preview.h \
	gnome-font-dialog.h \
	gnome-print-dialog.h \
	gnome-print-unit-selector.h \
	gnome-print-paper-selector.h \
	gnome-print-config-dialog.h \
	gnome-print-job-preview.h	

EXTRA_DIST = libgnomeprintui-2.2.pc.in \
	libgnomeprintui-uninstalled-2.2.pc.in \
	gnome-printui-marshal.list
CLEANFILES = $(BUILD_SOURCES)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgnomeprintui-2.2.pc

gnome-printui-marshal.h: gnome-printui-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=libgnomeprintui_marshal > $@

gnome-printui-marshal.c: gnome-printui-marshal.list $(GLIB_GENMARSHAL)
	echo "#include \"gnome-printui-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=libgnomeprintui_marshal >> $@

