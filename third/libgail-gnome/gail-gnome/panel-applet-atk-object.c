/*
 * LIBGAIL-GNOME -  Accessibility Toolkit Implementation for Bonobo
 * Copyright 2004 Sun Microsystems Inc.
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

#include <gdk/gdkkeysyms.h>

#include <libspi/remoteobject.h>

#include <bonobo/bonobo-exception.h>
#include "panel-applet-atk-object.h"
#include "gail-gnome-debug.h"

static gchar* applet_atk_priv = "applet-atk-private";
static gpointer parent_class = NULL;

typedef struct
{
  guint         action_idle_handler;
  GQueue        *action_queue;  
} PanelAppletAtkObjectPriv;

static void panel_applet_atk_object_class_init (PanelAppletAtkObjectClass *klass);
static void panel_applet_action_interface_init (AtkActionIface *iface);
static gboolean idle_do_action (gpointer data);

static gpointer
panel_applet_atk_object_private_create (PanelApplet *applet)
{
  /* zeroing bytes inits idle-handler and queue to 0, NULL */
  return g_new0 (PanelAppletAtkObjectPriv, 1);
}

static void
panel_applet_atk_object_private_destroy (PanelAppletAtkObjectPriv *priv)
{
  g_free (priv);
}

GType
panel_applet_atk_object_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static GTypeInfo tinfo =
      {
        0,
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) panel_applet_atk_object_class_init,
        (GClassFinalizeFunc) NULL,
        NULL, 0, 0,
        (GInstanceInitFunc) NULL,
      };

      /*
       * Figure out the size of the class and instance 
       * we are deriving from
       */
      AtkObjectFactory *factory;
      GType             derived_atk_type;
      GTypeQuery        query;

      static const GInterfaceInfo atk_action_info = {
	      (GInterfaceInitFunc) panel_applet_action_interface_init,
	      (GInterfaceFinalizeFunc) NULL,
	      NULL
      };

      factory = atk_registry_get_factory (atk_get_default_registry (), g_type_parent (PANEL_TYPE_APPLET));
      derived_atk_type = atk_object_factory_get_accessible_type (factory);
      g_type_query (derived_atk_type, &query);

      tinfo.class_size = query.class_size;
      tinfo.instance_size = query.instance_size;

      type = g_type_register_static (derived_atk_type, "PanelAppletAtkObject", &tinfo, 0) ;
      g_type_add_interface_static (type, ATK_TYPE_ACTION, &atk_action_info);
    }

  return type;
}

AtkObject *
panel_applet_atk_object_new (PanelApplet *applet)
{
  PanelAppletAtkObject *retval;
  PanelAppletAtkObjectPriv *applet_atk_object_priv;

  g_return_val_if_fail (PANEL_IS_APPLET (applet), NULL);

  retval = g_object_new (PANEL_APPLET_TYPE_ATK_OBJECT, NULL);

  atk_object_initialize (ATK_OBJECT (retval), GTK_WIDGET (applet));

  applet_atk_object_priv = panel_applet_atk_object_private_create (applet);
  g_object_set_data (G_OBJECT (retval), applet_atk_priv, applet_atk_object_priv);
  atk_object_set_role (ATK_OBJECT (retval), ATK_ROLE_EMBEDDED);

  return ATK_OBJECT (retval);
}

static void
panel_applet_atk_object_finalize (GObject *obj)
{
	gpointer priv;

	priv = g_object_get_data (obj, applet_atk_priv);
	if (priv) {
	      panel_applet_atk_object_private_destroy (priv);
	}
	G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static void
panel_applet_atk_object_class_init (PanelAppletAtkObjectClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	gobject_class->finalize = panel_applet_atk_object_finalize;
}

static gint
panel_applet_get_n_actions (AtkAction *action)
{
  return 2; /* activate, menu */
}

static gboolean
panel_applet_do_action (AtkAction *action,
			gint      i)
{
  GtkWidget *widget;
  gboolean return_value = TRUE;
  PanelAppletAtkObjectPriv *applet_atk_object_priv;

  widget = GTK_ACCESSIBLE (action)->widget;
  if (widget == NULL)
    /*
     * State is defunct
     */
  {
      return FALSE;
  }

  applet_atk_object_priv = g_object_get_data (G_OBJECT (action), applet_atk_priv);

  if (!GTK_WIDGET_VISIBLE (widget) || !applet_atk_object_priv)
  {
    return FALSE;
  }

  switch (i)
    {
    case 0:
    case 1:
      if (!applet_atk_object_priv->action_queue) 
	{
	  applet_atk_object_priv->action_queue = g_queue_new ();
	}
      g_queue_push_head (applet_atk_object_priv->action_queue, (gpointer) i);
      if (!applet_atk_object_priv->action_idle_handler) {
	applet_atk_object_priv->action_idle_handler = g_idle_add (idle_do_action, action);
      }
      break;
    default:
      return_value = FALSE;
      break;
    }
  return return_value; 
}

static gboolean
idle_do_action (gpointer data)
{
  PanelAppletAtkObject *applet_atk_object; 
  PanelAppletAtkObjectPriv *applet_atk_object_priv;
  GtkWidget *widget;

  applet_atk_object = PANEL_APPLET_ATK_OBJECT (data);
  applet_atk_object_priv = g_object_get_data (G_OBJECT (data), applet_atk_priv);

  if (!applet_atk_object_priv) return FALSE;

  applet_atk_object_priv->action_idle_handler = 0;

  widget = GTK_ACCESSIBLE (applet_atk_object)->widget;
  if (widget == NULL)
    /*
     * State is defunct
     */
    return FALSE;

  if (!GTK_WIDGET_VISIBLE (widget))
    return FALSE;

  while (!g_queue_is_empty (applet_atk_object_priv->action_queue)) 
    {
      GdkEvent tmp_event;
      gint action_number = (gint) g_queue_pop_head (applet_atk_object_priv->action_queue);
      switch (action_number)
	{
	case 0:
	    tmp_event.key.type = GDK_KEY_PRESS;
	    tmp_event.key.window = widget->window;
	    tmp_event.key.send_event = TRUE;
	    tmp_event.key.time = GDK_CURRENT_TIME;
	    tmp_event.key.state = 0;
	    tmp_event.key.keyval = GDK_space;
	    tmp_event.key.hardware_keycode = 0;
	    tmp_event.key.group = 0;
	    
	    break;
	case 1:
	  {
	    tmp_event.button.type = GDK_BUTTON_PRESS;
	    tmp_event.button.window = widget->window;
	    tmp_event.button.button = 3;
	    tmp_event.button.send_event = TRUE;
	    tmp_event.button.time = GDK_CURRENT_TIME;
	    tmp_event.button.axes = NULL;
	    
	    break;
	  }
	default:
	  g_assert_not_reached ();
	  break;
	}
      gtk_widget_event (widget, &tmp_event);
    }
  return FALSE; 
}

static G_CONST_RETURN gchar*
panel_applet_action_get_name (AtkAction *action,
			      gint      i)
{
  switch (i)
    {
    case 0:
      return "activate";
    case 1:
      return "menu";
    default:
      g_warning ("panel_applet_action_get_name: action number %d is out of range\n.", i);
      return "";
    }
}

static G_CONST_RETURN gchar*
panel_applet_action_get_description (AtkAction *action,
				     gint      i)
{
  /* TODO: supply strings marked for translation here */
  switch (i)
    {
    case 0:
      return "";
    case 1:
    default:
      return "";
    }
}

static void
panel_applet_action_interface_init (AtkActionIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->do_action = panel_applet_do_action;
  iface->get_n_actions = panel_applet_get_n_actions;
  iface->get_description = panel_applet_action_get_description;
  /*  iface->get_keybinding = panel_applet_action_get_keybinding; */
  iface->get_name = panel_applet_action_get_name;
  /* iface->set_description = panel_applet_action_set_description; */
}

