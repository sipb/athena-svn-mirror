/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

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
#include "htmlpainter.h"



HTMLColorSet *
html_colorset_new (GtkWidget *w)
{
	HTMLColorSet *s;

	s = g_new0 (HTMLColorSet, 1);

	/* these are default color settings */
	s->color [HTMLLinkColor]       = html_color_new_from_rgb (0, 0, 0xffff);
	s->color [HTMLALinkColor]      = html_color_new_from_rgb (0, 0, 0xffff);
	s->color [HTMLVLinkColor]      = html_color_new_from_rgb (0, 0, 0xffff);
	s->color [HTMLSpellErrorColor] = html_color_new_from_rgb (0xffff, 0, 0);

	if (w) {
		GtkStyle *style = gtk_widget_get_style (w);
		html_colorset_set_style (s, style);
	} else {
		s->color [HTMLBgColor]            = html_color_new_from_rgb (0xffff, 0xffff, 0xffff);
		s->color [HTMLHighlightColor]     = html_color_new_from_rgb (0x7fff, 0x7fff, 0xffff);
		s->color [HTMLHighlightTextColor] = html_color_new ();
		s->color [HTMLHighlightNFColor]   = html_color_new ();
		s->color [HTMLHighlightTextNFColor] = html_color_new ();
		s->color [HTMLTextColor]          = html_color_new ();
	}

	return s;
}


void
html_colorset_destroy (HTMLColorSet *set)
{
	int i;

	g_return_if_fail (set != NULL);

	for (i = 0; i < HTMLColors; i++) {
		if (set->color[i] != NULL)
			html_color_unref (set->color[i]);
	}

	if (set->slaves)
		g_slist_free (set->slaves);

	g_free (set);
}

void
html_colorset_add_slave (HTMLColorSet *set, HTMLColorSet *slave)
{
	set->slaves = g_slist_prepend (set->slaves, slave);
}

void
html_colorset_set_color (HTMLColorSet *s, GdkColor *color, HTMLColorId idx)
{
	GSList *cur;
	HTMLColorSet *cs;

	html_color_set (s->color [idx], color);
	s->changed [idx] = TRUE;

	/* forward change to slaves */
	cur = s->slaves;
	while (cur) {
		cs  = (HTMLColorSet *) cur->data;
		html_colorset_set_color (cs, color, idx);
		cur = cur->next;
	}
}

HTMLColor *
html_colorset_get_color (HTMLColorSet *s, HTMLColorId idx)
{
	return s->color [idx];
}

HTMLColor *
html_colorset_get_color_allocated (HTMLPainter *painter, HTMLColorId idx)
{
	HTMLColorSet *s = painter->color_set;

	html_color_alloc (s->color [idx], painter);
	return s->color [idx];
}

void
html_colorset_set_by (HTMLColorSet *s, HTMLColorSet *o)
{
	HTMLColorId i;

	for (i=0; i < HTMLColors; i++) {
		html_colorset_set_color (s, &o->color [i]->color, i);
		/* unset the changed flag */
		s->changed [i] = FALSE;
	}
}

void
html_colorset_set_unchanged (HTMLColorSet *s, HTMLColorSet *o)
{
	HTMLColorId i;

	for (i=0; i < HTMLColors; i++) {
		if (!s->changed[i]) {
			html_colorset_set_color (s, &o->color [i]->color, i);
			s->changed [i] = FALSE;
		}
	}
}	

#define SET_GCOLOR(t,c) \
        if (!s->changed [HTML ## t ## Color]) { \
                if (s->color [HTML ## t ## Color]) html_color_unref (s->color [HTML ## t ## Color]); \
                s->color [HTML ## t ## Color] = html_color_new_from_gdk_color (&c); \
        }

void
html_colorset_set_style (HTMLColorSet *s, GtkStyle *style)
{
	SET_GCOLOR (Bg,              style->base [GTK_STATE_NORMAL]);
	SET_GCOLOR (Text,            style->text [GTK_STATE_NORMAL]);
	SET_GCOLOR (Highlight,       style->bg   [GTK_STATE_SELECTED]);
	SET_GCOLOR (HighlightText,   style->fg   [GTK_STATE_SELECTED]);
	SET_GCOLOR (HighlightNF,     style->bg   [GTK_STATE_ACTIVE]);
	SET_GCOLOR (HighlightTextNF, style->fg   [GTK_STATE_ACTIVE]);
}	
