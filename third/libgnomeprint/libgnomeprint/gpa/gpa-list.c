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
 *  Copyright (C) 2000-2003 Ximian, Inc.
 *
 */

#include "config.h"
#include <string.h>
#include "gpa-utils.h"
#include "gpa-reference.h"
#include "gpa-list.h"


typedef struct _GPAListClass GPAListClass;
struct _GPAListClass {
	GPANodeClass node_class;
};

static void gpa_list_class_init (GPAListClass *klass);
static void gpa_list_init (GPAList *list);

static void      gpa_list_finalize  (GObject *object);
static gboolean  gpa_list_verify    (GPANode *node);
static gboolean  gpa_list_set_value (GPANode *node, const guchar *value);

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

	node_class->set_value = gpa_list_set_value;
	node_class->verify    = gpa_list_verify;
}

static void
gpa_list_init (GPAList *list)
{
	list->childtype = GPA_TYPE_NODE;
	list->can_have_default = FALSE;
	list->def = NULL;
}

static void
gpa_list_finalize (GObject *object)
{
	GPANode *child;
	GPAList *list;

	list = GPA_LIST (object);

	if (list->def)
		gpa_node_unref (GPA_NODE (list->def));
	list->def = NULL;

	child  = GPA_NODE (list)->children;
	while (child) {
		GPANode *next;
		next = child->next;
		gpa_node_detach_unref (child);
		child = next;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
gpa_list_verify (GPANode *node)
{
	GPAList *list;
	
	list = GPA_LIST (node);

	gpa_return_false_if_fail (list != NULL);
	gpa_return_false_if_fail (GPA_NODE (node)->qid != 0);

	if (!list->can_have_default) {
		gpa_return_false_if_fail (list->def == NULL);
		return TRUE;
	}

	return TRUE;
}

static gboolean
gpa_list_set_value (GPANode *list, const guchar *value)
{
	GPANode *child;
	
	g_return_val_if_fail (GPA_IS_LIST (list), FALSE);

	if (strchr (value, '.') != NULL) {
		g_warning ("Set default from name can't contain \".\"");
		return FALSE;
	}
	
	child = gpa_node_lookup (list, value);
	if (!child) {
		g_warning ("Can't find \"%s\" as a child of \"%s\". Default not set.",
			   value, gpa_node_id (GPA_NODE (list)));
		return FALSE;
	}

	return gpa_list_set_default (GPA_LIST (list), child);
}

/**
 * gpa_list_set_default:
 * @list: 
 * @def: 
 * 
 * Sets the default child of the list
 * 
 * Return Value: TRUE on success, FALSE otherwise
 **/
gboolean
gpa_list_set_default (GPAList *list, GPANode *def)
{
	g_return_val_if_fail (list != NULL, FALSE);
	g_return_val_if_fail (def  != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_LIST (list), FALSE);

	if (list->can_have_default == FALSE) {
		g_warning ("Trying to set the default of a GPAList which has ->can_have_default to FALSE\n");
		return FALSE;
	}

	if (list->def)
		return gpa_reference_set_reference (GPA_REFERENCE (list->def), def);

	list->def = gpa_reference_new (def, "Default");

	return TRUE;
}


/**
 * gpa_list_new:
 * @childtype: The GType of the members of the list
 * @list_name: The list name
 * @parent: 
 * 
 * Creates a new GPAList
 * 
 * Return Value: the newly created list, NULL on error
 **/
GPAList *
gpa_list_new (GType childtype, const gchar *list_name, gboolean can_have_default)
{
	GPAList *list;

	g_return_val_if_fail (g_type_is_a (childtype, GPA_TYPE_NODE), NULL);
	g_return_val_if_fail (list_name != NULL, NULL);

	list = (GPAList *) gpa_node_new (GPA_TYPE_LIST, list_name);

	list->childtype = childtype;
	list->can_have_default = can_have_default ? TRUE : FALSE;
	
	return list;
}


/**
 * gpa_list_get_default:
 * @list: 
 * 
 * Get the list default, makes the first item on the list
 * the default if no default was set.
 * 
 * Return Value: A refcounted GPANode, NULL on error or
 *               if the list does not have any childs
 **/
GPANode *
gpa_list_get_default (GPAList *list)
{
	GPANode *def;
	
	g_return_val_if_fail (list != NULL, NULL);
	g_return_val_if_fail (GPA_IS_LIST (list), NULL);

	if (!GPA_NODE (list)->children)
		return NULL;

	if (!list->def) {
		gpa_list_set_default (list,
				      GPA_NODE (list)->children);
	}
	g_assert (list->def);
	
	def = GPA_REFERENCE_REFERENCE (list->def);
	if (def)
		gpa_node_ref (def);
	
	return def;
}
