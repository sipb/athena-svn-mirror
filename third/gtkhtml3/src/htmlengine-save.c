/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 1998 World Wide Web Consortium
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

    Author: Ettore Perazzoli <ettore@helixcode.com>
    `encode_entities()' adapted from gnome-xml by Daniel Veillard
    <Daniel.Veillard@w3.org>.
*/

#include <config.h>
#include <string.h>

#include "config.h"
#include "htmlcolor.h"
#include "htmlengine.h"
#include "htmlimage.h"
#include "htmlentity.h"
#include "htmlengine-save.h"
#include "htmlsettings.h"

#include "gtkhtmldebug.h"


/* This routine was originally written by Daniel Velliard, (C) 1998 World Wide
   Web Consortium.  */
gchar *
html_encode_entities (const gchar *input, guint len, guint *encoded_len_return)
{
	gunichar uc;
	const gchar *p;
	guchar *buffer = NULL;
	guchar *out = NULL;
	gint buffer_size = 0;
	guint count;

	/* Allocate an translation buffer.  */
	buffer_size = 1000;
	buffer      = g_malloc (buffer_size);

	out   = buffer;
	p     = input;
	count = 0;

	while (p && *p && count < len) {
		if (out - buffer > buffer_size - 100) {
			gint index = out - buffer;

			buffer_size *= 2;
			buffer = g_realloc (buffer, buffer_size);
			out = &buffer[index];
		}
		uc = g_utf8_get_char (p);

		/* By default one have to encode at least '<', '>', '"' and '&'.  */

		if (uc == '<') {
			*out++ = '&';
			*out++ = 'l';
			*out++ = 't';
			*out++ = ';';
		} else if (uc == '>') {
			*out++ = '&';
			*out++ = 'g';
			*out++ = 't';
			*out++ = ';';
		} else if (uc == '&') {
			*out++ = '&';
			*out++ = 'a';
			*out++ = 'm';
			*out++ = 'p';
			*out++ = ';';
		} else if (uc == '"') {
			*out++ = '&';
			*out++ = 'q';
			*out++ = 'u';
			*out++ = 'o';
			*out++ = 't';
			*out++ = ';';
		} else if (uc == ENTITY_NBSP) {
			*out++ = '&';
			*out++ = 'n';
			*out++ = 'b';
			*out++ = 's';
			*out++ = 'p';
			*out++ = ';';
		} else if (((uc >= 0x20) && (uc < 0x80))
			   || (uc == '\n') || (uc == '\r') || (uc == '\t')) {
			/* Default case, just copy. */
			*out++ = uc;
		} else {
			char buf[10], *ptr;

			g_snprintf(buf, 9, "&#%d;", uc);

			ptr = buf;
			while (*ptr != 0)
				*out++ = *ptr++;
		}

		count++;
		p = g_utf8_next_char (p);
	}

	*out = 0;
	if (encoded_len_return)
		*encoded_len_return = out - buffer;

	return buffer;
}

gboolean
html_engine_save_encode (HTMLEngineSaveState *state,
			 const gchar *buffer,
			 guint length)
{
	guchar *encoded_buffer;
	guint encoded_length;
	gboolean success;

	g_return_val_if_fail (state != NULL, FALSE);
	g_return_val_if_fail (buffer != NULL, FALSE);

	if (length == 0)
		return TRUE;

	encoded_buffer = html_encode_entities ((const guchar *) buffer, length, &encoded_length);
	success = state->receiver (state->engine, encoded_buffer, encoded_length, state->user_data);

	g_free (encoded_buffer);
	return success;
}

gboolean
html_engine_save_encode_string (HTMLEngineSaveState *state,
				const gchar *s)
{
	guint len;

	g_return_val_if_fail (state != NULL, FALSE);
	g_return_val_if_fail (s != NULL, FALSE);

	len = strlen (s);

	return html_engine_save_encode (state, s, len);
}

gboolean
html_engine_save_output_stringv (HTMLEngineSaveState *state,
				 const char *format,
				 va_list ap)
{
	char *string;
	gboolean retval;

	string = g_strdup_vprintf (format, ap);
	retval = state->receiver (state->engine, string, strlen (string), state->user_data);
	g_free (string);

	return retval;
}

gboolean
html_engine_save_output_string (HTMLEngineSaveState *state,
				const gchar *format,
				...)
{
  va_list args;
  gboolean retval;
  
  g_return_val_if_fail (format != NULL, FALSE);
  g_return_val_if_fail (state != NULL, FALSE);
  
  va_start (args, format);
  retval = html_engine_save_output_stringv (state, format, args);
  va_end (args);
  
  return retval;
}



static gchar *
color_to_string (gchar *s, HTMLColor *c)
{
	gchar color [20];

	g_snprintf (color, 20, " %s=\"#%02x%02x%02x\"", s, c->color.red >> 8, c->color.green >> 8, c->color.blue >> 8);
	return g_strdup (color);
}

static gchar *
get_body (HTMLEngine *e)
{
	HTMLColorSet *cset;
	gchar *body;
	gchar *text;
	gchar *bg;
	gchar *bg_image;
	gchar *link;
	gchar *url = NULL;

	cset = e->settings->color_set;
	text = (cset->changed [HTMLTextColor]) ? color_to_string ("TEXT", cset->color [HTMLTextColor]) : g_strdup ("");
	link = (cset->changed [HTMLLinkColor]) ? color_to_string ("LINK", cset->color [HTMLLinkColor]) : g_strdup ("");
	bg   = (cset->changed [HTMLBgColor]) ? color_to_string ("BGCOLOR", cset->color [HTMLBgColor]) : g_strdup ("");
	bg_image = e->bgPixmapPtr ? g_strdup_printf (" BACKGROUND=\"%s\"",
						     url = html_image_resolve_image_url
						     (e->widget, ((HTMLImagePointer *) e->bgPixmapPtr)->url))
		: g_strdup ("");
	g_free (url);

	body = g_strconcat ("<BODY", text, link, bg, bg_image, ">\n", NULL);

	g_free (text);
	g_free (link);
	g_free (bg);
	g_free (bg_image);

	return body;
}

static gboolean
write_header (HTMLEngineSaveState *state)
{
	gboolean retval = TRUE;
	gchar *body;

	html_engine_clear_all_class_data (state->engine);
	/* Preface.  */
	if (! html_engine_save_output_string
	            (state,
		     "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 TRANSITIONAL//EN\">\n"
		     "<HTML>\n"))
		return FALSE;

	/* Header start.  FIXME: `GENERATOR' string?  */
	if (! html_engine_save_output_string
		     (state,
		      "<HEAD>\n"
		      "  <META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; CHARSET=UTF-8\">\n"
		      "  <META NAME=\"GENERATOR\" CONTENT=\"GtkHTML/%s\">\n", VERSION))
		return FALSE;

	/* Title.  */
	if (state->engine->title != NULL
	    && state->engine->title->str != NULL
	    && state->engine->title->str[0] != '\0') {
		if (! html_engine_save_output_string (state, "  <TITLE>")
		    || ! html_engine_save_encode_string (state, state->engine->title->str)
		    || ! html_engine_save_output_string (state, "</TITLE>\n"))
			return FALSE;
	}

	/* End of header.  */
	if (! html_engine_save_output_string (state, "</HEAD>\n"))
		return FALSE;

	/* Start of body.  */
	body = get_body (state->engine);
	if (!html_engine_save_output_string (state, "%s", body))
		retval = FALSE;
	g_free (body);

	return retval;
}

static gboolean
write_end (HTMLEngineSaveState *state)
{
	if (! html_engine_save_output_string (state, "</BODY>\n</HTML>\n"))
		return FALSE;

	html_engine_clear_all_class_data (state->engine);

	return TRUE;
}

gboolean
html_engine_save (HTMLEngine *engine,
		  HTMLEngineSaveReceiverFn receiver,
		  gpointer user_data)
{
	HTMLEngineSaveState state;

	if (engine->clue == NULL) {
		/* Empty document.  */
		return FALSE;
	}

	/* gtk_html_debug_dump_tree_simple (engine->clue, 1); */

	state.engine = engine;
	state.receiver = receiver;
	state.br_count = 0;
	state.error = FALSE;
	state.inline_frames = FALSE;
	state.user_data = user_data;
	state.last_level = 0;

	if (! write_header (&state))
		return FALSE;

	html_object_save (engine->clue, &state);
	if (state.error)
		return FALSE;

	if (! write_end (&state))
		return FALSE;

	return TRUE;
}

gboolean
html_engine_save_plain (HTMLEngine *engine,
			HTMLEngineSaveReceiverFn receiver,
			gpointer user_data)
{
	HTMLEngineSaveState state;
	
	if (engine->clue == NULL) {
		/* Empty document.  */
		return FALSE;
	}
	
	/* gtk_html_debug_dump_tree_simple (engine->clue, 1); */
	
	state.engine = engine;
	state.receiver = receiver;
	state.br_count = 0;
	state.error = FALSE;
	state.inline_frames = FALSE;
	state.user_data = user_data;
	state.last_level = 0;

	/* FIXME don't hardcode the length */
	html_object_save_plain (engine->clue, &state, 72);
	if (state.error)
		return FALSE;

	return TRUE;
}

static gboolean
html_engine_save_buffer_receiver (const HTMLEngine *engine,
				  const gchar      *data,
				  guint             len,
				  gpointer          user_data)
{
	g_string_append ((GString *)user_data, (gchar *)data);

	return TRUE;
}
	
void
html_engine_save_buffer_free (HTMLEngineSaveState *state)
{
	GString *string;

	g_return_if_fail (state != NULL);
	string = (GString *)state->user_data;

	g_string_free (string, TRUE);
	
	g_free (state);
}

guchar *
html_engine_save_buffer_peek_text (HTMLEngineSaveState *state)
{
	GString *string;
	
	g_return_val_if_fail (state != NULL, NULL);
	string = (GString *)state->user_data;
	
	return string->str;
}

HTMLEngineSaveState *
html_engine_save_buffer_new (HTMLEngine *engine, gboolean inline_frames)
{
	HTMLEngineSaveState *state = g_new0 (HTMLEngineSaveState, 1);

	if (state) {
		state->engine = engine;
		state->receiver = (HTMLEngineSaveReceiverFn)html_engine_save_buffer_receiver;
		state->br_count = 0;
		state->error = FALSE;
		state->inline_frames = inline_frames;
		state->user_data = (gpointer) g_string_new ("");
		state->last_level = 0;
	}

	return state;
}

gchar *
html_engine_save_get_sample_body (HTMLEngine *e,
				  HTMLObject *o)
{
	return get_body (e);
}

const gchar *
html_engine_save_get_paragraph_style (GtkHTMLParagraphStyle style)
{
	switch (style) {
	case GTK_HTML_PARAGRAPH_STYLE_NORMAL:
		return NULL;
	case GTK_HTML_PARAGRAPH_STYLE_H1:
		return "h1";
	case GTK_HTML_PARAGRAPH_STYLE_H2:
		return "h2";
	case GTK_HTML_PARAGRAPH_STYLE_H3:
		return "h3";
	case GTK_HTML_PARAGRAPH_STYLE_H4:
		return "h4";
	case GTK_HTML_PARAGRAPH_STYLE_H5:
		return "h5";
	case GTK_HTML_PARAGRAPH_STYLE_H6:
		return "h6";
	case GTK_HTML_PARAGRAPH_STYLE_ADDRESS:
		return "address";
	case GTK_HTML_PARAGRAPH_STYLE_PRE:
		return "pre";
	case GTK_HTML_PARAGRAPH_STYLE_ITEMDOTTED:
	case GTK_HTML_PARAGRAPH_STYLE_ITEMROMAN:
	case GTK_HTML_PARAGRAPH_STYLE_ITEMDIGIT:
	case GTK_HTML_PARAGRAPH_STYLE_ITEMALPHA:
		return "li";
	}

	g_warning ("Unknown GtkHTMLParagraphStyle %d", style);

	return NULL;
}

const gchar *
html_engine_save_get_paragraph_align (GtkHTMLParagraphAlignment align)
{
	switch (align) {
	case GTK_HTML_PARAGRAPH_ALIGNMENT_RIGHT:
		return "right";
	case GTK_HTML_PARAGRAPH_ALIGNMENT_CENTER:
		return "center";
	case GTK_HTML_PARAGRAPH_ALIGNMENT_LEFT:
		return "left";
	}

	g_warning ("Unknown GtkHTMLParagraphAlignment %d", align);

	return NULL;
}

gint
html_engine_save_string_append_nonbsp (GString *out, const guchar *s, guint length)
{	
	guint len = length;
	
	while (len--) {
		if (IS_UTF8_NBSP (s)) {
			g_string_append_c (out, ' ');
			s += 2;
			len--;
		} else {
			g_string_append_c (out, *s);
			s++;
		}
	}
	return length;
}
