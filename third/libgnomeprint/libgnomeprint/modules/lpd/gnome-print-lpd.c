/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-lpd.c: An lpd backend
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
 *   Andreas J. Guelzow <aguelzow@taliesin.ca>
 *
 *  Copyright 2004 Andreas J. Guelzow
 *
 */

#include <config.h>
#include <glib.h>
#include <gmodule.h>
#include <locale.h>
#include <string.h>

#include <libgnomeprint/gnome-print-module.h>
#include <libgnomeprint/gpa/gpa-model.h>
#include <libgnomeprint/gpa/gpa-printer.h>
#include <libgnomeprint/gpa/gpa-option.h>
#include <libgnomeprint/gpa/gpa-settings.h>
#include <libgnomeprint/gpa/gpa-state.h>
#include <libgnomeprint/gpa/gpa-utils.h>

#include "libgnomeprint/gnome-print-i18n.h"

/* Argument order: id  printer */

xmlChar *lpd_model_unknown_xml_template = 
"<?xml version=\"1.0\"?>"
"<Model Id=\"%s\" Version=\"1.0\">"
"  <Name>Unavailable PPD File</Name>"
"  <ModelVersion>0.0.1</ModelVersion>"
"  <Options>"
"    <Option Id=\"Transport\">"
"      <Option Id=\"Backend\" Type=\"List\" Default=\"LPD\">"
"        <Item Id=\"LPD\">"
"          <Name>LPD</Name>"
"          <Key Id=\"Module\" Value=\"libgnomeprint-lpr.so\"/>"
"          <Key Id=\"Printer\" Value=\"%s\"/>"
"        </Item>"
"      </Option>"
"    </Option>"
"    <Option Id=\"Output\">"
"      <Option Id=\"Media\">"
"        <Option Id=\"PhysicalSize\" Type=\"List\" Default=\"USLetter\">"
"          <Fill Ref=\"Globals.Media.PhysicalSize\"/>"
"        </Option>"
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
"      <Option Id=\"Job\">"
"        <Option Id=\"NumCopies\" Type=\"String\" Default=\"1\"/>"
"        <Option Id=\"NonCollatedCopiesHW\" Type=\"String\" Default=\"true\"/>"
"        <Option Id=\"CollatedCopiesHW\" Type=\"String\" Default=\"false\"/>"
"        <Option Id=\"Collate\" Type=\"String\" Default=\"false\"/>"
"        <Option Id=\"Duplex\" Type=\"String\" Default=\"true\"/>"
"        <Option Id=\"Tumble\" Type=\"String\" Default=\"false\"/>"
"        <Option Id=\"PrintToFile\" Type=\"String\" Default=\"false\" Locked=\"true\"/>"
"        <Option Id=\"FileName\" Type=\"String\" Default=\"output.ps\"/>"
"      </Option>"
"    </Option>"
"  </Options>"
"</Model>";

static gboolean
append_printer (GPAList *printers_list,  const char *name, 
		gboolean is_default)
{
	GPANode *settings = NULL;
	GPANode *printer  = NULL;
	GPANode *model    = NULL;
	gboolean retval = FALSE;
	char *xml;
	gchar *description = NULL;
	gchar *id = NULL;

	model = gpa_model_get_by_id ("LPD-unknown-unknown", TRUE);
	
	if (model == NULL) {
		xml = g_strdup_printf (lpd_model_unknown_xml_template, 
				       "LPD-unknown-unknown", name);
		model = gpa_model_new_from_xml_str (xml);
		g_free (xml);
	}

	if (model == NULL)
		return FALSE;

	settings = gpa_settings_new (GPA_MODEL (model), "Default", "SettIdFromLPD");
	if (settings == NULL) 
		goto append_printer_exit;

	description = g_strdup_printf (_("%s (via lpr)"), name);
	id = g_strdup_printf ("LPD::%s", name);
	printer = gpa_printer_new (id, description, GPA_MODEL (model), GPA_SETTINGS (settings));
	g_free (description);
	g_free (id);

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
		g_warning ("The LPD printer %s could not be created\n", name);

		my_gpa_node_unref (printer);
		my_gpa_node_unref (GPA_NODE (model));
		my_gpa_node_unref (settings);
	}

	return retval;
}

static void
gnome_print_lpd_printer_list_append (gpointer printers_list, 
				      const gchar *path)
{
	gchar *contents;

	if (!g_file_test ("/etc/printcap", G_FILE_TEST_IS_REGULAR))
		return;

	if (g_file_get_contents ("/etc/printcap", &contents,
				 NULL,NULL)) {
		gchar **lines = g_strsplit (contents, "\n", 0);
		gchar **this_line;

		for (this_line = lines; 
		     this_line != NULL && *this_line != NULL;
		     this_line++) {
			gchar **tokens;
			gchar **printers;

			g_strstrip(*this_line);
			if (g_str_has_prefix (*this_line, 
			"# This file was automatically generated by cupsd(8)"))
				break;
			if (g_str_has_prefix (*this_line, "#"))
				continue;
			if (strlen (*this_line) == 0)
				continue;
			tokens = g_strsplit (*this_line, ":", 2);
			if (tokens == NULL || *tokens == NULL)
				continue;
			printers = g_strsplit (*tokens, "|", 0);
			if (printers != NULL && *printers != NULL) {
				append_printer 
					(GPA_LIST (printers_list), 
					 *printers, FALSE);
				g_strfreev (printers);
			}
			g_strfreev (tokens);
		}
		g_strfreev (lines);
		g_free (contents);
	}
	return;
}





/*  ------------- GPA init ------------- */
G_MODULE_EXPORT gboolean gpa_module_init (GpaModuleInfo *info);

G_MODULE_EXPORT gboolean
gpa_module_init (GpaModuleInfo *info)
{
	info->printer_list_append = gnome_print_lpd_printer_list_append;
	return TRUE;
}
