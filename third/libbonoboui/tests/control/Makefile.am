noinst_PROGRAMS =		\
	test-control

INCLUDES =	\
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I$(top_srcdir) 				\
	-DBONOBO_UIDIR=\""$(datadir)/gnome/ui/"\"	\
	-DG_LOG_DOMAIN=\"Bonobo\"			\
	-DVERSION=\"$(VERSION)\"			\
	$(WARN_CFLAGS)					\
	$(BONOBOUI_CFLAGS)

LDADD =	$(top_builddir)/bonobo/libbonoboui-2.la $(BONOBOUI_LIBS)

test_control_SOURCES = test-control.c

