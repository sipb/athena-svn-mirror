/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-root.c:
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

#include <config.h>

#include <string.h>

#include "gpa-utils.h"
#include "gpa-printer.h"
#include "gpa-option.h"
#include "gpa-root.h"

/* Global variables */
GPARoot * gpa_root = NULL;
static gboolean initializing = FALSE;

static GPANodeClass *parent_class;

typedef struct _GPARootClass GPARootClass;
struct _GPARootClass {
	GPANodeClass node_class;
};

static void gpa_root_class_init (GPARootClass *klass);
static void gpa_root_init (GPARoot *root);
static void gpa_root_finalize (GObject *object);

GType
gpa_root_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPARootClass),
			NULL, NULL,
			(GClassInitFunc) gpa_root_class_init,
			NULL, NULL,
			sizeof (GPARoot),
			0,
			(GInstanceInitFunc) gpa_root_init
		};
		type = g_type_register_static (GPA_TYPE_NODE, "GPARoot", &info, 0);
	}
	return type;
}

static void
gpa_root_class_init (GPARootClass *klass)
{
	GObjectClass *object_class;
	GPANodeClass *node_class;

	object_class = (GObjectClass*) klass;
	node_class = (GPANodeClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gpa_root_finalize;
}


static void
gpa_root_init (GPARoot *root)
{
	/* Empty */
}

static void
gpa_root_finalize (GObject *object)
{
	GPARoot *root;

	root = (GPARoot *) object;

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/**
 * gpa_get_printers:
 * 
 * Get the list of loaded printers
 * 
 * Return Value: a refcounted GPANode pointer to the printers GPAList node
 **/
GPAList *
gpa_get_printers (void)
{
	if (!gpa_root) {
		g_warning ("gpa_init not called, gpa_get_printers failed");
		return NULL;
	}
	
	if (!gpa_root->printers) {
		g_warning ("Could not get printers list, gpa_root->printers is empty");
		return NULL;
	}

	return gpa_list_ref (gpa_root->printers);
}

void
gpa_shutdown (void)
{
	/* Implement. free stuff */
}

/**
 * gpa_initialized:
 * @void: 
 * 
 * Check if gpa has been initalized 
 * 
 * Return Value: TRUE if gpa_init has been called and was successfull
 **/
gboolean
gpa_initialized (void)
{
	if (initializing) {
		/* Not yet initialized but in the process of */
		return TRUE;
	}

	if (gpa_root) {
		return TRUE;
	}

	return FALSE;
}

gboolean
gpa_init (void)
{
	xmlNodePtr node;
	xmlDocPtr doc;
	gchar *file = NULL;
	GPANode *globals = NULL;
	GPAList *printers_list = NULL;

	if (gpa_initialized ())
		return TRUE;

	initializing = TRUE;

	file = g_build_filename (GPA_DATA_DIR, "globals.xml", NULL);
	doc = xmlParseFile (file);
	if (!doc) {
		g_warning ("Could not parse %s or file not found, please check your "
			   "libgnomeprint installation", file);
		goto init_done;
	}
	node = gpa_xml_node_get_child (doc->children, "Option");
	if (!node) {
		g_warning ("node \"Option\" not found in \"%s\", check your libgnomeprint "
			   "installation", file);
		goto init_done;
	}

	gpa_root = (GPARoot*) gpa_node_new (GPA_TYPE_ROOT, "GpaRootNode");
	globals = gpa_option_new_from_tree (node, GPA_NODE (gpa_root));
	if (!globals) {
		g_warning ("Error while reading \"%s\"", file);
		goto init_done;
	}
	/* gpa_option_new_from_tree does gpa_node_attach for us */

	printers_list = gpa_printer_list_load ();
	if (!printers_list) {
		g_warning ("Could not load printers list");
		goto init_done;
	}
	gpa_root->printers = gpa_node_attach (GPA_NODE (gpa_root), GPA_NODE (printers_list));
	
init_done:
	initializing = FALSE;
	g_free (file);
	my_xmlFreeDoc (doc);

	if ((!globals || !printers_list) && gpa_root) {
		gpa_node_unref (GPA_NODE (gpa_root));
		gpa_root = NULL;
		return FALSE;
	}
	
	return TRUE;
}
