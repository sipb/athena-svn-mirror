
libke_LTLIBRARIES = libke.la

INCLUDES = -I$(top_builddir)    		\
	   -I$(top_srcdir)/srutil 		\
	   -I$(top_srcdir)/srconf/libsrconf 	\
	   $(LIBKE_CFLAGS)			\
	   -DG_LOG_DOMAIN=\"gnopernicus\"

AM_LDFLAGS = $(LIBKE_LIBS) @LT_VERSION_INFO@

LDADD = ../../srutil/srutil.la ../../srconf/libsrconf/libsrconf.la $(LIBKE_LIBS)  $(X_LIBS)

libkeincludedir =$(includedir)/gnopernicus-1.0/libke
libkedir=$(libdir)/gnopernicus-1.0

libkeinclude_HEADERS = libke.h     

libke_la_SOURCES = libke.c       \
                   libke.h       



