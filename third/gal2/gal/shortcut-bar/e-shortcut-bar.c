/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * e-shortcut-bar.c
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
 * EShortcutBar displays a vertical bar with a number of Groups, each of which
 * contains any number of icons. It is used on the left of the main application
 * window so users can easily access items such as folders and files.
 *
 * The architecture is a bit complicated. EShortcutBar is a sublass of
 * EGroupBar (which supports a number of groups with buttons to slide them
 * into view). EShortcutBar places an EIconBar widget in each group page,
 * which displays an icon and name for each shortcut.
 */

#include <config.h>
#include <string.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkdnd.h>
#include <gtk/gtkrc.h>
#include <gtk/gtkselection.h>
#include <gtk/gtksignal.h>
#include <libgnome/gnome-util.h>
#include <gal/util/e-util.h>
#include <gal/e-text/e-entry.h>
#include "e-shortcut-bar.h"
#include "e-vscrolled-bar.h"

/* Drag and Drop stuff. */
enum {
	TARGET_SHORTCUT
};
static GtkTargetEntry target_table[] = {
	{ "E-SHORTCUT",     0, TARGET_SHORTCUT }
};
static guint n_targets = sizeof(target_table) / sizeof(target_table[0]);

gboolean   e_shortcut_bar_default_icon_loaded   = FALSE;
GdkPixbuf *e_shortcut_bar_default_icon		= NULL;
gchar	  *e_shortcut_bar_default_icon_filename = "gnome-folder.png";

static void e_shortcut_bar_class_init		(EShortcutBarClass *class);
static void e_shortcut_bar_init			(EShortcutBar	*shortcut_bar);
static void e_shortcut_bar_dispose		(GObject	*object);
static void e_shortcut_bar_finalize		(GObject	*object);

static void e_shortcut_bar_style_set (GtkWidget *widget, GtkStyle *prev_style);

static void e_shortcut_bar_disconnect_model	(EShortcutBar	*shortcut_bar,
						 gboolean        model_destroyed);

static void e_shortcut_bar_on_model_destroyed	(void           *data,
						 GObject        *where_the_model_was);
static void e_shortcut_bar_on_group_added	(EShortcutModel	*model,
						 gint		 group_num,
						 gchar		*group_name,
						 EShortcutBar	*shortcut_bar);
static void e_shortcut_bar_on_group_removed	(EShortcutModel	*model,
						 gint		 group_num,
						 EShortcutBar	*shortcut_bar);
static void e_shortcut_bar_on_item_added	(EShortcutModel *model,
						 gint		 group_num,
						 gint		 item_num,
						 gchar		*item_url,
						 gchar		*item_name,
						 GdkPixbuf      *item_image,
						 EShortcutBar	*shortcut_bar);
static void e_shortcut_bar_on_item_removed	(EShortcutModel *model,
						 gint		 group_num,
						 gint		 item_num,
						 EShortcutBar	*shortcut_bar);
static void e_shortcut_bar_on_item_updated	(EShortcutModel *model,
						 gint		 group_num,
						 gint		 item_num,
						 gchar          *item_url,
						 gchar          *item_name,
						 GdkPixbuf      *item_image,
						 EShortcutBar	*shortcut_bar);

static gint e_shortcut_bar_add_group		(EShortcutBar	*shortcut_bar,
						 gint		 position,
						 const gchar	*group_name);
static void e_shortcut_bar_remove_group		(EShortcutBar	*shortcut_bar,
						 gint		 group_num);
static gint e_shortcut_bar_add_item		(EShortcutBar	*shortcut_bar,
						 gint		 group_num,
						 gint		 position,
						 const gchar	*item_url,
						 const gchar	*item_name,
						 GdkPixbuf      *image);
static void e_shortcut_bar_remove_item		(EShortcutBar	*shortcut_bar,
						 gint		 group_num,
						 gint		 item_num);
static void e_shortcut_bar_update_item		(EShortcutBar 	*shortcut_bar,
						 gint         	 group_num,
						 gint         	 item_num,
						 const gchar  	*item_url,
						 const gchar  	*item_name,
						 GdkPixbuf    	*image);

static void e_shortcut_bar_set_canvas_style	(EShortcutBar	*shortcut_bar,
						 GtkWidget	*canvas);
static void e_shortcut_bar_item_selected	(EIconBar	*icon_bar,
						 GdkEvent	*event,
						 gint		 item_num,
						 EShortcutBar	*shortcut_bar);
static void e_shortcut_bar_item_dragged		(EIconBar	*icon_bar,
						 GdkEvent	*event,
						 gint		 item_num,
						 EShortcutBar	*shortcut_bar);
static void e_shortcut_bar_on_drag_data_get	(GtkWidget      *widget,
						 GdkDragContext *context,
						 GtkSelectionData *selection_data,
						 guint           info,
						 guint           time,
						 EShortcutBar	*shortcut_bar);
static gboolean e_shortcut_bar_on_drag_motion 	(GtkWidget      *widget,
					      	 GdkDragContext *context,
					      	 gint            x,
					      	 gint            y,
					      	 guint           time,
					      	 EShortcutBar    *shortcut_bar);
static gboolean e_shortcut_bar_on_drag_drop     (GtkWidget	 *widget,
						 GdkDragContext *context,
						 gint            x,
						 gint            y,
						 guint           time);
static void e_shortcut_bar_on_drag_data_received(GtkWidget      *widget,
						 GdkDragContext *context,
						 gint            x,
						 gint            y,
						 GtkSelectionData *data,
						 guint           info,
						 guint           time,
						 EShortcutBar	*shortcut_bar);
static void e_shortcut_bar_on_drag_data_delete	(GtkWidget      *widget,
						 GdkDragContext *context,
						 EShortcutBar   *shortcut_bar);
static void e_shortcut_bar_on_drag_end		(GtkWidget      *widget,
						 GdkDragContext *context,
						 EShortcutBar   *shortcut_bar);
static void e_shortcut_bar_stop_editing		(GtkWidget	*button,
						 EShortcutBar	*shortcut_bar);


enum
{
  ITEM_SELECTED,
  SHORTCUT_DROPPED,
  SHORTCUT_DRAGGED,
  SHORTCUT_DRAG_MOTION,
  SHORTCUT_DRAG_DATA_RECEIVED,
  LAST_SIGNAL
};

static guint e_shortcut_bar_signals[LAST_SIGNAL] = {0};

static EGroupBarClass *parent_class;

E_MAKE_TYPE(e_shortcut_bar, "EShortcutBar", EShortcutBar,
	    e_shortcut_bar_class_init, e_shortcut_bar_init,
	    e_group_bar_get_type())

static void
e_shortcut_bar_class_init (EShortcutBarClass *class)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;

	parent_class = g_type_class_ref (E_GROUP_BAR_TYPE);

	object_class = (GObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	e_shortcut_bar_signals[ITEM_SELECTED] =
		g_signal_new ("item_selected",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (EShortcutBarClass, item_selected),
			      NULL, NULL,
			      e_marshal_NONE__BOXED_INT_INT,
			      G_TYPE_NONE, 3, GDK_TYPE_EVENT,
			      G_TYPE_INT, G_TYPE_INT);

	e_shortcut_bar_signals[SHORTCUT_DROPPED] =
		g_signal_new ("shortcut_dropped",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (EShortcutBarClass, shortcut_dropped),
			      NULL, NULL,
			      e_marshal_NONE__INT_INT_STRING_STRING,
			      G_TYPE_NONE, 4,
			      G_TYPE_INT,
			      G_TYPE_INT,
			      G_TYPE_STRING,
			      G_TYPE_STRING);

	e_shortcut_bar_signals[SHORTCUT_DRAGGED] =
		g_signal_new ("shortcut_dragged",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (EShortcutBarClass, shortcut_dragged),
			      NULL, NULL,
			      e_marshal_NONE__INT_INT,
			      G_TYPE_NONE, 2,
			      G_TYPE_INT,
			      G_TYPE_INT);

	e_shortcut_bar_signals[SHORTCUT_DRAG_MOTION] =
		g_signal_new ("shortcut_drag_motion",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (EShortcutBarClass, shortcut_drag_motion),
			      NULL, NULL,
			      e_marshal_BOOLEAN__POINTER_POINTER_INT_INT_INT,
			      G_TYPE_BOOLEAN, 5,
			      G_TYPE_POINTER,
			      G_TYPE_POINTER,
			      G_TYPE_INT,
			      G_TYPE_INT,
			      G_TYPE_INT);

	e_shortcut_bar_signals[SHORTCUT_DRAG_DATA_RECEIVED] =
		g_signal_new ("shortcut_drag_data_received",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (EShortcutBarClass, shortcut_drag_data_received),
			      NULL, NULL,
			      e_marshal_BOOLEAN__POINTER_POINTER_POINTER_INT_INT_INT,
			      G_TYPE_BOOLEAN, 6,
			      G_TYPE_POINTER,
			      G_TYPE_POINTER,
			      G_TYPE_POINTER,
			      G_TYPE_INT,
			      G_TYPE_INT,
			      G_TYPE_INT);

	/* Method override */
	object_class->dispose  = e_shortcut_bar_dispose;
	object_class->finalize = e_shortcut_bar_finalize;

	widget_class->style_set = e_shortcut_bar_style_set;
}


static void
e_shortcut_bar_init (EShortcutBar *shortcut_bar)
{
	shortcut_bar->groups = g_array_new (FALSE, FALSE,
					    sizeof (EShortcutBarGroup));

	shortcut_bar->dragged_url = NULL;
	shortcut_bar->dragged_name = NULL;

	shortcut_bar->enable_drags = TRUE;
}


GtkWidget *
e_shortcut_bar_new (void)
{
	GtkWidget *shortcut_bar;

	gtk_widget_push_colormap (gdk_rgb_get_cmap ());

	shortcut_bar = GTK_WIDGET (g_object_new (E_TYPE_SHORTCUT_BAR, NULL));

	gtk_widget_pop_colormap ();

	return shortcut_bar;
}

static void
e_shortcut_bar_dispose (GObject *object)
{
	EShortcutBar *shortcut_bar;

	shortcut_bar = E_SHORTCUT_BAR (object);

	e_shortcut_bar_disconnect_model (shortcut_bar, FALSE);

	(* G_OBJECT_CLASS (parent_class)->dispose) (object);
}

static void
e_shortcut_bar_finalize (GObject *object)
{
	EShortcutBar *shortcut_bar;

	shortcut_bar = E_SHORTCUT_BAR (object);

	if (shortcut_bar->groups)
		g_array_free (shortcut_bar->groups, TRUE);

	g_free (shortcut_bar->dragged_url);

	g_free (shortcut_bar->dragged_name);

	(* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

static void
e_shortcut_bar_style_set (GtkWidget *widget, GtkStyle *prev_style)
{
	EShortcutBar *shortcut_bar;
	EShortcutBarGroup *group;
	gint num_groups, group_num;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (E_IS_SHORTCUT_BAR (widget));

	shortcut_bar = E_SHORTCUT_BAR (widget);

	num_groups = shortcut_bar->groups->len;
	for (group_num = 0; group_num < num_groups; group_num++) {
		group = &g_array_index (shortcut_bar->groups,
					EShortcutBarGroup, group_num);
		e_shortcut_bar_set_canvas_style (shortcut_bar,
						 group->icon_bar);
	}
}

void
e_shortcut_bar_set_model (EShortcutBar *shortcut_bar,
			  EShortcutModel *model)
{
	gint num_groups, group_num, num_items, item_num;
	gchar *group_name, *item_url, *item_name;
	GdkPixbuf *item_image;

	/* Disconnect any existing model. */
	e_shortcut_bar_disconnect_model (shortcut_bar, FALSE);

	shortcut_bar->model = model;

	if (!model)
		return;

	g_object_weak_ref (G_OBJECT (model), e_shortcut_bar_on_model_destroyed, shortcut_bar);

	g_signal_connect (model, "group_added",
			  G_CALLBACK (e_shortcut_bar_on_group_added),
			  shortcut_bar);
	g_signal_connect (model, "group_removed",
			  G_CALLBACK (e_shortcut_bar_on_group_removed),
			  shortcut_bar);
	g_signal_connect (model, "item_added",
			  G_CALLBACK (e_shortcut_bar_on_item_added),
			  shortcut_bar);
	g_signal_connect (model, "item_removed",
			  G_CALLBACK (e_shortcut_bar_on_item_removed),
			  shortcut_bar);
	g_signal_connect (model, "item_updated",
			  G_CALLBACK (e_shortcut_bar_on_item_updated),
			  shortcut_bar);

	/* Add any items already in the model. */
	num_groups = e_shortcut_model_get_num_groups (model);
	for (group_num = 0; group_num < num_groups; group_num++) {
		group_name = e_shortcut_model_get_group_name (model,
							      group_num);
		e_shortcut_bar_add_group (shortcut_bar, group_num, group_name);
		g_free (group_name);

		num_items = e_shortcut_model_get_num_items (model, group_num);
		for (item_num = 0; item_num < num_items; item_num++) {
			e_shortcut_model_get_item_info (model, group_num,
							item_num, &item_url,
							&item_name, &item_image);
			e_shortcut_bar_add_item (shortcut_bar, group_num,
						 item_num, item_url,
						 item_name, item_image);
			g_free (item_url);
			g_free (item_name);
			if (item_image != NULL)
				gdk_pixbuf_unref (item_image);
		}
	}
}


static void
e_shortcut_bar_disconnect_model (EShortcutBar *shortcut_bar,
				 gboolean model_destroyed)
{
	/* Remove all the current groups. */
	while (shortcut_bar->groups->len)
		e_shortcut_bar_remove_group (shortcut_bar, 0);

	if (! model_destroyed && shortcut_bar->model != NULL) {
		/* Disconnect all the signals in one go. */
		g_signal_handlers_disconnect_matched (shortcut_bar->model,
						      G_SIGNAL_MATCH_DATA,
						      0, 0, NULL, NULL,
						      shortcut_bar);

		g_object_weak_unref (G_OBJECT (shortcut_bar->model),
				     e_shortcut_bar_on_model_destroyed, shortcut_bar);
	}

	shortcut_bar->model = NULL;
}


static void
e_shortcut_bar_on_model_destroyed	(void *data,
					 GObject *where_the_model_was)
{
	EShortcutBar *shortcut_bar = E_SHORTCUT_BAR (data);

	e_shortcut_bar_disconnect_model (shortcut_bar, TRUE);
}


static void
e_shortcut_bar_on_group_added		(EShortcutModel *model,
					 gint		 group_num,
					 gchar		*group_name,
					 EShortcutBar	*shortcut_bar)
{
	e_shortcut_bar_add_group (shortcut_bar, group_num, group_name);
}


static void
e_shortcut_bar_on_group_removed		(EShortcutModel *model,
					 gint		 group_num,
					 EShortcutBar	*shortcut_bar)
{
	e_shortcut_bar_remove_group (shortcut_bar, group_num);
}


static void
e_shortcut_bar_on_item_added		(EShortcutModel *model,
					 gint		 group_num,
					 gint		 item_num,
					 gchar		*item_url,
					 gchar		*item_name,
					 GdkPixbuf      *item_image,
					 EShortcutBar	*shortcut_bar)
{
	e_shortcut_bar_add_item (shortcut_bar, group_num, item_num,
				 item_url, item_name, item_image);
}


static void
e_shortcut_bar_on_item_removed (EShortcutModel *model,
				gint group_num,
				gint item_num,
				EShortcutBar *shortcut_bar)
{
	e_shortcut_bar_remove_item (shortcut_bar, group_num, item_num);
}

static void
e_shortcut_bar_on_item_updated (EShortcutModel *model,
				gint            group_num,
				gint            item_num,
				gchar          *item_url,
				gchar          *item_name,
				GdkPixbuf      *image,
				EShortcutBar   *shortcut_bar)
{
	e_shortcut_bar_update_item (shortcut_bar, group_num, item_num,
				    item_url, item_name, image);
}

static gboolean
e_shortcut_bar_group_button_press (GtkWidget *widget,
				   GdkEventButton *button_event,
				   EShortcutBar *shortcut_bar)
{
	EGroupBar *group_bar;
	int i;

	group_bar = E_GROUP_BAR (shortcut_bar);

	for (i = 0; i < group_bar->children->len; i++) {
		EGroupBarChild *child;

		child = &g_array_index (group_bar->children, EGroupBarChild, i);
		if (widget == child->button) {
			g_signal_emit (shortcut_bar,
				       e_shortcut_bar_signals[ITEM_SELECTED], 0,
				       button_event, i, -1);

			break;
		}
	}

	return FALSE;
}

static gint
e_shortcut_bar_add_group	(EShortcutBar	*shortcut_bar,
				 gint		 position,
				 const gchar	*group_name)
{
	EShortcutBarGroup *group, tmp_group;
	gint group_num;
	GtkWidget *button, *label;

	g_return_val_if_fail (E_IS_SHORTCUT_BAR (shortcut_bar), -1);
	g_return_val_if_fail (group_name != NULL, -1);

	gtk_widget_push_colormap (gdk_rgb_get_cmap ());

	group_num = position;
	g_array_insert_val (shortcut_bar->groups, group_num, tmp_group);

	group = &g_array_index (shortcut_bar->groups,
				EShortcutBarGroup, group_num);

	group->vscrolled_bar = e_vscrolled_bar_new (NULL);
	gtk_widget_show (group->vscrolled_bar);
	g_signal_connect (E_VSCROLLED_BAR (group->vscrolled_bar)->up_button, "pressed",
			  G_CALLBACK (e_shortcut_bar_stop_editing), shortcut_bar);
	g_signal_connect (E_VSCROLLED_BAR (group->vscrolled_bar)->down_button, "pressed",
			  G_CALLBACK (e_shortcut_bar_stop_editing), shortcut_bar);

	group->icon_bar = e_icon_bar_new ();
	e_icon_bar_set_enable_drags (E_ICON_BAR (group->icon_bar),
				     shortcut_bar->enable_drags);
	gtk_widget_show (group->icon_bar);
	gtk_container_add (GTK_CONTAINER (group->vscrolled_bar),
			   group->icon_bar);
	g_signal_connect (group->icon_bar, "item_selected",
			  G_CALLBACK (e_shortcut_bar_item_selected),
			  shortcut_bar);
	g_signal_connect (group->icon_bar, "item_dragged",
			  G_CALLBACK (e_shortcut_bar_item_dragged),
			  shortcut_bar);
	g_signal_connect (group->icon_bar, "drag_data_get",
			  G_CALLBACK (e_shortcut_bar_on_drag_data_get),
			  shortcut_bar);
	g_signal_connect_after (group->icon_bar, "drag_motion",
			  G_CALLBACK (e_shortcut_bar_on_drag_motion),
			  shortcut_bar);
	g_signal_connect (group->icon_bar, "drag_drop",
			  G_CALLBACK (e_shortcut_bar_on_drag_drop),
			  shortcut_bar);
	g_signal_connect (group->icon_bar, "drag_data_received",
			  G_CALLBACK (e_shortcut_bar_on_drag_data_received),
			  shortcut_bar);
	g_signal_connect (group->icon_bar, "drag_data_delete",
			  G_CALLBACK (e_shortcut_bar_on_drag_data_delete),
			  shortcut_bar);
	g_signal_connect (group->icon_bar, "drag_end",
			  G_CALLBACK (e_shortcut_bar_on_drag_end),
			  shortcut_bar);

#ifndef E_USE_STYLES
	e_shortcut_bar_set_canvas_style (shortcut_bar, group->icon_bar);
#endif

	button = gtk_button_new ();
	g_signal_connect (button, "button_press_event",
			  G_CALLBACK (e_shortcut_bar_group_button_press),
			  shortcut_bar);

	label = e_entry_new ();
	g_object_set(label,
		     "draw_background", TRUE,
		     "draw_borders", FALSE,
		     "draw_button", TRUE,
		     "editable", FALSE,
		     "text", group_name,
		     "use_ellipsis", TRUE,
		     "justification", GTK_JUSTIFY_CENTER,
		     NULL);
	gtk_widget_show (label);
	gtk_container_add (GTK_CONTAINER (button), label);
	gtk_widget_show (button);
	g_signal_connect (button, "clicked",
			  G_CALLBACK (e_shortcut_bar_stop_editing),
			  shortcut_bar);

	e_group_bar_add_group (E_GROUP_BAR (shortcut_bar),
			       group->vscrolled_bar, button, group_num);

	gtk_widget_pop_colormap ();

	return group_num;
}


static void
e_shortcut_bar_remove_group	(EShortcutBar	*shortcut_bar,
				 gint		 group_num)
{
	e_group_bar_remove_group (E_GROUP_BAR (shortcut_bar), group_num);
	g_array_remove_index (shortcut_bar->groups, group_num);
}


static gint
e_shortcut_bar_add_item		(EShortcutBar	*shortcut_bar,
				 gint		 group_num,
				 gint		 position,
				 const gchar	*item_url,
				 const gchar	*item_name,
				 GdkPixbuf      *image)
{
	EShortcutBarGroup *group;
	gint item_num;

	g_return_val_if_fail (E_IS_SHORTCUT_BAR (shortcut_bar), -1);
	g_return_val_if_fail (group_num >= 0, -1);
	g_return_val_if_fail (group_num < shortcut_bar->groups->len, -1);
	g_return_val_if_fail (item_url != NULL, -1);
	g_return_val_if_fail (item_name != NULL, -1);

	group = &g_array_index (shortcut_bar->groups,
				EShortcutBarGroup, group_num);

	item_num = e_icon_bar_add_item (E_ICON_BAR (group->icon_bar),
					image, item_name, position);

	e_icon_bar_set_item_data_full (E_ICON_BAR (group->icon_bar), item_num,
				       g_strdup (item_url), g_free);

	return item_num;
}


static void
e_shortcut_bar_remove_item	(EShortcutBar	*shortcut_bar,
				 gint		 group_num,
				 gint		 item_num)
{
	EShortcutBarGroup *group;

	g_return_if_fail (E_IS_SHORTCUT_BAR (shortcut_bar));
	g_return_if_fail (group_num >= 0);
	g_return_if_fail (group_num < shortcut_bar->groups->len);

	group = &g_array_index (shortcut_bar->groups,
				EShortcutBarGroup, group_num);

	e_icon_bar_remove_item (E_ICON_BAR (group->icon_bar), item_num);
}

static void
e_shortcut_bar_update_item (EShortcutBar *shortcut_bar,
			    gint          group_num,
			    gint          item_num,
			    const gchar  *item_url,
			    const gchar  *item_name,
			    GdkPixbuf    *image)
{
	EShortcutBarGroup *group;
	EIconBar *icon_bar;
	EIconBarItem *item;

	g_return_if_fail (E_IS_SHORTCUT_BAR (shortcut_bar));
	g_return_if_fail (group_num >= 0);
	g_return_if_fail (group_num < shortcut_bar->groups->len);
	g_return_if_fail (item_url != NULL);
	g_return_if_fail (item_name != NULL);

	group = &g_array_index (shortcut_bar->groups,
				EShortcutBarGroup, group_num);

	icon_bar = E_ICON_BAR (group->icon_bar);

	item = &g_array_index (icon_bar->items, EIconBarItem, item_num);

	e_icon_bar_set_item_image (icon_bar, item_num, image);
	e_icon_bar_set_item_text (icon_bar, item_num, item_name);

	e_icon_bar_set_item_data_full (icon_bar, item_num,
				       g_strdup (item_url), g_free);
}

static void
e_shortcut_bar_set_canvas_style (EShortcutBar *shortcut_bar,
				 GtkWidget *canvas)
{
	GtkStyle *style;

	style = gtk_rc_get_style (GTK_WIDGET (shortcut_bar));

        gtk_widget_modify_bg (GTK_WIDGET (canvas), GTK_STATE_NORMAL, &style->bg[GTK_STATE_ACTIVE]);
}


void
e_shortcut_bar_set_view_type (EShortcutBar *shortcut_bar,
			      gint group_num,
			      EIconBarViewType view_type)
{
	EShortcutBarGroup *group;

	g_return_if_fail (E_IS_SHORTCUT_BAR (shortcut_bar));
	g_return_if_fail (group_num >= 0);
	g_return_if_fail (group_num < shortcut_bar->groups->len);

	group = &g_array_index (shortcut_bar->groups,
				EShortcutBarGroup, group_num);

	e_icon_bar_set_view_type (E_ICON_BAR (group->icon_bar), view_type);
}


EIconBarViewType
e_shortcut_bar_get_view_type (EShortcutBar *shortcut_bar,
			      gint group_num)
{
	EShortcutBarGroup *group;

	g_return_val_if_fail (E_IS_SHORTCUT_BAR (shortcut_bar), E_ICON_BAR_SMALL_ICONS);
	g_return_val_if_fail (group_num >= 0, E_ICON_BAR_SMALL_ICONS);
	g_return_val_if_fail (group_num < shortcut_bar->groups->len, E_ICON_BAR_SMALL_ICONS);

	group = &g_array_index (shortcut_bar->groups,
				EShortcutBarGroup, group_num);

	return E_ICON_BAR (group->icon_bar)->view_type;
}


static void
e_shortcut_bar_item_selected (EIconBar *icon_bar,
			      GdkEvent *event,
			      gint item_num,
			      EShortcutBar *shortcut_bar)
{
	gint group_num;

	group_num = e_group_bar_get_group_num (E_GROUP_BAR (shortcut_bar),
					       GTK_WIDGET (icon_bar)->parent);

	g_signal_emit (shortcut_bar,
		       e_shortcut_bar_signals[ITEM_SELECTED], 0,
		       event, group_num, item_num);
}


static void
e_shortcut_bar_item_dragged (EIconBar *icon_bar,
			     GdkEvent *event,
			     gint item_num,
			     EShortcutBar *shortcut_bar)
{
	GtkTargetList *target_list;
	gint group_num;

	group_num = e_group_bar_get_group_num (E_GROUP_BAR (shortcut_bar),
					       GTK_WIDGET (icon_bar)->parent);

	shortcut_bar->dragged_url = g_strdup (e_icon_bar_get_item_data (icon_bar, item_num));
	shortcut_bar->dragged_name = e_icon_bar_get_item_text (icon_bar, item_num);

	target_list = gtk_target_list_new (target_table, n_targets);
	gtk_drag_begin (GTK_WIDGET (icon_bar), target_list,
			GDK_ACTION_COPY | GDK_ACTION_MOVE,
			1, event);
	gtk_target_list_unref (target_list);
}


static void
e_shortcut_bar_on_drag_data_get (GtkWidget          *widget,
				 GdkDragContext     *context,
				 GtkSelectionData   *selection_data,
				 guint               info,
				 guint               time,
				 EShortcutBar	    *shortcut_bar)
{
	gchar *data;

	if (info == TARGET_SHORTCUT && shortcut_bar->dragged_name
	    && shortcut_bar->dragged_url) {
		data = g_strdup_printf ("%s%c%s", shortcut_bar->dragged_name,
					'\0', shortcut_bar->dragged_url);
		gtk_selection_data_set (selection_data,	selection_data->target,
					8, data,
					strlen (shortcut_bar->dragged_name)
					+ strlen (shortcut_bar->dragged_url)
					+ 2);
		g_free (data);
	}
}


static gboolean
e_shortcut_bar_on_drag_motion (GtkWidget          *widget,
			       GdkDragContext     *context,
			       gint                x,
			       gint                y,
			       guint               time,
			       EShortcutBar       *shortcut_bar)
{
	EIconBar *icon_bar;
	gboolean signal_retval;
	gint group_num, position, before_item;
	gint scroll_x, scroll_y;
	GdkDragAction action;

	icon_bar = E_ICON_BAR (widget);

	/* FIXME The canvas scrolling offset should probably be taken into
	   account by e_icon_bar_find_item_at_position(). */
	gnome_canvas_get_scroll_offsets (GNOME_CANVAS (icon_bar), &scroll_x, &scroll_y);
	position = e_icon_bar_find_item_at_position (icon_bar, x + scroll_x, y + scroll_y,
						     &before_item);
	group_num = e_group_bar_get_group_num (E_GROUP_BAR (shortcut_bar),
					       GTK_WIDGET (icon_bar)->parent);

	/* We only care about the current group.  */
	if (group_num != E_GROUP_BAR (shortcut_bar)->current_group_num) {
		gdk_drag_status (context, 0, time);
		return TRUE;
	}
 
	action = 0;

	/* If we are between icons, we can just choose the action by ourselves
	   if it's an E-SHORTCUT.  */
	if (before_item != -1) {
		/* A drag between two icons is allowed only if we are getting
		   the E-SHORTCUTS target.  */

		GList *p;

		for (p = context->targets; p != NULL; p = p->next) {
			char *target;

			target = gdk_atom_name ((GdkAtom) p->data);
			if (strcmp (target, "E-SHORTCUT") == 0) {
				gdk_drag_status (context, GDK_ACTION_MOVE, time);
				g_free (target);
				return TRUE;
			}

			g_free (target);
		}
	}

	signal_retval = FALSE;
	g_signal_emit (shortcut_bar,
		       e_shortcut_bar_signals[SHORTCUT_DRAG_MOTION], 0,
		       widget, context, time,
		       group_num, position, &signal_retval);

	if (signal_retval) {
		return TRUE;
	} else {
		gdk_drag_status (context, 0, time);
		return TRUE;
	}
}

static gboolean
e_shortcut_bar_on_drag_drop (GtkWidget *widget,
			     GdkDragContext *context,
			     gint x,
			     gint y,
			     guint time)
{
	char *target;
	GList *p;

	/* We want the E-SHORTCUT type.  */

	for (p = context->targets; p != NULL; p = p->next) {
		target = gdk_atom_name (GDK_POINTER_TO_ATOM (p->data));
		if (strcmp (target, "E-SHORTCUT") == 0) {
			g_free (target);
			gtk_drag_get_data (widget, context, GDK_POINTER_TO_ATOM (p->data), time);
			return TRUE;
		}
	}

	gtk_drag_get_data (widget, context, GDK_POINTER_TO_ATOM (context->targets->data), time);
	return TRUE;
}

static void  
e_shortcut_bar_on_drag_data_received  (GtkWidget          *widget,
				       GdkDragContext     *context,
				       gint                x,
				       gint                y,
				       GtkSelectionData   *data,
				       guint               info,
				       guint               time,
				       EShortcutBar	  *shortcut_bar)
{
	gchar *item_name, *item_url;
	EIconBar *icon_bar;
	gint position, group_num;
	gboolean signal_retval;
	gint scroll_x, scroll_y;
	gboolean before_item;
	char *target_type;

	icon_bar = E_ICON_BAR (widget);

	/* FIXME The canvas scrolling offset should probably be taken into
	   account by e_icon_bar_find_item_at_position().  */
	gnome_canvas_get_scroll_offsets (GNOME_CANVAS (icon_bar), &scroll_x, &scroll_y);
	position = e_icon_bar_find_item_at_position (icon_bar, x + scroll_x, y + scroll_y, &before_item);
	group_num = e_group_bar_get_group_num (E_GROUP_BAR (shortcut_bar),
					       GTK_WIDGET (icon_bar)->parent);

	target_type = gdk_atom_name (data->target);

	/* If it's a shortcut drop, we can handle it ourselves.  */
	if (position == -1
	    && strcmp (target_type, "E-SHORTCUT") == 0
	    && data->length >= 0 && data->format == 8) {
		item_name = data->data;
		item_url = item_name + strlen (item_name) + 1;

		g_signal_emit (shortcut_bar,
			       e_shortcut_bar_signals[SHORTCUT_DROPPED], 0,
			       group_num, before_item, item_url, item_name);

		gtk_drag_finish (context, TRUE, TRUE, time);
		g_free (target_type);
		return;
	}

	g_free (target_type);

	/* The drop is of some custom kind, so we try emitting the signal.  */
	signal_retval = FALSE;
	g_signal_emit (shortcut_bar,
		       e_shortcut_bar_signals[SHORTCUT_DRAG_DATA_RECEIVED], 0,
		       widget, context, data, time, group_num, position, &signal_retval);

	/* If the signal handler doesn't handle it, we just do a
	   gtk_drag_finish() reporting failure.  */
	if (! signal_retval)
		gtk_drag_finish (context, FALSE, FALSE, time);
}


static void  
e_shortcut_bar_on_drag_data_delete (GtkWidget          *widget,
				    GdkDragContext     *context,
				    EShortcutBar       *shortcut_bar)
{
	EIconBar *icon_bar;
	gint group_num;

	icon_bar = E_ICON_BAR (widget);

	group_num = e_group_bar_get_group_num (E_GROUP_BAR (shortcut_bar),
					       widget->parent);

	g_signal_emit (shortcut_bar, e_shortcut_bar_signals[SHORTCUT_DRAGGED], 0,
		       group_num, icon_bar->dragged_item_num);
}


static void
e_shortcut_bar_on_drag_end (GtkWidget      *widget,
			    GdkDragContext *context,
			    EShortcutBar   *shortcut_bar)
{
	g_free (shortcut_bar->dragged_name);
	shortcut_bar->dragged_name = NULL;

	g_free (shortcut_bar->dragged_url);
	shortcut_bar->dragged_url = NULL;
}


void
e_shortcut_bar_start_editing_item (EShortcutBar *shortcut_bar,
				   gint group_num,
				   gint item_num)
{
	EShortcutBarGroup *group;

	g_return_if_fail (E_IS_SHORTCUT_BAR (shortcut_bar));
	g_return_if_fail (group_num >= 0);
	g_return_if_fail (group_num < shortcut_bar->groups->len);

	group = &g_array_index (shortcut_bar->groups,
				EShortcutBarGroup, group_num);

	e_icon_bar_start_editing_item (E_ICON_BAR (group->icon_bar), item_num);
}


/* We stop editing any item when a scroll button is pressed. */
static void
e_shortcut_bar_stop_editing (GtkWidget *button,
			     EShortcutBar *shortcut_bar)
{
	EShortcutBarGroup *group;
	gint group_num;

	for (group_num = 0;
	     group_num < shortcut_bar->groups->len;
	     group_num++) {
		group = &g_array_index (shortcut_bar->groups,
					EShortcutBarGroup, group_num);
		e_icon_bar_stop_editing_item (E_ICON_BAR (group->icon_bar),
					      TRUE);
	}
}


/* Set whether items can be dragged, for drag-and-drop. */
void
e_shortcut_bar_set_enable_drags    (EShortcutBar	 *shortcut_bar,
				    gboolean	          enable_drags)
{
	EShortcutBarGroup *group;
	gint group_num;

	g_return_if_fail (E_IS_SHORTCUT_BAR (shortcut_bar));

	shortcut_bar->enable_drags = enable_drags;

	for (group_num = 0;
	     group_num < shortcut_bar->groups->len;
	     group_num++) {
		group = &g_array_index (shortcut_bar->groups,
					EShortcutBarGroup, group_num);
		e_icon_bar_set_enable_drags (E_ICON_BAR (group->icon_bar),
					     enable_drags);
	}
}

