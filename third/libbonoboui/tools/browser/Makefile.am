INCLUDES =	\
	-DBONOBO_BROWSER_DATADIR=\""$(datadir)"\"	\
        -DBONOBO_SUPPORT_LOCALEDIR=\""$(datadir)/locale"\"        \
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_srcdir)/bonobo-browser			\
	$(BONOBOUI_CFLAGS)				\
	$(WARN_CFLAGS)

bin_PROGRAMS = bonobo-browser

bonobo_browser_SOURCES = 		\
	bonobo-browser.c		\
	bonobo-browser.h		\
	component-info.c		\
	component-list.c		\
	component-list.h		\
	component-details.c		\
	component-details.h		\
	oaf-helper.c			\
	oaf-helper.h			\
	window.c

bonobo_browser_LDADD = \
	$(BONOBOUI_LIBS)				\
	$(top_builddir)/bonobo/libbonoboui-2.la

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = bonobo-browser.xml

Utilitiesdir = $(datadir)/applications

Utilities_in_files = bonobo-browser.desktop.in
Utilities_DATA = $(Utilities_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
        $(Utilities_in_files) 	\
        $(Utilities_DATA)	\
	bonobo-browser.xml

CLEANFILES = bonobo-browser.desktop

