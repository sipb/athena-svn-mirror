/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-ui-preferences.c: private wrappers for global UI preferences.
 *
 * Authors:
 *     Michael Meeks (michael@ximian.com)
 *     Martin Baulig (martin@home-of-linux.org)
 *
 * Copyright 2001 Ximian, Inc.
 */

#include <config.h>
#include <string.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <libgnome/gnome-gconf.h>
#include <bonobo/bonobo-ui-private.h>
#include <bonobo/bonobo-ui-preferences.h>

#define GLOBAL_INTERFACE_KEY "/desktop/gnome/interface"

static GConfEnumStringPair toolbar_styles[] = {
        { BONOBO_UI_TOOLBAR_STYLE_TEXT_ONLY,      "text" },
        { BONOBO_UI_TOOLBAR_STYLE_ICONS_ONLY,     "icons" },
        { BONOBO_UI_TOOLBAR_STYLE_ICONS_AND_TEXT, "both" },
        { BONOBO_UI_TOOLBAR_STYLE_PRIORITY_TEXT,  "both_horiz" }
};

static GConfClient *client;
static GSList *engine_list;
guint desktop_notify_id;
guint update_engines_idle_id;

static void keys_changed_fn (GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer user_data);

static gboolean
update_engines_idle_callback (gpointer data)
{
	BonoboUINode *root_node;
	BonoboUIEngine *engine;
	GSList *node;

	if (update_engines_idle_id == 0)
		return FALSE;
	
	for (node = engine_list; node; node = node->next) {
		engine = node->data;
		
		root_node = bonobo_ui_engine_get_path (engine, "/");

		bonobo_ui_engine_dirty_tree (engine, root_node);
	}

	update_engines_idle_id = 0;

	return FALSE;
}

void
bonobo_ui_preferences_add_engine (BonoboUIEngine *engine)
{
	if (!client)
		client = gconf_client_get_default ();
	
	if (engine_list == NULL) {
		/* We need to intialize the notifiers */
		gconf_client_add_dir (client, GLOBAL_INTERFACE_KEY, GCONF_CLIENT_PRELOAD_RECURSIVE, NULL);
		
		desktop_notify_id = gconf_client_notify_add (client, GLOBAL_INTERFACE_KEY,
							     keys_changed_fn,
							     NULL, NULL, NULL);
	}

	engine_list = g_slist_prepend (engine_list, engine);
}

void
bonobo_ui_preferences_remove_engine (BonoboUIEngine *engine)
{
	if (!g_slist_find (engine_list, engine))
		return;

	engine_list = g_slist_remove (engine_list, engine);

	if (engine_list == NULL) {
		/* Remove notification */
		gconf_client_remove_dir (client, GLOBAL_INTERFACE_KEY, NULL);
		gconf_client_notify_remove (client, desktop_notify_id);
		desktop_notify_id = 0;
	}
}


/*
 *   Yes Gconf's C API sucks, yes bonobo-config is a far better
 * way to access configuration, yes I hate this code; Michael.
 */
static gboolean
get (const char *key, gboolean def)
{
	gboolean ret;
	GError  *err = NULL;

	if (!client)					
		client = gconf_client_get_default ();	

	ret = gconf_client_get_bool (client, key, &err);

	if (err) {
		static int warned = 0;
		if (!warned++)
			g_warning ("Failed to get '%s': '%s'", key, err->message);
		g_error_free (err);
		ret = def;
	}

	return ret;
}

int
bonobo_ui_preferences_shutdown (void)
{
	int ret = 0;

	if (client) {
		g_object_unref (client);
		client = NULL;
	}

	ret = gconf_debug_shutdown ();
	if (ret)
		g_warning ("GConf's dirty shutdown");

	return ret;
}

#define DEFINE_BONOBO_UI_PREFERENCE(c_name, prop_name, def)      \
static gboolean cached_## c_name;                                \
\
gboolean                                                         \
bonobo_ui_preferences_get_ ## c_name (void)                      \
{                                                                \
	static gboolean value;					 \
                                                                 \
        if (!cached_##c_name) {                                  \
		value = get ("/desktop/gnome/interface/" prop_name, def); \
                cached_## c_name = TRUE;                                  \
	}                                                                 \
	return value;							  \
}


BonoboUIToolbarStyle
bonobo_ui_preferences_get_toolbar_style (void)
{
	BonoboUIToolbarStyle style;
	char *str;

	if (!client)
		client = gconf_client_get_default ();

	style = BONOBO_UI_TOOLBAR_STYLE_ICONS_AND_TEXT;

	str = gconf_client_get_string (client,
				       "/desktop/gnome/interface/toolbar_style",
				       NULL);
	
	if (str != NULL) {
		gconf_string_to_enum (toolbar_styles,
				      str, (gint *)&style);
		g_free (str);
	}

	return style;
}

DEFINE_BONOBO_UI_PREFERENCE (toolbar_detachable, "toolbar_detachable", TRUE);
DEFINE_BONOBO_UI_PREFERENCE (menus_have_icons,   "menus_have_icons",   TRUE);
DEFINE_BONOBO_UI_PREFERENCE (menus_have_tearoff, "menus_have_tearoff", FALSE);
DEFINE_BONOBO_UI_PREFERENCE (menubar_detachable, "menubar_detachable", TRUE);


static void
keys_changed_fn (GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer user_data)
{
	const char *key_name;

	key_name = gconf_entry_get_key (entry);
	g_return_if_fail (key_name != NULL);

	/* FIXME: update the values instead */
	if (!strcmp (key_name, GLOBAL_INTERFACE_KEY "/toolbar_detachable"))
		cached_toolbar_detachable = FALSE;
	else if (!strcmp (key_name, GLOBAL_INTERFACE_KEY "/menus_have_icons"))
		cached_menus_have_icons = FALSE;
	else if (!strcmp (key_name, GLOBAL_INTERFACE_KEY "/menus_have_tearoff"))
		cached_menus_have_tearoff = FALSE;
	else if (!strcmp (key_name, GLOBAL_INTERFACE_KEY "/menubar_detachable"))
		cached_menubar_detachable = FALSE;

	if (update_engines_idle_id != 0)
		return;

	update_engines_idle_id = gtk_idle_add (update_engines_idle_callback, NULL);
}
