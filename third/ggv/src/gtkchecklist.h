/* gtkchecklist.h
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
 * GNOME2ized by jaKa Mocnik
 */

#ifndef __GTK_CHECK_LIST_H__
#define __GTK_CHECK_LIST_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_CHECK_LIST			(gtk_check_list_get_type ())
#define GTK_CHECK_LIST(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_CHECK_LIST, GtkCheckList))
#define GTK_CHECK_LIST_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_CHECK_LIST, GtkCheckListClass))
#define GTK_IS_CHECK_LIST(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_CHECK_LIST))
#define GTK_IS_CHECK_LIST_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((obj), GTK_TYPE_CHECK_LIST))
#define GTK_CHECK_LIST_GET_CLASS(obj)           (G_TYPE_INSTANCE_GET_CLASS(obj, GTK_TYPE_CHECK_LIST, GtkCheckListClass))

typedef struct _GtkCheckList       GtkCheckList;
typedef struct _GtkCheckListClass  GtkCheckListClass;

struct _GtkCheckList
{
  GtkTreeView parent;

  GtkListStore *list_model;
};

struct _GtkCheckListClass
{
  GtkTreeViewClass parent_class;
};


GType     gtk_check_list_get_type    (void);
GtkWidget *gtk_check_list_new        (void);
void      gtk_check_list_append_row  (GtkCheckList *check_list,
				      const gchar  *text);
void      gtk_check_list_toggle_row  (GtkCheckList *check_list,
				      gint          row);
void      gtk_check_list_set_active  (GtkCheckList *check_list,
				      gint          row,
				      gboolean      toggled);
gboolean  gtk_check_list_get_active  (GtkCheckList *check_list,
				      gint          row);
gint      *gtk_check_list_get_active_list(GtkCheckList *check_list);
void      gtk_check_list_clear(GtkCheckList *check_list);

G_END_DECLS

#endif /* __GTK_CHECK_LIST_H__ */
