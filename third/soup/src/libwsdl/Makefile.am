## Process this file with automake to produce Makefile.in

@SET_MAKE@

INCLUDES = 					\
	-DG_LOG_DOMAIN=\"SOUP-WSDL-RUNTIME\" 	\
	-I$(top_srcdir)/src			\
	$(SOUP_DEBUG_FLAGS)			\
	$(GLIB_CFLAGS)				\
	$(XML_CFLAGS)

libwsdlincludedir = $(includedir)/soup/libwsdl

libwsdlinclude_HEADERS =	\
	wsdl.h			\
	wsdl-typecodes.h	\
	wsdl-schema.h		\
	wsdl-schema-glib.h	\
	wsdl-soap-parse.h	\
	wsdl-soap-marshal.h	\
	wsdl-soap-memory.h	\
	wsdl-param.h

lib_LTLIBRARIES = libwsdl.la 

libwsdl_la_LDFLAGS = -version-info $(SOUP_CURRENT):$(SOUP_REVISION):$(SOUP_AGE)

libwsdl_la_LIBADD =				\
	$(GLIB_LIBS)				\
	$(XML_LIBS)				\
	$(top_builddir)/src/libsoup/libsoup.la

libwsdl_la_SOURCES =		\
	wsdl.h			\
	wsdl-param.h		\
	wsdl-typecodes.c	\
	wsdl-typecodes.h	\
	wsdl-schema.c		\
	wsdl-schema.h		\
	wsdl-schema-glib.c	\
	wsdl-schema-glib.h	\
	wsdl-soap-parse.c	\
	wsdl-soap-parse.h	\
	wsdl-soap-marshal.c	\
	wsdl-soap-marshal.h	\
	wsdl-soap-memory.c	\
	wsdl-soap-memory.h

noinst_LIBRARIES = libwsdl-build.a

libwsdl_build_a_SOURCES =	\
	wsdl-typecodes.c	\
	wsdl-typecodes.h	\
	wsdl-schema.c		\
	wsdl-schema.h		\
	wsdl-schema-glib.c	\
	wsdl-schema-glib.h	\
	wsdl-typecodes-c.c	\
	wsdl-typecodes-c.h

EXTRA_DIST = 

