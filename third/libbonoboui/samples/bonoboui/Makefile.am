noinst_PROGRAMS = bonobo-hello

INCLUDES =	\
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I$(top_srcdir) 				\
	-DBONOBO_UIDIR=\""$(datadir)/gnome/ui/"\"	\
	-DG_LOG_DOMAIN=\"Bonobo\"			\
	-DVERSION=\"$(VERSION)\"			\
	$(WARN_CFLAGS)					\
	$(BONOBOUI_CFLAGS)				\
	-DHELLO_SRCDIR=\""$(srcdir)"/\"

LDADD =	$(top_builddir)/bonobo/libbonoboui-2.la $(BONOBOUI_LIBS)

bonobo_hello_SOURCES =	\
	hello.c

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = Bonobo_Sample_Hello.xml

EXTRA_DIST=$(ui_DATA)