noinst_LTLIBRARIES = libgpaui.la

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/intl \
	$(LIBGNOMEPRINTUI_CFLAGS)

CPPFLAGS = \
	-DWE_ARE_LIBGNOMEPRINT_UI \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DBINDIR=\""$(bindir)"\" \
	-DDATADIR=\""$(datadir)"\" 

libgpaui_la_SOURCES = \
	gpa-widget.c gpa-widget.h \
	printer-selector.c printer-selector.h \
	settings-selector.c settings-selector.h \
	transport-selector.c transport-selector.h

#	media-orientation.c media-orientation.h

#
#	add-printer-dialog.c add-printer-dialog.h
#
#	config-dialog.c config-dialog.h

