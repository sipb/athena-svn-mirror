if USE_GTHREADS
GTHREAD_PROGS = cothreads-gthreads
else
GTHREAD_PROGS =
endif

TESTS = cothreads dynamic ## $(GTHREAD_PROGS)
noinst_PROGRAMS = pth cothreads dynamic $(GTHREAD_PROGS)

# the two includes are necessary to get both our .h files and the generated
# ones
pth_CFLAGS = $(GLIB_CFLAGS) -I$(top_srcdir) -I$(top_builddir)
pth_LDADD = ../cothreads/libcothreads.la

cothreads_CFLAGS = $(GLIB_CFLAGS) -I$(top_srcdir) -I$(top_builddir)
cothreads_LDADD = ../cothreads/libcothreads.la

dynamic_CFLAGS = $(GLIB_CFLAGS) -I$(top_srcdir) -I$(top_builddir)
dynamic_LDADD = ../cothreads/libcothreads.la

cothreads_gthreads_SOURCES = cothreads.c
cothreads_gthreads_CFLAGS = $(GTHREAD_CFLAGS) -I$(top_srcdir) -I$(top_builddir)
cothreads_gthreads_LDADD = ../cothreads/libcothreads-gthreads.la $(GTHREAD_LIBS)
