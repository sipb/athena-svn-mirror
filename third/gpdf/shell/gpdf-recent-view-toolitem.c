/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Toolitem (button with dropdown menu) for recent files
 *
 * Copyright (C) 2002, 2003 Authors
 *
 * Authors:
 *   James Willcox <jwillcox@gnome.org> (code from gedit)
 *   Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
 *
 * GPdf is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPdf is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include <aconf.h>
#include "gpdf-recent-view-toolitem.h"

#include <libgnome/gnome-macros.h>
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <libgnomeui/gnome-popup-menu.h>
#include <recent-files/egg-recent-model.h>
#include <recent-files/egg-recent-view.h>
#include <recent-files/egg-recent-view-gtk.h>

#define PARENT_TYPE GTK_TYPE_TOGGLE_BUTTON
GNOME_CLASS_BOILERPLATE (GPdfRecentViewToolitem, gpdf_recent_view_toolitem,
                         GtkToggleButton, PARENT_TYPE)

struct _GPdfRecentViewToolitemPrivate {
	GtkTooltips *tooltips;
	GtkWidget *menu;
	EggRecentViewGtk *recent_view;
};

enum {
	ITEM_ACTIVATE_SIGNAL,
	LAST_SIGNAL
};

static guint gpdf_recent_view_toolitem_signals [LAST_SIGNAL];

static void
activate_cb (EggRecentViewGtk *view, EggRecentItem *item, gpointer user_data)
{
	g_return_if_fail (GPDF_IS_RECENT_VIEW_TOOLITEM (user_data));
	
	egg_recent_item_ref (item);
	g_signal_emit (
		G_OBJECT (user_data),
		gpdf_recent_view_toolitem_signals [ITEM_ACTIVATE_SIGNAL],
		0, item);
	egg_recent_item_unref (item);
}

static void
menu_position_under_widget (GtkMenu *menu, int *x, int *y,
			    gboolean *push_in, gpointer user_data)
{
	GtkWidget *w;
	int screen_width, screen_height;
	GtkRequisition requisition;

	w = GTK_WIDGET (user_data);
	
	gdk_window_get_origin (w->window, x, y);
	*x += w->allocation.x;
	*y += w->allocation.y + w->allocation.height;

	gtk_widget_size_request (GTK_WIDGET (menu), &requisition);

	screen_width = gdk_screen_width ();
	screen_height = gdk_screen_height ();

	*x = CLAMP (*x, 0, MAX (0, screen_width - requisition.width));
	*y = CLAMP (*y, 0, MAX (0, screen_height - requisition.height));
}

static void
gpdf_recent_view_toolitem_popup (GPdfRecentViewToolitem *toolitem,
				 guint button, guint32 activate_time)
{
	GtkWidget *widget;
	GtkMenu *menu;

	g_return_if_fail (GPDF_IS_RECENT_VIEW_TOOLITEM (toolitem));

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toolitem), TRUE);
	widget = GTK_WIDGET (toolitem);
	menu = GTK_MENU (toolitem->priv->menu);

	gtk_menu_popup (menu, NULL, NULL, menu_position_under_widget, widget,
			button, activate_time);

	
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toolitem), FALSE);
}

static gboolean
gpdf_recent_view_toolitem_button_press_event (GtkWidget *widget,
					      GdkEventButton *event)
{
	gpdf_recent_view_toolitem_popup (GPDF_RECENT_VIEW_TOOLITEM (widget),
					 event->button, event->time);
	
	return TRUE;
}

static gboolean
gpdf_recent_view_toolitem_key_press_event (GtkWidget *widget,
					   GdkEventKey *event)
{
	if (event->keyval == GDK_space ||
	    event->keyval == GDK_KP_Space ||
	    event->keyval == GDK_Return ||
	    event->keyval == GDK_KP_Enter) {
		gpdf_recent_view_toolitem_popup (
			GPDF_RECENT_VIEW_TOOLITEM (widget), 0, event->time);
	}

	return FALSE;
}

void
gpdf_recent_view_toolitem_set_model (GPdfRecentViewToolitem *toolitem,
				     EggRecentModel *model)
{
	GtkWidget *menu;
	EggRecentViewGtk *view;

	menu = toolitem->priv->menu;
	view = toolitem->priv->recent_view;
	egg_recent_view_set_model (EGG_RECENT_VIEW (view), model);
}

static void
gpdf_recent_view_toolitem_dispose (GObject *object)
{
	GPdfRecentViewToolitem *toolitem = GPDF_RECENT_VIEW_TOOLITEM (object);

	if (toolitem->priv->tooltips) {
		g_object_unref (toolitem->priv->tooltips);
		toolitem->priv->tooltips = NULL;
	}

	if (toolitem->priv->menu) {
		g_object_unref (toolitem->priv->menu);
		toolitem->priv->menu = NULL;
	}

	if (toolitem->priv->recent_view) {
		g_object_unref (toolitem->priv->recent_view);
		toolitem->priv->recent_view = NULL;
	}

	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
gpdf_recent_view_toolitem_finalize (GObject *object)
{
	GPdfRecentViewToolitem *toolitem = GPDF_RECENT_VIEW_TOOLITEM (object);

	if (toolitem->priv) {
		g_free (toolitem->priv);
		toolitem->priv = NULL;
	}

	GNOME_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

static void
gpdf_recent_view_toolitem_instance_init (GPdfRecentViewToolitem *toolitem)
{
	GPdfRecentViewToolitemPrivate *priv;
	AtkObject *atk_obj;

	GTK_WIDGET_UNSET_FLAGS (toolitem, GTK_CAN_FOCUS);  

	toolitem->priv = priv = g_new0 (GPdfRecentViewToolitemPrivate, 1);

	priv->tooltips = gtk_tooltips_new ();
	gtk_tooltips_set_tip (priv->tooltips, GTK_WIDGET (toolitem),
			      _("Open a recently used file"), NULL);
	g_object_ref (priv->tooltips);
	gtk_object_sink (GTK_OBJECT (priv->tooltips));
	atk_obj = gtk_widget_get_accessible (GTK_WIDGET (toolitem));
	atk_object_set_name (atk_obj, _("Recent"));

	gtk_button_set_relief (GTK_BUTTON (toolitem), GTK_RELIEF_NONE);
	gtk_container_add (GTK_CONTAINER (toolitem),
			   gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_OUT));

	priv->menu = gtk_menu_new ();
	g_object_ref (priv->menu);
	gtk_object_sink (GTK_OBJECT (priv->menu));
	gtk_widget_show (priv->menu);

	priv->recent_view = egg_recent_view_gtk_new (priv->menu, NULL);
	egg_recent_view_gtk_show_icons (priv->recent_view, TRUE);
	egg_recent_view_gtk_show_numbers (priv->recent_view, FALSE);
	g_signal_connect_object (G_OBJECT (priv->recent_view), "activate",
				 G_CALLBACK (activate_cb), toolitem, 0);
}

static void
gpdf_recent_view_toolitem_class_init (GPdfRecentViewToolitemClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = gpdf_recent_view_toolitem_dispose;
	G_OBJECT_CLASS (klass)->finalize = gpdf_recent_view_toolitem_finalize;

	GTK_WIDGET_CLASS (klass)->key_press_event =
		gpdf_recent_view_toolitem_key_press_event;
	GTK_WIDGET_CLASS (klass)->button_press_event =
		gpdf_recent_view_toolitem_button_press_event;

	gpdf_recent_view_toolitem_signals [ITEM_ACTIVATE_SIGNAL] = g_signal_new (
		"item_activate",
		G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (GPdfRecentViewToolitemClass, item_activate),
		NULL, NULL,
		g_cclosure_marshal_VOID__BOXED,
		G_TYPE_NONE, 1,
		EGG_TYPE_RECENT_ITEM);

	klass->item_activate = NULL;
}
