plugindir = $(GNOME_PRINT_MODULES_DIR)

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libgnomeprint/gpa \
	$(LIBGNOMEPRINT_CFLAGS) \
	-DG_LOG_DOMAIN=\"GnomePrintPAPIPlugin\"

plugin_LTLIBRARIES = libgnomeprintpapi.la

libgnomeprintpapi_la_SOURCES = gnome-print-papi.c gnome-print-papi-transport.c
libgnomeprintpapi_la_LDFLAGS = $(PAPI_LIBS) $(Z_LIBS)
libgnomeprintpapi_la_LIBADD  = ../../libgnomeprint-2-2.la
