SUBDIRS = stock cursors

INCLUDES =						\
	-I$(includedir)					\
	-DG_LOG_DOMAIN=\"GGV\"				\
	-DGNOMELOCALEDIR=\""${prefix}/${DATADIRNAME}/locale"\"	\
	-DGNOMEICONDIR=\""${prefix}/${DATADIRNAME}/pixmaps"\"		\
	-DDATADIR=\""$(datadir)"\"			\
	$(GGV_EXTRA_CFLAGS)				\
	$(GGV_CFLAGS)

bin_PROGRAMS = ggv

CORBA_SOURCES =			\
	Ggv.h			\
	Ggv-common.c		\
	Ggv-stubs.c		\
	Ggv-skels.c

noinst_LTLIBRARIES = libgtkgs.la

libgtkgs_la_SOURCES =			\
	$(CORBA_SOURCES)		\
	gtkgs.c				\
	gtkgs.h				\
	ps.c                            \
	ps.h				\
	ggvutils.c			\
	ggvutils.h			\
	gsdefaults.c			\
	gsdefaults.h			\
	gsio.c				\
	gsio.h				\
	ggv-msg-window.h		\
	ggv-msg-window.c		\
	ggv-file-sel.h			\
	ggv-file-sel.c			\
	ggv-prefs.c			\
	ggv-prefs.h			\
	ggv-prefs-ui.c			\
	ggv-prefs-ui.h			\
	gtkchecklist.c			\
	gtkchecklist.h			\
	cursors.c			\
	cursors.h

libgtkgs_la_LIBADD = 			\
        $(GGV_LIBS)						

ggv_SOURCES =                           \
	ggv-window.h                    \
	ggv-window.c                    \
	main.c                           

ggv_LDADD =	libgtkgs.la 

LIBS = $(GGV_LIBS)

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = ggv-ui.xml

idls = $(top_srcdir)/idl/Ggv.idl

$(CORBA_SOURCES): $(idls) $(ORBIT_IDL)
	$(ORBIT_IDL) -I $(BONOBO_IDLDIR) -I $(BONOBO_ACT_IDLDIR)  $(top_srcdir)/idl/Ggv.idl

Ggv.h: $(idls)

BUILT_SOURCES=$(CORBA_SOURCES)

EXTRA_DIST = $(ui_DATA)

CLEANFILES = $(BUILT_SOURCES)

