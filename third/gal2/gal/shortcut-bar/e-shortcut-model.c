/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * e-shortcut-model.c
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
 * EShortcutModel keeps track of a number of groups of shortcuts.
 * Each shortcut has a URL and a short textual name.
 * It is used as the model of the EShortcutBar.
 */

#include <config.h>
#include <gtk/gtksignal.h>
#include <gal/util/e-util.h>
#include "e-shortcut-model.h"

/* This contains information on one item. */
typedef struct _EShortcutModelItem   EShortcutModelItem;
struct _EShortcutModelItem
{
	gchar *name;
	gchar *url;
	GdkPixbuf *image;
};


/* This contains information on one group. */
typedef struct _EShortcutModelGroup   EShortcutModelGroup;
struct _EShortcutModelGroup
{
	gchar *name;

	/* An array of EShortcutModelItem. */
	GArray *items;
};

#define ESM_CLASS(e) ((EShortcutModelClass *)(GTK_OBJECT_GET_CLASS (e)))


static void e_shortcut_model_class_init	(EShortcutModelClass *class);
static void e_shortcut_model_init	(EShortcutModel	     *shortcut_model);
static void e_shortcut_model_dispose	(GObject	     *object);
static void e_shortcut_model_free_group (EShortcutModel	     *shortcut_model,
					 gint		      group_num);

static void e_shortcut_model_real_add_group	(EShortcutModel	*shortcut_model,
						 gint		 position,
						 const gchar	*group_name);
static void e_shortcut_model_real_remove_group	(EShortcutModel	*shortcut_model,
						 gint		 group_num);
static void e_shortcut_model_real_add_item	(EShortcutModel	*shortcut_model,
						 gint		 group_num,
						 gint		 position,
						 const gchar	*item_url,
						 const gchar	*item_name,
						 GdkPixbuf      *item_image);
static void e_shortcut_model_real_remove_item	(EShortcutModel	 *shortcut_model,
						 gint		  group_num,
						 gint		  item_num);

static void e_shortcut_model_real_update_item   (EShortcutModel *shortcut_model,
						 gint            group_num,
						 gint            item_num,
						 const gchar    *item_url,
						 const gchar    *item_name,
						 GdkPixbuf      *item_image);

static gint e_shortcut_model_real_get_num_groups(EShortcutModel *shortcut_model);
static gint e_shortcut_model_real_get_num_items	(EShortcutModel *shortcut_model,
						 gint		 group_num);
static gchar* e_shortcut_model_real_get_group_name (EShortcutModel *shortcut_model,
						    gint	    group_num);
static void e_shortcut_model_real_get_item_info	(EShortcutModel *shortcut_model,
						 gint		 group_num,
						 gint		 item_num,
						 gchar	       **item_url,
						 gchar	       **item_name,
						 GdkPixbuf     **item_image);

enum
{
	GROUP_ADDED,
	GROUP_REMOVED,
	ITEM_ADDED,
	ITEM_REMOVED,
	ITEM_UPDATED,

	LAST_SIGNAL
};

static guint e_shortcut_model_signals[LAST_SIGNAL] = {0};

#define PARENT_TYPE GTK_TYPE_OBJECT
static GtkObjectClass *parent_class;


E_MAKE_TYPE(e_shortcut_model, "EShortcutModel", EShortcutModel,
	    e_shortcut_model_class_init, e_shortcut_model_init,
	    GTK_TYPE_OBJECT)


static void
e_shortcut_model_class_init	(EShortcutModelClass	*class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_ref (PARENT_TYPE);

	object_class = (GObjectClass *) class;

	e_shortcut_model_signals[GROUP_ADDED] =
		g_signal_new ("group_added",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (EShortcutModelClass, group_added),
			      NULL, NULL,
			      e_marshal_NONE__INT_STRING,
			      G_TYPE_NONE, 2,
			      G_TYPE_INT, G_TYPE_STRING);

	e_shortcut_model_signals[GROUP_REMOVED] =
		g_signal_new ("group_removed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (EShortcutModelClass, group_removed),
			      NULL, NULL,
			      e_marshal_NONE__INT,
			      G_TYPE_NONE, 1,
			      G_TYPE_INT);

	e_shortcut_model_signals[ITEM_ADDED] =
		g_signal_new ("item_added",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (EShortcutModelClass, item_added),
			      NULL, NULL,
			      e_marshal_NONE__INT_INT_STRING_STRING_POINTER,
			      G_TYPE_NONE, 5,
			      G_TYPE_INT, G_TYPE_INT,
			      G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);

	e_shortcut_model_signals[ITEM_REMOVED] =
		g_signal_new ("item_removed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (EShortcutModelClass, item_removed),
			      NULL, NULL,
			      e_marshal_NONE__INT_INT,
			      G_TYPE_NONE, 2,
			      G_TYPE_INT, G_TYPE_INT);

	e_shortcut_model_signals[ITEM_UPDATED] =
		g_signal_new ("item_updated",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (EShortcutModelClass, item_updated),
			      NULL, NULL,
			      e_marshal_NONE__INT_INT_STRING_STRING_POINTER,
			      G_TYPE_NONE, 5,
			      G_TYPE_INT, G_TYPE_INT,
			      G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);

	/* Method override */
	object_class->dispose		= e_shortcut_model_dispose;

	class->group_added		= e_shortcut_model_real_add_group;
	class->group_removed		= e_shortcut_model_real_remove_group;
	class->item_added		= e_shortcut_model_real_add_item;
	class->item_removed		= e_shortcut_model_real_remove_item;
	class->item_updated             = e_shortcut_model_real_update_item;

	class->get_num_groups		= e_shortcut_model_real_get_num_groups;
	class->get_num_items		= e_shortcut_model_real_get_num_items;
	class->get_group_name		= e_shortcut_model_real_get_group_name;
	class->get_item_info		= e_shortcut_model_real_get_item_info;
}


static void
e_shortcut_model_init		(EShortcutModel	*shortcut_model)
{
	shortcut_model->groups = g_array_new (FALSE, FALSE,
					      sizeof (EShortcutModelGroup));
}


EShortcutModel *
e_shortcut_model_new		(void)
{
	EShortcutModel *shortcut_model;

	shortcut_model = E_SHORTCUT_MODEL (g_object_new (E_TYPE_SHORTCUT_MODEL, NULL));

	return shortcut_model;
}


static void
e_shortcut_model_dispose	(GObject	*object)
{
	EShortcutModel *shortcut_model;
	gint num_groups, group_num;

	shortcut_model = E_SHORTCUT_MODEL (object);

	if (shortcut_model->groups) {
		num_groups = shortcut_model->groups->len;
		for (group_num = 0; group_num < num_groups; group_num++)
			e_shortcut_model_free_group (shortcut_model, group_num);

		g_array_free (shortcut_model->groups, TRUE);
		shortcut_model->groups = NULL;
	}

	G_OBJECT_CLASS (parent_class)->dispose (object);
}


static void
e_shortcut_model_free_group	(EShortcutModel	*shortcut_model,
				 gint		 group_num)
{
	EShortcutModelGroup *group;
	EShortcutModelItem *item;
	gint item_num;

	group = &g_array_index (shortcut_model->groups,
				EShortcutModelGroup, group_num);

	g_free (group->name);

	for (item_num = 0; item_num < group->items->len; item_num++) {
		item = &g_array_index (group->items,
				       EShortcutModelItem, item_num);
		g_free (item->name);
		g_free (item->url);
	}
}


gint
e_shortcut_model_add_group	(EShortcutModel	*shortcut_model,
				 gint		 position,
				 const gchar	*group_name)
{
	g_return_val_if_fail (E_IS_SHORTCUT_MODEL (shortcut_model), -1);
	g_return_val_if_fail (group_name != NULL, -1);

	if (position == -1 || position > shortcut_model->groups->len)
		position = shortcut_model->groups->len;

	g_signal_emit (shortcut_model,
		       e_shortcut_model_signals[GROUP_ADDED], 0,
		       position, group_name);

	return position;
}


static void
e_shortcut_model_real_add_group	(EShortcutModel	*shortcut_model,
				 gint		 group_num,
				 const gchar	*group_name)
{
	EShortcutModelGroup *group, tmp_group;

	g_return_if_fail (E_IS_SHORTCUT_MODEL (shortcut_model));
	g_return_if_fail (group_num >= 0);
	g_return_if_fail (group_num <= shortcut_model->groups->len);
	g_return_if_fail (group_name != NULL);

	g_array_insert_val (shortcut_model->groups, group_num, tmp_group);

	group = &g_array_index (shortcut_model->groups,
				EShortcutModelGroup, group_num);

	group->name = g_strdup (group_name);
	group->items = g_array_new (FALSE, FALSE,
				    sizeof (EShortcutModelItem));
}


void
e_shortcut_model_remove_group	(EShortcutModel	*shortcut_model,
				 gint		 group_num)
{
	g_return_if_fail (E_IS_SHORTCUT_MODEL (shortcut_model));
	g_return_if_fail (group_num >= 0);
	g_return_if_fail (group_num < shortcut_model->groups->len);

	g_signal_emit (shortcut_model,
		       e_shortcut_model_signals[GROUP_REMOVED], 0,
		       group_num);
}


static void
e_shortcut_model_real_remove_group	(EShortcutModel	*shortcut_model,
					 gint		 group_num)
{
	g_return_if_fail (E_IS_SHORTCUT_MODEL (shortcut_model));
	g_return_if_fail (group_num >= 0);
	g_return_if_fail (group_num < shortcut_model->groups->len);

	e_shortcut_model_free_group (shortcut_model, group_num);
	g_array_remove_index (shortcut_model->groups, group_num);
}


gint
e_shortcut_model_add_item	(EShortcutModel	*shortcut_model,
				 gint		 group_num,
				 gint		 position,
				 const gchar	*item_url,
				 const gchar	*item_name,
				 GdkPixbuf      *item_image)
{
	EShortcutModelGroup *group;

	g_return_val_if_fail (E_IS_SHORTCUT_MODEL (shortcut_model), -1);
	g_return_val_if_fail (group_num >= 0, -1);
	g_return_val_if_fail (group_num < shortcut_model->groups->len, -1);
	g_return_val_if_fail (item_url != NULL, -1);
	g_return_val_if_fail (item_name != NULL, -1);

	group = &g_array_index (shortcut_model->groups,
				EShortcutModelGroup, group_num);

	if (position == -1 || position > group->items->len)
		position = group->items->len;

	g_signal_emit (shortcut_model,
		       e_shortcut_model_signals[ITEM_ADDED], 0,
		       group_num, position, item_url, item_name, item_image);

	return position;
}


static void
e_shortcut_model_real_add_item	(EShortcutModel	*shortcut_model,
				 gint		 group_num,
				 gint		 item_num,
				 const gchar	*item_url,
				 const gchar	*item_name,
				 GdkPixbuf      *item_image)
{
	EShortcutModelGroup *group;
	EShortcutModelItem *item, tmp_item;

	g_return_if_fail (E_IS_SHORTCUT_MODEL (shortcut_model));
	g_return_if_fail (group_num >= 0);
	g_return_if_fail (group_num < shortcut_model->groups->len);
	g_return_if_fail (item_url != NULL);
	g_return_if_fail (item_name != NULL);

	group = &g_array_index (shortcut_model->groups,
				EShortcutModelGroup, group_num);

	g_return_if_fail (item_num >= 0);
	g_return_if_fail (item_num <= group->items->len);

	g_array_insert_val (group->items, item_num, tmp_item);

	item = &g_array_index (group->items,
			       EShortcutModelItem, item_num);

	item->name = g_strdup (item_name);
	item->url = g_strdup (item_url);

	if (item_image != NULL)
		item->image = gdk_pixbuf_ref (item_image);
	else
		item->image = NULL;
}


void
e_shortcut_model_remove_item	(EShortcutModel	 *shortcut_model,
				 gint		  group_num,
				 gint		  item_num)
{
	g_signal_emit (shortcut_model,
		       e_shortcut_model_signals[ITEM_REMOVED], 0,
		       group_num, item_num);
}


static void
e_shortcut_model_real_remove_item	(EShortcutModel	 *shortcut_model,
					 gint		  group_num,
					 gint		  item_num)
{
	EShortcutModelGroup *group;
	EShortcutModelItem *item;

	g_return_if_fail (E_IS_SHORTCUT_MODEL (shortcut_model));
	g_return_if_fail (group_num >= 0);
	g_return_if_fail (group_num < shortcut_model->groups->len);

	group = &g_array_index (shortcut_model->groups,
				EShortcutModelGroup, group_num);

	g_return_if_fail (item_num >= 0);
	g_return_if_fail (item_num < group->items->len);

	item = &g_array_index (group->items,
			       EShortcutModelItem, item_num);

	g_free (item->name);
	g_free (item->url);

	g_array_remove_index (group->items, item_num);
}

static void
e_shortcut_model_real_update_item (EShortcutModel *shortcut_model,
				   gint            group_num,
				   gint            item_num,
				   const gchar    *item_url,
				   const gchar    *item_name,
				   GdkPixbuf      *item_image)
{
	EShortcutModelGroup *group;
	EShortcutModelItem *item;

	g_return_if_fail (E_IS_SHORTCUT_MODEL (shortcut_model));
	g_return_if_fail (group_num >= 0);
	g_return_if_fail (group_num < shortcut_model->groups->len);
	g_return_if_fail (item_url != NULL);
	g_return_if_fail (item_name != NULL);

	group = &g_array_index (shortcut_model->groups,
				EShortcutModelGroup, group_num);

	g_return_if_fail (item_num >= 0);
	g_return_if_fail (item_num <= group->items->len);

	item = &g_array_index (group->items,
			       EShortcutModelItem, item_num);

	g_free (item->name);
	g_free (item->url);

	if (item->image != NULL)
		gdk_pixbuf_unref (item->image);

	item->name = g_strdup (item_name);
	item->url = g_strdup (item_url);

	if (item->image != NULL)
		item->image = gdk_pixbuf_ref (item_image);
	else
		item->image = NULL;
}

void
e_shortcut_model_update_item (EShortcutModel *shortcut_model,
			      gint            group_num,
			      gint            item_num,
			      const gchar    *item_url,
			      const gchar    *item_name,
			      GdkPixbuf      *item_image)
{

	g_return_if_fail (E_IS_SHORTCUT_MODEL (shortcut_model));
	g_return_if_fail (group_num >= 0);
	g_return_if_fail (group_num < shortcut_model->groups->len);
	g_return_if_fail (item_url != NULL);
	g_return_if_fail (item_name != NULL);
			  
	g_signal_emit (shortcut_model,
		       e_shortcut_model_signals[ITEM_UPDATED], 0,
		       group_num, item_num, item_url, item_name, item_image);
}

gint
e_shortcut_model_get_num_groups	(EShortcutModel *shortcut_model)
{
	g_return_val_if_fail (E_IS_SHORTCUT_MODEL (shortcut_model), 0);

	return ESM_CLASS (shortcut_model)->get_num_groups (shortcut_model);
}


static gint
e_shortcut_model_real_get_num_groups	(EShortcutModel *shortcut_model)
{
	return shortcut_model->groups->len;
}


gint
e_shortcut_model_get_num_items	(EShortcutModel *shortcut_model,
				 gint		 group_num)
{
	g_return_val_if_fail (E_IS_SHORTCUT_MODEL (shortcut_model), 0);

	return ESM_CLASS (shortcut_model)->get_num_items (shortcut_model,
							  group_num);
}


static gint
e_shortcut_model_real_get_num_items	(EShortcutModel *shortcut_model,
					 gint		 group_num)
{
	EShortcutModelGroup *group;

	g_return_val_if_fail (group_num >= 0, 0);
	g_return_val_if_fail (group_num < shortcut_model->groups->len, 0);

	group = &g_array_index (shortcut_model->groups,
				EShortcutModelGroup, group_num);

	return group->items->len;
}


gchar*
e_shortcut_model_get_group_name	(EShortcutModel *shortcut_model,
				 gint		 group_num)
{
	g_return_val_if_fail (E_IS_SHORTCUT_MODEL (shortcut_model), NULL);

	return ESM_CLASS (shortcut_model)->get_group_name (shortcut_model,
							   group_num);
}


static gchar*
e_shortcut_model_real_get_group_name	(EShortcutModel *shortcut_model,
					 gint		 group_num)
{
	EShortcutModelGroup *group;

	g_return_val_if_fail (group_num >= 0, NULL);
	g_return_val_if_fail (group_num < shortcut_model->groups->len, NULL);

	group = &g_array_index (shortcut_model->groups,
				EShortcutModelGroup, group_num);

	return g_strdup (group->name);
}


void
e_shortcut_model_get_item_info	(EShortcutModel *shortcut_model,
				 gint		 group_num,
				 gint		 item_num,
				 gchar	       **item_url,
				 gchar	       **item_name,
				 GdkPixbuf     **item_image)
{
	g_return_if_fail (E_IS_SHORTCUT_MODEL (shortcut_model));

	ESM_CLASS (shortcut_model)->get_item_info (shortcut_model,
						   group_num, item_num,
						   item_url, item_name, item_image);
}


static void
e_shortcut_model_real_get_item_info	(EShortcutModel *shortcut_model,
					 gint		 group_num,
					 gint		 item_num,
					 gchar	       **item_url,
					 gchar	       **item_name,
					 GdkPixbuf     **item_image)
{
	EShortcutModelGroup *group;
	EShortcutModelItem *item;

	g_return_if_fail (group_num >= 0);
	g_return_if_fail (group_num < shortcut_model->groups->len);

	group = &g_array_index (shortcut_model->groups,
				EShortcutModelGroup, group_num);

	g_return_if_fail (item_num >= 0);
	g_return_if_fail (item_num < group->items->len);

	item = &g_array_index (group->items,
			       EShortcutModelItem, item_num);

	if (item_url)
		*item_url = g_strdup (item->url);

	if (item_name)
		*item_name = g_strdup (item->name);

	if (item_image)
		*item_image = item->image ? gdk_pixbuf_ref (item->image) : NULL;
}

