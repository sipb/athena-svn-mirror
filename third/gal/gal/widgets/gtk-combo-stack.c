/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gtk-combo-stack.c - A combo box for displaying stacks (useful for Undo lists)
 *
 * Copyright (C) 2000 ÉRDI Gergõ <cactus@cactus.rulez.org>
 *
 * Authors:
 *   ÉRDI Gergõ <cactus@cactus.rulez.org>
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

#include <libgnomeui/gnome-preferences.h>
#include <libgnomeui/gnome-stock.h>
#include "gtk-combo-stack.h"
#include "gal/util/e-util.h"

static GtkObjectClass *gtk_combo_stack_parent_class;

struct _GtkComboStackPrivate {
	GtkWidget *button;
	GtkWidget *list;
	GtkWidget *scrolled_window;

	gint num_items;
	gint curr_item;
};

enum {
	POP,
	LAST_SIGNAL
};
static gint gtk_combo_stack_signals [LAST_SIGNAL] = { 0, };

static void
gtk_combo_stack_finalize (GtkObject *object)
{
	GtkComboStack *combo = GTK_COMBO_STACK (object);
	
	g_free (combo->priv);
	
	GTK_OBJECT_CLASS (gtk_combo_stack_parent_class)->finalize (object);
}


static void
gtk_combo_stack_class_init (GtkObjectClass *object_class)
{
	object_class->finalize = &gtk_combo_stack_finalize;
	gtk_combo_stack_parent_class = gtk_type_class (gtk_combo_box_get_type ());
	
	gtk_combo_stack_signals [POP] = gtk_signal_new (
		"pop",
		GTK_RUN_LAST,
		E_OBJECT_CLASS_TYPE (object_class),
		GTK_SIGNAL_OFFSET (GtkComboBoxClass, pop_down_done),
		gtk_marshal_NONE__INT,
		GTK_TYPE_NONE, 1, GTK_TYPE_INT);
	
	E_OBJECT_CLASS_ADD_SIGNALS (object_class, gtk_combo_stack_signals, LAST_SIGNAL);
}

static void
gtk_combo_stack_init (GtkComboStack *object)
{
	object->priv = g_new0 (GtkComboStackPrivate, 1);
	object->priv->num_items = 0;
}

GtkType
gtk_combo_stack_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"GtkComboStack",
			sizeof (GtkComboStack),
			sizeof (GtkComboStackClass),
			(GtkClassInitFunc) gtk_combo_stack_class_init,
			(GtkObjectInitFunc) gtk_combo_stack_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_combo_box_get_type (), &info);
	}

	return type;
}

static void
gtk_combo_stack_clear_selection (GtkComboStack *combo)
{
	GList *children = gtk_container_children (GTK_CONTAINER (combo->priv->list));
	GList *curr_item;

	for (curr_item = children; curr_item; curr_item = curr_item->next) {
		gtk_widget_set_state (GTK_WIDGET (curr_item->data),
				      GTK_STATE_NORMAL);
	}
	g_list_free (children);
}

static void
button_cb (GtkWidget *button, gpointer data)
{
	GtkComboStack *combo = GTK_COMBO_STACK (data);

	gtk_combo_stack_pop (combo, 1);
}

static void
button_release_cb (GtkWidget *list, GdkEventButton *e, gpointer data)
{
	GtkComboStack *combo = GTK_COMBO_STACK (data);
	gint tmp, width, height;
	gdk_window_get_geometry (e->window, &tmp, &tmp, &width, &height, &tmp);
	
	if (e->x > width || e->y > height ||
	    e->x < 0 || e->y < 0)
		e->window = NULL;

	gtk_combo_stack_clear_selection (combo);
	gtk_combo_box_popup_hide (GTK_COMBO_BOX (combo));
	
	if (e->window &&
	    gdk_window_get_toplevel (e->window) ==
	    gdk_window_get_toplevel (list->window))
		if (combo->priv->curr_item != -1)
			gtk_combo_stack_pop (combo, combo->priv->curr_item);
}

static void
list_select_cb (GtkWidget *list, GtkWidget *child, gpointer data)
{
	GtkComboStack *combo = GTK_COMBO_STACK (data);
	gint index = combo->priv->num_items -
		GPOINTER_TO_INT (gtk_object_get_data
				 (GTK_OBJECT (child), "value")) + 1;
	guint i = 0;
	GList* items = gtk_container_children (GTK_CONTAINER (list));
	GList* curr_item;
	
	/* Clear selection */
	gtk_combo_stack_clear_selection (combo);

	/* Set selection state for every item we're about to pop */
	curr_item = items;
	for (i = 0; i < index && curr_item != NULL; i++)
	{
		gtk_widget_set_state (GTK_WIDGET (curr_item->data),
				      GTK_STATE_SELECTED);
		curr_item = g_list_next (curr_item);
	}

	g_list_free (items);

	/* Store selection data */
	combo->priv->curr_item = index;
}

static void
gtk_combo_stack_construct (GtkComboStack *combo,
			   const gchar *stock_name,
			   gboolean const is_scrolled)
{
	GtkWidget *button, *list, *scroll, *display_widget, *pixmap;
	
	button = combo->priv->button = gtk_button_new ();
	if (!gnome_preferences_get_toolbar_relief_btn ())
		gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
	GTK_WIDGET_UNSET_FLAGS (button, GTK_CAN_FOCUS);

	list = combo->priv->list = gtk_list_new ();

	/* Create the button */
	pixmap = gnome_stock_new_with_icon (stock_name);
	gtk_widget_show (pixmap);
	gtk_container_add (GTK_CONTAINER (button), pixmap);
	
	if (is_scrolled) {
		display_widget = scroll = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scroll),
						GTK_POLICY_NEVER,
						GTK_POLICY_AUTOMATIC);

		gtk_scrolled_window_add_with_viewport (
			GTK_SCROLLED_WINDOW(scroll), list);
		gtk_container_set_focus_hadjustment (
			GTK_CONTAINER (list),
			gtk_scrolled_window_get_hadjustment (
				GTK_SCROLLED_WINDOW (scroll)));
		gtk_container_set_focus_vadjustment (
			GTK_CONTAINER (list),
			gtk_scrolled_window_get_vadjustment (
				GTK_SCROLLED_WINDOW (scroll)));
		gtk_widget_set_usize (scroll, 0, 200); /* MAGIC NUMBER */
	} else
		display_widget = list;

	/* Set up the dropdown list */
	gtk_list_set_selection_mode (GTK_LIST (list), GTK_SELECTION_BROWSE);
	gtk_signal_connect (GTK_OBJECT (list), "select-child",
			    GTK_SIGNAL_FUNC (list_select_cb),
			    (gpointer) combo);
	gtk_signal_connect (GTK_OBJECT (list), "button_release_event",
			    GTK_SIGNAL_FUNC (button_release_cb),
			    (gpointer) combo);
	
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
			    GTK_SIGNAL_FUNC (button_cb),
			    (gpointer) combo);

	gtk_widget_show (display_widget);
	gtk_widget_show (button);
	gtk_combo_box_construct (GTK_COMBO_BOX (combo), button, display_widget);
	gtk_widget_set_sensitive (GTK_WIDGET (combo), FALSE);
}

GtkWidget*
gtk_combo_stack_new (const gchar *stock,
		     gboolean const is_scrolled)
{
	GtkComboStack *combo;

	combo = gtk_type_new (gtk_combo_stack_get_type ());
	gtk_combo_stack_construct (combo, stock, is_scrolled);

	return GTK_WIDGET (combo);
}

void
gtk_combo_stack_push_item (GtkComboStack *combo,
			   const gchar *item)
{
	GtkWidget *listitem;
	GList *tmp_list; /* We can only prepend GLists to a GtkList */
	
	g_return_if_fail (item != NULL);

	combo->priv->num_items++;

	listitem = gtk_list_item_new_with_label (item);
	gtk_object_set_data (GTK_OBJECT (listitem), "value",
			     GINT_TO_POINTER (combo->priv->num_items));
	gtk_widget_show (listitem);

	tmp_list = g_list_alloc ();
	tmp_list->data = listitem;
	tmp_list->next = NULL;
	gtk_list_prepend_items (GTK_LIST (combo->priv->list),
				tmp_list);
/*	gtk_list_unselect_all (GTK_LIST (combo->priv->list)); */
	gtk_combo_stack_clear_selection (combo);

	gtk_widget_set_sensitive (GTK_WIDGET (combo), TRUE);
}

void
gtk_combo_stack_pop (GtkComboStack *combo,
		     gint num)
{
	gtk_signal_emit_by_name (GTK_OBJECT (combo), "pop", num);
}

void
gtk_combo_stack_remove_top (GtkComboStack *combo,
			    gint num)
{
	gint i;
	GList *child, *children;
	GtkWidget *list = combo->priv->list;
	
	g_return_if_fail (combo->priv->num_items != 0);

	if (num > combo->priv->num_items)
		num = combo->priv->num_items;
	
	children = child = gtk_container_children (GTK_CONTAINER (list));
	for (i = 0; i < num; i++)
	{
		gtk_container_remove (GTK_CONTAINER (list), child->data);
		child = g_list_next (child);
	}
	g_list_free (children);

	gtk_combo_stack_clear_selection (combo);
	
	combo->priv->num_items -= num;
	combo->priv->curr_item = -1;
	if (!combo->priv->num_items)
		gtk_widget_set_sensitive (GTK_WIDGET (combo), FALSE);
}

void
gtk_combo_stack_clear (GtkComboStack *combo)
{
	combo->priv->num_items = 0;

	gtk_list_clear_items (GTK_LIST (combo->priv->list), 0, -1);
	gtk_widget_set_sensitive (GTK_WIDGET (combo), FALSE);
}

/*
 * Make sure stack is not deeper than @n elements.
 *
 * (Think undo/redo where we don't want to use too much memory.)
 */
void
gtk_combo_stack_truncate (GtkComboStack *combo, int n)
{
	if (combo->priv->num_items > n) {
		combo->priv->num_items = n;

		gtk_list_clear_items (GTK_LIST (combo->priv->list), n, -1);
		if (n == 0)
			gtk_widget_set_sensitive (GTK_WIDGET (combo), FALSE);
	}
}
