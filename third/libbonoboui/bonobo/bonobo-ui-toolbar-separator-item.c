/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * bonobo-ui-toolbar-separator-item.h
 *
 * Author:
 *    Ettore Perazzoli (ettore@ximian.com)
 *
 * Copyright (C) 2000 Ximian, Inc.
 */

#include <config.h>
#include <atk/atk.h>
#include <libgnome/gnome-macros.h>
#include <bonobo/bonobo-a11y.h>
#include <bonobo/bonobo-ui-toolbar-separator-item.h>

GNOME_CLASS_BOILERPLATE (BonoboUIToolbarSeparatorItem,
			 bonobo_ui_toolbar_separator_item,
			 BonoboUIToolbarItem,
			 bonobo_ui_toolbar_item_get_type ());

#define BORDER_WIDTH        2

#define SPACE_LINE_DIVISION 10
#define SPACE_LINE_START    3
#define SPACE_LINE_END      7

/* GtkWidget methods.  */

static AtkObject *impl_get_accessible (GtkWidget *widget);

static void
impl_size_request (GtkWidget *widget,
		   GtkRequisition *requisition)
{
	int border_width;

	border_width = GTK_CONTAINER (widget)->border_width;

	requisition->width  = 2 * border_width + widget->style->xthickness;
	requisition->height = 2 * border_width + widget->style->ythickness;
}

static int
impl_expose_event (GtkWidget *widget,
		   GdkEventExpose *expose)
{
	BonoboUIToolbarItem *item;
	const GtkAllocation *allocation;
	GtkOrientation orientation;
	int border_width;
	GdkRectangle *area = &expose->area;

	item = BONOBO_UI_TOOLBAR_ITEM (widget);

	allocation = &widget->allocation;
	border_width = GTK_CONTAINER (widget)->border_width;

	orientation = bonobo_ui_toolbar_item_get_orientation (item);

	if (orientation == GTK_ORIENTATION_HORIZONTAL)
		gtk_paint_vline (widget->style, widget->window,
				 GTK_WIDGET_STATE (widget), area, widget,
				 "toolbar",
				 allocation->y + allocation->height * SPACE_LINE_START / SPACE_LINE_DIVISION,
				 allocation->y + allocation->height * SPACE_LINE_END / SPACE_LINE_DIVISION,
				 allocation->x + border_width);
	else
		gtk_paint_hline (widget->style, widget->window,
				 GTK_WIDGET_STATE (widget), area, widget,
				 "toolbar",
				 allocation->x + allocation->width * SPACE_LINE_START / SPACE_LINE_DIVISION,
				 allocation->x + allocation->width * SPACE_LINE_END / SPACE_LINE_DIVISION,
				 allocation->y + border_width);

	return FALSE;
}

static void
bonobo_ui_toolbar_separator_item_class_init (
	BonoboUIToolbarSeparatorItemClass *separator_item_class)
{
	GtkWidgetClass *widget_class = (GtkWidgetClass *) separator_item_class;

	widget_class->size_request = impl_size_request;
	widget_class->expose_event = impl_expose_event;

	widget_class->get_accessible = impl_get_accessible;
}

static void
bonobo_ui_toolbar_separator_item_instance_init (
	BonoboUIToolbarSeparatorItem *toolbar_separator_item)
{
	gtk_container_set_border_width (GTK_CONTAINER (toolbar_separator_item), BORDER_WIDTH);
}


GtkWidget *
bonobo_ui_toolbar_separator_item_new (void)
{
	return g_object_new (
		bonobo_ui_toolbar_separator_item_get_type (), NULL);
}

/*
 * a11y bits.
 */

static AtkObjectClass *a11y_parent_class = NULL;

static void
separator_item_a11y_initialize (AtkObject *accessible, gpointer widget)
{
	accessible->role = ATK_ROLE_SEPARATOR;

	a11y_parent_class->initialize (accessible, widget);
}

static AtkStateSet*
separator_item_a11y_ref_state_set (AtkObject *accessible)
{
	GtkWidget *widget;
	AtkStateSet *state_set;
	GtkOrientation orientation;

	state_set = a11y_parent_class->ref_state_set (accessible);
	widget = GTK_ACCESSIBLE (accessible)->widget;

	if (widget == NULL)
		return state_set;

	orientation = bonobo_ui_toolbar_item_get_orientation  (
		BONOBO_UI_TOOLBAR_ITEM (widget));

	if (orientation == GTK_ORIENTATION_VERTICAL) {
		atk_state_set_add_state (state_set, ATK_STATE_VERTICAL);
		atk_state_set_remove_state (state_set, ATK_STATE_HORIZONTAL);
	} else {
		atk_state_set_add_state (state_set, ATK_STATE_HORIZONTAL);
		atk_state_set_remove_state (state_set, ATK_STATE_VERTICAL);
	}

	return state_set;
}

static void
separator_item_a11y_class_init (AtkObjectClass *klass)
{
	a11y_parent_class = g_type_class_peek_parent (klass);

	klass->initialize = separator_item_a11y_initialize;
	klass->ref_state_set = separator_item_a11y_ref_state_set;
}

static AtkObject *
impl_get_accessible (GtkWidget *widget)
{
	return bonobo_a11y_create_accessible_for (
		widget, NULL, separator_item_a11y_class_init, 0);
}
