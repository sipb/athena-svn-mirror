/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-vendor.c:
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

#define __GPA_VENDOR_C__
#define noGPA_VENDOR_DEBUG

#include <string.h>
#include <sys/types.h>
#include <dirent.h> /* For the DIR structure stuff */
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "gpa-utils.h"
#include "gpa-value.h"
#include "gpa-model.h"
#include "gpa-vendor.h"

/* GPAVendor */

/* fixme: Name should be a node */

static void gpa_vendor_class_init (GPAVendorClass *klass);
static void gpa_vendor_init (GPAVendor *vendor);

static void gpa_vendor_finalize (GObject *object);

static gboolean gpa_vendor_verify (GPANode *node);
static guchar *gpa_vendor_get_value (GPANode *node);
static GPANode *gpa_vendor_get_child (GPANode *node, GPANode *ref);
static GPANode *gpa_vendor_lookup (GPANode *node, const guchar *path);
static void gpa_vendor_modified (GPANode *node, guint flags);

static GPANode *gpa_vendor_new_from_file (const gchar *filename);

static GPANode *gpa_vendor_new_from_tree (xmlNodePtr tree);

static GPANodeClass *parent_class = NULL;

GType
gpa_vendor_get_type (void) {
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPAVendorClass),
			NULL, NULL,
			(GClassInitFunc) gpa_vendor_class_init,
			NULL, NULL,
			sizeof (GPAVendor),
			0,
			(GInstanceInitFunc) gpa_vendor_init
		};
		type = g_type_register_static (GPA_TYPE_NODE, "GPAVendor", &info, 0);
	}
	return type;
}

static void
gpa_vendor_class_init (GPAVendorClass *klass)
{
	GObjectClass *object_class;
	GPANodeClass *node_class;

	object_class = (GObjectClass *) klass;
	node_class = (GPANodeClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gpa_vendor_finalize;

	node_class->verify = gpa_vendor_verify;
	node_class->get_value = gpa_vendor_get_value;
	node_class->get_child = gpa_vendor_get_child;
	node_class->lookup = gpa_vendor_lookup;
	node_class->modified = gpa_vendor_modified;
}

static void
gpa_vendor_init (GPAVendor *vendor)
{
	vendor->name = NULL;
	vendor->url = NULL;
	vendor->models = NULL;
}

static void
gpa_vendor_finalize (GObject *object)
{
	GPAVendor *vendor;

	vendor = GPA_VENDOR (object);

	vendor->name = gpa_node_detach_unref (GPA_NODE (vendor), vendor->name);
	vendor->models = (GPAList *) gpa_node_detach_unref (GPA_NODE (vendor), GPA_NODE (vendor->models));
	if (vendor->url)
		vendor->url = gpa_node_detach_unref (GPA_NODE (vendor), vendor->url);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
gpa_vendor_verify (GPANode *node)
{
	GPAVendor *vendor;

	vendor = GPA_VENDOR (node);

	if (!GPA_NODE_ID_EXISTS (node))
		return FALSE;

	if (!vendor->name)
		return FALSE;
	if (!gpa_node_verify (vendor->name))
		return FALSE;
	if (!vendor->url)
		return FALSE;
	if (!gpa_node_verify (vendor->url))
		return FALSE;
	if (!vendor->models)
		return FALSE;
	if (!gpa_node_verify (GPA_NODE (vendor->models)))
		return FALSE;

	return TRUE;
}

static guchar *
gpa_vendor_get_value (GPANode *node)
{
	GPAVendor *vendor;

	vendor = GPA_VENDOR (node);

	if (GPA_NODE_ID_EXISTS (node))
		return g_strdup (GPA_NODE_ID (node));

	return NULL;
}

static GPANode *
gpa_vendor_get_child (GPANode *node, GPANode *ref)
{
	GPAVendor *vendor;
	GPANode *child;
	
	vendor = GPA_VENDOR (node);

	child = NULL;
	if (ref == NULL) {
		child = vendor->name;
	} else if (ref == vendor->name) {
		child = vendor->url;
	} else if (ref == vendor->url) {
		child = (GPANode *) vendor->models;
	}

	if (child)
		gpa_node_ref (child);

	return child;
}

static GPANode *
gpa_vendor_lookup (GPANode *node, const guchar *path)
{
	GPAVendor *vendor;
	GPANode *child;

	vendor = GPA_VENDOR (node);

	child = NULL;

	if (gpa_node_lookup_ref (&child, GPA_NODE (vendor->name), path, "Name"))
		return child;
	if (gpa_node_lookup_ref (&child, GPA_NODE (vendor->models), path, "Models"))
		return child;
	if (vendor->url && gpa_node_lookup_ref (&child, GPA_NODE (vendor->url), path, "URL"))
		return child;

	return NULL;
}

static void
gpa_vendor_modified (GPANode *node, guint flags)
{
	GPAVendor *vendor;

	vendor = GPA_VENDOR (node);

	if (vendor->name && (GPA_NODE_FLAGS (vendor->name) & GPA_MODIFIED_FLAG)) {
		gpa_node_emit_modified (vendor->name, 0);
	}
	if (vendor->url && (GPA_NODE_FLAGS (vendor->url) & GPA_MODIFIED_FLAG)) {
		gpa_node_emit_modified (vendor->url, 0);
	}
	if (vendor->models && (GPA_NODE_FLAGS (vendor->models) & GPA_MODIFIED_FLAG)) {
		gpa_node_emit_modified (GPA_NODE (vendor->models), 0);
	}
}

static GPANode *
gpa_vendor_new_from_tree (xmlNodePtr tree)
{
	GPAVendor *vendor;
	xmlChar *xmlid;
	xmlNodePtr xmlc;
	GPANode *name, *url;
	GPAList *models;

	g_return_val_if_fail (tree != NULL, NULL);

	xmlid = xmlGetProp (tree, "Id");
	g_return_val_if_fail (xmlid != NULL, NULL);

	vendor = NULL;
	name = NULL;
	url = NULL;
	models = NULL;

	for (xmlc = tree->xmlChildrenNode; xmlc != NULL; xmlc = xmlc->next) {
		if (!strcmp (xmlc->name, "Name")) {
			name = gpa_value_new_from_tree ("Name", xmlc);
		} else if (!strcmp (xmlc->name, "URL")) {
			url = gpa_value_new_from_tree ("URL", xmlc);
		} else if (!strcmp (xmlc->name, "Models")) {
			models = gpa_model_list_new_from_info_tree (xmlc);
		}
	}

	if (name && url && models) {
		vendor = (GPAVendor *) gpa_node_new (GPA_TYPE_VENDOR, xmlid);
		vendor->name = gpa_node_attach (GPA_NODE (vendor), name);
		vendor->url = gpa_node_attach (GPA_NODE (vendor), url);
		vendor->models = (GPAList *) gpa_node_attach (GPA_NODE (vendor), GPA_NODE (models));
	} else {
		if (name)
			gpa_node_unref (name);
		if (url)
			gpa_node_unref (url);
		if (models)
			gpa_node_unref (GPA_NODE (models));
	}

	xmlFree (xmlid);

	return (GPANode *) vendor;
}

static GPANode *
gpa_vendor_new_from_file (const gchar *filename)
{
	GPANode *vendor;
	xmlDocPtr doc;
	xmlNodePtr root;

	doc = xmlParseFile (filename);
	if (!doc)
		return NULL;
	root = doc->xmlRootNode;
	vendor = NULL;
	if (!strcmp (root->name, "Vendor")) {
		vendor = gpa_vendor_new_from_tree (root);
	}
	xmlFreeDoc (doc);
	return vendor;
}

/* GPAVendorList */

static void gpa_vendor_list_load_from_dir (GPAList *vendors, const gchar *dirname);

static GPAList *vendors = NULL;

static void
gpa_vendors_gone (gpointer data, GObject *gone)
{
#ifdef GPA_VENDOR_DEBUG
	g_print ("GPAVendor: Vendor list %p has gone\n", gone);
#endif

	vendors = NULL;
}

GPAList *
gpa_vendor_list_load (void)
{
	gchar *dirname;

	if (vendors)
		return (GPAList *) gpa_node_ref (GPA_NODE (vendors));

	vendors = GPA_LIST (gpa_list_new (GPA_TYPE_VENDOR, FALSE));
	gpa_node_construct (GPA_NODE (vendors), "Vendors");
	g_object_weak_ref (G_OBJECT (vendors), gpa_vendors_gone, &vendors);

	dirname = g_strdup_printf ("%s/%s", g_get_home_dir (), ".gnome/gnome-print-2.0/vendors");
	gpa_vendor_list_load_from_dir (vendors, dirname);
	g_free (dirname);
	/* fixme: */
	gpa_vendor_list_load_from_dir (vendors, DATADIR "/gnome-print-2.0/vendors");

	return vendors;
}

static void
gpa_vendor_list_load_from_dir (GPAList *vendors, const gchar *dirname)
{
	DIR *dir;
	struct dirent *dent;
	GSList *l;

	dir = opendir (dirname);
	if (!dir) return;

	l = NULL;
	while ((dent = readdir (dir))) {
		gint len;
		gchar *filename;
		GPANode *vendor;
		len = strlen (dent->d_name);
		if (len < 8) continue;
		if (strcmp (dent->d_name + len - 7, ".vendor")) continue;
		filename = g_strdup_printf ("%s/%s", dirname, dent->d_name);
		vendor = gpa_vendor_new_from_file (filename);
		/* fixme: test name clashes */
		if (vendor)
			l = g_slist_prepend (l, vendor);
		else
			g_warning ("Could not create vendor from file:%s\n",
				   filename);
		g_free (filename);

	}

	closedir (dir);

	while (l) {
		/* fixme: ordering */
		GPANode *vendor;
		vendor = GPA_NODE (l->data);
		l = g_slist_remove (l, vendor);
		vendor->next = vendors->children;
		vendors->children = vendor;
		vendor->parent = GPA_NODE (vendors);
	}
}

/* Public methods */

GPANode *
gpa_vendor_get_by_id (const guchar *id)
{
	GPAList *vendors;
	GPANode *child;

	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (*id != '\0', NULL);

	vendors = gpa_vendor_list_load ();

	for (child = vendors->children; child != NULL; child = child->next) {
		g_assert (GPA_IS_VENDOR (child));
		if (GPA_NODE_ID_COMPARE (child, id))
			break;
	}

	if (child)
		gpa_node_ref (child);
	else
		g_print ("Could not get vendor by id: %s\n", id);

	gpa_node_unref (gpa_node_cache (GPA_NODE (vendors)));

	return child;
}


