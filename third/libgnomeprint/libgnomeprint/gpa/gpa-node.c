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
 *  Copyright (C) 2000-2001 Ximian, Inc. and Jose M. Celorio
 *
 */

#define __GPA_NODE_C__

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include "gpa-utils.h"
#include "gpa-config.h"
#include "gpa-node-private.h"

/* GPANode */

enum {MODIFIED, LAST_SIGNAL};

static void gpa_node_class_init (GPANodeClass *klass);
static void gpa_node_init (GPANode *node);

static void gpa_node_finalize (GObject *object);

static gint gpa_node_modified_idle_hook (GPANode *node);

static GObjectClass *parent_class;
static guint node_signals[LAST_SIGNAL] = {0};

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

	object_class->finalize = gpa_node_finalize;
}

static void
gpa_node_init (GPANode *node)
{
	node->parent = NULL;
	node->next = NULL;
}

static void
gpa_node_finalize (GObject *object)
{
	GPANode *node;
	guint id;

	node = (GPANode *) object;

	g_assert (node->parent == NULL);
	g_assert (node->next == NULL);

	id = GPOINTER_TO_INT (g_object_get_data (object, "idle_id"));
	if (id != 0) {
		g_source_remove (id);
		g_object_steal_data (G_OBJECT (object), "idle_id");
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/* Methods */

GPANode *
gpa_node_new (GType type, const guchar *id)
{
	GPANode *node;

	g_return_val_if_fail (g_type_is_a (type, GPA_TYPE_NODE), NULL);

	node = g_object_new (type, NULL);

	if (id)
		node->qid = gpa_quark_from_string (id);

	return node;
}

GPANode *
gpa_node_construct (GPANode *node, const guchar *id)
{
	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (node), NULL);
	g_return_val_if_fail (!node->qid, NULL);

	if (id) node->qid = gpa_quark_from_string (id);

	return node;
}

GPANode *
gpa_node_duplicate (GPANode *node)
{
	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (node), NULL);

	if (GPA_NODE_GET_CLASS (node)->duplicate)
		return GPA_NODE_GET_CLASS (node)->duplicate (node);

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

	return TRUE;
}

guchar *
gpa_node_get_value (GPANode *node)
{
	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (node), NULL);

	if (GPA_NODE_GET_CLASS (node)->get_value)
		return GPA_NODE_GET_CLASS (node)->get_value (node);

	return NULL;
}

gboolean
gpa_node_set_value (GPANode *node, const guchar *value)
{
	gboolean ret;

	g_return_val_if_fail (node != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_NODE (node), FALSE);

	ret = FALSE;

	if (GPA_NODE_GET_CLASS (node)->set_value)
		ret = GPA_NODE_GET_CLASS (node)->set_value (node, value);

	if (ret) {
		gpa_node_request_modified (node, GPA_NODE_MODIFIED_FLAG);
	}

	return ret;
}

/* NB! ref->parent == node is not invariant due to references */

GPANode *
gpa_node_get_child (GPANode *node, GPANode *ref)
{
	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (node), NULL);
	g_return_val_if_fail (!ref || GPA_IS_NODE (ref), NULL);

	if (GPA_NODE_GET_CLASS (node)->get_child) {
		return GPA_NODE_GET_CLASS (node)->get_child (node, ref);
	}

	return NULL;
}

GPANode *
gpa_node_lookup (GPANode *node, const guchar *path)
{
	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (node), NULL);
	g_return_val_if_fail (path != NULL, NULL);
	g_return_val_if_fail (!*path || isalnum (*path), NULL);

	if (!*path) {
		gpa_node_ref (node);
		return node;
	}

	if (GPA_NODE_GET_CLASS (node)->lookup)
		return GPA_NODE_GET_CLASS (node)->lookup (node, path);

	return NULL;
}

/* Signal stuff */

void
gpa_node_request_modified (GPANode *node, guint flags)
{
	g_return_if_fail (node != NULL);
	g_return_if_fail (GPA_IS_NODE (node));

	if (!(GPA_NODE_FLAGS (node) & GPA_NODE_MODIFIED_FLAG)) {
		GPA_NODE_SET_FLAGS (node, GPA_NODE_MODIFIED_FLAG);
		if (node->parent) {
			gpa_node_request_modified (node->parent, flags);
		} else {
			guint id;
			id = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (node), "idle_id"));
			if (id == 0) {
				id = g_idle_add ((GSourceFunc) gpa_node_modified_idle_hook, node);
				g_object_set_data (G_OBJECT (node), "idle_id", GUINT_TO_POINTER (id));
			}
		}
	}
}

void
gpa_node_emit_modified (GPANode *node, guint flags)
{
	gpa_node_ref (node);

	GPA_NODE_UNSET_FLAGS (node, GPA_NODE_MODIFIED_FLAG);

	g_signal_emit (G_OBJECT (node), node_signals[MODIFIED], 0, flags);

	gpa_node_unref (node);
}

static gint
gpa_node_modified_idle_hook (GPANode *node)
{
	g_object_set_data (G_OBJECT (node), "idle_id", GUINT_TO_POINTER (0));

	gpa_node_emit_modified (node, 0);

	return FALSE;
}

/* Public methods */

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

/* fixme: remove this or make const (Lauris) */

guchar *
gpa_node_id (GPANode *node)
{
	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (node), NULL);

	if (node->qid)
		return g_strdup (gpa_quark_to_string (node->qid));

	return NULL;
}

/* These return referenced node or NULL */

GPANode *
gpa_node_get_parent (GPANode *node)
{
	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (node), NULL);

	if (node->parent)
		gpa_node_ref (node->parent);

	return node->parent;
}

GPANode *
gpa_node_get_path_node (GPANode *node, const guchar *path)
{
	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (node), NULL);
	g_return_val_if_fail (path != NULL, NULL);
	g_return_val_if_fail (!*path || isalnum (*path), NULL);

	return gpa_node_lookup (node, path);
}

/* Basic value manipulation */

guchar *
gpa_node_get_path_value (GPANode *node, const guchar *path)
{
	GPANode *ref;

	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (node), NULL);
	g_return_val_if_fail (path != NULL, NULL);
	g_return_val_if_fail (!*path || isalnum (*path), NULL);

	ref = gpa_node_lookup (node, path);

	if (ref) {
		guchar *value;
		value = gpa_node_get_value (ref);
		gpa_node_unref (ref);
		return value;
	}

	return NULL;
}

gboolean
gpa_node_set_path_value (GPANode *node, const guchar *path, const guchar *value)
{
	GPANode *ref;

	g_return_val_if_fail (node != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_NODE (node), FALSE);
	g_return_val_if_fail (path != NULL, FALSE);
	g_return_val_if_fail (!*path || isalnum (*path), FALSE);

	ref = gpa_node_lookup (node, path);

	if (ref) {
		gboolean ret;
		ret = gpa_node_set_value (ref, value);
		gpa_node_unref (ref);
		return ret;
	}

	return FALSE;
}

/* Convenience stuff */
gboolean
gpa_node_get_bool_path_value (GPANode *node, const guchar *path, gint *value)
{
	guchar *v;
	
	g_return_val_if_fail (node != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_NODE (node), FALSE);
	g_return_val_if_fail (path != NULL, FALSE);
	g_return_val_if_fail (!*path || isalnum (*path), FALSE);
	g_return_val_if_fail (value != NULL, FALSE);

	v = gpa_node_get_path_value (node, path);

	if (v != NULL) {
		if (!strcasecmp (v, "true") || !strcasecmp (v, "yes") || !strcasecmp (v, "y") || (atoi (v) > 0)) {
			*value = TRUE;
			return TRUE;
		}
		*value = FALSE;
		g_free (v);
		return TRUE;
	}
	
	return FALSE;
}

gboolean
gpa_node_get_int_path_value (GPANode *node, const guchar *path, gint *value)
{
	guchar *v;
	
	g_return_val_if_fail (node != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_NODE (node), FALSE);
	g_return_val_if_fail (path != NULL, FALSE);
	g_return_val_if_fail (!*path || isalnum (*path), FALSE);
	g_return_val_if_fail (value != NULL, FALSE);

	v = gpa_node_get_path_value (node, path);

	if (v != NULL) {
		*value = atoi (v);
		g_free (v);
		return TRUE;
	}
	
	return FALSE;
}

gboolean
gpa_node_get_double_path_value (GPANode *node, const guchar *path, gdouble *value)
{
	guchar *v;
	
	g_return_val_if_fail (node != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_NODE (node), FALSE);
	g_return_val_if_fail (path != NULL, FALSE);
	g_return_val_if_fail (!*path || isalnum (*path), FALSE);
	g_return_val_if_fail (value != NULL, FALSE);

	v = gpa_node_get_path_value (node, path);

	if (v != NULL) {
		gchar *loc;
		loc = setlocale (LC_NUMERIC, NULL);
		setlocale (LC_NUMERIC, "C");
		*value = atof (v);
		g_free (v);
		setlocale (LC_NUMERIC, loc);
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
	guchar *v;
	
	g_return_val_if_fail (node != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_NODE (node), FALSE);
	g_return_val_if_fail (path != NULL, FALSE);
	g_return_val_if_fail (!*path || isalnum (*path), FALSE);
	g_return_val_if_fail (value != NULL, FALSE);

	v = gpa_node_get_path_value (node, path);

	if (v != NULL) {
		gchar *loc, *e;
		loc = setlocale (LC_NUMERIC, NULL);
		setlocale (LC_NUMERIC, "C");
		*value = strtod (v, &e);
		if (e) {
			if (!strcmp (e, "mm")) {
				*value *= MM2PT;
			} else if (!strcmp (e, "cm")) {
				*value *= CM2PT;
			} else if (!strcmp (e, "in")) {
				*value *= IN2PT;
			}
		}
		g_free (v);
		setlocale (LC_NUMERIC, loc);
		return TRUE;
	}
	
	return FALSE;
}

gboolean
gpa_node_set_bool_path_value (GPANode *node, const guchar *path, gint value)
{
	g_return_val_if_fail (node != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_NODE (node), FALSE);
	g_return_val_if_fail (path != NULL, FALSE);
	g_return_val_if_fail (!*path || isalnum (*path), FALSE);

	return gpa_node_set_path_value (node, path, (value) ? "true" : "false");
}

gboolean
gpa_node_set_int_path_value (GPANode *node, const guchar *path, gint value)
{
	guchar c[64];
	
	g_return_val_if_fail (node != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_NODE (node), FALSE);
	g_return_val_if_fail (path != NULL, FALSE);
	g_return_val_if_fail (!*path || isalnum (*path), FALSE);

	g_snprintf (c, 64, "%d", value);

	return gpa_node_set_path_value (node, path, c);
}

gboolean
gpa_node_set_double_path_value (GPANode *node, const guchar *path, gdouble value)
{
	guchar c[64];
	gchar *loc;
	
	g_return_val_if_fail (node != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_NODE (node), FALSE);
	g_return_val_if_fail (path != NULL, FALSE);
	g_return_val_if_fail (!*path || isalnum (*path), FALSE);

	loc = setlocale (LC_NUMERIC, NULL);
	setlocale (LC_NUMERIC, "C");
	g_snprintf (c, 64, "%g", value);
	setlocale (LC_NUMERIC, loc);

	return gpa_node_set_path_value (node, path, c);
}

GPANode *
gpa_defaults (void)
{
	GPANode *config;

	config = gpa_config_new ();

	return config;
}



