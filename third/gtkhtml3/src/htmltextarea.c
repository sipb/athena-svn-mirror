/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Jonas Borgström <jonas_b@bitsmart.com>.

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
#include <gtk/gtkeditable.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtktextview.h>
#include "htmltextarea.h"


HTMLTextAreaClass html_textarea_class;
static HTMLEmbeddedClass *parent_class = NULL;


static void
destroy (HTMLObject *o)
{
	HTMLTextArea *ta;

	ta = HTML_TEXTAREA (o);

	if (ta->default_text)
		g_free (ta->default_text);

	HTML_OBJECT_CLASS (parent_class)->destroy (o);
}

static void
copy (HTMLObject *self,
      HTMLObject *dest)
{
	(* HTML_OBJECT_CLASS (parent_class)->copy) (self, dest);

	/* FIXME TODO this is not going to work.  */

	HTML_TEXTAREA (dest)->text = NULL;
	HTML_TEXTAREA (dest)->default_text = g_strdup (HTML_TEXTAREA (self)->default_text);

	/* FIXME g_warning ("HTMLTextArea::copy is not complete."); */
}


static void
reset (HTMLEmbedded *e)
{
	html_textarea_set_text ( HTML_TEXTAREA (e), HTML_TEXTAREA (e)->default_text);
}

static gchar *
encode (HTMLEmbedded *e)
{
	GString *encoding = g_string_new ("");
	gchar *encoded_str, *utf8_str, *gtk_text;

	if(strlen (e->name)) {
		GtkTextIter first, last;

		utf8_str = html_embedded_encode_string (e->name);
		encoding = g_string_append (encoding, utf8_str);
		g_free (utf8_str);

		encoding = g_string_append_c (encoding, '=');

		gtk_text_buffer_get_bounds (HTML_TEXTAREA (e)->buffer, &first, &last);
		gtk_text = gtk_text_buffer_get_text (HTML_TEXTAREA (e)->buffer, &first, &last, FALSE);

		encoded_str = html_embedded_encode_string (gtk_text);
		encoding = g_string_append (encoding, encoded_str);

		g_free (encoded_str);
		g_free (gtk_text);
	}

	utf8_str = encoding->str;
	g_string_free(encoding, FALSE);

	return utf8_str;
}

void
html_textarea_type_init (void)
{
	html_textarea_class_init (&html_textarea_class, HTML_TYPE_TEXTAREA, sizeof (HTMLTextArea));
}

void
html_textarea_class_init (HTMLTextAreaClass *klass,
			  HTMLType type,
			  guint object_size)
{
	HTMLEmbeddedClass *element_class;
	HTMLObjectClass *object_class;

	element_class = HTML_EMBEDDED_CLASS (klass);
	object_class = HTML_OBJECT_CLASS (klass);

	html_embedded_class_init (element_class, type, object_size);

	/* HTMLEmbedded methods.   */
	element_class->reset = reset;
	element_class->encode = encode;

	/* HTMLObject methods.   */
	object_class->destroy = destroy;
	object_class->copy = copy;

	parent_class = &html_embedded_class;
}

void
html_textarea_init (HTMLTextArea *ta,
		      HTMLTextAreaClass *klass,
		      GtkWidget *parent,
		      gchar *name,
		      gint row,
		      gint col)
{
	GtkWidget *sw;
	HTMLEmbedded *element;
	HTMLObject *object;
	PangoLayout *layout;
	gint width, height;

	element = HTML_EMBEDDED (ta);
	object = HTML_OBJECT (ta);

	html_embedded_init (element, HTML_EMBEDDED_CLASS (klass),
			   parent, name, NULL);

	ta->buffer = gtk_text_buffer_new (NULL);
	ta->text = gtk_text_view_new_with_buffer (ta->buffer);
	gtk_text_view_set_editable (GTK_TEXT_VIEW (ta->text), TRUE);

	gtk_widget_set_events (ta->text, GDK_BUTTON_PRESS_MASK);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (sw), ta->text);
	gtk_widget_show_all (sw);
	html_embedded_set_widget (element, sw);

	layout = pango_layout_new (gtk_widget_get_pango_context (ta->text));
	pango_layout_set_font_description (layout, ta->text->style->font_desc);
	pango_layout_set_text (layout, "0", 1);
	pango_layout_get_size (layout, &width, &height);
	g_object_unref (layout);

	gtk_widget_set_size_request (ta->text, (width / PANGO_SCALE) * col + 8, (height / PANGO_SCALE) * row + 4);

	ta->default_text = NULL;
}

HTMLObject *
html_textarea_new (GtkWidget *parent,
		     gchar *name,
		     gint row,
		     gint col)
{
	HTMLTextArea *ta;

	ta = g_new0 (HTMLTextArea, 1);
	html_textarea_init (ta, &html_textarea_class,
			      parent, name, row, col);

	return HTML_OBJECT (ta);
}

void html_textarea_set_text (HTMLTextArea *ta, gchar *text) 
{
	GtkTextIter begin, end;

	if (!ta->default_text)
		ta->default_text = g_strdup (text);

	gtk_text_buffer_get_bounds (ta->buffer, &begin, &end);
	gtk_text_buffer_delete (ta->buffer, &begin, &end);
	gtk_text_buffer_get_bounds (ta->buffer, &begin, &end);
	gtk_text_buffer_insert (ta->buffer, &begin, text, strlen (text));
}
