/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-settings.c: 
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors :
 *    Jose M. Celorio <chema@ximian.com>
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 2000-2003 Ximian, Inc.
 *
 */

#include "config.h"
#include <string.h>
#include <libxml/globals.h>
#include "gpa-utils.h"
#include "gpa-reference.h"
#include "gpa-model.h"
#include "gpa-option.h"
#include "gpa-key.h"
#include "gpa-settings.h"
#include "gpa-printer.h"
#include "gpa-root.h"

typedef struct _GPASettingsClass GPASettingsClass;

struct _GPASettingsClass {
	GPANodeClass node_class;
};

static void gpa_settings_class_init (GPASettingsClass *klass);
static void gpa_settings_init (GPASettings *settings);
static void gpa_settings_finalize (GObject *object);

static GPANode * gpa_settings_duplicate (GPANode *node);
static gboolean  gpa_settings_verify    (GPANode *node);
static guchar *  gpa_settings_get_value (GPANode *node);

static GPANodeClass *parent_class = NULL;

GType
gpa_settings_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPASettingsClass),
			NULL, NULL,
			(GClassInitFunc) gpa_settings_class_init,
			NULL, NULL,
			sizeof (GPASettings),
			0,
			(GInstanceInitFunc) gpa_settings_init
		};
		type = g_type_register_static (GPA_TYPE_NODE, "GPASettings", &info, 0);
	}
	return type;
}

static void
gpa_settings_class_init (GPASettingsClass *klass)
{
	GObjectClass *object_class;
	GPANodeClass *node_class;

	object_class = (GObjectClass *) klass;
	node_class = (GPANodeClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gpa_settings_finalize;

	node_class->duplicate = gpa_settings_duplicate;
	node_class->verify    = gpa_settings_verify;
	node_class->get_value = gpa_settings_get_value;
}

static void
gpa_settings_init (GPASettings *settings)
{
	settings->name    = NULL;
	settings->printer = NULL;
	settings->model   = NULL;
}

static void
gpa_settings_finalize (GObject *object)
{
	GPASettings *settings;
	GPANode *node;
	GPANode *child;

	settings = GPA_SETTINGS (object);
	node = GPA_NODE (settings);

	if (settings->printer)
		gpa_node_unref (GPA_NODE (settings->printer));
	gpa_node_unref (GPA_NODE (settings->model));
	settings->printer = NULL;
	settings->model   = NULL;

	g_free (settings->name);
	settings->name = NULL;

	child = GPA_NODE (settings)->children;
	while (child) {
		GPANode *next;
		if (G_OBJECT (child)->ref_count > 1) {
			g_warning ("GPASettings: Child %s has refcount %d\n",
				   GPA_NODE_ID (child), G_OBJECT (child)->ref_count);
		}
		next = child->next;
		gpa_node_detach_unref (child);
		child = next;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GPANode *
gpa_settings_duplicate (GPANode *node)
{
	GPASettings *settings, *new;
	GPANode *child;

	settings = GPA_SETTINGS (node);

	new = (GPASettings *) gpa_node_new (GPA_TYPE_SETTINGS, gpa_node_id (node));

	g_assert (settings->name);
	g_assert (settings->model);
	g_assert (settings->printer);

	new->name    = g_strdup (settings->name);
	new->model   = (GPAReference *) gpa_node_duplicate (GPA_NODE (settings->model));
	new->printer = (GPAReference *) gpa_node_duplicate (GPA_NODE (settings->printer));

	child = GPA_NODE (settings)->children;
	while (child) {
		gpa_node_attach (GPA_NODE (new),
				 gpa_node_duplicate (child));
		child = child->next;
	}
	gpa_node_reverse_children (GPA_NODE (new));

	return (GPANode *) new;
}

static gboolean
gpa_settings_verify (GPANode *node)
{
	if (gpa_node_id (node) == NULL) {
		g_print ("Settings needs to have an ID\n");
		return FALSE;
	}

	return TRUE;
}

static guchar *
gpa_settings_get_value (GPANode *node)
{
	return g_strdup (GPA_SETTINGS (node)->name);
}


/**
 * gpa_settings_append_stock_nodes:
 * @settings: 
 * 
 * Appends keys that should be present on all Settings
 **/
static void
gpa_settings_append_stock_nodes (GPANode *settings)
{
	GPANode *document;
	GPANode *app;
	GPANode *key;

	document = gpa_node_lookup (NULL, "Globals.Document");
	key = gpa_option_create_key (GPA_OPTION (document),
				     settings);
	g_assert (key);
	gpa_node_attach (settings, key);

	app = gpa_node_new (GPA_TYPE_KEY, "Application");
	gpa_node_attach (settings, app);
}

GPANode *
gpa_settings_new (GPAModel *model, const guchar *name, const guchar *id)
{
	GPASettings *settings;
	GPANode *child;
	GPANode *key;
	GSList *list;

	g_return_val_if_fail (model != NULL, NULL);
	g_return_val_if_fail (GPA_IS_MODEL (model), NULL);
	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (*id != '\0', NULL);
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (*name != '\0', NULL);
	g_return_val_if_fail (model->options, NULL);
	g_return_val_if_fail (model->options->children, NULL);

	settings = (GPASettings *) gpa_node_new (GPA_TYPE_SETTINGS, id);

	settings->name  = g_strdup (name);
	settings->model = gpa_reference_new (GPA_NODE (model), "Model");
	settings->printer = NULL; /* This is set by gpa_printer_new () */

	list = NULL;
	child = model->options->children;
	while (child) {
		key = gpa_option_create_key (GPA_OPTION (child),
					     GPA_NODE (settings));
		if (key)
			gpa_node_attach (GPA_NODE (settings), key);
		child = child->next;
	}

	gpa_settings_append_stock_nodes (GPA_NODE (settings));

	gpa_node_reverse_children (GPA_NODE (settings));

	return (GPANode *) settings;
}

GPANode *
gpa_settings_new_from_model_and_tree (GPANode *model, xmlNodePtr tree)
{
	GPASettings *settings;
	xmlChar *settings_id;
	xmlNodePtr xml_node;

	g_return_val_if_fail (model != NULL, NULL);
	g_return_val_if_fail (GPA_IS_MODEL (model), NULL);
	g_return_val_if_fail (tree != NULL, NULL);
	g_return_val_if_fail (!strcmp (tree->name, "Settings"), NULL);

	settings_id = xmlGetProp (tree, "Id");
	g_return_val_if_fail (settings_id != NULL, NULL);

	settings = NULL;
	for (xml_node = tree->xmlChildrenNode; xml_node != NULL; xml_node = xml_node->next) {
		
		/* <Name> */
		if (strcmp (xml_node->name, "Name") == 0) {
			xmlChar *settings_name;
			settings_name = xmlNodeGetContent (xml_node);
			if (!settings_name || !*settings_name) {
				g_warning ("Settings do not contain a valid <Name>\n");
				continue;
			}
			settings = (GPASettings *) gpa_settings_new (GPA_MODEL (model), settings_name, settings_id);
			xmlFree (settings_name);
			continue;
		}

		/* <Key> */
		if (strcmp (xml_node->name, "Key") == 0) {
			xmlChar *key_id;
			GPANode *key;
			
			if (!settings) {
				g_print ("Can't have <Key> before <Name> in settings\n");
				continue;
			}
			
			key_id = xmlGetProp (xml_node, "Id");

			if (!key_id || !*key_id) {
				g_warning ("Invalid Key id while parsing settings %s\n", settings_id);
				xmlFree (key_id);
				continue;
			}

			key = GPA_NODE (settings)->children;
			while (key) {
				if (GPA_NODE_ID_COMPARE (key, key_id)) {
					gpa_key_merge_from_tree (key, xml_node);
					break;
				}
				key = key->next;
			}
			xmlFree (key_id);
		}
	}

	if (!settings) {
		g_warning ("Could not create the \"%s\" settings.\n", settings_id);
	}
	xmlFree (settings_id);

	return (GPANode *) settings;
}

xmlNodePtr
gpa_settings_to_tree (GPASettings *settings)
{
	GPANode *child;
	xmlNodePtr node, key;

	g_return_val_if_fail (settings != NULL, NULL);
	g_return_val_if_fail (GPA_IS_SETTINGS (settings), NULL);

	node = xmlNewNode (NULL, "Settings");
	xmlSetProp  (node, "Id", GPA_NODE_ID (settings));
	xmlSetProp  (node, "Model",   GPA_NODE_ID (GPA_REFERENCE_REFERENCE (settings->model)));
	xmlSetProp  (node, "Printer", GPA_NODE_ID (GPA_REFERENCE_REFERENCE (settings->printer)));
	xmlNewChild (node, NULL, "Name",    settings->name);

	child = GPA_NODE (settings)->children;
	while (child) {
		key = gpa_key_to_tree (GPA_KEY (child));
		if (key)
			xmlAddChild (node, key);
		child = child->next;
	}
	
	return node;
}

gboolean
gpa_settings_copy (GPASettings *dst, GPASettings *src)
{
	GPANode *child;
	GSList *sl, *dl;

	g_return_val_if_fail (dst != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_SETTINGS (dst), FALSE);
	g_return_val_if_fail (src != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_SETTINGS (src), FALSE);

	g_return_val_if_fail (src->printer != NULL, FALSE);
	g_return_val_if_fail (dst->printer != NULL, FALSE);
	g_return_val_if_fail (src->model != NULL, FALSE);
	g_return_val_if_fail (dst->model != NULL, FALSE);

	dst->name = g_strdup (src->name);
	
	gpa_reference_set_reference (GPA_REFERENCE (dst->printer), GPA_REFERENCE_REFERENCE (src->printer));
	gpa_reference_set_reference (GPA_REFERENCE (dst->model),   GPA_REFERENCE_REFERENCE (src->model));

	dl = NULL;
	child = GPA_NODE (dst)->children;
	while (child) {
		dl = g_slist_prepend (dl, child);
		gpa_node_detach (child);
		child = child->next;
	}

	sl = NULL;
	child = GPA_NODE (src)->children;
	while (child) {
		sl = g_slist_prepend (sl, child);
		child = child->next;
	}

	while (sl) {
		GSList *l;
		for (l = dl; l != NULL; l = l->next) {
			if (GPA_NODE_ID_COMPARE (l->data, sl->data)) {
				/* We are in original too */
				child = GPA_NODE (l->data);
				dl = g_slist_remove (dl, l->data);
				gpa_node_attach (GPA_NODE (dst), child);
				gpa_key_merge_from_key (GPA_KEY (child), GPA_KEY (sl->data));
				break;
			}
		}
		if (!l) {
			/* Create new child */
			child = gpa_node_duplicate (GPA_NODE (sl->data));
			gpa_node_attach (GPA_NODE (dst), child);
		}
		sl = g_slist_remove (sl, sl->data);
	}

	while (dl) {
		gpa_node_unref (GPA_NODE (dl->data));
		dl = g_slist_remove (dl, dl->data);
	}

	return TRUE;
}

