
INCLUDES = -I$(srcdir)				\
	   -I$(srcdir)/../libsrlow		\
	   -I$(top_srcdir)/srutil		\
	   $(LIBSRLOW_TEST_CFLAGS)		\
	   -DG_LOG_DOMAIN=\"gnopernicus\"


noinst_PROGRAMS = srlow_test gtk_srlow_test

srlow_test_SOURCES = srlow_test.c SRTest.c SRTest.h

gtk_srlow_test_SOURCES =gtk_srlow_test.c SRTest.c SRTest.h

LDADD =  ../libsrlow/libsrlow.la  ../../srutil/libsrutil.la 
