/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-media.c: 
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

#define __GPA_MEDIA_C__
#define noGPA_MEDIA_DEBUG

#include <config.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "../gnome-print-i18n.h"
#include "gpa-utils.h"
#include "gpa-option.h"
#include "gpa-media.h"

static void gpa_media_read_layouts_from_tree (GPANode *layouts, xmlNodePtr node);

static GPANode *gpa_media_init_physicalsizes (GPANode *media);
static GPANode *gpa_media_init_physicalorientations (GPANode *media);
static GPANode *gpa_media_init_logicalorientations (GPANode *media);
static GPANode *gpa_media_init_layouts (GPANode *media);

static GPANode *media = NULL;
static time_t timestamp;
static time_t lastcheck;

static void
gpa_media_gone (gpointer data, GObject *gone)
{
#ifdef GPA_MEDIA_DEBUG
	g_print ("GPAMedia: Media %p is gone\n", gone);
#endif

	media = NULL;
}

GPANode *
gpa_media_load (void)
{
	GPANode *physicalsizes, *physicalorientations, *logicalorientations, *layouts;
	xmlDocPtr doc;
	xmlNodePtr root;
	struct stat s;

	if (media) {
		if (time (NULL) == lastcheck) {
			return gpa_node_ref (media);
		} else {
			lastcheck = time (NULL);
			if (!stat (DATADIR "/gnome-print-2.0/media/media.xml", &s)) {
				if (s.st_mtime == timestamp) {
#ifdef GPA_MEDIA_DEBUG
					g_print ("GPAMedia: Found valid media %p\n", media);
#endif
					return gpa_node_ref (media);
				}
			}
			g_object_weak_unref (G_OBJECT (media), gpa_media_gone, &media);
			media = NULL;
		}
	}

	/* Initialize Media */
	media = gpa_option_node_new ("Media");
	g_object_weak_ref (G_OBJECT (media), gpa_media_gone, &media);
	/* Bookeeping */
	lastcheck = time (NULL);
	if (!stat (DATADIR "/gnome-print-2.0/media/media.xml", &s)) {
		timestamp = s.st_mtime;
	}
#ifdef GPA_MEDIA_DEBUG
	g_print ("GPAMedia: Created new media %p\n", media);
#endif
	/* Initialize PhysicalSizes */
	physicalsizes = gpa_media_init_physicalsizes (media);
	/* Initialize PhysicalOrientations */
	physicalorientations = gpa_media_init_physicalorientations (media);
	/* Initialize LogicalOrientations */
	logicalorientations = gpa_media_init_logicalorientations (media);
	/* Initialize Layouts */
	layouts = gpa_media_init_layouts (media);

	/* Parse data */
	doc = xmlParseFile (DATADIR "/gnome-print-2.0/media/media.xml");
	g_return_val_if_fail (doc != NULL, NULL);
	root = doc->xmlRootNode;
	if (!strcmp (root->name, "Media")) {
		xmlNodePtr child;
		for (child = root->xmlChildrenNode; child != NULL; child = child->next) {
			if (!strcmp (child->name, "PhysicalSizes")) {
				xmlNodePtr size;
				/* Physical sizes */
				for (size = child->xmlChildrenNode; size != NULL; size = size->next) {
					if (!strcmp (size->name, "PhysicalSize")) {
						xmlChar *id, *width, *height, *name;
						id = xmlGetProp (size, "Id");
						width = xmlGetProp (size, "Width");
						height = xmlGetProp (size, "Height");
						name = gpa_xml_node_get_name (size);
						if (id && *id && width && *width && height && *height && name && *name) {
							GPANode *item, *key;
							item = gpa_option_item_new (id, name);
							key = gpa_option_key_new ("Width", width);
							gpa_option_item_append_child (GPA_OPTION_ITEM (item), GPA_OPTION (key));
							gpa_node_unref (key);
							key = gpa_option_key_new ("Height", height);
							gpa_option_item_append_child (GPA_OPTION_ITEM (item), GPA_OPTION (key));
							gpa_node_unref (key);
							gpa_option_list_append_child (GPA_OPTION_LIST (physicalsizes), GPA_OPTION (item));
							gpa_node_unref (item);
						}
						if (id)
							xmlFree (id);
						if (width)
							xmlFree (width);
						if (height)
							xmlFree (height);
						if (name)
							xmlFree (name);
					}
				}
			} else if (!strcmp (child->name, "PhysicalOrientations")) {
				/* Physical orientations (NOP) */
			} else if (!strcmp (child->name, "LogicalOrientations")) {
				/* Logical orientations (NOP) */
			} else if (!strcmp (child->name, "Layouts")) {
				gpa_media_read_layouts_from_tree (layouts, child);
			}
		}
	}

	xmlFreeDoc (doc);

	return media;
}

static void
gpa_media_read_layouts_from_tree (GPANode *layouts, xmlNodePtr node)
{
	GPANode *item;
	xmlNodePtr layout;

	item = NULL;

	for (layout = node->xmlChildrenNode; layout != NULL; layout = layout->next) {
		if (!strcmp (layout->name, "Layout")) {
			xmlChar *id, *vps, *lp, *pp, *w, *h, *name;
			id = xmlGetProp (layout, "Id");
			vps = xmlGetProp (layout, "ValidPhysicalSizes");
			lp = xmlGetProp (layout, "LogicalPages");
			pp = xmlGetProp (layout, "PhysicalPages");
			w = xmlGetProp (layout, "Width");
			h = xmlGetProp (layout, "Height");
			name = gpa_xml_node_get_name (layout);
			if (id && *id && vps && *vps && lp && *lp && pp && *pp && w && *w && h && *h && name && *name) {
				GPANode *item, *key, *pages;
				xmlNodePtr p;
				gint counter;

				item = gpa_option_item_new (id, name);

				key = gpa_option_key_new ("ValidPhysicalSizes", vps);
				gpa_option_item_append_child (GPA_OPTION_ITEM (item), GPA_OPTION (key));
				gpa_node_unref (key);
				key = gpa_option_key_new ("LogicalPages", lp);
				gpa_option_item_append_child (GPA_OPTION_ITEM (item), GPA_OPTION (key));
				gpa_node_unref (key);
				key = gpa_option_key_new ("PhysicalPages", pp);
				gpa_option_item_append_child (GPA_OPTION_ITEM (item), GPA_OPTION (key));
				gpa_node_unref (key);
				key = gpa_option_key_new ("Width", w);
				gpa_option_item_append_child (GPA_OPTION_ITEM (item), GPA_OPTION (key));
				gpa_node_unref (key);
				key = gpa_option_key_new ("Height", h);
				gpa_option_item_append_child (GPA_OPTION_ITEM (item), GPA_OPTION (key));
				gpa_node_unref (key);

				pages = gpa_option_key_new ("Pages", NULL);
				gpa_option_item_append_child (GPA_OPTION_ITEM (item), GPA_OPTION (pages));
				gpa_node_unref (pages);

				counter = 0;
				for (p = layout->xmlChildrenNode; p != NULL; p = p->next) {
					if (!strcmp (p->name, "Page")) {
						xmlChar *val;
						val = xmlGetProp (p, "transform");
						if (val && *val) {
							guchar c[32];
							g_snprintf (c, 32, "LP%d", counter++);
							key = gpa_option_key_new (c, val);
							gpa_option_key_append_child (GPA_OPTION_KEY (pages), GPA_OPTION (key));
							gpa_node_unref (key);
						}
						if (val) xmlFree (val);
					}
				}

				gpa_option_list_append_child (GPA_OPTION_LIST (layouts), GPA_OPTION (item));
				gpa_node_unref (item);
			}
			if (id) xmlFree (id);
			if (vps) xmlFree (vps);
			if (lp) xmlFree (lp);
			if (pp) xmlFree (pp);
			if (w) xmlFree (w);
			if (h) xmlFree (h);
			if (name) xmlFree (name);
		}
	}
}

static GPANode *
gpa_media_init_physicalsizes (GPANode *media)
{
	GPANode *physicalsizes, *custom, *key;

	physicalsizes = gpa_option_list_new ("PhysicalSizes");
	gpa_option_node_append_child (GPA_OPTION_NODE (media), GPA_OPTION (physicalsizes));
	gpa_node_unref (physicalsizes);
	custom = gpa_option_item_new ("Custom", _("Custom"));
	key = gpa_option_string_new ("Width", "210mm");
	gpa_option_item_append_child (GPA_OPTION_ITEM (custom), GPA_OPTION (key));
	gpa_node_unref (key);
	key = gpa_option_string_new ("Height", "297mm");
	gpa_option_item_append_child (GPA_OPTION_ITEM (custom), GPA_OPTION (key));
	gpa_node_unref (key);
	gpa_option_list_append_child (GPA_OPTION_LIST (physicalsizes), GPA_OPTION (custom));
	gpa_node_unref (custom);

	return physicalsizes;
}

static GPANode *
gpa_media_init_physicalorientations (GPANode *media)
{
	GPANode *node, *child, *key;

	node = gpa_option_list_new ("PhysicalOrientations");
	gpa_option_node_append_child (GPA_OPTION_NODE (media), GPA_OPTION (node));
	gpa_node_unref (node);

	child = gpa_option_item_new ("R0", _("Straight"));
	key = gpa_option_key_new ("Paper2PrinterTransform", "matrix(1 0 0 1 0 0)");
	gpa_option_item_append_child (GPA_OPTION_ITEM (child), GPA_OPTION (key));
	gpa_node_unref (key);
	gpa_option_list_append_child (GPA_OPTION_LIST (node), GPA_OPTION (child));
	gpa_node_unref (child);

	child = gpa_option_item_new ("R90", _("Rotated 90 degrees"));
	key = gpa_option_key_new ("Paper2PrinterTransform", "matrix(0 -1 1 0 0 1)");
	gpa_option_item_append_child (GPA_OPTION_ITEM (child), GPA_OPTION (key));
	gpa_node_unref (key);
	gpa_option_list_append_child (GPA_OPTION_LIST (node), GPA_OPTION (child));
	gpa_node_unref (child);

	child = gpa_option_item_new ("R180", _("Rotated 180 degrees"));
	key = gpa_option_key_new ("Paper2PrinterTransform", "matrix(-1 0 0 -1 1 1)");
	gpa_option_item_append_child (GPA_OPTION_ITEM (child), GPA_OPTION (key));
	gpa_node_unref (key);
	gpa_option_list_append_child (GPA_OPTION_LIST (node), GPA_OPTION (child));
	gpa_node_unref (child);

	child = gpa_option_item_new ("R270", _("Rotated 270 degrees"));
	key = gpa_option_key_new ("Paper2PrinterTransform", "matrix(0 1 -1 0 0 1 0)");
	gpa_option_item_append_child (GPA_OPTION_ITEM (child), GPA_OPTION (key));
	gpa_node_unref (key);
	gpa_option_list_append_child (GPA_OPTION_LIST (node), GPA_OPTION (child));
	gpa_node_unref (child);

	return node;
}

static GPANode *
gpa_media_init_logicalorientations (GPANode *media)
{
	GPANode *node, *child, *key;

	node = gpa_option_list_new ("LogicalOrientations");
	gpa_option_node_append_child (GPA_OPTION_NODE (media), GPA_OPTION (node));
	gpa_node_unref (node);

	child = gpa_option_item_new ("R0", _("Portrait"));
	key = gpa_option_key_new ("Page2LayoutTransform", "matrix(1 0 0 1 0 0)");
	gpa_option_item_append_child (GPA_OPTION_ITEM (child), GPA_OPTION (key));
	gpa_node_unref (key);
	gpa_option_list_append_child (GPA_OPTION_LIST (node), GPA_OPTION (child));
	gpa_node_unref (child);

	child = gpa_option_item_new ("R90", _("Landscape"));
	key = gpa_option_key_new ("Page2LayoutTransform", "matrix(0 1 -1 0 0 1)");
	gpa_option_item_append_child (GPA_OPTION_ITEM (child), GPA_OPTION (key));
	gpa_node_unref (key);
	gpa_option_list_append_child (GPA_OPTION_LIST (node), GPA_OPTION (child));
	gpa_node_unref (child);

	child = gpa_option_item_new ("R180", _("Upside down portrait"));
	key = gpa_option_key_new ("Page2LayoutTransform", "matrix(-1 0 0 -1 1 1)");
	gpa_option_item_append_child (GPA_OPTION_ITEM (child), GPA_OPTION (key));
	gpa_node_unref (key);
	gpa_option_list_append_child (GPA_OPTION_LIST (node), GPA_OPTION (child));
	gpa_node_unref (child);

	child = gpa_option_item_new ("R270", _("Upside down landscape"));
	key = gpa_option_key_new ("Page2LayoutTransform", "matrix(0 -1 1 0 1 0)");
	gpa_option_item_append_child (GPA_OPTION_ITEM (child), GPA_OPTION (key));
	gpa_node_unref (key);
	gpa_option_list_append_child (GPA_OPTION_LIST (node), GPA_OPTION (child));
	gpa_node_unref (child);

	return node;
}

static GPANode *
gpa_media_init_layouts (GPANode *media)
{
	GPANode *layouts, *plain, *key, *pages, *page;

	layouts = gpa_option_list_new ("Layouts");
	gpa_option_node_append_child (GPA_OPTION_NODE (media), GPA_OPTION (layouts));
	gpa_node_unref (layouts);

	plain = gpa_option_item_new ("Plain", _("Plain"));

	key = gpa_option_key_new ("ValidPhysicalSizes", "All");
	gpa_option_item_append_child (GPA_OPTION_ITEM (plain), GPA_OPTION (key));
	gpa_node_unref (key);
	key = gpa_option_key_new ("LogicalPages", "1");
	gpa_option_item_append_child (GPA_OPTION_ITEM (plain), GPA_OPTION (key));
	gpa_node_unref (key);
	key = gpa_option_key_new ("PhysicalPages", "1");
	gpa_option_item_append_child (GPA_OPTION_ITEM (plain), GPA_OPTION (key));
	gpa_node_unref (key);
	key = gpa_option_key_new ("Width", "1");
	gpa_option_item_append_child (GPA_OPTION_ITEM (plain), GPA_OPTION (key));
	gpa_node_unref (key);
	key = gpa_option_key_new ("Height", "1");
	gpa_option_item_append_child (GPA_OPTION_ITEM (plain), GPA_OPTION (key));
	gpa_node_unref (key);

	pages = gpa_option_key_new ("Pages", NULL);
	gpa_option_item_append_child (GPA_OPTION_NODE (plain), GPA_OPTION (pages));
	gpa_node_unref (pages);

	page = gpa_option_key_new ("LP0", "transform(1 0 0 1 0 0)");
	gpa_option_key_append_child (GPA_OPTION_KEY (pages), GPA_OPTION (page));
	gpa_node_unref (page);

	gpa_option_list_append_child (GPA_OPTION_LIST (layouts), GPA_OPTION (plain));
	gpa_node_unref (plain);

	return layouts;
}

