## Process this file with automake to produce Makefile.in

@SET_MAKE@

bin_PROGRAMS = soup-wsdl

INCLUDES = 				\
	-DG_LOG_DOMAIN=\"SOUP-WSDL\" 	\
	-I$(top_srcdir)/src		\
	$(SOUP_DEBUG_FLAGS) 		\
	$(GLIB_CFLAGS) 			\
	$(POPT_CFLAGS) 			\
	$(XML_CFLAGS)

soup_wsdl_LDADD =					\
	$(GLIB_LIBS)					\
	$(POPT_LIBS)					\
	$(XML_LIBS)					\
	$(top_builddir)/src/libsoup/libsoup.la		\
	$(top_builddir)/src/libwsdl/libwsdl-build.a

soup_wsdl_SOURCES = 		\
	main.c 			\
	wsdl.h			\
	wsdl-parse.c 		\
	wsdl-parse.h 		\
	wsdl-describe.c		\
	wsdl-describe.h		\
	wsdl-soap-emit.c	\
	wsdl-soap-emit.h	\
	wsdl-soap-stubs.c	\
	wsdl-soap-stubs.h 	\
	wsdl-soap-skels.c 	\
	wsdl-soap-skels.h 	\
	wsdl-soap-common.c	\
	wsdl-soap-common.h 	\
	wsdl-soap-headers.c 	\
	wsdl-soap-headers.h 	\
	wsdl-memory.c 		\
	wsdl-memory.h 		\
	wsdl-trace.c 		\
	wsdl-trace.h 		\
	wsdl-locate.c 		\
	wsdl-locate.h		\
	wsdl-thread.c		\
	wsdl-thread.h

soup_wsdl_LDFLAGS =
soup_wsdl_DEPENDENCIES =
