
/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library.
   
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
*/

#include <config.h>
#include <gtk/gtk.h>
#include <string.h>
#include "gtkhtml.h"
#include "htmltokenizer.h"
#include "gtkhtml-private.h"
#include "htmlcolorset.h"
#include "htmlgdkpainter.h"
#include "htmlprinter.h"
#include "htmlframe.h"
#include "htmlengine-search.h"
#include "htmlsearch.h"
#include "htmlselection.h"
#include "htmlsettings.h"

#ifndef USE_SCROLLED_WINDOW
#include <gal/widgets/e-scroll-frame.h>
#endif

HTMLFrameClass html_frame_class;
static HTMLEmbeddedClass *parent_class = NULL;
static gboolean calc_size (HTMLObject *o, HTMLPainter *painter, GList **changed_objs);

static void
frame_url_requested (GtkHTML *html, const char *url, GtkHTMLStream *handle, gpointer data)
{
	HTMLFrame *frame = HTML_FRAME (data);
	GtkHTML *parent = GTK_HTML (HTML_EMBEDDED(frame)->parent);
	char *new_url = NULL;
	
	/* FIXME this is not exactly the single safest method of expanding a relative url */
	if (frame->url && !strstr (url, ":")) {
		char *base = g_strdup (frame->url);
		if (*url == '/' && strstr (base, ":")) {
			int i = 0;
			char *cur = base; 
			
			while (*cur != '\0') {
				if (*cur == '/') i++;
				
				if (i == 3) {
					*(++cur) = '\0';
					break;
				}
				cur++;
			}
			new_url = g_strconcat (base, "/", url, NULL);
		} else if (*url == '/' && (*base == '/' || *base == '\0')) {
			new_url = NULL;
		} else {
			new_url = g_strconcat (base, "/", url, NULL);
		}		
		g_free (base);
	}
	g_warning ("url = %s, frame->url = %s, new_url = %s", url, frame->url, new_url);
	
	gtk_signal_emit_by_name (GTK_OBJECT (parent->engine), "url_requested", new_url ? new_url : url,
				 handle);
	
	g_free (new_url);
}

static void
frame_set_base (GtkHTML *html, const gchar *base, gpointer data)
{
	HTMLFrame *frame = HTML_FRAME (data);
	char *new_url = NULL;
	char *cur;
	
	/* FIXME this is not exactly the single safest method of expanding a relative url */
	if (frame->url && !strstr (base, ":"))
		new_url = g_strconcat (frame->url, "/", base, NULL);
	else 
		new_url = g_strdup (base);

	cur = new_url + strlen (new_url);
	while (cur >= new_url) {
		if (*cur == '/') {
			break;
		}
		cur--;
	}	
	*(cur + 1)= '\0';
			
	g_warning ("base = %s, frame->url = %s, new_url = %s", base, frame->url, new_url);

	g_free (frame->url);
	frame->url = new_url;
}

static void
frame_on_url (GtkHTML *html, const gchar *url, gpointer data)
{
	HTMLFrame *frame = HTML_FRAME (data);
	GtkHTML *parent = GTK_HTML (HTML_EMBEDDED(frame)->parent);

	gtk_signal_emit_by_name (GTK_OBJECT (parent), "on_url", url);
}

static void
frame_link_clicked (GtkHTML *html, const gchar *url, gpointer data)
{
	HTMLFrame *frame = HTML_FRAME (data);
	GtkHTML *parent = GTK_HTML (HTML_EMBEDDED(frame)->parent);

	gtk_signal_emit_by_name (GTK_OBJECT (parent), "link_clicked", url);
}

static void
frame_submit (GtkHTML *html, 
	      const gchar *method, 
	      const gchar *action, 
	      const gchar *encoding, 
	      gpointer data) 
{
	HTMLFrame *frame = HTML_FRAME (data);
	GtkHTML *parent = GTK_HTML (HTML_EMBEDDED(frame)->parent);

	gtk_signal_emit_by_name (GTK_OBJECT (parent), "submit", method, action, encoding);
}

static void
frame_size_changed (GtkHTML *html, gpointer data)
{
	HTMLFrame *frame = HTML_FRAME (data);
	GtkHTML *parent = GTK_HTML (HTML_EMBEDDED(frame)->parent);
	
	html_engine_schedule_update (parent->engine);
}

static gboolean
frame_object_requested (GtkHTML *html, GtkHTMLEmbedded *eb, gpointer data)
{
	HTMLFrame *frame = HTML_FRAME (data);
	GtkHTML *parent = GTK_HTML (HTML_EMBEDDED(frame)->parent);
	gboolean ret_val;

	ret_val = FALSE;
	gtk_signal_emit_by_name (GTK_OBJECT (parent), "object_requested", eb, &ret_val);
	return ret_val;
}

static void
frame_set_gdk_painter (HTMLFrame *frame, HTMLPainter *painter)
{
	if (painter)
		gtk_object_ref (GTK_OBJECT (painter));
	
	if (frame->gdk_painter)
		gtk_object_unref (GTK_OBJECT (frame->gdk_painter));

	frame->gdk_painter = painter;
}

HTMLObject *
html_frame_new (GtkWidget *parent, 
		 char *src, 
		 gint width, 
		 gint height,
		 gboolean border) 
{
	HTMLFrame *frame;
	
	frame = g_new (HTMLFrame, 1);
	
	html_frame_init (frame, 
			  &html_frame_class, 
			  parent,
			  src,
			  width,
			  height,
			  border);
	
	return HTML_OBJECT (frame);
}

static gboolean
html_frame_grab_cursor(GtkWidget *frame, GdkEvent *event)
{
	/* Keep the focus! Fight the power */
	return TRUE;
}

static gint
calc_min_width (HTMLObject *o,
		HTMLPainter *painter)
{
  int min_width;
  
  if (HTML_FRAME (o)->width < 0)
	  min_width =  html_engine_calc_min_width (GTK_HTML (HTML_FRAME (o)->html)->engine);
  else 
	  min_width = HTML_FRAME (o)->width;

  return min_width;
}

static void
set_max_width (HTMLObject *o, HTMLPainter *painter, gint max_width)
{
	HTMLEngine *e = GTK_HTML (HTML_FRAME (o)->html)->engine;

	o->max_width = max_width;
	html_object_set_max_width (e->clue, e->painter, max_width - e->leftBorder - e->rightBorder);
}

/* static void
reset (HTMLObject *o)
{
	HTMLFrame *frame;

	(* HTML_OBJECT_CLASS (parent_class)->reset) (o);
	frame = HTML_FRAME (o);
	html_object_reset (GTK_HTML (frame->html)->engine->clue);
} */

static void
draw_background (HTMLObject *self,
		 HTMLPainter *p,
		 gint x, gint y,
		 gint width, gint height,
		 gint tx, gint ty)
{
}

/* FIXME rodo - draw + set_painter is not much clean now, needs refactoring */

static void
draw (HTMLObject *o,
      HTMLPainter *p,
      gint x, gint y,
      gint width, gint height,
      gint tx, gint ty)
{
	HTMLFrame   *frame  = HTML_FRAME (o);
	HTMLEngine   *e       = GTK_HTML (frame->html)->engine;
	ArtIRect paint;

	if (GTK_OBJECT_TYPE (e->painter) == HTML_TYPE_PRINTER) {
		gint pixel_size = html_painter_get_pixel_size (e->painter);
		html_object_calc_intersection (o, &paint, x, y, width, height);
		if (art_irect_empty (&paint))
			return;

		html_object_draw (e->clue, e->painter,
				  x, y,
				  width - pixel_size * (e->leftBorder + e->rightBorder),
				  height - pixel_size * (e->topBorder + e->bottomBorder),
				  tx + pixel_size * e->leftBorder, ty + pixel_size * e->topBorder);
	} else
		(*HTML_OBJECT_CLASS (parent_class)->draw) (o, p, x, y, width, height, tx, ty);
}

static void
set_painter (HTMLObject *o, HTMLPainter *painter)
{
	HTMLFrame *frame;

	frame = HTML_FRAME (o);
	if (GTK_OBJECT_TYPE (GTK_HTML (frame->html)->engine->painter) != HTML_TYPE_PRINTER) {
		frame_set_gdk_painter (frame, GTK_HTML (frame->html)->engine->painter);
	}
	
	html_engine_set_painter (GTK_HTML (frame->html)->engine,
				 GTK_OBJECT_TYPE (painter) != HTML_TYPE_PRINTER ? frame->gdk_painter : painter);
}

static void
forall (HTMLObject *self,
	HTMLEngine *e,
	HTMLObjectForallFunc func,
	gpointer data)
{
	HTMLFrame *frame;

	frame = HTML_FRAME (self);
	(* func) (self, html_object_get_engine (self, e), data);
	html_object_forall (GTK_HTML (frame->html)->engine->clue, html_object_get_engine (self, e), func, data);
}

static gint
check_page_split (HTMLObject *self, gint y)
{
	return html_object_check_page_split (GTK_HTML (HTML_FRAME (self)->html)->engine->clue, y);
}

static gboolean
calc_size (HTMLObject *o, HTMLPainter *painter, GList **changed_objs)
{
	HTMLFrame *frame;
	HTMLEngine *e;
	gint width, height;
	gint old_width, old_ascent, old_descent;
	
	old_width = o->width;
	old_ascent = o->ascent;
	old_descent = o->descent;

	frame = HTML_FRAME (o);
	e     = GTK_HTML (frame->html)->engine;

	if ((frame->width < 0) && (frame->height < 0)) {
		e->width = o->max_width;
		html_engine_calc_size (e, changed_objs);

		height = html_engine_get_doc_height (e);
		width = html_engine_get_doc_width (e);

		gtk_widget_set_usize (frame->scroll, width, height);
		gtk_widget_queue_resize (frame->scroll);
		
		html_frame_set_scrolling (frame, GTK_POLICY_NEVER);

		o->width = width;
		o->ascent = height;
		o->descent = 0;
	} else
		return (* HTML_OBJECT_CLASS (parent_class)->calc_size) (o, painter, changed_objs);

	if (o->descent != old_descent
	    || o->ascent != old_ascent
	    || o->width != old_width)
		return TRUE;

	return FALSE;
}

static gboolean
search (HTMLObject *self, HTMLSearch *info)
{
	HTMLEngine *e = GTK_HTML (HTML_FRAME (self)->html)->engine;

	/* printf ("search\n"); */

	/* search_next? */
	if (info->stack && HTML_OBJECT (info->stack->data) == e->clue) {
		/* printf ("next\n"); */
		info->engine = GTK_HTML (GTK_HTML (HTML_FRAME (self)->html)->iframe_parent)->engine;
		html_search_pop (info);
		html_engine_unselect_all (e);
		return html_search_next_parent (info);
	}

	info->engine = e;
	html_search_push (info, e->clue);
	if (html_object_search (e->clue, info))
		return TRUE;
	html_search_pop (info);

	info->engine = GTK_HTML (GTK_HTML (HTML_FRAME (self)->html)->iframe_parent)->engine;
	/* printf ("FALSE\n"); */

	return FALSE;
}

static HTMLObject *
head (HTMLObject *self)
{
	return GTK_HTML (HTML_FRAME (self)->html)->engine->clue;
}

static HTMLObject *
tail (HTMLObject *self)
{
	return GTK_HTML (HTML_FRAME (self)->html)->engine->clue;
}

static HTMLEngine *
get_engine (HTMLObject *self, HTMLEngine *e)
{
	return GTK_HTML (HTML_FRAME (self)->html)->engine;
}

static HTMLObject*
check_point (HTMLObject *self,
	     HTMLPainter *painter,
	     gint x, gint y,
	     guint *offset_return,
	     gboolean for_cursor)
{
	HTMLEngine *e = GTK_HTML (HTML_FRAME (self)->html)->engine;

	if (x < self->x || x >= self->x + self->width
	    || y >= self->y + self->descent || y < self->y - self->ascent)
		return NULL;

	x -= self->x + e->leftBorder - e->x_offset;
	y -= self->y - self->ascent + e->topBorder - e->y_offset;

	if (for_cursor && (x < 0 || y < e->clue->y - e->clue->ascent))
		return html_object_check_point (e->clue, e->painter, 0, e->clue->y - e->clue->ascent,
						offset_return, for_cursor);

	if (for_cursor && (x > e->clue->width - 1 || y > e->clue->y + e->clue->descent - 1))
		return html_object_check_point (e->clue, e->painter, e->clue->width - 1, e->clue->y + e->clue->descent - 1,
						offset_return, for_cursor);

	return html_object_check_point (e->clue, e->painter, x, y, offset_return, for_cursor);
}

static gboolean
is_container (HTMLObject *self)
{
	return TRUE;
}

static void
append_selection_string (HTMLObject *self,
			 GString *buffer)
{
	html_object_append_selection_string (GTK_HTML (HTML_FRAME (self)->html)->engine->clue, buffer);
}

static gboolean
select_range (HTMLObject *self,
	      HTMLEngine *engine,
	      guint start,
	      gint length,
	      gboolean queue_draw)
{
	return html_object_select_range (GTK_HTML (HTML_FRAME (self)->html)->engine->clue,
					 GTK_HTML (HTML_FRAME (self)->html)->engine,
					 start, length, queue_draw);
}

static void
destroy (HTMLObject *o)
{
	HTMLFrame *frame = HTML_FRAME (o);

	frame_set_gdk_painter (frame, NULL);

	if (frame->html) {
		gtk_signal_disconnect_by_data (GTK_OBJECT (frame->html), o);
		frame->html = NULL;
	}
	g_free ((frame)->url);

	HTML_OBJECT_CLASS (parent_class)->destroy (o);
}

void
html_frame_set_margin_width (HTMLFrame *frame, gint margin_width)
{
	HTMLEngine *e;

	e = GTK_HTML (frame->html)->engine;

	e->leftBorder = e->rightBorder = margin_width;
	html_engine_schedule_redraw (e);
}

void
html_frame_set_margin_height (HTMLFrame *frame, gint margin_height)
{
	HTMLEngine *e;

	e = GTK_HTML (frame->html)->engine;

	e->bottomBorder = e->topBorder = margin_height;
	html_engine_schedule_redraw (e);
}

void
html_frame_set_scrolling (HTMLFrame *frame, GtkPolicyType scroll)
{
#if E_USE_SCROLLED_WINDOW
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (frame->scroll),
					scroll, scroll);
#else
	e_scroll_frame_set_policy (E_SCROLL_FRAME (frame->scroll),
				   scroll, scroll);
#endif					
}

void
html_frame_set_size (HTMLFrame *frame, gint width, gint height)
{
	g_return_if_fail (frame != NULL);

	if (width > 0)
		frame->width = width;

	if (height > 0)
		frame->height = height;
	
	gtk_widget_set_usize (frame->scroll, width, height);
}

void 
html_frame_init (HTMLFrame *frame,
		  HTMLFrameClass *klass,
		  GtkWidget *parent,
		  char *src,
		  gint width,
		  gint height,
		  gboolean border)
{
	HTMLEmbedded *em = HTML_EMBEDDED (frame);
	HTMLTokenizer *new_tokenizer;
	GtkWidget *new_widget;
	GtkHTML   *new_html;
	GtkHTML   *parent_html;
	GtkHTMLStream *handle;
	GtkWidget *scrolled_window;

	g_assert (GTK_IS_HTML (parent));
	parent_html = GTK_HTML (parent);

	html_embedded_init (em, HTML_EMBEDDED_CLASS (klass),
			    parent, NULL, NULL);
	
#if USE_SCROLLED_WINDOW
	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
#else
	scrolled_window = e_scroll_frame_new (NULL, NULL);
	e_scroll_frame_set_shadow_type (E_SCROLL_FRAME (scrolled_window), 
					border ? GTK_SHADOW_IN : GTK_SHADOW_NONE);

#endif

	new_widget = gtk_html_new ();
	new_html = GTK_HTML (new_widget);

	new_tokenizer = html_tokenizer_clone (parent_html->engine->ht);

	html_engine_set_tokenizer (new_html->engine, new_tokenizer);
	gtk_object_unref (GTK_OBJECT (new_tokenizer));
	new_tokenizer = NULL;

	gtk_html_set_default_content_type (new_html,
					   parent_html->priv->content_type);
	frame->html = new_widget;
	gtk_html_set_iframe_parent (new_html, parent, HTML_OBJECT (frame));
	gtk_container_add (GTK_CONTAINER (scrolled_window), new_widget);
	gtk_widget_show (new_widget);

	frame->width = width;
	frame->height = height;
	frame->gdk_painter = NULL;
	frame->url = NULL;

	if (src && strcmp (src, "")) {
		handle = gtk_html_begin (new_html);
		gtk_signal_emit_by_name (GTK_OBJECT (new_html->engine), 
					 "url_requested", src, handle);

		frame_set_base (new_html, src, frame);
	} else {		
		gtk_html_load_empty (new_html);
	}
	new_html->engine->clue->parent = HTML_OBJECT (frame);


	gtk_signal_connect (GTK_OBJECT (new_html), "url_requested",
			    GTK_SIGNAL_FUNC (frame_url_requested),
			    (gpointer)frame);
	gtk_signal_connect (GTK_OBJECT (new_html), "on_url",
			    GTK_SIGNAL_FUNC (frame_on_url), 
			    (gpointer)frame);
	gtk_signal_connect (GTK_OBJECT (new_html), "link_clicked",
			    GTK_SIGNAL_FUNC (frame_link_clicked),
			    (gpointer)frame);	
	gtk_signal_connect (GTK_OBJECT (new_html), "size_changed",
			    GTK_SIGNAL_FUNC (frame_size_changed),
			    (gpointer)frame);	
	gtk_signal_connect (GTK_OBJECT (new_html), "object_requested",
			    GTK_SIGNAL_FUNC (frame_object_requested),
			    (gpointer)frame);	
	gtk_signal_connect (GTK_OBJECT (new_html), "submit",
			    GTK_SIGNAL_FUNC (frame_submit),
			    (gpointer)frame);

	html_frame_set_margin_height (frame, 0);
	html_frame_set_margin_width (frame, 0);
	/*
	  gtk_signal_connect (GTK_OBJECT (new_html), "set_base",
	  GTK_SIGNAL_FUNC (frame_set_base), (gpointer)frame);

	  gtk_signal_connect (GTK_OBJECT (html), "button_press_event",
	  GTK_SIGNAL_FUNC (frame_button_press_event), frame);
	*/

	gtk_widget_set_usize (scrolled_window, width, height);

	gtk_widget_show (scrolled_window);	
	frame->scroll = scrolled_window;
	html_frame_set_scrolling (frame, GTK_POLICY_AUTOMATIC);

	html_embedded_set_widget (em, scrolled_window);

	gtk_signal_connect(GTK_OBJECT (scrolled_window), "button_press_event",
			   GTK_SIGNAL_FUNC (html_frame_grab_cursor), NULL);

	/* inherit the current colors from our parent */
	html_colorset_set_unchanged (new_html->engine->defaultSettings->color_set,
				     parent_html->engine->settings->color_set);
	html_colorset_set_unchanged (new_html->engine->settings->color_set,
				     parent_html->engine->settings->color_set);
	html_painter_set_focus (new_html->engine->painter, parent_html->engine->have_focus);
	/*
	gtk_signal_connect (GTK_OBJECT (html), "title_changed",
			    GTK_SIGNAL_FUNC (title_changed_cb), (gpointer)app);
	gtk_signal_connect (GTK_OBJECT (html), "button_press_event",
			    GTK_SIGNAL_FUNC (on_button_press_event), popup_menu);
	gtk_signal_connect (GTK_OBJECT (html), "redirect",
			    GTK_SIGNAL_FUNC (on_redirect), NULL);
	gtk_signal_connect (GTK_OBJECT (html), "object_requested",
			    GTK_SIGNAL_FUNC (object_requested_cmd), NULL);
	*/
}

void
html_frame_type_init (void)
{
	html_frame_class_init (&html_frame_class, HTML_TYPE_FRAME, sizeof (HTMLFrame));
}

void
html_frame_class_init (HTMLFrameClass *klass,
			HTMLType type,
		        guint size) 
{
	HTMLEmbeddedClass *embedded_class;
	HTMLObjectClass  *object_class;

	g_return_if_fail (klass != NULL);
	
	embedded_class = HTML_EMBEDDED_CLASS (klass);
	object_class = HTML_OBJECT_CLASS (klass);

	html_embedded_class_init (embedded_class, type, size);
	parent_class = &html_embedded_class;

	object_class->destroy                 = destroy;
	object_class->calc_size               = calc_size;
	object_class->calc_min_width          = calc_min_width;
	object_class->set_painter             = set_painter;
	/* object_class->reset                   = reset; */
	object_class->draw                    = draw;
	object_class->set_max_width           = set_max_width;
	object_class->forall                  = forall;
	object_class->check_page_split        = check_page_split;
	object_class->search                  = search;
	object_class->head                    = head;
	object_class->tail                    = tail;
	object_class->get_engine              = get_engine;
	object_class->check_point             = check_point;
	object_class->is_container            = is_container;
	object_class->draw_background         = draw_background;
	object_class->append_selection_string = append_selection_string;
	object_class->select_range            = select_range;
}



