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
#include "gailitem.h"

static void                  gail_item_class_init      (GailItemClass *klass);
static G_CONST_RETURN gchar* gail_item_get_name        (AtkObject     *obj);
static gint                  gail_item_get_n_children  (AtkObject     *obj);
static AtkObject*            gail_item_ref_child       (AtkObject     *obj,
                                                        gint          i);
static void                  gail_item_real_initialize (AtkObject     *obj,
                                                        gpointer      data);

static GailContainerClass* parent_class = NULL;

GType
gail_item_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo tinfo =
      {
        sizeof (GailItemClass),
        (GBaseInitFunc) NULL, /* base init */
        (GBaseFinalizeFunc) NULL, /* base finalize */
        (GClassInitFunc) gail_item_class_init, /* class init */
        (GClassFinalizeFunc) NULL, /* class finalize */
        NULL, /* class data */
        sizeof (GailItem), /* instance size */
        0, /* nb preallocs */
        (GInstanceInitFunc) NULL, /* instance init */
        NULL /* value table */
      };

      type = g_type_register_static (GAIL_TYPE_CONTAINER,
                                     "GailItem", &tinfo, 0);
    }

  return type;
}

static void
gail_item_class_init (GailItemClass *klass)
{
  AtkObjectClass *class = ATK_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  class->get_name = gail_item_get_name;
  class->get_n_children = gail_item_get_n_children;
  class->ref_child = gail_item_ref_child;
  class->initialize = gail_item_real_initialize;
}

AtkObject*
gail_item_new (GtkWidget *widget)
{
  GObject *object;
  AtkObject *accessible;

  g_return_val_if_fail (GTK_IS_ITEM (widget), NULL);

  object = g_object_new (GAIL_TYPE_ITEM, NULL);

  accessible = ATK_OBJECT (object);
  atk_object_initialize (accessible, widget);

  accessible->role = ATK_ROLE_LIST_ITEM;

  return accessible;
}

static G_CONST_RETURN gchar*
gail_item_get_name (AtkObject *obj)
{
  g_return_val_if_fail (GAIL_IS_ITEM (obj), NULL);

  if (ATK_OBJECT(obj)->name != NULL)
    return ATK_OBJECT(obj)->name;
  else
    {
      /*
       * Get the label child; this should work for GtkMenuItem and GtkListItem.
       */
      GtkWidget *widget;
      GList *children;
      GList *tmp_list;

      widget = GTK_ACCESSIBLE (obj)->widget;
      if (widget == NULL)
        /*
         * State is defunct
         */
        return NULL;

      if (!GTK_IS_CONTAINER (widget))
        return NULL;
    
      children = gtk_container_get_children (GTK_CONTAINER (widget));
      tmp_list = children;
      while (tmp_list != NULL)
        {
          GtkWidget *widget = tmp_list->data;
   
          if (GTK_IS_LABEL (widget))
            {
              g_list_free (children);
	      return gtk_label_get_text (GTK_LABEL(widget));
            }
          tmp_list = tmp_list->next;
        }
      g_list_free (children);
      return NULL;
    }
}

/*
 * We report that this object has no children
 */

static gint
gail_item_get_n_children (AtkObject* obj)
{
  return 0;
}

static AtkObject*
gail_item_ref_child (AtkObject *obj,
                     gint      i)
{
  return NULL;
}
 
static void
gail_item_real_initialize (AtkObject *obj,
                           gpointer  data)
{
  guint handler_id;

  ATK_OBJECT_CLASS (parent_class)->initialize (obj, data);
  /*
   * As we report the item as having no children we are not interested
   * in add and remove signals
   */
  handler_id = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (obj), "gail-add-handler-id")); 
   g_signal_handler_disconnect (data, handler_id);
  handler_id = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (obj), "gail-remove-handler-id"));
  g_signal_handler_disconnect (data, handler_id);
}
