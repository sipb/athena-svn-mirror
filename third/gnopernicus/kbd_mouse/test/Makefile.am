
INCLUDES = -I$(srcdir)				\
	   -I$(srcdir)/../libke  		\
	   -I$(top_srcdir)/srutil 		\
	   -I$(top_srcdir)/srconf/libsrconf 	\
	   $(LIBKE_CFLAGS)			\
	   -DG_LOG_DOMAIN=\"gnopernicus\"

noinst_PROGRAMS = ke_test

ke_test_SOURCES = testlibke.c

LDADD = ../libke/libke.la ../../srutil/libsrutil.la ../../srconf/libsrconf/libsrconf.la
