plugindir = $(GNOME_PRINT_MODULES_DIR)/transports
plugin_LTLIBRARIES = \
	$(FILE_LIB) \
	$(LPR_LIB) \
	$(CUSTOM_LIB)

AM_CPPFLAGS = \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"

INCLUDES = \
	-I$(top_srcdir) \
	$(LIBGNOMEPRINT_CFLAGS)

# FILE transport destination

FILE_LIB = libgnomeprint-file.la
FILE_SRC = gp-transport-file.c gp-transport-file.h

libgnomeprint_file_la_SOURCES = $(FILE_SRC)

libgnomeprint_file_la_LDFLAGS = -avoid-version -module -no-undefined

libgnomeprint_file_la_LIBADD = ../libgnomeprint-2-2.la \
	$(LIBGNOMEPRINT_LIBS)

# LPR transport destination

LPR_LIB = libgnomeprint-lpr.la
LPR_SRC = gp-transport-lpr.c gp-transport-lpr.h

libgnomeprint_lpr_la_SOURCES = $(LPR_SRC)

libgnomeprint_lpr_la_LDFLAGS = -avoid-version -module -no-undefined

libgnomeprint_lpr_la_LIBADD = ../libgnomeprint-2-2.la \
	$(LIBGNOMEPRINT_LIBS)

# Custom transport destination

CUSTOM_LIB = libgnomeprint-custom.la
CUSTOM_SRC = gp-transport-custom.c gp-transport-custom.h

libgnomeprint_custom_la_SOURCES = $(CUSTOM_SRC)

libgnomeprint_custom_la_LDFLAGS = -avoid-version -module -no-undefined

libgnomeprint_custom_la_LIBADD = ../libgnomeprint-2-2.la \
	$(LIBGNOMEPRINT_LIBS)

EXTRA_DIST = makefile.msc
