plugindir = $(GNOME_PRINT_MODULES_DIR)

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libgnomeprint/gpa \
	$(LIBGNOMEPRINT_CFLAGS) \
	-DG_LOG_DOMAIN=\"GnomePrintLpdPlugin\"

plugin_LTLIBRARIES = libgnomeprintlpd.la

libgnomeprintlpd_la_SOURCES = gnome-print-lpd.c
libgnomeprintlpd_la_LDFLAGS = $(TOOLS_LIBS) -avoid-version -module -no-undefined
libgnomeprintlpd_la_LIBADD  = ../../libgnomeprint-2-2.la
