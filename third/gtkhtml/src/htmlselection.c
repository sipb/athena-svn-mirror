/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library

   Copyright (C) 2000 Helix Code, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHcANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <gtk/gtkselection.h>
#include "htmlcursor.h"
#include "htmlengine-edit-cursor.h"
#include "htmlentity.h"
#include "htmlinterval.h"
#include "htmlselection.h"
#include "htmlengine-edit.h"
#include "htmlengine-edit-selection-updater.h"

guint32
html_selection_current_time (void)
{
	GdkEvent *event;

	event = gtk_get_current_event ();
	if (event != NULL)
		return gdk_event_get_time (event);

	return GDK_CURRENT_TIME;
}

void
html_engine_select_interval (HTMLEngine *e, HTMLInterval *i)
{
	e = html_engine_get_top_html_engine (e);
	html_engine_hide_cursor (e);
	if (e->selection && html_interval_eq (e->selection, i))
		html_interval_destroy (i);
	else {
		html_engine_unselect_all (e);
		e->selection = i;
		html_interval_select (e->selection, e);
	}

	html_engine_activate_selection (e, html_selection_current_time ());
	html_engine_show_cursor (e);
}

void
html_engine_select_region (HTMLEngine *e,
			   gint x1, gint y1,
			   gint x2, gint y2)
{
	HTMLPoint *a, *b;

	g_return_if_fail (e != NULL);
	g_return_if_fail (HTML_IS_ENGINE (e));

	e = html_engine_get_top_html_engine (e);
	if (e->clue == NULL)
		return;

	a = html_engine_get_point_at (e, x1, y1, TRUE);
	b = html_engine_get_point_at (e, x2, y2, TRUE);

	if (a && b) {
		HTMLInterval *new_selection;

		new_selection = html_interval_new_from_points (a ,b);
		html_interval_validate (new_selection);
		html_engine_select_interval (e, new_selection);
	}

	if (a)
		html_point_destroy (a);
	if (b)
		html_point_destroy (b);
}

void
html_engine_clear_selection (HTMLEngine *e)
{
	if (e->selection) {
		html_interval_destroy (e->selection);
		html_engine_edit_selection_updater_reset (e->selection_updater);
		e->selection = NULL;
		/*
		if (gdk_selection_owner_get (GDK_SELECTION_PRIMARY) == GTK_WIDGET (e->widget)->window)
			gtk_selection_owner_set (NULL, GDK_SELECTION_PRIMARY, 
						 html_selection_current_time ());    
		*/
	}
}

void
html_engine_unselect_all (HTMLEngine *e)
{
	e = html_engine_get_top_html_engine (e);
	if (e->selection) {
		html_engine_hide_cursor (e);
		html_interval_unselect (e->selection, e);
		html_engine_clear_selection (e);
		html_engine_show_cursor (e);
	}
}

static void
remove_mark (HTMLEngine *e)
{
	if (e->editable) {
		if (e->mark == NULL)
			return;

		html_cursor_destroy (e->mark);
		e->mark = NULL;
	}
}

void
html_engine_deactivate_selection (HTMLEngine *e)
{
	remove_mark (e);
	html_engine_clear_selection (e);
}

void
html_engine_disable_selection (HTMLEngine *e)
{
	g_return_if_fail (e != NULL);
	g_return_if_fail (HTML_IS_ENGINE (e));

	html_engine_hide_cursor (e);
	remove_mark (e);
	html_engine_unselect_all (e);
	e->selection_mode = FALSE;
	html_engine_show_cursor (e);
}

static gboolean
line_interval (HTMLEngine *e, HTMLCursor *begin, HTMLCursor *end)
{
	return html_cursor_beginning_of_line (begin, e) && html_cursor_end_of_line (end, e);
}

gboolean
html_selection_word (gunichar uc)
{
	return uc && uc != ' ' && uc != '\t' && uc != ENTITY_NBSP         /* white space */
		&& uc != '(' && uc != ')' && uc != '[' && uc != ']';
}

gboolean
html_selection_spell_word (gunichar uc, gboolean *cited)
{
	if (uc == '\'' || uc == '`') {
		*cited = TRUE;
		return FALSE;
	} else {
		return g_unichar_isalpha (uc);
	}
}

static gboolean
word_interval (HTMLEngine *e, HTMLCursor *begin, HTMLCursor *end)
{
	/* move to the begin of word */
	while (html_selection_word (html_cursor_get_prev_char (begin)))
		html_cursor_backward (begin, e);
	/* move to the end of word */
	while (html_selection_word (html_cursor_get_current_char (end)))
		html_cursor_forward (end, e);

	return (begin->object && end->object);
}

static void
selection_helper (HTMLEngine *e, gboolean (*get_interval)(HTMLEngine *e, HTMLCursor *begin, HTMLCursor *end))
{
	HTMLCursor *cursor, *begin, *end;
	HTMLInterval *i;

	html_engine_unselect_all (e);
	cursor = html_engine_get_cursor (e);

	if (cursor->object) {
		begin  = html_cursor_dup (cursor);
		end    = html_cursor_dup (cursor);

		if ((*get_interval) (e, begin, end)) {
			i = html_interval_new_from_cursor (begin, end);
			html_engine_select_interval (e, i);
		}

		html_cursor_destroy (begin);
		html_cursor_destroy (end);
	}
	html_cursor_destroy (cursor);	
}

void
html_engine_select_word (HTMLEngine *e)
{
	selection_helper (e, word_interval);
}

void
html_engine_select_line (HTMLEngine *e)
{
	selection_helper (e, line_interval);
}

gboolean
html_engine_is_selection_active (HTMLEngine *e)
{
	html_engine_edit_selection_updater_do_idle (e->selection_updater);
	return e->selection ? TRUE : FALSE;
}

static void
test_point (HTMLObject *o, HTMLEngine *e, gpointer data)
{
	HTMLPoint *point = (HTMLPoint *) data;

	if (point->object == o) {
		if (point->object == e->selection->from.object && point->offset < e->selection->from.offset)
			return;
		if (point->object == e->selection->to.object && point->offset > e->selection->to.offset)
			return;

		/* this indicates that object is IN the selection */
		point->object = NULL;
	}
}

gboolean
html_engine_point_in_selection (HTMLEngine *e, HTMLObject *obj, guint offset)
{
	HTMLPoint *point;
	gboolean rv;

	if (!html_engine_is_selection_active (e) || !obj)
		return FALSE;

	point = html_point_new (obj, offset);
	html_interval_forall (e->selection, e, test_point, point);
	rv = point->object == NULL;

	html_point_destroy (point);

	return rv;
}

void
html_engine_activate_selection (HTMLEngine *e, guint32 time)
{
	if (e->selection && e->block_selection == 0 && GTK_WIDGET_REALIZED (e->widget))
		gtk_selection_owner_set (GTK_WIDGET (e->widget), GDK_SELECTION_PRIMARY, time);	
}

void
html_engine_block_selection (HTMLEngine *e)
{
	e->block_selection ++;
}

void
html_engine_unblock_selection (HTMLEngine *e)
{
	e->block_selection --;
}
