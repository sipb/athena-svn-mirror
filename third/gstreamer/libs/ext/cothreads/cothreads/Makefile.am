# we're noinst until we're sure that we won't bork someone's installation

if USE_GTHREADS
GTHREADS_LIBS = libcothreads-gthreads.la
else
GTHREADS_LIBS =
endif

noinst_LTLIBRARIES = libcothreads.la $(GTHREADS_LIBS)

COTHREADS_FILES = cothreads.c pth_mctx.c

# the -I is necessary for non-srcdir builds
libcothreads_la_SOURCES = $(COTHREADS_FILES)
libcothreads_la_LIBADD = $(COTHREADS_LDFLAGS) $(GLIB_LIBS)
libcothreads_la_CFLAGS = $(COTHREADS_CFLAGS) $(GLIB_CFLAGS) \
                         -I$(top_builddir)

libcothreads_gthreads_la_SOURCES = $(COTHREADS_FILES)
libcothreads_gthreads_la_LIBADD = $(COTHREADS_LDFLAGS) $(GTHREAD_LIBS)
libcothreads_gthreads_la_CFLAGS = $(COTHREADS_CFLAGS) $(GTHREAD_CFLAGS) \
                                  -I$(top_builddir)

noinst_HEADERS = cothreads-private.h cothreads.h pth_p.h \
	config-public.h config-private.h cothreads-pth.h \
        cothreads-pth.h cothreads-pth-private.h
