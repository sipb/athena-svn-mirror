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
GPAList    *printers_list  = NULL;
GHashTable *models_dict    = NULL;
GPANode    *gpa_root; /* The parent of global_options */

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
	if (!printers_list) {
		g_warning ("Could not get printers list. gpa_init not called or "
			   "could not load any printers");
		return NULL;
	}

	return gpa_list_ref (printers_list);
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
	if (printers_list || models_dict) {
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
	GPANode *global_options;

	if (gpa_initialized ())
		return TRUE;

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

	gpa_root = gpa_node_new (GPA_TYPE_OPTION, "Globals");
	GPA_OPTION (gpa_root)->type = GPA_OPTION_TYPE_ROOT;
	global_options = gpa_option_new_from_tree (node, gpa_root);
	if (!global_options) {
		g_warning ("Error while reading \"%s\"", file);
		goto init_done;
	}

	printers_list = gpa_printer_list_load ();
	if (!printers_list) {
		g_warning ("Could not load printers list");
		goto init_done;
	}
	
init_done:
	g_free (file);
	my_xmlFreeDoc (doc);

	return printers_list ? TRUE : FALSE;
}
