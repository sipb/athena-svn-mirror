
libsrutil_LTLIBRARIES = libsrutil.la

INCLUDES = -I$(srcdir)				\
	   -I$(top_srcdir)/srconf/libsrconf	\
	   $(LIBSRUTIL_CFLAGS)			\
	   -DG_LOG_DOMAIN=\"gnopernicus\"

LDFLAGS = $(LIBSRUTIL_LIBS) @LT_VERSION_INFO@

LDADD = $(LIBSRUTIL_LIBS)  

libsrutilincludedir =$(includedir)/gnopernicus-1.0/libsrutil
libsrutildir =$(libdir)/gnopernicus-1.0

libsrutilinclude_HEADERS = SREvent.h	\
			   SRMessages.h	\
			   util.h	\
			   srintl.h

libsrutil_la_SOURCES = SREvent.c	\
		       SREvent.h	\
		       util.c		\
		       util.h		\
		       SRMessages.h	\
		       SRMessages.c
			       


