/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * e-shortcut-bar.h
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

#ifndef _E_SHORTCUT_BAR_H_
#define _E_SHORTCUT_BAR_H_

#include "e-group-bar.h"
#include "e-icon-bar.h"
#include "e-shortcut-model.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * EShortcutBar displays a vertical bar with a number of Groups, each of which
 * contains any number of icons. It is used on the left of the main application
 * window so users can easily access items such as folders and files.
 */

typedef struct _EShortcutBar       EShortcutBar;
typedef struct _EShortcutBarClass  EShortcutBarClass;


typedef GdkPixbuf* (*EShortcutBarIconCallback)   (EShortcutBar *shortcut_bar,
						  const gchar  *url,
						  gpointer      data);

/* This contains information on one group. */
typedef struct _EShortcutBarGroup   EShortcutBarGroup;
struct _EShortcutBarGroup
{
	/* This is the EVScrolledBar which scrolls the group. */
	GtkWidget *vscrolled_bar;

	/* This is the icon bar containing the child items. */
	GtkWidget *icon_bar;
};


#define E_TYPE_SHORTCUT_BAR	     (e_shortcut_bar_get_type ())
#define E_SHORTCUT_BAR(obj)          GTK_CHECK_CAST (obj, e_shortcut_bar_get_type (), EShortcutBar)
#define E_SHORTCUT_BAR_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, e_shortcut_bar_get_type (), EShortcutBarClass)
#define E_IS_SHORTCUT_BAR(obj)       GTK_CHECK_TYPE (obj, e_shortcut_bar_get_type ())


struct _EShortcutBar
{
	EGroupBar group_bar;

	/* This is the underlying model. */
	EShortcutModel *model;

	/* This is an array of EShortcutBarGroup elements. */
	GArray *groups;

	gchar *dragged_url;
	gchar *dragged_name;

	/* Whether we allow items to be dragged. */
	gboolean enable_drags;
};

struct _EShortcutBarClass
{
	EGroupBarClass parent_class;

	void   (*item_selected)        (EShortcutBar   *shortcut_bar,
					GdkEvent       *event,
					gint		group_num,
					gint		item_num);

	void   (*shortcut_dropped)     (EShortcutBar   *shortcut_bar,
					gint            group_num,
					gint            position,
					const gchar    *item_url,
					const char     *item_name);

	void   (*shortcut_dragged)     (EShortcutBar   *shortcut_bar,
					gint            group_num,
					gint            item_num);
};


GtkType	   	  e_shortcut_bar_get_type	     (void);
GtkWidget* 	  e_shortcut_bar_new		     (void);

/* Sets the underlying model. */
void		  e_shortcut_bar_set_model	     (EShortcutBar	 *shortcut_bar,
						      EShortcutModel	 *shortcut_model);

/* Sets/gets the view type for the group. */
void              e_shortcut_bar_set_view_type	     (EShortcutBar	 *shortcut_bar,
						      gint		  group_num,
						      EIconBarViewType view_type);
EIconBarViewType  e_shortcut_bar_get_view_type	     (EShortcutBar	 *shortcut_bar,
						      gint		  group_num);

void	   	  e_shortcut_bar_start_editing_item  (EShortcutBar	 *shortcut_bar,
						      gint		  group_num,
						      gint		  item_num);

/* Set whether items can be dragged, for drag-and-drop. */
void              e_shortcut_bar_set_enable_drags    (EShortcutBar	 *shortcut_bar,
						      gboolean	          enable_drags);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _E_SHORTCUT_BAR_H_ */
