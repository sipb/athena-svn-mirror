#define __GPA_WIDGET_C__

/*
 * gpa-widget
 *
 * Abstract base class for gnome-print configuration widgets
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 */

#include "gpa-widget.h"

static void gpa_widget_class_init (GPAWidgetClass *klass);
static void gpa_widget_init (GPAWidget *widget);

static void gpa_widget_destroy (GtkObject *object);

static void gpa_widget_show_all (GtkWidget *widget);
static void gpa_widget_hide_all (GtkWidget *widget);


static void gpa_widget_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void gpa_widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation);

static GtkBinClass *parent_class;

GtkType
gpa_widget_get_type (void)
{
	static GtkType widget_type = 0;
	if (!widget_type) {
		static const GtkTypeInfo widget_info = {
			"GPAWidget",
			sizeof (GPAWidget),
			sizeof (GPAWidgetClass),
			(GtkClassInitFunc) gpa_widget_class_init,
			(GtkObjectInitFunc) gpa_widget_init,
			NULL, NULL, NULL
		};
		widget_type = gtk_type_unique (GTK_TYPE_BIN, &widget_info);
	}
	return widget_type;
}

static void
gpa_widget_class_init (GPAWidgetClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	parent_class = gtk_type_class (GTK_TYPE_BIN);

	object_class->destroy = gpa_widget_destroy;

	widget_class->show_all = gpa_widget_show_all;
	widget_class->hide_all = gpa_widget_hide_all;

	widget_class->size_request = gpa_widget_size_request;
	widget_class->size_allocate = gpa_widget_size_allocate;
}

static void
gpa_widget_init (GPAWidget *widget)
{
	widget->config = NULL;
}

static void
gpa_widget_destroy (GtkObject *object)
{
	GPAWidget *gpw;

	gpw = (GPAWidget *) object;

	if (gpw->config) {
		gpw->config = gnome_print_config_unref (gpw->config);
	}

	if (((GtkObjectClass *) parent_class)->destroy)
		(* ((GtkObjectClass *) parent_class)->destroy) (object);
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

	g_return_val_if_fail (gtk_type_is_a (type, GPA_TYPE_WIDGET), NULL);

	gpw = gtk_type_new (type);

	if (config) gpa_widget_construct (gpw, config);

	return GTK_WIDGET (gpw);
}


