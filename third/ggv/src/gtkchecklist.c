/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* gtkchecklist.c
 * Copyright (C) 2001, 2002  Jonathan Blandford
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Based on CheckList.py from the eider module, written by Owen Taylor.
 */

/*
 * Completely GNOME2ized by jaKa Mocnik
 */

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "gtkchecklist.h"

static void gtk_check_list_init          (GtkCheckList      *check_list);
static void gtk_check_list_class_init    (GtkCheckListClass *class);

static GtkTreeViewClass *parent_class = NULL;

GType
gtk_check_list_get_type (void)
{
        static GType check_list_type = 0;

        if(!check_list_type) {
                static const GTypeInfo check_list_info = {
                        sizeof(GtkCheckListClass),
                        (GBaseInitFunc)NULL,
                        (GBaseFinalizeFunc)NULL,
                        (GClassInitFunc)gtk_check_list_class_init,
                        (GClassFinalizeFunc)NULL,
                        NULL,
                        sizeof(GtkCheckList),
                        0,
                        (GInstanceInitFunc)gtk_check_list_init,
                };

                check_list_type = g_type_register_static (GTK_TYPE_TREE_VIEW,
                                                          "GtkCheckList",
                                                          &check_list_info,
                                                          0);
        }

        return check_list_type;
}

static gboolean
key_press_event(GtkWidget *w, GdkEventKey *e)
{
        GtkCheckList *cl = GTK_CHECK_LIST(w);
        GtkTreeSelection *sel;
        GtkTreeIter iter;
        GtkTreeModel *model;
        GtkTreePath *path;

        if(e->state != 0 || e->type != GDK_KEY_PRESS) {
                if(GTK_WIDGET_CLASS(parent_class)->key_press_event)
                        (*GTK_WIDGET_CLASS(parent_class)->key_press_event)(w, e);
                return FALSE;
        }

        sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(cl));
        if(gtk_tree_selection_get_selected(sel, &model, &iter))
                path = gtk_tree_model_get_path(model, &iter);

        switch(e->keyval) {
        case GDK_KP_Page_Up:
        case GDK_Page_Up:
                if(gtk_tree_path_prev(path)) {
                        gtk_tree_view_set_cursor(GTK_TREE_VIEW(cl), path, NULL, FALSE);
                }
                gtk_tree_path_free(path);
                return TRUE;
        case GDK_KP_Page_Down:
        case GDK_Page_Down:
                gtk_tree_path_next(path);
                gtk_tree_view_set_cursor(GTK_TREE_VIEW(cl), path, NULL, FALSE);
                gtk_tree_path_free(path);
                return TRUE;
        default:
                gtk_tree_path_free(path);
                if(GTK_WIDGET_CLASS(parent_class)->key_press_event)
                        (*GTK_WIDGET_CLASS(parent_class)->key_press_event)(w, e);
                return FALSE;
        }
}

static void
gtk_check_list_init(GtkCheckList *cl)
{
}

static void
gtk_check_list_class_init (GtkCheckListClass *klass)
{
        GtkWidgetClass *wklass;

        wklass = GTK_WIDGET_CLASS(klass);
        wklass->key_press_event = key_press_event;

        parent_class = g_type_class_peek_parent(klass);
}

static void
toggle_renderer_toggled(GtkCellRendererToggle *cellrenderertoggle,
                        gchar *arg1,
                        gpointer user_data)
{
        GtkListStore *list;
        GtkTreeIter iter;
        GtkTreePath *path;
        gboolean val;

        list = GTK_LIST_STORE(user_data);
        path = gtk_tree_path_new_from_string(arg1);
        if(gtk_tree_model_get_iter(GTK_TREE_MODEL(list), &iter, path)) {
                gtk_tree_model_get(GTK_TREE_MODEL(list), &iter, 0, &val, -1);
                gtk_list_store_set(list, &iter, 0, !val, -1);
        }
        gtk_tree_path_free(path);
}

GtkWidget *
gtk_check_list_new (void)
{
        GtkCheckList *widget;
        GtkTreeViewColumn *column;
        GtkCellRenderer *renderer;

        widget = GTK_CHECK_LIST(g_object_new(GTK_TYPE_CHECK_LIST,
                                             "headers_visible", FALSE,
                                             NULL));

        widget->list_model = gtk_list_store_new(2, G_TYPE_BOOLEAN, G_TYPE_STRING);
        
        gtk_tree_view_set_model(GTK_TREE_VIEW(widget),
                                GTK_TREE_MODEL(widget->list_model));
        
        column = gtk_tree_view_column_new();
        renderer = gtk_cell_renderer_toggle_new();
        g_signal_connect(G_OBJECT(renderer), "toggled",
                         G_CALLBACK(toggle_renderer_toggled), widget->list_model);
        gtk_tree_view_column_pack_start(column, renderer, FALSE);
        gtk_tree_view_column_add_attribute(column, renderer, "active", 0);
        gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
        column = gtk_tree_view_column_new();
        renderer = gtk_cell_renderer_text_new();
        gtk_tree_view_column_pack_start(column, renderer, TRUE);
        gtk_tree_view_column_add_attribute(column, renderer, "text", 1);
        gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);

        return GTK_WIDGET(widget);
}

void
gtk_check_list_append_row (GtkCheckList *check_list,
			   const gchar  *text)
{
        GtkTreeIter iter;

        gtk_list_store_append(check_list->list_model, &iter);
        gtk_list_store_set(check_list->list_model, &iter, 0, FALSE, 1, text, -1);
}

void
gtk_check_list_clear(GtkCheckList *check_list)
{
        gtk_list_store_clear(check_list->list_model);
}

void
gtk_check_list_toggle_row (GtkCheckList *check_list,
			   gint          row)
{
        gchar str_path[16];
        GtkTreePath *path;
        gboolean val;
        GtkTreeIter iter;

        g_snprintf(str_path, 15, "%d", row);
        path = gtk_tree_path_new_from_string(str_path);
        if(gtk_tree_model_get_iter(GTK_TREE_MODEL(check_list->list_model),
                                   &iter, path)) {
                gtk_tree_model_get(GTK_TREE_MODEL(check_list->list_model),
                                   &iter, 0, &val, -1);
                gtk_list_store_set(check_list->list_model, &iter, 0, !val, -1);
        }        
        gtk_tree_path_free(path);
}

void
gtk_check_list_set_active (GtkCheckList *check_list,
                           gint          row,
                           gboolean      active)
{
        gchar str_path[16];
        GtkTreePath *path;
        GtkTreeIter iter;

        g_snprintf(str_path, 15, "%d", row);
        path = gtk_tree_path_new_from_string(str_path);
        if(gtk_tree_model_get_iter(GTK_TREE_MODEL(check_list->list_model),
                                   &iter, path))
                gtk_list_store_set(check_list->list_model, &iter, 0, active, -1);
        gtk_tree_path_free(path);
}

gboolean
gtk_check_list_get_active (GtkCheckList *check_list,
                           gint          row)
{
        gchar str_path[16];
        GtkTreePath *path;
        GtkTreeIter iter;
        gboolean retval;

        g_snprintf(str_path, 15, "%d", row);
        path = gtk_tree_path_new_from_string(str_path);
        if(gtk_tree_model_get_iter(GTK_TREE_MODEL(check_list->list_model),
                                   &iter, path))
                gtk_tree_model_get(GTK_TREE_MODEL(check_list->list_model),
                                   &iter, 0, &retval, -1);
        else
                retval = FALSE;
        gtk_tree_path_free(path);

        return retval;
}

gint *
gtk_check_list_get_active_list(GtkCheckList *check_list)
{
        GtkTreeIter iter;
        gint n, i, j;
        gint *retval;
        gboolean val;

        if(!gtk_tree_model_get_iter_root(GTK_TREE_MODEL(check_list->list_model),
                                         &iter)) {
                retval = g_new(gint, 1);
                retval[0] = -1;
                return retval;
        }

        n = 0;
        do {
                gtk_tree_model_get(GTK_TREE_MODEL(check_list->list_model),
                                   &iter, 0, &val, -1);                
                if(val)
                        n++;
        } while(gtk_tree_model_iter_next(GTK_TREE_MODEL(check_list->list_model),
                                         &iter));

        retval = g_new(gint, n + 1);
        gtk_tree_model_get_iter_root(GTK_TREE_MODEL(check_list->list_model),
                                     &iter);
        i = j = 0;
        while(i < n) {
                gtk_tree_model_get(GTK_TREE_MODEL(check_list->list_model),
                                   &iter, 0, &val, -1);                
                if(val)
                        retval[i++] = j;
                gtk_tree_model_iter_next(GTK_TREE_MODEL(check_list->list_model),
                                         &iter);
                j++;
        }

        retval[i] = -1;
        return retval;
}
