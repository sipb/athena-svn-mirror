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
#include "gtkhtmldebug.h"
#include "htmlcolor.h"
#include "htmlcolorset.h"
#include "htmlengine.h"
#include "htmlengine-edit.h"
#include "htmlengine-save.h"
#include "htmlimage.h"
#include "htmlpainter.h"
#include "htmlsearch.h"
#include "htmltable.h"
#include "htmltablepriv.h"
#include "htmltablecell.h"

#include "htmlshape.h"
#include "htmlframe.h"
#include "htmlframeset.h"


HTMLFramesetClass html_frameset_class;
static HTMLObjectClass *parent_class = NULL;

static gint
html_frameset_get_view_height (HTMLFrameset *set)
{
	HTMLObject *o = HTML_OBJECT (set);

	while (o->parent != NULL) {
		if (HTML_OBJECT_TYPE (o->parent) == HTML_TYPE_FRAMESET) {
			return (o->ascent + o->descent);
		}
		o = o->parent;
	}
	return html_engine_get_view_height (set->parent->engine);
}

static gint
html_frameset_get_view_width (HTMLFrameset *set)
{
	HTMLObject *o = HTML_OBJECT (set);

	while (o->parent != NULL) {
		if (HTML_OBJECT_TYPE (o->parent) == HTML_TYPE_FRAMESET) {
			return html_engine_get_view_width (HTML_FRAMESET (o->parent)->parent->engine);
		}

		o = o->parent;
	}
	return html_engine_get_view_width (set->parent->engine);
}

gboolean
html_frameset_append (HTMLFrameset *set, HTMLObject *frame)
{
	g_return_val_if_fail (frame != NULL, FALSE);
	g_return_val_if_fail (set != NULL, FALSE);

	if (set->frames->len >= set->cols->len * set->rows->len)
		return FALSE;

		
	g_ptr_array_add (set->frames, frame);
	html_object_set_parent (frame, HTML_OBJECT (set));
	return TRUE;
}

static void	       
calc_dimension (GPtrArray *dim, gint *span, gint total)
{
	HTMLLength *len;
	int i;
	int adj;
	int remain;
	int num_frac = 0;

	remain = total;
	for (i = 0; i < dim->len; i++) {
		len = g_ptr_array_index (dim, i);
		span[i] = 0;

		if (len->type == HTML_LENGTH_TYPE_PIXELS)
			span[i] = len->val;
                else if (len->type == HTML_LENGTH_TYPE_FRACTION)
			num_frac += len->val;
		else if (len->type == HTML_LENGTH_TYPE_PERCENT)
			span[i] = (total * len->val) / 100;  

		remain -= span[i];
	}

	
	if (remain > 0 && num_frac) {
		adj = remain / num_frac;
		for (i = 0; i < dim->len; i++) {
			len = g_ptr_array_index (dim, i);
			if (len->type == HTML_LENGTH_TYPE_FRACTION) {
				span[i] = adj * len->val;
				remain -= span[i];
			} 
		}
	}

	adj = 1;
	if (remain < 0)
		adj = -1;

	i = 0;	
	while (remain != 0) {	       
		if (span[i] > 0) {
			span[i] += adj;
			remain -= adj;
		} 
		
		i++;
		if (i >= dim->len)
			i = 0;
	}
}

static gboolean
calc_size (HTMLObject *o, HTMLPainter *painter, GList **changed_objs)
{
	HTMLFrameset *set = HTML_FRAMESET (o);
	HTMLObject *frame = NULL;

	gint view_width;
	gint view_height;

	gint remain_x;
	gint remain_y;
	gint r, c, i;

	gint *widths;
	gint *heights;

	view_width = html_frameset_get_view_width (set);
	view_height = html_frameset_get_view_height (set);
	
	o->ascent = view_height;
	o->width = view_width;
	o->descent = 0;

	heights = g_malloc (set->rows->len * sizeof (gint));
	widths = g_malloc (set->cols->len * sizeof (gint));

	calc_dimension (set->cols, widths, view_width);
	calc_dimension (set->rows, heights, view_height);

	remain_y = view_height;
	for (r = 0; r < set->rows->len; r++) {

		remain_x = view_width;
		for (c = 0; c < set->cols->len; c++) {
			i = (r * set->cols->len) + c;

			if (i < set->frames->len) {
				frame = g_ptr_array_index (set->frames, i);
				
				if (HTML_OBJECT_TYPE (frame) == HTML_TYPE_FRAME)
					html_frame_set_size (HTML_FRAME (frame), widths[c], heights[r]);
				else {
					HTML_OBJECT (frame)->width = widths[c];
					HTML_OBJECT (frame)->ascent = heights[r];
					HTML_OBJECT (frame)->descent = 0;
				}
				html_object_calc_size (HTML_OBJECT (frame), painter, changed_objs);
				
				HTML_OBJECT (frame)->x = view_width - remain_x;
				HTML_OBJECT (frame)->y = view_height + heights[r] - remain_y;

			}	
			
			remain_x -= widths[c];
		}
		
		remain_y -= heights[r];
	}		

	g_free (widths);
	g_free (heights);
	return TRUE;
}

void
html_frameset_init (HTMLFrameset *set, GtkHTML *parent, char *rows, char *cols)
{
	html_object_init (HTML_OBJECT (set), HTML_OBJECT_CLASS (&html_frameset_class));
	set->parent = parent;
	
	set->cols = NULL;
	set->rows = NULL;

	set->cols = g_ptr_array_new ();
	set->rows = g_ptr_array_new ();

	if (cols == NULL)
		cols = "100%";

	html_length_array_parse (set->cols, cols);

	if (rows == NULL) 
		rows = "100%";
	
	html_length_array_parse (set->rows, rows);

	set->frames = g_ptr_array_new ();
}


static void
draw (HTMLObject *o,
      HTMLPainter *p,
      gint x, gint y,
      gint width, gint height,
      gint tx, gint ty)
{
	HTMLFrameset *set;
	int i;
	set = HTML_FRAMESET (o);

	tx += o->x;
	ty += o->y - o->ascent;

	
	/* Do nothing by default.  We don't know how to paint ourselves.  */
	for (i = 0; i < set->frames->len; i++) {
		html_object_draw (HTML_OBJECT (g_ptr_array_index (set->frames, i)),
				  p, x - o->x, y - o->y + o->ascent,
				  width, height,
				  tx, ty);
	}
}

/* static void
draw_background (HTMLObject *self,
		 HTMLPainter *painter,
		 gint x, gint y, 
		 gint width, gint height,
		 gint tx, gint ty)
{
	ArtIRect paint;
	GdkColor color;

	html_object_calc_intersection (self, &paint, x, y, width, height);
	if (art_irect_empty (&paint))
	    return;

	
	gdk_color_parse ("#000000", &color);
       	html_painter_draw_background (painter,
				      &color,
				      NULL,
				      tx + paint.x0,
				      ty + paint.y0,
				      paint.x1 - paint.x0,
				      paint.y1 - paint.y0,
				      paint.x0 - self->x,
				      paint.y0 - (self->y - self->ascent));
} */

static void
destroy (HTMLObject *self)
{
	HTMLFrameset *set;
	gint i;	

	set = HTML_FRAMESET (self);
	for (i = 0; i < set->frames->len; i++)
		html_object_destroy (g_ptr_array_index (set->frames, i));

	html_length_array_destroy (set->cols);
	html_length_array_destroy (set->rows);

	(* parent_class->destroy) (self);
}

static void
set_max_width (HTMLObject *o, HTMLPainter *painter, gint w)
{
	HTMLFrameset *set = HTML_FRAMESET (o);
	gint remain_x;
	gint c, i;
	gint *widths;

	(* HTML_OBJECT_CLASS (parent_class)->set_max_width) (o, painter, w);
	widths     = g_malloc (set->cols->len * sizeof (gint));

	calc_dimension (set->cols, widths, w);

	remain_x = w;
	for (i = 0; i < set->frames->len; i++) {
		c = i % set->cols->len;
		if (i < set->frames->len)
			html_object_set_max_width (HTML_OBJECT (g_ptr_array_index (set->frames, i)), painter, widths [c]);
		remain_x -= widths[c];
	}
	g_free (widths);
}

static HTMLObject*
check_point (HTMLObject *self,
	     HTMLPainter *painter,
	     gint x, gint y,
	     guint *offset_return,
	     gboolean for_cursor)
{
	HTMLFrameset *set = HTML_FRAMESET (self);
	HTMLObject   *obj;
	int i;


	x -= self->x;
	y -= self->y - self->ascent;

	for (i = 0; i < set->frames->len; i++) {
		obj = html_object_check_point (g_ptr_array_index (set->frames, i), painter, 
					       x, y, offset_return, for_cursor);

		if (obj != NULL)
			return obj;

	}
    
	return NULL;
}

static gboolean
is_container (HTMLObject *self)
{
	return TRUE;
}

/* static void
reset (HTMLObject *self)
{
	HTMLFrameset *set;
	gint i;	

	(* HTML_OBJECT_CLASS (parent_class)->reset) (self);
	set = HTML_FRAMESET (self);
	for (i = 0; i < set->frames->len; i++)
		html_object_reset (g_ptr_array_index (set->frames, i));

} */

static void
forall (HTMLObject *self,
	HTMLEngine *e,
	HTMLObjectForallFunc func,
	gpointer data)
{
	HTMLFrameset *set;
	gint i;

	set = HTML_FRAMESET (self);
	for (i = 0; i < set->frames->len; i++)
		     html_object_forall (g_ptr_array_index (set->frames, i), e, func, data);
	(* func) (self, e, data);
}
 

HTMLObject *
html_frameset_new (GtkHTML *parent, gchar *rows, gchar *cols)
{
	HTMLFrameset *set;

	set = g_new (HTMLFrameset, 1);

	html_frameset_init (set, parent, rows, cols);

	return HTML_OBJECT (set);
}

static gint
calc_min_width (HTMLObject *o, HTMLPainter *p)
{
	return -1;
}

void
html_frameset_type_init (void)
{
	html_frameset_class_init (&html_frameset_class, HTML_TYPE_FRAMESET, sizeof (HTMLFrameset));
}

void
html_frameset_class_init (HTMLFramesetClass *klass,
			  HTMLType type,
			  guint object_size) 
{
	HTMLObjectClass *object_class = HTML_OBJECT_CLASS (klass);

	html_object_class_init (object_class, type, object_size);

	object_class->calc_size =   calc_size;
	object_class->draw =        draw;
	object_class->destroy =     destroy;
	object_class->check_point = check_point;
	object_class->set_max_width = set_max_width;

	/* object_class->draw_background = draw_background; */
	object_class->forall = forall;
	/* object_class->reset = reset; */
	object_class->is_container = is_container;

	object_class->calc_min_width = calc_min_width;
	/*
	object_class->copy = copy;	
	object_class->op_copy = op_copy;
	object_class->op_cut = op_cut;
	object_class->split = split;
	object_class->merge = merge;
	object_class->accepts_cursor = accepts_cursor;
	object_class->destroy = destroy;
	object_class->calc_preferred_width = calc_preferred_width;
	object_class->find_anchor = find_anchor;
	object_class->is_container = is_container;
	object_class->search = search;
	object_class->fit_line = fit_line;
	object_class->append_selection_string = append_selection_string;
	object_class->head = head;
	object_class->tail = tail;
	object_class->next = next;
	object_class->prev = prev;
	object_class->save = save;
	object_class->save_plain = save_plain;
	object_class->check_page_split = check_page_split;
	object_class->get_bg_color = get_bg_color;
	object_class->get_recursive_length = get_recursive_length;
	object_class->remove_child = remove_child;
	*/

	parent_class = &html_object_class;
}




