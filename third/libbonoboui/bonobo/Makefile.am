INCLUDES = \
	-I$(top_srcdir) 				\
	-I$(top_builddir)				\
	$(WARN_CFLAGS)					\
	$(BONOBOUI_CFLAGS)				\
	-DPLUGIN_DIR=\""$(PLUGIN_DIR)"\"		\
	-DBONOBO_UI_INTERNAL				\
	-DBONOBO_UIDIR=\""$(datadir)/gnome-2.0/ui/"\"	\
        -DBONOBO_LIBDIR=\""$(libdir)"\" 		\
        -DBONOBO_DATADIR=\""$(datadir)"\" 		\
        -DBONOBO_PIXMAPDIR=\""$(datadir)/pixmaps"\"	\
        -DBONOBO_BINDIR=\""$(bindir)"\" 		\
        -DBONOBO_LOCALSTATEDIR=\""$(localstatedir)"\" 	\
        -DBONOBO_LOCALEDIR=\""$(gnomelocaledir)"\" 	\
	-DG_DISABLE_DEPRECATED				\
	-DGDK_DISABLE_DEPRECATED			\
	-DGDK_PIXBUF_DISABLE_DEPRECATED			\
	-DGDK_MULTIHEAD_SAFE				\
	-DGTK_DISABLE_DEPRECATED			\
	-DGNOME_DISABLE_DEPRECATED			\
	-DBONOBO_DISABLE_DEPRECATED			\
	-DG_LOG_DOMAIN=\"Bonobo\"			\
	-DVERSION=\"$(VERSION)\"			\
	-DBONOBO_EXPLICIT_TRANSLATION_DOMAIN=\"$(GETTEXT_PACKAGE)\"

lib_LTLIBRARIES = libbonoboui-2.la

noinst_HEADERS = \
	bonobo-control-internal.h		\
	bonobo-ui-engine-config.h		\
	bonobo-ui-engine-private.h		\
	bonobo-ui-marshal.h			\
	bonobo-ui-node-private.h		\
	bonobo-ui-preferences.h			\
	bonobo-ui-private.h			\
	bonobo-ui-sync-menu.h			\
	bonobo-ui-sync-keys.h			\
	bonobo-ui-sync-status.h			\
	bonobo-ui-sync-toolbar.h		\
	bonobo-ui-toolbar.h			\
	bonobo-ui-toolbar-control-item.h  	\
	bonobo-ui-toolbar-popup-item.h		\
	bonobo-ui-xml.h

libbonoboincludedir = $(includedir)/libbonoboui-2.0/bonobo

libbonoboui_h_sources = \
	bonobo-canvas-component.h		\
	bonobo-canvas-item.h			\
	bonobo-control-frame.h			\
	bonobo-control.h			\
	bonobo-dock-band.h			\
	bonobo-dock-item.h			\
	bonobo-dock-layout.h			\
	bonobo-dock.h				\
	bonobo-file-selector-util.h		\
	bonobo-plug.h				\
	bonobo-property-control.h		\
	bonobo-selector-widget.h		\
	bonobo-selector.h			\
	bonobo-socket.h				\
	bonobo-ui-component.h			\
	bonobo-ui-config-widget.h		\
	bonobo-ui-container.h			\
	bonobo-ui-engine.h			\
	bonobo-ui-main.h			\
	bonobo-ui-node.h			\
	bonobo-ui-sync.h			\
	bonobo-ui-toolbar-button-item.h		\
	bonobo-ui-toolbar-item.h		\
	bonobo-ui-toolbar-toggle-button-item.h	\
	bonobo-ui-toolbar.h			\
	bonobo-ui-util.h			\
	bonobo-widget.h				\
	bonobo-window.h				\
	bonobo-zoomable-frame.h			\
	bonobo-zoomable.h

libbonoboui_2_la_LDFLAGS = \
	-no-undefined

libbonoboui_2_la_LIBADD = \
	$(BONOBOUI_LIBS) ${X_LIBS}

libbonoboui_2_la_SOURCES = \
	bonobo-a11y.c				\
	bonobo-a11y.h				\
	bonobo-dock.c				\
	bonobo-dock.h				\
	bonobo-dock-item.c			\
	bonobo-dock-item.h			\
	bonobo-dock-item-grip.c			\
	bonobo-dock-item-grip.h			\
	bonobo-dock-band.c			\
	bonobo-dock-band.h			\
	bonobo-dock-layout.c			\
	bonobo-dock-layout.h			\
	bonobo-ui-init-gtk.c			\
	bonobo-ui-main.h			\
	bonobo-ui-main.c			\
	bonobo-canvas-component.c		\
	bonobo-canvas-item.c			\
	bonobo-control-frame.c			\
	bonobo-control.c			\
	bonobo-file-selector-util.c		\
	bonobo-plug.c				\
	bonobo-property-control.c		\
	bonobo-selector.c			\
	bonobo-selector-widget.c		\
	bonobo-socket.c				\
	bonobo-ui-component.c            	\
	bonobo-ui-config-widget.c		\
	bonobo-ui-container.c            	\
	bonobo-ui-engine.c			\
	bonobo-ui-engine-config.c		\
	bonobo-ui-node.c                 	\
	bonobo-ui-preferences.c			\
	bonobo-ui-sync.c			\
	bonobo-ui-sync-menu.c			\
	bonobo-ui-sync-keys.c			\
	bonobo-ui-sync-status.c			\
	bonobo-ui-sync-toolbar.c		\
	bonobo-ui-toolbar-button-item.c  	\
	bonobo-ui-toolbar-control-item.c  	\
	bonobo-ui-toolbar-item.c		\
	bonobo-ui-toolbar-popup-item.c		\
	bonobo-ui-toolbar-toggle-button-item.c	\
	bonobo-ui-toolbar.c			\
	bonobo-ui-internal-toolbar.c		\
	bonobo-ui-type-builtins.c		\
	bonobo-ui-util.c			\
	bonobo-ui-xml.c				\
	bonobo-widget.c				\
	bonobo-window.c				\
	bonobo-zoomable.c			\
	bonobo-zoomable-frame.c			\
	bonobo-ui-marshal-main.c


$(libbonoboui_2_la_OBJECTS): $(libbonoboui_built_sources)

libbonoboui_built_public_sources = \
	bonobo-ui-type-builtins.h

libbonoboui_built_private_sources = \
	bonobo-ui-type-builtins.c	\
	bonobo-ui-marshal.c 		\
	bonobo-ui-marshal.h		\
	libbonoboui.defs

libbonoboui_built_sources = \
	$(libbonoboui_built_public_sources)	\
	$(libbonoboui_built_private_sources)

libbonoboinclude_HEADERS = \
	$(libbonoboui_h_sources) \
	$(libbonoboui_built_public_sources)

bonobo-ui-marshal.h: bonobo-ui-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=bonobo_ui_marshal > $@

bonobo-ui-marshal.c: bonobo-ui-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --body --prefix=bonobo_ui_marshal > $@

bonobo-ui-type-builtins.c: $(GLIB_MKENUMS) $(libbonoboui_h_sources)
	$(GLIB_MKENUMS) --fhead "#include \"libbonoboui.h\"\n\n" \
			--fhead "#include \"bonobo-ui-type-builtins.h\"\n\n" \
			--fprod "\n/* enumerations from \"@filename@\" */" \
			--vhead "static const G@Type@Value _@enum_name@_values[] = {" \
			--vprod "  { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
			--vtail "  { 0, NULL, NULL }\n};\n\n" \
			--vtail "GType\n@enum_name@_get_type (void)\n{\n" \
			--vtail "  static GType type = 0;\n\n" \
			--vtail "  if (!type)\n" \
			--vtail "    type = g_@type@_register_static (\"@EnumName@\", _@enum_name@_values);\n\n" \
			--vtail "  return type;\n}\n\n" \
		$(filter-out $<,$^) > xgen-gtbec \
	&& cp xgen-gtbec $(@F) \
	&& rm -f xgen-gtbec

bonobo-ui-type-builtins.h: $(GLIB_MKENUMS) $(libbonoboui_h_sources)
	$(GLIB_MKENUMS) --fhead "#ifndef __BONOBO_UI_TYPE_BUILTINS_H__\n" \
			--fhead "#define __BONOBO_UI_TYPE_BUILTINS_H__ 1\n\n" \
			--fhead "#include <glib-object.h>\n\n" \
			--fhead "G_BEGIN_DECLS\n\n" \
			--ftail "G_END_DECLS\n\n" \
			--ftail "#endif /* __BONOBO_UI_TYPE_BUILTINS_H__ */\n" \
			--fprod "\n/* --- @filename@ --- */" \
			--eprod "#define BONOBO_TYPE_@ENUMSHORT@ @enum_name@_get_type()\n" \
			--eprod "GType @enum_name@_get_type (void);\n" \
		$(filter-out $<,$^) > xgen-gth \
	&& cp xgen-gth $(@F) \
	&& rm -f xgen-gth

libbonoboui.defs: $(libbonoboui_h_sources) $(GLIB_MKENUMS)
	$(GLIB_MKENUMS) --comments ";; @comment@" \
			--fprod "; enumerations from \"@filename@\"\n " \
	                --vhead "(define-@type@ @EnumName@" \
	                --vprod "   (@valuenick@ @VALUENAME@)" \
	                --vtail ")" \
			$(filter-out $<,$^) > xgen-gdef \
	&& (cmp -s xgen-gdef libbonoboui.defs || cp xgen-gdef libbonoboui.defs) \
	&& rm -f xgen-gdef

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libbonoboui-2.0.pc

EXTRA_DIST = bonobo-insert-component.xpm libbonoboui-2.0.pc.in \
	libbonoboui-2.0-uninstalled.pc.in bonobo-ui-marshal.list
CLEANFILES = $(libbonoboui_built_sources)

BUILT_SOURCE = $(libbonoboui_built_sources)
DONT_DIST_SOURCE = $(libbonoboui_built_sources)

dist-hook:
	for file in $(DONT_DIST_SOURCE) ; do \
	    rm -f $(distdir)/$$file ; \
	done
