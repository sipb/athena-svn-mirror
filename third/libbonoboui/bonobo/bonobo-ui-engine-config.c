/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-ui-engine-config.c: The Bonobo UI/XML Sync engine user config code
 *
 * Author:
 *	Michael Meeks (michael@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */

#include "config.h"
#include <stdlib.h>
#include <string.h>

#include <glib/gmacros.h>
#include <gtk/gtk.h>

#include <bonobo/bonobo-i18n.h>
#include <bonobo/bonobo-ui-util.h>
#include <bonobo/bonobo-ui-private.h>
#include <bonobo/bonobo-ui-sync-menu.h>
#include <bonobo/bonobo-ui-config-widget.h>
#include <bonobo/bonobo-ui-engine-config.h>
#include <bonobo/bonobo-ui-engine-private.h>

#include <gconf/gconf-client.h>

#define PARENT_TYPE G_TYPE_OBJECT

static GObjectClass *parent_class = NULL;

struct _BonoboUIEngineConfigPrivate {
	char           *path; 

	GtkWindow      *opt_parent;

	BonoboUIEngine *engine;
	BonoboUIXml    *tree;

	GSList         *clobbers;

	GtkWidget      *dialog;
};

typedef struct {
	char *path;
	char *attr;
	char *value;
} clobber_t;

static void
clobber_destroy (BonoboUIXml *tree, clobber_t *cl)
{
	if (cl) {
		bonobo_ui_xml_remove_watch_by_data (tree, cl);

		g_free (cl->path);
		cl->path = NULL;

		g_free (cl->attr);
		cl->attr = NULL;

		g_free (cl->value);
		cl->value = NULL;

		g_free (cl);
	}
}

static void
clobbers_free (BonoboUIEngineConfig *config)
{
	GSList *l;

	for (l = config->priv->clobbers; l; l = l->next)
		clobber_destroy (config->priv->tree, l->data);

	g_slist_free (config->priv->clobbers);
	config->priv->clobbers = NULL;
}

void
bonobo_ui_engine_config_serialize (BonoboUIEngineConfig *config)
{
	GSList      *l;
	GSList      *values = NULL;
	GConfClient *client;

	g_return_if_fail (config->priv->path != NULL);

	for (l = config->priv->clobbers; l; l = l->next) {
		clobber_t   *cl = l->data;
		char        *str;

		/* This sucks, but so does gconf */
		str = g_strconcat (cl->path, ":",
				   cl->attr, ":",
				   cl->value, NULL);

		values = g_slist_prepend (values, str);
	}

	client = gconf_client_get_default ();
	
	gconf_client_set_list (
		client, config->priv->path,
		GCONF_VALUE_STRING, values, NULL);

	g_slist_foreach (values, (GFunc) g_free, NULL);
	g_slist_free (values);

	gconf_client_suggest_sync (client, NULL);

	g_object_unref (client);
}

static void
clobber_add (BonoboUIEngineConfig *config,
	     const char           *path,
	     const char           *attr,
	     const char           *value)
{
	clobber_t *cl = g_new0 (clobber_t, 1);

	cl->path  = g_strdup (path);
	cl->attr  = g_strdup (attr);
	cl->value = g_strdup (value);

	config->priv->clobbers = g_slist_prepend (
		config->priv->clobbers, cl);

	bonobo_ui_xml_add_watch (config->priv->tree, path, cl);
}

void
bonobo_ui_engine_config_add (BonoboUIEngineConfig *config,
			     const char           *path,
			     const char           *attr,
			     const char           *value)
{
	BonoboUINode *node;

	bonobo_ui_engine_config_remove (config, path, attr);

	clobber_add (config, path, attr, value);

	if ((node = bonobo_ui_xml_get_path (config->priv->tree, path))) {
		const char *existing;
		gboolean    set = TRUE;

		if ((existing = bonobo_ui_node_peek_attr (node, attr))) {
			if (!strcmp (existing, value))
				set = FALSE;
		}

		if (set) {
			bonobo_ui_node_set_attr (node, attr, value);
			bonobo_ui_xml_set_dirty (config->priv->tree, node);
			bonobo_ui_engine_update (config->priv->engine);
		}
	}
}

void
bonobo_ui_engine_config_remove (BonoboUIEngineConfig *config,
				const char           *path,
				const char           *attr)
{
	GSList *l, *next;
	BonoboUINode *node;

	for (l = config->priv->clobbers; l; l = next) {
		clobber_t *cl = l->data;

		next = l->next;

		if (!strcmp (cl->path, path) &&
		    !strcmp (cl->attr, attr)) {
			config->priv->clobbers = g_slist_remove (
				config->priv->clobbers, cl);
			clobber_destroy (config->priv->tree, cl);
		}
	}

	if ((node = bonobo_ui_xml_get_path (config->priv->tree, path))) {

		if (bonobo_ui_node_has_attr (node, attr)) {
			bonobo_ui_node_remove_attr (node, attr);
			bonobo_ui_xml_set_dirty (config->priv->tree, node);
			bonobo_ui_engine_update (config->priv->engine);
		}
	}
}

void
bonobo_ui_engine_config_hydrate (BonoboUIEngineConfig *config)
{
	GSList *l, *values;
	GConfClient *client;

	g_return_if_fail (config->priv->path != NULL);

	bonobo_ui_engine_freeze (config->priv->engine);

	clobbers_free (config);

	client = gconf_client_get_default ();

	values = gconf_client_get_list (
		client, config->priv->path, GCONF_VALUE_STRING, NULL);

	for (l = values; l; l = l->next) {
		char **strs = g_strsplit (l->data, ":", -1);

		if (!strs || !strs [0] || !strs [1] || !strs [2] || strs [3])
			g_warning ("Syntax error in '%s'", (char *) l->data);
		else
			bonobo_ui_engine_config_add (
				config, strs [0], strs [1], strs [2]);

		g_strfreev (strs);
		g_free (l->data);
	}

	g_slist_free (values);

	bonobo_ui_engine_thaw (config->priv->engine);

	g_object_unref (client);
}

typedef struct {
	BonoboUIEngine *engine;
	char           *path;
	BonoboUIEngineConfigFn     config_fn;
	BonoboUIEngineConfigVerbFn verb_fn;
} closure_t;

static void
closure_destroy (closure_t *c)
{
	g_free (c->path);
	g_free (c);
}

static void
emit_verb_on_cb (BonoboUIEngine *engine,
		 BonoboUINode   *popup_node,
		 closure_t      *c)
{
	if (c->verb_fn)
		c->verb_fn (bonobo_ui_engine_get_config (c->engine),
			    c->path, NULL, engine, popup_node);
}


static void
emit_event_on_cb (BonoboUIEngine *engine,
		  BonoboUINode   *popup_node,
		  const char     *state,
		  closure_t      *c)
{
	if (c->verb_fn)
		c->verb_fn (bonobo_ui_engine_get_config (c->engine),
			    c->path, state, engine, popup_node);
}

static BonoboUIEngine *
create_popup_engine (closure_t *c,
		     GtkMenu   *menu)
{
	BonoboUIEngine *engine;
	BonoboUISync   *smenu;
	BonoboUINode   *node;
	char           *str;

	engine = bonobo_ui_engine_new (NULL);
	smenu  = bonobo_ui_sync_menu_new (engine, NULL, NULL, NULL);

	bonobo_ui_engine_add_sync (engine, smenu);

	node = bonobo_ui_engine_get_path (c->engine, c->path);
	if (c->config_fn)
		str = c->config_fn (
			bonobo_ui_engine_get_config (c->engine),
			node, engine);
	else
		str = NULL;

	g_return_val_if_fail (str != NULL, NULL);

	node = bonobo_ui_node_from_string (str);
	bonobo_ui_util_translate_ui (node);
	bonobo_ui_engine_xml_merge_tree (
		engine, "/", node, "popup");

	bonobo_ui_sync_menu_add_popup (
		BONOBO_UI_SYNC_MENU (smenu),
		menu, "/popups/popup");

	g_signal_connect (G_OBJECT (engine),
			  "emit_verb_on",
			  (GCallback) emit_verb_on_cb, c);

	g_signal_connect (G_OBJECT (engine),
			  "emit_event_on",
			  (GCallback) emit_event_on_cb, c);

	bonobo_ui_engine_update (engine);

	return engine;
}

static int
config_button_pressed (GtkWidget      *widget,
		       GdkEventButton *event,
		       closure_t      *c)
{
	if (event->button == 3) {
		GtkWidget *menu;

		menu = gtk_menu_new ();

		create_popup_engine (c, GTK_MENU (menu));

		gtk_widget_show (GTK_WIDGET (menu));

		gtk_menu_popup (GTK_MENU (menu), NULL, NULL,
				NULL, NULL, 3, 0);

		return TRUE;
	} else
		return FALSE;
}

void
bonobo_ui_engine_config_connect (GtkWidget      *widget,
				 BonoboUIEngine *engine,
				 const char     *path,
				 BonoboUIEngineConfigFn     config_fn,
				 BonoboUIEngineConfigVerbFn verb_fn)
{
	BonoboUIEngineConfig *config;
	closure_t *c;

	config = bonobo_ui_engine_get_config (engine);
	if (!config || !config->priv->path)
		return;

	c = g_new0 (closure_t, 1);
	c->engine    = engine;
	c->path      = g_strdup (path);
	c->config_fn = config_fn;
	c->verb_fn   = verb_fn;

	g_signal_connect_data (
		widget, "button_press_event",
		G_CALLBACK (config_button_pressed),
		c,
		(GClosureNotify) closure_destroy, 0);
}

static void
bonobo_ui_engine_config_watch (BonoboUIXml    *xml,
			       const char     *path,
			       BonoboUINode   *opt_node,
			       gpointer        user_data)
{
	clobber_t *cl = user_data;

	if (opt_node) {
/*		g_warning ("Setting attr '%s' to '%s' on '%s",
		cl->attr, cl->value, path);*/
		bonobo_ui_node_set_attr (opt_node, cl->attr, cl->value);
	} else
		g_warning ("Stamp new config data onto NULL @ '%s'", path);
}

static void
impl_finalize (GObject *object)
{
	BonoboUIEngineConfig *config;
	BonoboUIEngineConfigPrivate *priv;

	config = BONOBO_UI_ENGINE_CONFIG (object);
	priv = config->priv;

	if (priv->dialog)
		gtk_widget_destroy (priv->dialog);

	g_free (priv->path);

	clobbers_free (config);

	g_free (priv);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
class_init (BonoboUIEngineClass *engine_class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (engine_class);

	object_class = G_OBJECT_CLASS (engine_class);

	object_class->finalize = impl_finalize;
}

static void
init (BonoboUIEngineConfig *config)
{
	BonoboUIEngineConfigPrivate *priv;

	priv = g_new0 (BonoboUIEngineConfigPrivate, 1);

	config->priv = priv;
}

GType
bonobo_ui_engine_config_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (BonoboUIEngineConfigClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (BonoboUIEngineConfig),
			0, /* n_preallocs */
			(GInstanceInitFunc) init
		};

		type = g_type_register_static (PARENT_TYPE, "BonoboUIEngineConfig",
					       &info, 0);
	}

	return type;
}

BonoboUIEngineConfig *
bonobo_ui_engine_config_construct (BonoboUIEngineConfig *config,
				   BonoboUIEngine       *engine,
				   GtkWindow            *opt_parent)
{
	config->priv->engine = engine;
	config->priv->tree   = bonobo_ui_engine_get_xml (engine);
	config->priv->opt_parent = opt_parent;

	bonobo_ui_xml_set_watch_fn (
		bonobo_ui_engine_get_xml (engine),
		bonobo_ui_engine_config_watch);

	return config;
}

BonoboUIEngineConfig *
bonobo_ui_engine_config_new (BonoboUIEngine *engine,
			     GtkWindow      *opt_parent)
{
	BonoboUIEngineConfig *config;

	g_return_val_if_fail (BONOBO_IS_UI_ENGINE (engine), NULL);

	config = g_object_new (bonobo_ui_engine_config_get_type (), NULL);

	return bonobo_ui_engine_config_construct (config, engine, opt_parent);
}

void
bonobo_ui_engine_config_set_path (BonoboUIEngine *engine,
				  const char     *path)
{
	BonoboUIEngineConfig *config;

	g_return_if_fail (BONOBO_IS_UI_ENGINE (engine));

	config = bonobo_ui_engine_get_config (engine);

	g_free (config->priv->path);
	config->priv->path = g_strdup (path);

	bonobo_ui_engine_config_hydrate (config);
}

const char *
bonobo_ui_engine_config_get_path (BonoboUIEngine *engine)
{
	BonoboUIEngineConfig *config;

	g_return_val_if_fail (BONOBO_IS_UI_ENGINE (engine), NULL);

	config = bonobo_ui_engine_get_config (engine);
	
	return config->priv->path;
}

static void
response_fn (GtkDialog            *dialog,
	     gint                  response_id,
	     BonoboUIEngineConfig *config)
{
	bonobo_ui_engine_config_serialize (config);

	gtk_widget_destroy (GTK_WIDGET (dialog));
}

static GtkWidget *
dialog_new (BonoboUIEngineConfig *config)
{
	GtkAccelGroup *accel_group;
	GtkWidget     *window, *cwidget;

	accel_group = gtk_accel_group_new ();

	window = gtk_dialog_new_with_buttons (_("Configure UI"), 
					      config->priv->opt_parent, 0,
					      GTK_STOCK_OK, GTK_RESPONSE_OK,
					      NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (window), GTK_RESPONSE_OK);

	g_signal_connect (window, "response",
			  G_CALLBACK (response_fn), config);

	cwidget = bonobo_ui_config_widget_new (config->priv->engine, accel_group);
	gtk_widget_show (cwidget);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (window)->vbox), cwidget);

	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);
	
	return window;
}

static void
null_dialog (GtkObject *object, 
	     BonoboUIEngineConfig *config)
{
	config->priv->dialog = NULL;
}

void
bonobo_ui_engine_config_configure (BonoboUIEngineConfig *config)
{
	if (!config->priv->path)
		return;

	/* Fire up a single non-modal dialog */
	if (config->priv->dialog) {
		gtk_window_activate_focus (
			GTK_WINDOW (config->priv->dialog));
		return;
	}

	config->priv->dialog = dialog_new (config);
	gtk_window_set_default_size (
		GTK_WINDOW (config->priv->dialog), 300, 300);
	gtk_widget_show (config->priv->dialog);
	g_signal_connect (GTK_OBJECT (config->priv->dialog),
			    "destroy", (GtkSignalFunc) null_dialog, config);
}

BonoboUIEngine *
bonobo_ui_engine_config_get_engine (BonoboUIEngineConfig *config)
{
	g_return_val_if_fail (BONOBO_IS_UI_ENGINE_CONFIG (config), NULL);

	return config->priv->engine;
}
