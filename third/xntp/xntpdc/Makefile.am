#AUTOMAKE_OPTIONS = no-dependencies
bin_PROGRAMS = xntpdc
INCLUDES = -I$(top_srcdir)/include
# LDADD might need RESLIB and ADJLIB
LDADD =	version.o ../libntp/libntp.a
DISTCLEANFILES = .version version.c
noinst_HEADERS = ntpdc.h
#EXTRA_DIST = xntpdc.mak README TAGS
EXTRA_DIST = xntpdc.mak
ETAGS_ARGS = Makefile.am

xntpdc_SOURCES = ntpdc.c ntpdc_ops.c

$(PROGRAMS): $(LDADD)

../libntp/libntp.a:
	cd ../libntp && $(MAKE)

version.o: $(xntpdc_OBJECTS) ../libntp/libntp.a Makefile
	$(top_srcdir)/scripts/mkversion xntpdc @VERSION@
	$(COMPILE) -c version.c
