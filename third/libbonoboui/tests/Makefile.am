SUBDIRS = control

noinst_PROGRAMS =		\
	test-ui			\
	test-ui-auto		\
	test-toolbar		\
	test-selector		\
	test-filesel		\
	test-focus		\
	test-sensitivity	\
	test-reparent		\
	test-events

bin_PROGRAMS = \
	test-moniker

INCLUDES =	\
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I$(top_srcdir) 				\
	-DBONOBO_UI_INTERNAL				\
	-DBONOBO_UIDIR=\""$(datadir)/gnome/ui/"\"	\
	-DBONOBO_TOPSRCDIR=\""$(top_srcdir)"\"		\
	-DG_LOG_DOMAIN=\"Bonobo\"			\
	-DVERSION=\"$(VERSION)\"			\
	$(WARN_CFLAGS)					\
	$(BONOBOUI_CFLAGS)

LDADD =	$(top_builddir)/bonobo/libbonoboui-2.la $(BONOBOUI_LIBS)

test_ui_SOURCES = test-ui.c
test_ui_auto_SOURCES = test-ui-auto.c
test_focus_SOURCES = test-focus.c
test_toolbar_SOURCES = test-toolbar.c
test_selector_SOURCES =	test-selector.c
test_sensitivity_SOURCES = test-sensitivity.c
test_reparent_SOURCES = test-reparent.c
test_moniker_SOURCES = test-moniker.c
test_filesel_SOURCE = test-filesel.c

TESTS = test-ui-auto
