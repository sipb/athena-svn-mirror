/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * e-group-bar.c
 * Copyright 1999, 2000, 2001, Ximian, Inc.
 *
 * Authors:
 *   Damon Chaplin <damon@ximian.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License, version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

/*
 * EGroupBar displays a vertical bar with a number of Groups, which are viewed
 * one at a time by selecting the Group's button. When a different Group is
 * selected, it slides into view, and the old Group slides out.
 * It is typically used on the left of the main application window so users
 * can easily access particular features.
 *
 * It is implemented like GtkNotebook, i.e. the main widgets are the children
 * of the EGroupBar and the button widgets are treated specially like the
 * GtkNotebook tab labels.
 */

#include <config.h>
#include "gal/util/e-i18n.h"
#include "gal/util/e-util.h"
#include <math.h>
#include <gnome.h>
#include "e-group-bar.h"

#define E_GROUP_BAR_SCROLL_TIMEOUT	10
#define E_GROUP_BAR_MIN_STEP_SIZE	4

#define E_GROUP_BAR_AUTO_SHOW_TIMEOUT	300


static void e_group_bar_class_init (EGroupBarClass *class);
static void e_group_bar_init (EGroupBar *group_bar);
static void e_group_bar_dispose (GObject *object);
static void e_group_bar_realize (GtkWidget *widget);
static void e_group_bar_unrealize (GtkWidget *widget);
static void e_group_bar_map (GtkWidget *widget);
static void e_group_bar_unmap (GtkWidget *widget);
static void e_group_bar_size_request (GtkWidget      *widget,
				      GtkRequisition *requisition);
static void e_group_bar_size_allocate (GtkWidget *widget,
				       GtkAllocation *allocation);
static void e_group_bar_add (GtkContainer *container,
			     GtkWidget    *widget);
static void e_group_bar_remove (GtkContainer     *container,
				GtkWidget        *widget);
static void e_group_bar_forall (GtkContainer   *container,
				gboolean	include_internals,
				GtkCallback     callback,
				gpointer        callback_data);

static void e_group_bar_create_group_button_window (EGroupBar *group_bar,
						    gint group_num);
static void e_group_bar_create_group_child_window (EGroupBar *group_bar,
						   gint group_num);
static gint e_group_bar_get_group_button_position (EGroupBar *group_bar,
						   gint group_num);
static gint e_group_bar_sum_button_heights (EGroupBar *group_bar,
					    gint first,
					    gint last);
static gint e_group_bar_get_child_height (EGroupBar *group_bar);
static gint e_group_bar_get_group_child_position (EGroupBar *group_bar,
						  gint group_num);

static void e_group_bar_on_button_clicked (GtkWidget *group_button,
					   EGroupBar *group_bar);
static gint e_group_bar_find_button (EGroupBar *group_bar,
				     GtkWidget *group_button);
static void e_group_bar_start_animation (EGroupBar *group_bar,
					 gint group_num);
static gboolean e_group_bar_timeout_handler (gpointer data);
static gint e_group_bar_get_increment (EGroupBar *group_bar,
				       gint window_y,
				       gint window_target_y);
static gboolean e_group_bar_on_button_drag_motion (GtkWidget	      *widget,
						   GdkDragContext     *context,
						   gint                x,
						   gint                y,
						   guint               time,
						   EGroupBar	      *group_bar);
static void e_group_bar_on_button_drag_leave (GtkWidget	     *widget,
					      GdkDragContext     *context,
					      guint               time,
					      EGroupBar	      *group_bar);
static gboolean e_group_bar_auto_show (gpointer data);
static void e_group_bar_stop_all_animation (EGroupBar	*group_bar);


#define PARENT_TYPE GTK_TYPE_CONTAINER
static GtkContainerClass *parent_class;


E_MAKE_TYPE (e_group_bar,
	     "EGroupBar",
	     EGroupBar,
	     e_group_bar_class_init,
	     e_group_bar_init,
	     PARENT_TYPE)

static void
e_group_bar_class_init (EGroupBarClass *class)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GtkContainerClass *container_class;

	object_class = (GObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;
	container_class = (GtkContainerClass *) class;

	parent_class = g_type_class_ref (PARENT_TYPE);

	/* Method override */
	object_class->dispose		= e_group_bar_dispose;

	widget_class->realize		= e_group_bar_realize;
	widget_class->unrealize		= e_group_bar_unrealize;
	widget_class->map		= e_group_bar_map;
	widget_class->unmap		= e_group_bar_unmap;
	widget_class->size_request	= e_group_bar_size_request;
 	widget_class->size_allocate	= e_group_bar_size_allocate;

	container_class->add		= e_group_bar_add;
	container_class->remove		= e_group_bar_remove;
	container_class->forall		= e_group_bar_forall;
}


static void
e_group_bar_init (EGroupBar *group_bar)
{

	GTK_WIDGET_UNSET_FLAGS (group_bar, GTK_NO_WINDOW);

	/* We don't want child resizes to propagate up to the parent. */
	gtk_container_set_resize_mode (GTK_CONTAINER (group_bar),
				       GTK_RESIZE_QUEUE);

	group_bar->children = g_array_new (FALSE, FALSE,
					   sizeof (EGroupBarChild));

	group_bar->current_group_num = -1;
	group_bar->buttons_homogeneous = TRUE;
	group_bar->max_button_height = 0;
	group_bar->animation_timeout_id = 0;
}


/**
 * e_group_bar_new:
 * @Returns: a new #EGroupBar.
 *
 * Creates a new #EGroupBar.
 **/
GtkWidget *
e_group_bar_new (void)
{
	GtkWidget *group_bar;

	group_bar = GTK_WIDGET (g_object_new (E_GROUP_BAR_TYPE, NULL));

	return group_bar;
}


static void
e_group_bar_dispose (GObject *object)
{
	EGroupBar *group_bar;

	group_bar = E_GROUP_BAR (object);

	e_group_bar_stop_all_animation (group_bar);

	/* The parent GtkContainer class will automatically destroy all the
	   child widgets, but it calls gtk_container_foreach() so we must not
	   destroy our children array until after. */
	G_OBJECT_CLASS (parent_class)->dispose (object);

	if (group_bar->children)
	    g_array_free (group_bar->children, TRUE);
	group_bar->children = NULL;
}

static void
e_group_bar_realize (GtkWidget *widget)
{
	EGroupBar *group_bar;
	EGroupBarChild *group;
	GdkWindowAttr attributes;
	gint attributes_mask;
	gint border_width, group_num;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (E_IS_GROUP_BAR (widget));

	group_bar = E_GROUP_BAR (widget);
	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

	border_width = GTK_CONTAINER (group_bar)->border_width;

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x + border_width;
	attributes.y = widget->allocation.y + border_width;
	attributes.width = widget->allocation.width - 2 * border_width;
	attributes.height = widget->allocation.height - 2 * border_width;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = gtk_widget_get_visual (widget);
	attributes.colormap = gtk_widget_get_colormap (widget);
	attributes.event_mask = gtk_widget_get_events (widget);
	attributes.event_mask |= (GDK_EXPOSURE_MASK);

	attributes_mask = GDK_WA_X | GDK_WA_Y
		| GDK_WA_VISUAL | GDK_WA_COLORMAP;

	widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
					 &attributes, attributes_mask);
	gdk_window_set_user_data (widget->window, widget);

	widget->style = gtk_style_attach (widget->style, widget->window);
	gtk_style_set_background (widget->style, widget->window,
				  GTK_STATE_ACTIVE);

	gdk_window_set_back_pixmap (widget->window, NULL, FALSE);

	/* Create windows for all the buttons & group canvases. */
	for (group_num = 0;
	     group_num < group_bar->children->len;
	     group_num++) {
		group = &g_array_index (group_bar->children,
					EGroupBarChild, group_num);
		e_group_bar_create_group_button_window (group_bar, group_num);
		e_group_bar_create_group_child_window (group_bar, group_num);
	}
}


static void
e_group_bar_unrealize (GtkWidget *widget)
{
	EGroupBar *group_bar;
	EGroupBarChild *group;
	gint group_num;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (E_IS_GROUP_BAR (widget));

	group_bar = E_GROUP_BAR (widget);

	/* Destroy the windows for all the buttons & group canvases. */
	for (group_num = 0;
	     group_num < group_bar->children->len;
	     group_num++) {
		group = &g_array_index (group_bar->children,
					EGroupBarChild, group_num);

		if (group->button_window) {
			gdk_window_set_user_data (group->button_window, NULL);
			gdk_window_destroy (group->button_window);
			group->button_window = NULL;
		}
		if (group->child_window) {
			gdk_window_set_user_data (group->child_window, NULL);
			gdk_window_destroy (group->child_window);
			group->child_window = NULL;
		}
	}

	if (GTK_WIDGET_CLASS (parent_class)->unrealize)
		(* GTK_WIDGET_CLASS (parent_class)->unrealize) (widget);
}


static void
e_group_bar_map (GtkWidget *widget)
{
	EGroupBar *group_bar;
	EGroupBarChild *group;
	gint group_num;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (E_IS_GROUP_BAR (widget));

	group_bar = E_GROUP_BAR (widget);

	GTK_WIDGET_SET_FLAGS (widget, GTK_MAPPED);

	/* We do this in reverse order, and lower all the child windows, so
	   the stacking order ends up correct. */
	for (group_num = group_bar->children->len - 1;
	     group_num >= 0;
	     group_num--) {
		group = &g_array_index (group_bar->children,
					EGroupBarChild, group_num);

		if (group->button_window) {
			gdk_window_show (group->button_window);
		}

		if (group->button
		    && GTK_WIDGET_VISIBLE (group->button)
		    && !GTK_WIDGET_MAPPED (group->button)) {
			gtk_widget_map (group->button);
		}

		if (group->child_window) {
			gdk_window_show (group->child_window);
			gdk_window_lower (group->child_window);
		}

		if (group->child
		    && GTK_WIDGET_VISIBLE (group->child)
		    && !GTK_WIDGET_MAPPED (group->child))
			gtk_widget_map (group->child);
	}
      
	gdk_window_show (widget->window);
}


static void
e_group_bar_unmap (GtkWidget *widget)
{
	EGroupBar *group_bar;
	EGroupBarChild *group;
	gint group_num;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (E_IS_GROUP_BAR (widget));

	group_bar = E_GROUP_BAR (widget);

	e_group_bar_stop_all_animation (group_bar);

	GTK_WIDGET_UNSET_FLAGS (widget, GTK_MAPPED);

	for (group_num = 0;
	     group_num < group_bar->children->len;
	     group_num++) {
		group = &g_array_index (group_bar->children,
					EGroupBarChild, group_num);

		if (group->button_window) {
			gdk_window_hide (group->button_window);
		}

		if (group->button
		    && GTK_WIDGET_MAPPED (group->button))
			gtk_widget_unmap (group->button);

		if (group->child_window) {
			gdk_window_hide (group->child_window);
		}

		if (group->child
		    && GTK_WIDGET_MAPPED (group->child))
			gtk_widget_unmap (group->child);
	}

	gdk_window_hide (widget->window);
}


static void
e_group_bar_size_request (GtkWidget      *widget,
			  GtkRequisition *requisition)
{
	EGroupBar *group_bar;
	EGroupBarChild *group;
	gint group_num, max_child_height;
	GtkRequisition child_requisition;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (E_IS_GROUP_BAR (widget));
	g_return_if_fail (requisition != NULL);

	group_bar = E_GROUP_BAR (widget);

	/* We set the requisition width to the largest requested width of the
	   child widgets. The requisition height is set to the sum of all the
	   button heights plus the height of the largest child. */
	requisition->width = 0;
	requisition->height = 0;

	/* We have to call size_request on all children, even though we don't
	   use the results, since some widgets like GtkLabel depend on it. */
	group_bar->max_button_height = 0;
	max_child_height = 0;
	for (group_num = 0;
	     group_num < group_bar->children->len;
	     group_num++) {
		group = &g_array_index (group_bar->children,
					EGroupBarChild, group_num);

		if (group->button) {
			gtk_widget_size_request (group->button,
						 &child_requisition);
			group->button_height = child_requisition.height;
		} else {
			group->button_height = 0;
		}

		group_bar->max_button_height = MAX (group_bar->max_button_height, group->button_height);
		requisition->height += child_requisition.height;

		if (group->child) {
			gtk_widget_size_request (group->child,
						 &child_requisition);
			max_child_height = MAX (max_child_height,
						child_requisition.height);
			requisition->width = MAX (requisition->width,
						  child_requisition.width);
		}
	}

	requisition->height += max_child_height;

	/* Add on the standard container border widths. */
	requisition->width += GTK_CONTAINER (widget)->border_width * 2;
	requisition->height += GTK_CONTAINER (widget)->border_width * 2;
}


static void
e_group_bar_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	EGroupBar *group_bar;
	EGroupBarChild *group;
	gint group_num, border_width, width, height, child_height, y;
	GtkAllocation button_allocation, child_allocation;

	group_bar = E_GROUP_BAR (widget);

	/* All child & button windows and widgets use the same width as the
	   group bar minus the border width. */
	border_width = GTK_CONTAINER (widget)->border_width;
	width = allocation->width - border_width * 2;
	height = allocation->height - border_width * 2;

	widget->allocation = *allocation;
	if (GTK_WIDGET_REALIZED (widget))
		gdk_window_move_resize (widget->window,
					allocation->x + border_width,
					allocation->y + border_width,
					width, height);

	/* All the child widgets use the same height. */
	child_height = e_group_bar_get_child_height (group_bar);

	/* The buttons are always in the top-left of the button windows, and
	   all have the same width. The height is calculated for each group. */
	button_allocation.x = 0;
	button_allocation.y = 0;
	button_allocation.width = width;

	/* The child widgets are always in the top-left of the child windows,
	   and all have the same width and height. */
	child_allocation.x = 0;
	child_allocation.y = 0;
	child_allocation.width = width;
	child_allocation.height = child_height;

	/* Step through the groups, placing the windows as necessary, and
	   allocating the areas for the child widgets. Note that if a button
	   or child window is in the middle of an animation, we just resize it
	   and update the target position, and let the animation continue. */
	for (group_num = 0;
	     group_num < group_bar->children->len;
	     group_num++) {
		group = &g_array_index (group_bar->children,
					EGroupBarChild, group_num);

		/* Calculate the y position of the button, which depends on
		   the currently selected group and the button heights. */
		y = e_group_bar_get_group_button_position (group_bar, group_num);
		button_allocation.height = group_bar->buttons_homogeneous ? group_bar->max_button_height : group->button_height;

		if (GTK_WIDGET_REALIZED (group->button)) {
			if (group->button_window_in_animation) {
				gdk_window_resize (group->button_window,
						   width, button_allocation.height);
				group->button_window_target_y = y;
			} else {
				gdk_window_move_resize (group->button_window,
							0, y, width, button_allocation.height);
			}
			gdk_window_raise (group->button_window);
		}
		gtk_widget_size_allocate (group->button, &button_allocation);

		if (GTK_WIDGET_REALIZED (group->child)) {
			if (group->child_window_in_animation) {
				gdk_window_resize (group->child_window,
						   width, child_height);
				group->child_window_target_y = y + button_allocation.height;
				gdk_window_raise (group->child_window);
			} else {
				gdk_window_move_resize (group->child_window,
							0, y + button_allocation.height,
							width, child_height);
				if (group_num == group_bar->current_group_num)
					gdk_window_raise (group->child_window);
			}
		}
		gtk_widget_size_allocate (group->child, &child_allocation);
	}
}


static void
e_group_bar_add (GtkContainer *container,
		 GtkWidget    *widget)
{
	EGroupBar *group_bar;
	GtkWidget *button;
	gchar buffer[32];

	g_return_if_fail (container != NULL);
	g_return_if_fail (E_IS_GROUP_BAR (container));
	g_return_if_fail (widget != NULL);

	group_bar = E_GROUP_BAR (container);

	g_snprintf (buffer, sizeof (buffer), _("Group %i"),
		    group_bar->children->len + 1);
	button = gtk_button_new_with_label (buffer);
	gtk_widget_show (button);

	e_group_bar_add_group (group_bar, widget, button, -1);
}


static void
e_group_bar_remove (GtkContainer     *container,
		    GtkWidget        *widget)
{
	EGroupBar *group_bar;
	gint group_num;

	g_return_if_fail (container != NULL);
	g_return_if_fail (E_IS_GROUP_BAR (container));
	g_return_if_fail (widget != NULL);

	group_bar = E_GROUP_BAR (container);

	group_num = e_group_bar_get_group_num (group_bar, widget);
	e_group_bar_remove_group (group_bar, group_num);
}


static void
e_group_bar_forall (GtkContainer   *container,
		    gboolean	    include_internals,
		    GtkCallback     callback,
		    gpointer        callback_data)
{
	EGroupBar *group_bar;
	EGroupBarChild *group;
	gint group_num;
	GList *tmp_list;

	g_return_if_fail (container != NULL);
	g_return_if_fail (E_IS_GROUP_BAR (container));
	g_return_if_fail (callback != NULL);

	group_bar = E_GROUP_BAR (container);

	if (group_bar->children == NULL)
		return;

	/* Note that drag-and-drop does not check the Z-order of widgets, so
	   we have to iterate through them from top to bottom, or it will
	   not work properly. We also have to use temporary lists so widgets
	   can be safely destroyed while iterating. */

	if (include_internals) {
		tmp_list = NULL;
		for (group_num = group_bar->children->len - 1;
		     group_num >= 0;
		     group_num--) {
			group = &g_array_index (group_bar->children,
						EGroupBarChild, group_num);

			if (group->button)
				tmp_list = g_list_prepend (tmp_list,
							   group->button);
		}

		g_list_foreach (tmp_list, (GFunc) callback, callback_data);
		g_list_free (tmp_list);
	}

	tmp_list = NULL;
	for (group_num = group_bar->children->len - 1;
	     group_num >= 0;
	     group_num--) {
		group = &g_array_index (group_bar->children,
					EGroupBarChild, group_num);

		if (group->child)
			tmp_list = g_list_prepend (tmp_list, group->child);
	}
	g_list_foreach (tmp_list, (GFunc) callback, callback_data);
	g_list_free (tmp_list);
}


static void
e_group_bar_create_group_button_window (EGroupBar *group_bar,
					gint group_num)
{
	EGroupBarChild *group;
	GtkWidget *widget;
	GdkWindowAttr attributes;
	gint attributes_mask;
	gint y, height, border_width;

	widget = GTK_WIDGET (group_bar);

	group = &g_array_index (group_bar->children,
				EGroupBarChild, group_num);
	y = e_group_bar_get_group_button_position (group_bar, group_num);
	height = group_bar->buttons_homogeneous ? group_bar->max_button_height
		: group->button_height;
	border_width = GTK_CONTAINER (group_bar)->border_width;

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = 0;
	attributes.y = y;
	attributes.width = widget->allocation.width - 2 * border_width;
	attributes.height = height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = gtk_widget_get_visual (widget);
	attributes.colormap = gtk_widget_get_colormap (widget);
	attributes.event_mask = gtk_widget_get_events (widget);
	attributes.event_mask |= (GDK_EXPOSURE_MASK);

	attributes_mask = GDK_WA_X | GDK_WA_Y
		| GDK_WA_VISUAL | GDK_WA_COLORMAP;

	group->button_window = gdk_window_new (widget->window, &attributes,
					       attributes_mask);
	gdk_window_set_user_data (group->button_window, widget);

	gtk_widget_set_parent_window (group->button,
				      group->button_window);
	gdk_window_set_back_pixmap (group->button_window, NULL, TRUE);
}


static void
e_group_bar_create_group_child_window (EGroupBar *group_bar,
				       gint group_num)
{
	EGroupBarChild *group;
	GtkWidget *widget;
	GdkWindowAttr attributes;
	gint attributes_mask;
	gint y, height, border_width;

	widget = GTK_WIDGET (group_bar);

	group = &g_array_index (group_bar->children,
				EGroupBarChild, group_num);
	y = e_group_bar_get_group_button_position (group_bar, group_num);
	y += group_bar->buttons_homogeneous ? group_bar->max_button_height
		: group->button_height;
	height = e_group_bar_get_child_height (group_bar);
	border_width = GTK_CONTAINER (group_bar)->border_width;

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = 0;
	attributes.y = y;
	attributes.width = widget->allocation.width - 2 * border_width;
	attributes.height = height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = gtk_widget_get_visual (widget);
	attributes.colormap = gtk_widget_get_colormap (widget);
	attributes.event_mask = gtk_widget_get_events (widget);
	attributes.event_mask |= (GDK_EXPOSURE_MASK);

	attributes_mask = GDK_WA_X | GDK_WA_Y
		| GDK_WA_VISUAL | GDK_WA_COLORMAP;

	group->child_window = gdk_window_new (widget->window, &attributes,
					       attributes_mask);
	gdk_window_set_user_data (group->child_window, widget);

	gtk_widget_set_parent_window (GTK_WIDGET (group->child),
				      group->child_window);
	gdk_window_set_back_pixmap (group->child_window, NULL, TRUE);
}


/* This returns the y position of a group's button within the EGroupBar window.
 */
static gint
e_group_bar_get_group_button_position (EGroupBar *group_bar,
				       gint group_num)
{
	gint border_width, window_height, y;

	border_width = GTK_CONTAINER (group_bar)->border_width;
	window_height = GTK_WIDGET (group_bar)->allocation.height - 2 * border_width;

	if (group_num <= group_bar->current_group_num)
		y = e_group_bar_sum_button_heights (group_bar, 0, group_num - 1);
	else
		y =  window_height - e_group_bar_sum_button_heights (group_bar, group_num, group_bar->children->len - 1);

	return y;
}


/* This returns the sum of all the buttons from first to last inclusive. */
static gint
e_group_bar_sum_button_heights (EGroupBar *group_bar, gint first, gint last)
{
	EGroupBarChild *group;
	gint height, group_num;

	height = 0;

	if (group_bar->buttons_homogeneous)
		return (last - first + 1) * group_bar->max_button_height;

	for (group_num = first; group_num <= last; group_num++) {
		group = &g_array_index (group_bar->children,
					EGroupBarChild, group_num);
		height += group->button_height;
	}

	return height;
}


static gint
e_group_bar_get_group_child_position (EGroupBar *group_bar,
				      gint group_num)
{
	EGroupBarChild *group;
	gint y;

	y = e_group_bar_get_group_button_position (group_bar, group_num);
	group = &g_array_index (group_bar->children,
				EGroupBarChild, group_num);
	y += group_bar->buttons_homogeneous ? group_bar->max_button_height
		: group->button_height;

	return y;
}


static gint
e_group_bar_get_child_height (EGroupBar *group_bar)
{
	EGroupBarChild *group;
	gint group_num;

	/* Start with the allocated height of the EGroupBar, less the border.*/
	group_bar->child_height = GTK_WIDGET (group_bar)->allocation.height;
	group_bar->child_height -= 2 * GTK_CONTAINER (group_bar)->border_width;

	/* Now subtract the heights of all the buttons. */
	if (group_bar->buttons_homogeneous) {
		group_bar->child_height -= group_bar->children->len * group_bar->max_button_height;
	} else {
		for (group_num = 0;
		     group_num < group_bar->children->len;
		     group_num++) {
			group = &g_array_index (group_bar->children,
						EGroupBarChild, group_num);
			group_bar->child_height -= group->button_height;
		}
	}

	return group_bar->child_height;
}


/*
 * Insertion, reordering and deletion of items.
 */

/**
 * e_group_bar_add_group:
 * @group_bar: an #EGroupBar.
 * @child: the child widget to add.
 * @button: the button used to show the child widget.
 * @position: the new group's position, or -1 to place it last.
 * @Returns: the position of the new group.
 *
 * Adds a new group to a #EGroupBar at the given position.
 **/
gint
e_group_bar_add_group		(EGroupBar	*group_bar,
				 GtkWidget	*child,
				 GtkWidget	*button,
				 gint		 position)
{
	EGroupBarChild *group, empty_group, *tmp_group;
	gint group_num, tmp_group_num;

	g_return_val_if_fail (group_bar != NULL, -1);
	g_return_val_if_fail (E_IS_GROUP_BAR (group_bar), -1);
	g_return_val_if_fail (child != NULL, -1);
	g_return_val_if_fail (button != NULL, -1);
	g_return_val_if_fail (GTK_IS_BUTTON (button), -1);

	/* Append an empty group to the children array and get a pointer to
	   it, so we can use it like a normal group. */
	if (position == -1)
		group_num = group_bar->children->len;
	else
		group_num = position;
	g_array_insert_val (group_bar->children, group_num, empty_group);
	group = &g_array_index (group_bar->children,
				EGroupBarChild, group_num);

	/* Initialize the group. */
	group->button = button;
	group->button_window = NULL;
	group->child = child;
	group->child_window = NULL;
	group->button_window_in_animation = FALSE;
	group->child_window_in_animation = FALSE;
	group->button_window_target_y = 0;
	group->child_window_target_y = 0;

	/* If we don't have a current group, set it to the first one.
	   Move the currently shown group index forward if necessary. */
	if (group_bar->current_group_num == -1)
		group_bar->current_group_num = 0;
	else if (group_bar->current_group_num >= group_num)
		group_bar->current_group_num++;

	/* If the EGroupBar widget is realize, we need to create the child
	   windows to put the button & child in. */
	if (GTK_WIDGET_REALIZED (group_bar)) {
		e_group_bar_create_group_button_window (group_bar, group_num);
		gdk_window_show (group->button_window);

		e_group_bar_create_group_child_window (group_bar, group_num);
		gdk_window_show (group->child_window);

		/* We need to lower all the child windows of the previous
		   groups, in reverse order, to keep the stacking order
		   correct. */
		for (tmp_group_num = group_num;
		     tmp_group_num >= 0;
		     tmp_group_num--) {
			tmp_group = &g_array_index (group_bar->children,
						    EGroupBarChild,
						    tmp_group_num);
			gdk_window_lower (tmp_group->child_window);
		}
	}

	gtk_widget_set_parent (group->button, GTK_WIDGET (group_bar));
	gtk_widget_set_parent (group->child, GTK_WIDGET (group_bar));

	if (GTK_WIDGET_REALIZED (group_bar)) {
		gtk_widget_realize (group->button);
		gtk_widget_realize (group->child);
	}

	if (GTK_WIDGET_VISIBLE (group_bar)
	    && GTK_WIDGET_MAPPED (group_bar)) {
		if (group->button
		    && GTK_WIDGET_VISIBLE (group->button)
		    && !GTK_WIDGET_MAPPED (group->button)) {
			gtk_widget_map (group->button);
			gtk_widget_queue_resize (group->button);
		}
		if (group->child
		    && GTK_WIDGET_VISIBLE (group->child)
		    && !GTK_WIDGET_MAPPED (group->child)) {
			gtk_widget_map (group->child);
			gtk_widget_queue_resize (group->child);
		}
	}

	g_signal_connect (group->button, "clicked",
			  G_CALLBACK (e_group_bar_on_button_clicked),
			  group_bar);
	
	g_signal_connect (group->button, "drag_leave",
			  G_CALLBACK (e_group_bar_on_button_drag_leave),
			  group_bar);

	/* Set up the button as a drop target.  */

	gtk_drag_dest_set (GTK_WIDGET (group->button),
			   0, NULL, 0,
			   GDK_ACTION_COPY | GDK_ACTION_MOVE);

	g_signal_connect (group->button, "drag_motion",
			  G_CALLBACK (e_group_bar_on_button_drag_motion),
			  group_bar);
	return group_num;
}


/**
 * e_group_bar_reorder_group:
 * @group_bar: an #EGroupBar.
 * @group_num: the index of the group to move.
 * @new_position: the new position of the group.
 *
 * Moves a group to a new position within the #EGroupBar.
 **/
void
e_group_bar_reorder_group	(EGroupBar	*group_bar,
				 gint		 group_num,
				 gint		 new_position)
{
	EGroupBarChild group, *tmp_group;
	gint tmp_group_num;

	g_return_if_fail (E_IS_GROUP_BAR (group_bar));
	g_return_if_fail (group_num >= 0);
	g_return_if_fail (group_num < group_bar->children->len);

	e_group_bar_stop_all_animation (group_bar);

	/* Copy the group. */
	group = g_array_index (group_bar->children,
			       EGroupBarChild, group_num);

	/* Remove the group from its current position. */
	g_array_remove_index (group_bar->children, group_num);

	/* Copy the group into its new position. */
	g_array_insert_val (group_bar->children, new_position, group);

	/* We need to lower the groups' windows so they are in the correct
	   z-order. We can skip unaffected windows. */
	for (tmp_group_num = MAX (group_num, new_position);
	     tmp_group_num >= 0;
	     tmp_group_num--) {
		tmp_group = &g_array_index (group_bar->children,
					    EGroupBarChild, tmp_group_num);
		gdk_window_lower (tmp_group->child_window);
	}

	/* Queue a resize so the groups get layed out properly. */
	gtk_widget_queue_resize (GTK_WIDGET (group_bar));
}


/**
 * e_group_bar_remove_group:
 * @group_bar: an #EGroupBar.
 * @group_num: the index of the group to remove.
 *
 * Removes a group from an #EGroupBar.
 **/
void
e_group_bar_remove_group	(EGroupBar	*group_bar,
				 gint		 group_num)
{
	EGroupBarChild *group;

	g_return_if_fail (E_IS_GROUP_BAR (group_bar));
	g_return_if_fail (group_num >= 0);
	g_return_if_fail (group_num < group_bar->children->len);

	group = &g_array_index (group_bar->children,
				EGroupBarChild, group_num);

	/* Stop any animation. */
	e_group_bar_stop_all_animation (group_bar);

	gtk_widget_unparent (group->child);
	if (group->button)
		gtk_widget_unparent (group->button);

	if (group->button_window) {
		gdk_window_set_user_data (group->button_window, NULL);
		gdk_window_destroy (group->button_window);
	}
	if (group->child_window) {
		gdk_window_set_user_data (group->child_window, NULL);
		gdk_window_destroy (group->child_window);
	}

	g_array_remove_index (group_bar->children, group_num);

	/* Make sure the current group is valid. */
	if (group_bar->current_group_num >= group_bar->children->len)
		group_bar->current_group_num = group_bar->children->len - 1;

	gtk_widget_queue_resize (GTK_WIDGET (group_bar));
}


/*
 * Getting & setting the current group.
 */

/**
 * e_group_bar_get_current_group_num:
 * @group_bar: an #EGroupBar.
 * @Returns: the index of the group currently displayed.
 *
 * Returns the index of the group currently displayed.
 **/
gint
e_group_bar_get_current_group_num	(EGroupBar	*group_bar)
{
	g_return_val_if_fail (E_IS_GROUP_BAR (group_bar), -1);

	return group_bar->current_group_num;
}


/**
 * e_group_bar_set_current_group_num:
 * @group_bar: an #EGroupBar.
 * @animate: if TRUE, and the #EGroupBar is visible, the group will slide into
 * position, as if the group's button was pressed.
 * @Returns: the index of the group to display.
 *
 * Sets the group to display.
 **/
void
e_group_bar_set_current_group_num	(EGroupBar	*group_bar,
					 gint		 group_num,
					 gboolean	 animate)
{
	g_return_if_fail (E_IS_GROUP_BAR (group_bar));

	/* If that already is the current group, just return. */
	if (group_bar->current_group_num == group_num)
		return;

	if (GTK_WIDGET_VISIBLE (group_bar)) {
		if (animate) {
			e_group_bar_start_animation (group_bar, group_num);
		} else {
			group_bar->current_group_num = group_num;
			e_group_bar_stop_all_animation (group_bar);
			gtk_widget_queue_resize (GTK_WIDGET (group_bar));
		}
	} else {
		/* The positions will be sorted out when the widget's size is
		   allocated. */
		group_bar->current_group_num = group_num;
		gtk_widget_queue_resize (GTK_WIDGET (group_bar));
	}
}


/*
 * Getting groups and group numbers.
 */

/**
 * e_group_bar_get_nth_group:
 * @group_bar: an #EGroupBar.
 * @group_num: the index of the group to get.
 * @Returns: the child widget at the given index.
 *
 * Returns the child widget at the given index.
 **/
GtkWidget*
e_group_bar_get_nth_group	(EGroupBar	*group_bar,
				 gint		 group_num)
{
	EGroupBarChild *group;

	g_return_val_if_fail (E_IS_GROUP_BAR (group_bar), NULL);
	g_return_val_if_fail (group_num >= 0, NULL);
	g_return_val_if_fail (group_num < group_bar->children->len, NULL);

	group = &g_array_index (group_bar->children,
				EGroupBarChild, group_num);
	return group->child;
}


/**
 * e_group_bar_get_group_num:
 * @group_bar: an #EGroupBar.
 * @child: the child widget to find.
 * @Returns: the index of the group containing the given widget.
 *
 * Returns the index of the group containing the given child widget.
 **/
gint
e_group_bar_get_group_num		(EGroupBar	*group_bar,
					 GtkWidget	*child)
{
	EGroupBarChild *group;
	gint group_num;

	g_return_val_if_fail (E_IS_GROUP_BAR (group_bar), -1);
	g_return_val_if_fail (child != NULL, -1);

	for (group_num = 0;
	     group_num < group_bar->children->len;
	     group_num++) {
		group = &g_array_index (group_bar->children,
					EGroupBarChild, group_num);

		if (group->child == child)
			return group_num;
	}

	return -1;
}


/**
 * e_group_bar_set_group_button_label:
 * @group_bar: an #EGroupBar.
 * @group_num: the index of the group.
 * @label: the label widget to place in the group's button.
 *
 * Sets the label widget for the given group's button, replacing any existing
 * widget in the button.
 **/
void
e_group_bar_set_group_button_label	(EGroupBar	*group_bar,
					 gint		 group_num,
					 GtkWidget	*label)
{
	EGroupBarChild *group;
	GtkWidget *button_child;

	g_return_if_fail (E_IS_GROUP_BAR (group_bar));
	g_return_if_fail (group_num >= 0);
	g_return_if_fail (group_num < group_bar->children->len);

	group = &g_array_index (group_bar->children,
				EGroupBarChild, group_num);

	button_child = GTK_BIN (group->button)->child;
	if (button_child) {
		gtk_container_remove (GTK_CONTAINER (group->button),
				      button_child);
	}

	if (label)
		gtk_container_add (GTK_CONTAINER (group->button), label);
}


/*
 * Getting & setting the EGroupBar options.
 */

/**
 * e_group_bar_get_buttons_homogeneous:
 * @group_bar: an #EGroupBar.
 * @Returns: TRUE if the buttons are homoegeneous.
 *
 * Returns TRUE if the buttons are homogeneous (i.e. all have the same height).
 **/
gboolean
e_group_bar_get_buttons_homogeneous (EGroupBar	*group_bar)
{
	g_return_val_if_fail (E_IS_GROUP_BAR (group_bar), TRUE);

	return group_bar->buttons_homogeneous;
}


/**
 * e_group_bar_set_buttons_homogeneous:
 * @group_bar: an #EGroupBar.
 * @homogeneous: TRUE if the buttons should be homoegeneous.
 *
 * Specifies whether the buttons should be homogeneous. When set to TRUE all
 * the group buttons will be set to the same height (equal to the largest
 * requested height). When set to FALSE the buttons will use their own
 * individual requested heights.
 **/
void
e_group_bar_set_buttons_homogeneous	(EGroupBar	*group_bar,
					 gboolean	 homogeneous)
{
	g_return_if_fail (E_IS_GROUP_BAR (group_bar));

	/* Just return if the setting hasn't changed. */
	if (group_bar->buttons_homogeneous == homogeneous)
		return;

	group_bar->buttons_homogeneous = homogeneous;

	/* Update the position & sizes of the buttons. */
	gtk_widget_queue_resize (GTK_WIDGET (group_bar));
}


static void
e_group_bar_on_button_clicked (GtkWidget *group_button,
			       EGroupBar *group_bar)
{
	gint group_num;

	/* Determine which group button was clicked. */
	group_num = e_group_bar_find_button (group_bar, group_button);

	if (group_num != -1)
		e_group_bar_start_animation (group_bar, group_num);
}


/* This returns the group containing the given button, or -1 if not found. */
static gint
e_group_bar_find_button (EGroupBar *group_bar,
			 GtkWidget *group_button)
{
	EGroupBarChild *group;
	gint group_num;

	/* Determine which group button was clicked. */
	for (group_num = 0;
	     group_num < group_bar->children->len;
	     group_num++) {
		group = &g_array_index (group_bar->children,
					EGroupBarChild, group_num);
		if (group->button == group_button)
			return group_num;
	}

	return -1;
}


static void
e_group_bar_start_animation (EGroupBar *group_bar,
			     gint group_num)
{
	EGroupBarChild *group, *old_group;
	gint old_group_num, step;

	old_group_num = group_bar->current_group_num;

	/* Return if it is already the current group. */
	if (old_group_num == group_num)
		return;

	group_bar->current_group_num = group_num;

	/* Calculate the target y position of the new current group button
	   and child, and map the child. */
	group = &g_array_index (group_bar->children,
				EGroupBarChild, group_num);
	group->button_window_target_y = e_group_bar_get_group_button_position (group_bar, group_num);
	group->button_window_in_animation = TRUE;

	group->child_window_target_y = e_group_bar_get_group_child_position (group_bar, group_num);
	group->child_window_in_animation = TRUE;

	/* Calculate the target y position of the current group button and
	   child. */
	old_group = &g_array_index (group_bar->children,
				    EGroupBarChild, old_group_num);
	old_group->button_window_target_y = e_group_bar_get_group_button_position (group_bar, old_group_num);
	old_group->button_window_in_animation = TRUE;

	old_group->child_window_target_y = e_group_bar_get_group_child_position (group_bar, old_group_num);
	old_group->child_window_in_animation = TRUE;

	/* We also need to animate the buttons in between the old group and the
	   new group. */
	step = (old_group_num < group_num) ? 1 : -1;
	old_group_num += step;
	while (old_group_num != group_num) {
		old_group = &g_array_index (group_bar->children,
					    EGroupBarChild, old_group_num);
		old_group->button_window_target_y = e_group_bar_get_group_button_position (group_bar, old_group_num);
		old_group->button_window_in_animation = TRUE;

		old_group->child_window_target_y = e_group_bar_get_group_child_position (group_bar, old_group_num);
		old_group->child_window_in_animation = TRUE;

		old_group_num += step;
	}

	/* Add a timeout handler if we haven't already got one. */
	if (group_bar->animation_timeout_id == 0) {
		group_bar->animation_timeout_id = g_timeout_add (E_GROUP_BAR_SCROLL_TIMEOUT, e_group_bar_timeout_handler, group_bar);
	}
}


static gboolean
e_group_bar_timeout_handler (gpointer data)
{
	EGroupBar *group_bar;
	EGroupBarChild *group;
	gint group_num, button_window_y, child_window_y;
	gboolean finished = TRUE;

	g_return_val_if_fail (E_IS_GROUP_BAR (data), FALSE);

	group_bar = E_GROUP_BAR (data);

	GDK_THREADS_ENTER ();

	for (group_num = 0;
	     group_num < group_bar->children->len;
	     group_num++) {
		group = &g_array_index (group_bar->children,
					EGroupBarChild, group_num);

		if (group->button_window_in_animation) {
			gdk_window_get_position (group->button_window, NULL,
						 &button_window_y);
			button_window_y += e_group_bar_get_increment (group_bar, button_window_y, group->button_window_target_y);
			if (button_window_y == group->button_window_target_y)
				group->button_window_in_animation = FALSE;
			else
				finished = FALSE;
			gdk_window_move (group->button_window,
					 0, button_window_y);
		}
		if (group->child_window_in_animation) {
			gdk_window_get_position (group->child_window, NULL,
						 &child_window_y);
			child_window_y += e_group_bar_get_increment (group_bar, child_window_y, group->child_window_target_y);
			if (child_window_y == group->child_window_target_y)
				group->child_window_in_animation = FALSE;
			else
				finished = FALSE;
			gdk_window_move (group->child_window,
					 0, child_window_y);
		}

	}

	if (finished)
		group_bar->animation_timeout_id = 0;

	GDK_THREADS_LEAVE ();

	return !finished;
}


static gint
e_group_bar_get_increment (EGroupBar *group_bar,
			   gint window_y,
			   gint window_target_y)
{
	gdouble percentage;
	gint distance, total_distance, step;

	total_distance = group_bar->child_height;
	distance = MIN (abs (window_target_y - window_y), total_distance);

	if (distance == 0)
		return 0;

	/* Convert the distance into an angle between -PI/2 and PI/2, so we can
	   then do a cosine of it. */
	percentage = cos (M_PI * ((gdouble)distance / (gdouble)total_distance) - M_PI / 2);

	/* Now multiply by our maximum step size to get the step size. */
	step = percentage * total_distance / 6;

	/* Add it to the minimum step size, but don't go too far. */
	step = step + E_GROUP_BAR_MIN_STEP_SIZE;
	step = MIN (step, distance);

	if (window_target_y > window_y)
		return step;
	else
		return -step;
}


static gboolean
e_group_bar_on_button_drag_motion (GtkWidget	      *widget,
				   GdkDragContext     *context,
				   gint                x,
				   gint                y,
				   guint               time,
				   EGroupBar	      *group_bar)
{
	gint group_num;

	if (!group_bar->auto_show_timeout_id) {
		group_num = e_group_bar_find_button (group_bar, widget);
		if (group_num != -1) {
			group_bar->auto_show_timeout_id = gtk_timeout_add (E_GROUP_BAR_AUTO_SHOW_TIMEOUT, e_group_bar_auto_show, group_bar);
			group_bar->auto_show_group_num = group_num;
		}
	}

	return FALSE;
}


static void
e_group_bar_on_button_drag_leave (GtkWidget	     *widget,
				  GdkDragContext     *context,
				  guint               time,
				  EGroupBar	      *group_bar)
{
	if (group_bar->auto_show_timeout_id) {
		gtk_timeout_remove (group_bar->auto_show_timeout_id);
		group_bar->auto_show_timeout_id = 0;
	}
}


static gboolean
e_group_bar_auto_show (gpointer data)
{
	EGroupBar *group_bar;

	g_return_val_if_fail (E_IS_GROUP_BAR (data), FALSE);

	group_bar = E_GROUP_BAR (data);

	GDK_THREADS_ENTER ();

	e_group_bar_start_animation (group_bar,
				     group_bar->auto_show_group_num);

	group_bar->auto_show_timeout_id = 0;

	GDK_THREADS_LEAVE ();

	return FALSE;
}


/* This removes all timeouts and sets all 'in_animation' flags to FALSE. */
static void
e_group_bar_stop_all_animation	(EGroupBar	*group_bar)
{
	EGroupBarChild *group;
	gint group_num;

	if (group_bar->animation_timeout_id) {
		g_source_remove (group_bar->animation_timeout_id);
		group_bar->animation_timeout_id = 0;
	}
	if (group_bar->auto_show_timeout_id) {
		g_source_remove (group_bar->auto_show_timeout_id);
		group_bar->auto_show_timeout_id = 0;
	}

	if (group_bar->children != NULL) {
		for (group_num = 0;
		     group_num < group_bar->children->len;
		     group_num++) {
			group = &g_array_index (group_bar->children,
						EGroupBarChild, group_num);
			group->button_window_in_animation = FALSE;
			group->child_window_in_animation = FALSE;
		}
	}
}


/*
 * Get the group button.
 */
GtkWidget*
e_group_bar_get_group_button		(EGroupBar	*group_bar,
					 gint		 group_num)
{
	EGroupBarChild *group;

	g_return_val_if_fail (E_IS_GROUP_BAR (group_bar), NULL);
	g_return_val_if_fail (group_num >= 0, NULL);
	g_return_val_if_fail (group_num < group_bar->children->len, NULL);

	group = &g_array_index (group_bar->children, EGroupBarChild,
				group_num);

	return group->button;
}
