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
#include <gdk/gdkkeysyms.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkentry.h>
#include <string.h>
#include <gal/widgets/e-unicode.h>

#include "htmltextinput.h"
#include "htmlform.h"


HTMLTextInputClass html_text_input_class;
static HTMLEmbeddedClass *parent_class = NULL;


/* HTMLObject methods.  */

static void
destroy (HTMLObject *o)
{
	HTMLTextInput *ti;

	ti = HTML_TEXTINPUT (o);

	if (ti->default_text)
		g_free (ti->default_text);

	HTML_OBJECT_CLASS (parent_class)->destroy (o);
}

static void
copy (HTMLObject *self,
      HTMLObject *dest)
{
	(* HTML_OBJECT_CLASS (parent_class)->copy) (self, dest);

	HTML_TEXTINPUT (dest)->size = HTML_TEXTINPUT (self)->size;
	HTML_TEXTINPUT (dest)->maxlen = HTML_TEXTINPUT (self)->maxlen;
	HTML_TEXTINPUT (dest)->password = HTML_TEXTINPUT (self)->password;
	HTML_TEXTINPUT (dest)->default_text = g_strdup (HTML_TEXTINPUT (self)->default_text);

	g_warning ("HTMLTextInput::copy is not complete");
}

static void
reset (HTMLEmbedded *e)
{
	e_utf8_gtk_entry_set_text (GTK_ENTRY(e->widget), HTML_TEXTINPUT(e)->default_text);
}

static gboolean
html_text_input_key_pressed (GtkWidget *w, GdkEventKey *ev, gpointer p) 
{
	HTMLEmbedded *e = HTML_EMBEDDED(p);
	HTMLEmbedded *next = NULL;
	HTMLEmbedded *current = NULL;
	gboolean found = FALSE;
	GList *node = NULL;
	
	if (ev->keyval == GDK_Return) {
		for (node = e->form->elements; node; node = node->next) {
			current = HTML_EMBEDDED(node->data);
			
			/* focus on the next visible element */
			if (current->widget && found
			    && HTML_OBJECT_TYPE(current) != HTML_TYPE_BUTTON
			    && HTML_OBJECT_TYPE(current) != HTML_TYPE_IMAGEINPUT) {
				next = current;
				break;
			}
			
			if (node->data == e)
				found = TRUE;
		}
		
		if (next)
			gtk_widget_grab_focus (next->widget);
		else if (found)
			html_form_submit (e->form);
		else
			g_warning ("Not in form's element list.  Couldn't focus successor.");
		
		gtk_signal_emit_stop_by_name(GTK_OBJECT(w), "key_press_event");
		return TRUE;
	}
	return FALSE;
}

/* HTMLEmbedded methods.  */

static gchar *
encode (HTMLEmbedded *e)
{
	GString *encoding = g_string_new ("");
	gchar *ptr;

	if(strlen (e->name)) {
		ptr = html_embedded_encode_string (e->name);
		encoding = g_string_append (encoding, ptr);
		g_free (ptr);

		encoding = g_string_append_c (encoding, '=');

		ptr = html_embedded_encode_string (e_utf8_gtk_entry_get_text (GTK_ENTRY (e->widget)));
		encoding = g_string_append (encoding, ptr);
		g_free (ptr);
	}

	ptr = encoding->str;
	g_string_free(encoding, FALSE);

	return ptr;
}


void
html_text_input_type_init (void)
{
	html_text_input_class_init (&html_text_input_class,
				    HTML_TYPE_TEXTINPUT,
				    sizeof (HTMLTextInput));
}

void
html_text_input_class_init (HTMLTextInputClass *klass,
			    HTMLType type,
			    guint object_size)
{
	HTMLEmbeddedClass *element_class;
	HTMLObjectClass *object_class;

	element_class = HTML_EMBEDDED_CLASS (klass);
	object_class = HTML_OBJECT_CLASS (klass);

	html_embedded_class_init (element_class, type, object_size);

	/* HTMLObject methods.   */
	object_class->destroy = destroy;
	object_class->copy = copy;

	/* HTMLEmbedded methods.   */
	element_class->reset = reset;
	element_class->encode = encode;

	parent_class = &html_embedded_class;
}

void
html_text_input_init (HTMLTextInput *ti,
		      HTMLTextInputClass *klass,
		      GtkWidget *parent,
		      gchar *name,
		      gchar *value,
		      gint size,
		      gint maxlen,
		      gboolean password)
{
	HTMLEmbedded *element;
	HTMLObject *object;
	GtkWidget *entry;
	gint min_width;

	element = HTML_EMBEDDED (ti);
	object = HTML_OBJECT (ti);

	html_embedded_init (element, HTML_EMBEDDED_CLASS (klass),
			   parent, name, value);

	entry = gtk_entry_new ();
	html_embedded_set_widget (element, entry);
	gtk_signal_connect_after (GTK_OBJECT (entry), "key_press_event",
				  GTK_SIGNAL_FUNC (html_text_input_key_pressed),
				  element);

	if (strlen (element->value))	
		e_utf8_gtk_entry_set_text (GTK_ENTRY(element->widget), element->value);

	ti->default_text = g_strdup (element->value);

	if (maxlen != -1)
		gtk_entry_set_max_length(GTK_ENTRY(element->widget), maxlen);

	gtk_entry_set_visibility (GTK_ENTRY(element->widget), !password);
	
	min_width = gdk_char_width (element->widget->style->font, '0') * size + 8;
	gtk_widget_set_usize (element->widget, min_width, -1);

	ti->size = size;
	ti->maxlen = maxlen;
}

HTMLObject *
html_text_input_new (GtkWidget *parent,
		     gchar *name,
		     gchar *value,
		     gint size,
		     gint maxlen,
		     gboolean password)
{
	HTMLTextInput *ti;

	ti = g_new0 (HTMLTextInput, 1);
	html_text_input_init (ti, &html_text_input_class,
			      parent, name, value, size,
			      maxlen, password);

	return HTML_OBJECT (ti);
}
