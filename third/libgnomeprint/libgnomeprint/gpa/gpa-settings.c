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
 *  Copyright (C) 2000-2001 Ximian, Inc. and Jose M. Celorio
 *
 */

#define __GPA_SETTINGS_C__

#include <string.h>
#include <libxml/globals.h>
#include "gpa-utils.h"
#include "gpa-value.h"
#include "gpa-reference.h"
#include "gpa-model.h"
#include "gpa-option.h"
#include "gpa-key.h"
#include "gpa-settings.h"

static void gpa_settings_class_init (GPASettingsClass *klass);
static void gpa_settings_init (GPASettings *settings);

static void gpa_settings_finalize (GObject *object);

static GPANode *gpa_settings_duplicate (GPANode *node);
static gboolean gpa_settings_verify (GPANode *node);
static guchar *gpa_settings_get_value (GPANode *node);
static GPANode *gpa_settings_get_child (GPANode *node, GPANode *ref);
static GPANode *gpa_settings_lookup (GPANode *node, const guchar *path);
static void gpa_settings_modified (GPANode *node, guint flags);

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
	node_class->verify = gpa_settings_verify;
	node_class->get_value = gpa_settings_get_value;
	node_class->get_child = gpa_settings_get_child;
	node_class->lookup = gpa_settings_lookup;
	node_class->modified = gpa_settings_modified;
}

static void
gpa_settings_init (GPASettings *settings)
{
	settings->name = NULL;
	settings->model = NULL;
	settings->keys = NULL;
}

static void
gpa_settings_finalize (GObject *object)
{
	GPASettings *settings;

	settings = GPA_SETTINGS (object);

	settings->name = gpa_node_detach_unref (GPA_NODE (settings), GPA_NODE (settings->name));
	settings->model = gpa_node_detach_unref (GPA_NODE (settings), GPA_NODE (settings->model));

	while (settings->keys) {
		if (G_OBJECT (settings->keys)->ref_count > 1) {
			g_warning ("GPASettings: Child %s has refcount %d\n", GPA_NODE_ID (settings->keys), G_OBJECT (settings->keys)->ref_count);
		}
		settings->keys = gpa_node_detach_unref_next (GPA_NODE (settings), GPA_NODE (settings->keys));
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GPANode *
gpa_settings_duplicate (GPANode *node)
{
	GPASettings *settings, *new;
	GPANode *child;
	GSList *l;

	settings = GPA_SETTINGS (node);

	new = (GPASettings *) gpa_node_new (GPA_TYPE_SETTINGS, GPA_NODE_ID (node));

	if (settings->name) {
		new->name = gpa_node_attach (GPA_NODE (new), gpa_node_duplicate (settings->name));
	}

	if (settings->model) {
		new->model = gpa_node_attach (GPA_NODE (new), gpa_node_duplicate (settings->model));
	}

	l = NULL;
	for (child = settings->keys; child != NULL; child = child->next) {
		GPANode *newchild;
		newchild = gpa_node_duplicate (child);
		if (newchild) l = g_slist_prepend (l, newchild);
	}

	while (l) {
		GPANode *newchild;
		newchild = GPA_NODE (l->data);
		l = g_slist_remove (l, newchild);
		newchild->parent = GPA_NODE (new);
		newchild->next = new->keys;
		new->keys = newchild;
	}

	return GPA_NODE (new);
}

static gboolean
gpa_settings_verify (GPANode *node)
{
	/* fixme: Verify on option */

	return TRUE;
}

static guchar *
gpa_settings_get_value (GPANode *node)
{
	/* fixme: */

	return NULL;
}

static GPANode *
gpa_settings_get_child (GPANode *node, GPANode *ref)
{
	GPASettings *settings;
	GPANode *child;

	settings = GPA_SETTINGS (node);

	child = NULL;
	if (!ref) {
		child = settings->name;
	} else if (ref == settings->name) {
		child = settings->model;
	} else if (ref == settings->model) {
		child = settings->keys;
	} else {
		if (ref->next) child = ref->next;
	}

	if (child) gpa_node_ref (child);

	return child;
}

static GPANode *
gpa_settings_lookup (GPANode *node, const guchar *path)
{
	GPASettings *settings;
	GPANode *child;
	const guchar *dot, *next;
	gint len;

	settings = GPA_SETTINGS (node);

	child = NULL;

	if (gpa_node_lookup_ref (&child, GPA_NODE (settings->name), path, "Name")) return child;
	if (gpa_node_lookup_ref (&child, GPA_NODE (settings->model), path, "Model")) return child;

	dot = strchr (path, '.');
	if (dot != NULL) {
		len = dot - path;
		next = dot + 1;
	} else {
		len = strlen (path);
		next = path + len;
	}

	for (child = settings->keys; child != NULL; child = child->next) {
		const guchar *cid;
		g_assert (GPA_IS_KEY (child));
		cid = GPA_NODE_ID (child);
		if (cid && strlen (cid) == len && !strncmp (cid, path, len)) {
			if (!next) {
				gpa_node_ref (child);
				return child;
			} else {
				return gpa_node_lookup (child, next);
			}
		}
	}

	return NULL;
}

static void
gpa_settings_modified (GPANode *node, guint flags)
{
	GPASettings *settings;
	GPANode *child;

	settings = GPA_SETTINGS (node);

	if (settings->name && GPA_NODE_FLAGS (settings->name) & GPA_MODIFIED_FLAG) {
		gpa_node_emit_modified (settings->name, 0);
	}
	if (settings->model && GPA_NODE_FLAGS (settings->model) & GPA_MODIFIED_FLAG) {
		gpa_node_emit_modified (settings->model, 0);
	}

	child = settings->keys;
	while (child) {
		GPANode *next;
		next = child->next;
		if (GPA_NODE_FLAGS (child) & GPA_MODIFIED_FLAG) {
			gpa_node_ref (child);
			gpa_node_emit_modified (child, 0);
			gpa_node_unref (child);
		}
		child = next;
	}
}

GPANode *
gpa_settings_new_empty (const guchar *name)
{
	GPASettings *settings;

	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (*name != '\0', NULL);

	settings = (GPASettings *) gpa_node_new (GPA_TYPE_SETTINGS, NULL);

	settings->name = gpa_value_new ("Name", name);
	settings->name->parent = GPA_NODE (settings);
	settings->model = gpa_reference_new_empty ();
	settings->model->parent = GPA_NODE (settings);

	return GPA_NODE (settings);
}

GPANode *
gpa_settings_new_from_model (GPANode *model, const guchar *name)
{
	GPASettings *settings;
	guchar *id;

	g_return_val_if_fail (model != NULL, NULL);
	g_return_val_if_fail (GPA_IS_MODEL (model), NULL);
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (*name != '\0', NULL);
	g_return_val_if_fail (gpa_node_verify (model), NULL);
	g_return_val_if_fail (GPA_MODEL_ENSURE_LOADED (model), NULL);

	id = gpa_id_new ("SETTINGS");
	settings = (GPASettings *) gpa_settings_new_from_model_full (model, id, name);
	g_free (id);

	return (GPANode *) settings;
}

GPANode *
gpa_settings_new_from_model_full (GPANode *model, const guchar *id, const guchar *name)
{
	GPASettings *settings;
	GPANode *child;
	GSList *l;

	g_return_val_if_fail (model != NULL, NULL);
	g_return_val_if_fail (GPA_IS_MODEL (model), NULL);
	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (*id != '\0', NULL);
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (*name != '\0', NULL);
	g_return_val_if_fail (gpa_node_verify (GPA_NODE (model)), NULL);
	g_return_val_if_fail (GPA_MODEL_ENSURE_LOADED (model), NULL);

	settings = (GPASettings *) gpa_node_new (GPA_TYPE_SETTINGS, id);

	settings->name = gpa_node_attach (GPA_NODE (settings), gpa_value_new ("Name", name));
	settings->model = gpa_node_attach (GPA_NODE (settings), gpa_reference_new (model));

	l = NULL;
	for (child = GPA_LIST (GPA_MODEL (model)->options)->children; child != NULL; child = child->next) {
		GPANode *key;
		key = gpa_key_new_from_option (child);
		if (key) l = g_slist_prepend (l, key);
	}

	while (l) {
		GPANode *key;
		key = GPA_NODE (l->data);
		l = g_slist_remove (l, key);
		key->parent = GPA_NODE (settings);
		key->next = settings->keys;
		settings->keys = key;
	}

	return (GPANode *) settings;
}

GPANode *
gpa_settings_new_from_model_and_tree (GPANode *model, xmlNodePtr tree)
{
	GPASettings *settings;
	xmlChar *xmlid;
	xmlNodePtr xmlc;

	g_return_val_if_fail (model != NULL, NULL);
	g_return_val_if_fail (GPA_IS_MODEL (model), NULL);
	g_return_val_if_fail (tree != NULL, NULL);
	g_return_val_if_fail (gpa_node_verify (GPA_NODE (model)), NULL);
	g_return_val_if_fail (GPA_MODEL_ENSURE_LOADED (model), NULL);
	g_return_val_if_fail (!strcmp (tree->name, "Settings"), NULL);

	xmlid = xmlGetProp (tree, "Id");
	g_return_val_if_fail (xmlid != NULL, NULL);

	settings = NULL;
	for (xmlc = tree->xmlChildrenNode; xmlc != NULL; xmlc = xmlc->next) {
		if (!strcmp (xmlc->name, "Name")) {
			xmlChar *content;
			content = xmlNodeGetContent (xmlc);
			if (content && *content) {
				settings = (GPASettings *) gpa_settings_new_from_model_full (model, xmlid, content);
				xmlFree (content);
			}
		} else if (!strcmp (xmlc->name, "Key") && settings) {
			xmlChar *keyid;
			keyid = xmlGetProp (xmlc, "Id");
			if (keyid) {
				GPANode *key;
				for (key = settings->keys; key != NULL; key = key->next) {
					if (GPA_NODE_ID_COMPARE (key, keyid)) {
						gpa_key_merge_from_tree (key, xmlc);
						break;
					}
				}
				xmlFree (keyid);
			}
		}
	}

	xmlFree (xmlid);

	if (!settings) {
		g_warning ("Settings node does not have valid <Name> tag");
	}

	return (GPANode *) settings;
}

xmlNodePtr
gpa_settings_write (xmlDocPtr doc, GPANode *node)
{
	GPASettings *settings;
	xmlNodePtr xmln, xmlc;
	GPANode *child;

	settings = GPA_SETTINGS (node);

	xmln = xmlNewDocNode (doc, NULL, "Settings", NULL);
	xmlSetProp (xmln, "Id", GPA_NODE_ID (node));

	xmlc = xmlNewChild (xmln, NULL, "Name", GPA_VALUE (settings->name)->value);

	for (child = settings->keys; child != NULL; child = child->next) {
		xmlc = gpa_key_write (doc, child);
		if (xmlc) xmlAddChild (xmln, xmlc);
	}

	return xmln;
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

	g_return_val_if_fail (GPA_VALUE_VALUE (src->name), FALSE);
	g_return_val_if_fail (GPA_VALUE_VALUE (dst->name), FALSE);

	g_return_val_if_fail (src->model != NULL, FALSE);
	g_return_val_if_fail (dst->model != NULL, FALSE);

	gpa_value_set_value_forced (GPA_VALUE (dst->name), GPA_VALUE_VALUE (src->name));

	gpa_reference_set_reference (GPA_REFERENCE (dst->model), GPA_REFERENCE_REFERENCE (src->model));

	dl = NULL;
	while (dst->keys) {
		dl = g_slist_prepend (dl, dst->keys);
		dst->keys = gpa_node_detach_next (GPA_NODE (dst), dst->keys);
	}

	sl = NULL;
	for (child = src->keys; child != NULL; child = child->next) {
		sl = g_slist_prepend (sl, child);
	}

	while (sl) {
		GSList *l;
		for (l = dl; l != NULL; l = l->next) {
			if (GPA_NODE (l->data)->qid && (GPA_NODE (l->data)->qid == GPA_NODE (sl->data)->qid)) {
				/* We are in original too */
				child = GPA_NODE (l->data);
				dl = g_slist_remove (dl, l->data);
				child->parent = GPA_NODE (dst);
				child->next = dst->keys;
				dst->keys = child;
				gpa_key_merge_from_key (GPA_KEY (child), GPA_KEY (sl->data));
				break;
			}
		}
		if (!l) {
			/* Create new child */
			child = gpa_node_duplicate (GPA_NODE (sl->data));
			child->parent = GPA_NODE (dst);
			child->next = dst->keys;
			dst->keys = child;
		}
		sl = g_slist_remove (sl, sl->data);
	}

	while (dl) {
		gpa_node_unref (GPA_NODE (dl->data));
		dl = g_slist_remove (dl, dl->data);
	}

	gpa_node_request_modified (GPA_NODE (dst), 0);

	return TRUE;
}

