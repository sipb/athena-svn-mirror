/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * Copyright (C) 2001, 2002 Anders Carlsson <andersca@gnu.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __GCONF_EDITOR_WINDOW_H__
#define __GCONF_EDITOR_WINDOW_H__

#include <gtk/gtkitemfactory.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtktextbuffer.h>
#include <gtk/gtktreemodel.h>
#include <gtk/gtktreeviewcolumn.h>
#include <gtk/gtkwindow.h>

#define GCONF_TYPE_EDITOR_WINDOW		  (gconf_editor_window_get_type ())
#define GCONF_EDITOR_WINDOW(obj)		  (GTK_CHECK_CAST ((obj), GCONF_TYPE_EDITOR_WINDOW, GConfEditorWindow))
#define GCONF_EDITOR_WINDOW_CLASS(klass)	  (GTK_CHECK_CLASS_CAST ((klass), GCONF_TYPE_EDITOR_WINDOW, GConfEditorWindowClass))
#define GCONF_IS_EDITOR_WINDOW(obj)	  (GTK_CHECK_TYPE ((obj), GCONF_TYPE_EDITOR_WINDOW))
#define GCONF_IS_EDITOR_WINDOW_CLASS(klass)  (GTK_CHECK_CLASS_TYPE ((obj), GCONF_TYPE_EDITOR_WINDOW))
#define GCONF_EDITOR_WINDOW_GET_CLASS(obj)   (GTK_CHECK_GET_CLASS ((obj), GCONF_TYPE_EDITOR_WINDOW, GConfEditorWindowClass))

typedef struct _GConfEditorWindow GConfEditorWindow;
typedef struct _GConfEditorWindowClass GConfEditorWindowClass;

struct _GConfEditorWindow {
	GtkWindow parent_instance;

	GtkWidget *tree_view;
	GtkTreeModel *tree_model;
	GtkTreeModel *sorted_tree_model;
	
	GtkWidget *list_view;
	GtkTreeModel *list_model;
	GtkTreeModel *sorted_list_model;
	
	GtkWidget *statusbar;

	GtkItemFactory *popup_factory;
	GtkItemFactory *item_factory;
	GtkTreeViewColumn *value_column;

	GtkWidget *key_name_label;
	GtkWidget *short_desc_label;
	GtkTextBuffer *long_desc_buffer;
	GtkWidget *owner_label;

	guint tearoffs_notify_id;
	guint icons_notify_id;
};

struct _GConfEditorWindowClass {
	GtkWindowClass parent_class;
};

GType gconf_editor_window_get_type (void);
GtkWidget *gconf_editor_window_new (void);

#endif /* __GCONF_EDITOR_WINDOW_H__ */

