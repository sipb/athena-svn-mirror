plugindir = $(GNOME_PRINT_MODULES_DIR)

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libgnomeprint/gpa \
	$(LIBGNOMEPRINT_CFLAGS) \
	-DG_LOG_DOMAIN=\"GnomePrintCupsPlugin\" \
	$(CUPS_CFLAGS)

plugin_LTLIBRARIES = libgnomeprintcups.la

libgnomeprintcups_la_SOURCES = gnome-print-cups.c gnome-print-cups-transport.c
libgnomeprintcups_la_LDFLAGS = $(CUPS_LIBS) $(TOOLS_LIBS) -avoid-version -module -no-undefined
libgnomeprintcups_la_LIBADD  = ../../libgnomeprint-2-2.la
