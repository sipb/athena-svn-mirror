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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gnome.h>
#include <bonobo.h>
#include <gal/widgets/e-unicode.h>

#include "htmlengine-edit-cut-and-paste.h"

#include "e-html-utils.h"
#include "menubar.h"
#include "gtkhtml.h"
#include "body.h"
#include "control-data.h"
#include "properties.h"
#include "image.h"
#include "text.h"
#include "paragraph.h"
#include "spell.h"
#include "table.h"
#include "template.h"

static void smiley_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname);

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
	gtk_html_paste (cd->html, FALSE);
}

static void
paste_quotation_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	gtk_html_paste (cd->html, TRUE);
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
	cd->properties_dialog = gtk_html_edit_properties_dialog_new (cd, TRUE, _("Insert"), ICONDIR "/insert-image-24.png");

	gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
						   GTK_HTML_EDIT_PROPERTY_IMAGE, _("Image"),
						   image_insertion,
						   image_insert_cb,
						   image_close_cb);
	gtk_html_edit_properties_dialog_show (cd->properties_dialog);
}

static void
insert_link_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	if (cd->properties_dialog)
		gtk_html_edit_properties_dialog_close (cd->properties_dialog);

	cd->properties_dialog = gtk_html_edit_properties_dialog_new (cd, TRUE, _("Insert"), ICONDIR "/insert-link-24.png");

	gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
						   GTK_HTML_EDIT_PROPERTY_LINK, _("Link"),
						   link_insert,
						   link_insert_cb,
						   link_close_cb);

	gtk_html_edit_properties_dialog_show (cd->properties_dialog);
	gtk_html_edit_properties_dialog_set_page (cd->properties_dialog, GTK_HTML_EDIT_PROPERTY_LINK);
}

static void
insert_rule_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	if (cd->properties_dialog)
		gtk_html_edit_properties_dialog_close (cd->properties_dialog);

	cd->properties_dialog = gtk_html_edit_properties_dialog_new (cd, TRUE, _("Insert"), ICONDIR "/insert-rule-24.png");

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

	cd->properties_dialog = gtk_html_edit_properties_dialog_new (cd, TRUE, _("Insert"), ICONDIR "/insert-table-24.png");

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

	cd->properties_dialog = gtk_html_edit_properties_dialog_new (cd, TRUE, _("Insert"), ICONDIR "/insert-object-24.png");

	gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
						   GTK_HTML_EDIT_PROPERTY_TABLE, _("Template"),
						   template_insert,
						   template_insert_cb,
						   template_close_cb);

	gtk_html_edit_properties_dialog_show (cd->properties_dialog);
}

static void
file_dialog_destroy (GtkWidget *w, GtkHTMLControlData *cd)
{
	cd->file_dialog = NULL;
}

#define BUFFER_SIZE 4096

static void
file_dialog_ok (GtkWidget *w, GtkHTMLControlData *cd)
{
	gchar *filename;
	gint fd;

	filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (cd->file_dialog));
	fd = open (filename, O_RDONLY);
	if (fd != -1) {
		GtkHTML *tmp;
		GtkHTMLStream *stream;
		gchar buffer [BUFFER_SIZE];
		ssize_t rb;

		tmp = GTK_HTML (gtk_html_new ());
		stream = gtk_html_begin_content (tmp, "text/html; charset=utf-8");
		if (!cd->file_html) {
			gtk_html_write (tmp, stream, "<PRE>", 5);
		}
		while ((rb = read (fd, buffer, BUFFER_SIZE - 1)) > 0) {
			gchar *native;

			buffer [rb] = 0;

			native = e_utf8_from_gtk_string (GTK_WIDGET (cd->html), buffer);
			if (cd->file_html) {
				gtk_html_write (tmp, stream, native, -1);
			} else {
				html_engine_paste_text (cd->html->engine, native, g_utf8_strlen (native, -1));
			}
			g_free (native);
		}
		if (!cd->file_html) {
			gtk_html_write (tmp, stream, "</PRE>", 6);
		}
		gtk_html_end (tmp, stream, rb >=0 ? GTK_HTML_STREAM_OK : GTK_HTML_STREAM_ERROR);
		gtk_html_insert_gtk_html (cd->html, tmp);

		close (fd);
	}
	gtk_widget_destroy (cd->file_dialog);
}

static void
insert_file_dialog (GtkHTMLControlData *cd, gboolean html)
{
	cd->file_html = html;
	if (cd->file_dialog != NULL) {
		gdk_window_show (GTK_WIDGET (cd->file_dialog)->window);
		return;
	}

	cd->file_dialog = gtk_file_selection_new (html ? _("Insert HTML file") : _("Insert text file"));
	gtk_file_selection_set_filename (GTK_FILE_SELECTION (cd->file_dialog), "~/");

	gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION (cd->file_dialog)->cancel_button),
				   "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT (cd->file_dialog));

	gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (cd->file_dialog)->ok_button),
			    "clicked", GTK_SIGNAL_FUNC (file_dialog_ok), cd);

	gtk_signal_connect (GTK_OBJECT (cd->file_dialog), "destroy",
			    GTK_SIGNAL_FUNC (file_dialog_destroy), cd);

	gtk_widget_show (cd->file_dialog);
}

static void
insert_text_file_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	insert_file_dialog (cd, FALSE);
}

static void
insert_html_file_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	insert_file_dialog (cd, TRUE);
}

static void 
indent_more_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	gtk_html_indent_push_level (cd->html, HTML_LIST_TYPE_BLOCKQUOTE);
}

static void 
indent_less_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	gtk_html_indent_pop_level (cd->html);
}

static void 
spell_check_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	spell_check_dialog (cd, TRUE);
}

static void
format_page_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	if (cd->properties_dialog)
		gtk_html_edit_properties_dialog_close (cd->properties_dialog);

	cd->properties_dialog = gtk_html_edit_properties_dialog_new (cd, FALSE, _("Properties"), ICONDIR "/properties-16.png");

	gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
						   GTK_HTML_EDIT_PROPERTY_BODY, _("Page"),
						   body_properties,
						   body_apply_cb,
						   body_close_cb);

	gtk_html_edit_properties_dialog_show (cd->properties_dialog);
	gtk_html_edit_properties_dialog_set_page (cd->properties_dialog, GTK_HTML_EDIT_PROPERTY_BODY);
}

static void
format_text_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	if (cd->properties_dialog)
		gtk_html_edit_properties_dialog_close (cd->properties_dialog);

	cd->properties_dialog = gtk_html_edit_properties_dialog_new (cd, FALSE, _("Properties"), ICONDIR "/properties-16.png");

	gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
						   GTK_HTML_EDIT_PROPERTY_BODY, _("Text"),
						   text_properties,
						   text_apply_cb,
						   text_close_cb);

	gtk_html_edit_properties_dialog_show (cd->properties_dialog);
	gtk_html_edit_properties_dialog_set_page (cd->properties_dialog, GTK_HTML_EDIT_PROPERTY_TEXT);
}

static void
format_paragraph_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	if (cd->properties_dialog)
		gtk_html_edit_properties_dialog_close (cd->properties_dialog);

	cd->properties_dialog = gtk_html_edit_properties_dialog_new (cd, FALSE, _("Properties"), ICONDIR "/properties-16.png");

	gtk_html_edit_properties_dialog_add_entry (cd->properties_dialog,
						   GTK_HTML_EDIT_PROPERTY_BODY, _("Paragraph"),
						   paragraph_properties,
						   paragraph_apply_cb,
						   paragraph_close_cb);

	gtk_html_edit_properties_dialog_show (cd->properties_dialog);
	gtk_html_edit_properties_dialog_set_page (cd->properties_dialog, GTK_HTML_EDIT_PROPERTY_PARAGRAPH);
}

BonoboUIVerb verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("EditUndo", undo_cb),
	BONOBO_UI_UNSAFE_VERB ("EditRedo", redo_cb),
	BONOBO_UI_UNSAFE_VERB ("EditCut", cut_cb),
	BONOBO_UI_UNSAFE_VERB ("EditCopy", copy_cb),
	BONOBO_UI_UNSAFE_VERB ("EditPaste", paste_cb),
	BONOBO_UI_UNSAFE_VERB ("EditPasteQuotation", paste_quotation_cb),
	BONOBO_UI_UNSAFE_VERB ("EditFind", search_cb),
	BONOBO_UI_UNSAFE_VERB ("EditFindRegex", search_regex_cb),
	BONOBO_UI_UNSAFE_VERB ("EditFindAgain", search_next_cb),
	BONOBO_UI_UNSAFE_VERB ("EditReplace", replace_cb),
	BONOBO_UI_UNSAFE_VERB ("EditSelectAll", select_all_cb),
	BONOBO_UI_UNSAFE_VERB ("EditSpellCheck", spell_check_cb),

	BONOBO_UI_UNSAFE_VERB ("InsertImage", insert_image_cb),
	BONOBO_UI_UNSAFE_VERB ("InsertLink",  insert_link_cb),
	BONOBO_UI_UNSAFE_VERB ("InsertRule",  insert_rule_cb),
	BONOBO_UI_UNSAFE_VERB ("InsertTable", insert_table_cb),
	BONOBO_UI_UNSAFE_VERB ("InsertTemplate", insert_template_cb),

	BONOBO_UI_UNSAFE_VERB ("InsertTextFile", insert_text_file_cb),
	BONOBO_UI_UNSAFE_VERB ("InsertHTMLFile", insert_html_file_cb),

	BONOBO_UI_UNSAFE_VERB ("InsertSmiley1", smiley_cb),
	BONOBO_UI_UNSAFE_VERB ("InsertSmiley2", smiley_cb),
	BONOBO_UI_UNSAFE_VERB ("InsertSmiley3", smiley_cb),
	BONOBO_UI_UNSAFE_VERB ("InsertSmiley4", smiley_cb),
	BONOBO_UI_UNSAFE_VERB ("InsertSmiley5", smiley_cb),
	BONOBO_UI_UNSAFE_VERB ("InsertSmiley6", smiley_cb),
	BONOBO_UI_UNSAFE_VERB ("InsertSmiley8", smiley_cb),
	BONOBO_UI_UNSAFE_VERB ("InsertSmiley9", smiley_cb),
	BONOBO_UI_UNSAFE_VERB ("InsertSmiley10", smiley_cb),
	BONOBO_UI_UNSAFE_VERB ("InsertSmiley11", smiley_cb),

	BONOBO_UI_UNSAFE_VERB ("IndentMore", indent_more_cb),
	BONOBO_UI_UNSAFE_VERB ("IndentLess", indent_less_cb),

	BONOBO_UI_UNSAFE_VERB ("FormatText", format_text_cb),
	BONOBO_UI_UNSAFE_VERB ("FormatParagraph", format_paragraph_cb),
	BONOBO_UI_UNSAFE_VERB ("FormatPage", format_page_cb),

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
	
	if (cd->block_font_style_change)
		return;

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
	bonobo_ui_component_set_prop (uic, "/commands/InsertSmiley1",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/InsertSmiley2",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/InsertSmiley3",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/InsertSmiley4",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/InsertSmiley5",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/InsertSmiley6",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/InsertSmiley8",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/InsertSmiley9",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/InsertSmiley10",
				      "sensitive", sensitive, &ev);
	bonobo_ui_component_set_prop (uic, "/commands/InsertSmiley11",
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
menubar_set_languages (GtkHTMLControlData *cd, const gchar *lstr)
{
	GString *str;
	gboolean enabled;
	gint i;

	if (!cd->languages)
		return;

	str = g_string_new (NULL);
	cd->block_language_changes = TRUE;
	for (i = 0; i < cd->languages->_length; i ++) {
		enabled = strstr (lstr, cd->languages->_buffer [i].abrev) != NULL;
		g_string_sprintf (str, "/commands/SpellLanguage%d", i + 1);
		bonobo_ui_component_set_prop (cd->uic, str->str, "state", enabled ? "1" : "0", NULL);
	}
	cd->block_language_changes = FALSE;
}

#define SMILEYS 11
static gchar *smiley [SMILEYS] = {
	":D",
	":O",
	":)",
	";)",
	"=)",
	":(",
	":-)",
	":-|",
	":-/",
	":-P",
	":~("
};

static void
smiley_cb (BonoboUIComponent *uic, GtkHTMLControlData *cd, const char *cname)
{
	gint i;

	g_return_if_fail (cname);

	i = atoi (cname + 12) - 1;

	if (i >=0 && i < SMILEYS) {
		gchar *s;
		s = g_strdup_printf ("<IMG ALT=\"%s\" SRC=\"file://" ICONDIR "/smiley-%d.png\">", smiley [i], i + 1);
		gtk_html_insert_html (cd->html, s);
		g_free (s);
	}
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
