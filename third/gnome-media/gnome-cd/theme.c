/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Authors: Iain Holmes <iain@ximian.com>
 *
 *  Copyright 2002 Iain Holmes 
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtkbin.h>
#include <gtk/gtkimage.h>

#include <libgnome/gnome-util.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

#include "gnome-cd.h"

GCDTheme *
theme_load (GnomeCD *gcd,
	    const char *theme_name)
{
	char *theme_path, *xml_file, *tmp;
	GCDTheme *theme;
	xmlDocPtr xml;
	xmlNodePtr ptr;
	
	g_return_val_if_fail (gcd != NULL, NULL);
	g_return_val_if_fail (theme_name != NULL, NULL);

	theme_path = g_build_filename (THEME_DIR, theme_name, NULL);
	if (g_file_test (theme_path, G_FILE_TEST_IS_DIR) == FALSE) {
		/* Theme dir isn't a dir */
		
		g_print ("Not a dir %s\n", theme_path);
		g_free (theme_path);
		theme_name = g_strdup ("lcd");
		theme_path = g_build_filename (THEME_DIR, theme_name, NULL);
	}
	
	tmp = g_strconcat (theme_name, ".theme", NULL);
	xml_file = g_build_filename (theme_path, tmp, NULL);
	g_free (tmp);

	if (g_file_test (xml_file,
			 G_FILE_TEST_IS_REGULAR |
			 G_FILE_TEST_IS_SYMLINK) == FALSE) {
		/* No .theme file */

		g_print ("No .theme file: %s\n", xml_file);
		g_free (theme_path);
		g_free (xml_file);
		return NULL;
	}

	xml = xmlParseFile (xml_file);
	g_free (xml_file);
	
	if (xml == NULL) {

		g_print ("No XML\n");
		g_free (theme_path);
		return NULL;
	}

	/* Check doc is right type */
	ptr = xmlDocGetRootElement (xml);
	if (ptr == NULL) {

		g_print ("No root\n");
		g_free (theme_path);
		xmlFreeDoc (xml);
		return NULL;
	}
	if (xmlStrcmp (ptr->name, "gnome-cd")) {
		g_print ("Not gnome-cd: %s\n", ptr->name);
		g_free (theme_path);
		xmlFreeDoc (xml);
		return NULL;
	}
	
	theme = g_new0 (GCDTheme, 1);

	theme->name = g_strdup (theme_name);
	/* Walk the tree filing in the values */
	ptr = ptr->xmlChildrenNode;
	while (ptr && xmlIsBlankNode (ptr)) {
		ptr = ptr->next;
	}

	if (ptr == NULL) {
		g_print ("Empty theme\n");
		return NULL;
	}

	while (ptr != NULL) {
		if (xmlStrcmp (ptr->name, (const xmlChar *) "display") == 0) {
			cd_display_parse_theme (gcd->display, theme, xml, ptr->xmlChildrenNode);
		}
		
		ptr = ptr->next;
	}
	
	return theme;
}

void
theme_free (GCDTheme *theme)
{
	g_return_if_fail (theme != NULL);
	
	g_free (theme->name);
	
	g_free (theme);
}

void
theme_change_widgets (GnomeCD *gcd)
{
}
