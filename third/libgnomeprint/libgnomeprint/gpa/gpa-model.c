/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-model.c: 
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

#define __GPA_MODEL_C__

#include <string.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include "gpa-utils.h"
#include "gpa-value.h"
#include "gpa-reference.h"
#include "gpa-vendor.h"
#include "gpa-option.h"
#include "gpa-model.h"

/* GPAModel */

static void gpa_model_class_init (GPAModelClass *klass);
static void gpa_model_init (GPAModel *model);
static void gpa_model_finalize (GObject *object);

static gboolean gpa_model_verify (GPANode *node);
static guchar *gpa_model_get_value (GPANode *node);
static GPANode *gpa_model_get_child (GPANode *node, GPANode *ref);
static GPANode *gpa_model_lookup (GPANode *node, const guchar *path);
static void gpa_model_modified (GPANode *node, guint flags);

static GHashTable *modeldict = NULL;
static GPANodeClass *parent_class = NULL;

GType
gpa_model_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPAModelClass),
			NULL, NULL,
			(GClassInitFunc) gpa_model_class_init,
			NULL, NULL,
			sizeof (GPAModel),
			0,
			(GInstanceInitFunc) gpa_model_init
		};
		type = g_type_register_static (GPA_TYPE_NODE, "GPAModel", &info, 0);
	}
	return type;
}

static void
gpa_model_class_init (GPAModelClass *klass)
{
	GObjectClass *object_class;
	GPANodeClass *node_class;

	object_class = (GObjectClass *) klass;
	node_class = (GPANodeClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gpa_model_finalize;

	node_class->verify = gpa_model_verify;
	node_class->get_value = gpa_model_get_value;
	node_class->get_child = gpa_model_get_child;
	node_class->lookup = gpa_model_lookup;
	node_class->modified = gpa_model_modified;
}

static void
gpa_model_init (GPAModel *model)
{
	model->loaded = FALSE;

	model->vendorid = NULL;

	model->name = NULL;
	model->vendor = NULL;
	model->options = NULL;
}

static void
gpa_model_vendor_gone (gpointer data, GObject *gone)
{
	GPAModel *model;

	model = GPA_MODEL (data);

	model->vendor = NULL;
}

static void
gpa_model_vendor_modified (GPANode *node, guint flags, GPANode *root)
{
	gpa_node_request_modified (root, flags);
}

static void
gpa_model_finalize (GObject *object)
{
	GPAModel *model;

	model = GPA_MODEL (object);

	if (GPA_NODE_ID_EXISTS (model)) {
		g_assert (modeldict != NULL);
#if 0
		g_assert (g_hash_table_lookup (modeldict, GPA_NODE_ID (model)) != NULL);
#endif
		g_hash_table_remove (modeldict, GPA_NODE_ID (model));
	}

	if (model->vendorid)
		g_free (model->vendorid);

	model->name = gpa_node_detach_unref (GPA_NODE (model), GPA_NODE (model->name));
	if (model->vendor) {
		g_signal_handlers_disconnect_by_func (G_OBJECT (model->vendor), gpa_model_vendor_modified, model);
		g_object_weak_unref (G_OBJECT (model->vendor), gpa_model_vendor_gone, model);
		model->vendor = NULL;
	}
	if (model->options) {
		model->options = gpa_node_detach_unref (GPA_NODE (model), GPA_NODE (model->options));
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
gpa_model_verify (GPANode *node)
{
	GPAModel *model;

	model = GPA_MODEL (node);

	if (!model->name)
		return FALSE;
	if (!gpa_node_verify (model->name))
		return FALSE;
	if (model->loaded) {
		if (!model->vendorid)
			return FALSE;
		if (model->vendor && !gpa_node_verify (model->vendor))
			return FALSE;
		if (!model->options)
			return FALSE;
		if (!gpa_node_verify (GPA_NODE (model->options)))
			return FALSE;
	}

	return TRUE;
}

static guchar *
gpa_model_get_value (GPANode *node)
{
	GPAModel *model;

	model = GPA_MODEL (node);

	if (GPA_NODE_ID_EXISTS (node)) return g_strdup (GPA_NODE_ID (node));

	return NULL;
}

static GPANode *
gpa_model_get_child (GPANode *node, GPANode *ref)
{
	GPAModel *model;
	GPANode *child;

	model = GPA_MODEL (node);

	child = NULL;
	if (ref == NULL) {
		child = model->name;
	} else if (ref == model->name) {
		if (model->vendor) {
			return gpa_node_ref (model->vendor);
		} else if (model->vendorid) {
			model->vendor = gpa_vendor_get_by_id (model->vendorid);
			g_object_weak_ref (G_OBJECT (model->vendor), gpa_model_vendor_gone, model);
			g_signal_connect (G_OBJECT (model->vendor), "modified", G_CALLBACK (gpa_model_vendor_modified), model);
			return model->vendor;
		}
	} else if (ref == model->vendor) {
		child = GPA_NODE (model->options);
	}

	if (child) gpa_node_ref (child);

	return child;
}

static GPANode *
gpa_model_lookup (GPANode *node, const guchar *path)
{
	GPAModel *model;
	GPANode *child;
	const guchar *subpath;

	model = GPA_MODEL (node);

	child = NULL;

	if (gpa_node_lookup_ref (&child, GPA_NODE (model->name), path, "Name"))
		return child;
	if (model->vendor) {
		if (gpa_node_lookup_ref (&child, GPA_NODE (model->vendor), path, "Vendor"))
			return child;
	} else if (model->vendorid) {
		subpath = gpa_node_lookup_check (path, "Vendor");
		if (subpath) {
			GPANode *vendor;
			vendor = gpa_node_cache (GPA_NODE (gpa_vendor_get_by_id (model->vendorid)));
			child = gpa_node_lookup (vendor, subpath);
			gpa_node_unref (vendor);
			return child;
		}
	}
	if (gpa_node_lookup_ref (&child, GPA_NODE (model->options), path, "Options")) return child;

	return NULL;
}

static void
gpa_model_modified (GPANode *node, guint flags)
{
	GPAModel *model;

	model = GPA_MODEL (node);

	if (model->name && (GPA_NODE_FLAGS (model->name) & GPA_MODIFIED_FLAG)) {
		gpa_node_emit_modified (model->name, 0);
	}
	if (model->vendor && (GPA_NODE_FLAGS (model->vendor) & GPA_MODIFIED_FLAG)) {
		gpa_node_emit_modified (model->vendor, 0);
	}
	if (model->options && (GPA_NODE_FLAGS (model->options) & GPA_MODIFIED_FLAG)) {
		gpa_node_emit_modified (GPA_NODE (model->options), 0);
	}
}

/* Public Methods */

GPANode *
gpa_model_new_from_info_tree (xmlNodePtr tree)
{
	GPAModel *model;
	xmlChar *xmlid;
	xmlNodePtr xmlc;
	GPANode *name;
	guchar *filename;

	g_return_val_if_fail (tree != NULL, NULL);

	/* Check that tree is <Model> */
	if (strcmp (tree->name, "Model")) {
		g_warning ("file %s: line %d: Base node is <%s>, should be <Model>", __FILE__, __LINE__, tree->name);
		return NULL;
	}
	/* Check that model has Id */
	xmlid = xmlGetProp (tree, "Id");
	if (!xmlid) {
		g_warning ("file %s: line %d: Model node does not have Id", __FILE__, __LINE__);
		return NULL;
	}

	/* Check for model file */
	filename = g_strdup_printf (DATADIR "/gnome-print-2.0/models/%s.model", xmlid);
	if (!g_file_test (filename, G_FILE_TEST_IS_REGULAR)) {
		g_warning ("Model description file is missing %s", xmlid);
		xmlFree (xmlid);
		g_free (filename);
		return NULL;
	}
	g_free (filename);

	if (!modeldict)
		modeldict = g_hash_table_new (g_str_hash, g_str_equal);
	model = g_hash_table_lookup (modeldict, xmlid);
	if (model != NULL) {
		gpa_node_ref (GPA_NODE (model));
		return GPA_NODE (model);
	}

	model = NULL;
	name = NULL;

	for (xmlc = tree->xmlChildrenNode; xmlc != NULL; xmlc = xmlc->next) {
		if (!strcmp (xmlc->name, "Name")) {
			name = gpa_value_new_from_tree ("Name", xmlc);
		}
	}

	if (name) {
		model = (GPAModel *) gpa_node_new (GPA_TYPE_MODEL, xmlid);
		model->name = name;
		name->parent = GPA_NODE (model);
		g_hash_table_insert (modeldict, (gpointer) GPA_NODE_ID (model), model);
	} else {
		g_warning ("Incomplete model description");
	}

	xmlFree (xmlid);

	return (GPANode *) model;
}

GPANode *
gpa_model_new_from_tree (xmlNodePtr tree)
{
	GPAModel *model;
	xmlChar *xmlid, *xmlver;
	xmlNodePtr xmlc;
	GPANode *name;
	GPANode *vendor;
	GPAList *options;

	g_return_val_if_fail (tree != NULL, NULL);

	/* Check that tree is <Model> */
	if (strcmp (tree->name, "Model")) {
		g_warning ("file %s: line %d: Base node is <%s>, should be <Model>", __FILE__, __LINE__, tree->name);
		return NULL;
	}
	/* Check that model has Id */
	xmlid = xmlGetProp (tree, "Id");
	if (!xmlid) {
		g_warning ("file %s: line %d: Model node does not have Id", __FILE__, __LINE__);
		return NULL;
	}
	/* Check Model definition version */
	xmlver = xmlGetProp (tree, "Version");
	if (!xmlver || strcmp (xmlver, "1.0")) {
		g_warning ("file %s: line %d: Wrong model version %s, should be 1.0", __FILE__, __LINE__, xmlver);
		xmlFree (xmlid);
		if (xmlver) xmlFree (xmlver);
		return NULL;
	}
	xmlFree (xmlver);

	/* Create modeldict, if not already present */
	if (!modeldict) modeldict = g_hash_table_new (g_str_hash, g_str_equal);
	/* Check, whether model is already loaded */
	model = g_hash_table_lookup (modeldict, xmlid);
	if (model != NULL) {
		gpa_node_ref (GPA_NODE (model));
		return GPA_NODE (model);
	}

	model = NULL;
	name = NULL;
	vendor = NULL;
	options = NULL;

	for (xmlc = tree->xmlChildrenNode; xmlc != NULL; xmlc = xmlc->next) {
		if (!strcmp (xmlc->name, "Name")) {
			/* Create <Name> node */
			name = gpa_value_new_from_tree ("Name", xmlc);
		} else if (!strcmp (xmlc->name, "Vendor")) {
			xmlChar *vendorid;
			vendorid = xmlNodeGetContent (xmlc);
			if (vendorid) {
				/* Create #vendor node */
				vendor = gpa_vendor_get_by_id (vendorid);
				xmlFree (vendorid);
			}
		} else if (!strcmp (xmlc->name, "Options")) {
			/* Create <Options> node */
			options = gpa_option_list_new_from_tree (xmlc);
		}
	}

	if (name && vendor && options) {
		/* Everything is OK */
		model = (GPAModel *) gpa_node_new (GPA_TYPE_MODEL, xmlid);
		model->name = name;
		name->parent = GPA_NODE (model);
		g_hash_table_insert (modeldict, (gpointer) GPA_NODE_ID (model), model);
		model->vendorid = g_strdup (GPA_NODE_ID (vendor));
		gpa_node_unref (gpa_node_cache (vendor));
		model->options = gpa_node_attach (GPA_NODE (model), GPA_NODE (options));

		model->loaded = TRUE;
	} else {
		if (!name) {
			g_warning ("file %s: line %d: Model does not have valid name", __FILE__, __LINE__);
		}
		if (!vendor) {
			g_warning ("file %s: line %d: Model does not have valid vendor", __FILE__, __LINE__);
		}
		if (!options) {
			g_warning ("file %s: line %d: Model does not have valid options", __FILE__, __LINE__);
		}
		if (name)
			gpa_node_unref (name);
		if (vendor)
			gpa_node_unref (vendor);
		if (options)
			gpa_node_unref (GPA_NODE (options));
	}

	xmlFree (xmlid);

	return (GPANode *) model;
}

/* GPAModelList */

GPAList *
gpa_model_list_new_from_info_tree (xmlNodePtr tree)
{
	GPAList *models;
	xmlNodePtr xmlc;
	GSList *l;

	g_return_val_if_fail (!strcmp (tree->name, "Models"), NULL);

	l = NULL;
	for (xmlc = tree->xmlChildrenNode; xmlc != NULL; xmlc = xmlc->next) {
		if (!strcmp (xmlc->name, "Model")) {
			GPANode *model;
			model = gpa_model_new_from_info_tree (xmlc);
			if (model)
				l = g_slist_prepend (l, model);
		}
	}

	models = GPA_LIST (gpa_list_new (GPA_TYPE_MODEL, FALSE));
	gpa_node_construct (GPA_NODE (models), "Models");

	while (l) {
		GPANode *model;
		model = GPA_NODE (l->data);
		l = g_slist_remove (l, model);
		model->parent = GPA_NODE (models);
		model->next = models->children;
		models->children = model;
	}

	return models;
}

GPANode *
gpa_model_get_by_id (const guchar *id)
{
	GPAModel *model;
	gchar *path;
	xmlDocPtr doc;
	xmlNodePtr root;

	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (*id != '\0', NULL);

	if (!modeldict)
		modeldict = g_hash_table_new (g_str_hash, g_str_equal);
	model = g_hash_table_lookup (modeldict, id);
	if (model) {
		gpa_node_ref (GPA_NODE (model));
		return GPA_NODE (model);
	}

	path = g_strdup_printf (DATADIR "/gnome-print-2.0/models/%s.model", id);
	if (!g_file_test (path, G_FILE_TEST_IS_REGULAR)) {
		g_free (path);
		return NULL;
	}

	doc = xmlParseFile (path);
	g_free (path);
	if (!doc) {
		return NULL;
	}

	model = NULL;
	root = doc->xmlRootNode;
	if (!strcmp (root->name, "Model")) {
		model = GPA_MODEL (gpa_model_new_from_tree (root));
	}
	xmlFreeDoc (doc);

	return GPA_NODE (model);
}

gboolean
gpa_model_load (GPAModel *model)
{
	gchar *path;
	xmlDocPtr doc;
	xmlNodePtr root;
	xmlChar *xmlid;
	xmlNodePtr xmlc;
	GPANode *vendor;
	GPAList *options;

	g_return_val_if_fail (model != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_MODEL (model), FALSE);
	g_return_val_if_fail (!model->loaded, FALSE);

	path = g_strdup_printf (DATADIR "/gnome-print-2.0/models/%s.model", GPA_NODE_ID (model));
	if (!g_file_test (path, G_FILE_TEST_IS_REGULAR)) {
		g_warning ("Model description file missing %s", GPA_NODE_ID (model));
		g_free (path);
		return FALSE;
	}

	doc = xmlParseFile (path);
	g_free (path);
	if (!doc) {
		g_warning ("Invalid model description file %s", GPA_NODE_ID (model));
		return FALSE;
	}

	root = doc->xmlRootNode;
	if (strcmp (root->name, "Model")) {
		g_warning ("Invalid model description file %s", GPA_NODE_ID (model));
		return FALSE;
	}

	xmlid = xmlGetProp (root, "Id");
	if (!xmlid || !GPA_NODE_ID_COMPARE (model, xmlid)) {
		g_warning ("Missing \"Id\" node in model description %s", GPA_NODE_ID (model));
		return FALSE;
	}

	vendor = NULL;
	options = NULL;

	for (xmlc = root->xmlChildrenNode; xmlc != NULL; xmlc = xmlc->next) {
		/* fixme: vendor should be initialized from info tree */
		if (!strcmp (xmlc->name, "Vendor")) {
			xmlChar *vendorid;
			vendorid = xmlNodeGetContent (xmlc);
			if (vendorid) {
				vendor = gpa_vendor_get_by_id (vendorid);
				xmlFree (vendorid);
			}
		} else if (!strcmp (xmlc->name, "Options")) {
			options = gpa_option_list_new_from_tree (xmlc);
		}
	}

	if (vendor && options) {
		model->vendorid = g_strdup (GPA_NODE_ID (vendor));
		gpa_node_unref (gpa_node_cache (vendor));
		model->options = gpa_node_attach (GPA_NODE (model), GPA_NODE (options));
	} else {
		g_warning ("Incomplete model description");
		if (vendor) gpa_node_unref (vendor);
		if (options) gpa_node_unref (GPA_NODE (options));
		return FALSE;
	}

	xmlFree (xmlid);
	xmlFreeDoc (doc);

	model->loaded = TRUE;

	return TRUE;
}


