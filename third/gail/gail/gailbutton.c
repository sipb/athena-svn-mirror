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
#include "gailbutton.h"

static void                  gail_button_class_init       (GailButtonClass *klass);
static void                  gail_button_object_init      (GailButton      *button);

static G_CONST_RETURN gchar* gail_button_get_name         (AtkObject       *obj);
static gint                  gail_button_get_n_children   (AtkObject       *obj);
static AtkObject*            gail_button_ref_child        (AtkObject       *obj,
                                                           gint            i);
static AtkStateSet*          gail_button_ref_state_set    (AtkObject       *obj);
static void                  gail_button_real_notify_gtk  (GObject         *obj,
                                                           GParamSpec      *pspec);

static void                  gail_button_real_initialize  (AtkObject       *obj,
                                                           gpointer        data);
static void                  gail_button_finalize         (GObject        *object);

static void                  gail_button_pressed_enter_handler  (GtkWidget       *widget);
static void                  gail_button_released_leave_handler (GtkWidget       *widget);


static void                  atk_action_interface_init  (AtkActionIface *iface);
static gboolean              gail_button_do_action      (AtkAction      *action,
                                                         gint           i);
static gint                  gail_button_get_n_actions  (AtkAction      *action);
static G_CONST_RETURN gchar* gail_button_get_description(AtkAction      *action,
                                                         gint           i);
static G_CONST_RETURN gchar* gail_button_get_keybinding (AtkAction      *action,
                                                         gint           i);
static G_CONST_RETURN gchar* gail_button_action_get_name(AtkAction      *action,
                                                         gint           i);
static gboolean              gail_button_set_description(AtkAction      *action,
                                                         gint           i,
                                                         const gchar    *desc);


/* AtkImage.h */
static void                  atk_image_interface_init   (AtkImageIface  *iface);
static G_CONST_RETURN gchar* gail_button_get_image_description 
                                                        (AtkImage       *image);
static void	             gail_button_get_image_position
                                                        (AtkImage       *image,
                                                         gint	        *x,
                                                         gint	        *y,
                                                         AtkCoordType   coord_type);
static void                  gail_button_get_image_size (AtkImage       *image,
                                                         gint           *width,
                                                         gint           *height);
static gboolean              gail_button_set_image_description 
                                                        (AtkImage       *image,
                                                         const gchar    *description);
static GtkImage*             get_image_from_button      (GtkWidget      *button);
static GtkWidget*            get_label_from_button      (GtkWidget      *button);
static void                  set_role_for_button        (AtkObject      *accessible,
                                                         GtkWidget      *button);

static GailContainer* parent_class = NULL;

GType
gail_button_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo tinfo =
      {
        sizeof (GailButtonClass),
        (GBaseInitFunc) NULL, /* base init */
        (GBaseFinalizeFunc) NULL, /* base finalize */
        (GClassInitFunc) gail_button_class_init, /* class init */
        (GClassFinalizeFunc) NULL, /* class finalize */
        NULL, /* class data */
        sizeof (GailButton), /* instance size */
        0, /* nb preallocs */
        (GInstanceInitFunc) gail_button_object_init, /* instance init */
        NULL /* value table */
      };

      static const GInterfaceInfo atk_action_info =
      {
        (GInterfaceInitFunc) atk_action_interface_init,
        (GInterfaceFinalizeFunc) NULL,
        NULL
      };

      static const GInterfaceInfo atk_image_info =
      {
        (GInterfaceInitFunc) atk_image_interface_init,
        (GInterfaceFinalizeFunc) NULL,
        NULL
      };

      type = g_type_register_static (GAIL_TYPE_CONTAINER,
                                     "GailButton", &tinfo, 0);

      g_type_add_interface_static (type, ATK_TYPE_ACTION,
                                   &atk_action_info);
      g_type_add_interface_static (type, ATK_TYPE_IMAGE,
                                   &atk_image_info);
    }

  return type;
}

static void
gail_button_class_init (GailButtonClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  AtkObjectClass *class = ATK_OBJECT_CLASS (klass);
  GailWidgetClass *widget_class;

  widget_class = (GailWidgetClass*)klass;

  gobject_class->finalize = gail_button_finalize;

  parent_class = g_type_class_peek_parent (klass);

  class->get_name = gail_button_get_name;
  class->get_n_children = gail_button_get_n_children;
  class->ref_child = gail_button_ref_child;
  class->ref_state_set = gail_button_ref_state_set;
  class->initialize = gail_button_real_initialize;

  widget_class->notify_gtk = gail_button_real_notify_gtk;
}

static void
gail_button_object_init (GailButton      *button)
{
  button->click_description = NULL;
  button->press_description = NULL;
  button->release_description = NULL;
  button->click_keybinding = NULL;
}

AtkObject* 
gail_button_new (GtkWidget *widget)
{
  GObject *object;
  AtkObject *accessible;

  g_return_val_if_fail (GTK_IS_BUTTON (widget), NULL);

  object = g_object_new (GAIL_TYPE_BUTTON, NULL);

  accessible = ATK_OBJECT (object);
  atk_object_initialize (accessible, widget);

  set_role_for_button (accessible, widget);

  return accessible;
}

static G_CONST_RETURN gchar*
gail_button_get_name (AtkObject *obj)
{
  g_return_val_if_fail (GAIL_IS_BUTTON (obj), NULL);

  if (obj->name != NULL)
    return obj->name;
  else
    {
      /*
       * Get the text on the label
       */
      GtkWidget *widget;
      GtkWidget *child;
      G_CONST_RETURN gchar* name = NULL;

      widget = GTK_ACCESSIBLE (obj)->widget;
      if (widget == NULL)
        /*
         * State is defunct
         */
        return NULL;

      g_return_val_if_fail (GTK_IS_BUTTON (widget), NULL);

      child = get_label_from_button (widget);
      if (GTK_IS_LABEL (child))
        name = gtk_label_get_text (GTK_LABEL (child)); 

      return name;
    }
}

static void
gail_button_real_initialize (AtkObject *obj,
                             gpointer   data)
{
  GailButton *button = GAIL_BUTTON (obj);
  guint handler_id;

  ATK_OBJECT_CLASS (parent_class)->initialize (obj, data);

  /*
   * As we report the button as having no children we are not interested
   * in add and remove signals
   */
  handler_id = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (obj), "gail-add-handler-id"));
  g_signal_handler_disconnect (data, handler_id);
  handler_id = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT(obj), "gail-remove-handler-id"));
  g_signal_handler_disconnect (data, handler_id);

  button->state = GTK_STATE_NORMAL;

  g_signal_connect (data,
                    "pressed",
                    G_CALLBACK (gail_button_pressed_enter_handler),
                    NULL);
  g_signal_connect (data,
                    "enter",
                    G_CALLBACK (gail_button_pressed_enter_handler),
                    NULL);
  g_signal_connect (data,
                    "released",
                    G_CALLBACK (gail_button_released_leave_handler),
                    NULL);
  g_signal_connect (data,
                    "leave",
                    G_CALLBACK (gail_button_released_leave_handler),
                    NULL);
}

static void
gail_button_real_notify_gtk (GObject           *obj,
                             GParamSpec        *pspec)
{
  GtkWidget *widget = GTK_WIDGET (obj);
  AtkObject* atk_obj = gtk_widget_get_accessible (widget);

  if (strcmp (pspec->name, "label") == 0)
    {
      if (atk_obj->name == NULL)
      {
        /*
         * The label has changed so notify a change in accessible-name
         */
        g_object_notify (G_OBJECT (atk_obj), "accessible-name");
      }
      /*
       * The label is the only property which can be changed
       */
      g_signal_emit_by_name (atk_obj, "visible_data_changed");
    }
  else
    GAIL_WIDGET_CLASS (parent_class)->notify_gtk (obj, pspec);
}

static void
atk_action_interface_init (AtkActionIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->do_action = gail_button_do_action;
  iface->get_n_actions = gail_button_get_n_actions;
  iface->get_description = gail_button_get_description;
  iface->get_keybinding = gail_button_get_keybinding;
  iface->get_name = gail_button_action_get_name;
  iface->set_description = gail_button_set_description;
}

static gboolean
gail_button_do_action (AtkAction *action,
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
      gtk_widget_activate (widget);
      break;
    case 1:
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
    case 2:
      button->in_button = FALSE;
      gtk_button_leave (button);
      gtk_button_released (button);
      break;
    default:
      return_value = FALSE;
      break;
    }
  return return_value; 
}

static gint
gail_button_get_n_actions (AtkAction *action)
{
  return 3;
}

static G_CONST_RETURN gchar*
gail_button_get_description (AtkAction *action,
                             gint      i)
{
  GailButton *button;
  G_CONST_RETURN gchar *return_value;

  button = GAIL_BUTTON (action);

  switch (i)
    {
    case 0:
      return_value = button->click_description;
      break;
    case 1:
      return_value = button->press_description;
      break;
    case 2:
      return_value = button->release_description;
      break;
    default:
      return_value = NULL;
      break;
    }
  return return_value; 
}

static G_CONST_RETURN gchar*
gail_button_get_keybinding (AtkAction *action,
                            gint      i)
{
  GailButton *button;
  G_CONST_RETURN gchar *return_value = NULL;


  switch (i)
    {
    case 0:
      {
        /*
         * We look for a mnemonic on the label
         */
        GtkWidget *widget;
        GtkWidget *child;

        button = GAIL_BUTTON (action);
        widget = GTK_ACCESSIBLE (button)->widget;
        if (widget == NULL)
          /*
           * State is defunct
           */
          return NULL;

        g_return_val_if_fail (GTK_IS_BUTTON (widget), NULL);

        child = get_label_from_button (widget);
        if (GTK_IS_LABEL (child))
          {
            guint key_val; 

            key_val = gtk_label_get_mnemonic_keyval (GTK_LABEL (child)); 
            if (key_val != GDK_VoidSymbol)
              return_value = gtk_accelerator_name (key_val, 0);
            g_free (button->click_keybinding);
            button->click_keybinding = g_strdup (return_value);
          }
        break;
      }
    default:
      break;
    }
  return return_value; 
}

static G_CONST_RETURN gchar*
gail_button_action_get_name (AtkAction *action,
                             gint      i)
{
  G_CONST_RETURN gchar *return_value;

  switch (i)
    {
    case 0:
      /*
       * This action is a "click" to activate a button or "toggle" to change
       * the state of a toggle button check box or radio button.
       */ 
      return_value = "click";
      break;
    case 1:
      /*
       * This action simulates a button press by simulating moving the
       * mouse into the button followed by pressing the left mouse button.
       */
      return_value = "press";
      break;
    case 2:
      /*
       * This action simulates releasing the left mouse button outside the 
       * button.
       *
       * To simulate releasing the left mouse button inside the button use
       * the click action.
       */
      return_value = "release";
      break;
    default:
      return_value = NULL;
      break;
    }
  return return_value; 
}

static gboolean
gail_button_set_description (AtkAction      *action,
                             gint           i,
                             const gchar    *desc)
{
  GailButton *button;
  gchar **value;

  button = GAIL_BUTTON (action);

  switch (i)
    {
    case 0:
      value = &button->click_description;
      break;
    case 1:
      value = &button->press_description;
      break;
    case 2:
      value = &button->release_description;
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

/*
 * We report that this object has no children
 */

static gint
gail_button_get_n_children (AtkObject* obj)
{
  return 0;
}

static AtkObject*
gail_button_ref_child (AtkObject *obj,
                       gint      i)
{
  return NULL;
}

static AtkStateSet*
gail_button_ref_state_set (AtkObject *obj)
{
  AtkStateSet *state_set;
  GtkWidget *widget;
  GtkButton *button;

  state_set = ATK_OBJECT_CLASS (parent_class)->ref_state_set (obj);
  widget = GTK_ACCESSIBLE (obj)->widget;

  if (widget == NULL)
    return state_set;

  button = GTK_BUTTON (widget);

  if (GTK_WIDGET_STATE (widget) == GTK_STATE_ACTIVE)
    atk_state_set_add_state (state_set, ATK_STATE_ARMED);

  return state_set;
}

/*
 * This is the signal handler for the "pressed" or "enter" signal handler
 * on the GtkButton.
 *
 * If the state is now GTK_STATE_ACTIVE we notify a property change
 */
static void
gail_button_pressed_enter_handler (GtkWidget       *widget)
{
  AtkObject *accessible;

  if (GTK_WIDGET_STATE (widget) == GTK_STATE_ACTIVE)
    {
      accessible = gtk_widget_get_accessible (widget);
      atk_object_notify_state_change (accessible, ATK_STATE_ARMED, TRUE);
      GAIL_BUTTON (accessible)->state = GTK_STATE_ACTIVE;
    }
}

/*
 * This is the signal handler for the "released" or "leave" signal handler
 * on the GtkButton.
 *
 * If the state was GTK_STATE_ACTIVE we notify a property change
 */
static void
gail_button_released_leave_handler (GtkWidget       *widget)
{
  AtkObject *accessible;

  accessible = gtk_widget_get_accessible (widget);
  if (GAIL_BUTTON (accessible)->state == GTK_STATE_ACTIVE)
    {
      atk_object_notify_state_change (accessible, ATK_STATE_ARMED, FALSE);
      GAIL_BUTTON (accessible)->state = GTK_STATE_NORMAL;
    }
}

static void
atk_image_interface_init (AtkImageIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->get_image_description = gail_button_get_image_description;
  iface->get_image_position = gail_button_get_image_position;
  iface->get_image_size = gail_button_get_image_size;
  iface->set_image_description = gail_button_set_image_description;
}

static GtkImage*
get_image_from_button (GtkWidget *button)
{
  GtkWidget *child;
  GList *list;
  GtkImage *image = NULL;

  child = gtk_bin_get_child (GTK_BIN (button));
  if (GTK_IS_ALIGNMENT (child))
    child = gtk_bin_get_child (GTK_BIN (child));
  if (GTK_IS_CONTAINER (child))
    {
      list = gtk_container_get_children (GTK_CONTAINER (child));
      if (!list)
        return NULL;
      if (GTK_IS_IMAGE (list->data))
        image = GTK_IMAGE (list->data);
      g_list_free (list);
    }

  return image;
}

static G_CONST_RETURN gchar* 
gail_button_get_image_description (AtkImage *image) {

  GtkWidget *widget;
  GtkImage  *button_image;
  AtkObject *obj;

  widget = GTK_ACCESSIBLE (image)->widget;

  if (widget == NULL)
    /*
     * State is defunct
     */
    return NULL;

  button_image = get_image_from_button (widget);

  if (button_image != NULL)
    {
      obj = gtk_widget_get_accessible (GTK_WIDGET (button_image));
      return atk_image_get_image_description (ATK_IMAGE (obj));
    }
  else 
    return NULL;
}

static void
gail_button_get_image_position (AtkImage     *image,
                                gint	     *x,
                                gint	     *y,
                                AtkCoordType coord_type)
{
  GtkWidget *widget;
  GtkImage  *button_image;
  AtkObject *obj;

  widget = GTK_ACCESSIBLE (image)->widget;

  if (widget == NULL)
    {
    /*
     * State is defunct
     */
      *x = G_MININT;
      *y = G_MININT;
      return;
    }

  button_image = get_image_from_button (widget);

  if (button_image != NULL)
    {
      obj = gtk_widget_get_accessible (GTK_WIDGET (button_image));
      atk_component_get_position (ATK_COMPONENT (obj), x, y, coord_type); 
    }
  else
    {
      *x = G_MININT;
      *y = G_MININT;
    }
}

static void
gail_button_get_image_size (AtkImage *image,
                            gint     *width,
                            gint     *height)
{
  GtkWidget *widget;
  GtkImage  *button_image;
  AtkObject *obj;

  widget = GTK_ACCESSIBLE (image)->widget;

  if (widget == NULL)
    {
    /*
     * State is defunct
     */
      *width = -1;
      *height = -1;
      return;
    }

  button_image = get_image_from_button (widget);

  if (button_image != NULL)
    {
      obj = gtk_widget_get_accessible (GTK_WIDGET (button_image));
      atk_image_get_image_size (ATK_IMAGE (obj), width, height); 
    }
  else
    {
      *width = -1;
      *height = -1;
    }
}

static gboolean
gail_button_set_image_description (AtkImage    *image,
                                   const gchar *description)
{
  GtkWidget *widget;
  GtkImage  *button_image;
  AtkObject *obj;

  widget = GTK_ACCESSIBLE (image)->widget;

  if (widget == NULL)
    /*
     * State is defunct
     */
    return FALSE;

  button_image = get_image_from_button (widget);

  if (button_image != NULL) 
    {
      obj = gtk_widget_get_accessible (GTK_WIDGET (button_image));
      return atk_image_set_image_description (ATK_IMAGE (obj), description);
    }
  else 
    return FALSE;
}

static void
gail_button_finalize (GObject            *object)
{
  GailButton *button = GAIL_BUTTON (object);

  g_free (button->click_description);
  g_free (button->press_description);
  g_free (button->release_description);
  g_free (button->click_keybinding);
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GtkWidget*
get_label_from_button (GtkWidget *button)
{
  GtkWidget *child;

  child = gtk_bin_get_child (GTK_BIN (button));
  if (GTK_IS_ALIGNMENT (child))
    child = gtk_bin_get_child (GTK_BIN (child));
  while (GTK_IS_CONTAINER (child))
    {
      /*
       * Child is not a label.  Search for a label in the container's children.
       */
      GList *children, *tmp_list;
 
      children = gtk_container_get_children (GTK_CONTAINER (child));

      child = NULL;
      for (tmp_list = children; tmp_list != NULL; tmp_list = tmp_list->next) 
	{
	  if (GTK_IS_LABEL (tmp_list->data))
	    {
	      child = GTK_WIDGET (tmp_list->data);
	      break;
	    }  
           /*
            * Label for button which are GtkreeView column headers are in a 
            * GtkHBox in a GtkAlignment.
            */
          if (GTK_IS_ALIGNMENT (tmp_list->data))
            {
              child = gtk_bin_get_child (GTK_BIN (tmp_list->data));
              break;
	    }
	}

      g_list_free (children);
    }
  return child;
}

static void
set_role_for_button (AtkObject *accessible,
                     GtkWidget *button)
{
  GtkWidget *parent;
  AtkRole role;

  parent = gtk_widget_get_parent (button);
  if (GTK_IS_TREE_VIEW (parent))
    {
      role = ATK_ROLE_TABLE_COLUMN_HEADER;
      /*
       * Even though the accessible parent of the column header will
       * be reported as the table because the parent widget of the
       * GtkTreeViewColumn's button is the GtkTreeView we set
       * the accessible parent for column header to be the table
       * to ensure that atk_object_get_index_in_parent() returns
       * the correct value; see gail_widget_get_index_in_parent().
       */
      atk_object_set_parent (accessible, gtk_widget_get_accessible (parent));
    }
  else
    role = ATK_ROLE_PUSH_BUTTON;

  accessible->role =  role;
}

