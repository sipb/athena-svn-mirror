/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-list.c: 
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

#define __GPA_LIST_C__

#include <string.h>
#include "gpa-utils.h"
#include "gpa-reference.h"
#include "gpa-list.h"

/* GPAList */

static void gpa_list_class_init (GPAListClass *klass);
static void gpa_list_init (GPAList *list);

static void gpa_list_finalize (GObject *object);

static GPANode *gpa_list_get_child (GPANode *node, GPANode *ref);
static GPANode *gpa_list_lookup (GPANode *node, const guchar *path);
static void gpa_list_modified (GPANode *node, guint flags);

static gboolean gpa_list_def_set_value (GPAReference *reference, const guchar *value, gpointer data);

static GPANodeClass *parent_class = NULL;

GType
gpa_list_get_type (void) {
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPAListClass),
			NULL, NULL,
			(GClassInitFunc) gpa_list_class_init,
			NULL, NULL,
			sizeof (GPAList),
			0,
			(GInstanceInitFunc) gpa_list_init
		};
		type = g_type_register_static (GPA_TYPE_NODE, "GPAList", &info, 0);
	}
	return type;
}

static void
gpa_list_class_init (GPAListClass *klass)
{
	GObjectClass *object_class;
	GPANodeClass *node_class;

	object_class = (GObjectClass *) klass;
	node_class = (GPANodeClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gpa_list_finalize;

	node_class->get_child = gpa_list_get_child;
	node_class->lookup = gpa_list_lookup;

	node_class->modified = gpa_list_modified;
}

static void
gpa_list_init (GPAList *list)
{
	list->childtype = GPA_TYPE_NODE;
	list->children = NULL;
	list->has_def = FALSE;
	list->def = NULL;
}

static void
gpa_list_finalize (GObject *object)
{
	GPAList *list;

	list = GPA_LIST (object);

	if (list->def) {
		list->def = gpa_node_detach_unref (GPA_NODE (list), GPA_NODE (list->def));
	}

	while (list->children) {
		GPANode *child;
		child = list->children;
		list->children = child->next;
		child->next = NULL;
		child->parent = NULL;
		gpa_node_unref (child);
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GPANode *
gpa_list_get_child (GPANode *node, GPANode *ref)
{
	GPAList *gpl;

	gpl = GPA_LIST (node);

	if (!ref) {
		if (gpl->children) gpa_node_ref (gpl->children);
		return gpl->children;
	}

	if (ref->next) gpa_node_ref (ref->next);

	return ref->next;
}

static GPANode *
gpa_list_lookup (GPANode *node, const guchar *path)
{
	GPAList *list;

	list = GPA_LIST (node);

	if (list->has_def) {
		GPANode *child;
		child = NULL;
		if (gpa_node_lookup_ref (&child, GPA_NODE (list->def), path, "Default")) return child;
	}

	return NULL;
}

static void
gpa_list_modified (GPANode *node, guint flags)
{
	GPAList *gpl;
	GPANode *child;

	gpl = GPA_LIST (node);

	child = gpl->children;
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

	if (gpl->has_def && gpl->def) {
		if (GPA_NODE_FLAGS (gpl->def) & GPA_MODIFIED_FLAG) {
			gpa_node_ref (GPA_NODE (gpl->def));
			gpa_node_emit_modified (GPA_NODE (gpl->def), 0);
			gpa_node_unref (GPA_NODE (gpl->def));
		}
	}
}

GPAList *
gpa_list_construct (GPAList *gpl, GType childtype, gboolean has_def)
{
	g_return_val_if_fail (gpl != NULL, NULL);
	g_return_val_if_fail (GPA_IS_LIST (gpl), NULL);
	g_return_val_if_fail (g_type_is_a (childtype, GPA_TYPE_NODE), NULL);

	gpl->childtype = childtype;
	gpl->has_def = has_def;
	if (has_def) {
		gpl->def = gpa_node_attach (GPA_NODE (gpl), gpa_reference_new_empty ());
		g_signal_connect (G_OBJECT (gpl->def), "set_value",
				  (GCallback) gpa_list_def_set_value, gpl);
	}

	return gpl;
}

gboolean
gpa_list_set_default (GPAList *list, GPANode *def)
{
	gboolean ret;

	g_return_val_if_fail (list != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_LIST (list), FALSE);
	g_return_val_if_fail (list->has_def, FALSE);
	g_return_val_if_fail (list->def != NULL, FALSE);
	g_return_val_if_fail (!def || GPA_IS_NODE (def), FALSE);
	g_return_val_if_fail (!def || GPA_NODE_ID_EXISTS (def), FALSE);
	g_return_val_if_fail (!def || def->parent == (GPANode *) list, FALSE);

	ret = gpa_node_set_value (GPA_NODE (list->def), GPA_NODE_ID (def));

	return ret;
}

static gboolean
gpa_list_def_set_value (GPAReference *reference, const guchar *value, gpointer data)
{
	GPAList *gpl;
	GPANode *child;

	gpl = GPA_LIST (data);

	for (child = gpl->children; child != NULL; child = child->next) {
		if (GPA_NODE_ID_COMPARE (child, value)) {
			gboolean ret;
			ret = gpa_reference_set_reference (GPA_REFERENCE (gpl->def), child);
			return TRUE;
		}
	}

	return FALSE;
}

GPANode *
gpa_list_new (GType childtype, gboolean has_def)
{
	GPAList *gpl;

	g_return_val_if_fail (g_type_is_a (childtype, GPA_TYPE_NODE), NULL);

	gpl = g_object_new (GPA_TYPE_LIST, NULL);

	gpa_list_construct (gpl, childtype, has_def);

	return GPA_NODE (gpl);
}

gboolean
gpa_list_add_child (GPAList *list, GPANode *child, GPANode *ref)
{
	g_return_val_if_fail (list != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_LIST (list), FALSE);
	g_return_val_if_fail (child != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_NODE (child), FALSE);
	g_return_val_if_fail (child->parent == NULL, FALSE);
	g_return_val_if_fail (child->next == NULL, FALSE);
	g_return_val_if_fail (!ref || GPA_IS_NODE (ref), FALSE);
	g_return_val_if_fail (!ref || ref->parent == GPA_NODE (list), FALSE);

	if (!ref) {
		child->next = list->children;
		list->children = child;
	} else {
		child->next = ref->next;
		ref->next = child;
	}

	child->parent = GPA_NODE (list);
	gpa_node_ref (child);

	gpa_node_request_modified (GPA_NODE (list), 0);

	return TRUE;
}

gint
gpa_list_get_length (GPAList *list)
{
	GPANode *child;
	gint num = 0;

	child = list->children;
	while (child) {
		num ++;
		child = child->next;
	}
	
	return num;
}
