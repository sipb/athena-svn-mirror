## Process this file with automake to produce Makefile.in.

# Documentation
SUBDIRS = C de da es it no

INCLUDES = 			\
	-I$(includedir)		\
	-I$(top_srcdir)/intl	\
	-I$(top_builddir)/intl	\
	$(GNOME_INCLUDEDIR)	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale\""

LDADD = \
	$(GNOME_LIBDIR)					\
	$(GNORBA_LIBS)					\
	$(ORB_LIBS)					\
	$(GNOMEUI_LIBS)					\
	$(ZVT_LIBS)					\
	$(INTLLIBS)


bin_PROGRAMS = gnome-terminal

EXTRA_DIST = gnome-terminal.desktop gnome-terminal.gnorba

CORBA_GENERATED =		\
	Terminal.h		\
	Terminal-stubs.c	\
	Terminal-skels.c	\
	Terminal-common.c

CLEANFILES += $(CORBA_GENERATED)
BUILT_SOURCES = $(CORBA_GENERATED)

$(CORBA_GENERATED) : $(top_srcdir)/idl/Terminal.idl
	$(ORBIT_IDL) `gnome-config --cflags idl` $(top_srcdir)/idl/Terminal.idl

corba.o: Terminal.h

gnome_terminal_SOURCES = 	\
	gnome-terminal.c	\
	corba.c			\
	gnome-terminal.h	\
	$(CORBA_GENERATED)

gnome_terminal_LDADD = $(LDADD)

gnorbadir = $(sysconfdir)/CORBA/servers
gnorba_DATA = gnome-terminal.gnorba

utildir = $(datadir)/gnome/apps/System
util_DATA = gnome-terminal.desktop
