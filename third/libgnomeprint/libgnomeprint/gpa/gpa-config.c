/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-config.c: 
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
 *  Authors:
 *    Chema Celorio <chema@ximian.com>
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 2000-2003 Ximian, Inc.
 *
 */

#include <config.h>

#include <string.h>
#include "gpa-utils.h"
#include "gpa-reference.h"
#include "gpa-printer.h"
#include "gpa-settings.h"
#include "gpa-root.h"
#include "gpa-config.h"
#include "gpa-key.h"

typedef struct _GPAConfigClass GPAConfigClass;
struct _GPAConfigClass {
	GPANodeClass node_class;
};

static void gpa_config_class_init (GPAConfigClass *klass);
static void gpa_config_init (GPAConfig *config);

static void gpa_config_finalize (GObject *object);

static gboolean  gpa_config_verify (GPANode *node);
static GPANode * gpa_config_duplicate (GPANode *node);

static GPANodeClass *parent_class;

GType
gpa_config_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPAConfigClass),
			NULL, NULL,
			(GClassInitFunc) gpa_config_class_init,
			NULL, NULL,
			sizeof (GPAConfig),
			0,
			(GInstanceInitFunc) gpa_config_init
		};
		type = g_type_register_static (GPA_TYPE_NODE, "GPAConfig", &info, 0);
	}
	return type;
}

static void
gpa_config_class_init (GPAConfigClass *klass)
{
	GObjectClass *object_class;
	GPANodeClass *node_class;

	object_class = (GObjectClass*) klass;
	node_class = (GPANodeClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gpa_config_finalize;

	node_class->duplicate = gpa_config_duplicate;
	node_class->verify    = gpa_config_verify;
}

/**
 * gpa_config_printer_modified:
 * @node: 
 * 
 * A different ->printer has been set, update ->settings since
 * the old ->settings where for the previous printer
 **/
static void
gpa_config_printer_modified (GPANode *node)
{
	GPANode *settings;
	GPANode *printer;
	GPAConfig *config;

	g_return_if_fail (GPA_IS_REFERENCE (node));
	g_return_if_fail (GPA_IS_CONFIG (node->parent));

	config = GPA_CONFIG (node->parent);
	printer = GPA_REFERENCE_REFERENCE (config->printer);
	
	if (config->settings != NULL && 
	    GPA_REFERENCE_REFERENCE (config->settings) != NULL &&
	    GPA_SETTINGS (GPA_REFERENCE_REFERENCE (config->settings))->printer != NULL &&
	    printer == GPA_REFERENCE_REFERENCE (GPA_SETTINGS (GPA_REFERENCE_REFERENCE (config->settings))->printer)) {
		return;
	}

	settings = gpa_printer_get_default_settings (GPA_PRINTER (printer));
	gpa_reference_set_reference (GPA_REFERENCE (config->settings),
				     settings);
	gpa_node_emit_modified (GPA_NODE (config));
}

static void
gpa_config_init (GPAConfig *config)
{
	GPANode *p, *s;

	p = GPA_NODE (gpa_reference_new_emtpy ("Printer"));
	s = GPA_NODE (gpa_reference_new_emtpy ("Settings"));

	config->printer  = gpa_node_attach (GPA_NODE (config), p);
	config->settings = gpa_node_attach (GPA_NODE (config), s);

	g_signal_connect (G_OBJECT (p), "modified", (GCallback)
			  gpa_config_printer_modified, NULL);
}

static void
gpa_config_finalize (GObject *object)
{
	GPAConfig *config;

	config = (GPAConfig *) object;

	config->printer  = gpa_node_detach_unref (GPA_NODE (config->printer));
	config->settings = gpa_node_detach_unref (GPA_NODE (config->settings));

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
gpa_config_verify (GPANode *node)
{
	GPAConfig *config;

	config = GPA_CONFIG (node);

	gpa_return_false_if_fail (config->printer);
	gpa_return_false_if_fail (gpa_node_verify (config->printer));
	gpa_return_false_if_fail (config->settings);
	gpa_return_false_if_fail (gpa_node_verify (config->settings));

	return TRUE;
}

GPAConfig *
gpa_config_new_full (GPAPrinter *printer, GPASettings * settings)
{
	GPAConfig * config;

	g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);
	g_return_val_if_fail (GPA_IS_SETTINGS (settings), NULL);
	
	config = (GPAConfig *) gpa_node_new (GPA_TYPE_CONFIG, "GpaConfigRootNode");

	gpa_reference_set_reference (GPA_REFERENCE (config->printer),  GPA_NODE (printer));
	gpa_reference_set_reference (GPA_REFERENCE (config->settings), GPA_NODE (settings));
	
	gpa_node_reverse_children (GPA_NODE (config));

	return config;
}

GPAConfig *
gpa_config_new (void)
{
	GPAConfig *config = NULL;
	GPANode *printer;
	GPANode *settings = NULL;

	gpa_init ();
	
	printer = gpa_printer_get_default ();
	if (printer == NULL) {
		g_warning ("Could not get the default printer");
		goto gpa_config_new_error;
	}

	settings = gpa_printer_get_default_settings (GPA_PRINTER (printer));

	config = gpa_config_new_full (GPA_PRINTER (printer), GPA_SETTINGS (settings));

gpa_config_new_error:
	if (printer)
		gpa_node_unref (printer);
	
	return config;
}

static GPANode *
gpa_config_duplicate (GPANode *node)
{
	GPAConfig *config = NULL, *new;
	GPANode *settings = NULL;

	config = GPA_CONFIG (node);

	settings = gpa_node_duplicate 
		(GPA_REFERENCE_REFERENCE(config->settings));

	new = gpa_config_new_full 
		(GPA_PRINTER (GPA_REFERENCE_REFERENCE(config->printer)), 
		 GPA_SETTINGS (settings));

	return GPA_NODE (new);
}


gchar *
gpa_config_to_string (GPAConfig *config, guint flags)
{
	GPANode *printer;
	GPANode *settings;
	xmlDocPtr doc;
	xmlNodePtr root, node;
	xmlChar *xml_str;
	gchar *str;
	gint size;

	g_return_val_if_fail (config != NULL, NULL);
	g_return_val_if_fail (GPA_IS_CONFIG (config), NULL);
	g_return_val_if_fail (config->printer != NULL, NULL);
	g_return_val_if_fail (config->settings != NULL, NULL);

	printer  = GPA_REFERENCE_REFERENCE (config->printer);
	settings = GPA_REFERENCE_REFERENCE (config->settings);

	g_return_val_if_fail (GPA_IS_PRINTER  (printer),  NULL);
	g_return_val_if_fail (GPA_IS_SETTINGS (settings), NULL);

	doc = xmlNewDoc ("1.0");
	root = xmlNewDocNode (doc, NULL, "GnomePrintConfig", NULL);
	xmlSetProp (root, "Version", "2.1");
	xmlSetProp (root, "LibgnomeprintVersion", VERSION);
	xmlSetProp (root, "SelectedSettings", gpa_node_id (settings));
	xmlDocSetRootElement (doc, root);

	node = gpa_settings_to_tree (GPA_SETTINGS (settings));

	xmlAddChild (root, node);

	xmlDocDumpFormatMemory	(doc, &xml_str, &size, TRUE);
	str = g_strndup (xml_str, size);
	xmlFree (xml_str);

	xmlFreeDoc (doc);

	return str;
}

GPAConfig *
gpa_config_from_string (const gchar *str, guint flags)
{
	GPASettings *new_settings = NULL;
	GPASettings *settings = NULL;
	GPAPrinter *printer = NULL;
	GPAConfig *config = NULL;
	GPAModel *model = NULL;
	GPANode *child;
	xmlDocPtr doc = NULL;
	xmlNodePtr tree, node;
	xmlChar *version, *settings_id, *printer_id, *model_id;

	version = settings_id = printer_id = model_id = NULL;

	gpa_init ();

	if (!str || !str[0])
		goto config_from_string_done;
	
	doc = xmlParseDoc ((char*) str);
	if (!doc) {
		g_warning ("Could not parse GPAConfig from string");
		goto config_from_string_done;
	}

	tree = doc->xmlRootNode;
	if (strcmp (tree->name, "GnomePrintConfig")) {
		g_warning ("Root node should be <GnomePrintConfig>, node found is <%s>", tree->name);
		goto config_from_string_done;
	}
	
	version = xmlGetProp (tree, "Version");
	if (!version || strcmp (version, "2.1")) {
		g_warning ("Invalid GnomePrintConfig version");
		goto config_from_string_done;
	}

	settings_id = xmlGetProp (tree, "SelectedSettings");
        if (!settings_id) {
		g_warning ("Settings ID not found, invalid GnomePrintConfig");
		goto config_from_string_done;
	}

	node = tree->xmlChildrenNode;
	for (;node != NULL; node = node->next) {
		xmlChar *child_id;
		
		if (!node->name)
			continue;
		
		if (strcmp (node->name, "Settings"))
			continue;

		child_id = xmlGetProp (node, "Id");

		if (!child_id)
			continue;

		if (strcmp (child_id, settings_id)) {
			my_xmlFree (child_id);
			continue;
		}

		my_xmlFree (child_id);
		break;
	}
	if (!node) {
		g_warning ("Could not find the selected settings in the settings list");
		goto config_from_string_done;
	}

	model_id   = xmlGetProp (node, "Model");
	printer_id = xmlGetProp (node, "Printer");
	if (!model_id || !printer_id || !model_id[0] || !printer_id[0]) {
		g_warning ("Model or Printer id missing or invalid from GnomePrintConfig");
		goto config_from_string_done;
	}
	
	model = (GPAModel *) gpa_model_hash_lookup (model_id);
	if (!model) {
		/* Right now we just drop the old config, this will happen when a printer
		 * is removed from the system or if we are loading a seralized GnomePrintConfig
		 * from another box (embedded in a file for example), so settings will not
		 * survive a printer or host change. However, we be a lot smarter about this
		 * in the future (Chema)
		 */
		g_print ("Model not found, discarding config\n");
		goto config_from_string_done;
	}

	printer = (GPAPrinter *) gpa_printer_get_by_id (printer_id);
	if (!printer) {
		/* ditto (Chema)
		 */
		g_print ("Printer not found, discarding config\n");
		goto config_from_string_done;
	}
	
	new_settings = (GPASettings *) gpa_settings_new_from_model_and_tree (GPA_NODE (model), node);
	if (!new_settings) {
		g_warning ("Could not create settings from model and tree\n");
		goto config_from_string_done;
	}

	settings = (GPASettings *) gpa_printer_get_settings_by_id (printer, gpa_node_id (GPA_NODE (new_settings)));
	if (!settings)
		settings = (GPASettings *) gpa_printer_get_default_settings (printer);
	
	child = gpa_node_get_child (GPA_NODE (settings), NULL);
	while (child) {
		GPANode *key;
		key = gpa_node_lookup (GPA_NODE (new_settings), gpa_node_id (child));
		if (key) {
			gpa_key_merge_from_key (GPA_KEY (child), GPA_KEY (key));
			gpa_node_unref (key);
		}
		child = child->next;
	}
		
	config = gpa_config_new_full (printer, settings);

	gpa_node_unref (GPA_NODE (new_settings));
	gpa_node_unref (GPA_NODE (printer));
	gpa_node_unref (GPA_NODE (settings));
	new_settings = NULL;
	settings     = NULL;
	printer      = NULL;
	
config_from_string_done:
	my_xmlFree (version);
	my_xmlFree (settings_id);
	my_xmlFree (model_id);
	my_xmlFree (printer_id);
	my_xmlFreeDoc (doc);

	if (!config) {
		my_gpa_node_unref (GPA_NODE (printer));
		my_gpa_node_unref (GPA_NODE (model));
		my_gpa_node_unref (GPA_NODE (settings));
		config = gpa_config_new ();
	}

	return config;
}
