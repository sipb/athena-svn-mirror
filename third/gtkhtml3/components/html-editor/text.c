/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2000,2001,2002 Ximian, Inc.
    Authors:  Radek Doulik (rodo@ximian.com)

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
#include <gal/widgets/widget-color-combo.h>

#include "htmlcolor.h"
#include "htmlcolorset.h"
#include "htmlengine-edit.h"
#include "htmlengine-edit-cut-and-paste.h"
#include "htmlengine-edit-fontstyle.h"
#include "htmlengine-save.h"
#include "htmlselection.h"
#include "htmlsettings.h"
#include "htmltext.h"

#include "text.h"
#include "properties.h"
#include "utils.h"

struct _GtkHTMLEditTextProperties {

	GtkHTMLControlData *cd;

	GtkWidget *color_combo;
	GtkWidget *style_option;
	GtkWidget *sel_size;
	GtkWidget *check [4];
	GtkWidget *entry_url;

	gboolean color_changed;
	gboolean style_changed;
	gboolean url_changed;

	GtkHTMLFontStyle style_and;
	GtkHTMLFontStyle style_or;
	HTMLColor *color;
	gchar *url;

	GtkHTML *sample;

	HTMLText *text;
};
typedef struct _GtkHTMLEditTextProperties GtkHTMLEditTextProperties;

#define STYLES 4
static GtkHTMLFontStyle styles [STYLES] = {
	GTK_HTML_FONT_STYLE_BOLD,
	GTK_HTML_FONT_STYLE_ITALIC,
	GTK_HTML_FONT_STYLE_UNDERLINE,
	GTK_HTML_FONT_STYLE_STRIKEOUT,
};

#define CVAL(i) (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (d->check [i])))

static gint get_size (GtkHTMLFontStyle s);

static void
fill_sample (GtkHTMLEditTextProperties *d)
{
	gchar *body, *size, *color, *bg, *a, *sa;

	if (d->url && *d->url) {
		gchar *enc_url;

		enc_url = html_encode_entities (d->url, g_utf8_strlen (d->url, -1), NULL);
		a = g_strdup_printf ("<a href=\"%s\">", d->url);
		g_free (enc_url);
	} else
		a = g_strdup ("");

	bg    = html_engine_save_get_sample_body (d->cd->html->engine, NULL);
	sa    = d->url && *d->url ? "</a>" : "";
	size  = g_strdup_printf ("<font size=%d>", get_size (d->style_or) + 1);
	
	color = g_strdup_printf ("<font color=#%02x%02x%02x>",
				 d->color->color.red   >> 8,
				 d->color->color.green >> 8,
				 d->color->color.blue  >> 8);
	
	body  = g_strconcat (bg, a,
			     CVAL (0) ? "<b>" : "",
			     CVAL (1) ? "<i>" : "",
			     CVAL (2) ? "<u>" : "",
			     CVAL (3) ? "<s>" : "",
			     size, color,
			     _("The quick brown fox jumps over the lazy dog."), sa, NULL);
	
	gtk_html_load_from_string (d->sample, body, -1);
	g_free (color);
	g_free (size);
	g_free (a);
	g_free (bg);
	g_free (body);
}

static void
color_changed (GtkWidget *w, GdkColor *color, gboolean custom, gboolean by_user, gboolean is_default,
	       GtkHTMLEditTextProperties *data)
{
	html_color_unref (data->color);
	data->color = color
		&& color != &html_colorset_get_color (data->cd->html->engine->settings->color_set, HTMLTextColor)->color
		? html_color_new_from_gdk_color (color)
		: html_colorset_get_color (data->cd->html->engine->settings->color_set, HTMLTextColor);
	html_color_ref (data->color);
	data->color_changed = TRUE;
	gtk_html_edit_properties_dialog_change (data->cd->properties_dialog);
	fill_sample (data);
}

static void
set_size (GtkWidget *w, GtkHTMLEditTextProperties *data)
{
	gint size = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (w), "size"));

	data->style_and &= ~GTK_HTML_FONT_STYLE_SIZE_MASK;
	data->style_or  &= ~GTK_HTML_FONT_STYLE_SIZE_MASK;
	data->style_or  |= size;
	data->style_changed = TRUE;
	gtk_html_edit_properties_dialog_change (data->cd->properties_dialog);
	fill_sample (data);
}

static void
set_style (GtkWidget *w, GtkHTMLEditTextProperties *d)
{
	GtkHTMLFontStyle style = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (w), "style"));

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w))) {
		d->style_or  |= style;
		d->style_and |= style;
	} else {
		d->style_or  &= ~style;
		d->style_and &= ~style;
	}

	d->style_changed = TRUE;
	gtk_html_edit_properties_dialog_change (d->cd->properties_dialog);
	fill_sample (d);
}

static gint
get_size (GtkHTMLFontStyle s)
{
	return (s & GTK_HTML_FONT_STYLE_SIZE_MASK)
		? (s & GTK_HTML_FONT_STYLE_SIZE_MASK) - GTK_HTML_FONT_STYLE_SIZE_1
		: 2;
}

static void
set_url (GtkWidget *w, GtkHTMLEditTextProperties *data)
{
	g_free (data->url);
	data->url = g_strdup (gtk_entry_get_text (GTK_ENTRY (data->entry_url)));
	data->url_changed = TRUE;

	gtk_html_edit_properties_dialog_change (data->cd->properties_dialog);
	fill_sample (data);
}

GtkWidget *
text_properties (GtkHTMLControlData *cd, gpointer *set_data)
{
	GtkHTMLEditTextProperties *data = g_new (GtkHTMLEditTextProperties, 1);
	GtkWidget *vbox, *frame, *table, *menu, *menuitem, *hbox, *t1;
	gboolean selection;
	const gchar *target;
	const gchar *url;
	gint i;

	selection = html_engine_is_selection_active (cd->html->engine);

	*set_data = data;

	data->cd = cd;
	data->color_changed   = FALSE;
	data->style_changed   = FALSE;
	data->url_changed     = FALSE;
	data->style_and       = GTK_HTML_FONT_STYLE_MAX;
	data->style_or        = html_engine_get_font_style (cd->html->engine);
	data->color           = html_engine_get_color (cd->html->engine);
	data->text            = HTML_TEXT (cd->html->engine->cursor->object);

	if (!data->color)
		data->color = html_colorset_get_color (data->cd->html->engine->settings->color_set, 
						       HTMLTextColor);

	target = html_engine_get_target (cd->html->engine);
	url  = html_engine_get_url (cd->html->engine);
	data->url = selection ? g_strconcat (url ? url : "", target ? "#" : "", target, NULL) : NULL;

	html_color_ref (data->color);

	table = gtk_table_new (3, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), 12);
	gtk_table_set_col_spacings (GTK_TABLE (table), 12);
	gtk_table_set_row_spacings (GTK_TABLE (table), 4);

	vbox = gtk_vbox_new (FALSE, 6);
	frame = gtk_frame_new (_("Style"));
	t1 = gtk_table_new (2, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (t1), 6);

#define ADD_CHECK(x,c,r) \
	data->check [i] = gtk_check_button_new_with_label (x); \
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (data->check [i]), data->style_or & styles [i]); \
        g_object_set_data (G_OBJECT (data->check [i]), "style", GUINT_TO_POINTER (styles [i])); \
        g_signal_connect (data->check [i], "toggled", G_CALLBACK (set_style), data); \
	gtk_table_attach (GTK_TABLE (t1), data->check [i], c, c + 1, r, r + 1, GTK_FILL | GTK_EXPAND, 0, 0, 0); \
        i++

	i=0;
	ADD_CHECK (_("Bold"), 0, 0);
	ADD_CHECK (_("Italic"), 0, 1);
	ADD_CHECK (_("Underline"), 1, 0);
	ADD_CHECK (_("Strikeout"), 1, 1);

	gtk_container_add (GTK_CONTAINER (frame), t1);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), frame);

	if (html_engine_is_selection_active (cd->html->engine)) {
		GtkWidget *f1;

		frame = gtk_frame_new (_("Click will follow this URL"));
		data->entry_url = gtk_entry_new ();
		if (data->url) {
			gtk_entry_set_text (GTK_ENTRY (data->entry_url), data->url);
		}
		f1 = gtk_frame_new (NULL);
		gtk_container_set_border_width (GTK_CONTAINER (f1), 6);
		gtk_frame_set_shadow_type (GTK_FRAME (f1), GTK_SHADOW_NONE);
		gtk_container_add (GTK_CONTAINER (f1), data->entry_url);
		gtk_container_add (GTK_CONTAINER (frame), f1);
		gtk_box_pack_start_defaults (GTK_BOX (vbox), frame);
		g_signal_connect (data->entry_url, "changed", G_CALLBACK (set_url), data);
	}

	gtk_table_attach_defaults (GTK_TABLE (table), vbox, 0, 1, 0, 2);

	frame = gtk_frame_new (_("Size"));
	menu = gtk_menu_new ();

#undef ADD_ITEM
#define ADD_ITEM(n) \
	menuitem = gtk_menu_item_new_with_label (_(n)); \
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem); \
        gtk_widget_show (menuitem); \
        g_signal_connect (menuitem, "activate", G_CALLBACK (set_size), data); \
        g_object_set_data (G_OBJECT (menuitem), "size", GINT_TO_POINTER (i)); i++;

	i=GTK_HTML_FONT_STYLE_SIZE_1;
	ADD_ITEM("-2");
	ADD_ITEM("-1");
	ADD_ITEM("+0");
	ADD_ITEM("+1");
	ADD_ITEM("+2");
	ADD_ITEM("+3");
	ADD_ITEM("+4");

	data->sel_size = gtk_option_menu_new ();
	gtk_option_menu_set_menu (GTK_OPTION_MENU (data->sel_size), menu);
	gtk_option_menu_set_history (GTK_OPTION_MENU (data->sel_size), get_size (data->style_or));
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);
	gtk_box_pack_start (GTK_BOX (vbox), data->sel_size, FALSE, FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);
	gtk_table_attach (GTK_TABLE (table), frame, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);

	/* color selection */
	frame = gtk_frame_new (_("Color"));
	hbox = gtk_hbox_new (FALSE, 12);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);

	data->color_combo = color_combo_new (NULL, _("Automatic"),
					     &data->color->color,
					     color_group_fetch ("text", data->cd));
        g_signal_connect (data->color_combo, "color_changed", G_CALLBACK (color_changed), data);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), data->color_combo, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

	gtk_container_add (GTK_CONTAINER (frame), hbox);
	gtk_table_attach (GTK_TABLE (table), frame, 1, 2, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);

	/* sample */
	gtk_table_attach (GTK_TABLE (table), sample_frame (&data->sample), 0, 2, 2, 3, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 0, 0);
	fill_sample (data);

	gtk_widget_show_all (table);

	return table;
}

gboolean
text_apply_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	GtkHTMLEditTextProperties *data = (GtkHTMLEditTextProperties *) get_data;

	if (data->style_changed || data->url_changed || data->color_changed) {
		HTMLEngine *e = cd->html->engine;
		gint position;

		position = e->cursor->position;

		if (!html_engine_is_selection_active (e) && e->cursor->object != HTML_OBJECT (data->text))
			if (!html_cursor_jump_to (e->cursor, e, HTML_OBJECT (data->text), 1)) {
				GtkWidget *dialog;
				printf ("d: %p\n", data->cd->properties_dialog);
				dialog = gtk_message_dialog_new (GTK_WINDOW (data->cd->properties_dialog->dialog),
								 GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
								 _("The editted text was removed from the document.\nCannot apply your changes."));
				gtk_dialog_run (GTK_DIALOG (dialog));
				gtk_widget_destroy (dialog);
				html_cursor_jump_to_position (e->cursor, e, position);
				return FALSE;
			}

		if (data->style_changed)
			gtk_html_set_font_style (cd->html, data->style_and, data->style_or);

		if (data->url_changed) {
			gchar *h;

			h = strchr (data->url, '#');
			if (h) {
				gchar *url;

				url = alloca (h - data->url + 1);
				url [h - data->url] = 0;
				strncpy (url, data->url, h - data->url);
				html_engine_edit_set_link (cd->html->engine, url, h);
			} else {
				html_engine_edit_set_link (cd->html->engine, data->url, NULL);
			}
		}

		if (data->color_changed)
			gtk_html_set_color (cd->html, data->color);

		data->color_changed = FALSE;
		data->style_changed = FALSE;
		data->url_changed   = FALSE;
		html_cursor_jump_to_position (e->cursor, e, position);
	}

	return TRUE;
}

void
text_close_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	GtkHTMLEditTextProperties *data = (GtkHTMLEditTextProperties *) get_data;

	html_color_unref (data->color);

	g_free (get_data);
}
