## Process this file with automake to produce Makefile.in

@SET_MAKE@

INCLUDES = 					\
	-DG_LOG_DOMAIN=\"SOUP-SSL-PROXY\"	\
	-I$(top_srcdir)/src			\
	$(SOUP_DEBUG_FLAGS)			\
	$(GLIB_CFLAGS)				\
	$(OPENSSL_CFLAGS)

noinst_PROGRAMS = soup-ssl-proxy

soup_ssl_proxy_LDADD =		\
	$(LINK_STATIC)		\
	$(OPENSSL_LIBS)		\
	$(GLIB_LIBS)		\
	$(LINK_DYNAMIC)

soup_ssl_proxy_SOURCES =	\
	soup-openssl.h		\
	soup-openssl.c		\
	soup-ssl-proxy.c

LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $(rcd_LDFLAGS) $(rcd_OBJECTS) $(rcd_LDADD) $(LIBS)

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) soup-ssl-proxy $(DESTDIR)$(bindir)/$(SSL_PROXY_NAME)

EXTRA_DIST = 

