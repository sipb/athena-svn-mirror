/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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
 *  Copyright (C) 2000-2001 Ximian, Inc. and Jose M. Celorio
 *
 */

#define __GPA_OPTION_C__

#include <string.h>
#include <libxml/globals.h>
#include "gpa-utils.h"
#include "gpa-value.h"
#include "gpa-key.h"
#include "gpa-option.h"

static void gpa_option_class_init (GPAOptionClass *klass);
static void gpa_option_init (GPAOption *option);
static void gpa_option_finalize (GObject *object);

static GPANode *gpa_option_duplicate (GPANode *node);
static gboolean gpa_option_verify (GPANode *node);
static guchar *gpa_option_get_value (GPANode *node);
static GPANode *gpa_option_get_child (GPANode *node, GPANode *ref);
static GPANode *gpa_option_lookup (GPANode *node, const guchar *path);
static void gpa_option_modified (GPANode *node, guint flags);

static GPANode *gpa_option_create_key_private (GPAOption *option);

static GPAOption *gpa_option_new_node_from_tree (xmlNodePtr tree, const guchar *id);
static GPAOption *gpa_option_new_key_from_tree (xmlNodePtr tree, const guchar *id);
static GPAOption *gpa_option_new_list_from_tree (xmlNodePtr tree, const guchar *id);
static GPAOption *gpa_option_new_item_from_tree (xmlNodePtr tree, const guchar *id);
static GPAOption *gpa_option_new_string_from_tree (xmlNodePtr tree, const guchar *id);

#define GPA_NOT -1
#define GPA_MAYBE 0
#define GPA_MUST 1

#define GPA_FORBIDDEN(v) ((v) < 0)
#define GPA_ALLOWED(v) ((v) == 0)
#define GPA_MUSTHAVE(v) ((v) > 0)
#define GPA_UNAMBIGUOUS(v) ((v) != 0)

static gboolean gpa_option_xml_check (xmlNodePtr node, gint def, gint val, gint name, gint children);

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
	node_class->verify = gpa_option_verify;
	node_class->get_value = gpa_option_get_value;
	node_class->get_child = gpa_option_get_child;
	node_class->lookup = gpa_option_lookup;
	node_class->modified = gpa_option_modified;

	klass->create_key = gpa_option_create_key_private;
}

static void
gpa_option_init (GPAOption *option)
{
	option->type = GPA_OPTION_TYPE_NONE;
	option->children = NULL;
	option->value = NULL;
}

static void
gpa_option_finalize (GObject *object)
{
	GPAOption *option;

	option = GPA_OPTION (object);

	if (option->name) {
		option->name = gpa_node_detach_unref (GPA_NODE (option), option->name);
	}

	while (option->children) {
		option->children = gpa_node_detach_unref_next (GPA_NODE (option), option->children);
	}

	if (option->value) {
		g_free (option->value);
		option->value = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GPANode *
gpa_option_duplicate (GPANode *node)
{
	GPAOption *option, *new;
	GPANode *child;
	GSList *l;

	option = GPA_OPTION (node);

	new = (GPAOption *) gpa_node_new (GPA_TYPE_OPTION, GPA_NODE_ID (node));
	new->type = option->type;
	if (option->name) {
		new->name = gpa_node_attach (GPA_NODE (new), gpa_node_duplicate (option->name));
	}
	if (option->value) {
		new->value = g_strdup (option->value);
	}

	l = NULL;
	for (child = option->children; child != NULL; child = child->next) {
		GPANode *newchild;
		newchild = gpa_node_duplicate (child);
		if (newchild)
			l = g_slist_prepend (l, newchild);
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
gpa_option_verify (GPANode *node)
{
	GPAOption *option;
	GPANode *child;

	option = GPA_OPTION (node);

	/* Each option has to have Id */
	if (!GPA_NODE_ID_EXISTS (node))
		return FALSE;

	switch (option->type) {
	case GPA_OPTION_TYPE_NODE:
		/* !Name, !Value, children */
		if (option->name)
			return FALSE;
		if (option->value)
			return FALSE;
		if (!option->children)
			return FALSE;
		for (child = option->children; child != NULL; child = child->next) {
			if (!GPA_IS_OPTION (child))
				return FALSE;
			if (!gpa_node_verify (child))
				return FALSE;
		}
		break;
	case GPA_OPTION_TYPE_KEY:
		/* !Name, Value || children */
		if (option->name)
			return FALSE;
		if (!option->value && !option->children)
			return FALSE;
		for (child = option->children; child != NULL; child = child->next) {
			if (!GPA_IS_OPTION (child))
				return FALSE;
			if (GPA_OPTION (child)->type != GPA_OPTION_TYPE_KEY)
				return FALSE;
			if (!gpa_node_verify (child))
				return FALSE;
		}
		break;
	case GPA_OPTION_TYPE_LIST:
		/* List should not have name */
		if (option->name)
			return FALSE;
		/* List has to have default value */
		if (!option->value)
			return FALSE;
		/* List has to have children of type item */
		if (!option->children)
			return FALSE;
		for (child = option->children; child != NULL; child = child->next) {
			if (!GPA_IS_OPTION (child))
				return FALSE;
			if (GPA_OPTION (child)->type != GPA_OPTION_TYPE_ITEM)
				return FALSE;
			if (!gpa_node_verify (child))
				return FALSE;
		}
		break;
	case GPA_OPTION_TYPE_ITEM:
		/* Item has to have name */
		if (!option->name)
			return FALSE;
		if (!gpa_node_verify (option->name))
			return FALSE;
		/* Item should not have value */
		if (option->value)
			return FALSE;
		/* Item may have children */
		for (child = option->children; child != NULL; child = child->next) {
			if (!GPA_IS_OPTION (child))
				return FALSE;
			if (!gpa_node_verify (child))
				return FALSE;
		}
		break;
	case GPA_OPTION_TYPE_STRING:
		/* String should not have name */
		if (option->name)
			return FALSE;
		/* String should have value */
		if (!option->value)
			return FALSE;
		/* String should not have children */
		if (option->children)
			return FALSE;
		break;
	default:
		return FALSE;
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
	if (GPA_NODE_ID_EXISTS (node))
		return g_strdup (GPA_NODE_ID (node));

	return NULL;
}

static GPANode *
gpa_option_get_child (GPANode *node, GPANode *ref)
{
	GPAOption *option;

	option = GPA_OPTION (node);

	if (!ref) {
		if (option->children)
			gpa_node_ref (option->children);
		return option->children;
	}

	if (ref->next)
		gpa_node_ref (ref->next);

	return ref->next;
}

static GPANode *
gpa_option_lookup (GPANode *node, const guchar *path)
{
	GPAOption *option;
	GPANode *child;
	const guchar *dot, *next;
	gint len;

	option = GPA_OPTION (node);

	if (!strncmp (path, "Name", 4)) {
		if (!option->name)
			return NULL;
		if (!path[4]) {
			gpa_node_ref (GPA_NODE (option->name));
			return GPA_NODE (option->name);
		} else {
			g_return_val_if_fail (path[4] == '.', NULL);
			return gpa_node_lookup (GPA_NODE (option->name), path + 5);
		}
	}

	dot = strchr (path, '.');
	if (dot != NULL) {
		len = dot - path;
		next = dot + 1;
	} else {
		len = strlen (path);
		next = path + len;
	}

	for (child = option->children; child != NULL; child = child->next) {
		const guchar *cid;
		g_assert (GPA_IS_OPTION (child));
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
gpa_option_modified (GPANode *node, guint flags)
{
	GPAOption *option;
	GPANode *child;

	option = GPA_OPTION (node);

	if (option->name && (GPA_NODE_FLAGS (option->name) & GPA_MODIFIED_FLAG)) {
		gpa_node_emit_modified (option->name, 0);
	}

	child = option->children;
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

static GPANode *
gpa_option_create_key_private (GPAOption *option)
{
	GPAKey *key;
	GPANode *child;
	GSList *l;

	key = NULL;

	switch (option->type) {
	case GPA_OPTION_TYPE_NODE:
	case GPA_OPTION_TYPE_KEY:
	case GPA_OPTION_TYPE_STRING:
		key = (GPAKey *) gpa_node_new (GPA_TYPE_KEY, GPA_NODE_ID (option));
		key->option = GPA_NODE (option);
		gpa_node_ref (key->option);
		if (option->value)
			key->value = g_strdup (option->value);
		l = NULL;
		for (child = option->children; child != NULL; child = child->next) {
			GPANode *kch;
			kch = gpa_option_create_key (GPA_OPTION (child));
			if (kch) {
				l = g_slist_prepend (l, kch);
			}
		}
		while (l) {
			GPANode *kch;
			kch = GPA_NODE (l->data);
			l = g_slist_remove (l, kch);
			kch->parent = GPA_NODE (key);
			kch->next = key->children;
			key->children = kch;
		}
		break;
	case GPA_OPTION_TYPE_LIST:
		key = (GPAKey *) gpa_node_new (GPA_TYPE_KEY, GPA_NODE_ID (option));
		key->option = GPA_NODE (option);
		gpa_node_ref (key->option);
		key->value = g_strdup (option->value);
		child = (GPANode *) gpa_option_get_child_by_id (option, option->value);
		if (child) {
			GPANode *ich;
			l = NULL;
			for (ich = GPA_OPTION (child)->children; ich != NULL; ich = ich->next) {
				GPANode *kch;
				kch = gpa_option_create_key (GPA_OPTION (ich));
				if (kch)
					l = g_slist_prepend (l, kch);
			}
			while (l) {
				GPANode *kch;
				kch = GPA_NODE (l->data);
				l = g_slist_remove (l, kch);
				kch->parent = GPA_NODE (key);
				kch->next = key->children;
				key->children = kch;
			}
			gpa_node_unref (child);
			break;
		} else {
			g_warning ("Default was not in list (memory leak)");
		}
		break;
	case GPA_OPTION_TYPE_ITEM:
		/* No key for item */
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	return (GPANode *) key;
}

GPANode *
gpa_option_new_from_tree (xmlNodePtr tree)
{
	GPAOption *option;
	xmlChar *xmlid;

	g_return_val_if_fail (tree != NULL, NULL);

	xmlid = xmlGetProp (tree, "Id");
	if (!xmlid) {
		g_warning ("Option node does not have Id");
		return NULL;
	}

	option = NULL;

	if (!strcmp (tree->name, "Key")) {
		option = gpa_option_new_key_from_tree (tree, xmlid);
	} else if (!strcmp (tree->name, "Item")) {
		option = gpa_option_new_item_from_tree (tree, xmlid);
	} else if (!strcmp (tree->name, "Option")) {
		xmlChar *xmltype;
		xmltype = xmlGetProp (tree, "Type");
		if (!xmltype) {
			option = gpa_option_new_node_from_tree (tree, xmlid);
		} else if (!strcmp (xmltype, "List")) {
			xmlFree (xmltype);
			option = gpa_option_new_list_from_tree (tree, xmlid);
		} else if (!strcmp (xmltype, "String")) {
			xmlFree (xmltype);
			option = gpa_option_new_string_from_tree (tree, xmlid);
		} else {
			xmlFree (xmltype);
		}
	}

	xmlFree (xmlid);

	return (GPANode *) option;
}

/* !Name, !Value, children */

static GPAOption *
gpa_option_new_node_from_tree (xmlNodePtr tree, const guchar *id)
{
	GPAOption *option;
	xmlNodePtr xmlc;
	GSList *l;

	if (!gpa_option_xml_check (tree, GPA_NOT, GPA_NOT, GPA_NOT, GPA_MUST)) {
		g_warning ("Option node structure is not correct");
		return NULL;
	}

	l = NULL;

	for (xmlc = tree->xmlChildrenNode; xmlc != NULL; xmlc = xmlc->next) {
		if (xmlc->type == XML_ELEMENT_NODE) {
			if (!strcmp (xmlc->name, "Option") || !strcmp (xmlc->name, "Key")) {
				GPANode *cho;
				cho = gpa_option_new_from_tree (xmlc);
				if (cho) l = g_slist_prepend (l, cho);
			} else {
				g_warning ("Invalid child in option tree %s", xmlc->name);
			}
		}
	}

	if (!l) {
		g_warning ("Option should have children");
		return NULL;
	}

	option = (GPAOption *) gpa_node_new (GPA_TYPE_OPTION, id);
	option->type = GPA_OPTION_TYPE_NODE;

	while (l) {
		GPANode *cho;
		cho = GPA_NODE (l->data);
		l = g_slist_remove (l, cho);
		cho->parent = GPA_NODE (option);
		cho->next = option->children;
		option->children = cho;
	}

	return option;
}

/* !Name, Value || children */

static GPAOption *
gpa_option_new_key_from_tree (xmlNodePtr tree, const guchar *id)
{
	GPAOption *option;
	xmlChar *xmlval;
	xmlNodePtr xmlc;
	GSList *l;

	if (!gpa_option_xml_check (tree, GPA_NOT, GPA_MAYBE, GPA_NOT, GPA_MAYBE)) {
		g_warning ("Option key structure is not correct");
		return NULL;
	}

	xmlval = xmlGetProp (tree, "Value");
	if (!xmlval && !tree->xmlChildrenNode) {
		g_warning ("Key node should have value or children or both");
		return NULL;
	}

	l = NULL;

	for (xmlc = tree->xmlChildrenNode; xmlc != NULL; xmlc = xmlc->next) {
		if (xmlc->type == XML_ELEMENT_NODE) {
			if (!strcmp (xmlc->name, "Key")) {
				GPANode *cho;
				cho = gpa_option_new_from_tree (xmlc);
				if (cho) l = g_slist_prepend (l, cho);
			} else {
				g_warning ("Invalid child in option tree %s", xmlc->name);
			}
		}
	}

	if (!xmlval && !l) {
		g_warning ("Key node should have value or children or both");
		return NULL;
	}

	option = (GPAOption *) gpa_node_new (GPA_TYPE_OPTION, id);
	option->type = GPA_OPTION_TYPE_KEY;
	if (xmlval) {
		option->value = g_strdup (xmlval);
		xmlFree (xmlval);
	}

	while (l) {
		GPANode *cho;
		cho = GPA_NODE (l->data);
		l = g_slist_remove (l, cho);
		cho->parent = GPA_NODE (option);
		cho->next = option->children;
		option->children = cho;
	}

	return option;
}

/* !Name, default, children */

static GPAOption *
gpa_option_new_list_from_tree (xmlNodePtr tree, const guchar *id)
{
	GPAOption *option;
	xmlChar *xmldef, *xmlval;
	xmlNodePtr xmlc;
	GSList *l;
	gboolean has_default;

	if (!gpa_option_xml_check (tree, GPA_MUST, GPA_NOT, GPA_NOT, GPA_MUST)) {
		g_warning ("Option list structure is not correct");
		return NULL;
	}

	xmldef = xmlGetProp (tree, "Default");

	l = NULL;
	has_default = FALSE;

	for (xmlc = tree->xmlChildrenNode; xmlc != NULL; xmlc = xmlc->next) {
		if (xmlc->type == XML_ELEMENT_NODE) {
			if (!strcmp (xmlc->name, "Item")) {
				GPANode *cho;
				cho = gpa_option_new_from_tree (xmlc);
				if (cho) {
					l = g_slist_prepend (l, cho);
					if (GPA_NODE_ID_COMPARE (cho, xmldef))
						has_default = TRUE;
				}
			} else if (!strcmp (xmlc->name, "Fill")) {
				xmlval = xmlGetProp (xmlc, "Ref");
				if (xmlval) {
					GPANode *def, *ref;
#ifdef __GNUC__	
#warning FIXME: This seems broken because this function is called when gpa_defaults is called so we recurse into here					
#endif	
					def = gpa_defaults ();
					ref = gpa_node_get_path_node (def, xmlval);
					gpa_node_unref (def);
					xmlFree (xmlval);
					if (GPA_OPTION_IS_LIST (ref)) {
						GPANode *lc;
						for (lc = ((GPAOption *) ref)->children; lc != NULL; lc = lc->next) {
							GPANode *new;
							new = gpa_node_duplicate (lc);
							l = g_slist_prepend (l, new);
							if (GPA_NODE_ID_COMPARE (new, xmldef))
								has_default = TRUE;
						}
					}
					if (ref)
						gpa_node_unref (ref);
				}
			} else {
				g_warning ("Invalid list child in option tree %s", xmlc->name);
			}
		}
	}

	if (!has_default) {
		g_warning ("Invalid default value in option list %s", xmldef);
		while (l) {
			gpa_node_unref (GPA_NODE (l->data));
			l = g_slist_remove (l, l->data);
		}
		xmlFree (xmldef);
		return NULL;
	}

	if (!l) {
		g_warning ("List has to have children of type item");
		xmlFree (xmldef);
		return NULL;
	}

	option = (GPAOption *) gpa_node_new (GPA_TYPE_OPTION, id);
	option->type = GPA_OPTION_TYPE_LIST;
	option->value = g_strdup (xmldef);
	xmlFree (xmldef);

	while (l) {
		GPANode *cho;
		cho = GPA_NODE (l->data);
		l = g_slist_remove (l, cho);
		cho->parent = GPA_NODE (option);
		cho->next = option->children;
		option->children = cho;
	}

	return option;
}

/* Name, !value, optional children */

static GPAOption *
gpa_option_new_item_from_tree (xmlNodePtr tree, const guchar *id)
{
	GPAOption *option;
	xmlChar *xmlval;
	xmlNodePtr xmlc;
	GPANode *name;
	GSList *l;

	if (!gpa_option_xml_check (tree, GPA_NOT, GPA_NOT, GPA_MUST, GPA_MAYBE)) {
		g_warning ("Option item structure is not correct");
		return NULL;
	}

	xmlval = gpa_xml_node_get_name (tree);
	name = gpa_value_new ("Name", xmlval);
	xmlFree (xmlval);

	l = NULL;

	for (xmlc = tree->xmlChildrenNode; xmlc != NULL; xmlc = xmlc->next) {
		if (xmlc->type == XML_ELEMENT_NODE) {
			if (!strcmp (xmlc->name, "Option") || !strcmp (xmlc->name, "Key")) {
				GPANode *cho;
				cho = gpa_option_new_from_tree (xmlc);
				if (cho)
					l = g_slist_prepend (l, cho);
			} else if (strcmp (xmlc->name, "Name")) {
				g_warning ("Invalid tag in option tree %s", xmlc->name);
			}
		}
	}

	option = (GPAOption *) gpa_node_new (GPA_TYPE_OPTION, id);
	option->type = GPA_OPTION_TYPE_ITEM;
	option->name = gpa_node_attach (GPA_NODE (option), name);

	while (l) {
		GPANode *cho;
		cho = GPA_NODE (l->data);
		l = g_slist_remove (l, cho);
		cho->parent = GPA_NODE (option);
		cho->next = option->children;
		option->children = cho;
	}

	return option;
}

/* !Name, default, !children */

static GPAOption *
gpa_option_new_string_from_tree (xmlNodePtr tree, const guchar *id)
{
	GPAOption *option;
	xmlChar *defval;

	if (!gpa_option_xml_check (tree, GPA_MUST, GPA_NOT, GPA_NOT, GPA_NOT)) {
		g_warning ("Option string structure is not correct");
		return NULL;
	}

	defval = xmlGetProp (tree, "Default");

	option = (GPAOption *) gpa_node_new (GPA_TYPE_OPTION, id);
	option->type = GPA_OPTION_TYPE_STRING;
	option->value = g_strdup (defval);
	xmlFree (defval);

	return option;
}

/* GPAOptionList */

GPAList *
gpa_option_list_new_from_tree (xmlNodePtr tree)
{
	GPAList *options;
	xmlNodePtr xmlc;
	GSList *l;

	g_return_val_if_fail (!strcmp (tree->name, "Options"), NULL);

	l = NULL;
	for (xmlc = tree->xmlChildrenNode; xmlc != NULL; xmlc = xmlc->next) {
		if (!strcmp (xmlc->name, "Option") || !strcmp (xmlc->name, "Item") || !strcmp (xmlc->name, "Key")) {
			GPANode *option;
			option = gpa_option_new_from_tree (xmlc);
			if (option)
				l = g_slist_prepend (l, option);
		}
	}

	options = GPA_LIST (gpa_list_new (GPA_TYPE_OPTION, FALSE));

	while (l) {
		GPANode *option;
		option = GPA_NODE (l->data);
		l = g_slist_remove (l, option);
		option->parent = GPA_NODE (options);
		option->next = options->children;
		options->children = option;
	}

	return options;
}

/* Public interface */

GPANode *
gpa_option_create_key (GPAOption *option)
{
	g_return_val_if_fail (option != NULL, NULL);
	g_return_val_if_fail (GPA_IS_OPTION (option), NULL);

	if (GPA_OPTION_GET_CLASS (option)->create_key)
		return GPA_OPTION_GET_CLASS (option)->create_key (option);

	return NULL;
}

/* Strictly private helpers */

GPAOption *
gpa_option_get_child_by_id (GPAOption *option, const guchar *id)
{
	GPANode *child;

	g_return_val_if_fail (option != NULL, NULL);
	g_return_val_if_fail (GPA_IS_OPTION (option), NULL);
	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (*id != '\0', NULL);

	for (child = option->children; child != NULL; child = child->next) {
		if (GPA_NODE_ID_COMPARE (child, id)) {
			g_assert (GPA_IS_OPTION (child));
			gpa_node_ref (child);
			return GPA_OPTION (child);
		}
	}

	return NULL;
}

GPANode *
gpa_option_node_new (const guchar *id)
{
	GPAOptionNode *option;

	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (*id != '\0', NULL);

	option = (GPAOption *) gpa_node_new (GPA_TYPE_OPTION, id);
	option->type = GPA_OPTION_TYPE_NODE;

	return (GPANode *) option;
}

GPANode *
gpa_option_list_new (const guchar *id)
{
	GPAOptionNode *option;

	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (*id != '\0', NULL);

	option = (GPAOption *) gpa_node_new (GPA_TYPE_OPTION, id);
	option->type = GPA_OPTION_TYPE_LIST;

	return (GPANode *) option;
}

GPANode *
gpa_option_item_new (const guchar *id, const guchar *name)
{
	GPAOptionNode *option;
	GPANode *child;

	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (*id != '\0', NULL);
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (*name != '\0', NULL);

	child = gpa_value_new ("Name", name);
	g_return_val_if_fail (child != NULL, NULL);

	option = (GPAOption *) gpa_node_new (GPA_TYPE_OPTION, id);
	option->type = GPA_OPTION_TYPE_ITEM;
	option->name = gpa_node_attach (GPA_NODE (option), child);

	return (GPANode *) option;
}

GPANode *
gpa_option_string_new (const guchar *id, const guchar *value)
{
	GPAOptionNode *option;

	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (*id != '\0', NULL);
	g_return_val_if_fail (value != NULL, NULL);
	g_return_val_if_fail (*value != '\0', NULL);

	option = (GPAOption *) gpa_node_new (GPA_TYPE_OPTION, id);
	option->type = GPA_OPTION_TYPE_STRING;
	option->value = g_strdup (value);

	return (GPANode *) option;
}

GPANode *
gpa_option_key_new (const guchar *id, const guchar *value)
{
	GPAOptionNode *option;

	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (*id != '\0', NULL);
	g_return_val_if_fail (!value || *value != '\0', NULL);

	option = (GPAOption *) gpa_node_new (GPA_TYPE_OPTION, id);
	option->type = GPA_OPTION_TYPE_KEY;
	if (value)
		option->value = g_strdup (value);

	return (GPANode *) option;
}

gboolean
gpa_option_node_append_child (GPAOptionNode *option, GPAOption *child)
{
	GPANode *ref;

	g_return_val_if_fail (option != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_OPTION (option), FALSE);
	g_return_val_if_fail (option->type == GPA_OPTION_TYPE_NODE, FALSE);
	g_return_val_if_fail (child != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_OPTION (child), FALSE);
	g_return_val_if_fail (GPA_NODE_PARENT (child) == NULL, FALSE);

	ref = option->children;
	while (ref && ref->next) ref = ref->next;

	if (!ref) {
		option->children = gpa_node_attach_ref (GPA_NODE (option), GPA_NODE (child));
	} else {
		ref->next = gpa_node_attach_ref (GPA_NODE (option), GPA_NODE (child));
	}

	gpa_node_request_modified (GPA_NODE (option), 0);

	return TRUE;
}

gboolean
gpa_option_list_append_child (GPAOptionList *option, GPAOption *child)
{
	GPANode *ref;

	g_return_val_if_fail (option != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_OPTION (option), FALSE);
	g_return_val_if_fail (option->type == GPA_OPTION_TYPE_LIST, FALSE);
	g_return_val_if_fail (child != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_OPTION (child), FALSE);
	g_return_val_if_fail (GPA_NODE_PARENT (child) == NULL, FALSE);

	ref = option->children;
	while (ref && ref->next) ref = ref->next;

	if (!ref) {
		option->children = gpa_node_attach_ref (GPA_NODE (option), GPA_NODE (child));
	} else {
		ref->next = gpa_node_attach_ref (GPA_NODE (option), GPA_NODE (child));
	}

	if (!option->value) {
		option->value = g_strdup (GPA_NODE_ID (child));
	}

	gpa_node_request_modified (GPA_NODE (option), 0);

	return TRUE;
}

gboolean
gpa_option_item_append_child (GPAOptionItem *option, GPAOption *child)
{
	GPANode *ref;

	g_return_val_if_fail (option != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_OPTION (option), FALSE);
	g_return_val_if_fail (option->type == GPA_OPTION_TYPE_ITEM, FALSE);
	g_return_val_if_fail (child != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_OPTION (child), FALSE);
	g_return_val_if_fail (GPA_NODE_PARENT (child) == NULL, FALSE);

	ref = option->children;
	while (ref && ref->next) ref = ref->next;

	if (!ref) {
		option->children = gpa_node_attach_ref (GPA_NODE (option), GPA_NODE (child));
	} else {
		ref->next = gpa_node_attach_ref (GPA_NODE (option), GPA_NODE (child));
	}

	gpa_node_request_modified (GPA_NODE (option), 0);

	return TRUE;
}

gboolean
gpa_option_key_append_child (GPAOptionKey *option, GPAOption *child)
{
	GPANode *ref;

	g_return_val_if_fail (option != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_OPTION (option), FALSE);
	g_return_val_if_fail (option->type == GPA_OPTION_TYPE_KEY, FALSE);
	g_return_val_if_fail (child != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_OPTION (child), FALSE);
	g_return_val_if_fail (GPA_NODE_PARENT (child) == NULL, FALSE);

	ref = option->children;
	while (ref && ref->next) ref = ref->next;

	if (!ref) {
		option->children = gpa_node_attach_ref (GPA_NODE (option), GPA_NODE (child));
	} else {
		ref->next = gpa_node_attach_ref (GPA_NODE (option), GPA_NODE (child));
	}

	gpa_node_request_modified (GPA_NODE (option), 0);

	return TRUE;
}

/* Helper for checking <Name> "default" and "value" elements */

static gboolean
gpa_option_xml_check (xmlNodePtr node, gint def, gint val, gint name, gint children)
{
	if (GPA_UNAMBIGUOUS (def)) {
		xmlChar *xmlval = xmlGetProp (node, "Default");
		if (xmlval && GPA_FORBIDDEN (def)) {
			g_warning ("Node does not have \"Default\" attribute");
			xmlFree (xmlval);
			return FALSE;
		} else if (!xmlval && GPA_MUSTHAVE (def)) {
			g_warning ("Node should not have \"Default\" attribute");
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
		/* This is quick check, as we do not verify children */
		haschild = FALSE;
		for (child = node->xmlChildrenNode; child != NULL; child = child->next) {
			if (!strcmp (child->name, "Option") ||
			    !strcmp (child->name, "Item") ||
			    !strcmp (child->name, "Key") ||
			    !strcmp (child->name, "Fill")) {
				if (GPA_FORBIDDEN (children)) {
					g_warning ("Node should not have children");
					return FALSE;
				}
				haschild = TRUE;
			}
		}
		if (!haschild && GPA_MUSTHAVE (children)) {
			g_warning ("Node must have children");
			return FALSE;
		}
	}

	return TRUE;
}

