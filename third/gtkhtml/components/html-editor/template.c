/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2001 Ximian, Inc.
    Authors:           Radek Doulik (rodo@ximian.com)

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
*/

#include <string.h>
#include <glade/glade.h>

#include "gtkhtml.h"
#include "htmlcursor.h"
#include "htmlengine-edit-fontstyle.h"
#include "htmlengine-edit-cut-and-paste.h"
#include "htmlengine-save.h"

#include "config.h"
#include "properties.h"
#include "template.h"
#include "utils.h"

#define GTK_HTML_EDIT_TEMPLATE_WIDTH       0
#define GTK_HTML_EDIT_TEMPLATE_SIZE        1
#define GTK_HTML_EDIT_TEMPLATE_SPINS       2

struct _GtkHTMLEditTemplateProperties {
	GtkHTMLControlData *cd;
	GtkHTML *sample;

	gint template;
	GtkWidget *clist_template;

	gint width;
	gboolean width_percent;
	GtkWidget *spin_width;
	GtkWidget *option_width_percent;

	HTMLHAlignType halign;
	GtkWidget *option_halign;

	gboolean   disable_change;
};
typedef struct _GtkHTMLEditTemplateProperties GtkHTMLEditTemplateProperties;

#define CHANGE if (!d->disable_change) gtk_html_edit_properties_dialog_change (d->cd->properties_dialog)
#define FILL 	if (!d->disable_change) fill_sample (d)

#define TEMPLATES 2
typedef struct {
	gchar *name;
	gint offset;

	gboolean has_width;
	gboolean has_halign;

	gint default_width;
	gboolean default_width_percent;
	HTMLHAlignType default_halign;

	gchar *template;
} TemplateInsertTemplate;


static TemplateInsertTemplate template_templates [TEMPLATES] = {
	{
		N_("Note"), 4,
		TRUE, TRUE, 80, TRUE, HTML_HALIGN_CENTER,
		N_("<table cellspacing=0 cellpadding=1 bgcolor=\"#ccccc0\"@width@@align@><tr><td>"
		   "<table bgcolor=\"#fffff0\" cellpadding=3 cellspacing=0 width=\"100%\">"
		   "<tr><td valign=top><img src=\"file://" ICONDIR "/bulb.png\" hspace=10></td>"
		   "<td width=\"100%\">"
		   "Place your text here"
		   "</td></tr></table></td></tr></table>")
	},
	{
		N_("Image frame"), 4,
		TRUE, TRUE, 200, FALSE, HTML_HALIGN_CENTER,
		N_("<table bgcolor=\"#c0c0c0\" cellspacing=\"0\" @width@@align@>"
		   "<tr><td>"
		   "<table bgcolor=\"#f2f2f2\" cellspacing=\"0\" cellpadding=\"8\" width=\"100%\">"
		   "<tr><td align=\"center\">"
		   "<img src=\"file://" ICONDIR "/empty_image.png\" align=\"top\" border=\"0\">"
		   "</td></tr>"
		   "<tr><td><font size=\"-1\">Image 1: <b>description</b></font></td>"
		   "</tr></table></td></tr></table>")
	},
};

static gchar *
substitute_string (gchar *str, const gchar *var_name, const gchar *value)
{
	gchar *substr;

	substr = strstr (str, var_name);
	if (substr) {
		gchar *new_str;

		*substr = 0;
		new_str = g_strdup_printf ("%s%s%s", str, value, substr + strlen (var_name));
		g_free (str);
		str = new_str;
	}

	return str;
}

static gchar *
get_sample_html (GtkHTMLEditTemplateProperties *d)
{
	gchar *html, *template, *body, *width, *align;

	width = template_templates [d->template].has_width
		? g_strdup_printf (" width=\"%d%s\"", d->width, d->width_percent ? "%" : "")
		: g_strdup ("");
	align = template_templates [d->template].has_halign || d->halign == HTML_HALIGN_NONE
		? g_strdup_printf (" align=%s", d->halign == HTML_HALIGN_LEFT
				   ? "left" : (d->halign == HTML_HALIGN_RIGHT ? "right" : "center"))
		: g_strdup ("");

	template   = g_strdup (_(template_templates [d->template].template));
	template   = substitute_string (template, "@width@", width);
	template   = substitute_string (template, "@align@", align);

	body   = html_engine_save_get_sample_body (d->cd->html->engine, NULL);
	html   = g_strconcat (body, template, NULL);

	g_free (template);
	g_free (width);
	g_free (align);
	g_free (body);	

	/* printf ("TEMPLATE: %s\n", html); */

	return html;
}

static void
fill_sample (GtkHTMLEditTemplateProperties *d)
{
	gchar *html;

	html = get_sample_html (d);
	gtk_html_load_from_string (d->sample, html, -1);
	g_free (html);
}

static GtkHTMLEditTemplateProperties *
data_new (GtkHTMLControlData *cd)
{
	GtkHTMLEditTemplateProperties *data = g_new0 (GtkHTMLEditTemplateProperties, 1);

	/* fill data */
	data->cd             = cd;
	data->disable_change = TRUE;

	/* default values */
	data->width          = 100;
	data->width_percent  = TRUE;
	data->halign         = HTML_HALIGN_NONE;

	return data;
}

#define UPPER_FIX(x) gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (d->spin_ ## x))->upper = 100000.0

static void
set_ui (GtkHTMLEditTemplateProperties *d)
{
	d->disable_change = TRUE;

	gtk_clist_select_row (GTK_CLIST (d->clist_template), d->template, 0);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_width), d->width);
	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_halign), d->halign - HTML_HALIGN_LEFT);
	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_width_percent), d->width_percent ? 1 : 0);

	d->disable_change = FALSE;

	FILL;
}

static void
changed_template (GtkWidget *w, gint row, gint column, GdkEvent *event, GtkHTMLEditTemplateProperties *d)
{
	d->template = row;

	if (!d->disable_change) {
		gtk_widget_set_sensitive (d->spin_width, template_templates [d->template].has_width);
		gtk_widget_set_sensitive (d->option_width_percent, template_templates [d->template].has_width);
		gtk_widget_set_sensitive (d->option_halign, template_templates [d->template].has_halign);

		if (template_templates [d->template].has_width) {
			d->width = template_templates [d->template].default_width;
			d->width_percent = template_templates [d->template].default_width_percent;
		}
		if (template_templates [d->template].has_halign)
			d->halign = template_templates [d->template].default_halign;

		if (template_templates [d->template].has_width || template_templates [d->template].has_halign)
			set_ui (d);
		else {
			FILL;
		}
		CHANGE;
	}
}

static void
fill_templates (GtkHTMLEditTemplateProperties *d)
{
	gchar *name [1];
	gint i;

	gtk_clist_freeze (GTK_CLIST (d->clist_template));
	for (i = 0; i < TEMPLATES; i ++) {
		name [0] = template_templates [i].name;
		gtk_clist_append (GTK_CLIST (d->clist_template), name);
	}
	gtk_clist_thaw (GTK_CLIST (d->clist_template));
}

static GtkWidget *
template_widget (GtkHTMLEditTemplateProperties *d, gboolean insert)
{
	GtkWidget *template_page, *frame;
	GladeXML *xml;

	xml = glade_xml_new (GLADE_DATADIR "/gtkhtml-editor-properties.glade", "vbox_template");
	if (!xml)
		g_error (_("Could not load glade file."));

	template_page = glade_xml_get_widget (xml, "vbox_template");

	d->clist_template = glade_xml_get_widget (xml, "clist_templates");
	gtk_signal_connect (GTK_OBJECT (d->clist_template), "select_row", changed_template, d);
	fill_templates (d);

	d->spin_width           = glade_xml_get_widget (xml, "spin_template_width");
	d->option_width_percent = glade_xml_get_widget (xml, "option_template_percent");
	d->option_halign        = glade_xml_get_widget (xml, "option_template_halign");
	UPPER_FIX (width);

	frame = sample_frame (&d->sample);
	gtk_widget_set_usize (frame, -1, 260);
	gtk_box_pack_start (GTK_BOX (template_page), frame, FALSE, FALSE, 0);

	d->disable_change = FALSE;

	gtk_widget_show_all (template_page);

	return template_page;
}

GtkWidget *
template_insert (GtkHTMLControlData *cd, gpointer *set_data)
{
	GtkHTMLEditTemplateProperties *data = data_new (cd);
	GtkWidget *rv;

	*set_data = data;
	rv = template_widget (data, TRUE);
	set_ui (data);
	gtk_clist_select_row (GTK_CLIST (data->clist_template), 0, 0);
	gtk_html_edit_properties_dialog_change (data->cd->properties_dialog);

	return rv;
}

void
template_insert_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	GtkHTMLEditTemplateProperties *d = (GtkHTMLEditTemplateProperties *) get_data;
	HTMLEngine *e = d->cd->html->engine;
	gchar *html;
	gint position;

	position = e->cursor->position + template_templates [d->template].offset;
	html = get_sample_html (d);
	gtk_html_append_html (d->cd->html, html);
	g_free (html);
	if (template_templates [d->template].offset >= 0)
		html_cursor_jump_to_position (e->cursor, e, position);
}

void
template_close_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	g_free (get_data);
}

