bin_PROGRAMS = bonobo-selector

bonobo_selector_SOURCES =	\
	bonobo-selector.c

INCLUDES =						\
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I$(srcdir) -I$(top_srcdir) 			\
	-I. -I$(top_srcdir)/bonobo			\
	$(BONOBOX_TEST_CFLAGS)

bonobo_selector_LDADD = 			\
	$(top_builddir)/bonobo/libbonobo.la	\
	$(top_builddir)/bonobo/libbonobox.la	\
	$(BONOBOX_TEST_LIBS)			\
	$(INTLLIBS)

oafdir = $(datadir)/oaf
oaf_DATA = Bonobo_Selector.oafinfo

EXTRA_DIST = $(oaf_DATA)

