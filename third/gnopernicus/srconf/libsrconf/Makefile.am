NULL=

libsrconf_LTLIBRARIES = libsrconf.la

INCLUDES = -I$(top_builddir)         		\
	    -I$(top_srcdir)/srutil      	\
	   $(INET6_ACTIVE) 			\
	   $(LIBSRCONF_CFLAGS)			\
	   -DG_LOG_DOMAIN=\"gnopernicus\"

AM_LDFLAGS = $(LIBSRCONF_LIBS) @LT_VERSION_INFO@

LDADD = ../../srutil/libsrutil.la $(LIBSRCONF_LIBS)

libsrconfincludedir =$(includedir)/gnopernicus-1.0/libsrconf
libsrconfdir=$(libdir)/gnopernicus-1.0

libsrconfinclude_HEADERS = libsrconf.h

libsrconf_la_SOURCES = libsrconf.c \
		       libsrconf.h
