INCLUDES = \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" -I$(top_srcdir)/src -I$(top_srcdir)/lib -I$(top_srcdir)/lib/widgets -I$(top_srcdir)/lib/xdap $(GNOME_INCLUDEDIR) @GNOMEMEETING_CFLAGS@ -DGNOMEMEETING_IMAGES=\""$(datadir)/pixmaps/"\" -DGNOMEMEETING_SOUNDS=\""$(datadir)/sounds/gnomemeeting/"\" -DSCHEMA_AGE=@SCHEMA_AGE@ -DDATADIR=\""$(datadir)"\" 

#-DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED

bin_SCRIPTS = gnomemeeting-config-tool
bin_PROGRAMS = gnomemeeting

gnomemeeting_SOURCES =		\
	common.h		\
	callbacks.cpp		\
	callbacks.h		\
	gdkvideoio.cpp		\
	gdkvideoio.h		\
	gnomemeeting.cpp	\
	gnomemeeting.h		\
	connection.cpp		\
	connection.h		\
	endpoint.cpp		\
	endpoint.h		\
	menu.cpp		\
	menu.h			\
	toolbar.cpp		\
	toolbar.h		\
	pref_window.cpp		\
	pref_window.h		\
	sound_handling.cpp	\
	sound_handling.h	\
	videograbber.h		\
	videograbber.cpp	\
	config.h		\
	config.cpp		\
	ldap_window.cpp		\
	ldap_window.h		\
	main_window.cpp		\
	main_window.h		\
	gatekeeper.cpp		\
	gatekeeper.h		\
	ils.cpp			\
	ils.h			\
	misc.cpp		\
	misc.h			\
	chat_window.cpp		\
	chat_window.h		\
	druid.cpp		\
	druid.h			\
	urlhandler.cpp		\
	urlhandler.h		\
	vfakeio.cpp		\
	vfakeio.h		\
	tray.h			\
	tray.cpp		\
	tools.cpp		\
	tools.h			\
	lid.cpp			\
	lid.h			\
	codec_info.cpp		\
	codec_info.h


gnomemeeting_LDADD = \
	@GNOMEMEETING_LIBS@ \
	$(top_builddir)/lib/libgnomemeeting.la

serverdir = $(libdir)/bonobo/servers
server_DATA = gnomemeeting.server
