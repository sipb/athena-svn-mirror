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
#include <gdk/gdkkeysyms.h>
#include "gailoptionmenu.h"

static void                  gail_option_menu_class_init       (GailOptionMenuClass *klass);

static void                  atk_action_interface_init         (AtkActionIface  *iface);

static gboolean              gail_option_menu_do_action        (AtkAction       *action,
                                                                gint            i);
static gint                  gail_option_menu_get_n_actions    (AtkAction       *action);
static G_CONST_RETURN gchar* gail_option_menu_get_description  (AtkAction       *action,
                                                                gint            i);
static G_CONST_RETURN gchar* gail_option_menu_action_get_name  (AtkAction       *action,
                                                                gint            i);
static gboolean              gail_option_menu_set_description  (AtkAction       *action,
                                                                gint            i,
                                                                const gchar     *desc);

static GailButtonClass* parent_class = NULL;

GType
gail_option_menu_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo tinfo =
      {
        sizeof (GailOptionMenuClass),
        (GBaseInitFunc) NULL, /* base init */
        (GBaseFinalizeFunc) NULL, /* base finalize */
        (GClassInitFunc) gail_option_menu_class_init, /* class init */
        (GClassFinalizeFunc) NULL, /* class finalize */
        NULL, /* class data */
        sizeof (GailOptionMenu), /* instance size */
        0, /* nb preallocs */
        (GInstanceInitFunc) NULL, /* instance init */
        NULL /* value table */
      };
  
      static const GInterfaceInfo atk_action_info =
      {
        (GInterfaceInitFunc) atk_action_interface_init,
        (GInterfaceFinalizeFunc) NULL,
        NULL
      };

      type = g_type_register_static (GAIL_TYPE_BUTTON,
                                     "GailOptionMenu", &tinfo, 0);
      g_type_add_interface_static (type, ATK_TYPE_ACTION,
                                   &atk_action_info);
    }

  return type;
}

static void
gail_option_menu_class_init (GailOptionMenuClass *klass)
{
  parent_class = g_type_class_peek_parent (klass);
}

AtkObject* 
gail_option_menu_new (GtkWidget *widget)
{
  GObject *object;
  AtkObject *accessible;

  g_return_val_if_fail (GTK_IS_OPTION_MENU (widget), NULL);

  object = g_object_new (GAIL_TYPE_OPTION_MENU, NULL);

  accessible = ATK_OBJECT (object);
  atk_object_initialize (accessible, widget);

  accessible->role = ATK_ROLE_PUSH_BUTTON;

  return accessible;
}

static void
atk_action_interface_init (AtkActionIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->do_action = gail_option_menu_do_action;
  iface->get_n_actions = gail_option_menu_get_n_actions;
  iface->get_description = gail_option_menu_get_description;
  iface->get_name = gail_option_menu_action_get_name;
  iface->set_description = gail_option_menu_set_description;
}

static gboolean
gail_option_menu_do_action (AtkAction *action,
                            gint      i)
{
  GtkButton *button; 
  GtkWidget *widget;
  gboolean return_value = TRUE;

  widget = GTK_ACCESSIBLE (action)->widget;
  if (widget == NULL)
    /*
     * State is defunct
     */
    return FALSE;

  if (!GTK_WIDGET_SENSITIVE (widget) || !GTK_WIDGET_VISIBLE (widget))
    return FALSE;

  button = GTK_BUTTON (widget); 
  switch (i)
    {
    case 0:
      {
        GdkEvent tmp_event;

        button->in_button = TRUE;
        gtk_button_enter (button);
        /*
         * Simulate a button press event. calling gtk_button_pressed() does
         * not get the job done for a GtkOptionMenu.  
         */
        tmp_event.button.type = GDK_BUTTON_PRESS;
        tmp_event.button.window = widget->window;
        tmp_event.button.button = 1;
        tmp_event.button.send_event = TRUE;
        tmp_event.button.time = GDK_CURRENT_TIME;
        tmp_event.button.axes = NULL;

        gtk_widget_event (widget, &tmp_event);
        break;
      }
    default:
      return_value = FALSE;
      break;
    }
  return return_value; 
}

static gint
gail_option_menu_get_n_actions (AtkAction *action)
{
  return 1;
}

static G_CONST_RETURN gchar*
gail_option_menu_get_description (AtkAction *action,
                                  gint      i)
{
  GailButton *button;
  G_CONST_RETURN gchar *return_value;

  button = GAIL_BUTTON (action);

  switch (i)
    {
    case 0:
      return_value = button->press_description;
      break;
    default:
      return_value = NULL;
      break;
    }
  return return_value; 
}

static G_CONST_RETURN gchar*
gail_option_menu_action_get_name (AtkAction *action,
                                  gint      i)
{
  G_CONST_RETURN gchar *return_value;

  switch (i)
    {
    case 0:
      /*
       * This action simulates a button press by simulating moving the
       * mouse into the button followed by pressing the left mouse button.
       */
      return_value = "press";
      break;
    default:
      return_value = NULL;
      break;
  }
  return return_value; 
}

static gboolean
gail_option_menu_set_description (AtkAction      *action,
                                  gint           i,
                                  const gchar    *desc)
{
  GailButton *button;
  gchar **value;

  button = GAIL_BUTTON (action);

  switch (i)
    {
    case 0:
      value = &button->press_description;
      break;
    default:
      value = NULL;
      break;
    }

  if (value)
    {
      g_free (*value);
      *value = g_strdup (desc);
      return TRUE;
    }
  else
    return FALSE;
}
