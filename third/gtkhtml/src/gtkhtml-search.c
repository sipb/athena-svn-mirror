/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright 1999, 2000 Helix Code, Inc.
    Authors:             Radek Doulik (rodo@helixcode.com)
    
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
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include "gtkhtml.h"
#include "gtkhtml-private.h"
#include "gtkhtml-search.h"
#include "htmlengine-search.h"
#include "htmlsearch.h"
#include "htmlselection.h"

struct _GtkHTMLISearch {
	GtkHTML  *html;
	gboolean forward;
	gboolean changed;
	guint focus_out;
	gchar *last_text;
};
typedef struct _GtkHTMLISearch GtkHTMLISearch;

static void
changed (GtkEntry *entry, GtkHTMLISearch *data)
{
	/* printf ("isearch changed to '%s'\n", gtk_entry_get_text (entry)); */
	if (*gtk_entry_get_text (GTK_ENTRY (data->html->priv->search_input_line))) {
		html_engine_search_incremental (data->html->engine,
						gtk_entry_get_text (GTK_ENTRY (data->html->priv->search_input_line)),
						data->forward);
	} else
		html_engine_unselect_all (data->html->engine);
	data->changed = TRUE;
}

static void
continue_search (GtkHTMLISearch *data, gboolean forward)
{
	HTMLEngine *e = data->html->engine;

	if (!data->changed && data->last_text && *data->last_text) {
		gtk_entry_set_text (GTK_ENTRY (data->html->priv->search_input_line), data->last_text);
		html_engine_search_incremental (data->html->engine, data->last_text, forward);
		data->changed = TRUE;
	} else if (*gtk_entry_get_text (GTK_ENTRY (data->html->priv->search_input_line))) {
		if (e->search_info)
			html_search_set_forward (e->search_info, forward);
		html_engine_search_next (e);
	}
	data->forward = forward;
}

static gboolean
hide (GtkHTMLISearch *data)
{
	gtk_signal_disconnect (GTK_OBJECT (data->html->priv->search_input_line), data->focus_out);
	gtk_grab_remove (GTK_WIDGET (data->html->priv->search_input_line));
	gtk_widget_grab_focus (GTK_WIDGET (data->html));
	gtk_widget_hide (GTK_WIDGET (data->html->priv->search_input_line));

	return FALSE;
}

static void
data_destroy (GtkHTMLISearch *data)
{
	g_free (data->last_text);
	g_free (data);
}

static gint
key_press (GtkWidget *widget, GdkEventKey *event, GtkHTMLISearch *data)
{
	gint rv = TRUE;

	if (event->state & GDK_CONTROL_MASK && event->keyval == GDK_s) {
		continue_search (data, TRUE);
	} else if (event->state & GDK_CONTROL_MASK && event->keyval == GDK_r) {
		continue_search (data, FALSE);
	} else if (event->keyval == GDK_Escape) {
		hide (data);
	} else
		rv = FALSE;

	return rv;
}

static gint
focus_out_event (GtkWidget *widget, GdkEventFocus *event, GtkHTMLISearch *data)
{
	hide (data);

	return FALSE;
}

static void
destroy (GtkWidget *w, GtkHTMLISearch *data)
{
	data_destroy (data);
}

void
gtk_html_isearch (GtkHTML *html, gboolean forward)
{
	GtkHTMLISearch *data;

	if (!html->editor_api->create_input_line)
		return;

	if (!html->priv->search_input_line) {
		html->priv->search_input_line  = (*html->editor_api->create_input_line) (html, html->editor_data);
		if (!html->priv->search_input_line)
			return;
		gtk_widget_ref (GTK_WIDGET (html->priv->search_input_line));

		data = g_new (GtkHTMLISearch, 1);
		gtk_object_set_data (GTK_OBJECT (html->priv->search_input_line), "search_data", data);

		data->html      = html;

		gtk_signal_connect (GTK_OBJECT (html->priv->search_input_line), "key_press_event",
				    GTK_SIGNAL_FUNC (key_press), data);
		gtk_signal_connect (GTK_OBJECT (html->priv->search_input_line), "changed",
				    GTK_SIGNAL_FUNC (changed), data);
		gtk_signal_connect (GTK_OBJECT (html->priv->search_input_line), "destroy",
				    GTK_SIGNAL_FUNC (destroy), data);
	} else {
		gtk_widget_show (GTK_WIDGET (html->priv->search_input_line));
		data = gtk_object_get_data (GTK_OBJECT (html->priv->search_input_line), "search_data");
	}

	data->forward   = forward;
	data->changed   = FALSE;
	data->last_text = NULL;

	if (html->engine->search_info) {
		data->last_text = g_strdup (html->engine->search_info->text);
		html_search_set_text (html->engine->search_info, "");
	}

	gtk_widget_grab_focus (GTK_WIDGET (html->priv->search_input_line));
	data->focus_out = gtk_signal_connect (GTK_OBJECT (html->priv->search_input_line), "focus_out_event",
					      GTK_SIGNAL_FUNC (focus_out_event), data);
}

gboolean
gtk_html_engine_search (GtkHTML *html, const gchar *text, gboolean case_sensitive, gboolean forward, gboolean regular)
{
	return html_engine_search (html->engine, text, case_sensitive, forward, regular);
}

void
gtk_html_engine_search_set_forward (GtkHTML *html, gboolean forward)
{
	html_engine_search_set_forward (html->engine, forward);
}

gboolean
gtk_html_engine_search_next (GtkHTML *html)
{
	return html_engine_search_next (html->engine);
}

gboolean
gtk_html_engine_search_incremental (GtkHTML *html, const gchar *text, gboolean forward)
{
	return html_engine_search_incremental (html->engine, text, forward);
}
