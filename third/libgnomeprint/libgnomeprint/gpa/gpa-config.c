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
 *  Authors :
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 2000-2001 Ximian, Inc.
 *
 */

#define __GPA_CONFIG_C__

#include <string.h>
#include "gpa-utils.h"
#include "gpa-reference.h"
#include "gpa-printer.h"
#include "gpa-settings.h"
#include "gpa-root.h"
#include "gpa-config.h"

/* GPAConfig */

static void gpa_config_class_init (GPAConfigClass *klass);
static void gpa_config_init (GPAConfig *config);

static void gpa_config_finalize (GObject *object);

static GPANode *gpa_config_duplicate (GPANode *node);
static gboolean gpa_config_verify (GPANode *node);
static guchar *gpa_config_get_value (GPANode *node);
static GPANode *gpa_config_get_child (GPANode *node, GPANode *ref);
static GPANode *gpa_config_lookup (GPANode *node, const guchar *path);
static void gpa_config_modified (GPANode *node, guint flags);

/* Helpers */

static gboolean gpa_config_printer_set_value (GPAReference *reference, const guchar *value, gpointer data);

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
	node_class->verify = gpa_config_verify;
	node_class->get_value = gpa_config_get_value;
	node_class->get_child = gpa_config_get_child;
	node_class->lookup = gpa_config_lookup;
	node_class->modified = gpa_config_modified;
}

static void
gpa_config_init (GPAConfig *config)
{
	config->globals = NULL;
	config->printer = NULL;
	config->settings = NULL;
}

static void
gpa_config_finalize (GObject *object)
{
	GPAConfig *config;

	config = (GPAConfig *) object;

	config->globals = gpa_node_detach_unref (GPA_NODE (config), GPA_NODE (config->globals));
	config->printer = gpa_node_detach_unref (GPA_NODE (config), GPA_NODE (config->printer));
	config->settings = gpa_node_detach_unref (GPA_NODE (config), GPA_NODE (config->settings));

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
gpa_config_verify (GPANode *node)
{
	GPAConfig *config;

	config = GPA_CONFIG (node);

	if (!config->globals)
		return FALSE;
	if (!gpa_node_verify (config->globals))
		return FALSE;
	if (!config->printer)
		return FALSE;
	if (!gpa_node_verify (config->printer))
		return FALSE;
	if (!config->settings)
		return FALSE;
	if (!gpa_node_verify (GPA_NODE (config->settings)))
		return FALSE;

	return TRUE;
}

static guchar *
gpa_config_get_value (GPANode *node)
{
	GPAConfig *config;

	config = GPA_CONFIG (node);

	return NULL;
}

static GPANode *
gpa_config_get_child (GPANode *node, GPANode *ref)
{
	GPAConfig *config;
	GPANode *child;

	config = GPA_CONFIG (node);

	child = NULL;
	if (ref == NULL) {
		child = config->globals;
	} else if (ref == config->globals) {
		child = GPA_NODE (config->printer);
	} else if (ref == GPA_NODE (config->printer)) {
		child = GPA_NODE (config->settings);
	}

	if (child)
		gpa_node_ref (child);

	return child;
}

static GPANode *
gpa_config_lookup (GPANode *node, const guchar *path)
{
	GPAConfig *config;
	GPANode *child;

	config = GPA_CONFIG (node);

	child = NULL;

	if (gpa_node_lookup_ref (&child, GPA_NODE (config->globals), path, "Globals"))
		return child;
	if (gpa_node_lookup_ref (&child, GPA_NODE (config->printer), path, "Printer"))
		return child;
	if (gpa_node_lookup_ref (&child, GPA_NODE (config->settings), path, "Settings"))
		return child;

	return NULL;
}

static void
gpa_config_modified (GPANode *node, guint flags)
{
	GPAConfig *config;

	config = GPA_CONFIG (node);

	if (config->globals && (GPA_NODE_FLAGS (config->globals) & GPA_MODIFIED_FLAG)) {
		gpa_node_emit_modified (config->globals, 0);
	}
	if (config->printer && (GPA_NODE_FLAGS (config->printer) & GPA_MODIFIED_FLAG)) {
		gpa_node_emit_modified (config->printer, 0);
	}
	if (config->settings && (GPA_NODE_FLAGS (config->settings) & GPA_MODIFIED_FLAG)) {
		gpa_node_emit_modified (GPA_NODE (config->settings), 0);
	}
}

GPANode *
gpa_config_new (void)
{
	GPAConfig *config;
	GPANode *globals;
	GPANode *printer;
	GPANode *settings;

	globals = GPA_NODE (gpa_root_get ());
	if (!globals) {
		g_warning ("Cannot read global configuration data");
		return NULL;
	}

	printer = gpa_printer_get_default ();
	if (printer) {
		GPANode *def;
		def = gpa_node_get_path_node (printer, "Settings.Default");
		if (def) {
			/* fixme: */
			settings = gpa_node_duplicate (GPA_REFERENCE_REFERENCE (def));
			gpa_node_unref (def);
		} else {
			settings = NULL;
		}
	} else {
		printer = gpa_reference_new_empty ();
		settings = gpa_settings_new_empty ("Default");
	}

	if (printer && settings) {
		config = (GPAConfig *) gpa_node_new (GPA_TYPE_CONFIG, NULL);
		config->globals = gpa_reference_new (globals);
		config->globals->parent = GPA_NODE (config);
		gpa_node_unref (globals);
		config->printer = gpa_reference_new (printer);
		g_signal_connect (G_OBJECT (config->printer), "set_value",
				  (GCallback) gpa_config_printer_set_value, config);
		config->printer->parent = GPA_NODE (config);
		gpa_node_unref (printer);
		config->settings = settings;
		config->settings->parent = GPA_NODE (config);
	} else {
		config = NULL;
		if (globals)
			gpa_node_unref (globals);
		if (printer)
			gpa_node_unref (printer);
		if (settings)
			gpa_node_unref (settings);
	}

	return GPA_NODE (config);
}

static gboolean
gpa_config_printer_set_value (GPAReference *reference, const guchar *value, gpointer data)
{
	GPAConfig *config;
	GPANode *printer;

	config = GPA_CONFIG (data);

	printer = gpa_printer_get_by_id (value);
	if (printer) {
		GPANode *def;
		def = gpa_printer_get_default_settings (GPA_PRINTER (printer));
		if (def) {
			gpa_reference_set_reference (GPA_REFERENCE (config->printer), printer);
			gpa_settings_copy (GPA_SETTINGS (config->settings), GPA_SETTINGS (def));
			gpa_node_unref (def);
		}
		gpa_node_unref (printer);
		return TRUE;
	}

	return FALSE;
}

static GPANode *
gpa_config_duplicate (GPANode *node)
{
	GPAConfig *config, *new;

	config = GPA_CONFIG (node);

	new = (GPAConfig *) gpa_node_new (GPA_TYPE_CONFIG, GPA_NODE_ID (node));

	if (config->globals)
		new->globals = gpa_node_attach (GPA_NODE (new), 
						gpa_node_duplicate (config->globals));
	if (config->printer)
		new->printer = gpa_node_attach (GPA_NODE (new), 
						gpa_node_duplicate (config->printer));
	if (config->settings)
		new->settings = gpa_node_attach (GPA_NODE (new), 
						 gpa_node_duplicate (config->settings));

	return GPA_NODE (new);
}

