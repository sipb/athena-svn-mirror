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

#include "gconf-bookmarks.h"

#include "gconf-stock-icons.h"
#include "gconf-tree-model.h"
#include <gconf/gconf-client.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkimagemenuitem.h>
#include <gtk/gtkobject.h>
#include <gtk/gtkseparatormenuitem.h>
#include <gtk/gtktreemodelsort.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtktreeview.h>
#include <string.h>


static void
gconf_bookmarks_bookmark_activated (GtkWidget *menuitem, GConfEditorWindow *window)
{
	char *key;
	GtkTreePath *path, *child_path;
	gint depth;
	gint i;

	key = g_object_get_data (G_OBJECT (menuitem), "gconf-key");

	child_path = gconf_tree_model_get_tree_path_from_gconf_path (GCONF_TREE_MODEL (window->tree_model), key);

	path = gtk_tree_model_sort_convert_child_path_to_path (GTK_TREE_MODEL_SORT (window->sorted_tree_model),
							       child_path);

	gtk_tree_path_free (child_path);
	
	/* kind of hackish, but it works! */
	depth = gtk_tree_path_get_depth (path);
	for (i = 0; i < depth; i++) {
		gint j;
		GtkTreePath *cpath = gtk_tree_path_copy (path);

		for (j = 0; j < (depth - i); j++)
			gtk_tree_path_up (cpath);

		gtk_tree_view_expand_row (GTK_TREE_VIEW (window->tree_view), cpath, FALSE);
		gtk_tree_path_free (cpath);
	}

	gtk_tree_selection_select_path (gtk_tree_view_get_selection (GTK_TREE_VIEW (window->tree_view)), path);
	gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (window->tree_view), path, NULL, TRUE, 0.5, 0.5);
	gtk_tree_path_free (path);
}

static void
gconf_bookmarks_update_menu (GtkWidget *menu)
{
	GSList *list, *tmp;
	GtkWidget *menuitem, *window;

	window = g_object_get_data (G_OBJECT (menu), "editor-window");
	
	/* Get the old list and then set it */
	list = gconf_client_get_list (gconf_client_get_default (),
				     "/apps/gconf-editor/bookmarks", GCONF_VALUE_STRING, NULL);

	if (list != NULL) {
		menuitem = gtk_separator_menu_item_new ();
		gtk_widget_show (menuitem);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	}

	for (tmp = list; tmp; tmp = tmp->next) {
		menuitem = gtk_image_menu_item_new_with_label (tmp->data);
		g_signal_connect (menuitem, "activate",
				  G_CALLBACK (gconf_bookmarks_bookmark_activated), window);
		g_object_set_data_full (G_OBJECT (menuitem), "gconf-key", g_strdup (tmp->data), g_free);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), gtk_image_new_from_stock (GCONF_STOCK_BOOKMARK,
													 GTK_ICON_SIZE_MENU));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
		gtk_widget_show_all (menuitem);
	}
}

static void
gconf_bookmarks_key_changed (GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer user_data)
{
	GList *child_list, *tmp;
	GtkWidget *menu_item;
	
	child_list = gtk_container_get_children (GTK_CONTAINER (user_data));

	for (tmp = child_list; tmp; tmp = tmp->next) {
		menu_item = tmp->data;

		if (g_object_get_data (G_OBJECT (menu_item), "gconf-key") != NULL ||
			GTK_IS_SEPARATOR_MENU_ITEM (menu_item)) {
			gtk_widget_destroy (menu_item);
		}
	}

	gconf_bookmarks_update_menu (GTK_WIDGET (user_data));
	
	g_list_free (child_list);
}

void
gconf_bookmarks_add_bookmark (const char *path)
{
	GSList *list, *tmp;

	/* Get the old list and then set it */
	list = gconf_client_get_list (gconf_client_get_default (),
				     "/apps/gconf-editor/bookmarks", GCONF_VALUE_STRING, NULL);

	/* FIXME: We need error handling here, also this function leaks memory */

	/* Check that the bookmark hasn't been added already */
	for (tmp = list; tmp; tmp = tmp->next) {
		if (strcmp (tmp->data, path) == 0) {
			g_slist_free (list);
			return;
		}
	}

	/* Append the new bookmark */
	list = g_slist_append (list, g_strdup (path));
	
	gconf_client_set_list (gconf_client_get_default (),
			       "/apps/gconf-editor/bookmarks", GCONF_VALUE_STRING, list, NULL);
	g_slist_free (list);
}

void
remove_notify_id (gpointer data)
{
	gconf_client_notify_remove (gconf_client_get_default (), GPOINTER_TO_INT (data));
}

void
gconf_bookmarks_hook_up_menu (GConfEditorWindow *window, GtkWidget *menu)
{
	guint notify_id;

	g_object_set_data (G_OBJECT (menu), "editor-window", window);
	
	/* Add a notify function */
	gconf_client_add_dir (gconf_client_get_default (), "/apps/gconf-editor/bookmarks",
			      GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
	notify_id = gconf_client_notify_add (gconf_client_get_default (), "/apps/gconf-editor/bookmarks",
					     gconf_bookmarks_key_changed, menu, NULL, NULL);
	g_object_set_data_full (G_OBJECT (menu), "notify-id", GINT_TO_POINTER (notify_id),
				remove_notify_id);

	gconf_bookmarks_update_menu (menu);

}
