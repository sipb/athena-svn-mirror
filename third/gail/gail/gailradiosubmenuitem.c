/* GAIL - The GNOME Accessibility Implementation Library
 * Copyright 2002 Sun Microsystems Inc.
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
#include "gailradiosubmenuitem.h"

static void      gail_radio_sub_menu_item_class_init        (GailRadioSubMenuItemClass *klass);
static void      gail_radio_sub_menu_item_instance_init     (GailRadioSubMenuItem      *radio_menu_item);

static AtkRelationSet* gail_radio_sub_menu_item_ref_relation_set (AtkObject       *obj)
;

static GailCheckSubMenuItemClass *parent_class = NULL;

GType
gail_radio_sub_menu_item_get_type (void)
{
  static GType type = 0;

  if (!type)
  {
    static const GTypeInfo tinfo =
    {
      sizeof (GailRadioSubMenuItemClass),
      (GBaseInitFunc) NULL, /* base init */
      (GBaseFinalizeFunc) NULL, /* base finalize */
      (GClassInitFunc) gail_radio_sub_menu_item_class_init, /* class init */
      (GClassFinalizeFunc) NULL, /* class finalize */
      NULL, /* class data */
      sizeof (GailRadioSubMenuItem), /* instance size */
      0, /* nb preallocs */
      (GInstanceInitFunc) gail_radio_sub_menu_item_instance_init, /* instance init */
      NULL /* value table */
    };

    type = g_type_register_static (GAIL_TYPE_CHECK_SUB_MENU_ITEM,
                                   "GailRadioSubMenuItem", &tinfo, 0);
  }

  return type;
}

static void
gail_radio_sub_menu_item_class_init (GailRadioSubMenuItemClass *klass)
{
  AtkObjectClass *class = ATK_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  class->ref_relation_set = gail_radio_sub_menu_item_ref_relation_set;
}

AtkObject* 
gail_radio_sub_menu_item_new (GtkWidget *widget)
{
  GObject *object;
  AtkObject *accessible;

  g_return_val_if_fail (GTK_IS_RADIO_MENU_ITEM (widget), NULL);

  object = g_object_new (GAIL_TYPE_RADIO_SUB_MENU_ITEM, NULL);

  accessible = ATK_OBJECT (object);
  atk_object_initialize (accessible, widget);

  accessible->role = ATK_ROLE_RADIO_MENU_ITEM;
  return accessible;
}

static void
gail_radio_sub_menu_item_instance_init (GailRadioSubMenuItem *radio_menu_item)
{
  radio_menu_item->old_group = NULL;
}

AtkRelationSet*
gail_radio_sub_menu_item_ref_relation_set (AtkObject *obj)
{
  GtkWidget *widget;
  AtkRelationSet *relation_set;
  GSList *list;
  GailRadioSubMenuItem *radio_menu_item;

  g_return_val_if_fail (GAIL_IS_RADIO_SUB_MENU_ITEM (obj), NULL);

  widget = GTK_ACCESSIBLE (obj)->widget;
  if (widget == NULL)
  {
    /*
     * State is defunct
     */
    return NULL;
  }
  radio_menu_item = GAIL_RADIO_SUB_MENU_ITEM (obj);

  relation_set = ATK_OBJECT_CLASS (parent_class)->ref_relation_set (obj);

  /*
   * If the radio menu_item'group has changed remove the relation
   */
  list = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (widget));
  
  if (radio_menu_item->old_group != list)
    {
      AtkRelation *relation;

      relation = atk_relation_set_get_relation_by_type (relation_set, ATK_RELATION_MEMBER_OF);
      atk_relation_set_remove (relation_set, relation);
    }

  if (!atk_relation_set_contains (relation_set, ATK_RELATION_MEMBER_OF))
  {
    /*
     * Get the members of the menu_item group
     */

    radio_menu_item->old_group = list;
    if (list)
    {
      AtkObject **accessible_array;
      guint list_length;
      AtkRelation* relation;
      gint i = 0;

      list_length = g_slist_length (list);
      accessible_array = (AtkObject**) g_malloc (sizeof (AtkObject *) * 
                          list_length);
      while (list != NULL)
      {
        GtkWidget* list_item = list->data;

        accessible_array[i++] = gtk_widget_get_accessible (list_item);

        list = list->next;
      }
      relation = atk_relation_new (accessible_array, list_length,
                                   ATK_RELATION_MEMBER_OF);
      atk_relation_set_add (relation_set, relation);
      /*
       * Unref the relation so that it is not leaked.
       */
      g_object_unref (relation);
    }
  }
  return relation_set;
}