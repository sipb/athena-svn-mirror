/* GAIL - The GNOME Accessibility Enabling Library
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
#include "gaillist.h"
#include "gailcombo.h"

static void         gail_list_class_init            (GailListClass  *klass); 

static gint         gail_list_get_index_in_parent   (AtkObject      *accessible);


static GailContainerClass *parent_class = NULL;

GType
gail_list_get_type (void)
{
  static GType type = 0;

  if (!type)
  {
    static const GTypeInfo tinfo =
    {
      sizeof (GailListClass),
      (GBaseInitFunc) NULL, /* base init */
      (GBaseFinalizeFunc) NULL, /* base finalize */
      (GClassInitFunc) gail_list_class_init, /* class init */
      (GClassFinalizeFunc) NULL, /* class finalize */
      NULL, /* class data */
      sizeof (GailList), /* instance size */
      0, /* nb preallocs */
      (GInstanceInitFunc) NULL, /* instance init */
      NULL /* value table */
    };

    type = g_type_register_static (GAIL_TYPE_CONTAINER,
                                   "GailList", &tinfo, 0);
  }
  return type;
}

static void
gail_list_class_init (GailListClass *klass)
{
  AtkObjectClass  *class = ATK_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  class->get_index_in_parent = gail_list_get_index_in_parent;
}

AtkObject* 
gail_list_new (GtkWidget *widget)
{
  GObject *object;
  AtkObject *accessible;

  g_return_val_if_fail (GTK_IS_LIST (widget), NULL);

  object = g_object_new (GAIL_TYPE_LIST, NULL);

  accessible = ATK_OBJECT (object);
  atk_object_initialize (accessible, widget);

  accessible->role = ATK_ROLE_LIST;

  return accessible;
}

static gint
gail_list_get_index_in_parent (AtkObject *accessible)
{
  /*
   * If the parent widget is a combo box then the index is 0
   * otherwise do the normal thing.
   */
  if (accessible->accessible_parent)
  {
    if (GAIL_IS_COMBO (accessible->accessible_parent))
      return 0;
  }
  return ATK_OBJECT_CLASS (parent_class)->get_index_in_parent (accessible);
}

