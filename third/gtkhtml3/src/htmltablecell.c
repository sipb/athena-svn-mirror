/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
   Copyright (C) 1997 Martin Jones (mjones@kde.org)
   Copyright (C) 1997 Torben Weis (weis@kde.org)
   Copyright (C) 1999 Anders Carlsson (andersca@gnu.org)
   Copyright (C) 1999, 2000 Helix Code, Inc.

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
#include <glib.h>
#include <libart_lgpl/art_rect.h>
#include <string.h>
#include "htmlcluev.h"
#include "htmlengine-edit.h"
#include "htmlengine-save.h"
#include "htmlpainter.h"
#include "htmlplainpainter.h"
#include "htmltable.h"
#include "htmltablepriv.h"
#include "htmltablecell.h"

/* FIXME: This always behaves as a transparent object, even when it
   actually is not.  */


HTMLTableCellClass html_table_cell_class;
static HTMLClueVClass *parent_class = NULL;


static void
draw_background_helper (HTMLTableCell *cell,
			HTMLPainter *p,
			GdkRectangle *paint,
			gint tx, gint ty)
{
	HTMLObject *o;
	HTMLTable  *t;
	GdkPixbuf  *pixbuf = NULL;
	GdkColor   *color = NULL;

	o = HTML_OBJECT (cell);
	t = HTML_IS_TABLE (o->parent) ? HTML_TABLE (o->parent) : NULL;

	if (cell->have_bg) {
		if (! cell->bg_allocated) {
			html_painter_alloc_color (p, &cell->bg);
			cell->bg_allocated = TRUE;
		}
		color = &cell->bg;
	}

	if (cell->have_bgPixmap) {
		if (cell->bgPixmap->animation) {
			pixbuf = gdk_pixbuf_animation_get_static_image (cell->bgPixmap->animation);
		}
	}
	if (!HTML_IS_PLAIN_PAINTER (p))
		html_painter_draw_background (p,
					      color,
					      pixbuf,
					      tx + paint->x,
					      ty + paint->y,
					      paint->width,
					      paint->height,
					      paint->x - o->x,
					      paint->y - (o->y - o->ascent));
}


/* HTMLObject methods.  */

static void
reset (HTMLObject *self)
{
	HTMLTableCell *cell;

	cell = HTML_TABLE_CELL (self);

	cell->bg_allocated = FALSE;

	(* HTML_OBJECT_CLASS (parent_class)->reset) (self);
}

static void
copy (HTMLObject *self, HTMLObject *dest)
{
	memcpy (dest, self, sizeof (HTMLTableCell));

	(* HTML_OBJECT_CLASS (parent_class)->copy) (self, dest);
}

static gboolean
merge (HTMLObject *self, HTMLObject *with, HTMLEngine *e, GList **left, GList **right, HTMLCursor *cursor)
{
	HTMLTableCell *c1 = HTML_TABLE_CELL (self);
	HTMLTableCell *c2 = HTML_TABLE_CELL (with);

	/* g_print ("merge cells %d,%d %d,%d\n", c1->row, c1->col, c2->row, c2->col); */

	if (HTML_OBJECT_TYPE (with) == HTML_TYPE_CLUEV || (c1->col == c2->col && c1->row == c2->row)) {
		gboolean rv;
		rv = (* HTML_OBJECT_CLASS (parent_class)->merge) (self, with, e, left, right, cursor);
		if (rv && with->parent && HTML_IS_TABLE (with->parent)) {
			self->next = NULL;
			html_object_remove_child (with->parent, with);
			/* FIXME spanning */
			html_table_set_cell (HTML_TABLE (self->parent),
					     HTML_TABLE_CELL (self)->row, HTML_TABLE_CELL (self)->col,
					     HTML_TABLE_CELL (self));
		}

		return rv;
	} else
		return FALSE;
}

static gint
calc_min_width (HTMLObject *o,
		HTMLPainter *painter)
{
	if (HTML_TABLE_CELL (o)->no_wrap)
		return MAX ((* HTML_OBJECT_CLASS (parent_class)->calc_min_width) (o, painter),
			    o->flags & HTML_OBJECT_FLAG_FIXEDWIDTH
			    ? HTML_TABLE_CELL (o)->fixed_width * html_painter_get_pixel_size (painter)
			    : 0);

	return (* HTML_OBJECT_CLASS (parent_class)->calc_min_width) (o, painter);
}

static gint
calc_preferred_width (HTMLObject *o,
		      HTMLPainter *painter)
{
	return o->flags & HTML_OBJECT_FLAG_FIXEDWIDTH
		? MAX (html_object_calc_min_width (o, painter), 
		       HTML_TABLE_CELL (o)->fixed_width * html_painter_get_pixel_size (painter))
		: (* HTML_OBJECT_CLASS (parent_class)->calc_preferred_width) (o, painter);
}

static void
draw (HTMLObject *o,
      HTMLPainter *p,
      gint x, gint y, 
      gint width, gint height,
      gint tx, gint ty)
{
	HTMLTableCell *cell = HTML_TABLE_CELL (o);
	GdkRectangle paint;

	
	if (!html_object_intersect (o, &paint, x, y, width, height))
		return;
	
	draw_background_helper (cell, p, &paint, tx, ty);

	(* HTML_OBJECT_CLASS (&html_cluev_class)->draw) (o, p, x, y, width, height, tx, ty);
}

static void
clue_move_children (HTMLClue *clue, gint x_delta, gint y_delta)
{
	HTMLObject *o;

	for (o = clue->head; o; o = o->next) {
		o->x += x_delta;
		o->y += y_delta;
	}
}

static gboolean
html_table_cell_real_calc_size (HTMLObject *o, HTMLPainter *painter, GList **changed_objs)
{
	HTMLTableCell *cell;
	gboolean rv;
	gint old_width, old_height;

	old_width  = o->width;
	old_height = o->ascent + o->descent;

	cell = HTML_TABLE_CELL (o);
	rv   = (* HTML_OBJECT_CLASS (parent_class)->calc_size) (o, painter, changed_objs);

	if (cell->fixed_height && o->ascent + o->descent < cell->fixed_height) {
		gint remains = cell->fixed_height - (o->ascent + o->descent);

		o->ascent += remains;

		switch (HTML_CLUE (o)->valign) {
		case HTML_VALIGN_TOP:
			break;
		case HTML_VALIGN_MIDDLE:
			clue_move_children (HTML_CLUE (o), 0, remains >> 1);
			break;
		case HTML_VALIGN_NONE:
		case HTML_VALIGN_BOTTOM:
			clue_move_children (HTML_CLUE (o), 0, remains);
			break;
		default:
			g_assert_not_reached ();
		}
		rv = TRUE;
	}

	if (o->parent && (o->width != old_width || o->ascent + o->descent != old_height))
		html_object_add_to_changed (changed_objs, o->parent);

	return rv;
}

static void
set_bg_color (HTMLObject *object, GdkColor *color)
{
	HTMLTableCell *cell;

	cell = HTML_TABLE_CELL (object);

	if (color == NULL) {
		cell->have_bg = FALSE;
		return;
	}

	if (cell->have_bg && ! gdk_color_equal (&cell->bg, color))
		cell->bg_allocated = FALSE;

	cell->bg = *color;
	cell->have_bg = TRUE;
}

static GdkColor *
get_bg_color (HTMLObject *o,
	      HTMLPainter *p)
{
	
	return HTML_TABLE_CELL (o)->have_bg
		? &HTML_TABLE_CELL (o)->bg
		: html_object_get_bg_color (o->parent, p);
}


#define SB if (!html_engine_save_output_string (state,
#define SE )) return FALSE

static gboolean
save (HTMLObject *self,
      HTMLEngineSaveState *state)
{
	HTMLTableCell *cell = HTML_TABLE_CELL (self);

	SB cell->heading ? "<TH" : "<TD" SE;
	if (cell->have_bg
	    && (!self->parent || !HTML_TABLE (self->parent)->bgColor
		|| !gdk_color_equal (&cell->bg, HTML_TABLE (self->parent)->bgColor)))
		SB " BGCOLOR=\"#%02x%02x%02x\"",
			cell->bg.red >> 8,
			cell->bg.green >> 8,
			cell->bg.blue >> 8 SE;
	
	if (cell->have_bgPixmap) {
		gchar * url = html_image_resolve_image_url (state->engine->widget, cell->bgPixmap->url);
		SB " BACKGROUND=\"%s\"", url SE;
		g_free (url);
	}
	if (cell->cspan != 1)
		SB " COLSPAN=\"%d\"", cell->cspan SE;

	if (cell->rspan != 1)
		SB " ROWSPAN=\"%d\"", cell->rspan SE;

	if (cell->percent_width) {
		SB " WIDTH=\"%d%%\"", cell->fixed_width SE;
	} else if (self->flags & HTML_OBJECT_FLAG_FIXEDWIDTH)
		SB " WIDTH=\"%d\"", cell->fixed_width SE;
	if (cell->no_wrap)
		SB " NOWRAP" SE;
	if (HTML_CLUE (cell)->halign != HTML_HALIGN_NONE)
		SB " ALIGN=\"%s\"",
                   html_engine_save_get_paragraph_align (html_alignment_to_paragraph (HTML_CLUE (cell)->halign)) SE;
	if (HTML_CLUE (cell)->valign != HTML_VALIGN_MIDDLE)
		SB " VALIGN=\"%s\"",
                   HTML_CLUE (cell)->valign == HTML_VALIGN_TOP ? "top" : "bottom" SE;
	SB ">\n" SE;
	if (!(*HTML_OBJECT_CLASS (parent_class)->save) (self, state))
		return FALSE;

	/*
	  save at least &nbsp; for emty cell
	  it's not really clean behavior, but I think it's better than saving empty cells.
	  let me know if you have better idea

	if (HTML_CLUE (self)->head && HTML_CLUE (self)->head == HTML_CLUE (self)->tail
	    && HTML_IS_CLUEFLOW (HTML_CLUE (self)->head) && html_clueflow_is_empty (HTML_CLUEFLOW (HTML_CLUE (self)->head)))
	    SB "&nbsp;" SE; */

	SB cell->heading ? "</TH>\n" : "</TD>\n" SE;

	return TRUE;
}


void
html_table_cell_type_init (void)
{
	html_table_cell_class_init (&html_table_cell_class, HTML_TYPE_TABLECELL, sizeof (HTMLTableCell));
}

void
html_table_cell_class_init (HTMLTableCellClass *klass,
			    HTMLType type,
			    guint object_size)
{
	HTMLObjectClass *object_class;
	HTMLClueVClass *cluev_class;

	object_class = HTML_OBJECT_CLASS (klass);
	cluev_class = HTML_CLUEV_CLASS (klass);

	html_cluev_class_init (cluev_class, type, object_size);

	object_class->reset = reset;
	object_class->copy = copy;
	object_class->calc_min_width = calc_min_width;
	object_class->calc_preferred_width = calc_preferred_width;
	object_class->calc_size = html_table_cell_real_calc_size;
	object_class->draw = draw;
	object_class->set_bg_color = set_bg_color;
	object_class->get_bg_color = get_bg_color;
	object_class->save = save;
	object_class->merge = merge;

	parent_class = &html_cluev_class;
}

void
html_table_cell_init (HTMLTableCell *cell,
		      HTMLTableCellClass *klass,
		      gint rs, gint cs,
		      gint pad)
{
	HTMLObject *object;
	HTMLClueV *cluev;
	HTMLClue *clue;

	object = HTML_OBJECT (cell);
	cluev = HTML_CLUEV (cell);
	clue = HTML_CLUE (cell);

	html_cluev_init (cluev, HTML_CLUEV_CLASS (klass), 0, 0, 0);

	object->flags &= ~HTML_OBJECT_FLAG_FIXEDWIDTH;

	clue->valign = HTML_VALIGN_MIDDLE;
	clue->halign = HTML_HALIGN_NONE;

	cell->fixed_width  = 0;
	cell->fixed_height = 0;
	cell->percent_width = FALSE;
	cell->percent_height = FALSE;

	cluev->padding = pad;
	cell->rspan = rs;
	cell->cspan = cs;
	cell->col = -1;
	cell->row = -1;

	cell->have_bg = FALSE;
	cell->have_bgPixmap = FALSE;
	cell->bg_allocated = FALSE;

	cell->no_wrap = FALSE;
	cell->heading = FALSE;
}

HTMLObject *
html_table_cell_new (gint rs, gint cs, gint pad)
{
	HTMLTableCell *cell;

	cell = g_new (HTMLTableCell, 1);
	html_table_cell_init (cell, &html_table_cell_class, rs, cs, pad);

	return HTML_OBJECT (cell);
}

void
html_table_cell_set_fixed_width (HTMLTableCell *cell, gint width, gboolean percented)
{
	if (percented)
		HTML_OBJECT (cell)->flags &= ~HTML_OBJECT_FLAG_FIXEDWIDTH;
	else
		HTML_OBJECT (cell)->flags |= HTML_OBJECT_FLAG_FIXEDWIDTH;
	cell->fixed_width = width;
	cell->percent_width = percented;
}

void
html_table_cell_set_fixed_height (HTMLTableCell *cell, gint height, gboolean percented)
{
	cell->fixed_height = height;
	cell->percent_height = percented;
}

void
html_table_cell_set_bg_pixmap (HTMLTableCell *cell,
				    HTMLImagePointer *imagePtr)
{
	if(imagePtr) {
		cell->have_bgPixmap = TRUE;
		cell->bgPixmap = imagePtr;
	}
}

void
html_table_cell_set_position (HTMLTableCell *cell, gint row, gint col)
{
	cell->col = col;
	cell->row = row;
}

gint
html_table_cell_get_fixed_width (HTMLTableCell *cell, HTMLPainter *painter)
{
	return html_painter_get_pixel_size (painter) * cell->fixed_width;
}
