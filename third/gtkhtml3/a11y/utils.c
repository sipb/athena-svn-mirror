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

#include "htmlembedded.h"
#include "htmlobject.h"

#include "cell.h"
#include "image.h"
#include "link.h"
#include "paragraph.h"
#include "table.h"
#include "text.h"
#include "utils.h"

static AtkObject *
create_accessible (HTMLObject *o, AtkObject *parent)
{
	AtkObject *accessible = NULL;

	switch (HTML_OBJECT_TYPE (o)) {
	case HTML_TYPE_CLUEFLOW:
		accessible = html_a11y_paragraph_new (o);
		break;
	case HTML_TYPE_TEXT:
		accessible = html_a11y_text_new (o);
		break;
	case HTML_TYPE_LINKTEXT:
		accessible = html_a11y_link_new (o);
		break;
	case HTML_TYPE_IMAGE:
		accessible = html_a11y_image_new (o);
		break;
	case HTML_TYPE_TABLE:
		accessible = html_a11y_table_new (o);
		break;
	case HTML_TYPE_TABLECELL:
		accessible = html_a11y_cell_new (o);
		break;
	case HTML_TYPE_RULE:
		accessible = html_a11y_new (o, ATK_ROLE_SEPARATOR);
		break;
	case HTML_TYPE_EMBEDDED:
	case HTML_TYPE_SELECT:
	case HTML_TYPE_RADIO:
	case HTML_TYPE_OBJECT:
	case HTML_TYPE_TEXTAREA:
	case HTML_TYPE_TEXTINPUT:
	case HTML_TYPE_BUTTON:
	case HTML_TYPE_CHECKBOX:
		accessible = gtk_widget_get_accessible (HTML_EMBEDDED (o)->widget);
		break;
	case HTML_TYPE_TEXTSLAVE: /* ignore */
		break;
	default:
		accessible = html_a11y_new (o, ATK_ROLE_UNKNOWN);
		break;
	}

	if (accessible && parent) {
		/* printf ("set parent of %p to %p\n", accessible, parent); */
		atk_object_set_parent (accessible, parent);
	}

	return accessible;
}

AtkObject *
html_utils_get_accessible (HTMLObject *o, AtkObject *parent)
{
	AtkObject *accessible;

	accessible = html_object_get_data (o, ACCESSIBLE_ID);

	if (!accessible) {
		accessible = create_accessible (o, parent);
		if (accessible) {
			g_object_ref (accessible);
			html_object_set_data_full (o, ACCESSIBLE_ID, accessible, g_object_unref);
		}
	}

	return accessible;
}
