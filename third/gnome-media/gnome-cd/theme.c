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

static char *
make_fullname (const char *theme_name,
	       const char *name)
{
	char *image;

	image = g_build_filename (THEME_DIR, theme_name, name, NULL);

	return image;
}

static void
parse_theme (GCDTheme *theme,
	     xmlDocPtr doc,
	     xmlNodePtr cur)
{
	while (cur != NULL) {
		if (xmlStrcmp (cur->name, (const xmlChar *) "image") == 0) {
			xmlChar *button;
			
			button = xmlGetProp (cur, (const xmlChar *) "button");
			if (button != NULL) {
				char *file, *full;
				
				file = xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
				full = make_fullname (theme->name, file);
				
				if (xmlStrcmp (button, "previous") == 0) {
					theme->previous = gdk_pixbuf_new_from_file (full, NULL);
					g_object_ref (G_OBJECT (theme->previous));
				} else if (xmlStrcmp (button, "rewind") == 0) {
					theme->rewind = gdk_pixbuf_new_from_file (full, NULL);
					g_object_ref (G_OBJECT (theme->rewind));
				} else if (xmlStrcmp (button, "play") == 0) {
					theme->play = gdk_pixbuf_new_from_file (full, NULL);
					g_object_ref (G_OBJECT (theme->play));
				} else if (xmlStrcmp (button, "pause") == 0) {
					theme->pause = gdk_pixbuf_new_from_file (full, NULL);
					g_object_ref (G_OBJECT (theme->pause));
				} else if (xmlStrcmp (button, "stop") == 0) {
					theme->stop = gdk_pixbuf_new_from_file (full, NULL);
					g_object_ref (G_OBJECT (theme->stop));
				} else if (xmlStrcmp (button, "forward") == 0) {
					theme->forward = gdk_pixbuf_new_from_file (full, NULL);
					g_object_ref (G_OBJECT (theme->forward));
				} else if (xmlStrcmp (button, "next") == 0) {
					theme->next = gdk_pixbuf_new_from_file (full, NULL);
					g_object_ref (G_OBJECT (theme->next));
				} else if (xmlStrcmp (button, "eject") == 0) {
					theme->eject = gdk_pixbuf_new_from_file (full, NULL);
					g_object_ref (G_OBJECT (theme->eject));
				} else {
					/* Hmmm */
				}
				
				g_free (full);
			} else {
				/* Check for menu */
				char *menu;
				
				menu = xmlGetProp (cur, (const xmlChar *) "menu");
				if (menu != NULL) {
					char *file, *full;

					file = xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
					full = make_fullname (theme->name, file);
					
					if (xmlStrcmp (menu, "previous") == 0) {
						theme->previous_menu = gdk_pixbuf_new_from_file (full, NULL);
					} else if (xmlStrcmp (menu, "stop") == 0) {
						theme->stop_menu = gdk_pixbuf_new_from_file (full, NULL);
					} else if (xmlStrcmp (menu, "play") == 0) {
						theme->play_menu = gdk_pixbuf_new_from_file (full, NULL);
					} else if (xmlStrcmp (menu, "next") == 0) {
						theme->play_menu = gdk_pixbuf_new_from_file (full, NULL);
					} else if (xmlStrcmp (menu, "eject") == 0) {
						theme->eject_menu = gdk_pixbuf_new_from_file (full, NULL);
					} else {
						/* Hmm */
					}

					g_free (full);
				}
			}
		}

		cur = cur->next;
	}
}

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
		} else if (xmlStrcmp (ptr->name, (const xmlChar *) "icons") == 0) {
			parse_theme (theme, xml, ptr->xmlChildrenNode);
		} else {
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
	g_object_unref (G_OBJECT (theme->previous));
/* 	g_object_unref (G_OBJECT (theme->previous_menu)); */
	g_object_unref (G_OBJECT (theme->rewind));
	g_object_unref (G_OBJECT (theme->play));
/* 	g_object_unref (G_OBJECT (theme->play_menu)); */
	g_object_unref (G_OBJECT (theme->pause));
	g_object_unref (G_OBJECT (theme->stop));
/* 	g_object_unref (G_OBJECT (theme->stop_menu)); */
	g_object_unref (G_OBJECT (theme->forward));
	g_object_unref (G_OBJECT (theme->next));
/* 	g_object_unref (G_OBJECT (theme->next_menu)); */
	g_object_unref (G_OBJECT (theme->eject));
/* 	g_object_unref (G_OBJECT (theme->eject_menu)); */
	
	g_free (theme);
}

void
theme_change_widgets (GnomeCD *gcd)
{
	gtk_image_set_from_pixbuf (GTK_IMAGE (GTK_BIN (gcd->back_b)->child),
				   gcd->theme->previous);
	gtk_image_set_from_pixbuf (GTK_IMAGE (GTK_BIN (gcd->rewind_b)->child),
				   gcd->theme->rewind);
	
	gtk_image_set_from_pixbuf (GTK_IMAGE (gcd->play_image),
				   gcd->theme->play);
	gtk_image_set_from_pixbuf (GTK_IMAGE (gcd->pause_image),
				   gcd->theme->pause);
	
	gtk_image_set_from_pixbuf (GTK_IMAGE (GTK_BIN (gcd->stop_b)->child),
				   gcd->theme->stop);
	gtk_image_set_from_pixbuf (GTK_IMAGE (GTK_BIN (gcd->ffwd_b)->child),
				   gcd->theme->forward);
	gtk_image_set_from_pixbuf (GTK_IMAGE (GTK_BIN (gcd->next_b)->child),
				   gcd->theme->next);
	gtk_image_set_from_pixbuf (GTK_IMAGE (GTK_BIN (gcd->eject_b)->child),
				   gcd->theme->eject);
}
