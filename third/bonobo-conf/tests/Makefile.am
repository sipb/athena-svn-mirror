INCLUDES =                                              \
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
        -I. -I$(srcdir) -I$(top_srcdir)                 \
        -I$(top_builddir)                               \
        -I$(includedir)                                 \
        $(GCONF_CFLAGS)					\
        $(BONOBO_CFLAGS)	

LDADD = $(BONOBO_LIBS) ../bonobo-conf/libbonobo_conf.la

noinst_PROGRAMS=test-moniker

test_moniker_SOURCES=test-moniker.c

if ENABLE_GCONF

GCONFTESTS = test-gconf

else

GCONFTESTS =

endif

xml_in_files = test-config.xml.in

noinst_DATA = $(xml_in_files:.xml.in=.xmldb)

%.xmldb: %.xml.in $(top_builddir)/xml-i18n-merge $(top_srcdir)/po/*.po
	$(top_builddir)/xml-i18n-merge -x $(top_srcdir)/po $< $*.xmldb

clean-local: 
	-rm $(noinst_DATA)

EXTRA_DIST = $(xml_in_files)

check_PROGRAMS = test-xml-strings #$(GCONFTESTS)

TESTS = test-xml-strings #$(GCONFTESTS)
