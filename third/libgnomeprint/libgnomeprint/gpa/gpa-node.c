/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-node.c:
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
 *  Copyright (C) 2000-2003 Ximian, Inc.
 *
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "gpa-root.h"
#include "gpa-utils.h"
#include "gpa-config.h"
#include "gpa-node-private.h"

extern int errno;

enum {MODIFIED, CHILD_ADDED, CHILD_REMOVED, LAST_SIGNAL};
static GObjectClass *parent_class;
static guint node_signals [LAST_SIGNAL] = {0};

static void gpa_node_class_init (GPANodeClass *klass);
static void gpa_node_init (GPANode *node);
static void gpa_node_finalize (GObject *object);

GType
gpa_node_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPANodeClass),
			NULL, NULL,
			(GClassInitFunc) gpa_node_class_init,
			NULL, NULL,
			sizeof (GPANode),
			0,
			(GInstanceInitFunc) gpa_node_init
		};
		type = g_type_register_static (G_TYPE_OBJECT, "GPANode", &info, 0);
	}
	return type;
}

static void
gpa_node_class_init (GPANodeClass *klass)
{
	GObjectClass *object_class;

	object_class = (GObjectClass*) klass;

	parent_class = g_type_class_peek_parent (klass);

	node_signals[MODIFIED] = g_signal_new ("modified",
					       G_OBJECT_CLASS_TYPE (object_class),
					       G_SIGNAL_RUN_FIRST,
					       G_STRUCT_OFFSET (GPANodeClass, modified),
					       NULL, NULL,
					       g_cclosure_marshal_VOID__UINT,
					       G_TYPE_NONE, 1, G_TYPE_UINT);
	node_signals[CHILD_ADDED] = g_signal_new ("child-added",
					       G_OBJECT_CLASS_TYPE (object_class),
					       G_SIGNAL_RUN_FIRST,
					       G_STRUCT_OFFSET (GPANodeClass, child_added),
					       NULL, NULL,
					       g_cclosure_marshal_VOID__OBJECT,
					       G_TYPE_NONE, 1, G_TYPE_OBJECT);

	node_signals[CHILD_REMOVED] = g_signal_new ("child-removed",
						    G_OBJECT_CLASS_TYPE (object_class),
						    G_SIGNAL_RUN_FIRST,
						    G_STRUCT_OFFSET (GPANodeClass, child_removed),
						    NULL, NULL,
						    g_cclosure_marshal_VOID__OBJECT,
						    G_TYPE_NONE, 1, G_TYPE_OBJECT);
	object_class->finalize = gpa_node_finalize;
}

static void
gpa_node_init (GPANode *node)
{
	node->parent   = NULL;
	node->next     = NULL;
	node->children = NULL;
}

static void
gpa_node_finalize (GObject *object)
{
	GPANode *node;
	guint id;

	node = (GPANode *) object;

	g_assert (node->parent == NULL);
	g_assert (node->next   == NULL);

	id = GPOINTER_TO_INT (g_object_get_data (object, "idle_id"));
	if (id != 0) {
		g_source_remove (id);
		g_object_steal_data (G_OBJECT (object), "idle_id");
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

GPANode *
gpa_node_new (GType type, const guchar *id)
{
	GPANode *node;

	g_return_val_if_fail (g_type_is_a (type, GPA_TYPE_NODE), NULL);

	node = g_object_new (type, NULL);

	if (id)
		node->qid = g_quark_from_string (id);

	return node;
}

GPANode *
gpa_node_duplicate (GPANode *node)
{
	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (node), NULL);

	if (GPA_NODE_GET_CLASS (node)->duplicate)
		return GPA_NODE_GET_CLASS (node)->duplicate (node);
	else
		g_warning ("Can't duplicate the \"%s\" node because the \"%s\" Class does not have a "
			   "duplicate method.", gpa_node_id (node), G_OBJECT_TYPE_NAME (node));

	/* FIXME: copy the flags from the src to the destination
	 * based on a flags_to_copy mask (Chema)
	 */

	return NULL;
}

gboolean
gpa_node_verify (GPANode *node)
{
	gboolean ret;

	g_return_val_if_fail (node != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_NODE (node), FALSE);

	ret = TRUE;

	if (GPA_NODE_GET_CLASS (node)->verify)
		return GPA_NODE_GET_CLASS (node)->verify (node);
	else
		g_error ("Can't verify the \"%s\" node because the \"%s\" Class does not have a "
			 "verify method.", gpa_node_id (node), G_OBJECT_TYPE_NAME (node));

	return TRUE;
}

guchar *
gpa_node_get_value (GPANode *node)
{
	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (node), NULL);

	if (GPA_NODE_GET_CLASS (node)->get_value)
		return GPA_NODE_GET_CLASS (node)->get_value (node);
	else
		g_warning ("Can't get_valued from \"%s\" node because the \"%s\" Class does not have a "
			   "get_value method.", GPA_NODE_ID (node), G_OBJECT_TYPE_NAME (node));

	return NULL;
}

GPANode *
gpa_node_get_parent (GPANode *node)
{
	return node->parent;
}

GPANode *
gpa_node_get_child (GPANode *node, GPANode *previous_child)
{
	GPANode *child;

	/* FIXME: Handle GPAReference ? (Chema) */
	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (node), NULL);
	g_return_val_if_fail (!previous_child || GPA_IS_NODE (previous_child), NULL);

	if (previous_child)
		child = previous_child->next;
	else
		child = node->children;

	if (child)
		gpa_node_ref (child);

	return child;
}	

static GPANode *
gpa_node_lookup_real (GPANode *node, guchar *path)
{
	GPANode *child;
	const guchar *next;
	guchar *dot;

	g_assert (node);
	g_assert (path);
	
	dot = strchr (path, '.');
	if (dot != NULL) {
		next = dot + 1;
		*dot = 0; /* allows us to compare with quarks */
	} else {
		next = NULL;
	}

	child = GPA_NODE (node)->children;
	while (child) {
		if (GPA_NODE_ID_COMPARE (child, path))
		    break;
		child = child->next;
	}
	if (next)
		*dot = '.';

	if (!child) {
		return NULL;
	}

	if (!next) {
		gpa_node_ref (child);
		return child;
	}

	return gpa_node_lookup (child, next);
}

GPANode *
gpa_node_lookup (GPANode *node, const guchar *path)
{
	GPANode *ret;
	gchar *p;

	g_return_val_if_fail (path != NULL, NULL);
	g_return_val_if_fail (*path != '\0', NULL);

	if (node == NULL) {
		node = GPA_NODE (gpa_root);
	}

	g_return_val_if_fail (GPA_IS_NODE(node), NULL);

	if (GPA_IS_REFERENCE (node)) {
		node = GPA_REFERENCE_REFERENCE (node);
	}

	if (node == NULL)
		return NULL;

	g_return_val_if_fail (GPA_IS_NODE (node), NULL);

	p = g_strdup (path);
	ret = gpa_node_lookup_real (node, p);
	g_free (p);

	return ret;
}

gboolean
gpa_node_set_value (GPANode *node, const guchar *value)
{
	gboolean ret = FALSE;

	g_return_val_if_fail (node != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_NODE (node), FALSE);
	g_return_val_if_fail (value != NULL, FALSE);

	if (GPA_NODE_GET_CLASS (node)->set_value) {
		ret = GPA_NODE_GET_CLASS (node)->set_value (node, value);
		if (ret)
			gpa_node_emit_modified (node);
	} else {
		g_warning ("Can't set_valued of \"%s\" to \"%s\" because the \"%s\" Class does not have a "
			   "set_value method.", gpa_node_id (node), value, G_OBJECT_TYPE_NAME (node));
	}

	return ret;
}

void
gpa_node_emit_modified (GPANode *node)
{
	g_signal_emit (G_OBJECT (node), node_signals [MODIFIED], 0, 0);
}

const guchar *
gpa_node_id (GPANode *node)
{
	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (node), NULL);

	if (node->qid)
		return g_quark_to_string (node->qid);

	return NULL;
}

/**
 * gpa_node_get_child_from_path:
 * @node: 
 * @path: 
 * 
 * This is just the public version of gpa_node_lookup. It gets you a child inside a tree
 * given by the @path
 * 
 * Return Value: the GPANode * below the @node leaf after having walked @path
 **/
GPANode *
gpa_node_get_child_from_path (GPANode *node, const guchar *path)
{
	g_return_val_if_fail (path != NULL, NULL);
	g_return_val_if_fail (!*path || g_ascii_isalnum (*path), NULL);

	return gpa_node_lookup (node, path);
}

guchar *
gpa_node_get_path_value (GPANode *node, const guchar *path)
{
	GPANode *child;

	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (node), NULL);
	g_return_val_if_fail (path != NULL, NULL);
	g_return_val_if_fail (!*path || g_ascii_isalnum (*path), NULL);

	child = gpa_node_lookup (node, path);

	if (child) {
		guchar *value;
		value = gpa_node_get_value (child);
		gpa_node_unref (child);
		return value;
	}

	return NULL;
}

/**
 * gpa_node_set_path_value:
 * @node: 
 * @path: 
 * @value: 
 * 
 * Set the value of the child pointed by @path starting from @parent
 *
 * Return Value: 
 **/
gboolean
gpa_node_set_path_value (GPANode *parent, const guchar *path, const guchar *value)
{
	GPANode *node;
	gboolean ret;

	g_return_val_if_fail (parent != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_NODE (parent), FALSE);
	g_return_val_if_fail (path != NULL, FALSE);
	g_return_val_if_fail (!*path || g_ascii_isalnum (*path), FALSE);

	node = gpa_node_lookup (parent, path);
	if (!node) {
		g_warning ("could not set the value of %s, node not found", path);
		return FALSE;
	}
	
	ret = gpa_node_set_value (node, value);
	gpa_node_unref (node);
	return ret;
}

/* Convenience stuff */
gboolean
gpa_node_get_int_path_value (GPANode *node, const guchar *path, gint *value)
{
	guchar *v;
	
	g_return_val_if_fail (node != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_NODE (node), FALSE);
	g_return_val_if_fail (path != NULL, FALSE);
	g_return_val_if_fail (!*path || g_ascii_isalnum (*path), FALSE);
	g_return_val_if_fail (value != NULL, FALSE);

	v = gpa_node_get_path_value (node, path);

	if (v != NULL) {
		*value = atoi (v);
		g_free (v);
		return TRUE;
	}
	
	return FALSE;
}

#define MM2PT (72.0 / 25.4)
#define CM2PT (72.0 / 2.54)
#define IN2PT (72.0)

gboolean
gpa_node_get_length_path_value (GPANode *node, const guchar *path, gdouble *value)
{
	gchar *e;
	guchar *v;
	
	g_return_val_if_fail (node != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_NODE (node), FALSE);
	g_return_val_if_fail (path != NULL, FALSE);
	g_return_val_if_fail (!*path || g_ascii_isalnum (*path), FALSE);
	g_return_val_if_fail (value != NULL, FALSE);

	v = gpa_node_get_path_value (node, path);

	if (v == NULL)
		return FALSE;
	
	*value = g_ascii_strtod (v, &e);
	/* Should we be skipping spaces between the digits and the unit? */
	if (e) {
		if (!g_ascii_strncasecmp (e, "mm", 2)) {
			*value *= MM2PT;
		} else if (!g_ascii_strncasecmp (e, "cm", 2)) {
			*value *= CM2PT;
		} else if (!g_ascii_strncasecmp (e, "in", 2)) {
			*value *= IN2PT;
		}
	}

	g_free (v);
	
	return TRUE;
}


GPANode *
gpa_node_ref (GPANode *node)
{
	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (node), NULL);

	g_object_ref (G_OBJECT (node));

	return node;
}

GPANode *
gpa_node_unref (GPANode *node)
{
	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (node), NULL);

	g_object_unref (G_OBJECT (node));

	return NULL;
}

GPANode *
gpa_node_attach (GPANode *parent, GPANode *child)
{
	g_return_val_if_fail (parent != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (parent), NULL);
	g_return_val_if_fail (child != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (child), NULL);
	g_return_val_if_fail (child->parent == NULL, NULL);
	g_return_val_if_fail (child->next == NULL, NULL);

	child->parent = parent;
	child->next = parent->children;
	parent->children = child;

	g_signal_emit (G_OBJECT (parent), node_signals[CHILD_ADDED], 0,
		       child);
	
	return child;
}

void
gpa_node_reverse_children (GPANode *node)
{
	GPANode *prev = NULL;
	GPANode *child;
	
	g_return_if_fail (node != NULL);
	g_return_if_fail (GPA_IS_NODE (node));

	child = node->children;
	while (child) {
		GPANode *next = child->next;
		child->next = prev;
		prev = child;
		child = next;
	}
	node->children = prev;
}

void
gpa_node_detach (GPANode *node)
{
	GPANode *parent;
	GPANode *prev;

	g_return_if_fail (node != NULL);

	g_assert (node->parent);
	g_assert (node->parent->children);

	parent = node->parent;
	if (parent->children == node) {
		parent->children = node->next;
	} else {
		prev = parent->children;
		while (prev) {
			if (prev->next == node)
				break;
			prev = prev->next;
		}
		g_assert (prev);
		prev->next = node->next;
	}

	node->parent = NULL;
	node->next = NULL;		

	g_signal_emit (G_OBJECT (parent), node_signals[CHILD_REMOVED], 0, node);
}

GPANode *
gpa_node_detach_unref (GPANode *child)
{
	g_return_val_if_fail (child != NULL, child);
	g_return_val_if_fail (GPA_IS_NODE (child), child);
	
	gpa_node_detach (child);
	gpa_node_unref (child);

	return NULL;
}

void
gpa_node_detach_unref_children (GPANode *node)
{
	GPANode *child;

	g_return_if_fail (node != NULL);
	g_return_if_fail (GPA_IS_NODE (node));

	child = node->children;
	while (child) {
		GPANode *next;
		next = child->next;
		gpa_node_detach (child);
		g_object_unref (G_OBJECT (child));
		child = next;
	}
}
