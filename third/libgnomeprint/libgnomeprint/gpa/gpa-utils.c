/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-utils.c:
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
 *  Contains pieces of code from glib, Copyright GLib Team and others 1997-2000.
 *
 */

#define __GPA_UTILS_C__

#include <string.h>
#include <unistd.h>
#include <time.h>
#include <libxml/parser.h>
#include "gpa-node-private.h"
#include "gpa-utils.h"
#include "gpa-value.h"
#include "gpa-reference.h"

#define noGPA_CACHE_DEBUG
#define noGPA_QUARK_DEBUG

guchar *
gpa_id_new (const guchar *key)
{
	static gint counter = 0;
	guchar *id;

	/* fixme: This is not very intelligent (Lauris) */
	id = g_strdup_printf ("%s-%.12d%.6d%.6d", key, (gint) time (NULL), getpid (), counter++);

	return id;
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

	return child;
}

GPANode *
gpa_node_attach_ref (GPANode *parent, GPANode *child)
{
	g_return_val_if_fail (parent != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (parent), NULL);
	g_return_val_if_fail (child != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (child), NULL);
	g_return_val_if_fail (child->parent == NULL, NULL);
	g_return_val_if_fail (child->next == NULL, NULL);

	gpa_node_ref (child);

	child->parent = parent;

	return child;
}

GPANode *
gpa_node_detach (GPANode *parent, GPANode *child)
{
	g_return_val_if_fail (parent != NULL, child);
	g_return_val_if_fail (GPA_IS_NODE (parent), child);
	g_return_val_if_fail (child != NULL, child);
	g_return_val_if_fail (GPA_IS_NODE (child), child);
	g_return_val_if_fail (child->parent == parent, child);

	child->parent = NULL;
	child->next = NULL;

	return NULL;
}

GPANode *
gpa_node_detach_unref (GPANode *parent, GPANode *child)
{
	g_return_val_if_fail (parent != NULL, child);
	g_return_val_if_fail (GPA_IS_NODE (parent), child);
	g_return_val_if_fail (child != NULL, child);
	g_return_val_if_fail (GPA_IS_NODE (child), child);
	g_return_val_if_fail (child->parent == parent, child);

	child->parent = NULL;
	child->next = NULL;

	gpa_node_unref (child);

	return NULL;
}

GPANode *
gpa_node_detach_next (GPANode *parent, GPANode *child)
{
	GPANode *next;

	g_return_val_if_fail (parent != NULL, child);
	g_return_val_if_fail (GPA_IS_NODE (parent), child);
	g_return_val_if_fail (child != NULL, child);
	g_return_val_if_fail (GPA_IS_NODE (child), child);
	g_return_val_if_fail (child->parent == parent, child);

	next = child->next;
	child->parent = NULL;
	child->next = NULL;

	return next;
}

GPANode *
gpa_node_detach_unref_next (GPANode *parent, GPANode *child)
{
	GPANode *next;

	g_return_val_if_fail (parent != NULL, child);
	g_return_val_if_fail (GPA_IS_NODE (parent), child);
	g_return_val_if_fail (child != NULL, child);
	g_return_val_if_fail (GPA_IS_NODE (child), child);
	g_return_val_if_fail (child->parent == parent, child);

	next = child->next;
	child->parent = NULL;
	child->next = NULL;

	gpa_node_unref (child);

	return next;
}

const guchar *
gpa_node_lookup_check (const guchar *path, const guchar *key)
{
	gint len;

	g_return_val_if_fail (path != NULL, NULL);
	g_return_val_if_fail (*path != '\0', NULL);
	g_return_val_if_fail (key != NULL, NULL);
	g_return_val_if_fail (*key != '\0', NULL);

	len = strlen (key);

	if (!strncmp (path, key, len)) {
		if (!path[len]) return path + len;
		if (path[len] == '.') return path + len + 1;
	}

	return NULL;
}

gboolean
gpa_node_lookup_ref (GPANode **child, GPANode *node, const guchar *path, const guchar *key)
{
	gint len;

	g_return_val_if_fail (*child == NULL, TRUE);
	g_return_val_if_fail (node != NULL, TRUE);
	g_return_val_if_fail (GPA_IS_NODE (node), TRUE);
	g_return_val_if_fail (path != NULL, TRUE);
	g_return_val_if_fail (*path != '\0', TRUE);
	g_return_val_if_fail (key != NULL, TRUE);
	g_return_val_if_fail (*key != '\0', TRUE);

	len = strlen (key);

	if (!strncmp (path, key, len)) {
		if (!path[len]) {
			gpa_node_ref (node);
			*child = node;
			return TRUE;
		} else {
			if (path[len] != '.') return FALSE;
			*child = gpa_node_lookup (node, path + len + 1);
			return TRUE;
		}
	}

	return FALSE;
}

/* XML helpers */

xmlChar *
gpa_xml_node_get_name (xmlNodePtr node)
{
	xmlNodePtr child;

	g_return_val_if_fail (node != NULL, NULL);

	for (child = node->xmlChildrenNode; child != NULL; child = child->next) {
		if (!strcmp (child->name, "Name")) {
			return xmlNodeGetContent (child);
		}
	}

	return NULL;
}

xmlNodePtr
gpa_xml_node_get_child (xmlNodePtr node, const guchar *name)
{
	xmlNodePtr child;

	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	for (child = node->xmlChildrenNode; child != NULL; child = child->next) {
		if (!strcmp (child->name, name)) return child;
	}

	return NULL;
}

static gboolean
gpa_node_cache_timeout (gpointer data)
{
#ifdef GPA_CACHE_DEBUG
	g_print ("GPACache: Dropping reference from %p\n", data);
#endif
	g_object_set_data (G_OBJECT (data), "gpa_cache_timeout", NULL);

	gpa_node_unref (GPA_NODE (data));

	return FALSE;
}

GPANode *
gpa_node_cache (GPANode *node)
{
	gint id;

	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (node), NULL);

	id = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (node), "gpa_cache_timeout"));

	if (id) {
		g_source_remove (id);
	} else {
		gpa_node_ref (node);
	}

	id = g_timeout_add (10000, gpa_node_cache_timeout, node);

	g_object_set_data (G_OBJECT (node), "gpa_cache_timeout", GUINT_TO_POINTER (id));

	return node;
}

/* Private quarks */

#define GPA_QUARK_BLOCK_SIZE 128
static GHashTable *qdict = NULL;
static const guchar **qarray = NULL;
static GQuark qseq = 0;

static GPAQuark
gpa_quark_new (const guchar *string)
{
	if ((qseq % GPA_QUARK_BLOCK_SIZE) == 0) {
		qarray = g_renew (const guchar *, qarray, qseq + GPA_QUARK_BLOCK_SIZE);
#ifdef GPA_QUARK_DEBUG
		g_print ("GPAQuark: Increasing buffer, now %d\n", qseq + GPA_QUARK_BLOCK_SIZE);
#endif
	}

	qarray[qseq++] = string;

	g_hash_table_insert (qdict, (gpointer) string, GUINT_TO_POINTER (qseq));

#ifdef GPA_QUARK_DEBUG
	g_print ("GPAQuark: New quark %d:%s\n", qseq, string);
#endif

	return qseq;
}

GPAQuark
gpa_quark_try_string (const guchar *string)
{
	GPAQuark q;

	g_return_val_if_fail (string != NULL, 0);

	q = (qdict) ? GPOINTER_TO_INT (g_hash_table_lookup (qdict, string)) : 0;

	return q;
}

GPAQuark
gpa_quark_from_string (const guchar *string)
{
	GPAQuark q;

	g_return_val_if_fail (string != NULL, 0);

	if (!qdict) qdict = g_hash_table_new (g_str_hash, g_str_equal);

	q = GPOINTER_TO_INT (g_hash_table_lookup (qdict, string));

	if (!q) q = gpa_quark_new (g_strdup (string));

	return q;
}

GPAQuark
gpa_quark_from_static_string (const guchar *string)
{
	GPAQuark q;

	g_return_val_if_fail (string != NULL, 0);

	if (!qdict) qdict = g_hash_table_new (g_str_hash, g_str_equal);

	q = GPOINTER_TO_INT (g_hash_table_lookup (qdict, string));

	if (!q) q = gpa_quark_new (string);

	return q;
}

const guchar *
gpa_quark_to_string (GPAQuark quark)
{
	if ((quark > 0) && (quark <= qseq)) return qarray[quark - 1];

	return NULL;
}

/**
 * gpa_utils_dump_tree_with_level:
 * @node: The node to dump its contents
 * @level: How deep we are in the tree, used so that we know how many spaces to print
 *         so that it really looks like a tree
 * 
 * Recursively prints a node and it childs.
 **/
static void
gpa_utils_dump_tree_with_level (GPANode *node, gint level)
{
	GPANode *ref = NULL;
	GPANode *child;
	int i;

	/* Print this leave indentation */
	for (i = 0; i < level; i++)
		g_print ("   ");

	/* Print the object itself */
	g_print ("%s [%s] (%d)", gpa_node_id (node),
		 G_OBJECT_TYPE_NAME (node), GPOINTER_TO_INT (node));

	if (strcmp (G_OBJECT_TYPE_NAME (node), "GPAReference") == 0) {
		GPANode *tmp = GPA_REFERENCE (node)->ref;
		g_print ("****");
		if (tmp)
			g_print ("     reference to a:%s\n", G_OBJECT_TYPE_NAME (tmp));

		if (level > 1)   /* We follow references only on level 0 & 1 so that we */
			return; /* see Global & Printer objects */
	} else {
		if (strcmp (G_OBJECT_TYPE_NAME (node), "GPAValue") == 0)
			g_print ("{%s}", ((GPAValue *) node)->value);
		g_print ("\n");
	}

	ref = NULL;
	while (TRUE) {
		child = gpa_node_get_child (node, ref);
		if (!child)
			break;
		ref = child;
		gpa_utils_dump_tree_with_level (child, level + 1);
	}
}

/**
 * gpa_utils_dump_tree:
 * @node: 
 * 
 * Dump the tree pointed by @node to the console. Used for debuging purposeses
 **/
void
gpa_utils_dump_tree (GPANode *node)
{
	g_print ("\n"
		 "-------------\n"
		 "Dumping a tree\n\n");

	gpa_utils_dump_tree_with_level (node, 0);
	
	g_print ("-------------\n");
}
