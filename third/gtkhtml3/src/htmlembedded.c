/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
    Copyright (C) 1997 Torben Weis (weis@kde.org)
    Copyright (C) 1999 Helix Code, Inc.

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
#include <stdio.h>
#include <gtk/gtksignal.h>
#include "gtkhtml.h"
#include "gtkhtml-embedded.h"
#include "htmlembedded.h"
#include "htmlframe.h"
#include "htmliframe.h"
#include "htmlpainter.h"
#include "htmlengine.h"

HTMLEmbeddedClass html_embedded_class;
static HTMLObjectClass *parent_class = NULL;

#define d(x)

static void
copy (HTMLObject *self,
      HTMLObject *dest)
{
	(* HTML_OBJECT_CLASS (parent_class)->copy) (self, dest);

	/* FIXME g_warning ("HTMLEmbedded::copy is not complete."); */

	HTML_EMBEDDED (dest)->name = g_strdup (HTML_EMBEDDED (self)->name);
	HTML_EMBEDDED (dest)->value = g_strdup (HTML_EMBEDDED (self)->value);
	HTML_EMBEDDED (dest)->form = HTML_EMBEDDED (self)->form;

	HTML_EMBEDDED (dest)->widget = NULL;
	HTML_EMBEDDED (dest)->parent = NULL;

	HTML_EMBEDDED (dest)->abs_x = HTML_EMBEDDED (self)->abs_x;
	HTML_EMBEDDED (dest)->abs_y = HTML_EMBEDDED (self)->abs_y;
}

static void
draw (HTMLObject *o,
      HTMLPainter *p,
      gint x, gint y,
      gint width, gint height,
      gint tx, gint ty)
{
	HTMLEmbedded *element = HTML_EMBEDDED(o);
	gint new_x, new_y;

	d (printf ("draw embedded %p\n", element));
	if (!element->widget)
		return;

	if (element->parent) {
		new_x = o->x + tx;
		new_y = o->y + ty - o->ascent;
		
		if (element->widget->parent) {
			if (new_x != element->abs_x || new_y != element->abs_y) {
				d (printf ("element: %p moveto: %d,%d shown: %d\n", element, new_x, new_y, GTK_WIDGET_VISIBLE (element->widget)));
				gtk_layout_move (GTK_LAYOUT(element->parent), element->widget, new_x, new_y);
			} else if (!GTK_HTML (element->parent)->engine->expose)
				gtk_widget_queue_draw (element->widget);
		}
	
		element->abs_x = new_x;
		element->abs_y = new_y;
		
		if (!element->widget->parent) {
			d (printf ("element: %p put: %d,%d shown: %d\n", element, new_x, new_y, GTK_WIDGET_VISIBLE (element->widget)));
			gtk_layout_put (GTK_LAYOUT(element->parent), element->widget, new_x, new_y);
		}
	}

	d (printf ("draw embedded %p - call painter\n", element));
	html_painter_draw_embedded (p, element, tx, ty);
}

static void
destroy (HTMLObject *o)
{
	HTMLEmbedded *element;

	d (printf ("destroy embedded %p\n", o));
	element = HTML_EMBEDDED (o);

	if(element->name)
		g_free(element->name);
	if(element->value)
		g_free(element->value);
	if(element->widget) {
		gtk_widget_hide (element->widget);
		g_signal_handlers_disconnect_matched (element->widget, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, element);
		if (element->changed_id > 0)
			g_signal_handler_disconnect (element->widget, element->changed_id);
		g_object_set_data (G_OBJECT (element->widget), "embeddedelement", NULL);
		if (element->widget->parent && element->parent) {
			g_assert (element->widget->parent == element->parent);
			gtk_container_remove (GTK_CONTAINER (element->parent), element->widget);
		} else {
		       	gtk_object_sink (GTK_OBJECT (element->widget));
		}
	}

	HTML_OBJECT_CLASS (parent_class)->destroy (o);
}

static void
reset (HTMLEmbedded *e)
{
	/* Nothing to do?  */
}

static gint
calc_min_width (HTMLObject *self,
		HTMLPainter *painter)
{
	GtkRequisition requisition;
	GtkWidget *widget;
	gint pixel_size;
	gint min_width;

	widget = HTML_EMBEDDED (self)->widget;

	if (widget == NULL || !GTK_WIDGET_VISIBLE (widget))
		return 0;
     
	requisition.width = requisition.height = 0;
	gtk_widget_size_request (widget, &requisition);
	pixel_size = html_painter_get_pixel_size (painter);

	min_width = requisition.width * pixel_size;

	return min_width;
}

static gboolean
html_embedded_real_calc_size (HTMLObject *self, HTMLPainter *painter, GList **changed_objs)
{
	GtkWidget *widget;
	HTMLEmbedded *emb = HTML_EMBEDDED (self);
	gint pixel_size;
	gint old_width, old_ascent, old_descent;
	GtkRequisition requisition;

	widget = emb->widget;
	if (widget == NULL)
		return FALSE;

	pixel_size = html_painter_get_pixel_size (painter);

	old_width = self->width;
	old_ascent = self->ascent;
	old_descent = self->descent;

	requisition.width = requisition.height = 0;
	gtk_widget_size_request (widget, &requisition);
	
	if (GTK_IS_HTML_EMBEDDED(widget))
		self->descent = GTK_HTML_EMBEDDED (widget)->descent * pixel_size;
	else
		self->descent = 0;

	self->width  = requisition.width  * pixel_size;
	self->ascent = requisition.height * pixel_size - self->descent;

	if (old_width != self->width || old_ascent != self->ascent || old_ascent != self->descent)
		return TRUE;

	return FALSE;
}

static gboolean
accepts_cursor (HTMLObject *o)
{
	return TRUE;
}

static gchar *
encode (HTMLEmbedded *e)
{
	return g_strdup ("");
}

void
html_embedded_reset (HTMLEmbedded *e)
{
	HTML_EMBEDDED_CLASS (HTML_OBJECT (e)->klass)->reset (e);
}

gchar *
html_embedded_encode (HTMLEmbedded *e)
{
	return HTML_EMBEDDED_CLASS (HTML_OBJECT (e)->klass)->encode (e);
}

void
html_embedded_reparent (HTMLEmbedded *e, GtkWidget *new_parent)
{
	HTML_EMBEDDED_CLASS (HTML_OBJECT (e)->klass)->reparent (e, new_parent);
}

void
html_embedded_set_form (HTMLEmbedded *e, HTMLForm *form)
{
	e->form = form;
}

gchar *
html_embedded_encode_string (const gchar *str)
{
        static gchar *safe = "$-._!*(),"; /* RFC 1738 */
        unsigned pos = 0;
        GString *encoded = g_string_new ("");
        gchar buffer[5], *ptr;
	guchar c;
	
        while ( pos < strlen(str) ) {

		c = (unsigned char) str[pos];
			
		if ( (( c >= 'A') && ( c <= 'Z')) ||
		     (( c >= 'a') && ( c <= 'z')) ||
		     (( c >= '0') && ( c <= '9')) ||
		     (strchr(safe, c))
		     )
			{
				encoded = g_string_append_c (encoded, c);
			}
		else if ( c == ' ' )
			{
				encoded = g_string_append_c (encoded, '+');
			}
		else if ( c == '\n' )
			{
				encoded = g_string_append (encoded, "%0D%0A");
			}
		else if ( c != '\r' )
			{
				sprintf( buffer, "%%%02X", (int)c );
				encoded = g_string_append (encoded, buffer);
				}
		pos++;
	}
	
	ptr = encoded->str;

	g_string_free (encoded, FALSE);

        return ptr;
}

void
html_embedded_type_init (void)
{
	html_embedded_class_init (&html_embedded_class, HTML_TYPE_EMBEDDED, sizeof (HTMLEmbedded));
}

void
html_embedded_class_init (HTMLEmbeddedClass *klass, 
			  HTMLType type,
			  guint size)
{
	HTMLObjectClass *object_class;

	g_return_if_fail (klass != NULL);

	object_class = HTML_OBJECT_CLASS (klass);
	html_object_class_init (object_class, type, size);

	/* HTMLEmbedded methods.   */
	klass->reset = reset;
	klass->encode = encode;

	/* HTMLObject methods.   */
	object_class->destroy = destroy;
	object_class->copy = copy;
	object_class->draw = draw;
	object_class->accepts_cursor = accepts_cursor;
	object_class->calc_size = html_embedded_real_calc_size;
	object_class->calc_min_width = calc_min_width;

	parent_class = &html_object_class;
}

void
html_embedded_init (HTMLEmbedded *element, 
		   HTMLEmbeddedClass *klass, 
		   GtkWidget *parent, 
		   gchar *name, 
		   gchar *value)
{
	HTMLObject *object;

	d (printf ("embedded %p init\n", element));

	object = HTML_OBJECT (element);
	html_object_init (object, HTML_OBJECT_CLASS (klass));

	element->form = NULL;
	if (name)
		element->name = g_strdup(name);
	else
		element->name = g_strdup("");
	if (value)
		element->value = g_strdup(value);
	else
		element->value = g_strdup("");
	element->widget = NULL;
	element->parent = parent;
	element->width  = 0;
	element->height = 0;
	element->abs_x  = element->abs_y = -1;
	element->changed_id = 0;
}


static gboolean
html_embedded_grab_cursor(GtkWidget *eb, GdkEvent *event)
{
	/* Keep the focus! Fight the power */
	return TRUE;
}

/* called when some state in an embedded html object has changed ... do a redraw */
static void
html_embedded_object_changed (GtkHTMLEmbedded *eb, HTMLEngine *e)
{
	HTMLObject *object;

	object = HTML_OBJECT (g_object_get_data (G_OBJECT (eb), "embeddedelement"));
	if (object)
		html_object_calc_size (object, e->painter, FALSE);
	
	html_engine_schedule_update(e);
}

HTMLEmbedded *
html_embedded_new_widget (GtkWidget *parent, GtkHTMLEmbedded *eb, HTMLEngine *engine)
{
	HTMLEmbedded *em;

	em = g_new0(HTMLEmbedded, 1);
	d (printf ("embedded %p new widget\n", em));

	html_embedded_init (em, HTML_EMBEDDED_CLASS (&html_embedded_class), parent, eb->name, "");
	html_embedded_set_widget (em, GTK_WIDGET (eb));

	/* pass em as the user_data so that the handler will disconnect 
	 * when the object is destoyed
	 */
	g_signal_connect(eb, "button_press_event", G_CALLBACK (html_embedded_grab_cursor), em);
	em->changed_id = g_signal_connect (eb, "changed", G_CALLBACK (html_embedded_object_changed), engine);
	/* printf ("id %u\n", em->changed_id); */

	return em;
}

static void
html_embedded_allocate (GtkWidget *w, GtkAllocation  *allocation, HTMLEmbedded *e)
{
	if (e->width != allocation->width || e->height != allocation->height) {
		if (e->width != allocation->width) {
			html_object_change_set (HTML_OBJECT (e), HTML_CHANGE_ALL_CALC);
			e->width = allocation->width;
		}
		e->height = allocation->height;
		
		if (GTK_IS_HTML (w->parent))
			html_engine_schedule_update (GTK_HTML (w->parent)->engine);
	}
}

void
html_embedded_set_widget (HTMLEmbedded *emb, GtkWidget *w)
{
	emb->widget = w;
	
	d (printf ("set embedded widget: %p widget: %p\n", emb, w));
	gtk_widget_show (w);

	g_object_set_data (G_OBJECT (w), "embeddedelement", emb);
	g_signal_connect (w, "size_allocate", G_CALLBACK (html_embedded_allocate), emb);
}

GtkWidget *
html_embedded_get_widget (HTMLEmbedded *e)
{
	return e->widget;
}

gboolean
html_object_is_embedded (HTMLObject *o)
{
	gboolean rv = FALSE;

	switch (o->klass->type) {
	case HTML_TYPE_EMBEDDED:
	case HTML_TYPE_TEXTINPUT:
	case HTML_TYPE_BUTTON:
	case HTML_TYPE_IMAGEINPUT:
	case HTML_TYPE_TEXTAREA:
	case HTML_TYPE_HIDDEN:
	case HTML_TYPE_RADIO:
	case HTML_TYPE_CHECKBOX:
	case HTML_TYPE_SELECT:
	case HTML_TYPE_IFRAME:
	case HTML_TYPE_FRAME:
		rv = TRUE;
	default:
		;
	}

	return rv;
}

gboolean
html_object_is_frame (HTMLObject *o)
{
	return HTML_IS_FRAME (o) || HTML_IS_IFRAME (o);
}
