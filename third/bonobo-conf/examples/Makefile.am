INCLUDES =                                              \
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
        -I.                                             \
        -I$(srcdir) -I$(top_srcdir)                     \
        -I$(top_builddir)                               \
        -I$(includedir)                                 \
        $(BONOBO_CFLAGS)	

noinst_PROGRAMS = evolution-calendar-config simple

evolution_calendar_config_SOURCES =			\
	evolution-calendar-config.c

evolution_calendar_config_LDADD = 			\
	$(BONOBO_LIBS) 					\
	../bonobo-conf/libbonobo_conf.la

simple_SOURCES = simple.c

simple_LDADD =			 			\
	$(BONOBO_LIBS) 					\
	../bonobo-conf/libbonobo_conf.la
