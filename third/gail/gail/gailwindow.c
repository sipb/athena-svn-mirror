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
#include "gailwindow.h"
#include "gailtoplevel.h"

enum {
  ACTIVATE,
  CREATE,
  DEACTIVATE,
  DESTROY,
  MAXIMIZE,
  MINIMIZE,
  RESIZE,
  RESTORE,
  LAST_SIGNAL
};

static void gail_window_class_init (GailWindowClass *klass);

static void                  gail_window_real_initialize (AtkObject    *obj,
                                                          gpointer     data);

static G_CONST_RETURN gchar* gail_window_get_name       (AtkObject     *accessible);

static AtkObject*            gail_window_get_parent     (AtkObject     *accessible);
static gint                  gail_window_get_index_in_parent (AtkObject *accessible);
static gboolean              gail_window_real_focus_gtk (GtkWidget     *widget,
                                                         GdkEventFocus *event);

static AtkStateSet*          gail_window_ref_state_set  (AtkObject     *accessible);
static void                  gail_window_real_notify_gtk (GObject      *obj,
                                                          GParamSpec   *pspec);

static gboolean              gail_window_state_event_gtk (GtkWidget           *widget,
                                                          GdkEventWindowState *event);

/* atkcomponent.h */
static void                  atk_component_interface_init (AtkComponentIface    *iface);

static void                  gail_window_get_extents      (AtkComponent         *component,
                                                           gint                 *x,
                                                           gint                 *y,
                                                           gint                 *width,
                                                           gint                 *height,
                                                           AtkCoordType         coord_type);
static void                  gail_window_get_size         (AtkComponent         *component,
                                                           gint                 *width,
                                                           gint                 *height);

static guint gail_window_signals [LAST_SIGNAL] = { 0, };

static gpointer parent_class = NULL;

GType
gail_window_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo tinfo =
      {
        sizeof (GailWindowClass),
        (GBaseInitFunc) NULL, /* base init */
        (GBaseFinalizeFunc) NULL, /* base finalize */
        (GClassInitFunc) gail_window_class_init, /* class init */
        (GClassFinalizeFunc) NULL, /* class finalize */
        NULL, /* class data */
        sizeof (GailWindow), /* instance size */
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
      type = g_type_register_static (GAIL_TYPE_CONTAINER,
                                     "GailWindow", &tinfo, 0);

      g_type_add_interface_static (type, ATK_TYPE_COMPONENT,
                                   &atk_component_info);
    }

  return type;
}

static void
gail_window_class_init (GailWindowClass *klass)
{
  GailWidgetClass *widget_class;
  AtkObjectClass  *class = ATK_OBJECT_CLASS (klass);

  widget_class = (GailWidgetClass*)klass;
  widget_class->focus_gtk = gail_window_real_focus_gtk;
  widget_class->notify_gtk = gail_window_real_notify_gtk;

  parent_class = g_type_class_peek_parent (klass);

  class->get_name = gail_window_get_name;
  class->get_parent = gail_window_get_parent;
  class->get_index_in_parent = gail_window_get_index_in_parent;
  class->ref_state_set = gail_window_ref_state_set;
  class->initialize = gail_window_real_initialize;

  gail_window_signals [ACTIVATE] =
    g_signal_new ("activate",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  NULL, /* default signal handler */
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  gail_window_signals [CREATE] =
    g_signal_new ("create",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  NULL, /* default signal handler */
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  gail_window_signals [DEACTIVATE] =
    g_signal_new ("deactivate",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  NULL, /* default signal handler */
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  gail_window_signals [DESTROY] =
    g_signal_new ("destroy",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  NULL, /* default signal handler */
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  gail_window_signals [MAXIMIZE] =
    g_signal_new ("maximize",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  NULL, /* default signal handler */
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  gail_window_signals [MINIMIZE] =
    g_signal_new ("minimize",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  NULL, /* default signal handler */
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  gail_window_signals [RESIZE] =
    g_signal_new ("resize",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  NULL, /* default signal handler */
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  gail_window_signals [RESTORE] =
    g_signal_new ("restore",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  NULL, /* default signal handler */
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

AtkObject*
gail_window_new (GtkWidget *widget)
{
  GObject *object;
  AtkObject *accessible;

  g_return_val_if_fail (widget != NULL, NULL);
  /*
   * A GailWindow can be created for a GtkHandleBox or a GtkWindow
   */
  if (!GTK_IS_WINDOW (widget) &&
      !GTK_IS_HANDLE_BOX (widget))
    g_return_val_if_fail (FALSE, NULL);

  object = g_object_new (GAIL_TYPE_WINDOW, NULL);

  accessible = ATK_OBJECT (object);
  atk_object_initialize (accessible, widget);


  if (GTK_IS_FILE_SELECTION (widget))
    accessible->role = ATK_ROLE_FILE_CHOOSER;
  else if (GTK_IS_COLOR_SELECTION_DIALOG (widget))
    accessible->role = ATK_ROLE_COLOR_CHOOSER;
  else if (GTK_IS_FONT_SELECTION_DIALOG (widget))
    accessible->role = ATK_ROLE_FONT_CHOOSER;
  else if (GTK_IS_DIALOG (widget))
    accessible->role = ATK_ROLE_DIALOG;
  else if (GTK_IS_WINDOW (widget))
    accessible->role = ATK_ROLE_FRAME;
  else if (GTK_IS_HANDLE_BOX (widget))
    accessible->role = ATK_ROLE_UNKNOWN;
  else
    accessible->role = ATK_ROLE_INVALID;

  return accessible;
}

static void
gail_window_real_initialize (AtkObject *obj,
                             gpointer  data)
{
  ATK_OBJECT_CLASS (parent_class)->initialize (obj, data);

  g_signal_connect (data,
                    "window_state_event",
                    G_CALLBACK (gail_window_state_event_gtk),
                    NULL);
}

static G_CONST_RETURN gchar*
gail_window_get_name (AtkObject *accessible)
{
  if (accessible->name)
    return accessible->name;

  else
    {
      /*
       * Get the window title if it exists
       */
      GtkWidget* widget = GTK_ACCESSIBLE (accessible)->widget; 

      if (widget == NULL)
        /*
         * State is defunct
         */
        return NULL;

      g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

      if (GTK_IS_WINDOW (widget))
        {
          GtkWindow *window = GTK_WINDOW (widget);
          G_CONST_RETURN gchar* name;
 
          name = gtk_window_get_title (window);
          if (name)
            return name;
        }
      return ATK_OBJECT_CLASS (parent_class)->get_name (accessible);
    }
}

static AtkObject*
gail_window_get_parent (AtkObject *accessible)
{
  AtkObject* parent;

  parent = ATK_OBJECT_CLASS (parent_class)->get_parent (accessible);

  if (!parent)
    parent = atk_get_root ();

  return parent;
}

static gint
gail_window_get_index_in_parent (AtkObject *accessible)
{
  GtkWidget* widget = GTK_ACCESSIBLE (accessible)->widget; 
  AtkObject* atk_obj = atk_get_root ();
  GailToplevel* toplevel = GAIL_TOPLEVEL (atk_obj);
  gint index = -1;

  if (widget == NULL)
    /*
     * State is defunct
     */
    return -1;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), -1);

  index = ATK_OBJECT_CLASS (parent_class)->get_index_in_parent (accessible);
  if (index != -1)
    return index;

  if (GTK_IS_WINDOW (widget))
    {
      GtkWindow *window = GTK_WINDOW (widget);

      index = g_list_index (toplevel->window_list, window);
    }
  return index;
}

static gboolean
gail_window_real_focus_gtk (GtkWidget     *widget,
                            GdkEventFocus *event)
{
  AtkObject* obj;

  obj = gtk_widget_get_accessible (widget);
  atk_object_notify_state_change (obj, ATK_STATE_ACTIVE, event->in);

  return FALSE;
}

static AtkStateSet*
gail_window_ref_state_set (AtkObject *accessible)
{
  AtkStateSet *state_set;
  GtkWidget *widget;
  GtkWindow *window;
  GdkWindowState state;
  GValue value = { 0, };

  state_set = ATK_OBJECT_CLASS (parent_class)->ref_state_set (accessible);
  widget = GTK_ACCESSIBLE (accessible)->widget;
 
  if (widget == NULL)
    return state_set;

  window = GTK_WINDOW (widget);

  if (window->has_focus)
    atk_state_set_add_state (state_set, ATK_STATE_ACTIVE);

  if (widget->window)
    {
      state = gdk_window_get_state (widget->window);
      if (state & GDK_WINDOW_STATE_ICONIFIED)
        atk_state_set_add_state (state_set, ATK_STATE_ICONIFIED);
    } 
  if (gtk_window_get_modal (window))
    atk_state_set_add_state (state_set, ATK_STATE_MODAL);

  g_value_init (&value, G_TYPE_BOOLEAN);
  g_object_get_property (G_OBJECT (window), "resizable", &value);
  if (g_value_get_boolean (&value))
    atk_state_set_add_state (state_set, ATK_STATE_RESIZABLE);
 
  return state_set;
}

static void
gail_window_real_notify_gtk (GObject		*obj,
                             GParamSpec		*pspec)
{
  GtkWidget *widget = GTK_WIDGET (obj);
  AtkObject* atk_obj = gtk_widget_get_accessible (widget);

  if (strcmp (pspec->name, "title") == 0)
    {
      if (atk_obj->name == NULL)
        {
        /*
         * The title has changed so notify a change in accessible-name
         */
          g_object_notify (G_OBJECT (atk_obj), "accessible-name");
	}
      g_signal_emit_by_name (atk_obj, "visible_data_changed");
    }
  else
    GAIL_WIDGET_CLASS (parent_class)->notify_gtk (obj, pspec);
}

static gboolean
gail_window_state_event_gtk (GtkWidget           *widget,
                             GdkEventWindowState *event)
{
  AtkObject* obj;

  obj = gtk_widget_get_accessible (widget);
  atk_object_notify_state_change (obj, ATK_STATE_ICONIFIED,
                         (event->new_window_state & GDK_WINDOW_STATE_ICONIFIED) != 0);
  return FALSE;
}

static void
atk_component_interface_init (AtkComponentIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->get_extents = gail_window_get_extents;
  iface->get_size = gail_window_get_size;
}

static void
gail_window_get_extents (AtkComponent  *component,
                         gint          *x,
                         gint          *y,
                         gint          *width,
                         gint          *height,
                         AtkCoordType  coord_type)
{
  GtkWidget *widget = GTK_ACCESSIBLE (component)->widget; 
  GdkRectangle rect;
  gint x_toplevel, y_toplevel;

  if (widget == NULL)
    /*
     * State is defunct
     */
    return;

  g_return_if_fail (GTK_IS_WINDOW (widget));

  if (!GTK_WIDGET_TOPLEVEL (widget))
    {
      AtkComponentIface *parent_iface;

      parent_iface = (AtkComponentIface *) g_type_interface_peek_parent (ATK_COMPONENT_GET_IFACE (component));
      parent_iface->get_extents (component, x, y, width, height, coord_type);
      return;
    }

  gdk_window_get_frame_extents (widget->window, &rect);

  *width = rect.width;
  *height = rect.height;
  if (!GTK_WIDGET_DRAWABLE (widget))
    {
      *x = G_MININT;
      *y = G_MININT;
      return;
    }
  *x = rect.x;
  *y = rect.y;
  if (coord_type == ATK_XY_WINDOW)
    {
      gdk_window_get_origin (widget->window, &x_toplevel, &y_toplevel);
      *x -= x_toplevel;
      *y -= y_toplevel;
    }
}

static void
gail_window_get_size (AtkComponent *component,
                      gint         *width,
                      gint         *height)
{
  GtkWidget *widget = GTK_ACCESSIBLE (component)->widget; 
  GdkRectangle rect;

  if (widget == NULL)
    /*
     * State is defunct
     */
    return;

  g_return_if_fail (GTK_IS_WINDOW (widget));

  if (!GTK_WIDGET_TOPLEVEL (widget))
    {
      AtkComponentIface *parent_iface;

      parent_iface = (AtkComponentIface *) g_type_interface_peek_parent (ATK_COMPONENT_GET_IFACE (component));
      parent_iface->get_size (component, width, height);
      return;
    }
  gdk_window_get_frame_extents (widget->window, &rect);

  *width = rect.width;
  *height = rect.height;
}

