/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
   Copyright (C) 2001 CodeFactory AB
   Copyright (C) 2001 Jonas Borgstr�m <jonas@codefactory.se>
   Copyright (C) 2001 Anders Carlsson <andersca@codefactory.se>
   
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

#include <string.h>
#include <stdlib.h>
#include "dom/html/dom-htmlobjectelement.h"
#include "layout/html/htmlboxembeddedobject.h"
#include "layout/html/htmlembedded.h"

static void
html_box_embedded_object_class_init (HtmlBoxClass *klass)
{
}

static void
html_box_embedded_object_init (HtmlBoxEmbeddedObject *object)
{
}

GType
html_box_embedded_object_get_type (void)
{
	static GType html_type = 0;
	
	if (!html_type) {
		static GTypeInfo type_info = {
			sizeof (HtmlBoxEmbeddedObjectClass),
			NULL,
			NULL,
			(GClassInitFunc) html_box_embedded_object_class_init,		       
			NULL,
			NULL,
			sizeof (HtmlBoxEmbeddedObject),
			16, 
			(GInstanceInitFunc) html_box_embedded_object_init
		};
		
		html_type = g_type_register_static (HTML_TYPE_BOX_EMBEDDED, "HtmlBoxEmbeddedObject", &type_info, 0);
	}
       
	return html_type;
}

HtmlBox *
html_box_embedded_object_new (HtmlView *view, DomNode *node)
{
	HtmlBoxEmbeddedObject *result;
	GtkWidget *widget;
	gboolean ret = FALSE;
	result = g_object_new (HTML_TYPE_BOX_EMBEDDED_OBJECT, NULL);

	html_box_embedded_set_view (HTML_BOX_EMBEDDED (result), view);

	widget = html_embedded_new (node, HTML_BOX_EMBEDDED (result));

	g_signal_emit_by_name (G_OBJECT (view), "request_object", widget, &ret);

	if (ret) {
		html_box_embedded_set_widget (HTML_BOX_EMBEDDED (result), widget);
		g_print ("setting widget\n");
	}
	else
		g_object_unref (G_OBJECT (widget));

	return HTML_BOX (result);
}


