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

#include <string.h>
#include <gtk/gtk.h>
#include "gailstatusbar.h"

static void         gail_statusbar_class_init          (GailStatusbarClass *klass);
static G_CONST_RETURN gchar* gail_statusbar_get_name   (AtkObject          *obj);
static gint         gail_statusbar_get_n_children      (AtkObject          *obj);
static AtkObject*   gail_statusbar_ref_child           (AtkObject          *obj,
                                                        gint               i);
static void         gail_statusbar_real_initialize     (AtkObject          *obj,
                                                        gpointer           data);

static gint        gail_statusbar_notify               (GObject            *obj,
                                                        GParamSpec         *pspec,
                                                        gpointer           user_data);

static GailContainerClass* parent_class = NULL;

GType
gail_statusbar_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo tinfo =
      {
        sizeof (GailStatusbarClass),
        (GBaseInitFunc) NULL, /* base init */
        (GBaseFinalizeFunc) NULL, /* base finalize */
        (GClassInitFunc) gail_statusbar_class_init, /* class init */
        (GClassFinalizeFunc) NULL, /* class finalize */
        NULL, /* class data */
        sizeof (GailStatusbar), /* instance size */
        0, /* nb preallocs */
        (GInstanceInitFunc) NULL, /* instance init */
        NULL /* value table */
      };

      type = g_type_register_static (GAIL_TYPE_CONTAINER,
                                     "GailStatusbar", &tinfo, 0);
    }
  return type;
}

static void
gail_statusbar_class_init (GailStatusbarClass *klass)
{
  AtkObjectClass  *class = ATK_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  class->get_name = gail_statusbar_get_name;
  class->get_n_children = gail_statusbar_get_n_children;
  class->ref_child = gail_statusbar_ref_child;
  class->initialize = gail_statusbar_real_initialize;
}

AtkObject* 
gail_statusbar_new (GtkWidget *widget)
{
  GObject *object;
  AtkObject *accessible;

  g_return_val_if_fail (GTK_IS_STATUSBAR (widget), NULL);

  object = g_object_new (GAIL_TYPE_STATUSBAR, NULL);

  accessible = ATK_OBJECT (object);
  atk_object_initialize (accessible, widget);

  accessible->role = ATK_ROLE_STATUSBAR;

  return accessible;
}

static G_CONST_RETURN gchar*
gail_statusbar_get_name (AtkObject *obj)
{
  g_return_val_if_fail (GAIL_IS_STATUSBAR (obj), NULL);

  if (ATK_OBJECT(obj)->name != NULL)
    return ATK_OBJECT(obj)->name;
  else
    {
      /*
       * Get the text on the label
       */
      GtkWidget *widget;
      GtkWidget *label;
      GValue  value = { 0, };

      widget = GTK_ACCESSIBLE (obj)->widget;
      if (widget == NULL)
        /*
         * State is defunct
         */
        return NULL;

     g_return_val_if_fail (GTK_IS_STATUSBAR (widget), NULL);
     label = GTK_STATUSBAR (widget)->label;
     g_return_val_if_fail (GTK_IS_LABEL (label), NULL);
    
     g_value_init (&value, G_TYPE_STRING);
     g_object_get_property (G_OBJECT (label), "label", &value);
     return g_value_get_string (&value);
   }
}

static gint
gail_statusbar_get_n_children (AtkObject          *obj)
{
  /*
   * We report a GailStatusbar as having no children
   */
  return 0;
}

static AtkObject*
gail_statusbar_ref_child (AtkObject          *obj,
                          gint               i)
{
  return NULL;
}

static void
gail_statusbar_real_initialize (AtkObject *obj,
                                gpointer  data)
{
  GailStatusbar *statusbar = GAIL_STATUSBAR (obj);
  GtkStatusbar *gtk_status_bar;
  guint handler_id;

  ATK_OBJECT_CLASS (parent_class)->initialize (obj, data);

  /*
   * As we report the statusbar as having no children we are not interested
   * in add and remove signals
   */
  handler_id = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (obj), "gail-add-handler-id"));
  g_signal_handler_disconnect (data, handler_id);
  handler_id = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (obj), "gail-remove-handler-id"));
  g_signal_handler_disconnect (data, handler_id);

  /*
   * We get notified of changes to the label
   */
  gtk_status_bar = GTK_STATUSBAR (data);
  g_signal_connect (G_OBJECT (gtk_status_bar->label),
                    "notify",
                    G_CALLBACK (gail_statusbar_notify),
                    statusbar);
}

static gint
gail_statusbar_notify (GObject    *obj, 
                       GParamSpec *pspec,
                       gpointer   user_data)
{
  AtkObject *atk_obj = ATK_OBJECT (user_data);

  if (strcmp (pspec->name, "label") == 0)
    if (atk_obj->name == NULL)
      /*
       * The label has changed so notify a change in accessible-name 
       */ 
      g_object_notify (G_OBJECT (atk_obj), "accessible-name");
  return 1;
}
