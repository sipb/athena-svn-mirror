libexecdir = $(libdir)/gnome-print-2.0/transports
libexec_LTLIBRARIES = \
	$(FILE_LIB) \
	$(LPR_LIB) \
	$(CUSTOM_LIB)

CPPFLAGS = \
	-DWE_ARE_LIBGNOMEPRINT_UI \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" 	\
	-DBINDIR=\""$(bindir)"\"	\
	-DDATADIR=\""$(datadir)"\" 

INCLUDES = \
	-I$(top_srcdir) \
	$(LIBGNOMEPRINT_CFLAGS)

# FILE transport destination

FILE_LIB = libgnomeprint-file.la
FILE_SRC = gp-transport-file.c gp-transport-file.h

libgnomeprint_file_la_SOURCES = $(FILE_SRC)

libgnomeprint_file_la_LDFLAGS = -version-info 0:0:0 -module

libgnomeprint_file_la_LIBADD =

# LPR transport destination

LPR_LIB = libgnomeprint-lpr.la
LPR_SRC = gp-transport-lpr.c gp-transport-lpr.h

libgnomeprint_lpr_la_SOURCES = $(LPR_SRC)

libgnomeprint_lpr_la_LDFLAGS = -version-info 0:0:0 -module

libgnomeprint_lpr_la_LIBADD =

# Custom transport destination

CUSTOM_LIB = libgnomeprint-custom.la
CUSTOM_SRC = gp-transport-custom.c gp-transport-custom.h

libgnomeprint_custom_la_SOURCES = $(CUSTOM_SRC)

libgnomeprint_custom_la_LDFLAGS = -version-info 0:0:0 -module

libgnomeprint_custom_la_LIBADD =

