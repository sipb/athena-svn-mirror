
noinst_PROGRAMS = control-client mag-test

control_client_SOURCES = control-client.c

mag_test_SOURCES = mag-test.c

INCLUDES = \
	$(DEPS_CFLAGS)		\
	-I$(top_srcdir)		\
	-I$(top_builddir)	\
	-I$(top_builddir)/magnifier 	\
	-I$(top_srcdir)/idl	

LDADD = $(DEPS_LIBS) $(top_builddir)/magnifier/libgnome-mag.la $(X_LIBS) $(POPT_LIBS)



