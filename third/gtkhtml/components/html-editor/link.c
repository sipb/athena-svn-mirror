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
#include <string.h>
#include <gal/widgets/e-unicode.h>
#include "htmlcolor.h"
#include "htmlcolorset.h"
#include "htmllinktext.h"
#include "htmlengine-edit.h"
#include "htmlengine-edit-fontstyle.h"
#include "htmlengine-edit-cut-and-paste.h"
#include "htmlselection.h"
#include "htmlsettings.h"

#include "properties.h"
#include "link.h"

struct _GtkHTMLEditLinkProperties {
	GtkHTMLControlData *cd;
	GtkWidget *entry_text;
	GtkWidget *entry_url;

	HTMLLinkText *link;

	gboolean url_changed;
};
typedef struct _GtkHTMLEditLinkProperties GtkHTMLEditLinkProperties;

static void
changed (GtkWidget *w, GtkHTMLEditLinkProperties *data)
{
	data->url_changed = TRUE;
	gtk_html_edit_properties_dialog_change (data->cd->properties_dialog);
}

static void
test_clicked (GtkWidget *w, GtkHTMLEditLinkProperties *data)
{
	const char *url = gtk_entry_get_text (GTK_ENTRY (data->entry_url));

	if (url)
		gnome_url_show (url);
}

static void
set_ui (GtkHTMLEditLinkProperties *data)
{
	gchar *text;
	gchar *url, *url8;

	text = e_utf8_to_gtk_string (data->entry_text, HTML_TEXT (data->link)->text);
	gtk_entry_set_text (GTK_ENTRY (data->entry_text), text);
	g_free (text);

	url8 = data->link->url && *data->link->url
		? g_strconcat (data->link->url, data->link->target && *data->link->target ? "#" : NULL,
			       data->link->target, NULL)
		: g_strdup ("");
	url = e_utf8_to_gtk_string (data->entry_url, url8);
	gtk_entry_set_text (GTK_ENTRY (data->entry_url), url);
	g_free (url);
	g_free (url8);
}

static GtkWidget *
link_widget (GtkHTMLEditLinkProperties *data, gboolean insert)
{
	GtkHTMLControlData *cd = data->cd;
	GtkWidget *vbox, *hbox, *button, *frame, *f1;

	vbox = gtk_vbox_new (FALSE, 3);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 3);

	data->entry_text = gtk_entry_new ();
	data->entry_url  = gtk_entry_new ();

	frame = gtk_frame_new (_("Link text"));
	f1    = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (f1), GTK_SHADOW_NONE);
	gtk_container_set_border_width (GTK_CONTAINER (f1), 3);
	gtk_container_add (GTK_CONTAINER (f1), data->entry_text);
	gtk_container_add (GTK_CONTAINER (frame), f1);
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

	if (html_engine_is_selection_active (cd->html->engine)) {
		gchar *str, *str_gtk;

		str = html_engine_get_selection_string (cd->html->engine);
		str_gtk = e_utf8_to_gtk_string (data->entry_text, str);
		gtk_entry_set_text (GTK_ENTRY (data->entry_text), str_gtk);
		g_free (str);
		g_free (str_gtk);
	}

	frame = gtk_frame_new (_("Click will follow this URL"));
	f1    = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (f1), GTK_SHADOW_NONE);
	gtk_container_set_border_width (GTK_CONTAINER (f1), 3);
	hbox = gtk_hbox_new (FALSE, 5);
	button = gtk_button_new_with_label (_("Test URL..."));
	gtk_box_pack_start (GTK_BOX (hbox), data->entry_url, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
	gtk_container_add (GTK_CONTAINER (f1), hbox);
	gtk_container_add (GTK_CONTAINER (frame), f1);
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

	if (!insert) {
		gtk_widget_set_sensitive (data->entry_text, FALSE);
		set_ui (data);
	}

	gtk_signal_connect (GTK_OBJECT (data->entry_text), "changed", changed, data);
	gtk_signal_connect (GTK_OBJECT (data->entry_url), "changed", changed, data);
	gtk_signal_connect (GTK_OBJECT (button), "clicked", test_clicked, data);

	gtk_widget_show_all (vbox);

	return vbox;
}

GtkWidget *
link_insert (GtkHTMLControlData *cd, gpointer *set_data)
{
	GtkHTMLEditLinkProperties *data = g_new (GtkHTMLEditLinkProperties, 1);

	*set_data = data;
	data->cd = cd;

	return link_widget (data, TRUE);
}

GtkWidget *
link_properties (GtkHTMLControlData *cd, gpointer *set_data)
{
	GtkHTMLEditLinkProperties *data = g_new (GtkHTMLEditLinkProperties, 1);

	g_return_val_if_fail (cd->html->engine->cursor->object, NULL);
	g_return_val_if_fail (HTML_IS_LINK_TEXT (cd->html->engine->cursor->object), NULL);

	*set_data = data;
	data->cd = cd;
	data->link = HTML_LINK_TEXT (cd->html->engine->cursor->object);

	return link_widget (data, FALSE);
}

void
link_apply_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	GtkHTMLEditLinkProperties *data = (GtkHTMLEditLinkProperties *) get_data;
	HTMLEngine *e = data->cd->html->engine;

	gchar *url;
	gchar *target;

	if (data->url_changed) {
		gchar *url_copy;

		url  = e_utf8_from_gtk_string (data->entry_url, gtk_entry_get_text (GTK_ENTRY (data->entry_url)));

		target = strchr (url, '#');

		url_copy = target ? g_strndup (url, target - url) : g_strdup (url);
		html_link_text_set_url (data->link, url_copy, target);
		html_engine_update_insertion_url_and_target (e);
		g_free (url_copy);
		g_free (url);
	}
}

void
link_insert_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	GtkHTMLEditLinkProperties *data = (GtkHTMLEditLinkProperties *) get_data;
	HTMLEngine *e = cd->html->engine;
	gchar *url;
	gchar *target;
	gchar *text;

	url  = gtk_entry_get_text (GTK_ENTRY (data->entry_url));
	text = gtk_entry_get_text (GTK_ENTRY (data->entry_text));
	if (url && text && *url && *text) {
		HTMLObject *new_link;
		gchar *url_copy;

		url  = e_utf8_from_gtk_string (data->entry_url, url);
		text = e_utf8_from_gtk_string (data->entry_text, text);

		target = strchr (url, '#');

		url_copy = target ? g_strndup (url, target - url) : g_strdup (url);
		new_link = html_link_text_new (text, GTK_HTML_FONT_STYLE_DEFAULT,
					       html_colorset_get_color (e->settings->color_set, HTMLLinkColor),
					       url_copy, target);

		html_engine_paste_object (e, new_link, g_utf8_strlen (text, -1));

		g_free (url_copy);
		g_free (url);
		g_free (text);
	}
}

void
link_close_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	g_free (get_data);
}
