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
 *  Copyright (C) 2000-2003 Ximian, Inc.
 *
 */

#include <config.h>

#include <string.h>
#include <libxml/parser.h>

#include "gpa-node-private.h"
#include "gpa-utils.h"
#include "gpa-reference.h"
#include "gpa-list.h"
#include "gpa-key.h"
#include "gpa-state.h"
#include "gpa-option.h"

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

	for (child = node->xmlChildrenNode; child != NULL; child = child->next)
		if (!strcmp (child->name, name))
			return child;

	return NULL;
}

/**
 * gpa_utils_dump_tree_with_level:
 * @node: The node to dump its contents
 * @level: How deep we are in the tree, used so that we know how many spaces to print
 *         so that it really looks like a tree
 * @follow_references: How deep in the tree do we dump info for the refrences
 * 
 * Recursively prints a node and it childs.
 **/
static void
gpa_utils_dump_tree_with_level (GPANode *node, gint level, gint follow_references)
{
	GPANode *previous_child = NULL;
	GPANode *child;
	int i;
	gboolean address = FALSE;

	if (level > 20) {
		g_error ("Level too deep. Aborting\n");
	}
	
	g_print ("[%2d]", level);
	/* Print this leave indentation */
	for (i = 0; i < level; i++) {
		g_print ("   ");
	}

	/* Print the object itself */
	g_print ("%s [%s] (%d)", gpa_node_id (node),
		 G_OBJECT_TYPE_NAME (node), (address ? GPOINTER_TO_INT (node) : 0));

	if (strcmp (G_OBJECT_TYPE_NAME (node), "GPAReference") == 0) {
		GPANode *tmp = GPA_REFERENCE (node)->ref;
		g_print ("****");
		if (tmp)
			g_print ("     reference to a:%s\n", G_OBJECT_TYPE_NAME (tmp));
		else
			g_print ("     empty reference\n");

		if (level <= follow_references) {
			gpa_utils_dump_tree_with_level (GPA_REFERENCE (node)->ref,
							level + 1, follow_references);
		}
		return;
	}

	if (strcmp (G_OBJECT_TYPE_NAME (node), "GPAKey") == 0)
		g_print (" {%s}", ((GPAKey *) node)->value);
	if (strcmp (G_OBJECT_TYPE_NAME (node), "GPAState") == 0)
		g_print (" state: [%s]", ((GPAState *) node)->value);
	if (strcmp (G_OBJECT_TYPE_NAME (node), "GPAOption") == 0) {
		GPAOption *option;
		option = GPA_OPTION (node);
		g_print (" {OptionType ");
		switch (option->type) {
		case GPA_OPTION_TYPE_STRING:
			g_print ("string [%s]", option->value);
			break;
		case GPA_OPTION_TYPE_LIST:
			g_print ("list [%s]", option->value);
			break;
		case GPA_OPTION_TYPE_KEY:
			g_print ("key [%s]", option->value);
			break;
		case GPA_OPTION_TYPE_NODE:
			g_print ("node");
			break;
		case GPA_OPTION_TYPE_ITEM:
			g_print ("item [%s]", option->value);
			break;
		case GPA_OPTION_TYPE_ROOT:
			g_print ("root");
			break;
		case GPA_OPTION_TYPE_NONE:
			default:
				g_assert_not_reached ();
		}
		g_print ("}");
	}
	if (strcmp (G_OBJECT_TYPE_NAME (node), "GPAList") == 0) {
		g_print (" {CanHaveDefault:%s}", GPA_LIST (node)->can_have_default ?
			 "Yes" : "No");
	}
	g_print ("\n");

	previous_child = NULL;
	while (TRUE) {
		child = gpa_node_get_child (node, previous_child);
		if (child == node) {
			g_error ("Error: child is the same as parent. Aborting.\n");
		}
		if (!child)
			break;
		previous_child = child;
		gpa_utils_dump_tree_with_level (child, level + 1, follow_references);
		gpa_node_unref (GPA_NODE (child));
	}
}

/**
 * gpa_utils_dump_tree:
 * @node:
 * @follow_references: How deep in the tree do we follow references
 * 
 * Dump the tree pointed by @node to the console. Used for debuging purposeses
 **/
void
gpa_utils_dump_tree (GPANode *node, gint follow_references)
{
	g_return_if_fail (node != NULL);
	g_return_if_fail (GPA_IS_NODE (node));
	
	g_print ("\n"
		 "-------------\n"
		 "Dumping a tree\n\n");

	gpa_utils_dump_tree_with_level (node, 0, follow_references);

	g_print ("-------------\n");
}

