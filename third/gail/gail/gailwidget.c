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
#include "gailwidget.h"
#include "gailnotebookpage.h"
#include "gailcanvaswidget.h"

static void gail_widget_class_init (GailWidgetClass *klass);

static void gail_widget_connect_widget_destroyed (GtkAccessible    *accessible);
static void gail_widget_destroyed                (GtkWidget        *widget,
                                                  GtkAccessible    *accessible);

static G_CONST_RETURN gchar* gail_widget_get_name (AtkObject *accessible);
static G_CONST_RETURN gchar* gail_widget_get_description (AtkObject *accessible);
static AtkObject* gail_widget_get_parent (AtkObject *accessible);
static AtkStateSet* gail_widget_ref_state_set (AtkObject *accessible);
static gint gail_widget_get_index_in_parent (AtkObject *accessible);

static void atk_component_interface_init (AtkComponentIface *iface);

static guint    gail_widget_add_focus_handler
                                           (AtkComponent    *component,
                                            AtkFocusHandler handler);

static void     gail_widget_get_extents    (AtkComponent    *component,
                                            gint            *x,
                                            gint            *y,
                                            gint            *width,
                                            gint            *height,
                                            AtkCoordType    coord_type);

static void     gail_widget_get_size       (AtkComponent    *component,
                                            gint            *width,
                                            gint            *height);

static AtkLayer gail_widget_get_layer      (AtkComponent *component);

static gboolean gail_widget_grab_focus     (AtkComponent    *component);


static void     gail_widget_remove_focus_handler 
                                           (AtkComponent    *component,
                                            guint           handler_id);

static gboolean gail_widget_set_extents    (AtkComponent    *component,
                                            gint            x,
                                            gint            y,
                                            gint            width,
                                            gint            height,
                                            AtkCoordType    coord_type);

static gboolean gail_widget_set_position   (AtkComponent    *component,
                                            gint            x,
                                            gint            y,
                                            AtkCoordType    coord_type);

static gboolean gail_widget_set_size       (AtkComponent    *component,
                                            gint            width,
                                            gint            height);

#if 0
/*
 * We will get the parent from the widget structure rather than
 * use this function
 */
static GtkWidget* _gail_widget_get_parent        (GtkWidget     *widget);
#endif

static gint       gail_widget_map_gtk            (GtkWidget     *widget);
static void       gail_widget_real_notify_gtk    (GObject       *obj,
                                                  GParamSpec    *pspec);
static void       gail_widget_notify_gtk         (GObject       *obj,
                                                  GParamSpec    *pspec);
static gboolean   gail_widget_focus_gtk          (GtkWidget     *widget,
                                                  GdkEventFocus *event);
static gboolean   gail_widget_real_focus_gtk     (GtkWidget     *widget,
                                                  GdkEventFocus *event);

static void       gail_widget_focus_event        (AtkObject     *obj,
                                                  gboolean      focus_in);

static void       gail_widget_real_initialize    (AtkObject     *obj,
                                                  gpointer      data);
static GtkWidget* gail_widget_find_viewport      (GtkWidget     *widget);
static gboolean   gail_widget_on_screen          (GtkWidget     *widget);

static gpointer parent_class = NULL;

GType
gail_widget_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo tinfo =
      {
        sizeof (GailWidgetClass),
        (GBaseInitFunc) NULL, /* base init */
        (GBaseFinalizeFunc) NULL, /* base finalize */
        (GClassInitFunc) gail_widget_class_init, /* class init */
        (GClassFinalizeFunc) NULL, /* class finalize */
        NULL, /* class data */
        sizeof (GailWidget), /* instance size */
        0, /* nb preallocs */
        (GInstanceInitFunc) NULL, /* instance init */
        NULL /* value table */
      };

      static const GInterfaceInfo atk_component_info =
      {
        (GInterfaceInitFunc) atk_component_interface_init,
        (GInterfaceFinalizeFunc) NULL,
        NULL
      };

      type = g_type_register_static (GTK_TYPE_ACCESSIBLE,
                                     "GailWidget", &tinfo, 0);
      g_type_add_interface_static (type, ATK_TYPE_COMPONENT,
                                   &atk_component_info);
    }

  return type;
}

static void
gail_widget_class_init (GailWidgetClass *klass)
{
  AtkObjectClass *class = ATK_OBJECT_CLASS (klass);
  GtkAccessibleClass *accessible_class = GTK_ACCESSIBLE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  klass->notify_gtk = gail_widget_real_notify_gtk;
  klass->focus_gtk = gail_widget_real_focus_gtk;

  accessible_class->connect_widget_destroyed = gail_widget_connect_widget_destroyed;

  class->get_name = gail_widget_get_name;
  class->get_description = gail_widget_get_description;
  class->get_parent = gail_widget_get_parent;
  class->ref_state_set = gail_widget_ref_state_set;
  class->get_index_in_parent = gail_widget_get_index_in_parent;
  class->initialize = gail_widget_real_initialize;
}

/**
 * This function  specifies the GtkWidget for which the GailWidget was created 
 * and specifies a handler to be called when the GtkWidget is destroyed.
 **/
static void 
gail_widget_real_initialize (AtkObject *obj,
                             gpointer  data)
{
  GtkAccessible *accessible;
  GtkWidget *widget;

  g_return_if_fail (GTK_IS_WIDGET (data));

  widget = GTK_WIDGET (data);

  accessible = GTK_ACCESSIBLE (obj);
  accessible->widget = widget;
  gtk_accessible_connect_widget_destroyed (accessible);
  g_signal_connect_after (widget,
                          "focus-in-event",
                          G_CALLBACK (gail_widget_focus_gtk),
                          NULL);
  g_signal_connect_after (widget,
                          "focus-out-event",
                          G_CALLBACK (gail_widget_focus_gtk),
                          NULL);
  g_signal_connect (widget,
                    "notify",
                    G_CALLBACK (gail_widget_notify_gtk),
                    widget);
  atk_component_add_focus_handler (ATK_COMPONENT (accessible),
                                   gail_widget_focus_event);
  /*
   * Add signal handlers for GTK signals required to support property changes
   */
  g_signal_connect (widget,
                    "map",
                    G_CALLBACK (gail_widget_map_gtk),
                    NULL);
  g_signal_connect (widget,
                    "unmap",
                    G_CALLBACK (gail_widget_map_gtk),
                    NULL);
  g_object_set_data (G_OBJECT (obj), "atk-component-layer",
		     GINT_TO_POINTER (ATK_LAYER_WIDGET));
}

AtkObject* 
gail_widget_new (GtkWidget *widget)
{
  GObject *object;
  AtkObject *accessible;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

  object = g_object_new (GAIL_TYPE_WIDGET, NULL);

  accessible = ATK_OBJECT (object);
  atk_object_initialize (accessible, widget);

  accessible->role = ATK_ROLE_UNKNOWN;

  return accessible;
}

/*
 * This function specifies the function to be called when the widget
 * is destroyed
 */
static void
gail_widget_connect_widget_destroyed (GtkAccessible *accessible)
{
  if (accessible->widget)
    {
      g_signal_connect_after (accessible->widget,
                              "destroy",
                              G_CALLBACK (gail_widget_destroyed),
                              accessible);
    }
}

/*
 * This function is called when the widget is destroyed.
 * It sets the widget field in the GtkAccessible structure to NULL
 * and emits a state-change signal for the state ATK_STATE_DEFUNCT
 */
static void 
gail_widget_destroyed (GtkWidget     *widget,
                       GtkAccessible *accessible)
{
  accessible->widget = NULL;
  atk_object_notify_state_change (ATK_OBJECT (accessible), ATK_STATE_DEFUNCT,
                                  TRUE);
}

static G_CONST_RETURN gchar*
gail_widget_get_name (AtkObject *accessible)
{
  if (accessible->name)
    return accessible->name;
  else
    {
      /*
       * Get the widget name is it exists
       */
      GtkWidget* widget = GTK_ACCESSIBLE (accessible)->widget;

      if (widget == NULL)
        /*
         * State is defunct
         */
        return NULL;

      g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

      return widget->name;
    }
}

static G_CONST_RETURN gchar*
gail_widget_get_description (AtkObject *accessible)
{
  if (accessible->description)
    return accessible->description;
  else
    {
      /* Get the tooltip from the widget */
      GtkAccessible *obj = GTK_ACCESSIBLE (accessible);
      GtkTooltipsData *data;

      g_return_val_if_fail (obj, NULL);

      if (obj->widget == NULL)
        /*
         * Object is defunct
         */
        return NULL;
 
      g_return_val_if_fail (GTK_WIDGET (obj->widget), NULL);
    
      data = gtk_tooltips_data_get (obj->widget);
      if (data == NULL)
        return NULL;

      return data->tip_text;
    }
}

static AtkObject* 
gail_widget_get_parent (AtkObject *accessible)
{
  AtkObject *parent;

  parent = accessible->accessible_parent;

  if (parent != NULL)
    g_return_val_if_fail (ATK_IS_OBJECT (parent), NULL);
  else
    {
      GtkWidget *widget, *parent_widget;

      widget = GTK_ACCESSIBLE (accessible)->widget;
      if (widget == NULL)
        /*
         * State is defunct
         */
        return NULL;
      g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

      parent_widget = widget->parent;
      if (parent_widget == NULL)
        return NULL;

      parent = gtk_widget_get_accessible (parent_widget);
    }
  return parent;
}

static AtkStateSet*
gail_widget_ref_state_set (AtkObject *accessible)
{
  GtkWidget *widget = GTK_ACCESSIBLE (accessible)->widget;
  AtkStateSet *state_set;

  state_set = ATK_OBJECT_CLASS (parent_class)->ref_state_set (accessible);

  if (widget == NULL)
    {
      atk_state_set_add_state (state_set, ATK_STATE_DEFUNCT);
    }
  else
    {
      if (GTK_WIDGET_IS_SENSITIVE (widget))
        {
          atk_state_set_add_state (state_set, ATK_STATE_SENSITIVE);
          atk_state_set_add_state (state_set, ATK_STATE_ENABLED);
        }
  
      if (GTK_WIDGET_CAN_FOCUS (widget))
        {
          atk_state_set_add_state (state_set, ATK_STATE_FOCUSABLE);
        }
      /*
       * We do not currently generate notifications when an ATK object 
       * corresponding to a GtkWidget changes visibility by being scrolled 
       * on or off the screen.  The testcase for this is the main window 
       * of the testgtk application in which a set of buttons in a GtkVBox 
       * is in a scrooled window with a viewport.
       *
       * To generate the notifications we would need to do the following: 
       * 1) Find the GtkViewPort among the antecendents of the objects
       * 2) Create an accesible for the GtkViewPort
       * 3) Connect to the value-changed signal on the viewport
       * 4) When the signal is received we need to traverse the children 
       * of the viewport and check whether the children are visible or not 
       * visible; we may want to restrict this to the widgets for which 
       * accessible objects have been created.
       * 5) We probably need to store a variable on_screen in the 
       * GailWidget data structure so we can determine whether the value has 
       * changed.
       */
      if (gail_widget_on_screen (widget))
        {
          if (GTK_WIDGET_VISIBLE (widget))
            {
              atk_state_set_add_state (state_set, ATK_STATE_VISIBLE);

              if (GTK_WIDGET_MAPPED (widget))
                {
                  atk_state_set_add_state (state_set, ATK_STATE_SHOWING);
                }
            }
        }
  
      if (GTK_WIDGET_HAS_FOCUS (widget))
        {
          atk_state_set_add_state (state_set, ATK_STATE_FOCUSED);
        }
    }
  return state_set;
}

static gint
gail_widget_get_index_in_parent (AtkObject *accessible)
{
  GtkWidget *widget;
  GtkWidget *parent_widget;
  gint index;
  GList *children;

  widget = GTK_ACCESSIBLE (accessible)->widget;

  if (widget == NULL)
    /*
     * State is defunct
     */
    return -1;

  if (accessible->accessible_parent)
    {
      AtkObject *parent;

      parent = accessible->accessible_parent;

      if (GAIL_IS_NOTEBOOK_PAGE (parent) ||
          GAIL_IS_CANVAS_WIDGET (parent))
        return 0;
      else
        {
          gint n_children, i;
          gboolean found = FALSE;

          n_children = atk_object_get_n_accessible_children (parent);
          for (i = 0; i < n_children; i++)
            {
              AtkObject *child;

              child = atk_object_ref_accessible_child (parent, i);
              if (child == accessible)
                found = TRUE;

              g_object_unref (child); 
              if (found)
                return i;
            }
        }
    }

  g_return_val_if_fail (GTK_IS_WIDGET (widget), -1);
  parent_widget = widget->parent;
  if (parent_widget == NULL)
    return -1;
  g_return_val_if_fail (GTK_IS_CONTAINER (parent_widget), -1);

  children = gtk_container_get_children (GTK_CONTAINER (parent_widget));

  index = g_list_index (children, widget);
  g_list_free (children);
  return index;  
}

static void 
atk_component_interface_init (AtkComponentIface *iface)
{
  g_return_if_fail (iface != NULL);

  /*
   * Use default implementation for contains and get_position
   */
  iface->add_focus_handler = gail_widget_add_focus_handler;
  iface->get_extents = gail_widget_get_extents;
  iface->get_size = gail_widget_get_size;
  iface->get_layer = gail_widget_get_layer;
  iface->grab_focus = gail_widget_grab_focus;
  iface->remove_focus_handler = gail_widget_remove_focus_handler;
  iface->set_extents = gail_widget_set_extents;
  iface->set_position = gail_widget_set_position;
  iface->set_size = gail_widget_set_size;
}

static guint 
gail_widget_add_focus_handler (AtkComponent    *component,
                               AtkFocusHandler handler)
{
  GSignalMatchType match_type;
  gulong ret;
  guint signal_id;

  match_type = G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC;
  signal_id = g_signal_lookup ("focus-event", ATK_TYPE_OBJECT);

  ret = g_signal_handler_find (component, match_type, signal_id, 0, NULL,
                               (gpointer) handler, NULL);
  if (!ret)
    {
      return g_signal_connect_closure_by_id (component, 
                                             signal_id, 0,
                                             g_cclosure_new (
                                             G_CALLBACK (handler), NULL,
                                             (GClosureNotify) NULL),
                                             FALSE);
    }
  else
    {
      return 0;
    }
}

static void 
gail_widget_get_extents (AtkComponent   *component,
                         gint           *x,
                         gint           *y,
                         gint           *width,
                         gint           *height,
                         AtkCoordType   coord_type)
{
  GdkWindow *window;
  gint x_window, y_window;
  gint x_toplevel, y_toplevel;
  GtkWidget *widget = GTK_ACCESSIBLE (component)->widget;

  if (widget == NULL)
    /*
     * Object is defunct
     */
    return;

  g_return_if_fail (GTK_IS_WIDGET (widget));

  *width = widget->allocation.width;
  *height = widget->allocation.height;
  if (!gail_widget_on_screen (widget) || (!GTK_WIDGET_DRAWABLE (widget)))
    {
      *x = G_MININT;
      *y = G_MININT;
      return;
    }

  if (widget->parent)
    {
      *x = widget->allocation.x;
      *y = widget->allocation.y;
      window = gtk_widget_get_parent_window (widget);
    }
  else
    {
      *x = 0;
      *y = 0;
      window = widget->window;
    }
  gdk_window_get_origin (window, &x_window, &y_window);
  *x += x_window;
  *y += y_window;

 
 if (coord_type == ATK_XY_WINDOW) 
    { 
      window = gdk_window_get_toplevel (widget->window);
      gdk_window_get_origin (window, &x_toplevel, &y_toplevel);

      *x -= x_toplevel;
      *y -= y_toplevel;
    }
}

static void 
gail_widget_get_size (AtkComponent   *component,
                      gint           *width,
                      gint           *height)
{
  GtkWidget *widget = GTK_ACCESSIBLE (component)->widget;

  if (widget == NULL)
    /*
     * Object is defunct
     */
    return;

  g_return_if_fail (GTK_IS_WIDGET (widget));

  *width = widget->allocation.width;
  *height = widget->allocation.height;
}

static AtkLayer
gail_widget_get_layer (AtkComponent *component)
{
  gint layer;
  layer = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (component), "atk-component-layer"));

  return (AtkLayer) layer;
}

static gboolean 
gail_widget_grab_focus (AtkComponent   *component)
{
  GtkWidget *widget = GTK_ACCESSIBLE (component)->widget;
  GtkWidget *toplevel;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);
  if (GTK_WIDGET_CAN_FOCUS (widget))
    {
      gtk_widget_grab_focus (widget);
      toplevel = gtk_widget_get_toplevel (widget);
      if (GTK_WIDGET_TOPLEVEL (toplevel))
        gtk_window_present (GTK_WINDOW (toplevel)); 
      return TRUE;
    }
  else
    return FALSE;
}

static void 
gail_widget_remove_focus_handler (AtkComponent   *component,
                                  guint          handler_id)
{
  g_signal_handler_disconnect (component, handler_id);
}

static gboolean 
gail_widget_set_extents (AtkComponent   *component,
                         gint           x,
                         gint           y,
                         gint           width,
                         gint           height,
                         AtkCoordType   coord_type)
{
  GtkWidget *widget = GTK_ACCESSIBLE (component)->widget;

  if (widget == NULL)
    /*
     * Object is defunct
     */
    return FALSE;
  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

  if (GTK_WIDGET_TOPLEVEL (widget))
    {
      if (coord_type == ATK_XY_WINDOW)
        {
          gint x_current, y_current;
          GdkWindow *window = widget->window;

          gdk_window_get_origin (window, &x_current, &y_current);
          x_current += x;
          y_current += y;
          if (x_current < 0 || y_current < 0)
            return FALSE;
          else
            {
              gtk_widget_set_uposition (widget, x_current, y_current);
              gtk_widget_set_usize (widget, width, height);
              return TRUE;
            }
        }
      else if (coord_type == ATK_XY_SCREEN)
        {  
          gtk_widget_set_uposition (widget, x, y);
          gtk_widget_set_usize (widget, width, height);
          return TRUE;
        }
    }
  return FALSE;
}

static gboolean
gail_widget_set_position (AtkComponent   *component,
                          gint           x,
                          gint           y,
                          AtkCoordType   coord_type)
{
  GtkWidget *widget = GTK_ACCESSIBLE (component)->widget;

  if (widget == NULL)
    /*
     * Object is defunct
     */
    return FALSE;
  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

  if (GTK_WIDGET_TOPLEVEL (widget))
    {
      if (coord_type == ATK_XY_WINDOW)
        {
          gint x_current, y_current;
          GdkWindow *window = widget->window;

          gdk_window_get_origin (window, &x_current, &y_current);
          x_current += x;
          y_current += y;
          if (x_current < 0 || y_current < 0)
            return FALSE;
          else
            {
              gtk_widget_set_uposition (widget, x_current, y_current);
              return TRUE;
            }
        }
      else if (coord_type == ATK_XY_SCREEN)
        {  
          gtk_widget_set_uposition (widget, x, y);
          return TRUE;
        }
    }
  return FALSE;
}

static gboolean 
gail_widget_set_size (AtkComponent   *component,
                      gint           width,
                      gint           height)
{
  GtkWidget *widget = GTK_ACCESSIBLE (component)->widget;

  if (widget == NULL)
    /*
     * Object is defunct
     */
    return FALSE;
  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

  if (GTK_WIDGET_TOPLEVEL (widget))
    {
      gtk_widget_set_usize (widget, width, height);
      return TRUE;
    }
  else
   return FALSE;
}

#if 0
static GtkWidget*
_gail_widget_get_parent (GtkWidget* widget)
{
  GtkWidget *parent_widget;
  gpointer pointer;
  GValue value = { 0, };

  g_return_val_if_fail (GTK_WIDGET (widget), NULL);

  g_value_init (&value, G_TYPE_OBJECT);
  g_object_get_property (G_OBJECT (widget), "parent", &value);
  pointer = g_value_get_object (&value);
 
  g_return_val_if_fail (GTK_IS_WIDGET (pointer), NULL);

  return GTK_WIDGET (pointer);
}
#endif

/*
 * This function is a signal handler for notify_in_event and focus_out_event
 * signal which gets emitted on a GtkWidget.
 */
static gboolean
gail_widget_focus_gtk (GtkWidget     *widget,
                       GdkEventFocus *event)
{
  GailWidget *gail_widget;
  GailWidgetClass *klass;

  gail_widget = GAIL_WIDGET (gtk_widget_get_accessible (widget));
  klass = GAIL_WIDGET_GET_CLASS (gail_widget);
  if (klass->focus_gtk)
    return klass->focus_gtk (widget, event);
  else
    return FALSE;
}

/*
 * This function is the signal handler defined for focus_in_event and
 * focus_out_event got GailWidget.
 *
 * It emits a focus-event signal on the GailWidget.
 */
static gboolean
gail_widget_real_focus_gtk (GtkWidget     *widget,
                            GdkEventFocus *event)
{
  AtkObject* accessible;
  gboolean return_val;
  return_val = FALSE;

  accessible = gtk_widget_get_accessible (widget);
  g_signal_emit_by_name (accessible, "focus_event", event->in, &return_val);
  return FALSE;
}

/*
 * This function is the signal handler defined for map and unmap signals.
 */
static gint
gail_widget_map_gtk (GtkWidget     *widget)
{
  AtkObject* accessible;

  accessible = gtk_widget_get_accessible (widget);
  atk_object_notify_state_change (accessible, ATK_STATE_SHOWING,
                                  GTK_WIDGET_MAPPED (widget));
  return 1;
}

/*
 * This function is a signal handler for notify signal which gets emitted 
 * when a property changes value on the GtkWidget associated with the object.
 *
 * It calls a function for the GailWidget type
 */
static void 
gail_widget_notify_gtk (GObject     *obj,
                        GParamSpec  *pspec)
{
  GailWidget *widget;
  GailWidgetClass *klass;

  widget = GAIL_WIDGET (gtk_widget_get_accessible (GTK_WIDGET (obj)));
  klass = GAIL_WIDGET_GET_CLASS (widget);
  if (klass->notify_gtk)
    klass->notify_gtk (obj, pspec);
}

/*
 * This function is a signal handler for notify signal which gets emitted 
 * when a property changes value on the GtkWidget associated with a GailWidget.
 *
 * It constructs an AtkPropertyValues structure and emits a "property_changed"
 * signal which causes the user specified AtkPropertyChangeHandler
 * to be called.
 */
static void 
gail_widget_real_notify_gtk (GObject     *obj,
                             GParamSpec  *pspec)
{
  GtkWidget* widget = GTK_WIDGET (obj);
  AtkObject* atk_obj = gtk_widget_get_accessible (widget);
  AtkState state;
  gboolean value;

  if (strcmp (pspec->name, "has-focus") == 0)
    /*
     * We use focus-in-event and focus-out-event signals to catch
     * focus changes so we ignore this.
     */
    return;
  else if (strcmp (pspec->name, "visible") == 0)
    {
      state = ATK_STATE_VISIBLE;
      value = GTK_WIDGET_VISIBLE (widget);
    }
  else if (strcmp (pspec->name, "sensitive") == 0)
    {
      state = ATK_STATE_SENSITIVE;
      value = GTK_WIDGET_SENSITIVE (widget);
    }
  else
    return;

  atk_object_notify_state_change (atk_obj, state, value);
}

static void 
gail_widget_focus_event (AtkObject   *obj,
                         gboolean    focus_in)
{
  atk_object_notify_state_change (obj, ATK_STATE_FOCUSED, focus_in);
}

static GtkWidget*
gail_widget_find_viewport (GtkWidget *widget)
{
  /*
   * Find an antecedent which is a GtkViewPort
   */
  GtkWidget *parent;

  parent = widget->parent;
  while (parent != NULL)
    {
      if (GTK_IS_VIEWPORT (parent))
        break;
      parent = parent->parent;
    }
  return parent;
}

/*
 * This function checks whether the widget has an antecedent which is 
 * a GtkViewport and, if so, whether any part of the widget intersects
 * the visible rectangle of the GtkViewport.
 */ 
static gboolean gail_widget_on_screen (GtkWidget *widget)
{
  GtkWidget *viewport;
  gboolean return_value;

  viewport = gail_widget_find_viewport (widget);
  if (viewport)
    {
      GtkAdjustment *adjustment;
      GdkRectangle visible_rect;

      adjustment = gtk_viewport_get_vadjustment (GTK_VIEWPORT (viewport));
      visible_rect.y = adjustment->value;
      adjustment = gtk_viewport_get_hadjustment (GTK_VIEWPORT (viewport));
      visible_rect.x = adjustment->value;
      visible_rect.width = viewport->allocation.width;
      visible_rect.height = viewport->allocation.height;
             
      if (((widget->allocation.x + widget->allocation.width) < visible_rect.x) ||
         ((widget->allocation.y + widget->allocation.height) < visible_rect.y) ||
         (widget->allocation.x > (visible_rect.x + visible_rect.width)) ||
         (widget->allocation.y > (visible_rect.y + visible_rect.height)))
        return_value = FALSE;
      else
        return_value = TRUE;
    }
  else
    return_value = TRUE;

  return return_value;
}
