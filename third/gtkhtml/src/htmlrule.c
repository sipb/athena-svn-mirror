/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library.

   Copyright (C) 1997 Martin Jones (mjones@kde.org)
   Copyright (C) 1997 Torben Weis (weis@kde.org)
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
#include "htmlcolor.h"
#include "htmlcolorset.h"
#include "htmlengine-save.h"
#include "htmlrule.h"
#include "htmlpainter.h"


HTMLRuleClass html_rule_class;
static HTMLObjectClass *parent_class = NULL;


/* HTMLObject methods.  */

#define HTML_RULE_MIN_SIZE 12

static void
copy (HTMLObject *self,
      HTMLObject *dest)
{
	(* HTML_OBJECT_CLASS (parent_class)->copy) (self, dest);

	HTML_RULE (dest)->length = HTML_RULE (self)->length;
	HTML_RULE (dest)->size   = HTML_RULE (self)->size;
	HTML_RULE (dest)->shade  = HTML_RULE (self)->shade;
	HTML_RULE (dest)->halign = HTML_RULE (self)->halign;
}

static void
set_max_width (HTMLObject *o, HTMLPainter *painter, gint max_width)
{
	o->max_width = max_width;
}

static gint
calc_min_width (HTMLObject *o,
		HTMLPainter *painter)
{
	gint pixel_size;

	pixel_size = html_painter_get_pixel_size (painter);

	if (HTML_RULE (o)->length > 0)
		return HTML_RULE (o)->length * pixel_size;

	return pixel_size;
}

static HTMLFitType
fit_line (HTMLObject *o,
	  HTMLPainter *painter,
	  gboolean start_of_line,
	  gboolean first_run,
	  gint width_left)
{
	if (!start_of_line)
		return HTML_FIT_NONE;
	o->width = width_left;

	if (o->percent == 0) {
		gint pixel_size = html_painter_get_pixel_size (painter);
		if (HTML_RULE (o)->length * pixel_size > width_left) {
			o->width = HTML_RULE (o)->length * pixel_size;
		}
	}

	return HTML_FIT_COMPLETE;
}

static gboolean
calc_size (HTMLObject *self, HTMLPainter *painter, GList **changed_objs)
{
	HTMLRule *rule;
	gint ascent, descent;
	gint pixel_size;
	gint height;
	gboolean changed;

	rule = HTML_RULE (self);
	pixel_size = html_painter_get_pixel_size (painter);

	if (rule->size >= HTML_RULE_MIN_SIZE) {
		height = rule->size;
	} else {
		height = HTML_RULE_MIN_SIZE;
	}

	ascent = (height / 2 + height % 2 + 1) * pixel_size;
	descent = (height / 2 + 1) * pixel_size;

	changed = FALSE;

	if (self->width > self->max_width) {
		self->width = self->max_width;
		changed = TRUE;
	}

	if (ascent != self->ascent) {
		self->ascent = ascent;
		changed = TRUE;
	}

	if (descent != self->descent) {
		self->descent = descent;
		changed = TRUE;
	}

	return changed;
}

static void
draw (HTMLObject *o,
      HTMLPainter *p, 
      gint x, gint y,
      gint width, gint height,
      gint tx, gint ty)
{
	HTMLRule *rule;
	guint w, h;
	gint xp, yp;
	gint pixel_size = html_painter_get_pixel_size (p);

	rule = HTML_RULE (o);
	
	if (y + height < o->y - o->ascent || y > o->y + o->descent)
		return;

	h = rule->size * pixel_size;
	xp = o->x + tx;
	yp = o->y + ty - (rule->size / 2 + rule->size % 2) * pixel_size;

	if (o->percent == 0)
		w = o->width;
	else
		/* The cast to `gdouble' is to avoid overflow (eg. when
                   printing).  */
		w = ((gdouble) o->width * o->percent) / 100;

	switch (rule->halign) {
	case HTML_HALIGN_LEFT:
		break;
	case HTML_HALIGN_CENTER:
	case HTML_HALIGN_NONE:
		/* Default is `align=center' according to the specs.  */
		xp += (o->width - w) / 2;
		break;
	case HTML_HALIGN_RIGHT:
		xp += o->width - w;
		break;
	default:
		g_warning ("Unknown HTMLRule alignment %d.", rule->halign);
	}

	if (rule->shade)
		html_painter_draw_panel (p, &((html_colorset_get_color (p->color_set, HTMLBgColor))->color),
					 xp, yp, w, h, GTK_HTML_ETCH_IN, 1);
	else {
		html_painter_set_pen (p, &html_colorset_get_color_allocated (p, HTMLTextColor)->color);
		html_painter_fill_rect (p, xp, yp, w, h);
	}
}

static gboolean
accepts_cursor (HTMLObject *self)
{
	return TRUE;
}

static gboolean
save (HTMLObject *self,
      HTMLEngineSaveState *state)
{
	gchar *size, *shade, *length;
	gboolean rv;

	size   = HTML_RULE (self)->size == 2 ? "" : g_strdup_printf (" SIZE=\"%d\"", HTML_RULE (self)->size);
	shade  = HTML_RULE (self)->shade ? "" : " NOSHADE";
	length = HTML_RULE (self)->length
		? g_strdup_printf (" LENGTH=\"%d\"", HTML_RULE (self)->length)
		: (self->percent > 0 && self->percent != 100 ? g_strdup_printf (" LENGTH=\"%d%%\"", self->percent) : "");

	rv = html_engine_save_output_string (state, "\n<HR%s%s%s>\n", shade, size, length);

	if (*size)
		g_free (size);
	if (*length)
		g_free (length);

	return rv;
}

static gboolean
save_plain (HTMLObject *self,
	    HTMLEngineSaveState *state,
	    gint requested_width)
{
	int i;
	
	if (!html_engine_save_output_string (state, "\n"))
		return FALSE;

	/* Fixme no alignment or percent */
	for (i = 0; i < requested_width; i++)
		if (!html_engine_save_output_string (state, "_"))
			return FALSE;
	
	if (!html_engine_save_output_string (state, "\n"))
		return FALSE;

	return TRUE;
}

void
html_rule_type_init (void)
{
	html_rule_class_init (&html_rule_class, HTML_TYPE_RULE, sizeof (HTMLRule));
}

void
html_rule_class_init (HTMLRuleClass *klass,
		      HTMLType type,
		      guint object_size)
{
	HTMLObjectClass *object_class;

	object_class = HTML_OBJECT_CLASS (klass);

	html_object_class_init (object_class, type, object_size);

	object_class->copy = copy;
	object_class->draw = draw;
	object_class->set_max_width = set_max_width;
	object_class->calc_min_width = calc_min_width;
	object_class->fit_line = fit_line;
	object_class->calc_size = calc_size;
	object_class->accepts_cursor = accepts_cursor;
	object_class->save = save;
	object_class->save_plain = save_plain;

	parent_class = &html_object_class;
}

void
html_rule_init (HTMLRule *rule,
		HTMLRuleClass *klass,
		gint length,
		gint percent,
		gint size,
		gboolean shade,
		HTMLHAlignType halign)
{
	HTMLObject *object;

	object = HTML_OBJECT (rule);

	html_object_init (object, HTML_OBJECT_CLASS (klass));

	if (size < 1)
		size = 1;	/* FIXME why? */
	rule->size = size;

	object->percent = percent;

	rule->length = length;
	rule->shade  = shade;
	rule->halign = halign;

	if (percent > 0) {
		object->flags &= ~ HTML_OBJECT_FLAG_FIXEDWIDTH;
		rule->length = 0;
	} else if (rule->length > 0) {
		object->flags |= HTML_OBJECT_FLAG_FIXEDWIDTH;
	} else {
		object->flags &= ~ HTML_OBJECT_FLAG_FIXEDWIDTH;
	}
}

HTMLObject *
html_rule_new (gint length,
	       gint percent,
	       gint size,
	       gboolean shade,
	       HTMLHAlignType halign)
{
	HTMLRule *rule;

	rule = g_new (HTMLRule, 1);
	html_rule_init (rule, &html_rule_class, length, percent,
			size, shade, halign);

	return HTML_OBJECT (rule);
}











