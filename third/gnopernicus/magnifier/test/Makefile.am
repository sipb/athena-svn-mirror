if BUILD_MAGNIFIER
noinst_PROGRAMS = tester 
endif
INCLUDES = -I$(srcdir)				\
	   -I$(srcdir)/../libmag  		\
	   -I$(top_srcdir)/srutil 		\
	   -I$(top_srcdir)/srconf/libsrconf  	\
	   $(LIBMAG_CFLAGS)			\
	   -DG_LOG_DOMAIN=\"gnopernicus\"

tester_SOURCES = tester.c

LDADD = ../libmag/libmag.la ../../srutil/libsrutil.la ../../srconf/libsrconf/libsrconf.la
