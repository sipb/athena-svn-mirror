#AUTOMAKE_OPTIONS = no-dependencies
bin_PROGRAMS = ntptrace
INCLUDES = -I$(top_srcdir)/include
# LDADD might need RESLIB and ADJLIB
LDADD =	version.o ../libntp/libntp.a
DISTCLEANFILES = .version version.c
noinst_HEADERS = ntptrace.h
#EXTRA_DIST = ntptrace.mak README TAGS save
EXTRA_DIST = ntptrace.mak
ETAGS_ARGS = Makefile.am

$(PROGRAMS): $(LDADD)

../libntp/libntp.a:
	cd ../libntp && $(MAKE)

version.o: $(ntptrace_OBJECTS) ../libntp/libntp.a Makefile
	$(top_srcdir)/scripts/mkversion ntptrace @VERSION@
	$(COMPILE) -c version.c
