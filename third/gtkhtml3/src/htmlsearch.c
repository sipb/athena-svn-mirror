/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc.
    Authors:           Radek Doulik (rodo@helixcode.com)

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

    TODO:

           - now we go thru the html tree without take care about vertical
             position of paragraph. so it is possible to find first match
             on bottom of page (ie. first column of table) and the second
             one on top (ie. top of second comlumn)
	     [also Netscape didn't take care of it]

*/

#include <config.h>

#include "htmlsearch.h"
#include "htmlobject.h"
#include "htmlentity.h"
#include "htmlengine.h"

static void
set_text (HTMLSearch *s, const gchar *text)
{
	s->text           = g_strdup (text);
	s->text_len       = g_utf8_strlen (text, -1);
}

HTMLSearch *
html_search_new (HTMLEngine *e, const gchar *text, gboolean case_sensitive, gboolean forward, gboolean regular)
{
	HTMLSearch *ns = g_new (HTMLSearch, 1);
	gint i;

	set_text (ns, text);
	ns->case_sensitive = case_sensitive;
	ns->forward        = forward;
	ns->found          = NULL;
	ns->engine         = e;

	if (html_engine_get_editable (e)) {
		HTMLObject *o;
 
		ns->stack = NULL;
		ns->start_pos = e->cursor->offset - 1;
		for (o = e->cursor->object; o; o = o->parent)
			html_search_push (ns, o);
		ns->stack = g_slist_reverse (ns->stack);
		ns->found = g_list_append (ns->found, e->cursor->object);
	} else {
		ns->stack     = NULL;
		ns->start_pos = 0;
		html_search_push (ns, e->clue);
	}

	/* translate table
	   could translate uppercase to lowercase if non case_sensitive */
	ns->trans = g_new (gchar, 256);
	for (i=0; i<256; i++) {
		ns->trans [i] = (case_sensitive) ? i : ((i>='A' && i<='Z') ? i+('a'-'A') : i);
	}
	/* 
	 * FIXME translating &nbsp; breaks horribly
	 * with utf8 and nonutf8 regex (see bug #24446)
	 * so we won't do it
	 */
	/* ns->trans [ENTITY_NBSP] = ' '; */

	ns->regular = regular;
	if (regular) {
#ifdef HAVE_GNU_REGEX
		const gchar *rv;

		ns->reb = g_new0 (regex_t, 1);

		ns->reb->translate = ns->trans;
		rv = re_compile_pattern (ns->text, ns->text_len, ns->reb);
		if (rv) {
			g_warning (rv);
		}
#else
		int rv_int;

		ns->reb = g_new0 (regex_t, 1);

		rv_int = regcomp (ns->reb, ns->text, (case_sensitive) ? 0 : REG_ICASE);
		if (rv_int) {
			char buf[1024];
			if (regerror(rv_int, ns->reb, buf, sizeof(buf))) {
				g_warning (buf);
			} else {
				g_warning ("regcomp failed, error code %d", rv_int);
			}
		}
#endif
	} else {
		ns->reb = NULL;
	}

	return ns;
}

void
html_search_destroy (HTMLSearch *search)
{
	g_free (search->text);
	if (search->stack)
		g_slist_free (search->stack);
	if (search->reb) {
		/* FIXME why this segfault for me? regfree (search->reb); */
		g_free (search->reb);
	}
	g_free (search->trans);

	g_free (search);
}

void
html_search_push (HTMLSearch *search, HTMLObject *obj)
{
	search->stack = g_slist_prepend (search->stack, obj);
}

HTMLObject *
html_search_pop (HTMLSearch *search)
{
	HTMLObject *obj;

	obj = HTML_OBJECT (search->stack->data);
	search->stack = g_slist_remove (search->stack, obj);

	return obj;
}

gboolean
html_search_child_on_stack (HTMLSearch *search, HTMLObject *obj)
{
	return search->stack && HTML_OBJECT (search->stack->data)->parent == obj;
}

gboolean
html_search_next_parent (HTMLSearch *search)
{
	return search->stack && search->stack->next
		? html_object_search (HTML_OBJECT (search->stack->next->data), search)
		: FALSE;
}

void
html_search_set_text (HTMLSearch *search, const gchar *text)
{
	g_free (search->text);
	set_text (search, text);
}

void
html_search_set_forward (HTMLSearch *search, gboolean forward)
{
	if (search)
		search->forward = forward;
}
