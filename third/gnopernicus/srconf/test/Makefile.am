
INCLUDES = -I$(srcdir)/../libsrconf 		\
	   -I$(top_srcdir)/srutil 		\
	   $(LIBSRCONF_TEST_CFLAGS)		\
	   -DG_LOG_DOMAIN=\"gnopernicus\"

noinst_PROGRAMS = test testlibsrconf

test_SOURCES = test.c

testlibsrconf_SOURCES = testlibsrconf.c

LDADD = ../libsrconf/libsrconf.la ../../srutil/libsrutil.la $(LIBSRCONF_TEST_LIBS)
