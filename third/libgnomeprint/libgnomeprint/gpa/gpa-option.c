/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-option.c: 
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

#include <config.h>

#include <string.h>
#include <libxml/globals.h>

#include "gpa-node.h"
#include "gpa-node-private.h"
#include "gpa-utils.h"
#include "gpa-key.h"
#include "gpa-option.h"
#include "gpa-root.h"
#include "gnome-print-i18n.h"

struct _GPAOptionClass {
	GPANodeClass node_class;
};

static void gpa_option_class_init (GPAOptionClass *klass);
static void gpa_option_init (GPAOption *option);
static void gpa_option_finalize (GObject *object);

static GPANode * gpa_option_duplicate (GPANode *node);
static gboolean  gpa_option_verify    (GPANode *node);
static guchar *  gpa_option_get_value (GPANode *node);

static GPANode *gpa_option_key_new_from_tree    (xmlNodePtr tree, GPANode *parent, const guchar *id);
static GPANode *gpa_option_list_new_from_tree   (xmlNodePtr tree, GPANode *parent, const guchar *id);
static GPANode *gpa_option_item_new_from_tree   (xmlNodePtr tree, GPANode *parent, const guchar *id);
static GPANode *gpa_option_string_new_from_tree (xmlNodePtr tree, GPANode *parent, const guchar *id);

#define GPA_NOT -1
#define GPA_MAYBE 0
#define GPA_MUST 1

#define GPA_FORBIDDEN(v) ((v) < 0)
#define GPA_ALLOWED(v) ((v) == 0)
#define GPA_MUSTHAVE(v) ((v) > 0)
#define GPA_UNAMBIGUOUS(v) ((v) != 0)

static gboolean gpa_option_xml_check (xmlNodePtr node, const gchar *id, gint def, gint val, gint name, gint children);

static GPANodeClass *parent_class;

GType
gpa_option_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPAOptionClass),
			NULL, NULL,
			(GClassInitFunc) gpa_option_class_init,
			NULL, NULL,
			sizeof (GPAOption),
			0,
			(GInstanceInitFunc) gpa_option_init
		};
		type = g_type_register_static (GPA_TYPE_NODE, "GPAOption", &info, 0);
	}
	return type;
}

static void
gpa_option_class_init (GPAOptionClass *klass)
{
	GObjectClass *object_class;
	GPANodeClass *node_class;

	object_class = (GObjectClass *) klass;
	node_class = (GPANodeClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gpa_option_finalize;

	node_class->duplicate = gpa_option_duplicate;
	node_class->verify    = gpa_option_verify;
	node_class->get_value = gpa_option_get_value;
}

static void
gpa_option_init (GPAOption *option)
{
	option->type = GPA_OPTION_TYPE_NONE;
	option->value    = NULL;
}

static void
gpa_option_finalize (GObject *object)
{
	GPAOption *option;

	option = GPA_OPTION (object);

	gpa_node_detach_unref_children (GPA_NODE (option));
	if (option->value)
		g_free (option->value);
	option->value = NULL;

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GPANode *
gpa_option_duplicate (GPANode *node)
{
	GPAOption *option, *new;
	GPANode *new_node;
	GPANode *child;

	option = GPA_OPTION (node);

	new_node = gpa_node_new (GPA_TYPE_OPTION, gpa_node_id (node));

	if ((GPA_NODE_FLAGS (node) & NODE_FLAG_LOCKED) == NODE_FLAG_LOCKED) {
		GPA_NODE_SET_FLAGS (new_node, NODE_FLAG_LOCKED);
	}
	
	new = GPA_OPTION (new_node);
	new->type = option->type;

	if (option->value)
		new->value = g_strdup (option->value);

	child = GPA_NODE (option)->children;
	while (child) {
		gpa_node_attach (new_node,
				 gpa_option_duplicate (child));
		child = child->next;
	}

	gpa_node_reverse_children (new_node);

	return new_node;
}

static gboolean
gpa_option_verify (GPANode *node)
{
	GPAOption *option;
	GPANode *child;

	option = GPA_OPTION (node);

	switch (option->type) {
	case GPA_OPTION_TYPE_NODE:
		/* !Value, children */
		gpa_return_false_if_fail (option->value == NULL);
		gpa_return_false_if_fail (GPA_NODE (option)->children != NULL);
		for (child = GPA_NODE (option)->children; child != NULL; child = child->next) {
			gpa_return_false_if_fail (GPA_IS_OPTION (child));
			gpa_return_false_if_fail (gpa_node_verify (child));
		}
		break;
	case GPA_OPTION_TYPE_KEY:
		/* Value || children */
		gpa_return_false_if_fail (option->value || GPA_NODE (option)->children);
		for (child = GPA_NODE (option)->children; child != NULL; child = child->next) {
			gpa_return_false_if_fail (GPA_IS_OPTION (child));
			gpa_return_false_if_fail (GPA_OPTION (child)->type == GPA_OPTION_TYPE_KEY);
			gpa_return_false_if_fail (gpa_node_verify (child));
		}
		break;
	case GPA_OPTION_TYPE_LIST:
		/* Value, children */
		gpa_return_false_if_fail (option->value != NULL);
		gpa_return_false_if_fail (GPA_NODE (option)->children != NULL);
		for (child = GPA_NODE (option)->children; child != NULL; child = child->next) {
			gpa_return_false_if_fail (GPA_IS_OPTION (option));
			gpa_return_false_if_fail (GPA_OPTION_IS_ITEM (child));
			gpa_return_false_if_fail (gpa_node_verify (child));
		}
		break;
	case GPA_OPTION_TYPE_ITEM:
		/* Value */
		gpa_return_false_if_fail (option->value != NULL);
		for (child = GPA_NODE (option)->children; child != NULL; child = child->next) {
			gpa_return_false_if_fail (GPA_IS_OPTION (child));
			gpa_return_false_if_fail (gpa_node_verify (child));
		}
		break;
	case GPA_OPTION_TYPE_STRING:
		/* Value, ! children */
		gpa_return_false_if_fail (option->value != NULL);
		gpa_return_false_if_fail (GPA_NODE (option)->children== NULL);
		break;
	default:
		g_warning ("Invalid option type!");
	}
	return TRUE;
}

static guchar *
gpa_option_get_value (GPANode *node)
{
	GPAOption *option;

	option = GPA_OPTION (node);

	if (option->value)
		return g_strdup (option->value);

	return NULL;
}

GPANode *
gpa_option_create_key (GPAOption *option, GPANode *parent)
{
	GPANode *key_node;
	GPANode *child;
	GPAKey *key;

	g_return_val_if_fail (option != NULL, NULL);

	key_node = gpa_node_new (GPA_TYPE_KEY, GPA_NODE_ID (option));
	key = (GPAKey *) key_node;
	key->option = gpa_node_ref (GPA_NODE (option));
	if (option->value)
		key->value = g_strdup (option->value);

	if (option->type == GPA_OPTION_TYPE_LIST) {
		child = gpa_option_get_child_by_id (option, option->value);
		if (child != NULL)
			child = child->children;
	} else {
		child = GPA_NODE (option)->children;
	}

	if ((GPA_NODE_FLAGS (option) & NODE_FLAG_LOCKED) == NODE_FLAG_LOCKED) {
		GPA_NODE_SET_FLAGS (key, NODE_FLAG_LOCKED);
	}

	while (child) {
		GPANode *child_key;
		child_key = gpa_option_create_key (GPA_OPTION (child),
						   key_node);
		if (child_key) {
			gpa_node_attach (key_node, child_key);
		}
		child = child->next;
	}
	gpa_node_reverse_children (GPA_NODE (key));

	return (GPANode *) key;
}

/**
 * gpa_option_new_from_fill_tag:
 * @node: 
 * 
 * Handle the options that have :
 * <Fill Ref="Globals.Media.x.y.z">
 **/
static gboolean
gpa_option_new_from_fill_tag (xmlNodePtr node, GPANode *parent, xmlChar *def, gboolean *has_default)
{
	xmlChar *value;
	GPANode *ref = NULL;
	gboolean retval = TRUE;

	value = xmlGetProp (node, "Ref");
			
	if (!value) {
		g_warning ("Invalid \"Fill\" node, must contain a \"Ref\" property");
		retval = FALSE;
		goto new_from_fill_tag_exit;
	}

	/* At this point, globals can only be used to pull information about the
	 * Media node, we load printers (which loads models, which loads options) in
	 * gpa_root_init so the only part of gpa_root that is loaded is Media.
	 * This feature is intended to be able to reference leaves cross Models
	 * so that you can say, the Paper size of this printer, take it form this other
	 * one (Chema)
	 */
	if (strncmp (value, "Globals.", strlen ("Globals.")) != 0) {
		g_warning ("Invalid \"Ref\" property. Should contain the \"Globals.\""
			   "prefix (%s)\n", value);
		retval = FALSE;
		goto new_from_fill_tag_exit;
	}

	ref = gpa_node_lookup (NULL, value);

	if (!ref) {
		g_warning ("Could not get %s from globals while trying to satisfy "
			   "a \"Fill\" node", value);
		retval = FALSE;
		goto new_from_fill_tag_exit;
	}

	if (GPA_OPTION_IS_LIST (ref)) {
		GPANode *child;
		child = ref->children;
		while (child) {
			GPANode *new = gpa_node_duplicate(child);
			gpa_node_attach (parent, new);
			if (GPA_NODE_ID_COMPARE (new, def))
				*has_default = TRUE;
			child = child->next;
		}
	}

	gpa_node_reverse_children (parent);

new_from_fill_tag_exit:
	my_xmlFree (value);
	my_gpa_node_unref (ref);

	return retval;
}

GPANode *
gpa_option_new_from_tree (xmlNodePtr tree, GPANode *parent)
{
	GPANode *option = NULL;
	xmlChar *id;
	xmlChar *type = NULL;
	xmlChar *locked = NULL;

	g_return_val_if_fail (tree != NULL, NULL);

	id = xmlGetProp (tree, "Id");
	if (id == NULL) {
		g_warning ("Option node does not have Id");
		goto new_from_tree_done;
	}

	if (!strcmp (tree->name, "Key")) {
		option = gpa_option_key_new_from_tree (tree, parent, id);
		goto new_from_tree_done;
	}

	if (!strcmp (tree->name, "Item")) {
		option = gpa_option_item_new_from_tree (tree, parent, id);
		goto new_from_tree_done;
	}
	
	if (strcmp (tree->name, "Option")) {
		g_warning ("Unexpected XML node loading option. (%s)\n", tree->name);
		goto new_from_tree_done;
	}

	type = xmlGetProp (tree, "Type");
	
	if (!type || !type[0]) {
		option = gpa_option_node_new_from_tree (tree, parent, id);
		goto new_from_tree_option_loaded;
	}

	if (strcmp (type, "List") == 0) {
		option = gpa_option_list_new_from_tree (tree, parent, id);
		goto new_from_tree_option_loaded;
	}

	if (strcmp (type, "String") == 0) {
		option = gpa_option_string_new_from_tree (tree, parent, id);
		goto new_from_tree_option_loaded;
	}

new_from_tree_option_loaded:
	locked = xmlGetProp (tree, "Locked");
	
	if (option) {
		if (locked && !strcmp (locked, "true")) {
			GPA_NODE_SET_FLAGS (option, NODE_FLAG_LOCKED);
		} else {
			GPA_NODE_UNSET_FLAGS (option, NODE_FLAG_LOCKED);
		}		
	}
	
new_from_tree_done:
	my_xmlFree (locked);
	my_xmlFree (id);
	my_xmlFree (type);

	return option;
}

GPANode *
gpa_option_node_new_from_tree (xmlNodePtr tree, GPANode *parent, const guchar *id)
{
	GPANode *option;
	xmlNodePtr xmlc;
	gboolean sucesss = FALSE;

	if (!gpa_option_xml_check (tree, id, GPA_NOT, GPA_NOT, GPA_NOT, GPA_MUST)) {
		g_warning ("Option node structure is not correct");
		return NULL;
	}

	option = gpa_option_node_new (parent, id);

	for (xmlc = tree->xmlChildrenNode; xmlc != NULL; xmlc = xmlc->next) {
		if (xmlc->type == XML_ELEMENT_NODE) {
			if (!strcmp (xmlc->name, "Option") || !strcmp (xmlc->name, "Key")) {
				GPANode *cho;
				cho = gpa_option_new_from_tree (xmlc, option);
				if (cho)
					sucesss = TRUE;
			} else {
				g_warning ("Invalid child in option tree %s", xmlc->name);
			}
		}
	}
	gpa_node_reverse_children (option);

	if (sucesss == FALSE ) {
		g_warning ("Option should have valid children");
		return NULL;
	}


	return option;
}

static GPANode *
gpa_option_key_new_from_tree (xmlNodePtr tree, GPANode *parent, const guchar *id)
{
	GPANode *option;
	xmlNodePtr node;
	xmlChar *value;

	if (!gpa_option_xml_check (tree, id, GPA_NOT, GPA_MAYBE, GPA_NOT, GPA_MAYBE)) {
		g_warning ("Option key structure is not correct");
		return NULL;
	}

	value = xmlGetProp (tree, "Value");
	if (!value && !tree->xmlChildrenNode) {
		g_warning ("Key node should have value or children or both");
		return NULL;
	}


	option = gpa_option_key_new (parent, id, value);
	
	xmlFree (value);

	node = tree->xmlChildrenNode;
	for (; node != NULL; node = node->next) {
		if (node->type != XML_ELEMENT_NODE)
			continue;

		if (strcmp (node->name, "Key") != 0)
			continue;

		gpa_option_new_from_tree (node, option);
	}

	gpa_node_reverse_children (option);

	return option;
}

static GPANode *
gpa_option_list_new_from_tree (xmlNodePtr tree, GPANode *parent, const guchar *id)
{
	xmlNodePtr node;
	xmlChar *def;
	GPANode *option;
	gboolean has_default = FALSE;

	if (!gpa_option_xml_check (tree, id, GPA_MUST, GPA_NOT, GPA_NOT, GPA_MUST)) {
		g_warning ("Option list structure is not correct");
		return NULL;
	}

	def = xmlGetProp (tree, "Default");

	option = gpa_option_list_new (parent, id, def);

	node = tree->xmlChildrenNode;
	for (; node != NULL; node = node->next) {
		
		if (node->type != XML_ELEMENT_NODE)
			continue;

		if (!strcmp (node->name, "Item")) {
			GPANode *item;
			item = gpa_option_new_from_tree (node, option);
			if (!item) {
				g_warning ("Could not create option "
					   "from a <Item> node");
				continue;
			}
			if (GPA_NODE_ID_COMPARE (item, def))
				has_default = TRUE;
			continue;
		}
		
		if (!strcmp (node->name, "Fill")) {
			if (!gpa_option_new_from_fill_tag (node, option, def, &has_default))
				option = NULL;
#if 0
			/* FIXME: we need to deatach the option correctly */
			gpa_node_detach_unref (parent, option);
#endif	
			goto option_list_new_from_tree_done;
		}

		g_warning ("Invalid XML node as a child of <Option Type=\"List\"> (%s)", node->name);
	}

	if (!has_default) {
		g_warning ("Invalid default value in %s, default was set to "
			   "\"%s\" but could not be found", id, def);
#if 0	
		gpa_node_unref (option);
#endif
		option = NULL;
	}

	gpa_node_reverse_children (option);
	
option_list_new_from_tree_done:
	
	xmlFree (def);

	return option;
}

static GPANode *
gpa_option_item_new_from_tree (xmlNodePtr tree, GPANode *parent, const guchar *id)
{
	xmlNodePtr node;
	GPANode *option;
	xmlChar *value;

	if (!gpa_option_xml_check (tree, id, GPA_NOT, GPA_NOT, GPA_MUST, GPA_MAYBE)) {
		g_warning ("Option item structure is not correct");
		return NULL;
	}

	value = gpa_xml_node_get_name (tree);
	g_return_val_if_fail (value != NULL, NULL);
	option = gpa_option_item_new (parent, id, value);
	xmlFree (value);

	node = tree->xmlChildrenNode;
	for (; node != NULL; node = node->next) {
		
		if (node->type != XML_ELEMENT_NODE)
			continue;

		if (!strcmp (node->name, "Option") ||
		    !strcmp (node->name, "Key")) {
			gpa_option_new_from_tree (node, option);
			continue;
		}

		if (!strcmp (node->name, "Name"))
			continue;
		
		g_warning ("Invalid XML node as a child of <Option Type=\"Item\"> (%s)", node->name);
		return NULL;
	}

	gpa_node_reverse_children (option);
	
	return option;
}

static GPANode *
gpa_option_string_new_from_tree (xmlNodePtr tree, GPANode *parent, const guchar *id)
{
	GPANode *option;
	xmlChar *value;

	if (!gpa_option_xml_check (tree, id, GPA_MUST, GPA_NOT, GPA_NOT, GPA_NOT)) {
		g_warning ("Option string structure is not correct");
		return NULL;
	}

	value = xmlGetProp (tree, "Default");
	option = gpa_option_string_new (parent, id, value);
	xmlFree (value);
	
	return option;
}

GPANode *
gpa_option_get_child_by_id (GPAOption *option, const guchar *id)
{
	GPANode *child;

	g_return_val_if_fail (option != NULL, NULL);
	g_return_val_if_fail (GPA_IS_OPTION (option), NULL);
	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (*id != '\0', NULL);

	child = GPA_NODE (option)->children;
	while (child) {
		if (GPA_NODE_ID_COMPARE (child, id))
			break;
		child = child->next;
	}

	if (!child) {
		g_warning ("Could not find child for option \"%s\" with id \"%s\"",
			   GPA_NODE_ID (option), id);
		return NULL;
	}

	return gpa_node_ref (child);
}

static GPANode *
gpa_option_new (GPANode *parent, GPAOptionType type,
		const guchar *id, const guchar *value)
{
	GPAOption *option;

	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (*id != '\0', NULL);
	g_return_val_if_fail (parent != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (parent), NULL);
	
	option = (GPAOption *) gpa_node_new (GPA_TYPE_OPTION, id);
	option->type = type;

	if (value)
		option->value = g_strdup (value);

	gpa_node_attach (parent, GPA_NODE (option));
	
	return (GPANode *) option;	
}

GPANode *
gpa_option_item_new (GPANode *parent, const guchar *id, const guchar *name)
{
	return gpa_option_new (parent, GPA_OPTION_TYPE_ITEM, id, name);
}

GPANode *
gpa_option_node_new (GPANode *parent, const guchar *id)
{
	return gpa_option_new (parent, GPA_OPTION_TYPE_NODE, id, NULL);
}

GPANode *
gpa_option_list_new (GPANode *parent, const guchar *id, const guchar *value)
{
	return gpa_option_new (parent, GPA_OPTION_TYPE_LIST, id, value);
}

GPANode *
gpa_option_string_new (GPANode *parent, const guchar *id, const guchar *value)
{
	return gpa_option_new (parent, GPA_OPTION_TYPE_STRING, id, value);
}

GPANode *
gpa_option_key_new (GPANode *parent, const guchar *id, const guchar *value)
{
	return gpa_option_new (parent, GPA_OPTION_TYPE_KEY, id, value);
}

/* Helper for checking <Name> "default" and "value" elements */
static gboolean
gpa_option_xml_check (xmlNodePtr node, const gchar* id, gint def, gint val, gint name, gint children)
{
	if (GPA_UNAMBIGUOUS (def)) {
		xmlChar *xmlval = xmlGetProp (node, "Default");
		if (xmlval && GPA_FORBIDDEN (def)) {
			g_warning ("Node should not have the \"Default\" attribute (%s)", id);
			xmlFree (xmlval);
			return FALSE;
		} else if (!xmlval && GPA_MUSTHAVE (def)) {
			g_warning ("Node must have have the \"Default\" attribute (%s)", id);
			xmlFree (xmlval);
			return FALSE;
		}
		xmlFree (xmlval);
	}

	if (GPA_UNAMBIGUOUS (val)) {
		xmlChar *xmlval = xmlGetProp (node, "Value");
		if (xmlval && GPA_FORBIDDEN (val)) {
			g_warning ("Node does not have \"Value\" attribute");
			xmlFree (xmlval);
			return FALSE;
		} else if (!xmlval && GPA_MUSTHAVE (val)) {
			g_warning ("Node should not have \"Value\" attribute");
			xmlFree (xmlval);
			return FALSE;
		}
		xmlFree (xmlval);
	}

	if (GPA_UNAMBIGUOUS (name)) {
		xmlChar *xmlval = gpa_xml_node_get_name (node);
		if (xmlval && GPA_FORBIDDEN (name)) {
			g_warning ("Node does not have <Name> element");
			xmlFree (xmlval);
			return FALSE;
		} else if (!xmlval && GPA_MUSTHAVE (name)) {
			g_warning ("Node should not have <Name> element");
			xmlFree (xmlval);
			return FALSE;
		}
		xmlFree (xmlval);
	}

	if (GPA_UNAMBIGUOUS (children)) {
		xmlNodePtr child;
		gboolean haschild;
		/* This is quick check, as we do not verify the children */
		haschild = FALSE;
		for (child = node->xmlChildrenNode; child != NULL; child = child->next) {
			if (!strcmp (child->name, "Option") ||
			    !strcmp (child->name, "Item") ||
			    !strcmp (child->name, "Key") ||
			    !strcmp (child->name, "Fill")) {
				if (GPA_FORBIDDEN (children)) {
					g_warning ("Node should not have children (%s)", id);
					return FALSE;
				}
				haschild = TRUE;
			}
		}
		if (!haschild && GPA_MUSTHAVE (children)) {
			g_warning ("Node must have children (%s)", id);
			return FALSE;
		}
	}

	return TRUE;
}


/**
 * gpa_option_get_name:
 * @node: 
 * 
 * Returns the translated name for the @node option
 * 
 * Return Value: a strduped string on success, NULL otherwise
 **/
gchar *
gpa_option_get_name (GPANode *node)
{
	GPAOption *option;

	g_return_val_if_fail (GPA_IS_OPTION (node), NULL);

	option = GPA_OPTION (node);

	if (option->value == NULL)
		return NULL;

	return g_strdup (_(option->value));
}
