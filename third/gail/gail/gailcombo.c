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
#include "gailcombo.h"

static void         gail_combo_class_init              (GailComboClass *klass);
static void         gail_combo_object_init             (GailCombo      *combo);
static void         gail_combo_real_initialize         (AtkObject      *obj,
                                                        gpointer       data);

static void         gail_combo_selection_changed_gtk   (GtkWidget      *widget,
                                                        gpointer       data);

static gint         gail_combo_get_n_children          (AtkObject      *obj);
static AtkObject*   gail_combo_ref_child               (AtkObject      *obj,
                                                        gint           i);
static void         gail_combo_finalize                (GObject        *object);
static void         atk_action_interface_init          (AtkActionIface *iface);

static gboolean     gail_combo_do_action               (AtkAction      *action,
                                                        gint           i);
static gint         gail_combo_get_n_actions           (AtkAction      *action)
;
static G_CONST_RETURN gchar* gail_combo_get_description(AtkAction      *action,
                                                        gint           i);
static G_CONST_RETURN gchar* gail_combo_get_name       (AtkAction      *action,
                                                        gint           i);
static gboolean              gail_combo_set_description(AtkAction      *action,
                                                        gint           i,
                                                        const gchar    *desc);

static void         atk_selection_interface_init       (AtkSelectionIface *iface);
static gboolean     gail_combo_add_selection           (AtkSelection   *selection,
                                                        gint           i);
static gboolean     gail_combo_clear_selection         (AtkSelection   *selection);
static AtkObject*   gail_combo_ref_selection           (AtkSelection   *selection,
                                                        gint           i);
static gint         gail_combo_get_selection_count     (AtkSelection   *selection);
static gboolean     gail_combo_is_child_selected       (AtkSelection   *selection,
                                                        gint           i);
static gboolean     gail_combo_remove_selection        (AtkSelection   *selection,
                                                        gint           i);

static gint         _gail_combo_button_release         (gpointer       data);
static gint         _gail_combo_popup_release          (gpointer       data);

static gboolean     _gail_combo_is_entry_editable      (GtkWidget      *entry);

static gpointer parent_class = NULL;

GType
gail_combo_get_type (void)
{
  static GType type = 0;

  if (!type)
  {
    static const GTypeInfo tinfo =
    {
      sizeof (GailComboClass),
      (GBaseInitFunc) NULL, /* base init */
      (GBaseFinalizeFunc) NULL, /* base finalize */
      (GClassInitFunc) gail_combo_class_init, /* class init */
      (GClassFinalizeFunc) NULL, /* class finalize */
      NULL, /* class data */
      sizeof (GailCombo), /* instance size */
      0, /* nb preallocs */
      (GInstanceInitFunc) gail_combo_object_init, /* instance init */
      NULL /* value table */
    };

    static const GInterfaceInfo atk_action_info =
    {
        (GInterfaceInitFunc) atk_action_interface_init,
        (GInterfaceFinalizeFunc) NULL,
        NULL
    };
    static const GInterfaceInfo atk_selection_info =
    {
        (GInterfaceInitFunc) atk_selection_interface_init,
        (GInterfaceFinalizeFunc) NULL,
        NULL
    };

    type = g_type_register_static (GAIL_TYPE_CONTAINER,
                                   "GailCombo", &tinfo, 0);

    g_type_add_interface_static (type, ATK_TYPE_ACTION,
                                 &atk_action_info);
    g_type_add_interface_static (type, ATK_TYPE_SELECTION,
                                 &atk_selection_info);
  }

  return type;
}

static void
gail_combo_class_init (GailComboClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  AtkObjectClass *class = ATK_OBJECT_CLASS (klass);

  gobject_class->finalize = gail_combo_finalize;

  parent_class = g_type_class_peek_parent (klass);

  class->get_n_children = gail_combo_get_n_children;
  class->ref_child = gail_combo_ref_child;
  class->initialize = gail_combo_real_initialize;
}

static void
gail_combo_object_init (GailCombo      *combo)
{
  combo->press_description = NULL;
}

AtkObject* 
gail_combo_new (GtkWidget *widget)
{
  GObject *object;
  AtkObject *accessible;

  g_return_val_if_fail (GTK_IS_COMBO (widget), NULL);

  object = g_object_new (GAIL_TYPE_COMBO, NULL);

  accessible = ATK_OBJECT (object);
  atk_object_initialize (accessible, widget);

  accessible->role = ATK_ROLE_COMBO_BOX;

  return accessible;
}

static void
gail_combo_real_initialize (AtkObject *obj,
                            gpointer  data)
{
  GtkCombo *combo;
  GtkWidget *list, *entry;
  AtkObject *child_accessible;

  ATK_OBJECT_CLASS (parent_class)->initialize (obj, data);

  combo = GTK_COMBO (data);
  list = combo->list;
  entry = combo->entry;

  g_signal_connect (list,
                    "selection_changed",
                    G_CALLBACK (gail_combo_selection_changed_gtk),
                    obj);
  child_accessible = gtk_widget_get_accessible (entry);
  atk_object_set_parent (child_accessible, obj);
  child_accessible = gtk_widget_get_accessible (list);
  atk_object_set_parent (child_accessible, obj);
}

static void
gail_combo_selection_changed_gtk (GtkWidget      *widget,
                                  gpointer       data)
{
  g_signal_emit_by_name (data, "selection_changed");
}

static gboolean
_gail_combo_is_entry_editable (GtkWidget       *entry)
{
  GValue value = { 0, };

  g_value_init (&value, G_TYPE_BOOLEAN);
  g_object_get_property (G_OBJECT (entry), "editable", &value);
  return g_value_get_boolean (&value);
}

/*
 * The children of a GailCombo are the list of items and the entry field
 * if it is editable.
 */
static gint
gail_combo_get_n_children (AtkObject* obj)
{
  gint n_children = 1;
  GtkWidget *widget;

  g_return_val_if_fail (GAIL_IS_COMBO (obj), 0);

  widget = GTK_ACCESSIBLE (obj)->widget;
  if (widget == NULL)
  {
    /*
     * State is defunct
     */
    return 0;
  }
  if (_gail_combo_is_entry_editable (GTK_COMBO (widget)->entry))
    n_children++;

  return n_children;
}

static AtkObject*
gail_combo_ref_child (AtkObject *obj,
                      gint      i)
{
  AtkObject *accessible;
  GtkWidget *widget;

  g_return_val_if_fail (GAIL_IS_COMBO (obj), NULL);

  if (i < 0 || i > 1)
  {
    return NULL;
  }

  widget = GTK_ACCESSIBLE (obj)->widget;
  if (widget == NULL)
  {
    /*
     * State is defunct
     */
    return NULL;
  }

  if (i == 0)
  {
    accessible = gtk_widget_get_accessible (GTK_COMBO (widget)->list);
  }
  else if (_gail_combo_is_entry_editable (GTK_COMBO (widget)->entry))
  {
    accessible = gtk_widget_get_accessible (GTK_COMBO (widget)->entry);
  }
  else
    return NULL;

  g_object_ref (accessible);
  return accessible;
}

static void
atk_action_interface_init (AtkActionIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->do_action = gail_combo_do_action;
  iface->get_n_actions = gail_combo_get_n_actions;
  iface->get_description = gail_combo_get_description;
  iface->get_name = gail_combo_get_name;
  iface->set_description = gail_combo_set_description;
}

/*
 * This action is the pressing of the button on the combo box.
 * The behavior is different depending on whether the list is being
 * displayed or removed.
 *
 * A button press event is simulated on the appropriate widget and 
 * a button release event is simulated in an idle function.
 */
static gboolean
gail_combo_do_action (AtkAction *action,
                       gint      i)
{
  GtkCombo *combo;
  GtkWidget *action_widget;
  GtkWidget *widget;

  widget = GTK_ACCESSIBLE (action)->widget;
  if (widget == NULL)
  {
    /*
     * State is defunct
     */
    return FALSE;
  }

  if (!GTK_WIDGET_SENSITIVE (widget) || !GTK_WIDGET_VISIBLE (widget))
    return FALSE;

  combo = GTK_COMBO (widget);
  if (i == 0)
  {
    gboolean do_popup;
    GdkEvent tmp_event;

    do_popup = !GTK_WIDGET_MAPPED (combo->popwin);

    tmp_event.button.type = GDK_BUTTON_PRESS; 
    tmp_event.button.window = widget->window;
    tmp_event.button.button = 1; 
    tmp_event.button.send_event = TRUE;
    tmp_event.button.time = GDK_CURRENT_TIME;
    tmp_event.button.axes = NULL;

    if (do_popup)
    {
      /* Pop up list */
      action_widget = combo->button;

      gtk_widget_event (action_widget, &tmp_event);

      gtk_idle_add (_gail_combo_button_release, combo);
    }
    else
    {
      /* Pop down list */
      tmp_event.button.window = combo->list->window;
      gdk_window_set_user_data (combo->list->window, combo->button);
      action_widget = combo->popwin;
    
      gtk_widget_event (action_widget, &tmp_event);
      gtk_idle_add (_gail_combo_popup_release, combo);
    }
    return TRUE;
  }
  else
    return FALSE;
}


static gint
gail_combo_get_n_actions (AtkAction *action)
{
  /*
   * The default behavior of a combo box is to have one action -
   */
  return 1;
}

static G_CONST_RETURN gchar*
gail_combo_get_description (AtkAction *action,
                           gint      i)
{
  if (i == 0)
  {
    GailCombo *combo;

    combo = GAIL_COMBO (action);
    return combo->press_description;
  }
  else
  {
    return NULL;
  }
}

static G_CONST_RETURN gchar*
gail_combo_get_name (AtkAction *action,
                     gint      i)
{
  if (i == 0)
  {
    return "press";
  }
  else
    return NULL;
}

static void
atk_selection_interface_init (AtkSelectionIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->add_selection = gail_combo_add_selection;
  iface->clear_selection = gail_combo_clear_selection;
  iface->ref_selection = gail_combo_ref_selection;
  iface->get_selection_count = gail_combo_get_selection_count;
  iface->is_child_selected = gail_combo_is_child_selected;
  iface->remove_selection = gail_combo_remove_selection;
  /*
   * select_all_selection does not make sense for a combo box
   * so no implementation is provided.
   */
}

static gboolean
gail_combo_add_selection (AtkSelection   *selection,
                          gint           i)
{
  GtkCombo *combo;
  GtkWidget *widget;

  widget = GTK_ACCESSIBLE (selection)->widget;
  if (widget == NULL)
  {
    /*
     * State is defunct
     */
    return FALSE;
  }
  combo = GTK_COMBO (widget);

  gtk_list_select_item (GTK_LIST (combo->list), i);
  return TRUE;
}

static gboolean 
gail_combo_clear_selection (AtkSelection   *selection)
{
  GtkCombo *combo;
  GtkWidget *widget;

  widget = GTK_ACCESSIBLE (selection)->widget;
  if (widget == NULL)
  {
    /*
     * State is defunct
     */
    return FALSE;
  }
  combo = GTK_COMBO (widget);

  gtk_list_unselect_all (GTK_LIST (combo->list));
  return TRUE;
}

static AtkObject*
gail_combo_ref_selection (AtkSelection   *selection,
                          gint           i)
{
  GtkCombo *combo;
  GList * list;
  GtkWidget *item;
  AtkObject *obj;
  GtkWidget *widget;

  widget = GTK_ACCESSIBLE (selection)->widget;
  if (widget == NULL)
  {
    /*
     * State is defunct
     */
    return NULL;
  }
  combo = GTK_COMBO (widget);

  /*
   * A combo box can have only one selection.
   */
  if (i != 0)
    return NULL;

  list = GTK_LIST (combo->list)->selection;

  if (list == NULL)
    return NULL;

  item = GTK_WIDGET (list->data);

  obj = gtk_widget_get_accessible (item);
  g_object_ref (obj);
  return obj;
}

static gint
gail_combo_get_selection_count (AtkSelection   *selection)
{
  GtkCombo *combo;
  GList * list;
  GtkWidget *widget;

  widget = GTK_ACCESSIBLE (selection)->widget;
  if (widget == NULL)
  {
    /*
     * State is defunct
     */
    return 0;
  }
  combo = GTK_COMBO (widget);

  /*
   * The number of children currently selected is either 1 or 0 so we
   * do not bother to count the elements of the selected list.
   */
  list = GTK_LIST (combo->list)->selection;

  if (list == NULL)
    return 0;
  else
    return 1;
}

static gboolean
gail_combo_is_child_selected (AtkSelection   *selection,
                              gint           i)
{
  GtkCombo *combo;
  GList * list;
  GtkWidget *item;
  gint j;
  GtkWidget *widget;

  widget = GTK_ACCESSIBLE (selection)->widget;
  if (widget == NULL)
  {
    /*
     * State is defunct
     */
    return FALSE;
  }
  combo = GTK_COMBO (widget);

  list = GTK_LIST (combo->list)->selection;

  if (list == NULL)
    return FALSE;

  item = GTK_WIDGET (list->data);

  j = g_list_index (GTK_LIST (combo->list)->children, item);

  return (j == i);
}

static gboolean
gail_combo_remove_selection (AtkSelection   *selection,
                             gint           i)
{
  if (atk_selection_is_child_selected (selection, i))
  {
    atk_selection_clear_selection (selection);
  }    
  return TRUE;
}

static gint 
_gail_combo_popup_release (gpointer data)
{
  GtkCombo *combo;
  GtkWidget *action_widget;
  GdkEvent tmp_event;

  combo = GTK_COMBO (data);
  if (combo->current_button == 0)
    return FALSE;

  tmp_event.button.type = GDK_BUTTON_RELEASE; 
  tmp_event.button.button = 1; 
  tmp_event.button.time = GDK_CURRENT_TIME;
  action_widget = combo->button;

  gtk_widget_event (action_widget, &tmp_event);

  return FALSE;
}

static gint 
_gail_combo_button_release (gpointer data)
{
  GtkCombo *combo;
  GtkWidget *action_widget;
  GdkEvent tmp_event;

  combo = GTK_COMBO (data);
  if (combo->current_button == 0)
    return FALSE;

  tmp_event.button.type = GDK_BUTTON_RELEASE; 
  tmp_event.button.button = 1; 
  tmp_event.button.window = combo->list->window;
  tmp_event.button.time = GDK_CURRENT_TIME;
  gdk_window_set_user_data (combo->list->window, combo->button);
  action_widget = combo->list;

  gtk_widget_event (action_widget, &tmp_event);

  return FALSE;
}

static gboolean
gail_combo_set_description (AtkAction      *action,
                            gint           i,
                            const gchar    *desc)
{
  if (i == 0)
  {
    GailCombo *combo;

    combo = GAIL_COMBO (action);
    g_free (combo->press_description);
    combo->press_description = g_strdup (desc);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

static void
gail_combo_finalize (GObject            *object)
{
  GailCombo *combo = GAIL_COMBO (object);

  g_free (combo->press_description);
  G_OBJECT_CLASS (parent_class)->finalize (object);
}
