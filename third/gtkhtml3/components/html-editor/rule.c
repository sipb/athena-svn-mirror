/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc.
    Authors:           Radek Doulik (rodo@helixcode.com)

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

#include <config.h>
#include <libgnome/gnome-i18n.h>
#include <string.h>
#include <glade/glade.h>

#include "gtkhtml.h"
#include "htmlcursor.h"
#include "htmlengine-edit-fontstyle.h"
#include "htmlengine-edit-cut-and-paste.h"
#include "htmlengine-edit-rule.h"
#include "htmlengine-save.h"
#include "htmlrule.h"

#include "properties.h"
#include "rule.h"
#include "utils.h"

#define GTK_HTML_EDIT_RULE_WIDTH       0
#define GTK_HTML_EDIT_RULE_SIZE        1
#define GTK_HTML_EDIT_RULE_SPINS       2

struct _GtkHTMLEditRuleProperties {
	GtkHTMLControlData *cd;

	HTMLRule *rule;
	GtkHTML  *sample;
	gboolean  insert;

	gboolean   changed_length;
	gint       length;
	gboolean   length_percent;
	GtkWidget *spin_length;
	GtkWidget *option_length_percent;

	gboolean   changed_width;
	gint       width;
	GtkWidget *spin_width;

	gboolean        changed_align;
	HTMLHAlignType  align;
	GtkWidget      *option_align;

	gboolean        changed_shaded;
	gboolean        shaded;
	GtkWidget      *check_shaded;

	gint       template;
	GtkWidget *option_template;

	gboolean   disable_change;
};
typedef struct _GtkHTMLEditRuleProperties GtkHTMLEditRuleProperties;

#define CHANGE if (!d->disable_change) gtk_html_edit_properties_dialog_change (d->cd->properties_dialog)
#define FILL 	if (!d->disable_change) fill_sample (d)

#define TEMPLATES 3
typedef struct {
	gchar *name;
	gint offset;

	gboolean can_set_shaded;
	gboolean can_set_width;

	gchar *rule;
} RuleInsertTemplate;


static RuleInsertTemplate rule_templates [TEMPLATES] = {
	{
		N_("Plain"), 1,
		TRUE, TRUE,
		"<hr@length@@width@@align@@shaded@>"
	},
	{
		N_("Blue 3D"), 1,
		FALSE, FALSE,
		"<table@length@@align@ cellspacing=0 cellpadding=0>"
		"<tr>"
		"<td><img src=\"file://" ICONDIR "/rule-blue-left.png\"></td>"
		"<td width=\"100%\" background=\"file://" ICONDIR "/rule-blue-center.png\">"
		"<img src=\"file://" ICONDIR "/transparent.png\"></td>"
		"<td><img src=\"file://" ICONDIR "/rule-blue-right.png\"></td>"
		"</tr>"
		"</table>"
	},
	{
		N_("Yellow, flowers"), 1,
		FALSE, FALSE,
		"<table@length@@align@ cellspacing=0 cellpadding=0>"
		"<tr>"
		"<td><img src=\"file://" ICONDIR "/rule-yellow-flowers-left.png\"></td>"
		"<td background=\"file://" ICONDIR "/rule-yellow-flowers-center.png\">"
		"<img src=\"file://" ICONDIR "/transparent.png\" width=10 height=1></td>"
		"<td background=\"file://" ICONDIR "/rule-yellow-flowers-center.png\">"
		"<img src=\"file://" ICONDIR "/flowers.png\"></td>"
		"<td width=\"100%\" background=\"file://" ICONDIR "/rule-yellow-flowers-center.png\">"
		"<img src=\"file://" ICONDIR "/transparent.png\"></td>"
		"<td><img src=\"file://" ICONDIR "/rule-yellow-flowers-right.png\"></td>"
		"</tr>"
		"</table>"
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
get_sample_html (GtkHTMLEditRuleProperties *d, gboolean insert)
{
	gchar *html, *rule, *body, *length, *width, *align, *shaded;

	length = g_strdup_printf (" width=\"%d%s\"", d->length, d->length_percent ? "%" : "");
	width  = g_strdup_printf (" size=%d", d->width);
	shaded = g_strdup (d->shaded ? "" : " noshade");
	align  = g_strdup_printf (" align=%s", d->align == HTML_HALIGN_LEFT
				   ? "left" : (d->align == HTML_HALIGN_RIGHT ? "right" : "center"));

	rule   = g_strdup (rule_templates [d->template].rule);
	rule   = substitute_string (rule, "@length@", length);
	rule   = substitute_string (rule, "@width@", width);
	rule   = substitute_string (rule, "@shaded@", shaded);
	rule   = substitute_string (rule, "@align@", align);

	body   = html_engine_save_get_sample_body (d->cd->html->engine, NULL);
	html   = g_strconcat (body, insert ? "" : "<br>", rule, NULL);

	g_free (length);
	g_free (width);
	g_free (shaded);
	g_free (align);
	g_free (body);	

	/* printf ("RULE: %s\n", html); */

	return html;
}

static void
fill_sample (GtkHTMLEditRuleProperties *d)
{
	gchar *html;

	html = get_sample_html (d, FALSE);
	gtk_html_load_from_string (d->sample, html, -1);
	g_free (html);
}

static void
changed_length (GtkWidget *check, GtkHTMLEditRuleProperties *d)
{
	d->length = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_length));
	if (!d->disable_change)
		d->changed_length = TRUE;
	FILL;
	CHANGE;
}

static void
changed_width (GtkWidget *check, GtkHTMLEditRuleProperties *d)
{
	d->width = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_width));
	if (!d->disable_change)
		d->changed_width = TRUE;
	FILL;
	CHANGE;
}

static void
changed_length_percent (GtkWidget *w, GtkHTMLEditRuleProperties *d)
{
	d->length_percent = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w))) ? TRUE : FALSE;
	if (!d->disable_change)
		d->changed_length = TRUE;
	FILL;
	CHANGE;
}

static void
changed_align (GtkWidget *w, GtkHTMLEditRuleProperties *d)
{
	gint index;

	index = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w)));
	d->align = index == 0 ? HTML_HALIGN_LEFT : (index == 1 ? HTML_HALIGN_CENTER : HTML_HALIGN_RIGHT);
	if (!d->disable_change)
		d->changed_align = TRUE;
	FILL;
	CHANGE;
}

static void
shaded_toggled (GtkWidget *check, GtkHTMLEditRuleProperties *d)
{
	d->shaded = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check));
	if (!d->disable_change)
		d->changed_shaded = TRUE;
	FILL;
	CHANGE;
}

static GtkHTMLEditRuleProperties *
data_new (GtkHTMLControlData *cd)
{
	GtkHTMLEditRuleProperties *data = g_new0 (GtkHTMLEditRuleProperties, 1);

	/* fill data */
	data->cd             = cd;
	data->disable_change = TRUE;
	data->rule           = NULL;

	/* default values */
	data->length         = 100;
	data->width          = 2;
	data->length_percent = TRUE;
	data->shaded         = TRUE;
	data->align          = HTML_HALIGN_CENTER;

	return data;
}

#define UPPER_FIX(x) gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (d->spin_ ## x))->upper = 100000.0

static void
set_ui (GtkHTMLEditRuleProperties *d)
{
	d->disable_change = TRUE;

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_width),  d->width);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_length), d->length);
	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_length_percent), d->length_percent);
	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_align), d->align == HTML_HALIGN_CENTER
				     ? 1 : (d->align == HTML_HALIGN_LEFT ? 0 : 2));

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d->check_shaded), d->shaded);

	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_template), d->template);

	d->disable_change = FALSE;

	FILL;
}

static void
changed_template (GtkWidget *w, GtkHTMLEditRuleProperties *d)
{
	d->template = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w)));

	gtk_widget_set_sensitive (d->spin_width, rule_templates [d->template].can_set_width);
	gtk_widget_set_sensitive (d->check_shaded, rule_templates [d->template].can_set_shaded);

	set_ui (d);

	CHANGE;
}

static void
fill_templates (GtkHTMLEditRuleProperties *d)
{
	GtkWidget *menu;
	gint i;

	menu = gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_template));

	for (i = 0; i < TEMPLATES; i ++)
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_menu_item_new_with_label (_(rule_templates [i].name)));
	gtk_menu_set_active (GTK_MENU (menu), 0);
	gtk_container_remove (GTK_CONTAINER (menu), gtk_menu_get_active (GTK_MENU (menu)));
}

static GtkWidget *
rule_widget (GtkHTMLEditRuleProperties *d, gboolean insert)
{
	GtkWidget *rule_page;
	GladeXML *xml;

	xml = glade_xml_new (GLADE_DATADIR "/gtkhtml-editor-properties.glade", "rule_page", NULL);
	if (!xml)
		g_error (_("Could not load glade file."));

	rule_page = glade_xml_get_widget (xml, "rule_page");

	d->spin_length   = glade_xml_get_widget (xml, "spin_rule_length");
	g_signal_connect (d->spin_length, "value_changed", G_CALLBACK (changed_length), d);
	UPPER_FIX (length);
	d->spin_width   = glade_xml_get_widget (xml, "spin_rule_width");
	g_signal_connect (d->spin_width, "value_changed", G_CALLBACK (changed_width), d);
	UPPER_FIX (width);
	d->option_length_percent = glade_xml_get_widget (xml, "option_rule_percent");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_length_percent)),
			  "selection-done", G_CALLBACK (changed_length_percent), d);

	d->option_align = glade_xml_get_widget (xml, "option_rule_align");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_align)),
			  "selection-done", G_CALLBACK (changed_align), d);

	d->check_shaded = glade_xml_get_widget (xml, "check_rule_shaded");
	g_signal_connect (d->check_shaded, "toggled", G_CALLBACK (shaded_toggled), d);

	gtk_box_pack_start (GTK_BOX (rule_page), sample_frame (&d->sample), FALSE, FALSE, 0);

	d->insert = insert;
	if (insert) {
		d->option_template = glade_xml_get_widget (xml, "option_rule_template");
		g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_template)),
				  "selection-done", G_CALLBACK (changed_template), d);
		fill_templates (d);
		gtk_widget_show_all (rule_page);
	} else {
		gtk_widget_show_all (rule_page);
		gtk_widget_hide (glade_xml_get_widget (xml, "frame_template"));
	}

	d->disable_change = FALSE;

	return rule_page;
}

GtkWidget *
rule_properties (GtkHTMLControlData *cd, gpointer *set_data)
{
	GtkHTMLEditRuleProperties *d = data_new (cd);
	GtkWidget *rv;

	g_assert (HTML_OBJECT_TYPE (cd->html->engine->cursor->object) == HTML_TYPE_RULE);

	*set_data         = d;
	d->rule           = HTML_RULE (cd->html->engine->cursor->object);
	d->shaded         = d->rule->shade;
	d->length_percent = HTML_OBJECT (d->rule)->percent > 0 ? TRUE : FALSE;
	d->length         = HTML_OBJECT (d->rule)->percent > 0 ? HTML_OBJECT (d->rule)->percent : d->rule->length;
	d->width          = d->rule->size;
	d->align          = d->rule->halign;

	rv = rule_widget (d, FALSE);
	set_ui (d);

	return rv;
}

GtkWidget *
rule_insert (GtkHTMLControlData *cd, gpointer *set_data)
{
	GtkHTMLEditRuleProperties *data = data_new (cd);
	GtkWidget *rv;

	*set_data = data;
	rv = rule_widget (data, TRUE);
	set_ui (data);
	gtk_html_edit_properties_dialog_change (data->cd->properties_dialog);

	return rv;
}

gboolean
rule_insert_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	GtkHTMLEditRuleProperties *d = (GtkHTMLEditRuleProperties *) get_data;
	gchar *html;

	html = get_sample_html (d, TRUE);
	gtk_html_append_html (d->cd->html, html);
	g_free (html);
	/* html_engine_insert_rule (cd->html->engine,
				 d->length, d->length_percent ? d->length : 0, d->width,
				 d->shaded, d->align); */
	return TRUE;
}

gboolean
rule_apply_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	GtkHTMLEditRuleProperties *d = (GtkHTMLEditRuleProperties *) get_data;
	HTMLEngine *e = d->cd->html->engine;
	gint position;

	position = e->cursor->position;

	if (e->cursor->object != HTML_OBJECT (d->rule))
		if (!html_cursor_jump_to (e->cursor, e, HTML_OBJECT (d->rule), 1)) {
			GtkWidget *dialog;
			printf ("d: %p\n", d->cd->properties_dialog);
			dialog = gtk_message_dialog_new (GTK_WINDOW (d->cd->properties_dialog->dialog),
							 GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
							 _("The editted rule was removed from the document.\nCannot apply your changes."));
			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
			html_cursor_jump_to_position (e->cursor, e, position);
			return FALSE;
		}

	html_rule_set (d->rule, cd->html->engine, d->length, d->length_percent ? d->length : 0,
		       d->width, d->shaded, d->align);
	html_cursor_jump_to_position (e->cursor, e, position);

	return TRUE;
}

void
rule_close_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	g_free (get_data);
}
