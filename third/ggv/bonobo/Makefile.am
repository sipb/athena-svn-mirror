INCLUDES =                                              \
        -DGNOMELOCALEDIR=\""${prefix}/${DATADIRNAME}/locale"\"        \
	-DGNOMEICONDIR=\""$(datadir)/pixmaps"\"		\
	-DG_LOG_DOMAIN=\"GGV\"				\
	-DDATADIR=\""$(datadir)"\"			\
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_srcdir)/src				\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	$(GGV_EXTRA_CFLAGS)				\
	$(GGV_CFLAGS)

libexec_PROGRAMS = ggv-postscript-viewer

ggv_postscript_viewer_SOURCES =	\
	ggv-postscript-view.c	\
	ggv-postscript-view.h	\
	ggv-control.c		\
	ggv-control.h		\
	ggv-sidebar.c		\
	ggv-sidebar.h		\
	main.c

# if XINERAMA
# XINERAMA_LIBS =			\
#	-lXinerama
# else
# XINERAMA_LIBS =
# endif

ggv_postscript_viewer_LDADD = 			\
	$(GGV_LIBS)				\
	$(INTLLIBS)				\
	../src/libgtkgs.la
#	$(XINERAMA_LIBS)

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = ggv-postscript-view-ui.xml ggv-control-ui.xml


server_in_files = GNOME_GGV.server.in.in

serverdir = $(libdir)/bonobo/servers
server_DATA = $(server_in_files:.server.in.in=.server)
$(server_in_files:.server.in.in=.server.in): $(server_in_files)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@
@INTLTOOL_SERVER_RULE@

CLEANFILES = $(server_DATA)

EXTRA_DIST = $(ui_DATA) $(server_in_files)
