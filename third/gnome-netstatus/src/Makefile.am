
INCLUDES = \
	-I$(top_builddir)/gnome-netstatus \
	-DPREFIX=\""$(prefix)\"" \
	-DSYSCONFDIR=\""$(sysconfdir)\"" \
	-DDATADIR=\""$(datadir)\"" \
	-DLIBDIR=\""$(libdir)\"" \
	$(NETSTATUS_CFLAGS) \
	$(NETSTATUS_DEBUG_CFLAGS) \
	$(WARN_CFLAGS) \
	-DNETSTATUS_ICONDIR=\""$(datadir)/icons/gnome-netstatus\"" \
	-DNETSTATUS_GLADEDIR=\""$(gladedir)\"" \
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

libexec_PROGRAMS = gnome-netstatus-applet

gnome_netstatus_applet_LDADD = $(NETSTATUS_LIBS)

gnome_netstatus_applet_SOURCES = \
	netstatus-applet.c \
	netstatus-applet.h \
	netstatus-util.h \
	netstatus-util.c \
	netstatus-iface.c \
	netstatus-iface.h \
	netstatus-sysdeps.h \
	netstatus-sysdeps.c \
	netstatus-icon.c \
	netstatus-icon.h \
	netstatus-dialog.c \
	netstatus-dialog.h \
	netstatus-fallback-pixbuf.h \
	netstatus-enums.c \
	netstatus-enums.h

netstatus_enum_headers = $(srcdir)/netstatus-util.h

netstatus-enums.c: @REBUILD@ $(netstatus_enum_headers)
	glib-mkenums    --fhead "#include <glib-object.h>\n" \
	                --fhead "#include \"netstatus-enums.h\"\n\n" \
	                --fprod "\n/* enumerations from \"@filename@\" */" \
	                --fprod "\n#include \"@filename@\"\n" \
	                --vhead "static const GEnumValue _@enum_name@_values[] = {" \
	                --vprod "  { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
	                --vtail "  { 0, NULL, NULL }\n};\n\n" \
	                --vtail "GType\n@enum_name@_get_type (void)\n{\n" \
	                --vtail "  static GType type = 0;\n\n" \
	                --vtail "  if (!type)\n" \
	                --vtail "    type = g_enum_register_static (\"@EnumName@\", _@enum_name@_values);\n\n" \
	                --vtail "  return type;\n}\n\n" \
                $(netstatus_enum_headers) > $@

netstatus-enums.h: @REBUILD@ $(netstatus_enum_headers)
	glib-mkenums    --fhead "#ifndef __NETSTATUS_ENUMS_H__\n" \
	                --fhead "#define __NETSTATUS_ENUMS_H__ 1\n\n" \
	                --fhead "G_BEGIN_DECLS\n\n" \
	                --ftail "G_END_DECLS\n\n" \
	                --ftail "#endif /* __PANEL_ENUMS_H__ */\n" \
	                --fprod "\n/* --- @filename@ --- */" \
	                --eprod "#define NETSTATUS_TYPE_@ENUMSHORT@ @enum_name@_get_type()\n" \
	                --eprod "GType @enum_name@_get_type (void);\n" \
                $(netstatus_enum_headers) >  $@

BUILT_SOURCES = \
	netstatus-enums.c \
	netstatus-enums.h

netstatus-fallback-pixbuf.h: $(top_srcdir)/icons/gnome-netstatus-disconn.png
	$(GDK_PIXBUF_CSOURCE) --raw --name=fallback_icon_data $(top_srcdir)/icons/gnome-netstatus-error.png > $(srcdir)/netstatus-fallback-pixbuf.h

$(gnome_netstatus_applet_OBJECTS): netstatus-fallback-pixbuf.h
$(OBJECTS): netstatus-fallback-pixbuf.h

noinst_DATA = netstatus-fallback-pixbuf.h

gladedir   = $(datadir)/gnome-netstatus
glade_DATA = gnome-netstatus.glade

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_NetstatusApplet_Factory.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files).in
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

@INTLTOOL_SERVER_RULE@

schemasdir       = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = netstatus.schemas.in
schemas_DATA     = $(schemas_in_files:.schemas.in=.schemas)

@INTLTOOL_SCHEMAS_RULE@

if GCONF_SCHEMAS_INSTALL
# don't do this if we are building in eg. rpm
install-data-local:
	if test -z "$(DESTDIR)" ; then \
		for p in $(schemas_DATA) ; do \
			GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/src/$$p ; \
		done \
	fi
else
install-data-local:
endif

uidir    = $(datadir)/gnome-2.0/ui
ui_DATA  = GNOME_NetstatusApplet.xml

CLEANFILES = $(BUILT_SOURCES) $(noinst_DATA)
EXTRA_DIST = \
	$(glade_DATA) \
	$(ui_DATA) \
	$(schemas_in_files) \
	GNOME_NetstatusApplet_Factory.server.in.in

dist-hook:
	cd $(distdir) ; rm -f $(CLEANFILES)

