/*  This file is part of the GtkHTML library.
 *
 *  Copyright 2002 Ximian, Inc.
 *
 *  Author: Radek Doulik
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#include "htmlclueflow.h"
#include "paragraph.h"

static void html_a11y_paragraph_class_init (HTMLA11YParagraphClass *klass);
static void html_a11y_paragraph_init       (HTMLA11YParagraph *a11y_paragraph);

static AtkObjectClass *parent_class = NULL;

GType
html_a11y_paragraph_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo tinfo = {
			sizeof (HTMLA11YParagraphClass),
			NULL,                                                      /* base init */
			NULL,                                                      /* base finalize */
			(GClassInitFunc) html_a11y_paragraph_class_init,           /* class init */
			NULL,                                                      /* class finalize */
			NULL,                                                      /* class data */
			sizeof (HTMLA11YParagraph),                                /* instance size */
			0,                                                         /* nb preallocs */
			(GInstanceInitFunc) html_a11y_paragraph_init,              /* instance init */
			NULL                                                       /* value table */
		};

		type = g_type_register_static (G_TYPE_HTML_A11Y, "HTMLA11YParagraph", &tinfo, 0);
	}

	return type;
}

static void
html_a11y_paragraph_finalize (GObject *obj)
{
}

static void
html_a11y_paragraph_initialize (AtkObject *obj, gpointer data)
{
	/* printf ("html_a11y_paragraph_initialize\n"); */

	if (ATK_OBJECT_CLASS (parent_class)->initialize)
		ATK_OBJECT_CLASS (parent_class)->initialize (obj, data);
}

static void
html_a11y_paragraph_class_init (HTMLA11YParagraphClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	AtkObjectClass *atk_class = ATK_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	atk_class->initialize = html_a11y_paragraph_initialize;
	gobject_class->finalize = html_a11y_paragraph_finalize;
}

static void
html_a11y_paragraph_init (HTMLA11YParagraph *a11y_paragraph)
{
}

AtkObject *
html_a11y_paragraph_new (HTMLObject *html_obj)
{
	GObject *object;
	AtkObject *accessible;

	g_return_val_if_fail (HTML_IS_CLUEFLOW (html_obj), NULL);

	object = g_object_new (G_TYPE_HTML_A11Y_PARAGRAPH, NULL);

	accessible = ATK_OBJECT (object);
	atk_object_initialize (accessible, html_obj);

	accessible->role = ATK_ROLE_PANEL; /* FIX2 fixme PARAGRAPH */

	/* printf ("created new html accessible paragraph object\n"); */

	return accessible;
}
