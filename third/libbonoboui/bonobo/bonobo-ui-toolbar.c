/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * bonobo-ui-toolbar.h
 *
 * Author:
 *    Ettore Perazzoli (ettore@ximian.com)
 *
 * Copyright (C) 2000 Ximian, Inc.
 */

#include <config.h>
#include <glib/gi18n.h>
#include <libgnome/gnome-macros.h>
#include <bonobo/bonobo-ui-private.h>
#include <bonobo/bonobo-ui-toolbar.h>
#include <bonobo/bonobo-ui-toolbar-item.h>
#include <bonobo/bonobo-ui-toolbar-popup-item.h>

GNOME_CLASS_BOILERPLATE (BonoboUIToolbar,
			 bonobo_ui_toolbar,
			 GObject,
			 GTK_TYPE_CONTAINER);
enum {
	PROP_0,
	PROP_ORIENTATION,
	PROP_IS_FLOATING,
	PROP_PREFERRED_WIDTH,
	PROP_PREFERRED_HEIGHT
};

struct _BonoboUIToolbarPrivate {
	/* The orientation of this toolbar.  */
	GtkOrientation orientation;

	/* Is the toolbar currently floating */
	gboolean is_floating;

	/* The style of this toolbar.  */
	BonoboUIToolbarStyle style;

	/* Styles to use in different orientations */
	BonoboUIToolbarStyle hstyle;
	BonoboUIToolbarStyle vstyle;

	/* Sizes of the toolbar.  This is actually the height for
           horizontal toolbars and the width for vertical toolbars.  */
	int max_width, max_height;
	int total_width, total_height;

	/* position of left edge of left-most pack-end item */
	int end_position;

	/* List of all the items in the toolbar.  Both the ones that have been
           unparented because they don't fit, and the ones that are visible.
           The BonoboUIToolbarPopupItem is not here though.  */
	GList *items;

	/* Pointer to the first element in the `items' list that doesn't fit in
           the available space.  This is updated at size_allocate.  */
	GList *first_not_fitting_item;

	/* The pop-up button.  When clicked, it pops up a window with all the
           items that don't fit.  */
	BonoboUIToolbarItem *popup_item;

	/* The window we pop-up when the pop-up item is clicked.  */
	GtkWidget *popup_window;

	/* The vbox within the pop-up window.  */
	GtkWidget *popup_window_vbox;

	/* Whether we have moved items to the pop-up window.  This is to
           prevent the size_allocation code to incorrectly hide the pop-up
           button in that case.  */
	gboolean items_moved_to_popup_window;

	GtkTooltips *tooltips;
};

enum {
	SET_ORIENTATION,
	STYLE_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/* Width of the pop-up window.  */

#define POPUP_WINDOW_WIDTH 200

/* Utility functions.  */

static void
parentize_widget (BonoboUIToolbar *toolbar,
		  GtkWidget       *widget)
{
	g_assert (widget->parent == NULL);

	/* The following is done according to the Bible, widget_system.txt, IV, 1.  */

	gtk_widget_set_parent (widget, GTK_WIDGET (toolbar));
}

static void
set_attributes_on_child (BonoboUIToolbarItem *item,
			 GtkOrientation orientation,
			 BonoboUIToolbarStyle style)
{
	bonobo_ui_toolbar_item_set_orientation (item, orientation);

	switch (style) {
	case BONOBO_UI_TOOLBAR_STYLE_PRIORITY_TEXT:
		if (! bonobo_ui_toolbar_item_get_want_label (item))
			bonobo_ui_toolbar_item_set_style (item, BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_ONLY);
		else if (orientation == GTK_ORIENTATION_HORIZONTAL)
			bonobo_ui_toolbar_item_set_style (item, BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_AND_TEXT_HORIZONTAL);
		else
			bonobo_ui_toolbar_item_set_style (item, BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_AND_TEXT_VERTICAL);
		break;

	case BONOBO_UI_TOOLBAR_STYLE_ICONS_AND_TEXT:
		if (orientation == GTK_ORIENTATION_VERTICAL)
			bonobo_ui_toolbar_item_set_style (item, BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_AND_TEXT_HORIZONTAL);
		else
			bonobo_ui_toolbar_item_set_style (item, BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_AND_TEXT_VERTICAL);
		break;
	case BONOBO_UI_TOOLBAR_STYLE_ICONS_ONLY:
		bonobo_ui_toolbar_item_set_style (item, BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_ONLY);
		break;
	case BONOBO_UI_TOOLBAR_STYLE_TEXT_ONLY:
		bonobo_ui_toolbar_item_set_style (item, BONOBO_UI_TOOLBAR_ITEM_STYLE_TEXT_ONLY);
		break;
	default:
		g_assert_not_reached ();
	}
}

/* Callbacks to do widget housekeeping.  */

static void
item_destroy_cb (GtkObject *object,
		 void *data)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;
	GtkWidget *widget;

	toolbar = BONOBO_UI_TOOLBAR (data);
	priv = toolbar->priv;

	widget = GTK_WIDGET (object);
	priv->items = g_list_remove (priv->items, object);
	g_object_unref (object);
}

static void
item_activate_cb (BonoboUIToolbarItem *item,
		  void *data)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;

	toolbar = BONOBO_UI_TOOLBAR (data);
	priv = toolbar->priv;

	bonobo_ui_toolbar_toggle_button_item_set_active (
		BONOBO_UI_TOOLBAR_TOGGLE_BUTTON_ITEM (priv->popup_item), FALSE);
}

static void
item_set_want_label_cb (BonoboUIToolbarItem *item,
			gboolean want_label,
			void *data)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;

	toolbar = BONOBO_UI_TOOLBAR (data);
	priv = toolbar->priv;

	set_attributes_on_child (item, priv->orientation, priv->style);

	gtk_widget_queue_resize (GTK_WIDGET (toolbar));
}

/* The pop-up window foo.  */

/* Return TRUE if there are actually any items in the pop-up menu.  */
static void
create_popup_window (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;
	GtkWidget *hbox;
	GList *p;
	int row_width;

	priv = toolbar->priv;

	row_width = 0;
	hbox = NULL;

	for (p = priv->first_not_fitting_item; p != NULL; p = p->next) {
		GtkRequisition item_requisition;
		GtkWidget *item_widget;

		item_widget = GTK_WIDGET (p->data);

		if (! GTK_WIDGET_VISIBLE (item_widget) ||
			bonobo_ui_toolbar_item_get_pack_end (BONOBO_UI_TOOLBAR_ITEM (item_widget)))

			continue;

		if (item_widget->parent != NULL)
			gtk_container_remove (GTK_CONTAINER (item_widget->parent), item_widget);

		gtk_widget_get_child_requisition (item_widget, &item_requisition);

		set_attributes_on_child (BONOBO_UI_TOOLBAR_ITEM (item_widget),
					 GTK_ORIENTATION_HORIZONTAL,
					 priv->style);

		if (hbox == NULL
		    || (row_width > 0 && item_requisition.width + row_width > POPUP_WINDOW_WIDTH)) {
			hbox = gtk_hbox_new (FALSE, 0);
			gtk_box_pack_start (GTK_BOX (priv->popup_window_vbox), hbox, FALSE, TRUE, 0);
			gtk_widget_show (hbox);
			row_width = 0;
		}

		gtk_box_pack_start (GTK_BOX (hbox), item_widget, FALSE, TRUE, 0);

		row_width += item_requisition.width;
	}
}

static void
hide_popup_window (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;
	GList *p;

	priv = toolbar->priv;

	gdk_display_pointer_ungrab
		(gtk_widget_get_display (priv->popup_window),
		 GDK_CURRENT_TIME);

	gtk_grab_remove (priv->popup_window);
	gtk_widget_hide (priv->popup_window);

	priv->items_moved_to_popup_window = FALSE;

	/* Reset the attributes on all the widgets that were moved to the
           window and move them back to the toolbar.  */
	for (p = priv->items; p != NULL; p = p->next) {
		GtkWidget *item_widget;

		item_widget = GTK_WIDGET (p->data);
		if (item_widget->parent != GTK_WIDGET (toolbar)) {
			set_attributes_on_child (BONOBO_UI_TOOLBAR_ITEM (item_widget),
						 priv->orientation, priv->style);
			gtk_container_remove (GTK_CONTAINER (item_widget->parent), item_widget);
			parentize_widget (toolbar, item_widget);
		}
	}

	gtk_widget_queue_resize (GTK_WIDGET (toolbar));
}

static void
popup_window_button_release_cb (GtkWidget *widget,
				GdkEventButton *event,
				void *data)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;

	toolbar = BONOBO_UI_TOOLBAR (data);
	priv = toolbar->priv;

	bonobo_ui_toolbar_toggle_button_item_set_active
		(BONOBO_UI_TOOLBAR_TOGGLE_BUTTON_ITEM (priv->popup_item), FALSE);
}

static void
popup_window_map_cb (GtkWidget *widget,
		     void *data)
{
	BonoboUIToolbar *toolbar;

	toolbar = BONOBO_UI_TOOLBAR (data);

	if (gdk_pointer_grab (widget->window, TRUE,
			      (GDK_BUTTON_PRESS_MASK
			       | GDK_BUTTON_RELEASE_MASK
			       | GDK_ENTER_NOTIFY_MASK
			       | GDK_LEAVE_NOTIFY_MASK
			       | GDK_POINTER_MOTION_MASK),
			      NULL, NULL, GDK_CURRENT_TIME) != 0) {
		g_warning ("Toolbar pop-up pointer grab failed.");
		return;
	}

	gtk_grab_add (widget);
}

static void
show_popup_window (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;
	const GtkAllocation *toolbar_allocation;
	gint x, y;
	GdkScreen *screen;
	gint screen_width, screen_height;
	gint window_width, window_height;

	priv = toolbar->priv;

	priv->items_moved_to_popup_window = TRUE;

	create_popup_window (toolbar);

	gdk_window_get_origin (GTK_WIDGET (toolbar)->window, &x, &y);
	toolbar_allocation = & GTK_WIDGET (toolbar)->allocation;
	if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
		x += toolbar_allocation->x + toolbar_allocation->width;
	else
		y += toolbar_allocation->y + toolbar_allocation->height;

	gtk_window_get_size (GTK_WINDOW (priv->popup_window),
			     &window_width, &window_height);
	screen = gtk_widget_get_screen (GTK_WIDGET (toolbar));
	screen_width = gdk_screen_get_width (screen);
	screen_height = gdk_screen_get_height (screen);
	if ((x + window_width) > screen_width)
		x -= window_width;
	if ((y + window_height) > screen_height)
		x += toolbar_allocation->width;

	gtk_window_move (GTK_WINDOW (priv->popup_window), x, y);

	g_signal_connect (priv->popup_window, "map",
			  G_CALLBACK (popup_window_map_cb), toolbar);

	gtk_widget_show (priv->popup_window);
}

static void
popup_item_toggled_cb (BonoboUIToolbarToggleButtonItem *toggle_button_item,
		       void *data)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;
	gboolean active;

	toolbar = BONOBO_UI_TOOLBAR (data);
	priv = toolbar->priv;

	active = bonobo_ui_toolbar_toggle_button_item_get_active (toggle_button_item);

	if (active)
		show_popup_window (toolbar);
	else 
		hide_popup_window (toolbar);
}

/* Layout handling.  */

static int
get_popup_item_size (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;
	GtkRequisition requisition;

	priv = toolbar->priv;

	gtk_widget_get_child_requisition (
		GTK_WIDGET (priv->popup_item), &requisition);

	if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
		return requisition.width;
	else
		return requisition.height;
}

/* Update the various sizes.  This is performed during ::size_request.  */

static void
accumulate_item_size (BonoboUIToolbarPrivate *priv,
		      GtkWidget              *item_widget)
{
	GtkRequisition item_requisition;

	gtk_widget_size_request (item_widget, &item_requisition);

	priv->max_width     = MAX (priv->max_width,  item_requisition.width);
	priv->total_width  += item_requisition.width;
	priv->max_height    = MAX (priv->max_height, item_requisition.height);
	priv->total_height += item_requisition.height;
}

static void
update_sizes (BonoboUIToolbar *toolbar)
{
	GList *p;
	BonoboUIToolbarPrivate *priv;

	priv = toolbar->priv;

	priv->max_width = priv->total_width = 0;
	priv->max_height = priv->total_height = 0;

	for (p = priv->items; p != NULL; p = p->next) {
		GtkWidget *item_widget;

		item_widget = GTK_WIDGET (p->data);
		if (! GTK_WIDGET_VISIBLE (item_widget) ||
		    item_widget->parent != GTK_WIDGET (toolbar))
			continue;

		accumulate_item_size (priv, item_widget);
	}

	if (priv->items_moved_to_popup_window)
		accumulate_item_size (priv, GTK_WIDGET (priv->popup_item));
}

static void
allocate_popup_item (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;
	GtkRequisition popup_item_requisition;
	GtkAllocation popup_item_allocation;
	GtkAllocation *toolbar_allocation;
	int border_width;

	priv = toolbar->priv;

	/* FIXME what if there is not enough space?  */

	toolbar_allocation = & GTK_WIDGET (toolbar)->allocation;

	border_width = GTK_CONTAINER (toolbar)->border_width;

	gtk_widget_get_child_requisition (
		GTK_WIDGET (priv->popup_item), &popup_item_requisition);

	popup_item_allocation.x = toolbar_allocation->x;
	popup_item_allocation.y = toolbar_allocation->y;

	if (priv->orientation == GTK_ORIENTATION_HORIZONTAL) {
		popup_item_allocation.x      = priv->end_position - popup_item_requisition.width - border_width;
		popup_item_allocation.y      += border_width;
		popup_item_allocation.width  = popup_item_requisition.width;
		popup_item_allocation.height = toolbar_allocation->height - 2 * border_width;
	} else {
		popup_item_allocation.x      += border_width;
		popup_item_allocation.y      = priv->end_position - popup_item_requisition.height - border_width;
		popup_item_allocation.width  = toolbar_allocation->width - 2 * border_width;
		popup_item_allocation.height = popup_item_requisition.height;
	}

	gtk_widget_size_allocate (GTK_WIDGET (priv->popup_item), &popup_item_allocation);
}

static void
setup_popup_item (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;
	GList *p;

	priv = toolbar->priv;

	if (priv->items_moved_to_popup_window) {
		gtk_widget_show (GTK_WIDGET (priv->popup_item));
		allocate_popup_item (toolbar);
		return;
	}

	for (p = priv->first_not_fitting_item; p != NULL; p = p->next) {
		GtkWidget *item_widget;

		item_widget = GTK_WIDGET (p->data);

		if (GTK_WIDGET_VISIBLE (item_widget)) {
			gtk_widget_show (GTK_WIDGET (priv->popup_item));
			allocate_popup_item (toolbar);
			return;
		}
	}

	gtk_widget_hide (GTK_WIDGET (priv->popup_item));
}

/*
 * This is a dirty hack.  We cannot hide the items with gtk_widget_hide ()
 * because we want to let the user be in control of the physical hidden/shown
 * state, so we just move the widget to a non-visible area.
 */
static void
hide_not_fitting_items (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;
	GtkAllocation child_allocation;
	GList *p;

	priv = toolbar->priv;

	child_allocation.x      = 40000;
	child_allocation.y      = 40000;
	child_allocation.width  = 1;
	child_allocation.height = 1;

	for (p = priv->first_not_fitting_item; p != NULL; p = p->next) {
		if (bonobo_ui_toolbar_item_get_pack_end (BONOBO_UI_TOOLBAR_ITEM (p->data)))
			continue;
		gtk_widget_size_allocate (GTK_WIDGET (p->data), &child_allocation);
	}
}

static void
size_allocate_helper (BonoboUIToolbar *toolbar,
		      const GtkAllocation *allocation)
{
	BonoboUIToolbarPrivate *priv;
	GtkAllocation child_allocation;
	BonoboUIToolbarItem *item;
	GtkRequisition child_requisition;
	int border_width;
	int space_required;
	int available_space;
	int extra_space;
	int num_expandable_items;
	int popup_item_size;
	int item_size_left_to_place;
	int acc_space;
	gboolean first_expandable;
	GList *p;

	GTK_WIDGET (toolbar)->allocation = *allocation;

	priv = toolbar->priv;

	border_width = GTK_CONTAINER (toolbar)->border_width;
	popup_item_size = get_popup_item_size (toolbar);

	if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
		available_space = MAX ((int) allocation->width - 2 * border_width, popup_item_size);
	else
		available_space = MAX ((int) allocation->height - 2 * border_width, popup_item_size);

	child_allocation.x = allocation->x + border_width;
	child_allocation.y = allocation->y + border_width;

	/* 
	 * if there is exactly one toolbar item, handle it specially, by giving it all of the available space,
	 * even if it doesn't fit, since we never want everything in the pop-up.
	 */	 
	if (priv->items != NULL && priv->items->next == NULL) {
		item = BONOBO_UI_TOOLBAR_ITEM (priv->items->data);		
		gtk_widget_get_child_requisition (GTK_WIDGET (item), &child_requisition);
		child_allocation.width = child_requisition.width;
		child_allocation.height = child_requisition.height;

		if (bonobo_ui_toolbar_item_get_expandable (item)) {
			if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
				child_allocation.width = available_space;
			else
				child_allocation.height = available_space;
		
		} 
		gtk_widget_size_allocate (GTK_WIDGET (item), &child_allocation);		
		
		return;
	}

	/* first, make a pass through the items to layout the ones that are packed on the right */
	priv->end_position = allocation->x + available_space;
	acc_space = 0;
	for (p = g_list_last (priv->items); p != NULL; p = p->prev) {

		item = BONOBO_UI_TOOLBAR_ITEM (p->data);
		if (! bonobo_ui_toolbar_item_get_pack_end (item))
			continue;

		gtk_widget_get_child_requisition (GTK_WIDGET (item), &child_requisition);

		if (priv->orientation == GTK_ORIENTATION_HORIZONTAL) {
			acc_space += child_requisition.width;
			item_size_left_to_place -= child_requisition.width;
			priv->end_position -= child_requisition.width;
			
			child_allocation.x = priv->end_position;
			child_allocation.width = child_requisition.width;
			child_allocation.height = priv->max_height;
		} else {
			acc_space += child_requisition.height;
			priv->end_position -= child_requisition.height;
			
			child_allocation.y = priv->end_position;
			child_allocation.height = child_requisition.height;
			child_allocation.width = priv->max_width;
		}
		
		gtk_widget_size_allocate (GTK_WIDGET (item), &child_allocation);
	}
	available_space -= acc_space;
	if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
		item_size_left_to_place = priv->total_width - acc_space;
	else
		item_size_left_to_place = priv->total_height - acc_space;
	
	/* make a pass through the items to determine how many fit */	
	space_required = 0;
	num_expandable_items = 0;

	child_allocation.x = allocation->x + border_width;
	child_allocation.y = allocation->y + border_width;

	for (p = priv->items; p != NULL; p = p->next) {
		int item_size;

		item = BONOBO_UI_TOOLBAR_ITEM (p->data);
		if (! GTK_WIDGET_VISIBLE (item) || GTK_WIDGET (item)->parent != GTK_WIDGET (toolbar) ||
			bonobo_ui_toolbar_item_get_pack_end (item))
			continue;

		gtk_widget_get_child_requisition (GTK_WIDGET (item), &child_requisition);

		if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
			item_size = child_requisition.width;
		else
			item_size = child_requisition.height;

/*		g_message ("Item  size %4d, space_required %4d, available %4d left to place %4d",
			   item_size, space_required, available_space, item_size_left_to_place); */
		
		if (item_size_left_to_place > available_space - space_required &&
		    space_required + item_size > available_space - popup_item_size)
			break;
		
		space_required += item_size;
		item_size_left_to_place -= item_size;

		if (bonobo_ui_toolbar_item_get_expandable (item))
			num_expandable_items ++;
	}

	priv->first_not_fitting_item = p;

	/* determine the amount of space available for expansion */
	if (priv->first_not_fitting_item != NULL) {
		extra_space = 0;
	} else {
		extra_space = available_space - space_required;
		if (priv->first_not_fitting_item != NULL)
			extra_space -= popup_item_size;
	}

	first_expandable = FALSE;

	for (p = priv->items; p != priv->first_not_fitting_item; p = p->next) {
		BonoboUIToolbarItem *item;
		GtkRequisition child_requisition;
		int expansion_amount;

		item = BONOBO_UI_TOOLBAR_ITEM (p->data);
		if (! GTK_WIDGET_VISIBLE (item) || GTK_WIDGET (item)->parent != GTK_WIDGET (toolbar) ||
			bonobo_ui_toolbar_item_get_pack_end (item))
			continue;

		gtk_widget_get_child_requisition (GTK_WIDGET (item), &child_requisition);

		if (! bonobo_ui_toolbar_item_get_expandable (item)) {
			expansion_amount = 0;
		} else {
			g_assert (num_expandable_items != 0);

			expansion_amount = extra_space / num_expandable_items;
			if (first_expandable) {
				expansion_amount += extra_space % num_expandable_items;
				first_expandable = FALSE;
			}
		}

		if (priv->orientation == GTK_ORIENTATION_HORIZONTAL) {
			child_allocation.width  = child_requisition.width + expansion_amount;
			child_allocation.height = priv->max_height;
		} else {
			child_allocation.width  = priv->max_width;
			child_allocation.height = child_requisition.height + expansion_amount;
		}

		gtk_widget_size_allocate (GTK_WIDGET (item), &child_allocation);

		if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
			child_allocation.x += child_allocation.width;
		else
			child_allocation.y += child_allocation.height;
	}

	hide_not_fitting_items (toolbar);
	setup_popup_item (toolbar);
}

/* GObject methods.  */

static void
impl_dispose (GObject *object)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;
	GList *items, *p, *next;

	toolbar = BONOBO_UI_TOOLBAR (object);
	priv = toolbar->priv;

	items = priv->items;
	for (p = items; p; p = next) {
		GtkWidget *item_widget;

		next = p->next;
		item_widget = GTK_WIDGET (p->data);
		if (item_widget->parent == NULL) {
			items = g_list_remove (items, item_widget);
			gtk_widget_destroy (item_widget);
		}
	}

	if (priv->popup_item &&
	    GTK_WIDGET (priv->popup_item)->parent == NULL)
		gtk_widget_destroy (GTK_WIDGET (priv->popup_item));
	priv->popup_item = NULL;

	if (priv->popup_window != NULL)
		gtk_widget_destroy (priv->popup_window);
	priv->popup_window = NULL;

	if (priv->tooltips)
		gtk_object_sink (GTK_OBJECT (priv->tooltips));
	priv->tooltips = NULL;

	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
impl_finalize (GObject *object)
{
	BonoboUIToolbar *toolbar = (BonoboUIToolbar *) object;
	
	g_list_free (toolbar->priv->items);
	g_free (toolbar->priv);

	GNOME_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

/* GtkWidget methods.  */

static void
impl_size_request (GtkWidget *widget,
		   GtkRequisition *requisition)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;
	int border_width;

	toolbar = BONOBO_UI_TOOLBAR (widget);
	priv = toolbar->priv;

	g_assert (priv->popup_item != NULL);

	update_sizes (toolbar);

	border_width = GTK_CONTAINER (toolbar)->border_width;

	if (priv->is_floating) {
		if (priv->orientation == GTK_ORIENTATION_HORIZONTAL) {
			requisition->width  = priv->total_width;
			requisition->height = priv->max_height;
		} else {
			requisition->width  = priv->max_width;
			requisition->height = priv->total_height;
		}
	} else {
		GtkRequisition popup_item_requisition;

		gtk_widget_size_request (GTK_WIDGET (priv->popup_item), &popup_item_requisition);

		if (priv->orientation == GTK_ORIENTATION_HORIZONTAL) {
			requisition->width  = popup_item_requisition.width;
			requisition->height = MAX (popup_item_requisition.height, priv->max_height);
		} else {
			requisition->width  = MAX (popup_item_requisition.width,  priv->max_width);
			requisition->height = popup_item_requisition.height;
		}
	}

	requisition->width  += 2 * border_width;
	requisition->height += 2 * border_width;
}

static void
impl_size_allocate (GtkWidget *widget,
		    GtkAllocation *allocation)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;

	toolbar = BONOBO_UI_TOOLBAR (widget);
	priv = toolbar->priv;

	size_allocate_helper (toolbar, allocation);
}

static void
impl_map (GtkWidget *widget)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;
	GList *p;

	toolbar = BONOBO_UI_TOOLBAR (widget);
	priv = toolbar->priv;

	GTK_WIDGET_SET_FLAGS (toolbar, GTK_MAPPED);

	for (p = priv->items; p != NULL; p = p->next) {
		GtkWidget *item_widget;

		item_widget = GTK_WIDGET (p->data);
		if (item_widget->parent != GTK_WIDGET (toolbar))
			continue;

		if (GTK_WIDGET_VISIBLE (item_widget) && ! GTK_WIDGET_MAPPED (item_widget))
			gtk_widget_map (item_widget);
	}

	if (GTK_WIDGET_VISIBLE (priv->popup_item) && ! GTK_WIDGET_MAPPED (priv->popup_item))
		gtk_widget_map (GTK_WIDGET (priv->popup_item));
}

static void
impl_unmap (GtkWidget *widget)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;
	GList *p;

	toolbar = BONOBO_UI_TOOLBAR (widget);
	priv = toolbar->priv;

	for (p = priv->items; p != NULL; p = p->next) {
		GtkWidget *item_widget;

		item_widget = GTK_WIDGET (p->data);
		if (item_widget->parent != GTK_WIDGET (toolbar))
			continue;

		if (GTK_WIDGET_VISIBLE (item_widget) && GTK_WIDGET_MAPPED (item_widget))
			gtk_widget_unmap (item_widget);
	}

	if (GTK_WIDGET_VISIBLE (priv->popup_item) && GTK_WIDGET_MAPPED (priv->popup_item))
		gtk_widget_unmap (GTK_WIDGET (priv->popup_item));
}

static int
impl_expose_event (GtkWidget *widget,
		   GdkEventExpose *event)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;
	GtkShadowType shadow_type;
	GList *p;

	if (! GTK_WIDGET_DRAWABLE (widget))
		return TRUE;

	toolbar = BONOBO_UI_TOOLBAR (widget);
	priv = toolbar->priv;

	gtk_widget_style_get (widget, "shadow_type", &shadow_type, NULL);

	gtk_paint_box (widget->style,
		       widget->window,
		       GTK_WIDGET_STATE (widget),
		       shadow_type,
		       &event->area, widget, "toolbar",
		       widget->allocation.x,
		       widget->allocation.y,
		       widget->allocation.width,
		       widget->allocation.height);

	for (p = priv->items; p != NULL; p = p->next) {
		GtkWidget *item_widget;

		item_widget = GTK_WIDGET (p->data);
		if (item_widget->parent != GTK_WIDGET (toolbar))
			continue;

		if (! GTK_WIDGET_NO_WINDOW (item_widget))
			continue;

		gtk_container_propagate_expose (
			GTK_CONTAINER (widget), item_widget, event);
	}

	gtk_container_propagate_expose (
		GTK_CONTAINER (widget), GTK_WIDGET (priv->popup_item), event);

	return TRUE;
}


/* GtkContainer methods.  */

static void
impl_remove (GtkContainer *container,
	     GtkWidget    *child)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;

	toolbar = BONOBO_UI_TOOLBAR (container);
	priv = toolbar->priv;

	if (child == (GtkWidget *) priv->popup_item)
		priv->popup_item = NULL;

	gtk_widget_unparent (child);

	gtk_widget_queue_resize (GTK_WIDGET (container));
}

static void
impl_forall (GtkContainer *container,
	     gboolean include_internals,
	     GtkCallback callback,
	     void *callback_data)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;
	GList *p;

	toolbar = BONOBO_UI_TOOLBAR (container);
	priv = toolbar->priv;

	p = priv->items;
	while (p != NULL) {
		GtkWidget *child;
		GList *pnext;

		pnext = p->next;

		child = GTK_WIDGET (p->data);
		if (child->parent == GTK_WIDGET (toolbar))
			(* callback) (child, callback_data);

		p = pnext;
	}

	if (priv->popup_item)
		(* callback) (GTK_WIDGET (priv->popup_item),
			      callback_data);
}


/* BonoboUIToolbar signals.  */

static void
impl_set_orientation (BonoboUIToolbar *toolbar,
		      GtkOrientation orientation)
{
	BonoboUIToolbarPrivate *priv;
	GList *p;

	priv = toolbar->priv;

	if (orientation == priv->orientation)
		return;

	priv->orientation = orientation;

	for (p = priv->items; p != NULL; p = p->next) {
		BonoboUIToolbarItem *item;

		item = BONOBO_UI_TOOLBAR_ITEM (p->data);
		set_attributes_on_child (item, orientation, priv->style);
	}

	bonobo_ui_toolbar_item_set_orientation (
		BONOBO_UI_TOOLBAR_ITEM (priv->popup_item), orientation);

	gtk_widget_queue_resize (GTK_WIDGET (toolbar));
}

static void
impl_style_changed (BonoboUIToolbar *toolbar)
{
	GList *p;
	BonoboUIToolbarStyle style;
	BonoboUIToolbarPrivate *priv;

	priv = toolbar->priv;

	style = (priv->orientation == GTK_ORIENTATION_HORIZONTAL) ? priv->hstyle : priv->vstyle;

	if (style == priv->style)
		return;

	priv->style = style;

	for (p = priv->items; p != NULL; p = p->next) {
		BonoboUIToolbarItem *item;

		item = BONOBO_UI_TOOLBAR_ITEM (p->data);
		set_attributes_on_child (item, priv->orientation, style);
	}

	gtk_widget_queue_resize (GTK_WIDGET (toolbar));
}

static void
impl_get_property (GObject    *object,
		   guint       property_id,
		   GValue     *value,
		   GParamSpec *pspec)
{
	BonoboUIToolbar *toolbar = BONOBO_UI_TOOLBAR (object);
	BonoboUIToolbarPrivate *priv = toolbar->priv;
	gint border_width;

	border_width = GTK_CONTAINER (object)->border_width;
	
	switch (property_id) {
	case PROP_ORIENTATION:
		g_value_set_uint (
			value, bonobo_ui_toolbar_get_orientation (toolbar));
		break;
	case PROP_IS_FLOATING:
		g_value_set_boolean (value, priv->is_floating);
		break;
	case PROP_PREFERRED_WIDTH:
		update_sizes (toolbar);
		if (bonobo_ui_toolbar_get_orientation (toolbar) ==
		    GTK_ORIENTATION_HORIZONTAL)
			g_value_set_uint (value, priv->total_width + 2 * border_width);
		else
			g_value_set_uint (value, priv->max_width + 2 * border_width);
		break;
	case PROP_PREFERRED_HEIGHT:
		update_sizes (toolbar);
		if (bonobo_ui_toolbar_get_orientation (toolbar) ==
		    GTK_ORIENTATION_HORIZONTAL)
			g_value_set_uint (value, priv->max_height + 2 * border_width);
		else
			g_value_set_uint (value, priv->total_height + 2 * border_width);
		break;
	default:
		break;
	};
}

static void
impl_set_property (GObject      *object,
		   guint         property_id,
		   const GValue *value,
		   GParamSpec   *pspec)
{
	BonoboUIToolbar *toolbar = BONOBO_UI_TOOLBAR (object);
	BonoboUIToolbarPrivate *priv = toolbar->priv;

	switch (property_id) {
	case PROP_ORIENTATION:
		bonobo_ui_toolbar_set_orientation (
			toolbar, g_value_get_enum (value));
		break;
	case PROP_IS_FLOATING:
		priv->is_floating = g_value_get_boolean (value);
		break;
	default:
		break;
	};
}

static void
bonobo_ui_toolbar_class_init (BonoboUIToolbarClass *toolbar_class)
{
	GObjectClass *gobject_class;
	GtkWidgetClass *widget_class;
	GtkContainerClass *container_class;

	gobject_class = (GObjectClass *) toolbar_class;
	gobject_class->finalize     = impl_finalize;
	gobject_class->dispose      = impl_dispose;
	gobject_class->get_property = impl_get_property;
	gobject_class->set_property = impl_set_property;

	widget_class = GTK_WIDGET_CLASS (toolbar_class);
	widget_class->size_request  = impl_size_request;
	widget_class->size_allocate = impl_size_allocate;
	widget_class->map           = impl_map;
	widget_class->unmap         = impl_unmap;
	widget_class->expose_event  = impl_expose_event;

	container_class = GTK_CONTAINER_CLASS (toolbar_class);
	container_class->remove = impl_remove;
	container_class->forall = impl_forall;

	toolbar_class->set_orientation = impl_set_orientation;
	toolbar_class->style_changed   = impl_style_changed;


	g_object_class_install_property (
		gobject_class,
		PROP_ORIENTATION,
		g_param_spec_enum ("orientation",
				   _("Orientation"),
				   _("Orientation"),
				   GTK_TYPE_ORIENTATION,
				   GTK_ORIENTATION_HORIZONTAL,
				   G_PARAM_READWRITE));

	g_object_class_install_property (
		gobject_class,
		PROP_IS_FLOATING,
		g_param_spec_boolean ("is_floating",
				      _("is floating"),
				      _("whether the toolbar is floating"),
				      FALSE,
				      G_PARAM_READWRITE));

	g_object_class_install_property (
		gobject_class,
		PROP_PREFERRED_WIDTH,
		g_param_spec_uint ("preferred_width",
				   _("Preferred width"),
				   _("Preferred width"),
				   0, G_MAXINT, 0,
				   G_PARAM_READABLE));

	g_object_class_install_property (
		gobject_class,
		PROP_PREFERRED_HEIGHT,
		g_param_spec_uint ("preferred_height",
				   _("Preferred height"),
				   _("Preferred height"),
				   0, G_MAXINT, 0,
				   G_PARAM_READABLE));

	signals[SET_ORIENTATION]
		= g_signal_new ("set_orientation",
				G_TYPE_FROM_CLASS (gobject_class),
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET (BonoboUIToolbarClass,
						 set_orientation),
				NULL, NULL,
				g_cclosure_marshal_VOID__INT,
				G_TYPE_NONE, 1, G_TYPE_INT);

	signals[STYLE_CHANGED]
		= g_signal_new ("set_style",
				G_TYPE_FROM_CLASS (gobject_class),
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET (BonoboUIToolbarClass,
						 style_changed),
				NULL, NULL,
				g_cclosure_marshal_VOID__VOID,
				G_TYPE_NONE, 0);

	gtk_widget_class_install_style_property (
	        widget_class,
		g_param_spec_enum ("shadow_type",
				   _("Shadow type"),
				   _("Style of bevel around the toolbar"),
				   GTK_TYPE_SHADOW_TYPE,
				   GTK_SHADOW_OUT,
				   G_PARAM_READABLE));
}

static void
bonobo_ui_toolbar_instance_init (BonoboUIToolbar *toolbar)
{
	AtkObject *ao;
	BonoboUIToolbarStyle style;
	BonoboUIToolbarPrivate *priv;

	GTK_WIDGET_SET_FLAGS (toolbar, GTK_NO_WINDOW);

	priv = g_new (BonoboUIToolbarPrivate, 1);

	style = BONOBO_UI_TOOLBAR_STYLE_ICONS_AND_TEXT;

	priv->orientation                 = GTK_ORIENTATION_HORIZONTAL;
	priv->is_floating		  = FALSE;
	priv->style                       = style;
	priv->hstyle                      = style;
	priv->vstyle                      = style;
	priv->max_width			  = 0;
	priv->total_width		  = 0;
	priv->max_height		  = 0;
	priv->total_height		  = 0;
	priv->popup_item                  = NULL;
	priv->items                       = NULL;
	priv->first_not_fitting_item      = NULL;
	priv->popup_window                = NULL;
	priv->popup_window_vbox           = NULL;
	priv->items_moved_to_popup_window = FALSE;
	priv->tooltips                    = gtk_tooltips_new ();

	toolbar->priv = priv;

	ao = gtk_widget_get_accessible (GTK_WIDGET (toolbar));
	if (ao)
		atk_object_set_role (ao, ATK_ROLE_TOOL_BAR);
}

void
bonobo_ui_toolbar_construct (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;
	GtkWidget *frame;

	g_return_if_fail (toolbar != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR (toolbar));

	priv = toolbar->priv;

	priv->popup_item = BONOBO_UI_TOOLBAR_ITEM (bonobo_ui_toolbar_popup_item_new ());
	bonobo_ui_toolbar_item_set_orientation (priv->popup_item, priv->orientation);
	parentize_widget (toolbar, GTK_WIDGET (priv->popup_item));

	g_signal_connect (GTK_OBJECT (priv->popup_item), "toggled",
			    G_CALLBACK (popup_item_toggled_cb), toolbar);

	priv->popup_window = gtk_window_new (GTK_WINDOW_POPUP);
	g_signal_connect (GTK_OBJECT (priv->popup_window), "button_release_event",
			    G_CALLBACK (popup_window_button_release_cb), toolbar);

	frame = gtk_frame_new (NULL);
	gtk_widget_show (frame);
	gtk_container_add (GTK_CONTAINER (priv->popup_window), frame);

	priv->popup_window_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (priv->popup_window_vbox);
	gtk_container_add (GTK_CONTAINER (frame), priv->popup_window_vbox);
}

GtkWidget *
bonobo_ui_toolbar_new (void)
{
	BonoboUIToolbar *toolbar;

	toolbar = g_object_new (bonobo_ui_toolbar_get_type (), NULL);

	bonobo_ui_toolbar_construct (toolbar);

	return GTK_WIDGET (toolbar);
}


void
bonobo_ui_toolbar_set_orientation (BonoboUIToolbar *toolbar,
				   GtkOrientation orientation)
{
	g_return_if_fail (toolbar != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR (toolbar));
	g_return_if_fail (orientation == GTK_ORIENTATION_HORIZONTAL ||
			  orientation == GTK_ORIENTATION_VERTICAL);

	g_signal_emit (toolbar, signals[SET_ORIENTATION], 0, orientation);
	g_signal_emit (toolbar, signals[STYLE_CHANGED], 0);
}

GtkOrientation
bonobo_ui_toolbar_get_orientation (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;

	g_return_val_if_fail (toolbar != NULL, GTK_ORIENTATION_HORIZONTAL);
	g_return_val_if_fail (BONOBO_IS_UI_TOOLBAR (toolbar), GTK_ORIENTATION_HORIZONTAL);

	priv = toolbar->priv;

	return priv->orientation;
}


BonoboUIToolbarStyle
bonobo_ui_toolbar_get_style (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;

	g_return_val_if_fail (toolbar != NULL, BONOBO_UI_TOOLBAR_STYLE_PRIORITY_TEXT);
	g_return_val_if_fail (BONOBO_IS_UI_TOOLBAR (toolbar), BONOBO_UI_TOOLBAR_STYLE_PRIORITY_TEXT);

	priv = toolbar->priv;

	return priv->style;
}

GtkTooltips *
bonobo_ui_toolbar_get_tooltips (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;

	g_return_val_if_fail (toolbar != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_UI_TOOLBAR (toolbar), NULL);

	priv = toolbar->priv;

	return priv->tooltips;
}

void
bonobo_ui_toolbar_insert (BonoboUIToolbar *toolbar,
			  BonoboUIToolbarItem *item,
			  int position)
{
	BonoboUIToolbarPrivate *priv;

	g_return_if_fail (BONOBO_IS_UI_TOOLBAR (toolbar));
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR_ITEM (item));

	priv = toolbar->priv;

	/*
	 *  This ugly hack is here since we might have unparented
	 * a widget and then re-added it to the toolbar at a later
	 * date, and un-parenting doesn't work quite properly yet.
	 *
	 *  Un-parenting is down to the widget possibly being a
	 * child of either this widget, or the popup window.
	 */
	if (!g_list_find (priv->items, item)) {
		g_object_ref (item);
		gtk_object_sink (GTK_OBJECT (item));
		priv->items = g_list_insert (priv->items, item, position);
	}

	g_signal_connect_object (
		item, "destroy",
		G_CALLBACK (item_destroy_cb),
		toolbar, 0);
	g_signal_connect_object (
		item, "activate",
		G_CALLBACK (item_activate_cb),
		toolbar, 0);
	g_signal_connect_object (
		item, "set_want_label",
		G_CALLBACK (item_set_want_label_cb),
		toolbar, 0);

	g_object_ref (toolbar);
	g_object_ref (item);

	set_attributes_on_child (item, priv->orientation, priv->style);
	parentize_widget (toolbar, GTK_WIDGET (item));

	gtk_widget_queue_resize (GTK_WIDGET (toolbar));

	g_object_unref (item);
	g_object_unref (toolbar);
}

GList *
bonobo_ui_toolbar_get_children (BonoboUIToolbar *toolbar)
{
	GList *ret = NULL, *l;

	g_return_val_if_fail (BONOBO_IS_UI_TOOLBAR (toolbar), NULL);

	for (l = toolbar->priv->items; l; l = l->next) {
		GtkWidget *item_widget;

		item_widget = GTK_WIDGET (l->data);
		if (item_widget->parent != NULL) /* Unparented but still here */
			ret = g_list_prepend (ret, item_widget);
	}

	return g_list_reverse (ret);
}

void
bonobo_ui_toolbar_set_hv_styles (BonoboUIToolbar      *toolbar,
				 BonoboUIToolbarStyle  hstyle,
				 BonoboUIToolbarStyle  vstyle)
{
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR (toolbar));

	toolbar->priv->hstyle = hstyle;
	toolbar->priv->vstyle = vstyle;

	g_signal_emit (toolbar, signals [STYLE_CHANGED], 0);
}

void
bonobo_ui_toolbar_show_tooltips (BonoboUIToolbar *toolbar,
				 gboolean         show_tips)
{
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR (toolbar));

	if (show_tips)
		gtk_tooltips_enable (toolbar->priv->tooltips);
	else
		gtk_tooltips_disable (toolbar->priv->tooltips);
}
