INCLUDES =                                              \
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-DDATADIR=\""$(datadir)"\"			\
        -I.                                             \
        -I$(srcdir) -I$(top_srcdir)                     \
        -I$(top_builddir)                               \
        -I$(includedir)                                 \
        $(GCONF_CFLAGS)					\
        $(BONOBO_CFLAGS)	

OAF_FILES = 						\
	Bonobo_Moniker_config.oaf			\
	Bonobo_Moniker_xmldb.oaf			\
	Bonobo_Moniker_gconf.oaf			

oafdir = $(datadir)/oaf
oaf_DATA = $(OAF_FILES)

Bonobo_Moniker_config.oaf : $(srcdir)/Bonobo_Moniker_config.oaf.in $(top_builddir)/config.status
	sed -e "s|\@MONIKER_LIBDIR\@|$(monikerdir)|" \
	$(srcdir)/Bonobo_Moniker_config.oaf.in > Bonobo_Moniker_config.oaf 
Bonobo_Moniker_xmldb.oaf : $(srcdir)/Bonobo_Moniker_xmldb.oaf.in $(top_builddir)/config.status
	sed -e "s|\@MONIKER_LIBDIR\@|$(monikerdir)|" \
	$(srcdir)/Bonobo_Moniker_xmldb.oaf.in > Bonobo_Moniker_xmldb.oaf 
Bonobo_Moniker_gconf.oaf : $(srcdir)/Bonobo_Moniker_gconf.oaf.in $(top_builddir)/config.status
	sed -e "s|\@MONIKER_LIBDIR\@|$(monikerdir)|" \
	$(srcdir)/Bonobo_Moniker_gconf.oaf.in > Bonobo_Moniker_gconf.oaf 

clean-local:
	-rm -f $(OAF_FILES)

bin_PROGRAMS = bonobo-moniker-xmldb

bonobo_moniker_xmldb_SOURCES =			\
	bonobo-config-xmldb.c			\
	bonobo-config-xmldb.h			\
	bonobo-config-dirdb.c			\
	bonobo-config-dirdb.h			\
	bonobo-moniker-xmldb.c

bonobo_moniker_xmldb_LDADD = $(BONOBO_LIBS) ../bonobo-conf/libbonobo_conf.la

monikerdir = $(libdir)/bonobo/monikers

if ENABLE_GCONF

GCONF_LTLIB = libmoniker_gconf.la

libmoniker_gconf_la_SOURCES = 			\
	bonobo-moniker-gconf.c			\
	bonobo-config-gconfdb.h			\
	bonobo-config-gconfdb.c			

libmoniker_gconf_la_LIBADD = $(GCONF_LIBS) 			

else

GCONF_LTLIB=

endif

moniker_LTLIBRARIES = libmoniker_config.la $(GCONF_LTLIB)

libmoniker_config_la_SOURCES =	 		\
	bonobo-moniker-config.c 		

libmoniker_config_la_LIBADD = $(BONOBO_LIBS) ../bonobo-conf/libbonobo_conf.la

noinst_PROGRAMS = test-xmldb

test_xmldb_SOURCES = 			\
	test-xmldb.c 			\
	bonobo-config-xmldb.h 		\
	bonobo-config-xmldb.c		\
	bonobo-config-dirdb.h 		\
	bonobo-config-dirdb.c

test_xmldb_LDADD = $(BONOBO_LIBS) ../bonobo-conf/libbonobo_conf.la

EXTRA_DIST = 					\
	Bonobo_Moniker_config.oaf.in		\
	Bonobo_Moniker_xmldb.oaf.in		\
	Bonobo_Moniker_gconf.oaf.in

