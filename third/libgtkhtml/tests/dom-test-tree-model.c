/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
   Copyright (C) 2000 CodeFactory AB
   Copyright (C) 2000 Jonas Borgstr\366m <jonas@codefactory.se>
   Copyright (C) 2000 Anders Carlsson <andersca@codefactory.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <gtk/gtk.h>
#include "dom-test-tree-model.h"
#include "dom-test-pixmaps.h"

static GType
dom_test_tree_model_get_column_type (GtkTreeModel *tree_model, gint index)
{
	switch (index) {
	case 0:
		return G_TYPE_STRING;
	case 1:
		return GDK_TYPE_PIXBUF;
	default:
		return G_TYPE_INVALID;	
	}
}

static void
dom_test_tree_model_get_value (GtkTreeModel *tree_model, GtkTreeIter *iter,
			       gint column, GValue *value)
{
	static GdkPixbuf *elem_pixbuf = NULL, *text_pixbuf = NULL;
	
	if (!elem_pixbuf) {
		elem_pixbuf = gdk_pixbuf_new_from_xpm_data (elem_xpm);
		text_pixbuf = gdk_pixbuf_new_from_xpm_data (text_xpm);
	}

	switch (column) {
	case 0:
		g_value_init (value, G_TYPE_STRING);
		if (iter->user_data == NULL)
			break;
		switch (dom_Node__get_nodeType (DOM_NODE (iter->user_data))) {
		case DOM_TEXT_NODE:
			g_value_set_string (value, dom_Node__get_nodeValue (DOM_NODE (iter->user_data), NULL));
			break;
		case DOM_DOCUMENT_NODE:
		case DOM_ELEMENT_NODE:
		default:
			g_value_set_string (value, dom_Node__get_nodeName (DOM_NODE (iter->user_data)));
		}
		break;
	case 1:
		g_value_init (value, GDK_TYPE_PIXBUF);
		if (iter->user_data == NULL)
			break;
		switch (dom_Node__get_nodeType (DOM_NODE (iter->user_data))) {
		case DOM_TEXT_NODE:
			g_value_set_instance (value, text_pixbuf);
			break;
		case DOM_DOCUMENT_NODE:
		case DOM_ELEMENT_NODE:
		default:
			g_value_set_instance (value, elem_pixbuf);
		}

	}
}

static GtkTreePath *
dom_test_tree_model_get_path (GtkTreeModel *tree_model, GtkTreeIter *iter)
{
	DomTestTreeModel *model = DOM_TEST_TREE_MODEL (tree_model);
	GtkTreePath *retval;
	DomNode *tmp_node;
	gint i = 0;

	g_assert (dom_Node__get_parentNode (DOM_NODE (iter->user_data)) != NULL);
	
	if (dom_Node__get_parentNode (DOM_NODE (iter->user_data)) == model->root) {
		retval = gtk_tree_path_new_root ();
		tmp_node = dom_Node__get_firstChild (model->root);
	}
	else {
		GtkTreeIter tmp_iter = *iter;

		tmp_iter.user_data = dom_Node__get_parentNode (DOM_NODE (iter->user_data));

		retval = dom_test_tree_model_get_path (tree_model, &tmp_iter);
		tmp_node = dom_Node__get_firstChild (dom_Node__get_parentNode (DOM_NODE (iter->user_data)));
	}

	if (retval == NULL)
		return NULL;

	if (tmp_node == NULL) {
		gtk_tree_path_free (retval);
		return NULL;
	}

	for (;tmp_node; tmp_node = dom_Node__get_nextSibling (tmp_node)) {
		if (tmp_node == DOM_NODE (iter->user_data))
			break;
		i++;

	}

	if (tmp_node == NULL) {
		gtk_tree_path_free (retval);
		return NULL;
	}
	
	gtk_tree_path_append_index (retval, i);
	
	return retval;
}

static gboolean
dom_test_tree_model_iter_nth_child (GtkTreeModel *tree_model,
				    GtkTreeIter  *iter,
				    GtkTreeIter  *parent,
				    gint          n)
{
	DomTestTreeModel *dom_tree_model = DOM_TEST_TREE_MODEL (tree_model);

	if (dom_tree_model->type == DOM_TEST_TREE_MODEL_TREE) {
		if (parent == NULL || parent->user_data == NULL) {
			iter->user_data = DOM_NODE (dom_tree_model->root);
			
			return TRUE;
		}
		else {
			DomNode *node;
			gint i;
			
			node = dom_Node__get_firstChild (parent->user_data);
			
			for (i = 0; i < n; i++) {
				if (!node)
					return FALSE;
				
				node = dom_Node__get_nextSibling (node);
			}
			iter->user_data = node;
			
			return TRUE;
		}
	}
	else {
		if (parent == NULL) {
			DomNode *node;
			
			if ((node = dom_Node__get_firstChild (DOM_NODE (dom_tree_model->root))) == NULL) {
				g_print ("we return false!\n");
				return FALSE;
			}
			else {
				gint i;
				for (i = 0; i < n; i++) {
					if (!node)
						return FALSE;
					
					node = dom_Node__get_nextSibling (node);
				}
				iter->user_data = node;
				
				return TRUE;
			}
		}
		else {
			g_warning ("weeee");
		}
	}
	g_print ("Nth child!\n");
	return FALSE;
}

static gboolean
dom_test_tree_model_iter_children (GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreeIter *parent)
{
	if ((parent->user_data == NULL) || !dom_Node_hasChildNodes (DOM_NODE (parent->user_data)))
		return FALSE;
	
	iter->user_data = dom_Node__get_firstChild (DOM_NODE (parent->user_data));
	return TRUE;
}

static gboolean
dom_test_tree_model_iter_parent (GtkTreeModel *tree_model,
				 GtkTreeIter  *iter,
				 GtkTreeIter  *child)
{
	if (dom_Node__get_parentNode (DOM_NODE (child->user_data)) == NULL)
		return FALSE;

	iter->user_data = dom_Node__get_parentNode (DOM_NODE (child->user_data));
	return TRUE;
}


static gboolean
dom_test_tree_model_iter_next (GtkTreeModel  *tree_model,
			       GtkTreeIter   *iter)
{
	if (iter->user_data == NULL)
		return FALSE;
	if (dom_Node__get_nextSibling (DOM_NODE (iter->user_data)) == NULL)
		return FALSE;
	
	iter->user_data = dom_Node__get_nextSibling (DOM_NODE (iter->user_data));

	return TRUE;
}

static gboolean
dom_test_tree_model_get_iter (GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreePath *path)
{
	DomTestTreeModel *dom_tree_model;
	GtkTreeIter parent;
	gint *indices;
	gint depth, i;

	dom_tree_model = DOM_TEST_TREE_MODEL (tree_model);
	
	indices = gtk_tree_path_get_indices (path);
	depth = gtk_tree_path_get_depth (path);

	g_return_val_if_fail (depth > 0, FALSE);

	parent.user_data = dom_tree_model->root;

	if (!gtk_tree_model_iter_nth_child (tree_model, iter, &parent, indices[0]))
		return FALSE;

	for (i = 1; i < depth; i++) {
		parent = *iter;
		if (!gtk_tree_model_iter_nth_child (tree_model, iter, &parent, indices[i]))
			return FALSE;
	}

	return TRUE;
}

static gboolean
dom_test_tree_model_iter_has_child (GtkTreeModel *tree_model, GtkTreeIter *iter)
{
	if (DOM_TEST_TREE_MODEL (tree_model)->type == DOM_TEST_TREE_MODEL_LIST)
		return FALSE;

	if (iter->user_data == NULL)
		return FALSE;
	
	return dom_Node_hasChildNodes (DOM_NODE (iter->user_data));
}

static gint
dom_test_tree_model_get_n_columns (GtkTreeModel *model)
{
	return 1;
}

static void
dom_test_tree_model_tree_model_init (GtkTreeModelIface *iface)
{
	iface->get_n_columns = dom_test_tree_model_get_n_columns;
	iface->get_column_type = dom_test_tree_model_get_column_type;
	iface->iter_next = dom_test_tree_model_iter_next;
	iface->iter_children = dom_test_tree_model_iter_children;
	iface->iter_has_child = dom_test_tree_model_iter_has_child;
	iface->iter_nth_child = dom_test_tree_model_iter_nth_child;
	iface->iter_parent = dom_test_tree_model_iter_parent;
	iface->get_value = dom_test_tree_model_get_value;
	iface->get_path = dom_test_tree_model_get_path;
	iface->get_iter = dom_test_tree_model_get_iter;
}

GType
dom_test_tree_model_get_type (void)
{
	static GType dom_tree_model_type = 0;

	if (!dom_tree_model_type) {
		static const GTypeInfo dom_tree_model_info = {
			sizeof (DomTestTreeModelClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			NULL,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (DomTestTreeModel),
			16,   /* n_preallocs */
			NULL,
		};

		static const GInterfaceInfo tree_model_info =
		{
			(GInterfaceInitFunc) dom_test_tree_model_tree_model_init,
			NULL,
			NULL
		};

		dom_tree_model_type = g_type_register_static (G_TYPE_OBJECT, "DomTestTreeModel", &dom_tree_model_info, 0);

		g_type_add_interface_static (dom_tree_model_type,
					     GTK_TYPE_TREE_MODEL,
					     &tree_model_info);
	}

	return dom_tree_model_type;
}

DomTestTreeModel *
dom_test_tree_model_new (DomNode *root, DomTestTreeModelType type)
{
	DomTestTreeModel *result = g_object_new (DOM_TYPE_TEST_TREE_MODEL, NULL);

	g_print ("New tree model: %d\n", type);
	
	result->type = type;

	result->root = root;
	
	return result;
}
