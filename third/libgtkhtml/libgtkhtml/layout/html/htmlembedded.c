/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
   Copyright (C) 2001 CodeFactory AB
   Copyright (C) 2001 Jonas Borgström <jonas@codefactory.se>
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

#include "layout/html/htmlembedded.h"

static void
html_embedded_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	GtkBin *bin;
	
	g_return_if_fail (widget != NULL);
	g_return_if_fail (requisition != NULL);
	
	bin = GTK_BIN (widget);

	if (bin->child) {
		gtk_widget_size_request (bin->child, requisition);
	} else {
		requisition->width = widget->requisition.width;
		requisition->height = widget->requisition.height;
	}
}

static void
html_embedded_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	GtkBin *bin;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (allocation != NULL);
	
	bin = GTK_BIN (widget);

	if (bin->child && GTK_WIDGET_VISIBLE (bin->child)) {
		gtk_widget_size_allocate(bin->child, allocation);
	}
	widget->allocation = *allocation;
}

void 
html_embedded_set_descent (HtmlEmbedded *embedded, gint descent)
{
	g_assert (embedded != NULL);
	html_box_embedded_set_descent (embedded->box_embedded, descent);
}

gint
html_embedded_get_descent (HtmlEmbedded *embedded)
{
	g_assert (embedded != NULL);
	return html_box_embedded_get_descent (embedded->box_embedded);
}

DomNode *
html_embedded_get_dom_node (HtmlEmbedded *embedded)
{
	g_assert (embedded != NULL);
	return embedded->node;
}

static void
html_embedded_class_init (GtkWidgetClass *klass)
{
	klass->size_request = html_embedded_size_request;
	klass->size_allocate = html_embedded_size_allocate;
}

static void
html_embedded_init (HtmlEmbedded *object)
{
}

GType
html_embedded_get_type (void)
{
	static GType html_type = 0;
	
	if (!html_type) {
		static GTypeInfo type_info = {
			sizeof (HtmlEmbeddedClass),
			NULL,
			NULL,
			(GClassInitFunc) html_embedded_class_init,
			NULL,
			NULL,
			sizeof (HtmlEmbedded),
			16, 
			(GInstanceInitFunc) html_embedded_init
		};
		
		html_type = g_type_register_static (GTK_TYPE_BIN, "HtmlEmbedded", &type_info, 0);
	}
       
	return html_type;
}

GtkWidget *
html_embedded_new (DomNode *node, HtmlBoxEmbedded *box_embedded)
{
	HtmlEmbedded *embedded = g_object_new (HTML_TYPE_EMBEDDED, NULL);

	embedded->node = node;
	embedded->box_embedded = box_embedded;

	return GTK_WIDGET (embedded);
}
