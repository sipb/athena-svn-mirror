/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-cups.c: A cups backend thingy
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useoful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors:
 *    Dave Camp <dave@ximian.com>
 *    Chema Celorio <chema@celorio.com>
 *
 *  Copyright 2002  Ximian, Inc. and authors
 *
 */

#include <config.h>
#include <glib.h>
#include <gmodule.h>

#include <libgnomeprint/gnome-print-module.h>
#include <libgnomeprint/gpa/gpa-model.h>
#include <libgnomeprint/gpa/gpa-printer.h>
#include <libgnomeprint/gpa/gpa-option.h>
#include <libgnomeprint/gpa/gpa-settings.h>

#include <cups/cups.h>
#include <cups/ppd.h>

/* Argument order: id, name */

xmlChar *model_xml_template = 
"<?xml version=\"1.0\"?>"
"<Model Id=\"%s\" Version=\"1.0\">"
"  <Name>%s</Name>"
"  <ModelVersion>0.0.1</ModelVersion>"
"  <Options>"
"    <Option Id=\"Transport\">"
"      <Option Id=\"Backend\" Type=\"List\" Default=\"CUPS\">"
"        <Item Id=\"CUPS\">"
"          <Name>CUPS</Name>"
"          <Key Id=\"Module\" Value=\"libgnomeprintcups.so\"/>"
"        </Item>"
"      </Option>"
"    </Option>"
"    <Option Id=\"Output\">"
"      <Option Id=\"Media\">"
"        <Option Id=\"PhysicalOrientation\" Type=\"List\" Default=\"R0\">"
"          <Fill Ref=\"Globals.Media.PhysicalOrientation\"/>"
"        </Option>"
"        <Key Id=\"Margins\">"
"          <Key Id=\"Left\" Value=\"0\"/>"
"          <Key Id=\"Right\" Value=\"0\"/>"
"          <Key Id=\"Top\" Value=\"0\"/>"
"          <Key Id=\"Bottom\" Value=\"0\"/>"
"        </Key>"
"      </Option>"
"    </Option>"
"    <Option Id=\"Document\">"
"      <Option Id=\"Page\">"
"        <Option Id=\"Layout\" Type=\"List\" Default=\"Plain\">"
"          <Fill Ref=\"Globals.Document.Page.Layout\"/>"
"        </Option>"
"        <Option Id=\"LogicalOrientation\" Type=\"List\" Default=\"R0\">"
"          <Fill Ref=\"Globals.Document.Page.LogicalOrientation\"/>"
"        </Option>"
"        <Option Id=\"Margins\">"
"          <Option Id=\"Left\" Type=\"String\" Default=\"2 cm\"/>"
"          <Option Id=\"Right\" Type=\"String\" Default=\"2 cm\"/>"
"          <Option Id=\"Top\" Type=\"String\" Default=\"3 cm\"/>"
"          <Option Id=\"Bottom\" Type=\"String\" Default=\"3 cm\"/>"
"        </Option>"
"      </Option>"
"      <Option Id=\"PreferedUnit\" Type=\"String\" Default=\"cm\"/>"
"      <Option Id=\"Name\" Type=\"String\" Default=\"\"/>"
"    </Option>"
#if 0
"    <Option Id=\"Icon\">"
"      <Option Id=\"Filename\" Type=\"String\" Default=\"" DATADIR "/pixmaps/nautilus/default/i-printer.png\"/>"
"    </Option>"
#endif
"  </Options>"
"</Model>";

static GPANode *
gpa_model_new_from_xml (char *string)
{
	GPANode *model;
	xmlNodePtr root;
	xmlDocPtr doc;
	
	doc = xmlParseDoc (string);
	if (!doc) {
		g_warning ("Could not parse model xml");
		return NULL;
	}
	
	root = doc->xmlRootNode;
	model = gpa_model_new_from_tree (root);
	
	xmlFreeDoc (doc);
	
	return model;
}

static char *
get_paper_text (ppd_file_t *ppd, ppd_size_t *size)
{
	/* This is dumb and slow and ugly and crappy and I hate myself. (Dave)*/
	int i;
	
	for (i = 0; i < ppd->num_groups; i++) {
		ppd_group_t *group = &ppd->groups[i];
		int j;
		for (j = 0; j < group->num_options; j++) {
			ppd_option_t *option = &group->options[j];
			if (!strcmp (option->keyword, "PageSize")) {
				int k;
				for (k = 0; k < option->num_choices; k++) {
					if (!strcmp (option->choices[k].choice,
						     size->name)) {
						return g_strdup (option->choices[k].text);
					}
				}
			}
		}
	}
	return g_strdup (size->name);
}

static GPANode *
load_paper_sizes (ppd_file_t *ppd, GPANode *parent)
{
	ppd_option_t *option;
	GPANode *node;
	int i;

	option = ppdFindOption (ppd, "PageSize");

	if (!option)
		/* Should we not add the PageSize if we can't find it
		 * or create a default one w/ A4 & USLetter? (Chema)
		 */
		return NULL;
	
	node = gpa_option_list_new (parent, "PhysicalSize",
				    option->defchoice);
			
	for (i = 0; i < ppd->num_sizes; i++) {
		GPANode *size;
		gchar *height;
		gchar *width;
		gchar *paper_name;

		paper_name = get_paper_text (ppd, &ppd->sizes[i]);
		size = gpa_option_item_new (node, ppd->sizes[i].name, paper_name);
		g_free (paper_name);

		width  = g_strdup_printf ("%d", (int)ppd->sizes[i].width);
		height = g_strdup_printf ("%d", (int)ppd->sizes[i].length);

		gpa_option_key_new (size, "Width",  width);
		gpa_option_key_new (size, "Height", height);

		g_free (width);
		g_free (height);
	}

	gpa_node_reverse_children (node);

	return node;
}

static void
load_paper_sources (ppd_file_t *ppd, GPANode *parent)
{
	ppd_option_t *option;
	GPANode *node;
	int i;

	option = ppdFindOption (ppd, "InputSlot");
	if (!option)
		return;
	
	node = gpa_option_list_new (parent, "PaperSource",
				    option->defchoice);
	
	for (i = 0; i < option->num_choices; i++)
		gpa_option_item_new (node,
				     option->choices[i].choice,
				     option->choices[i].choice);
}

static GPAModel *
get_model (const gchar *printer, ppd_file_t *ppd)
{
	GPANode *media;
	GPANode *model;
	GPANode *output;
	char *xml;
	char *id;

	id = g_strdup_printf ("Cups-%s-%s", ppd->manufacturer, ppd->nickname);

	model = gpa_model_get_by_id (id, TRUE);
	
	if (model) {
		g_free (id);
		return GPA_MODEL (model);
	}

	xml = g_strdup_printf (model_xml_template, id, ppd->nickname);
	model = gpa_model_new_from_xml (xml);
	g_free (xml);

	output = gpa_node_lookup (model, "Options.Output");
	media  = gpa_node_lookup (model, "Options.Output.Media");

	load_paper_sizes   (ppd, media);
	load_paper_sources (ppd, output);

	gpa_node_unref (output);
	gpa_node_unref (media);

	g_free (id);
	
	return (GPAModel *)model;
}

static gboolean
append_printer (GPAList *printers_list, const char *name, gboolean is_default)
{
	GPANode *settings = NULL;
	GPANode *printer  = NULL;
	GPAModel *model    = NULL;
	const char *filename;
	ppd_file_t *ppd;
	gboolean retval = FALSE;

	filename = cupsGetPPD (name);
	ppd = ppdOpenFile (filename);
	if (ppd == NULL)
		/* See bug#: 102938 */
		return FALSE;

	model = get_model (name, ppd);
	if (model == NULL)
		goto append_printer_exit;

	settings = gpa_settings_new (model, "Default", "SettIdFromCups");
	if (settings == NULL)
		goto append_printer_exit;

	printer = gpa_printer_new (name, name, model, GPA_SETTINGS (settings));
	if (printer == NULL)
		goto append_printer_exit;

	if (gpa_node_verify (printer)) {
		gpa_list_prepend (printers_list, printer);
		if (is_default)
			gpa_list_set_default (printers_list, printer);
		retval = TRUE;
	}

 append_printer_exit:
	if (retval == FALSE) {
		g_warning ("The CUPS printer %s could not be created\n", name);

		my_gpa_node_unref (printer);
		my_gpa_node_unref (GPA_NODE (model));
		my_gpa_node_unref (settings);
	}

	if (ppd) {
		ppdClose (ppd);
		unlink (filename);
	}
	
	return retval;
}

static void
gnome_print_cups_printer_list_append (gpointer printers_list)
{
	gboolean is_default;
	const gchar *def;
	gchar **printers;
	gint num_printers;
	gint i;

	g_return_if_fail (printers_list != NULL);
	g_return_if_fail (GPA_IS_LIST (printers_list));
	
	num_printers = cupsGetPrinters (&printers);
	if (num_printers < 1)
		return;

	def = cupsGetDefault ();
	
	for (i = 0; i < num_printers; i ++) {
		is_default = (def && !strcmp (printers [i], def));
		append_printer (GPA_LIST (printers_list),
				printers [i], is_default);
	}
	
	for (i = 0; i < num_printers; i ++)
		free (printers[i]);

	free (printers);
	
	return;
}


/*  ------------- GPA init ------------- */
G_MODULE_EXPORT gboolean gpa_module_init (GpaModuleInfo *info);

G_MODULE_EXPORT gboolean
gpa_module_init (GpaModuleInfo *info)
{
	info->printer_list_append = gnome_print_cups_printer_list_append;
	return TRUE;
}
