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
#include "utils.h"

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
		gnome_url_show (url, NULL);
}

static void
set_ui (GtkHTMLEditLinkProperties *data)
{
	gchar *url;

	gtk_entry_set_text (GTK_ENTRY (data->entry_text), HTML_TEXT (data->link)->text);

	url = data->link->url && *data->link->url
		? g_strconcat (data->link->url, data->link->target && *data->link->target ? "#" : NULL,
			       data->link->target, NULL)
		: g_strdup ("");
	gtk_entry_set_text (GTK_ENTRY (data->entry_url), url);
	g_free (url);
}

static gboolean stock_test_url_added = FALSE;
#define GTKHTML_STOCK_TEST_URL "gtkhtml-stock-test-url"
static GtkStockItem test_url_items [] =
{
	{ GTKHTML_STOCK_TEST_URL, N_("Test URL..."), 0, 0, NULL }
};

static GtkWidget *
link_widget (GtkHTMLEditLinkProperties *data, gboolean insert)
{
	GtkHTMLControlData *cd = data->cd;
	GtkWidget *vbox, *hbox, *button, *frame, *f1;

	if (!stock_test_url_added) {
		GdkPixbuf *pixbuf;
		GError *error = NULL;

		pixbuf = gdk_pixbuf_new_from_file (ICONDIR "/insert-link-16.png", &error);
		if (!pixbuf) {
			g_error_free (error);
		} else {
			GtkIconSet *test_url_iconset = gtk_icon_set_new_from_pixbuf (pixbuf);

			if (test_url_iconset) {
				GtkIconFactory *factory = gtk_icon_factory_new ();

				gtk_icon_factory_add (factory, GTKHTML_STOCK_TEST_URL, test_url_iconset);
				gtk_icon_factory_add_default (factory);
			}
			gtk_stock_add_static (test_url_items, G_N_ELEMENTS (test_url_items));
		}
		stock_test_url_added = TRUE;
	}

	vbox = gtk_vbox_new (FALSE, 6);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);

	data->entry_text = gtk_entry_new ();
	data->entry_url  = gtk_entry_new ();

	frame = gtk_frame_new (_("Link text"));
	f1    = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (f1), GTK_SHADOW_NONE);
	gtk_container_set_border_width (GTK_CONTAINER (f1), 6);
	gtk_container_add (GTK_CONTAINER (f1), data->entry_text);
	gtk_container_add (GTK_CONTAINER (frame), f1);
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

	if (html_engine_is_selection_active (cd->html->engine)) {
		gchar *str;

		str = html_engine_get_selection_string (cd->html->engine);
		gtk_entry_set_text (GTK_ENTRY (data->entry_text), str);
		g_free (str);
	}

	frame = gtk_frame_new (_("Click will follow this URL"));
	f1    = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (f1), GTK_SHADOW_NONE);
	gtk_container_set_border_width (GTK_CONTAINER (f1), 6);
	hbox = gtk_hbox_new (FALSE, 12);
	button = gtk_button_new_from_stock (GTKHTML_STOCK_TEST_URL);
	gtk_box_pack_start (GTK_BOX (hbox), data->entry_url, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
	gtk_container_add (GTK_CONTAINER (f1), hbox);
	gtk_container_add (GTK_CONTAINER (frame), f1);
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

	if (!insert) {
		gtk_widget_set_sensitive (data->entry_text, FALSE);
		set_ui (data);
	}

	g_signal_connect (data->entry_text, "changed", G_CALLBACK (changed), data);
	g_signal_connect (data->entry_url, "changed", G_CALLBACK (changed), data);
	g_signal_connect (button, "clicked", G_CALLBACK (test_clicked), data);

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

gboolean
link_apply_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	GtkHTMLEditLinkProperties *data = (GtkHTMLEditLinkProperties *) get_data;
	HTMLEngine *e = data->cd->html->engine;

	const gchar *url;
	gchar *target;

	if (data->url_changed) {
		gchar *url_copy;
		gint position;

		position = e->cursor->position;

		if (e->cursor->object != HTML_OBJECT (data->link))
			if (!html_cursor_jump_to (e->cursor, e, HTML_OBJECT (data->link), 1)) {
				GtkWidget *dialog;
				printf ("d: %p\n", data->cd->properties_dialog);
				dialog = gtk_message_dialog_new (GTK_WINDOW (data->cd->properties_dialog->dialog),
								 GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
								 _("The editted link was removed from the document.\nCannot apply your changes."));
				gtk_dialog_run (GTK_DIALOG (dialog));
				gtk_widget_destroy (dialog);
				html_cursor_jump_to_position (e->cursor, e, position);
				return FALSE;
			}

		url  = gtk_entry_get_text (GTK_ENTRY (data->entry_url));

		target = strchr (url, '#');

		url_copy = target ? g_strndup (url, target - url) : g_strdup (url);
		html_link_text_set_url (data->link, url_copy, target);
		html_engine_update_insertion_url_and_target (e);
		g_free (url_copy);
		html_cursor_jump_to_position (e->cursor, e, position);
	}

	return TRUE;
}

gboolean
link_insert_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	GtkHTMLEditLinkProperties *data = (GtkHTMLEditLinkProperties *) get_data;
	HTMLEngine *e = cd->html->engine;
	const gchar *url;
	gchar *target;
	const gchar *text;

	url  = gtk_entry_get_text (GTK_ENTRY (data->entry_url));
	text = gtk_entry_get_text (GTK_ENTRY (data->entry_text));
	if (url && text && *url && *text) {
		HTMLObject *new_link;
		gchar *url_copy;

		target = strchr (url, '#');

		url_copy = target ? g_strndup (url, target - url) : g_strdup (url);
		new_link = html_link_text_new (text, GTK_HTML_FONT_STYLE_DEFAULT,
					       html_colorset_get_color (e->settings->color_set, HTMLLinkColor),
					       url_copy, target);

		html_engine_paste_object (e, new_link, g_utf8_strlen (text, -1));

		g_free (url_copy);
	}

	return TRUE;
}

void
link_close_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	g_free (get_data);
}
