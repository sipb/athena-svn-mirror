/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-key.c: 
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
 *    Lauris Kaplinski <lauris@ximian.com>
 *    Chema Celorio <chema@ximian.com>
 *
 *  Copyright (C) 2000-2003 Ximian, Inc.
 *
 */

#include <config.h>

#include <string.h>
#include <libxml/globals.h>

#include "gpa-utils.h"
#include "gpa-reference.h"
#include "gpa-option.h"
#include "gpa-key.h"

static void gpa_key_class_init (GPAKeyClass *klass);
static void gpa_key_init (GPAKey *key);

static void gpa_key_finalize (GObject *object);

static GPANode * gpa_key_duplicate (GPANode *node);
static gboolean  gpa_key_verify    (GPANode *node);
static guchar *  gpa_key_get_value (GPANode *node);
static gboolean  gpa_key_set_value (GPANode *node, const guchar *value);

static gboolean gpa_key_merge_children_from_key (GPAKey *dst, GPAKey *src);
static gboolean gpa_key_merge_from_option (GPAKey *key, GPAOption *option);
static gboolean gpa_key_merge_children_from_option (GPAKey *key, GPAOption *option);

static GPANodeClass *parent_class = NULL;

GType
gpa_key_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPAKeyClass),
			NULL, NULL,
			(GClassInitFunc) gpa_key_class_init,
			NULL, NULL,
			sizeof (GPAKey),
			0,
			(GInstanceInitFunc) gpa_key_init
		};
		type = g_type_register_static (GPA_TYPE_NODE, "GPAKey", &info, 0);
	}
	return type;
}

static void
gpa_key_class_init (GPAKeyClass *klass)
{
	GObjectClass *object_class;
	GPANodeClass *node_class;

	object_class = (GObjectClass *) klass;
	node_class = (GPANodeClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gpa_key_finalize;

	node_class->duplicate = gpa_key_duplicate;
	node_class->verify    = gpa_key_verify;
	node_class->get_value = gpa_key_get_value;
	node_class->set_value = gpa_key_set_value;
}

static void
gpa_key_init (GPAKey *key)
{
	key->option   = NULL;
	key->value    = NULL;
}

static void
gpa_key_finalize (GObject *object)
{
	GPANode *child;
	GPANode *node;
	GPAKey *key;

	key = GPA_KEY (object);
	node = GPA_NODE (key);

	child = node->children;
	while (child) {
		GPANode *next;
		if (G_OBJECT (child)->ref_count > 1) {
			g_warning ("GPAKey: Child %s has refcount %d\n",
				   gpa_node_id (child), G_OBJECT (child)->ref_count);
		}
		next = child->next;
		gpa_node_detach_unref (child);
		child = next;
	}
	node->children = NULL;

	if (key->option)
		gpa_node_unref (key->option);
	if (key->value)
		g_free (key->value);

	key->value = NULL;
	key->option = NULL;

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GPANode *
gpa_key_duplicate (GPANode *node)
{
	GPAKey *key, *new;
	GPANode *child;

	key = GPA_KEY (node);

	new = (GPAKey *) gpa_node_new (GPA_TYPE_KEY, gpa_node_id (node));

	if ((GPA_NODE_FLAGS (node) & NODE_FLAG_LOCKED) == NODE_FLAG_LOCKED) {
		GPA_NODE_SET_FLAGS (key, NODE_FLAG_LOCKED);
	}
	
	if (key->value)
		new->value = g_strdup (key->value);
	if (key->option)
		new->option = gpa_node_ref (key->option);

	child = GPA_NODE (key)->children;
	while (child) {
		gpa_node_attach (GPA_NODE (new),
				 gpa_node_duplicate (child));
		child = child->next;
	}

	gpa_node_reverse_children (GPA_NODE (new));

	return GPA_NODE (new);
}

static gboolean
gpa_key_verify (GPANode *node)
{
	gpa_return_false_if_fail (GPA_IS_KEY (node));
	gpa_return_false_if_fail (GPA_IS_OPTION (GPA_KEY (node)->option));

	return TRUE;
}

static guchar *
gpa_key_get_value (GPANode *node)
{
	GPAKey *key;

	key = GPA_KEY (node);

	if (key->value)
		return g_strdup (key->value);

	return NULL;
}

static gboolean
gpa_key_set_value (GPANode *node, const guchar *value)
{
	GPAOption *option;
	GPANode *child;
	GPAKey *key;

	g_return_val_if_fail (node != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_KEY (node), FALSE);
	g_return_val_if_fail (value != NULL, FALSE);
	g_return_val_if_fail (GPA_KEY (node)->value != NULL, FALSE);

	key    = GPA_KEY (node);
	option = GPA_KEY_OPTION (key);

	if (option == NULL) {
		   g_free (key->value);
		   key->value = g_strdup (value);
		   return TRUE;
	}
	
	g_return_val_if_fail (option != NULL, FALSE);

	if (strcmp (key->value, value) == 0)
		return FALSE;

	switch (option->type) {
	case GPA_OPTION_TYPE_LIST:
		child = gpa_option_get_child_by_id (option, value);
		if (!child) {
			g_warning ("Could not set value of \"%s\" to \"%s\"", GPA_NODE_ID (option), value);
			return FALSE;
		}
		g_free (key->value);
		key->value = g_strdup (value);
		gpa_key_merge_children_from_option (key, GPA_OPTION (child));
		gpa_node_unref (child);
		break;
	case GPA_OPTION_TYPE_STRING:
	case GPA_OPTION_TYPE_KEY:
		g_free (key->value);
		key->value = g_strdup (value);
		break;
	default:
		g_warning ("Cant set value of %s to %s, set value for type not set. Current val:%s (%d)",
			   gpa_node_id (node), value, key->value, option->type);
		return FALSE;
	}

	return TRUE;
}

xmlNodePtr
gpa_key_to_tree (GPAKey *key)
{
	xmlNodePtr node = NULL;
	GPANode *child;

	g_return_val_if_fail (GPA_IS_KEY (key), NULL);
	
	node = xmlNewNode (NULL, "Key");
	xmlSetProp (node, "Id", GPA_NODE_ID (key));
	if (key->value)
		xmlSetProp (node, "Value", key->value);

	child = GPA_NODE (key)->children;
	while (child) {
		xmlAddChild (node,
			     gpa_key_to_tree (GPA_KEY (child)));
		child = child->next;
	}

	return node;
}

gboolean
gpa_key_merge_from_tree (GPANode *key, xmlNodePtr tree)
{
	xmlNodePtr node;
	xmlChar *value;
	xmlChar *id;

	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_KEY (key), FALSE);
	g_return_val_if_fail (tree != NULL, FALSE);
	g_return_val_if_fail (strcmp (tree->name, "Key") == 0, FALSE);

	id = xmlGetProp (tree, "Id");
	g_assert (id);
	g_assert (GPA_NODE_ID_COMPARE (key, id));
	xmlFree (id);

	value = xmlGetProp (tree, "Value");
	if (value) {
		gpa_node_set_value (key, value);
		xmlFree (value);
	}

	for (node = tree->xmlChildrenNode; node != NULL; node = node->next) {
		GPANode *child;
		xmlChar *child_id;
		
		if (strcmp (node->name, "Key"))
			continue;
		
		child_id = xmlGetProp (node, "Id");
		if (!child_id || !child_id[0]) {
			g_warning ("Invalid or missing key id while loading a GPAKey [%s]\n",
				   gpa_node_id (key));
			continue;
		}
		child = key->children;
		while (child) {
			if (GPA_NODE_ID_COMPARE (child, child_id)) {
				gpa_key_merge_from_tree (child, node);
				break;
			}
			child = child->next;
		}
		xmlFree (child_id);
	}

	return TRUE;
}

static gboolean
gpa_key_merge_children_from_key (GPAKey *dst, GPAKey *src)
{
	GPANode *child;
	GSList *dl, *sl;

	dl = NULL;
	child = GPA_NODE (dst)->children;
	while (child) {
		GPANode *next;
		dl = g_slist_prepend (dl, child);
		next = child->next;
		gpa_node_detach (child);
		child = next;
	}
	g_assert (GPA_NODE (dst)->children == NULL);

	sl = NULL;
	for (child = GPA_NODE (src)->children; child != NULL; child = child->next) {
		sl = g_slist_prepend (sl, child);
	}

	while (sl) {
		GSList *l;
		for (l = dl; l != NULL; l = l->next) {
			if (GPA_NODE (l->data)->qid != GPA_NODE (sl->data)->qid)
				continue;

			/* present in both src and dst, merge */
			child = GPA_NODE (l->data);
			dl = g_slist_remove (dl, l->data);
			gpa_node_attach (GPA_NODE (dst), child);
			gpa_key_merge_from_key (GPA_KEY (child), GPA_KEY (sl->data));
			break;
		}
		/* present in src but not dest, add */
		if (!l) {
			child = gpa_node_duplicate (GPA_NODE (sl->data));
			gpa_node_attach (GPA_NODE (dst), child);
		}
		sl = g_slist_remove (sl, sl->data);
	}

	while (dl) {
		/* present in dest but not src, delete */
		gpa_node_unref (GPA_NODE (dl->data));
		dl = g_slist_remove (dl, dl->data);
	}

	return TRUE;
}

gboolean
gpa_key_merge_from_key (GPAKey *dst, GPAKey *src)
{
	g_return_val_if_fail (dst != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_KEY (dst), FALSE);
	g_return_val_if_fail (src != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_KEY (src), FALSE);

	if (dst->value)
		g_free (dst->value);
	dst->value = g_strdup (src->value);

	if (dst->option)
		gpa_node_unref (dst->option);
	if (src->option)
		dst->option = gpa_node_ref (src->option);
	else
		dst->option = NULL;

	gpa_key_merge_children_from_key (dst, src);

	return TRUE;
}

static gboolean
gpa_key_merge_children_from_option (GPAKey *key, GPAOption *option)
{
	GPANode *child;
	GSList *keys = NULL, *keys2;
	GSList *options = NULL;
	GSList *options_orig = NULL;

	child = GPA_NODE (key)->children;
	while (child) {
		keys = g_slist_prepend (keys, child);
		child = child->next;
	}

	child = GPA_NODE (option)->children;
	while (child) {
		options = g_slist_prepend (options, child);
		child = child->next;
	}
#ifdef DEBUG
{
	GSList *l;
	l = keys;
	g_print ("Keys List:\n");
	while (l) {
		g_print ("  %s\n", gpa_node_id (l->data));
		l = l->next;
	}
	l = options;
	g_print ("Options List:\n");
	while (l) {
		g_print ("  %s\n", gpa_node_id (l->data));
		l = l->next;
	}
	g_print ("\n");
}
#endif
	keys2 = g_slist_copy (keys);
	options_orig = options;
	while (options) {
		GSList *l;
		l = keys2;
		while (l) {
			if (GPA_NODE (options->data)->qid == GPA_NODE (l->data)->qid) {
				GPAKey *key = l->data;
#ifdef DEBUG	
				g_print ("IS IN BOTH %s\n", gpa_node_id (l->data));
#endif
				if (key->value == NULL)
					g_warning ("merge key from option, key->value is NULL, should not happen");
				else
					g_free (GPA_KEY (l->data)->value);
				key->value = g_strdup (GPA_OPTION (options->data)->value);
				if (GPA_NODE (options->data)->children) {
					gpa_key_merge_from_option (GPA_KEY (l->data),
								   GPA_OPTION (options->data));
				}

				keys = g_slist_remove (keys, l->data);
				break;
			}
			l = l->next;
		}
		if (l == NULL) {
#ifdef DEBUG
			g_print ("IS ONLY IN option, add: %s\n", gpa_node_id (options->data));
#endif
			/* Is only in new */
			gpa_node_attach (GPA_NODE (key),
					 gpa_option_create_key (options->data,
								GPA_NODE (key)));
		}
		options = options->next;
	}
	g_slist_free (keys2);
	while (keys) {
#ifdef DEBUG
		g_print ("IS ONLY IN keys, remove %s\n", gpa_node_id (keys->data));
#endif
		/* Is only in old */
		gpa_node_detach_unref (GPA_NODE (keys->data));
		keys = g_slist_remove (keys, keys->data);
	}
	
	g_slist_free (options_orig);

	return TRUE;
}

static gboolean
gpa_key_merge_from_option (GPAKey *key, GPAOption *option)
{
	GPANode *och = NULL;

	gpa_node_unref (key->option);
	gpa_node_ref (GPA_NODE (option));
	key->option = GPA_NODE (option);

	if (option->type == GPA_OPTION_TYPE_ITEM)
		return TRUE;

	if (key->value)
		g_free (key->value);
	key->value = NULL;

	if (option->type != GPA_OPTION_TYPE_LIST) {
		if (key->value) {
			g_free (key->value);
			key->value = NULL;
		}
		if (option->value)
			key->value = g_strdup (option->value);
		gpa_key_merge_children_from_option (key, option);
		return TRUE;
	}
	
	if (key->value)
		och = gpa_option_get_child_by_id (option, key->value);
	if (och) {
		gpa_key_merge_children_from_option (key, GPA_OPTION (och));
		gpa_node_unref (och);
		return TRUE;
	}

	if (key->value)
		g_free (key->value);
	key->value = g_strdup (option->value);
	och = gpa_option_get_child_by_id (option, key->value);
	if (!och) {
		g_warning ("List does not contain default item %s", key->value);
		/* FIXME. set one child as value ? (Chema)*/
		return FALSE;
	}
		
	gpa_key_merge_children_from_option (key, GPA_OPTION (och));
	gpa_node_unref (och);

	return TRUE;
}


gboolean
gpa_key_insert (GPANode *parent, const guchar *path, const guchar *value)
{
	GPANode *new;
	const guchar *dot;
	
	g_return_val_if_fail (GPA_IS_KEY (parent), FALSE);
	g_return_val_if_fail (path != NULL, FALSE);

	/* For now, only insert first level keys. (Chema) */
	dot = strchr (path, '.');
	if (dot) {
		g_print ("We only support top level key_inserts");
		return FALSE;
	}

	new = gpa_node_new (GPA_TYPE_KEY, path);
	if (value)
		GPA_KEY (new)->value = g_strdup (value);
	gpa_node_attach (parent, new);

	return TRUE;
}
