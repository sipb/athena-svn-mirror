/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-printer.c: 
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

#define __GPA_PRINTER_C__
#define noGPA_PRINTER_DEBUG

#include <string.h>
#include <sys/types.h>
#include <dirent.h> /* For the DIR structure stuff */
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "gpa-utils.h"
#include "gpa-value.h"
#include "gpa-reference.h"
#include "gpa-settings.h"
#include "gpa-model.h"
#include "gpa-printer.h"

/* GPAPrinter */

static void gpa_printer_class_init (GPAPrinterClass *klass);
static void gpa_printer_init (GPAPrinter *printer);

static void gpa_printer_finalize (GObject *object);

static gboolean gpa_printer_verify (GPANode *node);
static guchar *gpa_printer_get_value (GPANode *node);
static GPANode *gpa_printer_get_child (GPANode *node, GPANode *ref);
static GPANode *gpa_printer_lookup (GPANode *node, const guchar *path);
static void gpa_printer_modified (GPANode *node, guint flags);

static GPANode *gpa_printer_new_from_file (const gchar *filename);

static GHashTable *namedict = NULL;

static GPANodeClass *parent_class = NULL;

GType
gpa_printer_get_type (void) {
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPAPrinterClass),
			NULL, NULL,
			(GClassInitFunc) gpa_printer_class_init,
			NULL, NULL,
			sizeof (GPAPrinter),
			0,
			(GInstanceInitFunc) gpa_printer_init
		};
		type = g_type_register_static (GPA_TYPE_NODE, "GPAPrinter", &info, 0);
	}
	return type;
}

static void
gpa_printer_class_init (GPAPrinterClass *klass)
{
	GObjectClass *object_class;
	GPANodeClass *node_class;

	object_class = (GObjectClass *) klass;
	node_class = (GPANodeClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gpa_printer_finalize;

	node_class->verify = gpa_printer_verify;
	node_class->get_value = gpa_printer_get_value;
	node_class->get_child = gpa_printer_get_child;
	node_class->lookup = gpa_printer_lookup;
	node_class->modified = gpa_printer_modified;
}

static void
gpa_printer_init (GPAPrinter *printer)
{
	printer->name = NULL;
	printer->settings = NULL;
	printer->model = NULL;
}

static void
gpa_printer_finalize (GObject *object)
{
	GPAPrinter *printer;

	printer = GPA_PRINTER (object);

	if (printer->name) {
		g_assert (namedict != NULL);
		if (printer == g_hash_table_lookup (namedict, GPA_VALUE (printer->name)->value)) {
			g_hash_table_remove (namedict, GPA_VALUE (printer->name)->value);
		}
	}

	printer->name = gpa_node_detach_unref (GPA_NODE (printer), GPA_NODE (printer->name));
	printer->settings = (GPAList *) gpa_node_detach_unref (GPA_NODE (printer), GPA_NODE (printer->settings));
	printer->model = gpa_node_detach_unref (GPA_NODE (printer), GPA_NODE (printer->model));

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
gpa_printer_verify (GPANode *node)
{
	GPAPrinter *printer;

	printer = GPA_PRINTER (node);

	if (GPA_NODE_ID_EXISTS (node))
		return FALSE;
	if (!printer->name)
		return FALSE;
	if (!gpa_node_verify (printer->name))
		return FALSE;
	if (!printer->settings)
		return FALSE;
	if (!gpa_node_verify (GPA_NODE (printer->settings)))
		return FALSE;
	if (!printer->model)
		return FALSE;
	if (!gpa_node_verify (printer->model))
		return FALSE;

	return TRUE;
}

static guchar *
gpa_printer_get_value (GPANode *node)
{
	GPAPrinter *printer;

	printer = GPA_PRINTER (node);

	if (GPA_NODE_ID_EXISTS (node))
		return g_strdup (GPA_NODE_ID (node));

	return NULL;
}

static GPANode *
gpa_printer_get_child (GPANode *node, GPANode *ref)
{
	GPAPrinter *printer;
	GPANode *child;

	printer = GPA_PRINTER (node);

	g_return_val_if_fail (printer->settings != NULL, NULL);
	g_return_val_if_fail (printer->model != NULL, NULL);
	/* Model is reference */

	child = NULL;
	if (ref == NULL) {
		child = printer->name;
	} else if (ref == printer->name) {
		child = GPA_NODE (printer->settings);
	} else if (ref == GPA_NODE (printer->settings)) {
		child = printer->model;
	}

	if (child)
		gpa_node_ref (child);

	return child;
}

static GPANode *
gpa_printer_lookup (GPANode *node, const guchar *path)
{
	GPAPrinter *printer;
	GPANode *child;

	printer = GPA_PRINTER (node);

	child = NULL;

	if (gpa_node_lookup_ref (&child, GPA_NODE (printer->name), path, "Name"))
		return child;
	if (gpa_node_lookup_ref (&child, GPA_NODE (printer->settings), path, "Settings"))
		return child;
	if (gpa_node_lookup_ref (&child, GPA_NODE (printer->model), path, "Model"))
		return child;

	return NULL;
}

static void
gpa_printer_modified (GPANode *node, guint flags)
{
	GPAPrinter *printer;

	printer = GPA_PRINTER (node);

	if (printer->name && (GPA_NODE_FLAGS (printer->name) & GPA_MODIFIED_FLAG)) {
		gpa_node_emit_modified (printer->name, 0);
	}
	if (printer->model && (GPA_NODE_FLAGS (printer->model) & GPA_MODIFIED_FLAG)) {
		gpa_node_emit_modified (printer->model, 0);
	}
	if (printer->settings && (GPA_NODE_FLAGS (printer->settings) & GPA_MODIFIED_FLAG)) {
		gpa_node_emit_modified (GPA_NODE (printer->settings), 0);
	}
}

/* Public methods */

GPANode *
gpa_printer_new_from_tree (xmlNodePtr tree)
{
	GPAPrinter *printer;
	xmlChar *xmlid, *xmlver;
	xmlNodePtr xmlc;
	GPANode *name;
	GPANode *model;
	GSList *l;

	g_return_val_if_fail (tree != NULL, NULL);

	/* Check that tree is <Printer> */
	if (strcmp (tree->name, "Printer")) {
		g_warning ("file %s: line %d: Base node is <%s>, should be <Printer>", __FILE__, __LINE__, tree->name);
		return NULL;
	}
	/* Check that printer has Id */
	xmlid = xmlGetProp (tree, "Id");
	if (!xmlid) {
		g_warning ("file %s: line %d: Printer node does not have Id", __FILE__, __LINE__);
		return NULL;
	}

	/* Check Printer definition version */
	xmlver = xmlGetProp (tree, "Version");
	if (!xmlver || strcmp (xmlver, "1.0")) {
		g_warning ("file %s: line %d: Wrong printer version %s, should be 1.0", __FILE__, __LINE__, xmlver);
		xmlFree (xmlid);
		if (xmlver) xmlFree (xmlver);
		return NULL;
	}
	xmlFree (xmlver);

	if (!namedict)
		namedict = g_hash_table_new (g_str_hash, g_str_equal);

	printer = NULL;
	name = NULL;
	model = NULL;
	l = NULL;

	for (xmlc = tree->xmlChildrenNode; xmlc != NULL; xmlc = xmlc->next) {
		if (!strcmp (xmlc->name, "Name")) {
			xmlChar *content;
			content = xmlNodeGetContent (xmlc);
			if (content && *content) {
#if 0
				if (!g_hash_table_lookup (namedict, content)) {
					name = gpa_value_new ("Name", content);
				}
#else
				name = gpa_value_new ("Name", content);
#endif
				xmlFree (content);
			}
		} else if (!strcmp (xmlc->name, "Settings")) {
			if (model) {
				GPANode *settings;
				settings = gpa_settings_new_from_model_and_tree (model, xmlc);
				if (settings) l = g_slist_prepend (l, settings);
			} else {
				g_warning ("Settings without model in printer definition");
			}
		} else if (!strcmp (xmlc->name, "Model")) {
			xmlChar *content;
			content = xmlNodeGetContent (xmlc);
			if (content && *content) {
				model = gpa_model_get_by_id (content);
				xmlFree (content);
			}
		}
	}

	if (name && model && l) {
		printer = (GPAPrinter *) gpa_node_new (GPA_TYPE_PRINTER, xmlid);
		/* Name */
		printer->name = name;
		name->parent = GPA_NODE (printer);
#if 0
		g_assert (!g_hash_table_lookup (namedict, GPA_VALUE (name)->value));
#endif
		g_hash_table_insert (namedict, GPA_VALUE (name)->value, printer);
		/* Settings */
		/* fixme: here are defaults again... */
		printer->settings = GPA_LIST (gpa_list_new (GPA_TYPE_SETTINGS, TRUE));
		GPA_NODE (printer->settings)->parent = GPA_NODE (printer);
		while (l) {
			GPANode *settings;
			settings = GPA_NODE (l->data);
			l = g_slist_remove (l, settings);
			settings->parent = GPA_NODE (printer->settings);
			settings->next = printer->settings->children;
			printer->settings->children = settings;
		}
		/* fixme: */
		if (printer->settings->children)
			gpa_list_set_default (printer->settings, printer->settings->children);
		/* Model */
		printer->model = gpa_reference_new (model);
		printer->model->parent = GPA_NODE (printer);
		gpa_node_unref (GPA_NODE (model));
	} else {
		if (name)
			gpa_node_unref (name);
		if (model)
			gpa_node_unref (model);
		while (l) {
			gpa_node_unref (GPA_NODE (l->data));
			l = g_slist_remove (l, l->data);
		}
	}

	xmlFree (xmlid);

	return (GPANode *) printer;
}

static GPANode *
gpa_printer_new_from_file (const gchar *filename)
{
	GPANode *printer;
	xmlDocPtr doc;
	xmlNodePtr root;

	doc = xmlParseFile (filename);
	if (!doc)
		return NULL;
	root = doc->xmlRootNode;
	printer = NULL;
	if (!strcmp (root->name, "Printer")) {
		printer = gpa_printer_new_from_tree (root);
	}
	xmlFreeDoc (doc);
	return printer;
}

/* GPAPrinterList */

static void gpa_printer_list_load_from_dir (GPAList *printers, const gchar *dirname);

static GPAList *printers = NULL;

static void
gpa_printers_gone (gpointer data, GObject *gone)
{
#ifdef GPA_PRINTER_DEBUG
	g_print ("GPAPrinter: Printer list %p has gone\n", gone);
#endif

	printers = NULL;
}

GPAList *
gpa_printer_list_load (void)
{
	gchar *dirname;

	if (printers) {
		return (GPAList *) gpa_node_ref (GPA_NODE (printers));
	}

	printers = GPA_LIST (gpa_node_new (GPA_TYPE_LIST, "Printers"));
	gpa_list_construct (GPA_LIST (printers), GPA_TYPE_PRINTER, TRUE);
	g_object_weak_ref (G_OBJECT (printers), gpa_printers_gone, &printers);

	dirname = g_strdup_printf ("%s/%s", g_get_home_dir (), ".gnome/printers");
	gpa_printer_list_load_from_dir (printers, dirname);
	g_free (dirname);
	/* fixme: */
	gpa_printer_list_load_from_dir (printers, DATADIR "/gnome-print-2.0/printers");

	/* fixme: During parsing, please */
	if (printers->children) {
		gpa_node_set_path_value (GPA_NODE (printers), "Default", GPA_NODE_ID (printers->children));
	}

	return printers;
}

static void
gpa_printer_list_load_from_dir (GPAList *printers, const gchar *dirname)
{
	DIR *dir;
	struct dirent *dent;
	GHashTable *iddict;
	GSList *l;

	dir = opendir (dirname);
	if (!dir)
		return;

	l = NULL;
	iddict = g_hash_table_new (g_str_hash, g_str_equal);
	while ((dent = readdir (dir))) {
		gint len;
		gchar *filename;
		GPANode *printer;
		len = strlen (dent->d_name);
		if (len < 9)
			continue;
		if (strcmp (dent->d_name + len - 8, ".printer"))
			continue;
		filename = g_strdup_printf ("%s/%s", dirname, dent->d_name);
		printer = gpa_printer_new_from_file (filename);
		g_free (filename);
		if (printer) {
			if (g_hash_table_lookup (iddict, GPA_NODE_ID (printer))) {
				gpa_node_unref (printer);
			} else {
				g_hash_table_insert (iddict, (gpointer) GPA_NODE_ID (printer), printer);
				l = g_slist_prepend (l, printer);
			}
		}
	}
	g_hash_table_destroy (iddict);
	closedir (dir);

	while (l) {
		/* fixme: ordering */
		GPANode *printer;
		printer = GPA_NODE (l->data);
		l = g_slist_remove (l, printer);
		printer->next = printers->children;
		printers->children = printer;
		printer->parent = GPA_NODE (printers);
	}
}


/* Deprecated */

GPANode *
gpa_printer_get_default (void)
{
	GPAList *printers;
	GPANode *def;

	printers = gpa_printer_list_load ();

	if (printers->def) {
		def = GPA_REFERENCE_REFERENCE (printers->def);
	} else {
		def = printers->children;
	}

	if (def)
		gpa_node_ref (def);

	gpa_node_unref (gpa_node_cache (GPA_NODE (printers)));

	return def;
}

GPANode *
gpa_printer_get_by_id (const guchar *id)
{
	GPAList *printers;
	GPANode *child;

	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (*id != '\0', NULL);

	printers = gpa_printer_list_load ();

	child = NULL;

	if (printers) {
		for (child = printers->children; child != NULL; child = child->next) {
			g_assert (GPA_IS_PRINTER (child));
			if (GPA_NODE_ID_COMPARE (child, id))
				break;
		}
	}

	if (child)
		gpa_node_ref (child);

	gpa_node_unref (gpa_node_cache (GPA_NODE (printers)));

	return child;
}

GPANode *
gpa_printer_get_default_settings (GPAPrinter *printer)
{
	g_return_val_if_fail (printer != NULL, NULL);
	g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);

	if (printer->settings && printer->settings->children) {
		g_assert (GPA_IS_SETTINGS (printer->settings->children));
		gpa_node_ref (printer->settings->children);
		return printer->settings->children;
	}

	return NULL;
}

/* fixme: Thorough check */

GPANode *
gpa_printer_new_from_model (GPAModel *model, const guchar *name)
{
	GPAList *printers;
	GPAPrinter *printer;
	GPANode *settings;
	guchar *id;

	g_return_val_if_fail (model != NULL, NULL);
	g_return_val_if_fail (GPA_IS_MODEL (model), NULL);
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (*name != '\0', NULL);

	if (!namedict)
		namedict = g_hash_table_new (g_str_hash, g_str_equal);
#if 0
	g_return_val_if_fail (!g_hash_table_lookup (namedict, name), NULL);
#endif

	printers = gpa_printer_list_load ();

	id = gpa_id_new (GPA_NODE_ID (model));
	printer = (GPAPrinter *) gpa_node_new (GPA_TYPE_PRINTER, id);
	g_free (id);

	/* Name */
	printer->name = gpa_node_attach (GPA_NODE (printer), gpa_value_new ("Name", name));
	g_hash_table_insert (namedict, GPA_VALUE (printer->name)->value, printer);

	/* Settings */
	/* fixme: Implement helper */
	/* fixme: defaults again */
	printer->settings = GPA_LIST (gpa_node_attach (GPA_NODE (printer), gpa_list_new (GPA_TYPE_SETTINGS, TRUE)));
	settings = gpa_settings_new_from_model (GPA_NODE (model), "Default");
	gpa_list_add_child (printer->settings, settings, NULL);
	gpa_node_unref (settings);
	/* fixme: */
	gpa_list_set_default (printer->settings, settings);

	/* Model */
	printer->model = gpa_node_attach (GPA_NODE (printer), gpa_reference_new (GPA_NODE (model)));

	gpa_list_add_child (printers, GPA_NODE (printer), NULL);

	gpa_node_unref (gpa_node_cache (GPA_NODE (printers)));

	return (GPANode *) printer;
}

gboolean
gpa_printer_save (GPAPrinter *printer)
{
	xmlDocPtr doc;
	xmlNodePtr root, xmln;
	GPANode *child;
	guchar *filename;

	g_return_val_if_fail (printer != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_PRINTER (printer), FALSE);

	g_return_val_if_fail (gpa_node_verify (GPA_NODE (printer)), FALSE);

	doc = xmlNewDoc ("1.0");
	root = xmlNewDocNode (doc, NULL, "Printer", NULL);
	xmlSetProp (root, "Version", "1.0");
	xmlSetProp (root, "Id", GPA_NODE_ID (printer));
	xmlDocSetRootElement (doc, root);

	xmln = xmlNewChild (root, NULL, "Name", GPA_VALUE (printer->name)->value);

	xmln = xmlNewChild (root, NULL, "Model", GPA_NODE_ID (GPA_REFERENCE (printer->model)->ref));

	for (child = printer->settings->children; child != NULL; child = child->next) {
		xmln = gpa_settings_write (doc, child);
		if (xmln)
			xmlAddChild (root, xmln);
	}

	filename = g_strdup_printf ("%s/.gnome/printers/%s.printer", g_get_home_dir (), GPA_NODE_ID (printer));
	xmlSaveFile (filename, doc);
	g_free (filename);

	xmlFreeDoc (doc);

	return TRUE;
}

