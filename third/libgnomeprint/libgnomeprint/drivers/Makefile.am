OMNI2_LIB = libgnomeprint-omni2.la
OMNI2_SRC = gnome-print-omni2.cpp gnome-print-omni2.hpp
OMNI2_LAD = -L$(OMNI_LIBDIR) -lomni
libgnomeprint_omni2_la_SOURCES = $(OMNI2_SRC)
libgnomeprint_omni2_la_LDFLAGS = -version-info 0:0:0 -module
libgnomeprint_omni2_la_LIBADD = $(OMNI2_LAD)

FAX_LIB = libgnomeprint-fax.la
FAX_SRC = gnome-print-fax.c gnome-print-fax.h gnome-print-fax-g3.h
libgnomeprint_fax_la_SOURCES = $(FAX_SRC)
libgnomeprint_fax_la_LDFLAGS =  -version-info 0:0:0 -module
libgnomeprint_fax_la_LIBADD =

libexecdir = $(libdir)/gnome-print-2.0/drivers

if HAVE_OMNI
omni_libraries = $(OMNI2_LIB)
omni_sources = $(OMNI2_SRC)
omni_libadd = $(OMNI2_LAD)
else
omni_libraries =
omni_sources =
omni_libadd =
endif

libexec_LTLIBRARIES = \
	$(FAX_LIB) \
	$(omni_libraries)
noinst_LTLIBRARIES =

CPPFLAGS = \
	-DWE_ARE_LIBGNOMEPRINT_UI \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" 	\
	-DBINDIR=\""$(bindir)"\"	\
	-DDATADIR=\""$(datadir)"\" 

INCLUDES = \
	-I$(top_srcdir) \
	$(OMNI_CFLAGS) \
	$(LIBGNOMEPRINT_CFLAGS)

