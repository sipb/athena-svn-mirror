## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libxalflaunch.la

libxalflaunch_la_SOURCES = xalflaunch.c 

libxalflaunch_la_LDFLAGS = -version-info 0:1:0

CFLAGS = $(X_CFLAGS) -DLIBDIR=\""$(libdir)"\"

##INCLUDES = -I$(top_srcdir)/intl @GDK_IMLIB_CFLAGS@ @XML_CFLAGS@ @GNOME_CFLAGS@ @GTK_CFLAGS@ -DDIA_SHEETDIR=\"$(sheetdir)\"

## INCLUDES = @GTK_CFLAGS@

## LDFLAGS = -ldl @GTK_LIBS@

if NEED_LD_INIT_OPTION
LDFLAGS = -ldl -init _init
else
LDFLAGS = -ldl
endif
