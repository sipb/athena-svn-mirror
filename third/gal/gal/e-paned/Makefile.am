INCLUDES =					\
	-I$(top_srcdir)				\
	$(GNOME_INCLUDEDIR)			\
	-DG_LOG_DOMAIN=\"e-paned\"

noinst_LTLIBRARIES = libepaned.la

# WARNING
# WARNING Be sure to add any files here into the parent Makefile too.
# WARNING
libepaned_la_SOURCES = 		\
	e-hpaned.c		\
	e-paned.c		\
	e-vpaned.c

libepanedincludedir = $(includedir)/gal/e-paned

libepanedinclude_HEADERS = 			\
	e-hpaned.h		\
	e-paned.h		\
	e-vpaned.h
