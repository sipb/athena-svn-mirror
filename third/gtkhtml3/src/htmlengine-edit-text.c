/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library

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
#include <ctype.h>
#include <string.h>
#include "htmlcolorset.h"
#include "htmlcursor.h"
#include "htmlengine-edit-text.h"
#include "htmlengine-edit-movement.h"
#include "htmlengine-edit.h"
#include "htmlselection.h"
#include "htmltext.h"
#include "htmlengine.h"
#include "htmlimage.h"
#include "htmlsettings.h"
#include "htmlundo.h"

static gboolean
find_first (HTMLEngine *e)
{
	guchar c;

	c = html_cursor_get_current_char (e->cursor);
	while (c == 0 || ! g_unichar_isalnum (c) || c == ' ') {
		if (!html_cursor_forward (e->cursor, e))
			return FALSE;
		c = html_cursor_get_current_char (e->cursor);
	}

	return TRUE;
}

static void
upper_lower (HTMLObject *obj, HTMLEngine *e, gpointer data)
{
	if (html_object_is_text (obj)) {
		gboolean up = GPOINTER_TO_INT (data);
		guchar *old_text;

		old_text = HTML_TEXT (obj)->text;
		HTML_TEXT (obj)->text = up ? g_utf8_strup (old_text, -1) : g_utf8_strdown (old_text, -1);
		g_free (old_text);
		HTML_TEXT (obj)->text_bytes = g_utf8_strlen (HTML_TEXT (obj)->text, -1);
	}
}

void
html_engine_capitalize_word (HTMLEngine *e)
{
	if (find_first (e)) {
		guchar c;

		html_undo_level_begin (e->undo, "Capitalize word", "Revert word capitalize");
		html_engine_set_mark (e);
		html_cursor_forward (e->cursor, e);
		html_engine_cut_and_paste (e, "up 1st", "revert up 1st",
					   upper_lower, GINT_TO_POINTER (TRUE));
		html_engine_disable_selection (e);

		c = html_cursor_get_current_char (e->cursor);
		if (g_unichar_isalnum (c)) {
			html_engine_set_mark (e);
			html_engine_forward_word (e);
			html_engine_cut_and_paste (e, "down rest", "revert down rest",
						   upper_lower, GINT_TO_POINTER (FALSE));
			html_engine_disable_selection (e);
		}
		html_undo_level_end (e->undo);
	}
}

void
html_engine_upcase_downcase_word (HTMLEngine *e, gboolean up)
{
	if (find_first (e)) {
		html_engine_set_mark (e);
		html_engine_forward_word (e);
		html_engine_cut_and_paste (e,
					   up ? "Upcase word" : "Downcase word",
					   up ? "Revert word upcase" : "Revert word downcase",
					   upper_lower, GINT_TO_POINTER (up));
		html_engine_disable_selection (e);
	}
}

static void
set_link (HTMLObject *obj, HTMLEngine *e, gpointer data)
{
	const char *complete_url = data;

	if (html_object_is_text (obj) || HTML_IS_IMAGE (obj)) {
		char *url = NULL;
		char *target = NULL;

		if (complete_url) {
			url = g_strdup (complete_url);
			target = strrchr (url, '#');
			if (target) {
				*target = 0;
				target ++;
			}
		}

		if (html_object_is_text (obj)) {
			if (complete_url)
				html_text_add_link (HTML_TEXT (obj), e, url, target, 0, HTML_TEXT (obj)->text_len);
			else
				html_text_remove_links (HTML_TEXT (obj));
		} else if (HTML_IS_IMAGE (obj)) {
			if (complete_url)
				html_object_set_link (obj,
						      url && *url
						      ? html_colorset_get_color (e->settings->color_set, HTMLLinkColor)
						      : html_colorset_get_color (e->settings->color_set, HTMLTextColor),
						      url, target);
			else
				html_object_remove_link (obj,
							 html_colorset_get_color (e->settings->color_set, HTMLTextColor));
		}

		g_free (url);
	}
}

void
html_engine_set_link (HTMLEngine *e, const char *complete_url)
{
	html_engine_cut_and_paste (e,
				   complete_url ? "Set link" : "Remove link",
				   complete_url ? "Remove link" : "Set link",
				   set_link, (gpointer) complete_url);
}
