noinst_LTLIBRARIES = libgpa.la

CPPFLAGS = \
	-DWE_ARE_LIBGNOMEPRINT_UI \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DBINDIR=\""$(bindir)"\" \
	-DDATADIR=\""$(datadir)"\" 

INCLUDES = \
	-I$(top_srcdir) \
	$(LIBGNOMEPRINT_CFLAGS)

libgpa_la_SOURCES = \
	gpa-node.c gpa-node.h gpa-node-private.h \
	gpa-value.c gpa-value.h \
	gpa-list.c gpa-list.h \
	gpa-reference.c gpa-reference.h \
	\
	gpa-vendor.c gpa-vendor.h \
	gpa-model.c gpa-model.h \
	gpa-option.c gpa-option.h \
	gpa-printer.c gpa-printer.h \
	gpa-media.c gpa-media.h \
	\
	gpa-root.c gpa-root.h \
	gpa-config.c gpa-config.h \
	gpa-key.c gpa-key.h \
	gpa-settings.c gpa-settings.h \
	\
	gpa-utils.c gpa-utils.h

libgpaincdir = $(includedir)/libgnomeprint-2.0/libgnomeprint/private

libgpainc_HEADERS = gpa-private.h
