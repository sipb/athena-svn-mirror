/* GAIL - The GNOME Accessibility Implementation Library
 * Copyright 2001 Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gtk/gtk.h>
#include "gailclistcell.h"

static void	 gail_clist_cell_class_init        (GailCListCellClass *klass);

static G_CONST_RETURN gchar* gail_clist_cell_get_name (AtkObject *accessible);

GType
gail_clist_cell_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo tinfo =
      {
        sizeof (GailCListCellClass),
        (GBaseInitFunc) NULL, /* base init */
        (GBaseFinalizeFunc) NULL, /* base finalize */
        (GClassInitFunc) gail_clist_cell_class_init, /* class init */
        (GClassFinalizeFunc) NULL, /* class finalize */
        NULL, /* class data */
        sizeof (GailCListCell), /* instance size */
        0, /* nb preallocs */
        (GInstanceInitFunc) NULL, /* instance init */
        NULL /* value table */
      };

      type = g_type_register_static (GAIL_TYPE_CELL,
                                     "GailCListCell", &tinfo, 0);
    }
  return type;
}

static void	 
gail_clist_cell_class_init (GailCListCellClass *klass)
{
  AtkObjectClass *class = ATK_OBJECT_CLASS (klass);

  class->get_name = gail_clist_cell_get_name;
}

AtkObject* 
gail_clist_cell_new (void)
{
  GObject *object;
  AtkObject *atk_object;

  object = g_object_new (GAIL_TYPE_CLIST_CELL, NULL);

  g_return_val_if_fail (object != NULL, NULL);

  atk_object = ATK_OBJECT (object);
  atk_object->role = ATK_ROLE_TABLE_CELL;

  g_return_val_if_fail (!ATK_IS_TEXT (atk_object), NULL);
  
  return atk_object;
}

static G_CONST_RETURN gchar*
gail_clist_cell_get_name (AtkObject *accessible)
{
  if (accessible->name)
    return accessible->name;
  else
    {
      /*
       * Get the cell's text if it exists
       */
      GailCell *cell = GAIL_CELL (accessible);
      GtkWidget* widget = cell->widget;
      GtkCList *clist;
      gchar *text = NULL;
      gint row, column;

      if (widget == NULL)
        /*
         * State is defunct
         */
        return NULL;
 
      clist = GTK_CLIST (widget);
      g_return_val_if_fail (clist->columns, NULL);
      row = cell->index / clist->columns;
      column = cell->index % clist->columns;
      gtk_clist_get_text (clist, row, column, &text);
      return text;
    }
}