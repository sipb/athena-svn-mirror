## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_builddir)				\
	-I$(top_srcdir)					\
	-DG_LOG_DOMAIN=\"BonoboUI\"			\
	$(BONOBOUI_GLADE_CFLAGS)

glademoduledir = $(libdir)/libglade/2.0
glademodule_LTLIBRARIES = libbonobo.la

libbonobo_la_LIBADD = \
	$(top_builddir)/bonobo/libbonoboui-2.la	\
	$(BONOBOUI_GLADE_LIBS)

libbonobo_la_LDFLAGS = \
	-export-dynamic -module -avoid-version -no-undefined

libbonobo_la_SOURCES = \
	glade-bonobo.c
