libmag_LTLIBRARIES = libmag.la

INCLUDES = -I$(top_builddir)         		\
	   -I$(top_srcdir)/srconf/libsrconf  	\
	   -I$(top_srcdir)/srutil	     	\
	   $(LIBMAG_CFLAGS)			\
	   -DG_LOG_DOMAIN=\"gnopernicus\"

AM_LDFLAGS =$(LIBMAG_LIBS) @LT_VERSION_INFO@

LDADD = ../../srconf/libsrconf/libsrconf.la $(LIBMAG_LIBS)

libmagincludedir =$(includedir)/gnopernicus-1.0/libmag
libmagdir=$(libdir)/gnopernicus-1.0

libmaginclude_HEADERS = magxmlapi.h     

if BUILD_MAGNIFIER
libmag_la_SOURCES = magxmlapi.h     	\
		    magxml.h		\
		    magxml.c		\
		    mag.h		\
		    mag.c		\
		    mag_ctrl.h		\
		    mag_ctrl.c
EXTRA_libmag_la_SOURCES = magxmlapi.h     	\
			  magxmlapi.c

else
libmag_la_SOURCES = magxmlapi.h     	\
		    magxmlapi.c
EXTRA_libmag_la_SOURCES = magxmlapi.h     	\
		    magxml.h		\
		    magxml.c		\
		    mag.h		\
		    mag.c		\
		    mag_ctrl.h		\
		    mag_ctrl.c
endif

