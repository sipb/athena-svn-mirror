/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    Author: Ettore Perazzoli <ettore@helixcode.com>
*/

#include <config.h>
#include <gnome.h>
#include <bonobo.h>

#include "menubar.h"
#include "gtkhtml.h"
#include "control-data.h"
#include "properties.h"
#include "image.h"
#include "text.h"
#include "link.h"
#include "spell.h"
#include "table.h"
#include "template.h"

static void
undo_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	gtk_html_undo (cd->html);
}

static void
redo_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	gtk_html_redo (cd->html);
}

static void
cut_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	gtk_html_cut (cd->html);
}

static void
copy_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	gtk_html_copy (cd->html);
}

static void
paste_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	gtk_html_paste (cd->html);
}

static void
search_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	search (cd, FALSE);
}

static void
search_regex_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	search (cd, TRUE);
}

static void
search_next_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	search_next (cd);
}

static void
select_all_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	gtk_html_command (cd->html, "select-all");
}

static void
replace_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	replace (cd);
}

static void
insert_image_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	if (cd->properties_dialog)
		gtk_html_edit_properties_dialog_close (cd->properties_dialog);
	cd->properties_dialog = gtk_html_edit_properties_dialog_new (cd, TRUE, _("Insert"));
	gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
						   GTK_HTML_EDIT_PROPERTY_IMAGE, _("Image"),
						   image_insertion,
						   image_insert_cb,
						   image_close_cb);
	gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
						   GTK_HTML_EDIT_PROPERTY_LINK, _("Link"),
						   link_properties,
						   link_apply_cb,
						   link_close_cb);
	gtk_html_edit_properties_dialog_show (cd->properties_dialog);
}

static void
insert_link_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	if (cd->properties_dialog)
		gtk_html_edit_properties_dialog_close (cd->properties_dialog);

	cd->properties_dialog = gtk_html_edit_properties_dialog_new (cd, FALSE, _("Insert"));

	gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
						   GTK_HTML_EDIT_PROPERTY_TEXT, _("Text"),
						   text_properties,
						   text_apply_cb,
						   text_close_cb);
	gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
						   GTK_HTML_EDIT_PROPERTY_LINK, _("Link"),
						   link_properties,
						   link_apply_cb,
						   link_close_cb);

	gtk_html_edit_properties_dialog_show (cd->properties_dialog);
	gtk_html_edit_properties_dialog_set_page (cd->properties_dialog, GTK_HTML_EDIT_PROPERTY_LINK);
}

static void
insert_rule_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	if (cd->properties_dialog)
		gtk_html_edit_properties_dialog_close (cd->properties_dialog);

	cd->properties_dialog = gtk_html_edit_properties_dialog_new (cd, TRUE, _("Insert"));

	gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
						   GTK_HTML_EDIT_PROPERTY_RULE, _("Rule"),
						   rule_insert,
						   rule_insert_cb,
						   rule_close_cb);

	gtk_html_edit_properties_dialog_show (cd->properties_dialog);
}

void
insert_table (GtkHTMLControlData *cd)
{
	if (cd->properties_dialog)
		gtk_html_edit_properties_dialog_close (cd->properties_dialog);

	cd->properties_dialog = gtk_html_edit_properties_dialog_new (cd, TRUE, _("Insert"));

	gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
						   GTK_HTML_EDIT_PROPERTY_TABLE, _("Table"),
						   table_insert,
						   table_insert_cb,
						   table_close_cb);

	gtk_html_edit_properties_dialog_show (cd->properties_dialog);
}

static void
insert_table_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	insert_table (cd);
}

static void
insert_template_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	if (cd->properties_dialog)
		gtk_html_edit_properties_dialog_close (cd->properties_dialog);

	cd->properties_dialog = gtk_html_edit_properties_dialog_new (cd, TRUE, _("Insert"));

	gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
						   GTK_HTML_EDIT_PROPERTY_TABLE, _("Template"),
						   template_insert,
						   template_insert_cb,
						   template_close_cb);

	gtk_html_edit_properties_dialog_show (cd->properties_dialog);
}

static void
properties_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	gchar *argv[2] = {"gtkhtml-properties-capplet", NULL};
	if (gnome_execute_async (NULL, 1, argv) < 0)
		gnome_error_dialog (_("Cannot execute gtkhtml properties"));
}

static void 
indent_more_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	gtk_html_modify_indent_by_delta (GTK_HTML (cd->html), +1);
}

static void 
indent_less_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	gtk_html_modify_indent_by_delta (GTK_HTML (cd->html), -1);
}

static void 
spell_check_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	spell_check_dialog (cd, TRUE);
}

BonoboUIVerb verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("EditUndo", undo_cb),
	BONOBO_UI_UNSAFE_VERB ("EditRedo", redo_cb),
	BONOBO_UI_UNSAFE_VERB ("EditCut", cut_cb),
	BONOBO_UI_UNSAFE_VERB ("EditCopy", copy_cb),
	BONOBO_UI_UNSAFE_VERB ("EditPaste", paste_cb),
	BONOBO_UI_UNSAFE_VERB ("EditFind", search_cb),
	BONOBO_UI_UNSAFE_VERB ("EditFindRegex", search_regex_cb),
	BONOBO_UI_UNSAFE_VERB ("EditFindAgain", search_next_cb),
	BONOBO_UI_UNSAFE_VERB ("EditReplace", replace_cb),
	BONOBO_UI_UNSAFE_VERB ("EditProperties", properties_cb),
	BONOBO_UI_UNSAFE_VERB ("EditSelectAll", select_all_cb),
	BONOBO_UI_UNSAFE_VERB ("EditSpellCheck", spell_check_cb),

	BONOBO_UI_UNSAFE_VERB ("InsertImage", insert_image_cb),
	BONOBO_UI_UNSAFE_VERB ("InsertLink",  insert_link_cb),
	BONOBO_UI_UNSAFE_VERB ("InsertRule",  insert_rule_cb),
	BONOBO_UI_UNSAFE_VERB ("InsertTable", insert_table_cb),
	BONOBO_UI_UNSAFE_VERB ("InsertTemplate", insert_template_cb),

	BONOBO_UI_UNSAFE_VERB ("IndentMore", indent_more_cb),
	BONOBO_UI_UNSAFE_VERB ("IndentLess", indent_less_cb),

	BONOBO_UI_VERB_END
};

static struct {
	GtkHTMLFontStyle style;
	const gchar *verb;
} font_style_assoc[] = {
	{GTK_HTML_FONT_STYLE_SIZE_1, "/commands/FontSizeNegTwo"},
	{GTK_HTML_FONT_STYLE_SIZE_2, "/commands/FontSizeNegOne"},
	{GTK_HTML_FONT_STYLE_SIZE_3, "/commands/FontSizeZero"},
	{GTK_HTML_FONT_STYLE_SIZE_4, "/commands/FontSizeOne"},
	{GTK_HTML_FONT_STYLE_SIZE_5, "/commands/FontSizeTwo"},
	{GTK_HTML_FONT_STYLE_SIZE_6, "/commands/FontSizeThree"},
	{GTK_HTML_FONT_STYLE_SIZE_7, "/commands/FontSizeFour"},
	{GTK_HTML_FONT_STYLE_BOLD,    "/commands/FormatBold"},
	{GTK_HTML_FONT_STYLE_ITALIC, "/commands/FormatItalic"},
	{GTK_HTML_FONT_STYLE_UNDERLINE, "/commands/FormatUnderline"},
	{GTK_HTML_FONT_STYLE_STRIKEOUT, "/commands/FormatStrikeout"},
	{GTK_HTML_FONT_STYLE_FIXED, "/commands/FormatFixed"},
	{GTK_HTML_FONT_STYLE_SUBSCRIPT, "/commands/FormatSubscript"},
	{GTK_HTML_FONT_STYLE_SUBSCRIPT, "/commands/FormatSuperscript"},
	{0, NULL}
};

static struct {
	GtkHTMLParagraphStyle style;
	const gchar *verb;
} paragraph_style_assoc[] = {
	{GTK_HTML_PARAGRAPH_STYLE_NORMAL, "/commands/HeadingNormal"},
	{GTK_HTML_PARAGRAPH_STYLE_H1, "/commands/HeadingH1"},
	{GTK_HTML_PARAGRAPH_STYLE_H2, "/commands/HeadingH2"},
	{GTK_HTML_PARAGRAPH_STYLE_H3, "/commands/HeadingH3"},
	{GTK_HTML_PARAGRAPH_STYLE_H4, "/commands/HeadingH4"},
	{GTK_HTML_PARAGRAPH_STYLE_H5, "/commands/HeadingH5"},
	{GTK_HTML_PARAGRAPH_STYLE_H6, "/commands/HeadingH6"},
	{GTK_HTML_PARAGRAPH_STYLE_ADDRESS, "/commands/HeadingAddress"},
	{GTK_HTML_PARAGRAPH_STYLE_PRE, "/commands/HeadingPreformat"},
	{GTK_HTML_PARAGRAPH_STYLE_ITEMDOTTED, "/commands/HeadingBulletedList"},
	{GTK_HTML_PARAGRAPH_STYLE_ITEMROMAN, "/commands/HeadingRomanList"},
	{GTK_HTML_PARAGRAPH_STYLE_ITEMDIGIT, "/commands/HeadingNumberedList"},
	{GTK_HTML_PARAGRAPH_STYLE_ITEMALPHA, "/commands/HeadingAlphabeticalList"},
	{0, NULL}
};

static struct {
	GtkHTMLParagraphAlignment style;
	const gchar *verb;
} paragraph_align_assoc[] = {
	{GTK_HTML_PARAGRAPH_ALIGNMENT_LEFT, "/commands/AlignLeft"},
	{GTK_HTML_PARAGRAPH_ALIGNMENT_CENTER, "/commands/AlignCenter"},
	{GTK_HTML_PARAGRAPH_ALIGNMENT_RIGHT, "/commands/AlignRight"},
	{0, NULL}
};

static void 
paragraph_align_cb (BonoboUIComponent           *component,
		    const char                  *path,
		    Bonobo_UIComponent_EventType type,
		    const char                  *state,
		    gpointer                     user_data)
     
{
	GtkHTMLControlData *cd = (GtkHTMLControlData *)user_data;
	int i;

	if (cd->block_font_style_change || !atoi(state))
		return;

	/* g_warning ("wowee %s :: %s", path, state); */
	for (i = 0; paragraph_align_assoc[i].verb != NULL; i++) {
		if (!strcmp (path, paragraph_align_assoc[i].verb + 10)) {
			/* g_warning ("setting style to: %s", 
			   paragraph_align_assoc[i].verb); */

			gtk_html_set_paragraph_alignment (cd->html, 
							  paragraph_align_assoc[i].style);
			return;
		}
	}
}

static void 
paragraph_style_cb (BonoboUIComponent           *component,
		    const char                  *path,
		    Bonobo_UIComponent_EventType type,
		    const char                  *state,
		    gpointer                     user_data)
     
{
	GtkHTMLControlData *cd = (GtkHTMLControlData *)user_data;
	int i;
	
	/* g_warning ("wowee %s :: %s", path, state); */
	if (!atoi(state))
		return; 

	for (i = 0; paragraph_style_assoc[i].verb != NULL; i++) {
		if (!strcmp (path, paragraph_style_assoc[i].verb + 10)) {
			/* g_warning ("setting style to: %s", 
			   paragraph_style_assoc[i].verb); */

			gtk_html_set_paragraph_style (cd->html, paragraph_style_assoc[i].style);
			return;
		}
	}
}

static void 
font_size_cb (BonoboUIComponent           *component,
	       const char                  *path,
	       Bonobo_UIComponent_EventType type,
	       const char                  *state,
	       gpointer                     user_data)

{
	GtkHTMLControlData *cd = (GtkHTMLControlData *)user_data;
	int i;
	
	if (cd->block_font_style_change)
		return;

	/* g_warning ("wowee %s :: %s", path, state); */
	for (i = 0; font_style_assoc[i].verb != NULL; i++) {
		if (!strcmp (path, font_style_assoc[i].verb + 10)) {
			if (font_style_assoc[i].style > GTK_HTML_FONT_STYLE_MAX) {
				if (atoi (state)) {
					gtk_html_set_font_style (cd->html, ~0,
								 font_style_assoc[i].style); 
					
					
				} else {
					gint mask = ~0 & ~font_style_assoc[i].style;
					
					gtk_html_set_font_style (cd->html, 
								 mask, 0);
				}			    
				
			} else { 
				if (atoi (state))
					gtk_html_set_font_style (cd->html, 
								 GTK_HTML_FONT_STYLE_MAX 
								 & ~GTK_HTML_FONT_STYLE_SIZE_MASK, 
								 font_style_assoc[i].style);
			}
		}
	}
}

static void 
menubar_update_font_style (GtkWidget *widget, 
			   GtkHTMLFontStyle style, 
			   GtkHTMLControlData *cd)
{
	BonoboUIComponent *uic;
	int size, i;
	CORBA_Environment ev;
	
	CORBA_exception_init (&ev);

	uic = bonobo_control_get_ui_component (cd->control);
	
	g_return_if_fail (uic != NULL);

	size = ((int)style) & GTK_HTML_FONT_STYLE_SIZE_MASK;

	cd->block_font_style_change++;
	for (i = 0; font_style_assoc[i].verb != NULL; i++) {
		/* deal with sizes */
		if (size == font_style_assoc[i].style) {
			bonobo_ui_component_set_prop (uic, font_style_assoc[i].verb, 
						      "state", "1", &ev);
		}

		/* deal with styles */
		if (font_style_assoc[i].style > GTK_HTML_FONT_STYLE_SIZE_MASK) {
			char *state = (int)font_style_assoc[i].style & style ? "1" : "0";

			bonobo_ui_component_set_prop (uic, font_style_assoc[i].verb,
						      "state", state, &ev);
		}
	} 
	cd->block_font_style_change--;

	CORBA_exception_free (&ev);
}

static void
menubar_update_paragraph_alignment (GtkHTML *html, 
				    GtkHTMLParagraphAlignment style, 
				    GtkHTMLControlData *cd)
{
	BonoboUIComponent *uic;
	char *path = NULL;
	int i;
	
	uic = bonobo_control_get_ui_component (cd->control);

	g_return_if_fail (uic != NULL);

	for (i = 0; paragraph_align_assoc[i].verb != NULL; i++) {
		if (paragraph_align_assoc[i].style == style) {
			path = (char *)paragraph_align_assoc[i].verb;
			break;
		}
	}

	if (path) {
		CORBA_Environment ev;

		CORBA_exception_init (&ev);
		cd->block_font_style_change ++;
		bonobo_ui_component_set_prop (uic, path,
					      "state", "1", &ev);
		cd->block_font_style_change --;
		CORBA_exception_free (&ev);	
	} else {
		g_warning ("Unknown Paragraph Alignment");
	}
}

static void
menubar_update_paragraph_style (GtkHTML *html, 
				GtkHTMLParagraphStyle style, 
				GtkHTMLControlData *cd)
{
	BonoboUIComponent *uic;
	const char *path = NULL;
	int i;

	uic = bonobo_control_get_ui_component (cd->control);

	g_return_if_fail (uic != NULL);

	for (i = 0; paragraph_style_assoc[i].verb != NULL; i++) {
		if (paragraph_style_assoc[i].style == style) {
			path = paragraph_style_assoc[i].verb;
			break;
		}
	}

	if (path) {
		CORBA_Environment ev;

		CORBA_exception_init (&ev);
		cd->block_font_style_change ++;
		bonobo_ui_component_set_prop (uic, path,
					      "state", "1", &ev);
		cd->block_font_style_change --;
		CORBA_exception_free (&ev);	
	} else {
		g_warning ("Unknown Paragraph Style");
	}
}

void
menubar_update_format (GtkHTMLControlData *cd)
{
	CORBA_Environment ev;
	BonoboUIComponent *uic;
	gchar *sensitive;

	uic = bonobo_control_get_ui_component (cd->control);

	g_return_if_fail (uic != NULL);

	sensitive = (cd->format_html ? "1" : "0");

	CORBA_exception_init (&ev);

	bonobo_ui_component_freeze (uic, &ev);

	bonobo_ui_component_set_prop (uic, "/commands/InsertImage",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/InsertLink",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/InsertRule",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/InsertTable",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/InsertTemplate",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/InsertTemplate",
				      "sensitive", sensitive, &ev);

	bonobo_ui_component_set_prop (uic, "/commands/FormatBold",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/FormatItalic",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/FormatUnderline",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/FormatStrikeout",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/FormatPlain",
				      "sensitive", sensitive, &ev);
	
	bonobo_ui_component_set_prop (uic, "/commands/AlignLeft",
				      "sensitive", sensitive, &ev);		
	bonobo_ui_component_set_prop (uic, "/commands/AlignRight",
				      "sensitive", sensitive, &ev);	
	bonobo_ui_component_set_prop (uic, "/commands/AlignCenter",
				      "sensitive", sensitive, &ev);	

	bonobo_ui_component_set_prop (uic, "/commands/HeadingH1",
				      "sensitive", sensitive, &ev);	
	bonobo_ui_component_set_prop (uic, "/commands/HeadingH2",
				      "sensitive", sensitive, &ev);	
	bonobo_ui_component_set_prop (uic, "/commands/HeadingH3",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/HeadingH4",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/HeadingH5",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/HeadingH6",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/HeadingAddress",
				      "sensitive", sensitive, &ev);

	bonobo_ui_component_thaw (uic, &ev);	

	CORBA_exception_free (&ev);	
}

void
menubar_setup (BonoboUIComponent  *uic,
	       GtkHTMLControlData *cd)
{
	int i;

	g_return_if_fail (cd->html != NULL);
	g_return_if_fail (GTK_IS_HTML (cd->html));
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (uic));

	gtk_signal_connect (GTK_OBJECT (cd->html), "current_paragraph_style_changed",
			    GTK_SIGNAL_FUNC (menubar_update_paragraph_style), cd);

	gtk_signal_connect (GTK_OBJECT (cd->html), "current_paragraph_alignment_changed",
			    GTK_SIGNAL_FUNC (menubar_update_paragraph_alignment), cd);

	gtk_signal_connect (GTK_OBJECT (cd->html), "insertion_font_style_changed",
			    GTK_SIGNAL_FUNC (menubar_update_font_style), cd);

	bonobo_ui_component_add_verb_list_with_data (uic, verbs, cd);

	for (i = 0; paragraph_style_assoc[i].verb != NULL; i++) {
		bonobo_ui_component_add_listener (uic, paragraph_style_assoc[i].verb + 10, paragraph_style_cb, cd);
	}

	for (i = 0; paragraph_align_assoc[i].verb != NULL; i++) {
		bonobo_ui_component_add_listener (uic, paragraph_align_assoc[i].verb + 10, paragraph_align_cb, cd);
	}

	for (i = 0; font_style_assoc[i].verb != NULL; i++) {
		bonobo_ui_component_add_listener (uic, font_style_assoc[i].verb + 10, font_size_cb, cd);
	}

	bonobo_ui_util_set_ui (uic, GNOMEDATADIR,
			       "GNOME_GtkHTML_Editor.xml",
			       "GNOME_GtkHTML_Editor");
}





