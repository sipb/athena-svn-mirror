libsrs_LTLIBRARIES = libsrs.la

INCLUDES = -I$(top_builddir)  			\
	    -I$(top_srcdir)/srutil		\
	    -I$(top_srcdir)/srconf/libsrconf 	\
	   $(LIBSRS_CFLAGS)			\
	   -DG_LOG_DOMAIN=\"gnopernicus\"	\
	   -DSRS_NO_MARKERS_SUPPORTED 		\
	   -DSRS_ENABLE_OPTIMIZATION

AM_LDFLAGS = $(LIBSRS_LIBS) @LT_VERSION_INFO@

LDADD = ../../srconf/libsrconf/libsrconf.la $(LIBSRS_LIBS)

libsrsincludedir =$(includedir)/gnopernicus-1.0/libsrs
libsrsdir=$(libdir)/gnopernicus-1.0

libsrsinclude_HEADERS = srs-xml.h

libsrs_la_SOURCES = srs-gs.h		\
		    srs-gs.c		\
		    srs-gs-wrap.h	\
		    srs-gs-wrap.c	\
		    srs-speech.h  	\
		    srs-speech.c	\
		    srs-xml.h		\
		    srs-xml.c		\
		    spgscb.h		\
		    spgscb.c

