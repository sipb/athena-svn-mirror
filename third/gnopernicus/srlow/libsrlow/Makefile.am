
libsrlow_LTLIBRARIES = libsrlow.la

INCLUDES = -I$(top_builddir)         		\
	   -I$(top_srcdir)/srutil            	\
	   $(LIBSRLOW_CFLAGS)			\
	   -DG_LOG_DOMAIN=\"gnopernicus\"

AM_LDFLAGS = $(LIBSRLOW_LIBS) @LT_VERSION_INFO@

LDADD = $(LIBSRLOW_LIBS)  

libsrlowincludedir =$(includedir)/gnopernicus-1.0/libsrlow
libsrlowdir =$(libdir)/gnopernicus-1.0

libsrlowinclude_HEADERS = SRLow.h     \
			  SRObject.h  \
			  screen-review.h

libsrlow_la_SOURCES = SRLow.c       \
		      SRLow.h       \
		      SRObject.c    \
		      SRObject.h    \
		      screen-review.c\
		      screen-review.h     



