## Process this file with automake to produce Makefile.in

@SET_MAKE@

INCLUDES = 				\
	-DG_LOG_DOMAIN=\"SOUP-APACHE\" 	\
	-I$(top_srcdir)/src		\
	$(SOUP_DEBUG_FLAGS)		\
	$(GLIB_CFLAGS)			\
	$(XML_CFLAGS)			\
	$(APACHE_CFLAGS)

lib_LTLIBRARIES = libsoup-apache.la

libsoup_apache_la_LDFLAGS = \
	-version-info $(SOUP_CURRENT):$(SOUP_REVISION):$(SOUP_AGE)

libsoup_apache_la_LIBADD = $(APACHE_LIBS)	

libsoup_apache_la_SOURCES = soup-apache.c

EXTRA_DIST = 

