/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-widget.c: Abstract base class for gnome-print configuration widgets
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors :
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 2000-2001 Ximian, Inc.
 *
 */

#include <config.h>
#include "gpa-widget.h"

static void gpa_widget_class_init (GPAWidgetClass *klass);
static void gpa_widget_init (GPAWidget *widget);

static void gpa_widget_finalize      (GObject *object);
static void gpa_widget_show_all      (GtkWidget *widget);
static void gpa_widget_hide_all      (GtkWidget *widget);
static void gpa_widget_size_request  (GtkWidget *widget, GtkRequisition *requisition);
static void gpa_widget_size_allocate (GtkWidget *widget, GtkAllocation  *allocation);

static GtkBinClass *parent_class;

GType
gpa_widget_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPAWidgetClass),
			NULL, NULL,
			(GClassInitFunc) gpa_widget_class_init,
			NULL, NULL,
			sizeof (GPAWidget),
			0,
			(GInstanceInitFunc) gpa_widget_init,
			NULL,
		};
		type = g_type_register_static (GTK_TYPE_BIN, "GPAWidget", &info, 0);
	}
	return type;
}

static void
gpa_widget_class_init (GPAWidgetClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	parent_class = gtk_type_class (GTK_TYPE_BIN);

	object_class->finalize = gpa_widget_finalize;

	widget_class->show_all      = gpa_widget_show_all;
	widget_class->hide_all      = gpa_widget_hide_all;
	widget_class->size_request  = gpa_widget_size_request;
	widget_class->size_allocate = gpa_widget_size_allocate;
}

static void
gpa_widget_init (GPAWidget *widget)
{
	widget->config = NULL;
}

static void
gpa_widget_finalize (GObject *object)
{
	GPAWidget *gpw;

	return;
	
	gpw = (GPAWidget *) object;

	if (gpw->config)
		gpw->config = gnome_print_config_unref (gpw->config);
	
	if (((GObjectClass *) parent_class)->finalize)
		(* ((GObjectClass *) parent_class)->finalize) (object);
}

static void
gpa_widget_show_all (GtkWidget *widget)
{
	gtk_widget_show (widget);
}

static void
gpa_widget_hide_all (GtkWidget *widget)
{
	gtk_widget_hide (widget);
}

static void
gpa_widget_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	if (((GtkBin *) widget)->child)
		gtk_widget_size_request (((GtkBin *) widget)->child, requisition);
}

static void
gpa_widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	widget->allocation = *allocation;

	if (((GtkBin *) widget)->child)
		gtk_widget_size_allocate (((GtkBin *) widget)->child, allocation);
}

gboolean
gpa_widget_construct (GPAWidget *gpw, GnomePrintConfig *config)
{
	g_return_val_if_fail (gpw != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_WIDGET (gpw), FALSE);
	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (gpw->config == NULL, FALSE);

	gpw->config = gnome_print_config_ref (config);

	if (GPA_WIDGET_GET_CLASS (gpw)->construct)
		return GPA_WIDGET_GET_CLASS (gpw)->construct (gpw);

	return TRUE;
}

GtkWidget *
gpa_widget_new (GtkType type, GnomePrintConfig *config)
{
	GPAWidget *gpw;

	g_return_val_if_fail (g_type_is_a (type, GPA_TYPE_WIDGET), NULL);

	gpw = g_object_new (type, NULL);

	if (config)
		gpa_widget_construct (gpw, config);

	return GTK_WIDGET (gpw);
}


