INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_builddir)			\
	$(CDDBSLAVE_CFLAGS)			\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
	-DDATADIR=\""$(datadir)"\"		\
	-DG_LOG_DOMAIN=\"CDDBSlave2\"

bin_PROGRAMS = cddb-slave2-properties
lib_LTLIBRARIES = libcddb-slave2.la
libexec_PROGRAMS = CDDBSlave2 cddb-track-editor

IDLS = GNOME_Media_CDDBSlave2.idl

IDL_GENERATED =					\
	GNOME_Media_CDDBSlave2.h		\
	GNOME_Media_CDDBSlave2-common.c		\
	GNOME_Media_CDDBSlave2-skels.c		\
	GNOME_Media_CDDBSlave2-stubs.c

GNOME_Media_CDDBSlave2-impl.o: GNOME_Media_CDDBSlave.h

$(IDL_GENERATED): $(IDLS)
	$(ORBIT_IDL) -I $(BONOBO_IDLDIR) -I $(BONOBO_ACT_IDLDIR)	\
	-I $(srcdir) -I $(datadir)/idl 	\
		$(srcdir)/GNOME_Media_CDDBSlave2.idl

idldir = $(datadir)/idl
idl_DATA = $(IDLS)

CDDBSlave2_SOURCES =				\
	$(IDL_GENERATED)			\
	cddb-slave.c				\
	cddb-slave.h				\
	cddb-parser.c				\
	cddb-parser.h				\
	gnet.h					\
	gnet-private.c				\
	gnet-private.h				\
	iochannel.c				\
	iochannel.h				\
	inetaddr.c				\
	inetaddr.h				\
	main.c					\
	socks.c					\
	socks.h					\
	socks-private.c				\
	socks-private.h				\
	tcp.c					\
	tcp.h

CDDBSlave2_LDADD =				\
	$(CDDBSLAVE_LIBS)				\
	$(INTLLIBS)

cddb_track_editor_SOURCES = 			\
	$(IDL_GENERATED)			\
	cddb-disclosure.c	\
	cddb-disclosure.h			\
	cddb-track-editor.c

cddb_track_editor_LDADD =				\
	$(CDDBSLAVE_LIBS)				\
	libcddb-slave2.la

cddb_slave2_properties_LDADD = $(CDDBSLAVE_LIBS)
cddb_slave2_properties_SOURCES = cddb-slave-capplet.c	\
				 gnet.h			\
				 gnet-private.c		\
				 gnet-private.h		\
				 iochannel.c		\
				 iochannel.h		\
				 inetaddr.c		\
				 inetaddr.h		\
				 socks.c		\
				 socks.h		\
				 socks-private.c	\
				 socks-private.h	\
				 tcp.c			\
				 tcp.h

libcddb_slave2includedir = $(includedir)/cddb-slave2
libcddb_slave2_la_SOURCES =			\
	$(IDL_GENERATED)			\
	cddb-slave-client.c

libcddb_slave2include_HEADERS =			\
	cddb-slave-client.h

schemadir=$(sysconfdir)/gconf/schemas
schema_DATA=CDDB-Slave2.schemas


server_in_files = GNOME_Media_CDDBSlave2.server.in.in

serverdir = $(libdir)/bonobo/servers
server_DATA = $(server_in_files:.server.in.in=.server)
$(server_in_files:.server.in.in=.server.in): $(server_in_files)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@
@INTLTOOL_SERVER_RULE@
@INTLTOOL_DESKTOP_RULE@
@INTLTOOL_SCHEMAS_RULE@

desktopdir = $(datadir)/control-center-2.0/capplets
Desktop_in_files = cddb-slave.desktop.in
desktop_DATA = $(Desktop_in_files:.desktop.in=.desktop)

EXTRA_DIST = $(server_in_files) $(Desktop_in_files) $(IDLS) $(schema_DATA)

install-schemas:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)

install-data-local: install-schemas
