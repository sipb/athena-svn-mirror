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
 *  Copyright (C) 2000-2003 Ximian, Inc.
 *
 */

#include <config.h>

#include <string.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include "gpa-utils.h"
#include "gpa-reference.h"
#include "gpa-option.h"
#include "gpa-model.h"
#include "gpa-root.h"
#include "gpa-node.h"
#include "gpa-utils.h"

static void gpa_model_class_init (GPAModelClass *klass);
static void gpa_model_init (GPAModel *model);
static void gpa_model_finalize (GObject *object);

static gboolean  gpa_model_verify    (GPANode *node);

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

	node_class->verify    = gpa_model_verify;
}

static void
gpa_model_init (GPAModel *model)
{
	model->name    = NULL;
	model->options = NULL;
}

static void
gpa_model_finalize (GObject *object)
{
	GPAModel *model;

	model = GPA_MODEL (object);

	g_hash_table_remove (models_dict, GPA_NODE_ID (model));

	my_g_free (model->name);
	model->name    = NULL;
	model->options = gpa_node_detach_unref (model->options);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
gpa_model_verify (GPANode *node)
{
	GPAModel *model;

	model = GPA_MODEL (node);

	gpa_return_false_if_fail (model->name != NULL);
	gpa_return_false_if_fail (gpa_node_verify (model->options));

	return TRUE;
}

/**
 * gpa_model_hash_lookup:
 * @id: 
 * 
 * Lookup a model in the global model dictionary
 * 
 * Return Value: a referenced GPAModel if found, NULL otherwise
 **/
GPANode *
gpa_model_hash_lookup (const gchar *id)
{
	GPANode *model;
	
	if (!models_dict) {
		models_dict = g_hash_table_new (g_str_hash, g_str_equal);
		return NULL;
	}

	model = g_hash_table_lookup (models_dict, id);
	if (model)
		gpa_node_ref (model);

	return model;
}

/**
 * gpa_model_hash_insert:
 * @model: 
 * 
 * Insert a Model into the global models dictionary
 **/
void
gpa_model_hash_insert (GPAModel *model)
{
	GPANode *check;
	const gchar *id = GPA_NODE_ID (model);
		
	if (!models_dict)
		models_dict = g_hash_table_new (g_str_hash, g_str_equal);

	check = gpa_model_hash_lookup (id);
	if (check) {
		g_warning ("Model %s already in hash, replacing it", id);
		gpa_node_unref (check);
	}

	g_hash_table_insert (models_dict, g_strdup (id), model);
}

/**
 * gpa_model_new_from_tree:
 * @tree: 
 * 
 * Load a GPAModel from an XML node
 * 
 * Return Value: 
 **/
GPANode *
gpa_model_new_from_tree (xmlNodePtr tree)
{
	xmlNodePtr node;
	xmlChar *version = NULL;
	xmlChar *id =  NULL;
	GPAModel *model = NULL;

	g_return_val_if_fail (tree != NULL, NULL);
	g_return_val_if_fail (tree->name != NULL, NULL);

	if (strcmp (tree->name, "Model")) {
		g_warning ("Root node should be <Model>, node found is <%s>", tree->name);
		goto new_from_tree_done;
	}
	
	id = xmlGetProp (tree, "Id");
	if (!id) {
		g_warning ("Model node does not have Id");
		goto new_from_tree_done;
	}

	model = (GPAModel *) gpa_model_hash_lookup (id);
	if (model) {
		g_warning ("Model %s already loded", id);
		goto new_from_tree_done;
	}

	version = xmlGetProp (tree, "Version");
	if (!version || strcmp (version, "1.0")) {
		g_warning ("Wrong model version %s, should be 1.0.",version);
		goto new_from_tree_done;
	}

	model = (GPAModel *) gpa_node_new (GPA_TYPE_MODEL, id);

	for (node = tree->xmlChildrenNode; node != NULL; node = node->next) {
		if (!node->name)
			continue;
		
		if (!strcmp (node->name, "Name")) {
			xmlChar *name;
			name = xmlNodeGetContent (node);
			model->name = g_strdup (node->name);
			xmlFree (name);
			continue;
		}

		if (!strcmp (node->name, "Options")) {
			GPANode *options;
			options = gpa_option_node_new_from_tree (node,
								 GPA_NODE (model),
								 "Options");
			model->options = options;
			continue;
		}

	}

	if (!model->name || !model->options) {
		g_warning ("Could not load Model \"%s\", name or options missing or invalid", id);
		gpa_node_unref (GPA_NODE (model));
		model = NULL;
		goto new_from_tree_done;
	}

	gpa_node_reverse_children (GPA_NODE (model));
	gpa_model_hash_insert (model);
	
new_from_tree_done:
	my_xmlFree (id);
	my_xmlFree (version);

	return (GPANode *) model;
}

/**
 * gpa_model_get_by_id:
 * @id: 
 * @quiet: don't warn if the model is not found
 * 
 * Lookup a model by its id, if the model is not found in the
 * models dictionary, we attempt to load it from GPA_DATADIR/model
 * 
 * Return Value: the loaded GPAModel, NULL on error or if the model
 *               could not be loaded
 **/
GPANode *
gpa_model_get_by_id (const guchar *id, gboolean quiet)
{
	xmlDocPtr doc = NULL;
	GPANode *model;
	gchar *path = NULL;
	gchar *file;

	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (*id != '\0', NULL);

	model = gpa_model_hash_lookup (id);
	if (model) {
		goto get_by_id_done;
	}

	file = g_strconcat (id, ".xml", NULL);
	path = g_build_filename (GPA_DATA_DIR, "models", file, NULL);
	g_free (file);
	if (!g_file_test (path, G_FILE_TEST_IS_REGULAR)) {
		if (!quiet)
			g_warning ("Could not get model by id \"%s\" from \"%s\"", id, path);
		goto get_by_id_done;
	}

	doc = xmlParseFile (path);
	if (!doc) {
		g_warning ("Could not parse XML. Model by id \"%s\" from \"%s\"", id, path);
		goto get_by_id_done;
	}

	model = gpa_model_new_from_tree (doc->xmlRootNode);

get_by_id_done:
	my_xmlFreeDoc (doc);
	my_g_free (path);

	return model;
}

