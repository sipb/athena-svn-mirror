INCLUDES =                                              \
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
        -I.                                             \
        -I$(srcdir) -I$(top_srcdir)                     \
        -I$(top_builddir)                               \
        -I$(includedir)                                 \
        $(BONOBO_CFLAGS)				


bin_PROGRAMS = cedit


cedit_SOURCES = cedit.c
cedit_LDADD = $(BONOBO_LIBS) 
