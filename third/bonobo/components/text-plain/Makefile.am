bin_PROGRAMS = bonobo-text-plain

bonobo_text_plain_SOURCES =	\
	bonobo-text-plain.c

INCLUDES =						\
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I$(srcdir) -I$(top_srcdir) 			\
	-I. -I$(top_srcdir)/bonobo			\
	$(BONOBOX_TEST_CFLAGS)

bonobo_text_plain_LDADD = 			\
	$(top_builddir)/bonobo/libbonobo.la	\
	$(top_builddir)/bonobo/libbonobox.la	\
	$(BONOBOX_TEST_LIBS)			\
	$(INTLLIBS)

oafdir = $(datadir)/oaf

oaf_DATA = Bonobo_Sample_Text.oafinfo

EXTRA_DIST = $(oaf_DATA)

