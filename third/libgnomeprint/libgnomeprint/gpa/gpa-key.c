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
 *    Jose M. Celorio <chema@ximian.com>
 *
 *  Copyright (C) 2000-2001 Ximian, Inc. and Jose M. Celorio
 *
 */

#define __GPA_KEY_C__

#include <string.h>
#include <libxml/globals.h>
#include "gpa-utils.h"
#include "gpa-reference.h"
#include "gpa-option.h"
#include "gpa-key.h"

static void gpa_key_class_init (GPAKeyClass *klass);
static void gpa_key_init (GPAKey *key);

static void gpa_key_finalize (GObject *object);

static GPANode *gpa_key_duplicate (GPANode *node);
static gboolean gpa_key_verify (GPANode *node);
static guchar *gpa_key_get_value (GPANode *node);
static gboolean gpa_key_set_value (GPANode *node, const guchar *value);
static GPANode *gpa_key_get_child (GPANode *node, GPANode *ref);
static GPANode *gpa_key_lookup (GPANode *node, const guchar *path);
static void gpa_key_modified (GPANode *node, guint flags);

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
	node_class->verify = gpa_key_verify;
	node_class->get_value = gpa_key_get_value;
	node_class->set_value = gpa_key_set_value;
	node_class->get_child = gpa_key_get_child;
	node_class->lookup = gpa_key_lookup;
	node_class->modified = gpa_key_modified;
}

static void
gpa_key_init (GPAKey *key)
{
	key->children = NULL;
	key->option = NULL;
	key->value = NULL;
}

static void
gpa_key_finalize (GObject *object)
{
	GPAKey *key;

	key = GPA_KEY (object);

	while (key->children) {
		if (G_OBJECT (key->children)->ref_count > 1) {
			g_warning ("GPAKey: Child %s has refcount %d\n", GPA_NODE_ID (key->children), G_OBJECT (key->children)->ref_count);
		}
		key->children = gpa_node_detach_unref_next (GPA_NODE (key), key->children);
	}

	if (key->option) {
		gpa_node_unref (key->option);
		key->option = NULL;
	}

	if (key->value) {
		g_free (key->value);
		key->value = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GPANode *
gpa_key_duplicate (GPANode *node)
{
	GPAKey *key, *new;
	GPANode *child;
	GSList *l;

	key = GPA_KEY (node);

	new = (GPAKey *) gpa_node_new (GPA_TYPE_KEY, GPA_NODE_ID (node));

	if (key->value) new->value = g_strdup (key->value);

	if (key->option) {
		new->option = key->option;
		gpa_node_ref (new->option);
	}

	l = NULL;
	for (child = key->children; child != NULL; child = child->next) {
		GPANode *newchild;
		newchild = gpa_node_duplicate (child);
		if (newchild) l = g_slist_prepend (l, newchild);
	}

	while (l) {
		GPANode *newchild;
		newchild = GPA_NODE (l->data);
		l = g_slist_remove (l, newchild);
		newchild->parent = GPA_NODE (new);
		newchild->next = new->children;
		new->children = newchild;
	}

	return GPA_NODE (new);
}

static gboolean
gpa_key_verify (GPANode *node)
{
	/* fixme: Verify on option */

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
	GPAKey *key;
	GPAOption *option;

	key = GPA_KEY (node);

	/* fixme: Is that clever? */
	g_return_val_if_fail (value != NULL, FALSE);
	g_return_val_if_fail (key->value != NULL, FALSE);

	if (key->value && !strcmp (key->value, value))
		return TRUE;

	option = GPA_KEY_OPTION (key);
	g_return_val_if_fail (option != NULL, FALSE);

	if (GPA_OPTION_IS_LIST (option)) {
		GPAOption *child;
		child = gpa_option_get_child_by_id (option, value);
		if (child) {
			/* Everything is legal */
			g_free (key->value);
			key->value = g_strdup (value);
			gpa_key_merge_children_from_option (key, child);
			gpa_node_unref (GPA_NODE (child));
			gpa_node_request_modified (node, 0);
			return TRUE;
		}
	} else if (GPA_OPTION_IS_STRING (option)) {
		if (!value)
			value = option->value;
		if (value && key->value && !strcmp (value, key->value))
			return TRUE;
		if (key->value)
			g_free (key->value);
		key->value = g_strdup (value);
		gpa_node_request_modified (node, 0);
		return TRUE;
	}

	return FALSE;
}

static GPANode *
gpa_key_get_child (GPANode *node, GPANode *ref)
{
	GPAKey *key;

	key = GPA_KEY (node);

	if (!ref) {
		if (key->children)
			gpa_node_ref (key->children);
		return key->children;
	}

	if (ref->next)
		gpa_node_ref (ref->next);

	return ref->next;
}

static GPANode *
gpa_key_lookup (GPANode *node, const guchar *path)
{
	GPAKey *key;
	GPANode *child;
	const guchar *dot, *next;
	gint len;

	key = GPA_KEY (node);

	child = NULL;
	if (gpa_node_lookup_ref (&child, GPA_NODE (key->option), path, "Option"))
		return child;

	dot = strchr (path, '.');
	if (dot != NULL) {
		len = dot - path;
		next = dot + 1;
	} else {
		len = strlen (path);
		next = path + len;
	}

	for (child = key->children; child != NULL; child = child->next) {
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
gpa_key_modified (GPANode *node, guint flags)
{
	GPAKey *key;
	GPANode *child;

	key = GPA_KEY (node);

	child = key->children;
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

	if (key->option && GPA_NODE_FLAGS (key->option) & GPA_NODE_MODIFIED_FLAG) {
		gpa_node_emit_modified (key->option, 0);
	}
}

GPANode *
gpa_key_new_from_option (GPANode *node)
{
	return gpa_option_create_key (GPA_OPTION (node));
}

xmlNodePtr
gpa_key_write (xmlDocPtr doc, GPANode *node)
{
	GPAKey *key;
	GPAOption *option;
	xmlNodePtr xmln, xmlc;
	GPANode *child;

	g_return_val_if_fail (doc != NULL, NULL);
	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GPA_IS_KEY (node), NULL);
	g_return_val_if_fail (GPA_KEY_HAS_OPTION (node), NULL);

	key = GPA_KEY (node);

	option = GPA_KEY_OPTION (key);

	if (option->type != GPA_OPTION_TYPE_KEY) {
		xmln = xmlNewDocNode (doc, NULL, "Key", NULL);
		xmlSetProp (xmln, "Id", GPA_NODE_ID (node));
		if (key->value) xmlSetProp (xmln, "Value", key->value);

		for (child = key->children; child != NULL; child = child->next) {
			xmlc = gpa_key_write (doc, child);
			if (xmlc)
				xmlAddChild (xmln, xmlc);
		}
	} else {
		xmln = NULL;
	}

	return xmln;
}

gboolean
gpa_key_merge_from_tree (GPANode *key, xmlNodePtr tree)
{
	xmlChar *xmlid, *xmlval;
	xmlNodePtr xmlc;

	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_KEY (key), FALSE);
	g_return_val_if_fail (tree != NULL, FALSE);
	g_return_val_if_fail (!strcmp (tree->name, "Key"), FALSE);

	xmlid = xmlGetProp (tree, "Id");
	g_assert (xmlid);
	g_assert (GPA_NODE_ID_COMPARE (key, xmlid));
	xmlFree (xmlid);

	xmlval = xmlGetProp (tree, "Value");
	if (xmlval) {
		gpa_node_set_value (key, xmlval);
		xmlFree (xmlval);
	}

	for (xmlc = tree->xmlChildrenNode; xmlc != NULL; xmlc = xmlc->next) {
		if (!strcmp (xmlc->name, "Key")) {
			xmlChar *keyid;
			keyid = xmlGetProp (xmlc, "Id");
			if (keyid) {
				GPANode *kch;
				for (kch = GPA_KEY (key)->children; kch != NULL; kch = kch->next) {
					if (GPA_NODE_ID_COMPARE (kch, keyid)) {
						gpa_key_merge_from_tree (kch, xmlc);
						break;
					}
				}
				xmlFree (keyid);
			}
		}
	}

	return TRUE;
}

gboolean
gpa_key_copy (GPANode *dst, GPANode *src)
{
	gboolean modified;

	g_return_val_if_fail (dst != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_KEY (dst), FALSE);
	g_return_val_if_fail (src != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_KEY (src), FALSE);

	modified = FALSE;

	return FALSE;
}

static gboolean
gpa_key_merge_children_from_key (GPAKey *dst, GPAKey *src)
{
	GPANode *child;
	GSList *dl, *sl;

	dl = NULL;
	while (dst->children) {
		dl = g_slist_prepend (dl, dst->children);
		dst->children = gpa_node_detach_next (GPA_NODE (dst), dst->children);
	}

	sl = NULL;
	for (child = src->children; child != NULL; child = child->next) {
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
				child->next = dst->children;
				dst->children = child;
				gpa_key_merge_from_key (GPA_KEY (child), GPA_KEY (sl->data));
				break;
			}
		}
		if (!l) {
			/* Create new child */
			child = gpa_node_duplicate (GPA_NODE (sl->data));
			child->parent = GPA_NODE (dst);
			child->next = dst->children;
			dst->children = child;
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

gboolean
gpa_key_merge_from_key (GPAKey *dst, GPAKey *src)
{
	g_return_val_if_fail (dst != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_KEY (dst), FALSE);
	g_return_val_if_fail (src != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_KEY (src), FALSE);

	g_return_val_if_fail (src->option != NULL, FALSE);

	if (dst->value)
		g_free (dst->value);
	dst->value = g_strdup (src->value);

	if (dst->option)
		gpa_node_unref (dst->option);
	dst->option = src->option;
	gpa_node_ref (dst->option);

	gpa_key_merge_children_from_key (dst, src);

	gpa_node_request_modified (GPA_NODE (dst), 0);

	return TRUE;
}

static gboolean
gpa_key_merge_children_from_option (GPAKey *key, GPAOption *option)
{
	GPANode *child;
	GSList *kl, *ol;

	kl = NULL;
	while (key->children) {
		kl = g_slist_prepend (kl, key->children);
		key->children = gpa_node_detach_next (GPA_NODE (key), key->children);
	}

	ol = NULL;
	for (child = option->children; child != NULL; child = child->next) {
		ol = g_slist_prepend (ol, child);
	}

	while (ol) {
		GSList *l;
		for (l = kl; l != NULL; l = l->next) {
			if (GPA_NODE (l->data)->qid && (GPA_NODE (l->data)->qid == GPA_NODE (ol->data)->qid)) {
				/* We are in original too */
				child = GPA_NODE (l->data);
				kl = g_slist_remove (kl, l->data);
				child->parent = GPA_NODE (key);
				child->next = key->children;
				key->children = child;
				gpa_key_merge_from_option (GPA_KEY (child), GPA_OPTION (ol->data));
				break;
			}
		}
		if (!l) {
			/* Create from option */
			child = gpa_key_new_from_option (GPA_NODE (ol->data));
			child->parent = GPA_NODE (key);
			child->next = key->children;
			key->children = child;
		}
		ol = g_slist_remove (ol, ol->data);
	}

	while (kl) {
		gpa_node_unref (GPA_NODE (kl->data));
		kl = g_slist_remove (kl, kl->data);
	}

	gpa_node_request_modified (GPA_NODE (key), 0);

	return TRUE;
}

static gboolean
gpa_key_merge_from_option (GPAKey *key, GPAOption *option)
{
	GPAOption *och;

	gpa_node_unref (key->option);
	gpa_node_ref (GPA_NODE (option));
	key->option = GPA_NODE (option);

	switch (option->type) {
	case GPA_OPTION_TYPE_NODE:
	case GPA_OPTION_TYPE_KEY:
	case GPA_OPTION_TYPE_STRING:
		if (key->value) {
			g_free (key->value);
			key->value = NULL;
		}
		if (option->value) key->value = g_strdup (option->value);
		gpa_key_merge_children_from_option (key, option);
		break;
	case GPA_OPTION_TYPE_LIST:
		och = NULL;
		if (key->value) och = gpa_option_get_child_by_id (option, key->value);
		if (och) {
			gpa_key_merge_children_from_option (key, och);
			gpa_node_unref (GPA_NODE (och));
		} else {
			if (key->value) g_free (key->value);
			key->value = g_strdup (option->value);
			och = gpa_option_get_child_by_id (option, key->value);
			if (och) {
				gpa_key_merge_children_from_option (key, och);
				gpa_node_unref (GPA_NODE (och));
			} else {
				g_warning ("List does not contain default item");
			}
		}
		break;
	case GPA_OPTION_TYPE_ITEM:
		/* No key for item */
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	gpa_node_request_modified (GPA_NODE (key), 0);

	return TRUE;

}

