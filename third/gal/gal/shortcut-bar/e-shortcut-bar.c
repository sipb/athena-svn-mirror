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
#include <libgnome/gnome-defs.h>
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
static void e_shortcut_bar_destroy		(GtkObject	*object);

static void e_shortcut_bar_disconnect_model	(EShortcutBar	*shortcut_bar);

static void e_shortcut_bar_on_model_destroyed	(EShortcutModel	*model,
						 EShortcutBar	*shortcut_bar);
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
  LAST_SIGNAL
};

static guint e_shortcut_bar_signals[LAST_SIGNAL] = {0};

static EGroupBarClass *parent_class;

static void
e_shortcut_bar_marshal_NONE__INT_INT_STRING_STRING (GtkObject *object,
						    GtkSignalFunc func,
						    gpointer func_data,
						    GtkArg *args)
{
	void (*rfunc) (GtkObject *, gint, gint, gchar *, gchar *, gpointer) = func;

	(*rfunc) (object,
		  GTK_VALUE_INT (args[0]),
		  GTK_VALUE_INT (args[1]),
		  GTK_VALUE_STRING (args[2]),
		  GTK_VALUE_STRING (args[3]),
		  func_data);
}

E_MAKE_TYPE(e_shortcut_bar, "EShortcutBar", EShortcutBar,
	    e_shortcut_bar_class_init, e_shortcut_bar_init,
	    e_group_bar_get_type())


static void
e_shortcut_bar_class_init (EShortcutBarClass *class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	parent_class = gtk_type_class (e_group_bar_get_type ());

	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	e_shortcut_bar_signals[ITEM_SELECTED] =
		gtk_signal_new ("item_selected",
				GTK_RUN_LAST | GTK_RUN_ACTION,
				E_OBJECT_CLASS_TYPE (object_class),
				GTK_SIGNAL_OFFSET (EShortcutBarClass,
						   item_selected),
				gtk_marshal_NONE__POINTER_INT_INT,
				GTK_TYPE_NONE, 3, GTK_TYPE_GDK_EVENT,
				GTK_TYPE_INT, GTK_TYPE_INT);

	e_shortcut_bar_signals[SHORTCUT_DROPPED] =
		gtk_signal_new ("shortcut_dropped",
				GTK_RUN_LAST | GTK_RUN_ACTION,
				E_OBJECT_CLASS_TYPE (object_class),
				GTK_SIGNAL_OFFSET (EShortcutBarClass, shortcut_dropped),
				e_shortcut_bar_marshal_NONE__INT_INT_STRING_STRING,
				GTK_TYPE_NONE, 4,
				GTK_TYPE_INT,
				GTK_TYPE_INT,
				GTK_TYPE_STRING,
				GTK_TYPE_STRING);

	e_shortcut_bar_signals[SHORTCUT_DRAGGED] =
		gtk_signal_new ("shortcut_dragged",
				GTK_RUN_LAST | GTK_RUN_ACTION,
				E_OBJECT_CLASS_TYPE (object_class),
				GTK_SIGNAL_OFFSET (EShortcutBarClass, shortcut_dragged),
				gtk_marshal_NONE__INT_INT,
				GTK_TYPE_NONE, 2,
				GTK_TYPE_INT,
				GTK_TYPE_INT);

	E_OBJECT_CLASS_ADD_SIGNALS (object_class, e_shortcut_bar_signals,
				      LAST_SIGNAL);

	/* Method override */
	object_class->destroy		= e_shortcut_bar_destroy;
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
	gtk_widget_push_visual (gdk_rgb_get_visual ());

	shortcut_bar = GTK_WIDGET (gtk_type_new (e_shortcut_bar_get_type ()));

	gtk_widget_pop_visual ();
	gtk_widget_pop_colormap ();

	return shortcut_bar;
}


static void
e_shortcut_bar_destroy (GtkObject *object)
{
	EShortcutBar *shortcut_bar;

	shortcut_bar = E_SHORTCUT_BAR (object);

	e_shortcut_bar_disconnect_model (shortcut_bar);

	g_array_free (shortcut_bar->groups, TRUE);

	g_free (shortcut_bar->dragged_url);
	g_free (shortcut_bar->dragged_name);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}


void
e_shortcut_bar_set_model (EShortcutBar *shortcut_bar,
			  EShortcutModel *model)
{
	gint num_groups, group_num, num_items, item_num;
	gchar *group_name, *item_url, *item_name;
	GdkPixbuf *item_image;

	/* Disconnect any existing model. */
	e_shortcut_bar_disconnect_model (shortcut_bar);

	shortcut_bar->model = model;

	if (!model)
		return;

	gtk_signal_connect (GTK_OBJECT (model), "destroy",
			    GTK_SIGNAL_FUNC (e_shortcut_bar_on_model_destroyed),
			    shortcut_bar);
	gtk_signal_connect (GTK_OBJECT (model), "group_added",
			    GTK_SIGNAL_FUNC (e_shortcut_bar_on_group_added),
			    shortcut_bar);
	gtk_signal_connect (GTK_OBJECT (model), "group_removed",
			    GTK_SIGNAL_FUNC (e_shortcut_bar_on_group_removed),
			    shortcut_bar);
	gtk_signal_connect (GTK_OBJECT (model), "item_added",
			    GTK_SIGNAL_FUNC (e_shortcut_bar_on_item_added),
			    shortcut_bar);
	gtk_signal_connect (GTK_OBJECT (model), "item_removed",
			    GTK_SIGNAL_FUNC (e_shortcut_bar_on_item_removed),
			    shortcut_bar);
	gtk_signal_connect (GTK_OBJECT (model), "item_updated",
			    GTK_SIGNAL_FUNC (e_shortcut_bar_on_item_updated),
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
e_shortcut_bar_disconnect_model (EShortcutBar *shortcut_bar)
{
	/* Remove all the current groups. */
	while (shortcut_bar->groups->len)
		e_shortcut_bar_remove_group (shortcut_bar, 0);

	if (shortcut_bar->model) {
		/* Disconnect all the signals in one go. */
		gtk_signal_disconnect_by_data (GTK_OBJECT (shortcut_bar->model), shortcut_bar);
		shortcut_bar->model = NULL;
	}
}


static void
e_shortcut_bar_on_model_destroyed	(EShortcutModel	*model,
					 EShortcutBar	*shortcut_bar)
{
	e_shortcut_bar_disconnect_model (shortcut_bar);
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

static void
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
			gtk_signal_emit (GTK_OBJECT (shortcut_bar), e_shortcut_bar_signals[ITEM_SELECTED],
					 button_event, i, -1);
			break;
		}
	}
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
	gtk_widget_push_visual (gdk_rgb_get_visual ());

	group_num = position;
	g_array_insert_val (shortcut_bar->groups, group_num, tmp_group);

	group = &g_array_index (shortcut_bar->groups,
				EShortcutBarGroup, group_num);

	group->vscrolled_bar = e_vscrolled_bar_new (NULL);
	gtk_widget_show (group->vscrolled_bar);
	gtk_signal_connect (
		GTK_OBJECT (E_VSCROLLED_BAR (group->vscrolled_bar)->up_button),
		"pressed", GTK_SIGNAL_FUNC (e_shortcut_bar_stop_editing), shortcut_bar);
	gtk_signal_connect (
		GTK_OBJECT (E_VSCROLLED_BAR (group->vscrolled_bar)->down_button),
		"pressed", GTK_SIGNAL_FUNC (e_shortcut_bar_stop_editing), shortcut_bar);

	group->icon_bar = e_icon_bar_new ();
	e_icon_bar_set_enable_drags (E_ICON_BAR (group->icon_bar),
				     shortcut_bar->enable_drags);
	gtk_widget_show (group->icon_bar);
	gtk_container_add (GTK_CONTAINER (group->vscrolled_bar),
			   group->icon_bar);
	gtk_signal_connect (GTK_OBJECT (group->icon_bar), "item_selected",
			    GTK_SIGNAL_FUNC (e_shortcut_bar_item_selected),
			    shortcut_bar);
	gtk_signal_connect (GTK_OBJECT (group->icon_bar), "item_dragged",
			    GTK_SIGNAL_FUNC (e_shortcut_bar_item_dragged),
			    shortcut_bar);
	gtk_signal_connect (GTK_OBJECT (group->icon_bar), "drag_data_get",
			    GTK_SIGNAL_FUNC (e_shortcut_bar_on_drag_data_get),
			    shortcut_bar);
	gtk_signal_connect (GTK_OBJECT (group->icon_bar), "drag_data_received",
			    GTK_SIGNAL_FUNC (e_shortcut_bar_on_drag_data_received),
			    shortcut_bar);
	gtk_signal_connect (GTK_OBJECT (group->icon_bar), "drag_data_delete",
			    GTK_SIGNAL_FUNC (e_shortcut_bar_on_drag_data_delete),
			    shortcut_bar);
	gtk_signal_connect (GTK_OBJECT (group->icon_bar), "drag_end",
			    GTK_SIGNAL_FUNC (e_shortcut_bar_on_drag_end),
			    shortcut_bar);

#ifndef E_USE_STYLES
	e_shortcut_bar_set_canvas_style (shortcut_bar, group->icon_bar);
#endif

	button = gtk_button_new ();
	gtk_signal_connect (GTK_OBJECT (button), "button_press_event",
			    GTK_SIGNAL_FUNC (e_shortcut_bar_group_button_press),
			    shortcut_bar);

	label = e_entry_new ();
	gtk_object_set(GTK_OBJECT(label),
		       "draw_background", FALSE,
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
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
			    GTK_SIGNAL_FUNC (e_shortcut_bar_stop_editing),
			    shortcut_bar);

	gtk_drag_dest_set (GTK_WIDGET (group->icon_bar),
			   GTK_DEST_DEFAULT_ALL,
			   target_table, n_targets,
			   GDK_ACTION_COPY | GDK_ACTION_MOVE);
	gtk_drag_dest_set (GTK_WIDGET (button),
			   GTK_DEST_DEFAULT_ALL,
			   target_table, n_targets,
			   GDK_ACTION_COPY | GDK_ACTION_MOVE);

	e_group_bar_add_group (E_GROUP_BAR (shortcut_bar),
			       group->vscrolled_bar, button, group_num);

	gtk_widget_pop_visual ();
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
        GtkRcStyle *rc_style;

        rc_style = gtk_rc_style_new ();

        rc_style->color_flags[GTK_STATE_NORMAL] = GTK_RC_FG | GTK_RC_BG;
        rc_style->fg[GTK_STATE_NORMAL].red   = 65535;
        rc_style->fg[GTK_STATE_NORMAL].green = 65535;
        rc_style->fg[GTK_STATE_NORMAL].blue  = 65535;

        rc_style->bg[GTK_STATE_NORMAL].red   = 0x8000;
        rc_style->bg[GTK_STATE_NORMAL].green = 0x8000;
        rc_style->bg[GTK_STATE_NORMAL].blue  = 0x8000;

        gtk_widget_modify_style (GTK_WIDGET (canvas), rc_style);
        gtk_rc_style_unref (rc_style);
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

	gtk_signal_emit (GTK_OBJECT (shortcut_bar),
			 e_shortcut_bar_signals[ITEM_SELECTED],
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

	icon_bar = E_ICON_BAR (widget);
	position = icon_bar->dragging_before_item_num;

	if ((data->length >= 0) && (data->format == 8)
	    && (position != -1) && (info == TARGET_SHORTCUT)) {
		item_name = data->data;
		item_url = item_name + strlen (item_name) + 1;

		group_num = e_group_bar_get_group_num (E_GROUP_BAR (shortcut_bar),
						       GTK_WIDGET (icon_bar)->parent);

		gtk_signal_emit (GTK_OBJECT (shortcut_bar),
				 e_shortcut_bar_signals[SHORTCUT_DROPPED],
				 group_num, position, item_url, item_name);

		gtk_drag_finish (context, TRUE, TRUE, time);
		return;
	}
  
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

	gtk_signal_emit (GTK_OBJECT (shortcut_bar), e_shortcut_bar_signals[SHORTCUT_DRAGGED],
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

