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

/* Various debugging routines.  */

#include <config.h>
#include "gtkhtml-compat.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "gtkhtml.h"
#include "htmlobject.h"
#include "htmltext.h"
#include "htmltextslave.h"
#include "htmltable.h"
#include "htmltablecell.h"
#include "htmlclue.h"
#include "htmlclueflow.h"
#include "htmlframe.h"
#include "htmlframeset.h"
#include "htmliframe.h"
#include "htmlengine.h"
#include "htmltype.h"
#include "htmlenums.h"
#include "htmlenumutils.h"

#include "gtkhtmldebug.h"


/**
 * gtk_html_debug_log:
 * @html: A GtkHTML widget
 * @format: A format string, in printf() style
 * 
 * If @html has debugging turned on, print out the message, just like libc
 * printf().  Otherwise, just do nothing.
 **/
void
gtk_html_debug_log (GtkHTML *html,
		    const gchar *format,
		    ...)
{
	va_list ap;

	if (! html->debug)
		return;

	va_start (ap, format);
	vprintf (format, ap);
}


static const gchar *
clueflow_style_to_string (HTMLClueFlowStyle style)
{
	switch (style) {
	case HTML_CLUEFLOW_STYLE_NORMAL:
		return "Normal";
	case HTML_CLUEFLOW_STYLE_H1:
		return "H1";
	case HTML_CLUEFLOW_STYLE_H2:
		return "H2";
	case HTML_CLUEFLOW_STYLE_H3:
		return "H3";
	case HTML_CLUEFLOW_STYLE_H4:
		return "H4";
	case HTML_CLUEFLOW_STYLE_H5:
		return "H5";
	case HTML_CLUEFLOW_STYLE_H6:
		return "H6";
	case HTML_CLUEFLOW_STYLE_ADDRESS:
		return "Address";
	case HTML_CLUEFLOW_STYLE_PRE:
		return "Pre";
	case HTML_CLUEFLOW_STYLE_LIST_ITEM:
		return "List Item";
	default:
		return "UNKNOWN";
	}
}


void
gtk_html_debug_dump_table (HTMLObject *o,
			   gint level)
{
	gint c, r;
	HTMLTable *table = HTML_TABLE (o);

	for (r = 0; r < table->totalRows; r++) {
		for (c = 0; c < table->totalCols; c++) {
			gtk_html_debug_dump_tree (HTML_OBJECT (table->cells[r][c]), level);
		}
	}

}

static void
gtk_html_debug_dump_table_simple (HTMLObject *o, gint level)
{
	gint c, r;
	HTMLTable *table = HTML_TABLE (o);

	for (r = 0; r < table->totalRows; r++) {
		for (c = 0; c < table->totalCols; c++) {
			gtk_html_debug_dump_tree_simple (HTML_OBJECT (table->cells[r][c]), level);
		}
	}

}

void
gtk_html_debug_dump_object (HTMLObject *obj,
			    gint level)
{
	gint i;
	for (i = 0; i < level; i++)
		g_print (" ");

	g_print ("ObjectType: %s Pos: %d, %d, MinWidth: %d, Width: %d PrefWidth: %d MaxWidth: %d Ascent %d Descent %d",
		 html_type_name (HTML_OBJECT_TYPE (obj)),
		 obj->x, obj->y, obj->min_width, obj->width, obj->pref_width, obj->max_width, obj->ascent, obj->descent);

	if (HTML_OBJECT_TYPE (obj) == HTML_TYPE_CLUEFLOW)
		g_print (" [%s, %d]",
			 clueflow_style_to_string (HTML_CLUEFLOW (obj)->style), HTML_CLUEFLOW (obj)->level);
	else if (HTML_OBJECT_TYPE (obj) == HTML_TYPE_TEXTSLAVE) {
		gchar *sl_text = g_strndup (html_text_get_text (HTML_TEXT (HTML_TEXT_SLAVE (obj)->owner),
								HTML_TEXT_SLAVE (obj)->posStart),
					    html_text_get_index (HTML_TEXT (HTML_TEXT_SLAVE (obj)->owner),
								 HTML_TEXT_SLAVE (obj)->posStart
								 + HTML_TEXT_SLAVE (obj)->posLen)
					    - html_text_get_index (HTML_TEXT (HTML_TEXT_SLAVE (obj)->owner),
								   HTML_TEXT_SLAVE (obj)->posStart));
		g_print ("[start %d end %d] \"%s\" ",
			 HTML_TEXT_SLAVE (obj)->posStart,
			 HTML_TEXT_SLAVE (obj)->posStart + HTML_TEXT_SLAVE (obj)->posLen - 1,
			 sl_text);
		g_free (sl_text);
	}

	g_print ("\n");

	switch (HTML_OBJECT_TYPE (obj)) {
	case HTML_TYPE_TABLE:
		gtk_html_debug_dump_table (obj, level + 1);
		break;
	case HTML_TYPE_TEXT:
	case HTML_TYPE_LINKTEXT:
		for (i = 0; i < level; i++)
			g_print (" ");
		g_print ("Text (%d): \"%s\"\n",
			 HTML_TEXT (obj)->text_len, HTML_TEXT (obj)->text);
		break;

	case HTML_TYPE_CLUEH:
	case HTML_TYPE_CLUEV:
	case HTML_TYPE_CLUEFLOW:
		/* g_print ("Head: %p Tail: %p\n", HTML_CLUE (obj)->head, HTML_CLUE (obj)->tail); */
	case HTML_TYPE_CLUEALIGNED:
	case HTML_TYPE_TABLECELL:
		for (i = 0; i < level; i++) g_print (" ");
		g_print ("HAlign: %s VAlign: %s\n",
			 html_halign_name (HTML_CLUE (obj)->halign),
			 html_valign_name (HTML_CLUE (obj)->valign));
		gtk_html_debug_dump_tree (HTML_CLUE (obj)->head, level + 1);
		break;
	case HTML_TYPE_IFRAME:
		gtk_html_debug_dump_tree (GTK_HTML (HTML_IFRAME (obj)->html)->engine->clue, level + 1);
		break;
	case HTML_TYPE_FRAME:
		gtk_html_debug_dump_tree (GTK_HTML (HTML_FRAME (obj)->html)->engine->clue, level + 1);
		break;
	case HTML_TYPE_FRAMESET: {
		gint i;

		for (i = 0; i < HTML_FRAMESET (obj)->frames->len; i++)
			gtk_html_debug_dump_tree (g_ptr_array_index (HTML_FRAMESET (obj)->frames, i), level + 1);
		}
		break;
	default:
		break;
	}
}

void
gtk_html_debug_dump_tree (HTMLObject *o,
			  gint level)
{
	HTMLObject *obj;

	obj = o;
	while (obj) {
		gtk_html_debug_dump_object (obj, level);
		obj = obj->next;
	}
}


static void
debug_word_width (HTMLText *t, gint level)
{
	guint i;

	for (i = 0; i < level; i++)
		g_print ("\t");

	printf ("words: %d | ", t->words);
	for (i = 0; i < t->words; i ++)
		printf ("%d ", t->word_width [i]);
	printf ("\n");
}

static void
dump_data (GQuark key_id, gpointer data, gpointer user_data)
{
	gint i, level = GPOINTER_TO_INT (user_data);

	for (i = 0; i < level; i++)
		g_print ("\t");

	printf ("%s: '%s'\n", g_quark_to_string (key_id), (gchar *) data);
}

static void
dump_object_simple (HTMLObject *obj,
		    gint level)
{
	gint i;

	for (i = 0; i < level; i++)
		g_print ("\t");

	if (html_object_is_text (obj)) {
		g_print ("%s `%s'\n",
			 html_type_name (HTML_OBJECT_TYPE (obj)),
			 HTML_TEXT (obj)->text);
		debug_word_width (HTML_TEXT (obj), level);
	} else if (HTML_OBJECT_TYPE (obj) == HTML_TYPE_TEXTSLAVE) {
		HTMLTextSlave *slave = HTML_TEXT_SLAVE (obj);
		gchar *text;

		text = alloca (slave->posLen+1);
		text [slave->posLen] = 0;
		strncpy (text, slave->owner->text + slave->posStart, slave->posLen);
		g_print ("%s `%s'\n",
			 html_type_name (HTML_OBJECT_TYPE (obj)),
			 text);
	} else if (HTML_OBJECT_TYPE (obj) == HTML_TYPE_TABLECELL) {
		g_print ("%s %d,%d\n", html_type_name (HTML_OBJECT_TYPE (obj)),
			 HTML_TABLE_CELL (obj)->row, HTML_TABLE_CELL (obj)->col);
	} else if (HTML_OBJECT_TYPE (obj) == HTML_TYPE_TABLE) {
		g_print ("%s %d,%d\n", html_type_name (HTML_OBJECT_TYPE (obj)),
			 HTML_TABLE (obj)->totalRows, HTML_TABLE (obj)->totalCols);
	} else
		g_print ("%s\n", html_type_name (HTML_OBJECT_TYPE (obj)));

	if (obj->object_data)
		g_datalist_foreach (&obj->object_data, dump_data, GINT_TO_POINTER (level));
}

void
gtk_html_debug_dump_object_type (HTMLObject *o)
{
	dump_object_simple (o, 0);
}

void
gtk_html_debug_dump_tree_simple (HTMLObject *o,
				 gint level)
{
	HTMLObject *obj;

	for (obj = o; obj != NULL; obj = obj->next) {
		if (HTML_OBJECT_TYPE (obj) == HTML_TYPE_TEXTSLAVE)
			continue;

		dump_object_simple (obj, level);

		switch (HTML_OBJECT_TYPE (obj)) {
		case HTML_TYPE_CLUEH:
		case HTML_TYPE_CLUEV:
		case HTML_TYPE_CLUEFLOW:
		case HTML_TYPE_CLUEALIGNED:
		case HTML_TYPE_TABLECELL:
			gtk_html_debug_dump_tree_simple (HTML_CLUE (obj)->head, level + 1);
			break;
		case HTML_TYPE_TABLE:
			gtk_html_debug_dump_table_simple (obj, level + 1);
			break;
		default:
			break;
		}
	}
}

void
gtk_html_debug_dump_list_simple (GList *list,
				 gint level)
{
	HTMLObject *obj;
	GList *p;

	for (p = list; p != NULL; p = p->next) {
		obj = HTML_OBJECT (p->data);

		if (HTML_OBJECT_TYPE (obj) == HTML_TYPE_TEXTSLAVE)
			continue;

		dump_object_simple (obj, level);
	}
}
