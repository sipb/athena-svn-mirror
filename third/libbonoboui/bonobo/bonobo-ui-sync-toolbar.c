/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * bonobo-ui-sync-toolbar.h: The Bonobo UI/XML sync engine for toolbars
 *
 * Author:
 *	Michael Meeks (michael@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */

#include <config.h>
#include <stdlib.h>
#include <string.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <bonobo/bonobo-i18n.h>

#include <bonobo/bonobo-ui-xml.h>
#include <bonobo/bonobo-ui-util.h>
#include <bonobo/bonobo-ui-engine.h>
#include <bonobo/bonobo-ui-engine-config.h>
#include <bonobo/bonobo-ui-sync.h>
#include <bonobo/bonobo-ui-sync-toolbar.h>
#include <bonobo/bonobo-ui-preferences.h>
#include <bonobo/bonobo-ui-private.h>

#include <bonobo/bonobo-ui-toolbar.h>
#include <bonobo/bonobo-ui-toolbar-button-item.h>
#include <bonobo/bonobo-ui-toolbar-toggle-button-item.h>
#include <bonobo/bonobo-ui-toolbar-separator-item.h>
#include <bonobo/bonobo-ui-toolbar-popup-item.h>
#include <bonobo/bonobo-ui-toolbar-control-item.h>

static GObjectClass *parent_class = NULL;

static GQuark dockitem_id = 0;
static GQuark toolitem_id = 0;

#define PARENT_TYPE bonobo_ui_sync_get_type ()

static BonoboUIToolbarControlDisplay
decode_control_disp (const char *txt)
{
	if (!txt || !strcmp (txt, "control"))
		return BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_CONTROL;

	else if (!strcmp (txt, "button"))
		return BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_BUTTON;

	else if (!strcmp (txt, "none"))
		return BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_NONE;

	else
		return BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_CONTROL;
}

static gboolean 
string_array_contains (char **str_array, const char *match)
{
	int i = 0;
	char *string;

	while ((string = str_array [i++]))
		if (strcmp (string, match) == 0)
			return TRUE;

	return FALSE;
}

static void
impl_bonobo_ui_sync_toolbar_state (BonoboUISync     *sync,
				   BonoboUINode     *node,
				   BonoboUINode     *cmd_node,
				   GtkWidget        *widget,
				   GtkWidget        *parent)
{
	char *type, *label, *txt;
	char *min_width;
	char *behavior;
	char **behavior_array;
	gboolean priority;

	/* FIXME: to debug control problem */
	gtk_widget_show (widget);

	if ((behavior = bonobo_ui_engine_get_attr (node, cmd_node, "behavior"))) {
		
		behavior_array = g_strsplit (behavior, ",", -1);
		bonobo_ui_node_free_string (behavior);

		bonobo_ui_toolbar_item_set_expandable (
			BONOBO_UI_TOOLBAR_ITEM (widget),
			string_array_contains (behavior_array, "expandable"));

		bonobo_ui_toolbar_item_set_pack_end (
			BONOBO_UI_TOOLBAR_ITEM (widget),
			string_array_contains (behavior_array, "pack-end"));

		g_strfreev (behavior_array);
	}

	if ((txt = bonobo_ui_engine_get_attr (node, cmd_node, "priority"))) {
		priority = atoi (txt);
		bonobo_ui_node_free_string (txt);
	} else
		priority = FALSE;

	bonobo_ui_toolbar_item_set_want_label (
		BONOBO_UI_TOOLBAR_ITEM (widget), priority);

	type  = bonobo_ui_engine_get_attr (node, cmd_node, "type");
	label = bonobo_ui_engine_get_attr (node, cmd_node, "label");
	
	if (!type || !strcmp (type, "toggle")) {

		if (BONOBO_IS_UI_TOOLBAR_BUTTON_ITEM (widget) &&
		    (bonobo_ui_node_peek_attr (node, "pixtype") ||
		     bonobo_ui_node_peek_attr (cmd_node, "pixtype"))) {
			GtkWidget *image;
			BonoboUIToolbarButtonItem *button_item;

			button_item = (BonoboUIToolbarButtonItem *) widget;

			image = bonobo_ui_toolbar_button_item_get_image (button_item);
			if (!image) {
				image = gtk_image_new ();
				bonobo_ui_toolbar_button_item_set_image (button_item, image);
			}

			bonobo_ui_util_xml_set_image (
				GTK_IMAGE (image), node, cmd_node,
				GTK_ICON_SIZE_LARGE_TOOLBAR);
			gtk_widget_show (image);
		}

		if (label)
			bonobo_ui_toolbar_button_item_set_label (
				BONOBO_UI_TOOLBAR_BUTTON_ITEM (widget), label);
	}

	bonobo_ui_node_free_string (type);
	bonobo_ui_node_free_string (label);

	if (bonobo_ui_node_has_name (node, "control")) {
		const char *txt;
		BonoboUIToolbarControlDisplay hdisp, vdisp;
		gboolean sensitive;
		
		txt = bonobo_ui_node_peek_attr (node, "hdisplay");
		hdisp = decode_control_disp (txt);

		txt = bonobo_ui_node_peek_attr (node, "vdisplay");
		vdisp = decode_control_disp (txt);

		bonobo_ui_toolbar_control_item_set_display (
			BONOBO_UI_TOOLBAR_CONTROL_ITEM (widget), hdisp, vdisp);

		txt = bonobo_ui_node_peek_attr (node, "sensitive");
		if (txt)
			sensitive = atoi (txt);
		else
			sensitive = TRUE;

		bonobo_ui_toolbar_control_item_set_sensitive (
			BONOBO_UI_TOOLBAR_CONTROL_ITEM (widget), sensitive);
	}

	if ((min_width = bonobo_ui_engine_get_attr (node, cmd_node, "min_width"))) {
		bonobo_ui_toolbar_item_set_minimum_width (BONOBO_UI_TOOLBAR_ITEM (widget),
							  atoi (min_width));
		bonobo_ui_node_free_string (min_width);
	}
	
	if ((txt = bonobo_ui_engine_get_attr (node, cmd_node, "tip"))) {
		bonobo_ui_toolbar_item_set_tooltip (
			BONOBO_UI_TOOLBAR_ITEM (widget),
			bonobo_ui_toolbar_get_tooltips (
				BONOBO_UI_TOOLBAR (parent)), txt);

		bonobo_ui_node_free_string (txt);
	}

	bonobo_ui_engine_queue_update (
		sync->engine, widget, node, cmd_node);
}

static gint
exec_verb_cb (GtkWidget *item, BonoboUIEngine *engine)
{
	bonobo_ui_engine_emit_verb_on_w (engine, GTK_WIDGET (item));

	return FALSE;
}

static gint
win_item_emit_ui_event (BonoboUIToolbarItem *item,
			const char          *state,
			BonoboUIEngine      *engine)
{
	BonoboUINode     *node = bonobo_ui_engine_widget_get_node (
		GTK_WIDGET (item));

	g_return_val_if_fail (node != NULL, FALSE);

	bonobo_ui_engine_emit_event_on (engine, node, state);

	return FALSE;
}


static GtkWidget *
toolbar_build_control (BonoboUISync     *sync,
		       BonoboUINode     *node,
		       BonoboUINode     *cmd_node,
		       int              *pos,
		       GtkWidget        *parent)
{
	GtkWidget  *item;
	
	g_return_val_if_fail (sync != NULL, NULL);
	g_return_val_if_fail (node != NULL, NULL);

	if ((item = bonobo_ui_engine_node_get_widget (
		sync->engine, node))) {

		g_assert (item->parent == NULL);

	} else {
		Bonobo_Control control;

		control = bonobo_ui_engine_node_get_object (
			sync->engine, node);

		if (control != CORBA_OBJECT_NIL) {
			item = bonobo_ui_toolbar_control_item_new (control);

			if (!item)
				return NULL;

			bonobo_ui_engine_stamp_custom (
				sync->engine, node);
		} else
			return NULL;
	}

	gtk_widget_show (item);

	bonobo_ui_toolbar_insert (BONOBO_UI_TOOLBAR (parent),
				  BONOBO_UI_TOOLBAR_ITEM (item),
				  (*pos)++);

	return item;
}

static GtkWidget *
toolbar_build_widget (BonoboUISync *sync,
		      BonoboUINode *node,
		      BonoboUINode *cmd_node,
		      int          *pos,
		      GtkWidget    *parent)
{
	char         *type, *stock_id;
	GtkWidget    *item;

	g_return_val_if_fail (sync != NULL, NULL);
	g_return_val_if_fail (node != NULL, NULL);

	type = bonobo_ui_engine_get_attr (node, cmd_node, "type");

	if ((stock_id = bonobo_ui_engine_get_attr (node, cmd_node, "stockid"))) {
		GtkStockItem  stock_item;
		GtkIconSet   *icon_set;

		if (!gtk_stock_lookup (stock_id, &stock_item))
			g_warning ("Unknown stock id '%s' on %s", stock_id,
				   bonobo_ui_xml_make_path (node));
		else {
			gchar *label;
			int i, len;

			label = g_strdup (dgettext (stock_item.translation_domain, stock_item.label));

			len = strlen (label);
			for (i = 0; i < len; i++) {
				if (label [i] == '_') {
					memmove (label+i, label+i+1, strlen (label+i+1) + 1);
					len--;
				}
			}

			bonobo_ui_node_set_attr (node, "label", label);

			g_free (label);
		}

		icon_set = gtk_icon_factory_lookup_default (stock_id);

		if (icon_set) {
			bonobo_ui_node_set_attr (node, "pixtype", "stock");
			bonobo_ui_node_set_attr (node, "pixname", stock_id);
		}
	}

	if (bonobo_ui_node_has_name (node, "separator")) {
		item = bonobo_ui_toolbar_separator_item_new ();
		gtk_widget_set_sensitive (item, FALSE);

	} else if (!type)
		item = bonobo_ui_toolbar_button_item_new (NULL, NULL);
	
	else if (!strcmp (type, "toggle"))
		item = bonobo_ui_toolbar_toggle_button_item_new (NULL, NULL);
	
	else {
		/* FIXME: Implement radio-toolbars */
		g_warning ("Invalid type '%s'", type);
		return NULL;
	}

	bonobo_ui_node_free_string (type);
	
	bonobo_ui_toolbar_insert (BONOBO_UI_TOOLBAR (parent),
				  BONOBO_UI_TOOLBAR_ITEM (item),
				  (*pos)++);
	gtk_widget_show (item);

	return item;
}

static GtkWidget *
impl_bonobo_ui_sync_toolbar_build (BonoboUISync     *sync,
				   BonoboUINode     *node,
				   BonoboUINode     *cmd_node,
				   int              *pos,
				   GtkWidget        *parent)
{
	GtkWidget *widget;
	char      *verb;
	
	if (bonobo_ui_node_has_name (node, "control"))
		widget = toolbar_build_control (
			sync, node, cmd_node, pos, parent);
	else
		widget = toolbar_build_widget (
			sync, node, cmd_node, pos, parent);

	if (widget) {
		/* FIXME: What about "id"s ! ? */
		if ((verb = bonobo_ui_engine_get_attr (node, NULL, "verb"))) {
			g_signal_connect (GTK_OBJECT (widget), "activate",
					    (GtkSignalFunc) exec_verb_cb,
					    sync->engine);
			bonobo_ui_node_free_string (verb);
		}
		
		g_signal_connect (GTK_OBJECT (widget), "state_altered",
				    (GtkSignalFunc) win_item_emit_ui_event,
				    sync->engine);
	}

	return widget;
}

static GtkWidget *
impl_bonobo_ui_sync_toolbar_build_placeholder (BonoboUISync     *sync,
					       BonoboUINode     *node,
					       BonoboUINode     *cmd_node,
					       int              *pos,
					       GtkWidget        *parent)
{
	GtkWidget *widget;

	widget = bonobo_ui_toolbar_separator_item_new ();
	gtk_widget_set_sensitive (widget, FALSE);
	
	bonobo_ui_toolbar_insert (BONOBO_UI_TOOLBAR (parent),
				  BONOBO_UI_TOOLBAR_ITEM (widget),
				  (*pos)++);

	return widget;
}

static BonoboDockItem *
get_dock_item (BonoboUISyncToolbar *sync,
	       const char          *dockname)
{
	guint dummy;
	BonoboDockPlacement *dummy_placement = NULL;
	
	g_return_val_if_fail (dockname != NULL, NULL);

	return bonobo_dock_get_item_by_name (sync->dock,
					    dockname,
					    dummy_placement, &dummy,
					    &dummy, &dummy);
}

static GList *
impl_bonobo_ui_sync_toolbar_get_widgets (BonoboUISync *sync,
					 BonoboUINode *node)
{
	const char     *dockname;
	BonoboDockItem *item;

	dockname = bonobo_ui_node_peek_attr (node, "name");
	item = get_dock_item (BONOBO_UI_SYNC_TOOLBAR (sync), dockname);

	if (!item) {
		g_warning ("Serious internal error building toolbar");
		return NULL;
	}

	return bonobo_ui_toolbar_get_children (
		BONOBO_UI_TOOLBAR (GTK_BIN (item)->child));
}

static void
impl_bonobo_ui_sync_toolbar_state_update (BonoboUISync *sync,
					  GtkWidget    *widget,
					  const char   *new_state)
{
	g_return_if_fail (widget != NULL);

	if (new_state) {
		if (BONOBO_IS_UI_TOOLBAR_ITEM (widget))
			bonobo_ui_toolbar_item_set_state (
				BONOBO_UI_TOOLBAR_ITEM (widget), new_state);
		
		else
			g_warning ("TESTME: strange, setting "
				   "state '%s' on weird object '%s'",
				   new_state,
				   g_type_name_from_instance (
					   (GTypeInstance *) widget));
	}
}

static void
impl_dispose (GObject *object)
{
	BonoboUISyncToolbar *sync = (BonoboUISyncToolbar *) object;

	if (sync->dock) {
		g_object_unref (sync->dock);
		sync->dock = NULL;
	}

	parent_class->dispose (object);
}

static gboolean
impl_bonobo_ui_sync_toolbar_ignore_widget (BonoboUISync *sync,
					   GtkWidget    *widget)
{
	return BONOBO_IS_UI_TOOLBAR_POPUP_ITEM (widget);
}

static BonoboUIToolbarStyle
parse_look (const char *look)
{
	if (look) {
		if (!strcmp (look, "both"))
			return BONOBO_UI_TOOLBAR_STYLE_ICONS_AND_TEXT;

		if (!strcmp (look, "icon"))
			return BONOBO_UI_TOOLBAR_STYLE_ICONS_ONLY;

		if (!strcmp (look, "text"))
			return BONOBO_UI_TOOLBAR_STYLE_TEXT_ONLY;

		if (!strcmp (look, "both_horiz"))
			return BONOBO_UI_TOOLBAR_STYLE_PRIORITY_TEXT;
	}

	return bonobo_ui_preferences_get_toolbar_style ();
}

BonoboUIToolbarStyle
bonobo_ui_sync_toolbar_get_look (BonoboUIEngine *engine,
				 BonoboUINode   *node)
{
	const char *txt;
	BonoboUIToolbarStyle look;

	if ((txt = bonobo_ui_node_peek_attr (node, "look")))
		look = parse_look (txt);

	else {
		GtkWidget *widget;

		widget = bonobo_ui_engine_node_get_widget (engine, node);

		if (!widget || !BONOBO_IS_UI_TOOLBAR (widget) ||
		    bonobo_ui_toolbar_get_orientation (BONOBO_UI_TOOLBAR (widget)) ==
		    GTK_ORIENTATION_HORIZONTAL) {
			txt = bonobo_ui_node_peek_attr (node, "hlook");
			look = parse_look (txt);
		} else {
			txt = bonobo_ui_node_peek_attr (node, "vlook");
			look = parse_look (txt);
		}
	}		
	
	return look;
}

static char *
do_config_popup (BonoboUIEngineConfig *config,
		 BonoboUINode         *config_node,
		 BonoboUIEngine       *popup_engine)
{
	char *ret;
	gboolean tip;
	const char *txt;
	BonoboUIToolbarStyle style;
	
	tip = TRUE;
	if ((txt = bonobo_ui_node_peek_attr (config_node, "tips")))
		tip = atoi (txt);

	style = bonobo_ui_sync_toolbar_get_look (bonobo_ui_engine_config_get_engine (config),
						 config_node);

	ret = g_strdup_printf (
		"<Root>"
		"<commands>"
		"<cmd name=\"LookBoth\" state=\"%d\"/>"
		"<cmd name=\"LookIcon\" state=\"%d\"/>"
		"<cmd name=\"LookText\" state=\"%d\"/>"
		"</commands>"
		"<popups>"
		"<popup>"
		"<submenu label=\"%s\">"
		"<menuitem verb=\"LookBoth\" label=\"%s\" set=\"both\" "
		"type=\"radio\" group=\"look\"/>"
		"<menuitem verb=\"LookIcon\" label=\"%s\" set=\"icon\" "
		"type=\"radio\" group=\"look\"/>"
		"<menuitem verb=\"LookText\" label=\"%s\" set=\"text\" "
		"type=\"radio\" group=\"look\"/>"
		"</submenu>"
		"<separator/>"
		"<menuitem verb=\"Tip\" label=\"%s\" set=\"%d\"/>"
		"<menuitem verb=\"Hide\" label=\"%s\"/>"
		"<menuitem verb=\"Customize\" label=\"%s\" tip=\"%s\""
		" pixtype=\"stock\" pixname=\"Preferences\"/>"
		"</popup>"
		"</popups>"
		"</Root>",
		style == BONOBO_UI_TOOLBAR_STYLE_ICONS_AND_TEXT,
		style == BONOBO_UI_TOOLBAR_STYLE_ICONS_ONLY,
		style == BONOBO_UI_TOOLBAR_STYLE_PRIORITY_TEXT,
		_("Look"), _("B_oth"), _("_Icon"), _("T_ext"),
		tip ? _("Hide t_ips") :  _("Show t_ips"), !tip,
		_("_Hide toolbar"), _("Customi_ze"),
		_("Customize the toolbar"));

	return ret;
}

static void
config_verb_fn (BonoboUIEngineConfig *config,
		const char           *path,
		const char           *opt_state,
		BonoboUIEngine       *popup_engine,
		BonoboUINode         *popup_node)
{
	const char *verb;
	gboolean changed = TRUE;

	if ((verb = bonobo_ui_node_peek_attr (popup_node, "verb"))) {
		const char *set;

		set = bonobo_ui_node_peek_attr (popup_node, "set");

		if (!strcmp (verb, "Hide"))
			bonobo_ui_engine_config_add (
				config, path, "hidden", "1");

		else if (!strcmp (verb, "Show"))
			bonobo_ui_engine_config_remove (
				config, path, "hidden");

		else if (!strcmp (verb, "Tip"))
			bonobo_ui_engine_config_add (
				config, path, "tips", set);

		else if (!strncmp (verb, "Look", 4)) {
			if (opt_state && atoi (opt_state))
				bonobo_ui_engine_config_add (
					config, path, "look", set);
			else
				changed = FALSE;
			
		} else if (!strcmp (verb, "Customize")) {
			bonobo_ui_engine_config_configure (config);
			changed = FALSE;

		} else
			g_warning ("Unknown verb '%s'", verb);
	}


	if (changed)
		bonobo_ui_engine_config_serialize (config);
}

static BonoboDockItem *
create_dockitem (BonoboUISyncToolbar *sync,
		 BonoboUINode        *node,
		 const char          *dockname)
{
	BonoboDockItem *item;
	BonoboDockItemBehavior beh = 0;
	const char *prop;
	char      **behavior_array;
	gboolean force_detachable = FALSE;
	BonoboDockPlacement placement = BONOBO_DOCK_TOP;
	gint band_num = 1;
	gint position = 0;
	guint offset = 0;
	gboolean in_new_band = TRUE;
	gboolean can_config = TRUE;
	BonoboUIToolbar *toolbar;

	if ((prop = bonobo_ui_node_peek_attr (node, "behavior"))) {
		behavior_array = g_strsplit (prop, ",", -1);
	
		if (string_array_contains (behavior_array, "detachable"))
			force_detachable = TRUE;

		if (string_array_contains (behavior_array, "exclusive"))
			beh |= BONOBO_DOCK_ITEM_BEH_EXCLUSIVE;

		if (string_array_contains (behavior_array, "never vertical"))
			beh |= BONOBO_DOCK_ITEM_BEH_NEVER_VERTICAL;

		if (string_array_contains (behavior_array, "never floating"))
			beh |= BONOBO_DOCK_ITEM_BEH_NEVER_FLOATING;

		if (string_array_contains (behavior_array, "never horizontal"))
			beh |= BONOBO_DOCK_ITEM_BEH_NEVER_HORIZONTAL;

		g_strfreev (behavior_array);
	}

	if (!force_detachable && !bonobo_ui_preferences_get_toolbar_detachable ())
		beh |= BONOBO_DOCK_ITEM_BEH_LOCKED;

	item = BONOBO_DOCK_ITEM (bonobo_dock_item_new (
		dockname, beh));

	bonobo_dock_item_set_shadow_type (item, GTK_SHADOW_OUT);

	if ((prop = bonobo_ui_node_peek_attr (node, "placement"))) {
		if (!strcmp (prop, "top"))
			placement = BONOBO_DOCK_TOP;
		else if (!strcmp (prop, "right"))
			placement = BONOBO_DOCK_RIGHT;
		else if (!strcmp (prop, "bottom"))
			placement = BONOBO_DOCK_BOTTOM;
		else if (!strcmp (prop, "left"))
			placement = BONOBO_DOCK_LEFT;
		else if (!strcmp (prop, "floating"))
			placement = BONOBO_DOCK_FLOATING;
	}

	if ((prop = bonobo_ui_node_peek_attr (node, "band_num")))
		band_num = atoi (prop);

	if ((prop = bonobo_ui_node_peek_attr (node, "position")))
		position = atoi (prop);

	if ((prop = bonobo_ui_node_peek_attr (node, "offset")))
		offset = atoi (prop);

	if ((prop = bonobo_ui_node_peek_attr (node, "in_new_band")))
		in_new_band = atoi (prop);

	bonobo_dock_add_item (sync->dock, item,
			      placement, band_num,
			      position, offset, in_new_band);

		
	toolbar = BONOBO_UI_TOOLBAR (bonobo_ui_toolbar_new ());

	gtk_container_set_border_width (GTK_CONTAINER (toolbar), 2);
	gtk_container_add (GTK_CONTAINER (item),
			   GTK_WIDGET (toolbar));
	gtk_widget_show (GTK_WIDGET (toolbar));

	if ((prop = bonobo_ui_node_peek_attr (node, "config")))
		can_config = atoi (prop);
	else
		can_config = TRUE;

	if (can_config) {
		char *path;
		path = bonobo_ui_xml_make_path (node);

		bonobo_ui_engine_config_connect (
			GTK_WIDGET (item), sync->parent.engine,
			path, do_config_popup, config_verb_fn);

		bonobo_ui_engine_config_connect (
			GTK_WIDGET (toolbar), sync->parent.engine,
			path, do_config_popup, config_verb_fn);

		g_free (path);
	}

	return item;
}

static void
impl_bonobo_ui_sync_toolbar_remove_root (BonoboUISync *sync,
					 BonoboUINode *node)
{
	const char *name;

	if ((name = bonobo_ui_node_peek_attr (node, "name"))) {
		BonoboDockItem *item;

		item = get_dock_item (BONOBO_UI_SYNC_TOOLBAR (sync), name);

		if (item)
			gtk_widget_destroy (GTK_WIDGET (item));
	}
}

static void
impl_bonobo_ui_sync_toolbar_update_root (BonoboUISync *sync,
					 BonoboUINode *node)
{
	const char *txt;
	const char *dockname;
	gboolean    tooltips;
	gboolean    detachable;
	BonoboDockItem *item;
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarStyle look;

	dockname = bonobo_ui_node_peek_attr (node, "name");

	g_return_if_fail (dockname != NULL);

	item = get_dock_item (BONOBO_UI_SYNC_TOOLBAR (sync), dockname);
	
	if (!item)
		item = create_dockitem (BONOBO_UI_SYNC_TOOLBAR (sync),
					node, dockname);

	if ((txt = bonobo_ui_node_peek_attr (node, "behavior")) &&
	    strstr (txt, "detachable"))
		detachable = TRUE;
	else
		detachable = bonobo_ui_preferences_get_toolbar_detachable ();
	bonobo_dock_item_set_locked (item, !detachable);

	toolbar = BONOBO_UI_TOOLBAR (GTK_BIN (item)->child);

	bonobo_ui_engine_stamp_root (sync->engine, node, GTK_WIDGET (toolbar));

	if ((txt = bonobo_ui_node_peek_attr (node, "look"))) {
		look = parse_look (txt);
		bonobo_ui_toolbar_set_hv_styles (toolbar, look, look);

	} else {
		BonoboUIToolbarStyle vlook, hlook;

		txt = bonobo_ui_node_peek_attr (node, "hlook");
		hlook = parse_look (txt);

		txt = bonobo_ui_node_peek_attr (node, "vlook");
		vlook = parse_look (txt);

		bonobo_ui_toolbar_set_hv_styles (toolbar, hlook, vlook);
	}		

#if 0
	if ((txt = bonobo_ui_node_get_attr (node, "relief"))) {

		if (!strcmp (txt, "normal"))
			bonobo_ui_toolbar_set_relief (
				toolbar, GTK_RELIEF_NORMAL);

		else if (!strcmp (txt, "half"))
			bonobo_ui_toolbar_set_relief (
				toolbar, GTK_RELIEF_HALF);
		else
			bonobo_ui_toolbar_set_relief (
				toolbar, GTK_RELIEF_NONE);
		bonobo_ui_node_free_string (txt);
	}
#endif

	if ((txt = bonobo_ui_node_peek_attr (node, "tips")))
		tooltips = atoi (txt);
	else
		tooltips = TRUE;

	bonobo_ui_toolbar_show_tooltips (toolbar, tooltips);

       /*
	* FIXME: It shouldn't be necessary to explicitly resize the
	* dock, since resizing a widget is supposed to resize it's parent,
	* but the dock is not resized correctly on dockitem show / hides.
	*/
	if (bonobo_ui_sync_do_show_hide (sync, node, NULL, GTK_WIDGET (item)))
		gtk_widget_queue_resize (GTK_WIDGET (
			BONOBO_UI_SYNC_TOOLBAR (sync)->dock));
}

static gboolean
impl_bonobo_ui_sync_toolbar_can_handle (BonoboUISync *sync,
					BonoboUINode *node)
{
	if (!dockitem_id) {
		dockitem_id = g_quark_from_static_string ("dockitem");
		toolitem_id = g_quark_from_static_string ("toolitem");
	}

	return (node->name_id == dockitem_id ||
		node->name_id == toolitem_id);
}

GtkWidget *
impl_bonobo_ui_sync_toolbar_wrap_widget (BonoboUISync *sync,
					 GtkWidget    *custom_widget)
{
	if (!BONOBO_IS_UI_TOOLBAR_ITEM (custom_widget))
		return bonobo_ui_toolbar_control_item_new_widget (custom_widget);
	else
		return custom_widget;
}

static void
class_init (BonoboUISyncClass *sync_class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (sync_class);

	object_class = G_OBJECT_CLASS (sync_class);
	object_class->dispose = impl_dispose;

	sync_class->sync_state = impl_bonobo_ui_sync_toolbar_state;
	sync_class->build      = impl_bonobo_ui_sync_toolbar_build;
	sync_class->build_placeholder = impl_bonobo_ui_sync_toolbar_build_placeholder;

	sync_class->get_widgets   = impl_bonobo_ui_sync_toolbar_get_widgets;
	sync_class->ignore_widget = impl_bonobo_ui_sync_toolbar_ignore_widget;
	sync_class->remove_root   = impl_bonobo_ui_sync_toolbar_remove_root;
	sync_class->update_root   = impl_bonobo_ui_sync_toolbar_update_root;

	sync_class->state_update  = impl_bonobo_ui_sync_toolbar_state_update;
	sync_class->can_handle    = impl_bonobo_ui_sync_toolbar_can_handle;

	sync_class->wrap_widget   = impl_bonobo_ui_sync_toolbar_wrap_widget;
}

GType
bonobo_ui_sync_toolbar_get_type (void)
{
	static GType type = 0;

	if (type == 0) {
		GTypeInfo info = {
			sizeof (BonoboUISyncToolbarClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (BonoboUISyncToolbar),
			0, /* n_preallocs */
			(GInstanceInitFunc) NULL
		};

		type = g_type_register_static (PARENT_TYPE, "BonoboUISyncToolbar",
					       &info, 0);
	}

	return type;
}

BonoboUISync *
bonobo_ui_sync_toolbar_new (BonoboUIEngine  *engine,
			    BonoboDock       *dock)
{
	BonoboUISyncToolbar *sync;

	g_return_val_if_fail (BONOBO_IS_UI_ENGINE (engine), NULL);

	sync = g_object_new (BONOBO_TYPE_UI_SYNC_TOOLBAR, NULL);

	sync->dock = g_object_ref (dock);

	return bonobo_ui_sync_construct (
		BONOBO_UI_SYNC (sync), engine, FALSE, TRUE);
}
