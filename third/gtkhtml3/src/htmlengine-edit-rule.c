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
#include "htmlengine.h"
#include "htmlrule.h"
#include "htmlcluealigned.h"
#include "htmlengine-edit-cut-and-paste.h"
#include "htmlengine-edit-rule.h"

void
html_engine_insert_rule (HTMLEngine      *e,
			 gint            length,
			 gint            percent,
			 gint            size,
			 gboolean        shade,
			 HTMLHAlignType  halign)
{
	HTMLObject *rule;

	g_return_if_fail (e != NULL);
	g_return_if_fail (HTML_IS_ENGINE (e));

	rule = html_rule_new (length, percent, size, shade, halign);

	html_engine_paste_object (e, rule, 1);
}

#define SET(x)  if (rule->x != x) { changed = TRUE; rule->x = x; }
#define SETO(x) if (HTML_OBJECT (rule)->x != x) { changed = TRUE; HTML_OBJECT (rule)->x = x; }

void
html_rule_set (HTMLRule *rule, HTMLEngine *e, gint length, gint percent, gint size, gboolean shade, HTMLHAlignType halign)
{
	gboolean changed = FALSE;

	SET  (length);
	SET  (size);
	SETO (percent);
	SET  (shade);
	SET  (halign);

	if (changed)
		html_engine_schedule_update (e);
}
