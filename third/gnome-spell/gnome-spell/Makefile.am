PLUGIN_DIR=$(libdir)/bonobo/plugin

gladedir = $(datadir)/gnome-spell-@GNOME_SPELL_RELEASE@
glade_DATA = spell-checker.glade

END =

INCLUDES =                                              \
        -I$(srcdir)                                     \
	$(ASPELL_INC)					\
        -DGNOMEDATADIR=\""$(datadir)"\"         	\
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-DGLADE_DATADIR=\"$(gladedir)\"			\
	-DPLUGIN_DIR=\""$(PLUGIN_DIR)"\"		\
	-DASPELL_DICT=\""$(ASPELL_DICT)"\"		\
	$(GNOME_SPELL_CFLAGS)				\
	$(END)

SPELL_IDL = Spell.idl

IDL_GENERATED = 		\
	Spell.h			\
	Spell-common.c		\
	Spell-skels.c		\
	Spell-stubs.c

EXTRA_DIST = 			\
	$(SPELL_IDL)		\
	$(glade_DATA)

CLEANFILES = $(IDL_GENERATED)

$(IDL_GENERATED): $(SPELL_IDL)
	$(ORBIT_IDL) $(BONOBO_IDL_INCLUDES) $(srcdir)/Spell.idl

Spell-impl.o: Spell.h

privlibdir = "$(libdir)/gnome-spell"
privlib_LTLIBRARIES = libgnome-spell-component.la

noinst_PROGRAMS =				\
	test-gnome-spell-component

noinst_LTLIBRARIES = libgnome-spell-idl.la

libgnome_spell_idl_la_SOURCES =	\
	$(IDL_GENERATED)	\
	$(END)

libgnome_spell_component_la_LDFLAGS = -release $(API_VERSION)
libgnome_spell_component_la_LIBADD = 		\
	libgnome-spell-idl.la		\
	$(ASPELL_LIBS)		\
	$(GNOME_SPELL_LIBS)			\
	$(END)

libgnome_spell_component_la_SOURCES =		\
	control.h				\
	control.c				\
	dictionary.h				\
	dictionary.c				\
	spell-factory.c				\
	$(END)

test_gnome_spell_component_LDADD =		\
	libgnome-spell-idl.la		\
	$(ASPELL_LIBS)				\
	$(GNOME_SPELL_LIBS)			\
	$(END)

test_gnome_spell_component_SOURCES =		\
	test-spell.c

serverdir = $(libdir)/bonobo/servers
server_in_files = GNOME_Spell.server.in.in

server_DATA = $(server_in_files:.server.in.in=.server)
$(server_in_files:.server.in.in=.server.in): $(server_in_files) Makefile
	sed -e "s|\@GNOME_SPELL_COMPONENT_LOCATION\@|$(privlibdir)|g" \
	    -e "s|\@API_VERSION\@|$(API_VERSION)|g" \
	    -e "s|\@GNOME_SPELL_RELEASE\@|$(GNOME_SPELL_RELEASE)|g" $< > $@

@INTLTOOL_SERVER_RULE@

idldir    = $(datadir)/idl
idl_DATA  = 					\
	Spell.idl				\
	$(END)

EXTRA_DIST += $(server_in_files) $(server_DATA) $(idl_DATA)

install-data-hook:
	mv $(DESTDIR)$(idldir)/Spell.idl $(DESTDIR)$(idldir)/Spell-@GNOME_SPELL_RELEASE@.idl
