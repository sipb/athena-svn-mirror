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
 *  Copyright (C) 2000-2003 Ximian, Inc.
 *
 */

#include "config.h"

#include <locale.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#include <gmodule.h>
#include <glibconfig.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include "gpa-utils.h"
#include "gpa-reference.h"
#include "gpa-settings.h"
#include "gpa-model.h"
#include "gpa-printer.h"
#include "gpa-root.h"

typedef struct _GPAPrinterClass GPAPrinterClass;
struct _GPAPrinterClass {
	GPANodeClass node_class;
};

typedef struct _GpaModuleInfo GpaModuleInfo;
struct _GpaModuleInfo {
	GPAList *  (*printer_list_append) 
	(GPAList *printers, const gchar* path);
	GPAList *printers;
};

static void gpa_printer_class_init (GPAPrinterClass *klass);
static void gpa_printer_init (GPAPrinter *printer);

static void gpa_printer_finalize (GObject *object);

static gboolean  gpa_printer_verify    (GPANode *node);
static guchar  * gpa_printer_get_value (GPANode *node);
static GPANode * gpa_printer_new_from_file (const gchar *file);

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

	node_class->verify    = gpa_printer_verify;
	node_class->get_value = gpa_printer_get_value;
}

static void
gpa_printer_init (GPAPrinter *printer)
{
	printer->name     = NULL;
	printer->model    = NULL;
	printer->settings = NULL;
	printer->is_complete = FALSE;
	printer->module_path = NULL;
}

static void
gpa_printer_finalize (GObject *object)
{
	GPAPrinter *printer;

	printer = GPA_PRINTER (object);

	my_g_free (printer->name);
	gpa_node_detach_unref (printer->settings);
	gpa_node_detach_unref (printer->model);
	printer->name     = NULL;
	printer->settings = NULL;
	printer->model    = NULL;
	printer->is_complete = FALSE;
	my_g_free (printer->module_path);
	printer->module_path = NULL;
	

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gpa_printer_load_data (GPAPrinter *printer)
{
	GpaModuleInfo info;
	GModule *handle  = NULL;
	gboolean (*init) (GpaModuleInfo *info);
	void (*load_data) (GPAPrinter *printer);

	if (printer->is_complete)
		return;

	g_return_if_fail (printer->module_path != NULL);

	if ((handle = printer->module_handle) != NULL &&
	     g_module_symbol (handle, "gpa_module_init", 
			      (gpointer*) &init) &&
	     g_module_symbol (handle, "gpa_module_load_data", 
			      (gpointer*) &load_data)) {
		if (init (&info))
			load_data (printer);
		printer->module_handle = handle;
	} else
		g_warning ("gpa_module_load_data cannot be retrieved from "
			   "module %s", printer->module_path);

	return;
}


static gboolean
gpa_printer_verify (GPANode *node)
{
	GPAPrinter *printer;

	printer = GPA_PRINTER (node);

	gpa_return_false_if_fail (printer->name);

	if (!printer->is_complete)
		return TRUE;

	gpa_return_false_if_fail (printer->settings);
	gpa_return_false_if_fail (gpa_node_verify (printer->settings));
	gpa_return_false_if_fail (printer->model);
	gpa_return_false_if_fail (gpa_node_verify (printer->model));
	
	return TRUE;
}

guchar *
gpa_printer_get_value (GPANode *node)
{
	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GPA_IS_PRINTER (node), NULL);

	return g_strdup (GPA_PRINTER (node)->name);
}

/**
 * gpa_printer_new_from_tree:
 * @tree: The xml tree where to create the printer from
 * 
 * Create a GPAPrinter form an xml tree.
 * 
 * Return Value: a newly created GPAPrinter or NULL on error
 **/
static GPANode *
gpa_printer_new_from_tree (xmlNodePtr tree)
{
	xmlNodePtr node;
	GPANode *settings, *printer, *model;
	xmlChar *name, *id, *version;
	const gchar *lang;

	g_return_val_if_fail (tree != NULL, NULL);
	g_return_val_if_fail (tree->name != NULL, NULL);

	settings = printer = model = NULL;
	name = id = version = NULL;
	
	if (strcmp (tree->name, "Printer")) {
		g_warning ("Base node is <%s>, should be <Printer>", tree->name);
		goto gpa_printer_new_from_tree_error;
	}
	
	id = xmlGetProp (tree, "Id");
	if (!id) {
		g_warning ("Printer node does not have Id, could not load printer");
		goto gpa_printer_new_from_tree_error;
	}

	version = xmlGetProp (tree, "Version");
	if (!version || strcmp (version, "1.0")) {
		g_warning ("Wrong printer version \"%s\" should be \"1.0\" "
			   "for printer \"%s\"", version, id);
		goto gpa_printer_new_from_tree_error;
	}

#ifdef HAVE_LC_MESSAGES
	lang = setlocale (LC_MESSAGES, NULL);
#endif
	node = tree->xmlChildrenNode;
	for (; node != NULL; node = node->next) {

		if (strcmp (node->name, "Name") == 0) {
			xmlChar *node_lang;
			node_lang = xmlNodeGetLang (node);
			if (node_lang && lang && !strcmp (lang, node_lang)) {
				my_xmlFree (name);
				name = xmlNodeGetContent (node);
			}
			if (node_lang == NULL && name == NULL) {
				name = xmlNodeGetContent (node);
			}
			xmlFree (node_lang);
			continue;
		}

		if (strcmp (node->name, "Model") == 0) {
			xmlChar *model_id = xmlNodeGetContent (node);
			model = gpa_model_get_by_id (model_id, FALSE);
			my_xmlFree (model_id);
			continue;
		}
		
		if (strcmp (node->name, "Settings") == 0) {
			/* We don't support multiple settings per printer yet */
			g_assert (settings == NULL);
			if (!model) {
				g_warning ("<Model> node should come before <Settings> (\"%s\")", id);
				continue;
			}
			settings = gpa_settings_new_from_model_and_tree (model, node);
			continue;
		}

	}

	if (!name || !name[0]) {
		g_warning ("Invalid or missing <Name> for printer \"%s\"", id);
		goto gpa_printer_new_from_tree_error;
	}
	if (!model) {
		g_warning ("Invalid or missing <Model> for printer \"%s\"\n", id);
		goto gpa_printer_new_from_tree_error;
	}
	if (!settings) {
		g_warning ("Invalid or missing <Settings> for printer \"%s\"\n", id);
		goto gpa_printer_new_from_tree_error;
	}

	printer = gpa_printer_new (id, name, GPA_MODEL (model), GPA_SETTINGS (settings));

gpa_printer_new_from_tree_error:
	my_xmlFree (name);
	my_xmlFree (id);
	my_xmlFree (version);

	if (!printer) {
		my_gpa_node_unref (settings);
		my_gpa_node_unref (model);
	}
	
	return printer;
}

/**
 * gpa_printer_new_from_file:
 * @file: 
 * 
 * Load a new printer from @filename, file should contain a XML description
 * 
 * Return Value: 
 **/
static GPANode *
gpa_printer_new_from_file (const gchar *file)
{
	GPANode *printer = NULL;
	xmlDocPtr doc;
	xmlNodePtr node;

	doc = xmlParseFile (file);
	if (!doc) {
		g_warning ("Could not parse %s\n", file);
		return NULL;
	}

	node = doc->xmlRootNode;
	printer = gpa_printer_new_from_tree (node);
	xmlFreeDoc (doc);

	if (!printer || !gpa_node_verify (printer)) {
		g_warning ("Could not load printer from %s", file);
		printer = NULL;
	}
	
	return printer;
}

/**
 * gpa_printer_list_load_from_module:
 * @path: 
 * 
 * Load printers from a module
 **/
static gboolean
gpa_printer_list_load_from_module (GPAList *printers, const gchar *path)
{
	GpaModuleInfo info;
	GModule *handle;
	gboolean (*init) (GpaModuleInfo *info);
	gint retval = FALSE;

	handle = g_module_open (path, G_MODULE_BIND_LAZY);
	if (!handle) {
		g_warning ("Can't g_module_open %s\n", path);
	        return retval;
	}

	if (!g_module_symbol (handle, "gpa_module_init", (gpointer*) &init)) {
		g_warning ("Error. Module %s does not contains an init function\n", path);
		goto module_error;
	}
	
	if (!(init) (&info)) {
		g_warning ("Could not initialize module %s\n", path);
		goto module_error;
	}

	(info.printer_list_append) (printers, path);
	retval = TRUE;
	
module_error:
	g_module_close (handle);

	return retval;
}

/**
 * gpa_printer_list_load_from_module_dir:
 * @list: 
 * @dir_path: 
 * 
 * Loads the printers from the gnome-print-modules. We load the module and find
 * for the gpa_printer_append symbol. If found, we asume this module is ok and we
 * call the function so that we get the printers from it. Modules append to
 * the list of printers passed to them the printers, they can steal the default
 * priner themselves, at this point we've set the default printer from the GNOME
 * as GENERIC postscript
 **/
static gboolean
gpa_printer_list_load_from_module_dir (GPAList *printers, const gchar *dir_path)
{
	struct dirent *entry;
	DIR *dir;
	gint ext_len = strlen (G_MODULE_SUFFIX);
	g_assert (ext_len > 0);

	if (!g_module_supported ()) {
		g_warning ("g_module is not supported on this platform an thus we can't "
			   "load dynamic printers\n");
		return FALSE;
	}
	
	dir = opendir (dir_path);
	if (!dir) {
		/* Not an error. since modules are optional */
		return TRUE;
	}

	while ((entry = readdir (dir)) != NULL) {
		gchar *path;
		gint len;

		len = strlen (entry->d_name);

		if (len < ext_len + 2) /* 2 = one char + 1 for '.'*/
			continue;

		if (*(entry->d_name + len - ext_len - 1) != '.' || 
		    strcmp (entry->d_name + len - ext_len, G_MODULE_SUFFIX))
			continue;
		
		path = g_build_filename (dir_path, entry->d_name, NULL);
		gpa_printer_list_load_from_module (printers, path);
		g_free (path);
	}
	closedir (dir);
	
	return TRUE;
}

/**
 * gpa_printer_list_load_from_dir:
 * @printers: 
 * @dir_name: The path where to load printers from
 * 
 * Loads printers from xml files inside @dir_name
 * 
 * Return Value: FALSE on error
 **/
static gboolean
gpa_printer_list_load_from_dir (GPAList *printers, const gchar *dir_name)
{
	struct dirent *entry;
	DIR *dir;

	dir = opendir (dir_name);
	if (!dir)
		return FALSE;

	while ((entry = readdir (dir))) {
		GPANode *printer;
		gchar *file;
		gint len;

		len = strlen (entry->d_name);
		if (len < 5)
			continue;
		
		if (strcmp (entry->d_name + len - 4, ".xml"))
			continue;

		file = g_build_filename (dir_name, entry->d_name, NULL);
		printer = gpa_printer_new_from_file (file);
		g_free (file);

		if (!printer)
			continue;

		gpa_list_prepend (printers, printer);

		if (strcmp (GPA_NODE_ID (printer), "GENERIC") == 0)
		    gpa_list_set_default (printers, printer);
	}
	closedir (dir);

	return TRUE;
}


/**
 * gpa_printer_list_load:
 * @void: 
 * 
 * Loads the configured printers, should only be called once per
 * process. Use gpa_root_get_printers to get the list of printers.
 * 
 * Return Value: a GPAList node with childs of type GPAPrinter
 **/
GPAList *
gpa_printer_list_load (void)
{
	GPAList *printers;
	GPANode *p;

	if (gpa_root && gpa_root->printers != NULL) {
		g_warning ("gpa_printer_list_load should only be called once");
		return NULL;
	}

	printers = gpa_list_new (GPA_TYPE_PRINTER, "Printers", TRUE);

	gpa_printer_list_load_from_dir        (printers, GPA_DATA_DIR "/printers");
	gpa_printer_list_load_from_module_dir (printers, GPA_MODULES_DIR);

	if (GPA_NODE (printers)->children == NULL) {
		g_warning ("Could not load any Printer. Check your libgnomeprint installation\n");
		gpa_node_unref (GPA_NODE (printers));
		return NULL;
	}

	p = gpa_node_get_child (GPA_NODE (printers), NULL);
	while (p) {
		gpa_printer_get_default_settings (GPA_PRINTER (p));
		p = gpa_node_get_child (GPA_NODE (printers), p);
	}
	gpa_list_reverse (printers);

	return printers;
}


/**
 * gpa_printer_get_default:
 * @void: 
 * 
 * Get the default printer on the system. If no defaults are
 * set, the sets it to the first printer on the list.
 * 
 * Return Value: a refcounted default printer, NULL on error or
 *               if no printers are loaded
 **/
GPANode *
gpa_printer_get_default (void)
{
	if (!gpa_root ||
	    !gpa_root->printers ||
	    !gpa_root->printers->children) {
		g_warning ("Global printer list not loaded");
		return NULL;
	}

	return gpa_list_get_default (GPA_LIST (gpa_root->printers));
}

/**
 * gpa_printer_get_by_id:
 * @id: 
 * 
 * Get a printer from the global printer list by id
 * 
 * Return Value: a refcounted printer node, NULL if the printer
 *               was not found or error
 **/
GPANode *
gpa_printer_get_by_id (const guchar *id)
{
	GPANode *child = NULL;

	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (*id != '\0', NULL);

	if (!gpa_root || !gpa_root->printers)
		return NULL;

	child = GPA_NODE (gpa_root->printers)->children;
	for (; child != NULL; child = child->next) {
		if (GPA_NODE_ID_COMPARE (child, id))
			break;
	}

	if (child)
		gpa_node_ref (child);

	return child;
}

void
gpa_printer_set_polling (GPAPrinter *printer, gboolean poll)
{
	void (*set_polling) (GPAPrinter *printer, gboolean poll);
	
	if (!printer->module_handle)
		return;

	if (printer->polling == poll)
		return;
	
	if (!g_module_symbol (printer->module_handle, "gpa_module_polling", 
			      (gpointer*) &set_polling))
		return;
	set_polling (printer, poll);
	printer->polling = poll;
}

/**
 * gpa_printer_get_default_settings:
 * @printer: 
 * 
 * Returns a refcounted GPANode * of the default settings of @printer
 * 
 * Return Value: 
 **/
GPANode *
gpa_printer_get_default_settings (GPAPrinter *printer)
{
	GPANode *child = NULL;
	
	g_return_val_if_fail (printer != NULL, NULL);
	g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);

	gpa_printer_load_data (printer);

	if (printer->is_complete)
		child = gpa_list_get_default (GPA_LIST (printer->settings));

	return child;
}

/**
 * gpa_printer_new_stub:
 * @id: 
 * @name: 
 *
 * Create a new printer node and set it up
 * 
 * Return Value: the newly created printer, NULL on error
 **/
GPANode *
gpa_printer_new_stub (const gchar *id, const gchar *name, 
		      const gchar *path)
{
	GPAPrinter *printer;
	GPANode *check;

	g_return_val_if_fail (id && id[0], NULL);
	g_return_val_if_fail (name && name[0], NULL);
	g_return_val_if_fail (gpa_initialized (), NULL);

	check = gpa_printer_get_by_id (id);
	if (check) {
		g_warning ("Can't create printer \"%s\" because the id \"%s\" is already used", name, id);
		gpa_node_unref (check);
		return NULL;
	}


	printer = (GPAPrinter *) gpa_node_new (GPA_TYPE_PRINTER, id);
	printer->name     = g_strdup (name);

	if (path != NULL) {
		printer->module_path = g_strdup (path);
		printer->module_handle = g_module_open (path, G_MODULE_BIND_LAZY);
	}
	printer->is_complete = FALSE;

	return (GPANode *) printer;	
}

gboolean  
gpa_printer_complete_stub (GPAPrinter *printer, 
			   GPAModel *model, GPASettings *settings)
{
	GPAList *list;
	GPAList *state;

	g_return_val_if_fail (printer->is_complete != TRUE, FALSE);
	g_return_val_if_fail (model != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_MODEL (model), FALSE);
	g_return_val_if_fail (settings != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_SETTINGS (settings), FALSE);
	g_return_val_if_fail (gpa_initialized (), FALSE);

	list = gpa_list_new (GPA_TYPE_SETTINGS, "Settings", TRUE);
	state = gpa_list_new (GPA_TYPE_NODE, "State", TRUE);
	
	printer->model    = gpa_node_attach (GPA_NODE (printer),
					     GPA_NODE (gpa_reference_new (GPA_NODE (model), "Model")));
	printer->settings = gpa_node_attach (GPA_NODE (printer),
					     GPA_NODE (list));
	printer->state = gpa_node_attach (GPA_NODE (printer),
					  GPA_NODE (state));

	printer->is_complete = TRUE;

	gpa_node_reverse_children (GPA_NODE (printer));

	gpa_list_prepend     (list, GPA_NODE (settings));
	gpa_list_set_default (list, GPA_NODE (settings));

	settings->printer = gpa_reference_new (GPA_NODE (printer), "Printer");
	/* We sink the model reference because we have a GPAReference to it
	 * we take ownership of the settings, so we don't unref them
	 */
	gpa_node_unref (GPA_NODE (model));

	return TRUE;
}



/**
 * gpa_printer_new:
 * @id: 
 * @name: 
 * @model: 
 * @settings: 
 *
 * Create a new printer node and set it up
 * We consume the refcount of @model & @setting so if you need
 * them, you should ref them before calling this function
 * 
 * Return Value: the newly created printer, NULL on error
 **/
GPANode *
gpa_printer_new (const gchar *id, const gchar *name, GPAModel *model, GPASettings *settings)
{
	GPAPrinter *printer;

	printer = GPA_PRINTER (gpa_printer_new_stub (id, name, NULL));
	if (printer == NULL)
		return NULL;
	
	if (!gpa_printer_complete_stub (printer, model, settings)) {
		gpa_node_unref ((GPANode *) printer);
		printer = NULL;
		return NULL;
	}

	if (!gpa_printer_verify ((GPANode *) printer)) {
		g_warning ("The newly created printer %s could not be verified", id);
		gpa_node_unref ((GPANode *) printer);
		printer = NULL;
		return NULL;
	}
	
	return (GPANode *) printer;
}


GPANode *
gpa_printer_get_settings_by_id (GPAPrinter *printer, const guchar *id)
{
	GPANode *child;

	g_return_val_if_fail (printer != NULL, NULL);
	g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);
	g_return_val_if_fail (id && id[0], NULL);

	gpa_printer_load_data (printer);

	g_assert (printer->settings);
	child = printer->settings->children;
	while (child) {
		if (GPA_NODE_ID_COMPARE (child, id))
			break;
		child = gpa_node_get_child (printer->settings, child);
	}
	if (child)
		gpa_node_ref (child);
	
	return child;
}

GPANode *
gpa_printer_get_state (GPAPrinter *printer)
{
	return printer->state;
}

GPANode *
gpa_printer_get_state_by_id (GPAPrinter *printer, const guchar *id)
{
	GPANode *child;

	g_return_val_if_fail (printer != NULL, NULL);
	g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);
	g_return_val_if_fail (id && id[0], NULL);

	gpa_printer_load_data (printer);

	g_assert (printer->state);
	child = gpa_node_get_child (printer->state, NULL);
	while (child) {
		if (GPA_NODE_ID_COMPARE (child, id))
			break;
		child = gpa_node_get_child (printer->state, child);
	}
	if (child)
		gpa_node_ref (child);
	
	return child;
}
