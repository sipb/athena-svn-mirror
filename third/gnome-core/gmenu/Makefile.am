INCLUDES = 			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DGNOME_DATADIR=\""$(datadir)"\"

bin_PROGRAMS = gmenu

gmenu_SOURCES =	\
	desktop.c		\
	edit.c			\
	gmenu.h			\
	main.c			\
	order.c			\
	save.c			\
	tree.c			\
	treedel.c		\
	treednd.c		\
	treenew.c		\
	treeutil.c		\
	utils.c

gmenu_LDADD = \
	$(GNOME_LIBDIR)					\
	$(GNOMEUI_LIBS)					\
	$(INTLLIBS)

EXTRA_DIST = gmenu.desktop top.xpm unknown.xpm

sysdir = $(datadir)/gnome/apps/Settings
sys_DATA = gmenu.desktop
