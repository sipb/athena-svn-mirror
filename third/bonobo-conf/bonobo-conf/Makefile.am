INCLUDES =                                              \
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
        -I.                                             \
        -I$(srcdir) -I$(top_srcdir)                     \
        -I$(top_builddir)                               \
        -I$(includedir)                                 \
        $(BONOBO_CONF_CFLAGS)

CORBA_SOURCE =			\
	Bonobo_Config.h		\
	Bonobo_Config-common.c	\
	Bonobo_Config-stubs.c	\
	Bonobo_Config-skels.c

idl_flags = `gnome-config --cflags idl`

$(CORBA_SOURCE): ../idl/Bonobo_Config.idl
	orbit-idl $(top_srcdir)/idl/Bonobo_Config.idl -I$(top_srcdir)/idl $(idl_flags)

noinst_PROGRAMS = test-config-moniker config-moniker-demo wombat-example

lib_LTLIBRARIES = libbonobo_conf.la

libbonobo_confincludedir = $(includedir)/bonobo-conf
libbonobo_confinclude_HEADERS =	 		\
	Bonobo_Config.h				\
	bonobo-config-database.h		\
	bonobo-config-subdir.h			\
	bonobo-property-frame.h			\
	bonobo-property-bag-proxy.h		\
	bonobo-property-editor.h		\
	bonobo-config-utils.h			\
	bonobo-preferences.h			\
	bonobo-config-control.h

libbonobo_conf_la_SOURCES =	 		\
	$(CORBA_SOURCE)				\
	gtkwtreeitem.h				\
	gtkwtreeitem.c				\
	gtkwtree.h				\
	gtkwtree.c				\
	bonobo-config-utils.h			\
	bonobo-config-utils.c			\
	bonobo-config-database.h		\
	bonobo-config-database.c		\
	bonobo-config-subdir.h			\
	bonobo-config-subdir.c			\
	bonobo-config-bag.h			\
	bonobo-config-bag.c			\
	bonobo-config-bag-property.h		\
	bonobo-config-bag-property.c		\
	bonobo-config-property.h		\
	bonobo-config-property.c		\
	bonobo-subproperty.h			\
	bonobo-subproperty.c			\
	bonobo-property-proxy.h			\
	bonobo-property-proxy.c			\
	bonobo-property-bag-proxy.h		\
	bonobo-property-bag-proxy.c		\
	bonobo-property-editor.h		\
	bonobo-property-editor.c		\
	bonobo-property-bag-editor.h		\
	bonobo-property-bag-editor.c		\
	bonobo-property-editor-basic.c		\
	bonobo-property-editor-boolean.c	\
	bonobo-property-editor-filename.c	\
	bonobo-property-editor-color.c		\
	bonobo-property-editor-option.c		\
	bonobo-property-editor-default.c	\
	bonobo-property-editor-enum.c		\
	bonobo-property-frame.h			\
	bonobo-property-frame.c			\
	bonobo-preferences.h			\
	bonobo-preferences.c			\
	bonobo-config-control.h 		\
	bonobo-config-control.c	

#	bonobo-property-editor-struct.h		\
#	bonobo-property-editor-struct.c		\
#	bonobo-property-editor-list.h		\
#	bonobo-property-editor-list.c	

test_config_moniker_SOURCES = test-config-moniker.c
test_config_moniker_LDADD = libbonobo_conf.la $(BONOBO_LIBS)

config_moniker_demo_SOURCES = config-moniker-demo.c
config_moniker_demo_LDADD = libbonobo_conf.la $(BONOBO_LIBS) 

wombat_example_SOURCES = wombat-example.c
wombat_example_LDADD = libbonobo_conf.la $(BONOBO_LIBS) 

BUILT_SOURCES = $(CORBA_SOURCE)

CLEANFILES += $(BUILT_SOURCES)

dist-hook:
	cd $(distdir) ; rm -f $(BUILT_SOURCES)

