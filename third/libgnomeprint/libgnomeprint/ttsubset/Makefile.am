noinst_LTLIBRARIES = libttsubset.la

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libgnomeprint \
	$(LIBGNOMEPRINT_CFLAGS) \
	-DBINDIR=\""$(bindir)"\" \
	-DDATADIR=\""$(datadir)"\" \
     -DGPA_MODULES_DIR=\""$(GNOME_PRINT_MODULES_DIR)"\" \
     -DGPA_DATA_DIR=\""$(GNOME_PRINT_DATA_DIR)"\" \
	\
	-DNO_MAPPERS -DNO_TYPE3	\
	-DG_DISABLE_DEPRECATED				\
	-DGDK_DISABLE_DEPRECATED				\
	-DGTK_DISABLE_DEPRECATED				\
	-DGDK_PIXBUF_DISABLE_DEPRECATED		\
	-DGNOME_DISABLE_DEPRECATED

libttsubset_la_SOURCES = \
	sft.c sft.h \
	list.c list.h \
	ttcr.c ttcr.h \
	crc32.c crc32.h \
	gnome-print-tt-subset.c gnome-print-tt-subset.h

EXTRA_DIST = makefile.msc
