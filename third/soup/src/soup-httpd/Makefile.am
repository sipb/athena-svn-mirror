## Process this file with automake to produce Makefile.in

@SET_MAKE@

INCLUDES = 				\
	-DG_LOG_DOMAIN=\"SOUP-HTTPD\" 	\
	-DSYSCONFDIR=\"$(sysconfdir)\" 	\
	-I$(top_srcdir)/src		\
	$(SOUP_DEBUG_FLAGS)		\
	$(GLIB_CFLAGS)			\
	$(POPT_CFLAGS)			\
	$(XML_CFLAGS)

bin_PROGRAMS = soup-httpd

soup_httpd_LDADD =				\
	$(GMODULE_LIBS)				\
	$(POPT_LIBS)				\
	$(top_builddir)/src/libsoup/libsoup.la

soup_httpd_SOURCES = soup-httpd.c

EXTRA_DIST = 

