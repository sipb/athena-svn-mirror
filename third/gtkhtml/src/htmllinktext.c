/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the KDE libraries

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

#include "htmlcolorset.h"
#include "htmllinktext.h"
#include "htmlengine-edit-cut-and-paste.h"
#include "htmlengine-save.h"
#include "htmltext.h"
#include "htmlsettings.h"

HTMLLinkTextClass html_link_text_class;
static HTMLTextClass *parent_class = NULL;

/* HTMLObject methods.  */

static void
destroy (HTMLObject *object)
{
	HTMLLinkText *link_text;

	link_text = HTML_LINK_TEXT (object);
	g_free (link_text->url);
	g_free (link_text->target);

	(* HTML_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
copy_helper (HTMLText *self,
	     HTMLText *dest)
{
	HTML_LINK_TEXT (dest)->url = g_strdup (HTML_LINK_TEXT (self)->url);
	HTML_LINK_TEXT (dest)->target = g_strdup (HTML_LINK_TEXT (self)->target);
}

static void
copy (HTMLObject *self,
      HTMLObject *dest)
{
	(* HTML_OBJECT_CLASS (parent_class)->copy) (self, dest);
	copy_helper (HTML_TEXT (self), HTML_TEXT (dest));
}

static HTMLObject *
new_link (HTMLText *t, gint begin, gint end)
{
	return HTML_OBJECT (html_link_text_new_with_len (html_text_get_text (t, begin),
							 end - begin, t->font_style, t->color,
							 HTML_LINK_TEXT (t)->url,
							 HTML_LINK_TEXT (t)->target));
}

static HTMLObject *
op_copy (HTMLObject *self, HTMLEngine *e, GList *from, GList *to, guint *len)
{
	return html_text_op_copy_helper (HTML_TEXT (self), from, to, len, new_link);
}

static HTMLObject *
op_cut (HTMLObject *self, HTMLEngine *e, GList *from, GList *to, GList *left, GList *right, guint *len)
{
	return html_text_op_cut_helper (HTML_TEXT (self), e, from, to, left, right, len, new_link);
}

static int
compare (char *str1, char *str2)
{
	if (str1 == str2)
		return 0;
	
	if (str1 && str2)
		return strcasecmp (str1, str2);
	else 
		return 1;
}
	 
static gboolean
object_merge (HTMLObject *self, HTMLObject *with, HTMLEngine *e, GList **left, GList **right, HTMLCursor *cursor)
{
	return compare (HTML_LINK_TEXT (self)->url, HTML_LINK_TEXT (with)->url)
		|| compare (HTML_LINK_TEXT (self)->target, HTML_LINK_TEXT (with)->target)
		? FALSE
		: (* HTML_OBJECT_CLASS (parent_class)->merge) (self, with, e, left, right, cursor);
}

static const gchar *
get_url (HTMLObject *object)
{
	return HTML_LINK_TEXT (object)->url;
}

static const gchar *
get_target (HTMLObject *object)
{
	return HTML_LINK_TEXT (object)->target;
}

static HTMLObject *
set_link (HTMLObject *self, HTMLColor *color, const gchar *url, const gchar *target)
{
	HTMLText *text = HTML_TEXT (self);
	HTMLLinkText *lt = HTML_LINK_TEXT (self);

	if (url) {
		g_free (lt->url);
		g_free (lt->target);
		lt->url = g_strdup (url);
		lt->target = g_strdup (target);
		return NULL;
	} else {	
		return html_text_new_with_len (text->text, text->text_len, text->font_style, color);
	}
}

static GtkHTMLFontStyle
get_font_style (const HTMLText *text)
{
	GtkHTMLFontStyle font_style;

	font_style = HTML_TEXT_CLASS (parent_class)->get_font_style (text);
	font_style = gtk_html_font_style_merge (font_style, GTK_HTML_FONT_STYLE_UNDERLINE);

	return font_style;
}

static gboolean
save (HTMLObject *self,
      HTMLEngineSaveState *state)
{
	if (! html_engine_save_output_string (state, "<A HREF=\"")
	    || ! html_engine_save_output_string (state, "%s", HTML_LINK_TEXT (self)->url)
	    || ! html_engine_save_output_string (state, "\">"))
		return FALSE;

	if (! HTML_OBJECT_CLASS (parent_class)->save (self, state))
		return FALSE;

	if (! html_engine_save_output_string (state, "</A>"))
		return FALSE;

	return TRUE;
}

void
html_link_text_type_init (void)
{
	html_link_text_class_init (&html_link_text_class,
				   HTML_TYPE_LINKTEXT,
				   sizeof (HTMLLinkText));
}

void
html_link_text_class_init (HTMLLinkTextClass *klass,
			   HTMLType type,
			   guint size)
{
	HTMLObjectClass *object_class;
	HTMLTextClass *text_class;

	object_class = HTML_OBJECT_CLASS (klass);
	text_class   = HTML_TEXT_CLASS (klass);

	html_text_class_init (text_class, type, size);

	object_class->destroy = destroy;
	object_class->copy = copy;
	object_class->op_copy = op_copy;
	object_class->op_cut = op_cut;
	object_class->merge = object_merge;
	object_class->get_url = get_url;
	object_class->get_target = get_target;
	object_class->set_link = set_link;
	object_class->save = save;

	text_class->get_font_style = get_font_style;
	parent_class = &html_text_class;
}

void
html_link_text_init (HTMLLinkText *link_text_object,
		     HTMLLinkTextClass *klass,
		     const gchar *text,
		     gint len,
		     GtkHTMLFontStyle font_style,
		     HTMLColor *color,
		     const gchar *url,
		     const gchar *target)
{
	HTMLText *text_object;

	text_object = HTML_TEXT (link_text_object);

	html_text_init (text_object,
			HTML_TEXT_CLASS (klass),
			text, len,
			font_style,
			color);

	link_text_object->url = g_strdup (url);
	link_text_object->target = g_strdup (target);
}

HTMLObject *
html_link_text_new_with_len (const gchar *text,
			     gint len,
			     GtkHTMLFontStyle font_style,
			     HTMLColor *color,
			     const gchar *url,
			     const gchar *target)
{
	HTMLLinkText *link_text_object;

	g_return_val_if_fail (text != NULL, NULL);

	link_text_object = g_new (HTMLLinkText, 1);

	html_link_text_init (link_text_object,
			     &html_link_text_class,
			     text,
			     len,
			     font_style,
			     color,
			     url,
			     target);

	return HTML_OBJECT (link_text_object);
}

HTMLObject *
html_link_text_new (const gchar *text,
		    GtkHTMLFontStyle font_style,
		    HTMLColor *color,
		    const gchar *url,
		    const gchar *target)
{
	return html_link_text_new_with_len (text, -1, font_style, color, url, target);
}

HTMLObject *
html_link_text_to_text (HTMLLinkText *link, HTMLEngine *e)
{
	HTMLObject *new_text;

	new_text = html_text_new (HTML_TEXT (link)->text,
				  HTML_TEXT (link)->font_style,
				  html_colorset_get_color (e->settings->color_set, HTMLTextColor));
	html_text_set_font_face (HTML_TEXT (new_text), HTML_TEXT (link)->face);

	return new_text;
}
