# WHAT YOU NEED TO CHANGE: Change all the filenames to refer to your
# component.  Also add whatever libraries, include paths, etc you want,
# and change the G_LOG_DOMAIN.

INCLUDES =						\
	-DG_DISABLE_DEPRECATED				\
	-DGTK_DISABLE_DEPRECATED			\
	-DGNOME_DISABLE_DEPRECATED			\
	-DG_LOG_DOMAIN=\"Nautilus-Test\"		\
	-DDATADIR=\"$(datadir)\"			\
	-I$(top_srcdir)					\
	-I$(top_builddir)				\
	$(DISABLE_DEPRECATED_CFLAGS)			\
	$(NAUTILUS_CFLAGS)

libexec_PROGRAMS = nautilus-test-view

nautilus_test_view_SOURCES =				\
	nautilus-test-view.h          			\
	nautilus-test-view.c          			\
	main.c

nautilus_test_view_LDADD =				\
	$(NAUTILUS_LIBS)

noinst_PROGRAMS = test

test_SOURCES =						\
	nautilus-test-view.h          			\
	nautilus-test-view.c          			\
	test.c

test_LDADD =						\
	$(NAUTILUS_LIBS)

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = nautilus-test-view-ui.xml

server_in_files = Nautilus_View_test.server.in.in

serverdir = $(libdir)/bonobo/servers
server_DATA = $(server_in_files:.server.in.in=.server)

@INTLTOOL_SERVER_RULE@

EXTRA_DIST = $(server_in_files) $(ui_DATA) README
CLEANFILES = $(server_DATA) $(server_DATA).in
