NULL=

noinst_PROGRAMS = tester remote

tester_SOURCES = tester.c	\
		 brlinp.c 	\
		 brlinp.h

remote_SOURCES = remote.c	\
		 brlinp.c	\
		 brlinp.h 	\
		 remoteinit.c	\
		 remoteinit.h

INCLUDES = -I$(srcdir)/../libbrl		\
	   -I$(top_srcdir)/srutil		\
	   -I$(top_srcdir)/srconf/libsrconf 	\
	   -I$(srcdir)				\
	   $(INET6_ACTIVE) -DGNOPERNICUSLOCALEDIR=\""$(gnopernicuslocaledir)"\" \
	   $(LIBBRL_TEST_CFLAGS)		\
	   -DG_LOG_DOMAIN=\"gnopernicus\"

LDADD =  ../libbrl/libbrl.la			\
	 ../../srutil/libsrutil.la		\
	 ../../srconf/libsrconf/libsrconf.la	\
	 $(LIBBRL_TEST_LIBS)			\
	 $(BRLTTY_LIBRARY)

EXTRA_DIST = brl_out_test.xml
