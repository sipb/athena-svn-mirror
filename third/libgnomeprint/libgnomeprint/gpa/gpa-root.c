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
 *  Copyright (C) 2000-2001 Ximian, Inc. and Jose M. Celorio
 *
 */

#define __GPA_ROOT_C__

#include <string.h>
#include "gpa-utils.h"
#include "gpa-vendor.h"
#include "gpa-printer.h"
#include "gpa-media.h"
#include "gpa-root.h"

#define noGPA_ROOT_DEBUG

/* GPARoot */

static void gpa_root_class_init (GPARootClass *klass);
static void gpa_root_init (GPARoot *root);

static void gpa_root_finalize (GObject *object);

static GPANode *gpa_root_get_child (GPANode *node, GPANode *ref);
static GPANode *gpa_root_lookup (GPANode *node, const guchar *path);
static void gpa_root_modified (GPANode *node, guint flags);

static GPANode *root_instance = NULL;

/* Helpers */

static GPANodeClass *parent_class;

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

	node_class->get_child = gpa_root_get_child;
	node_class->lookup = gpa_root_lookup;

	node_class->modified = gpa_root_modified;
}

static void
gpa_root_init (GPARoot *root)
{
	root_instance = GPA_NODE (root);

	root->media = NULL;
	root->vendors = NULL;
	root->printers = NULL;

#if 0
	root->vendors = gpa_node_attach (GPA_NODE (root), GPA_NODE (gpa_vendor_list_load ()));
#endif
#if 0
	root->printers = gpa_node_attach (GPA_NODE (root), GPA_NODE (gpa_printer_list_load ()));
#endif
}

static void
gpa_root_vendors_gone (gpointer data, GObject *gone)
{
	GPARoot *root;

	root = GPA_ROOT (data);

	root->vendors = NULL;
}

static void
gpa_root_vendors_modified (GPANode *node, guint flags, GPANode *root)
{
	gpa_node_request_modified (root, flags);
}

static void
gpa_root_printers_gone (gpointer data, GObject *gone)
{
	GPARoot *root;

	root = GPA_ROOT (data);

	root->printers = NULL;
}

static void
gpa_root_printers_modified (GPANode *node, guint flags, GPANode *root)
{
	gpa_node_request_modified (root, flags);
}

static void
gpa_root_media_gone (gpointer data, GObject *gone)
{
	GPARoot *root;

	root = GPA_ROOT (data);

	root->media = NULL;
}

static void
gpa_root_media_modified (GPANode *node, guint flags, GPANode *root)
{
	gpa_node_request_modified (root, flags);
}

static void
gpa_root_finalize (GObject *object)
{
	GPARoot *root;

	root = (GPARoot *) object;

	if (root->vendors) {
		/* Disconnect vendors */
		g_signal_handlers_disconnect_by_func (G_OBJECT (root->vendors), gpa_root_vendors_modified, root);
		g_object_weak_unref (G_OBJECT (root->vendors), gpa_root_vendors_gone, root);
		root->vendors = NULL;
	}

	if (root->printers) {
		/* Disconnect printers */
		g_signal_handlers_disconnect_by_func (G_OBJECT (root->printers), gpa_root_printers_modified, root);
		g_object_weak_unref (G_OBJECT (root->printers), gpa_root_printers_gone, root);
		root->printers = NULL;
	}

	if (root->media) {
		/* Disconnect media */
		g_signal_handlers_disconnect_by_func (G_OBJECT (root->media), gpa_root_media_modified, root);
		g_object_weak_unref (G_OBJECT (root->media), gpa_root_media_gone, root);
		root->media = NULL;
	}

	root_instance = NULL;

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GPANode *
gpa_root_get_child (GPANode *node, GPANode *ref)
{
	GPARoot *root;

	root = GPA_ROOT (node);

	if (!ref) {
		if (root->vendors) {
			return gpa_node_ref (root->vendors);
		} else {
			root->vendors = GPA_NODE (gpa_vendor_list_load ());
			g_object_weak_ref (G_OBJECT (root->vendors), gpa_root_vendors_gone, root);
			g_signal_connect (G_OBJECT (root->vendors), "modified", G_CALLBACK (gpa_root_vendors_modified), root);
			return root->vendors;
		}
	} else if (ref == root->vendors) {
		if (root->printers) {
			return gpa_node_ref (root->printers);
		} else {
			root->printers = GPA_NODE (gpa_printer_list_load ());
			g_object_weak_ref (G_OBJECT (root->printers), gpa_root_printers_gone, root);
			g_signal_connect (G_OBJECT (root->printers), "modified", G_CALLBACK (gpa_root_printers_modified), root);
			return root->printers;
		}
	} else if (ref == root->printers) {
		if (root->media) {
			return gpa_node_ref (root->media);
		} else {
			root->media = gpa_media_load ();
			g_object_weak_ref (G_OBJECT (root->media), gpa_root_media_gone, root);
			g_signal_connect (G_OBJECT (root->media), "modified", G_CALLBACK (gpa_root_media_modified), root);
			return root->media;
		}
	}

	return NULL;
}

static GPANode *
gpa_root_lookup (GPANode *node, const guchar *path)
{
	GPARoot *root;
	GPANode *child;
	const guchar *subpath;

	root = GPA_ROOT (node);

	child = NULL;

	subpath = gpa_node_lookup_check (path, "Vendors");
	if (subpath) {
		GPANode *vendors;
		vendors = gpa_node_cache (GPA_NODE (gpa_vendor_list_load ()));
		child = gpa_node_lookup (vendors, subpath);
		gpa_node_unref (vendors);
		return child;
	}
	subpath = gpa_node_lookup_check (path, "Printers");
	if (subpath) {
		GPANode *printers;
		printers = gpa_node_cache (GPA_NODE (gpa_printer_list_load ()));
		child = gpa_node_lookup (printers, subpath);
		gpa_node_unref (printers);
		return child;
	}
	subpath = gpa_node_lookup_check (path, "Media");
	if (subpath) {
		GPANode *media;
		media = gpa_node_cache (gpa_media_load ());
		child = gpa_node_lookup (media, subpath);
		gpa_node_unref (media);
		return child;
	}

	return NULL;
}

static void
gpa_root_modified (GPANode *node, guint flags)
{
	GPARoot *root;

	root = GPA_ROOT (node);

	if (root->vendors && (GPA_NODE_FLAGS (root->vendors) & GPA_MODIFIED_FLAG)) {
		gpa_node_emit_modified (GPA_NODE (root->vendors), 0);
	}
	if (root->printers && (GPA_NODE_FLAGS (root->printers) & GPA_MODIFIED_FLAG)) {
		gpa_node_emit_modified (GPA_NODE (root->printers), 0);
	}

	/* Media gets its own idle thread */
}

GPANode *
gpa_root_get (void)
{
	GPANode *root;

	root = gpa_node_new (GPA_TYPE_ROOT, "Globals");

	return root;
}


