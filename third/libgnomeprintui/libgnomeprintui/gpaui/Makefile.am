noinst_LTLIBRARIES = libgpaui.la

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/intl \
	-I$(top_srcdir)/libgnomeprintui \
	-DWE_ARE_LIBGNOMEPRINT_INTERNALS \
	$(LIBGNOMEPRINTUI_CFLAGS)	\
	-DG_DISABLE_DEPRECATED		\
	-DGDK_DISABLE_DEPRECATED	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED




AM_CPPFLAGS = \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DBINDIR=\""$(bindir)"\" \
	-DDATADIR=\""$(datadir)"\" 

libgpaui_la_SOURCES = \
	gpa-widget.c         gpa-widget.h \
	\
	gpa-option-menu.c        gpa-option-menu.h \
	gpa-printer-selector.c   gpa-printer-selector.h   \
	gpa-settings-selector.c  gpa-settings-selector.h  \
	gpa-transport-selector.c gpa-transport-selector.h \
	gpa-checkbutton.c        gpa-checkbutton.h        \
	gpa-radiobutton.c        gpa-radiobutton.h        \
	gpa-spinbutton.c         gpa-spinbutton.h         \
	gpa-print-to-file.c      gpa-print-to-file.h      \
	\
	gpa-paper-preview-item.c gpa-paper-preview-item.h \
	\
	gpa-tree-viewer.c        gpa-tree-viewer.h 

