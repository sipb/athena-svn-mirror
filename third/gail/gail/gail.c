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

#include <stdio.h>
#include <stdlib.h>
#include <atk/atk.h>
#include <gtk/gtk.h>
#include "gail.h"
#include "gailfactory.h"

static gboolean gail_focus_watcher      (GSignalInvocationHint *ihint,
                                         guint                  n_param_values,
                                         const GValue          *param_values,
                                         gpointer               data);
static gboolean gail_select_watcher     (GSignalInvocationHint *ihint,
                                         guint                  n_param_values,
                                         const GValue          *param_values,
                                         gpointer               data);
static gboolean gail_deselect_watcher   (GSignalInvocationHint *ihint,
                                         guint                  n_param_values,
                                         const GValue          *param_values,
                                         gpointer               data);
static gboolean gail_switch_page_watcher(GSignalInvocationHint *ihint,
                                         guint                  n_param_values,
                                         const GValue          *param_values,
                                         gpointer               data);
static AtkObject* gail_get_accessible_for_widget (GtkWidget    *widget,
                                                  gboolean     *transient);
static void     gail_finish_select       (GtkWidget            *widget);
static void     gail_map_cb              (GtkWidget            *widget);
static void     gail_show_cb             (GtkWidget            *widget);
static gint     gail_focus_idle_handler  (gpointer             data);
static void     gail_focus_notify        (GtkWidget            *widget);
static void     gail_focus_notify_when_idle (GtkWidget            *widget);

static void     gail_focus_tracker_init ();

static GtkWidget* focus_widget = NULL;
static GtkWidget* next_focus_widget = NULL;
static GtkWidget* focus_before_menu = NULL;
static guint focus_notify_handler = 0;    

GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_WIDGET, gail_widget, gail_widget_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_CONTAINER, gail_container, gail_container_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_BUTTON, gail_button, gail_button_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_ITEM, gail_item, gail_item_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_MENU_ITEM, gail_menu_item, gail_menu_item_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_TOGGLE_BUTTON, gail_toggle_button, gail_toggle_button_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_IMAGE, gail_image, gail_image_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_TEXT_VIEW, gail_text_view, gail_text_view_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_COMBO, gail_combo, gail_combo_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_ENTRY, gail_entry, gail_entry_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_MENU_SHELL, gail_menu_shell, gail_menu_shell_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_MENU, gail_menu, gail_menu_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_WINDOW, gail_window, gail_window_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_RANGE, gail_range, gail_range_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_CLIST, gail_clist, gail_clist_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_LABEL, gail_label, gail_label_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_STATUSBAR, gail_statusbar, gail_statusbar_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_NOTEBOOK, gail_notebook, gail_notebook_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_CALENDAR, gail_calendar, gail_calendar_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_PROGRESS_BAR, gail_progress_bar, gail_progress_bar_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_SPIN_BUTTON, gail_spin_button, gail_spin_button_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_TREE_VIEW, gail_tree_view, gail_tree_view_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_FRAME, gail_frame, gail_frame_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_RADIO_BUTTON, gail_radio_button, gail_radio_button_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_ARROW, gail_arrow, gail_arrow_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_PIXMAP, gail_pixmap, gail_pixmap_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_SEPARATOR, gail_separator, gail_separator_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_BOX, gail_box, gail_box_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_SCROLLED_WINDOW, gail_scrolled_window, gail_scrolled_window_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_LIST, gail_list, gail_list_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_PANED, gail_paned, gail_paned_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_SCROLLBAR, gail_scrollbar, gail_scrollbar_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_OPTION_MENU, gail_option_menu, gail_option_menu_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_CANVAS, gail_canvas, gail_canvas_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_CHECK_MENU_ITEM, gail_check_menu_item, gail_check_menu_item_new)
GAIL_ACCESSIBLE_FACTORY (GAIL_TYPE_RADIO_MENU_ITEM, gail_radio_menu_item, gail_radio_menu_item_new)

static AtkObject*
gail_get_accessible_for_widget (GtkWidget *widget,
                                gboolean  *transient)
{
  AtkObject *obj = NULL;

  *transient = FALSE;
  if (!widget)
    return NULL;

  if (GTK_IS_ENTRY (widget))
    {
      GtkWidget *other_widget = widget->parent;
      if (GTK_IS_COMBO (other_widget))
        widget = other_widget;
    } 
  else if (GTK_IS_NOTEBOOK (widget)) 
    {
      GtkNotebook *notebook;
      gint page_num = -1;

      notebook = GTK_NOTEBOOK (widget);
      /*
       * Report the currently focused tab rather than the currently selected tab
       */
      if (notebook->focus_tab)
        {
          page_num = g_list_index (notebook->children, notebook->focus_tab->data);
        }
      if (page_num != -1)
        {
          obj = gtk_widget_get_accessible (widget);
          obj = atk_object_ref_accessible_child (obj, page_num);
          g_object_unref (obj);
        }
    }
  else if (GTK_IS_TREE_VIEW (widget)) 
    {
      obj = gail_tree_view_ref_focus_cell (GTK_TREE_VIEW (widget));
      if (obj)
        *transient = TRUE;
    }
  else if (GNOME_IS_CANVAS (widget)) 
    {
      GnomeCanvas *canvas;

      canvas = GNOME_CANVAS (widget);

      if (canvas->focused_item)
        {
          obj = atk_gobject_accessible_for_object (G_OBJECT (canvas->focused_item));
        }
    }
  if (obj == NULL)
    {
      AtkObject *focus_object;

      obj = gtk_widget_get_accessible (widget);
      focus_object = g_object_get_data (G_OBJECT (obj), "gail-focus-object");
      if (focus_object)
        obj = focus_object;
    }

  return obj;
}

static gboolean
gail_focus_watcher (GSignalInvocationHint *ihint,
                    guint                  n_param_values,
                    const GValue          *param_values,
                    gpointer               data)
{
  GObject *object;
  GtkWidget *widget;
  GdkEvent *event;

  object = g_value_get_object (param_values + 0);
  g_return_val_if_fail (GTK_IS_WIDGET(object), FALSE);

  event = g_value_get_boxed (param_values + 1);
  widget = GTK_WIDGET (object);

  if (event->type == GDK_FOCUS_CHANGE) 
    {
      if (event->focus_change.in)
        {
          if (GTK_IS_WINDOW (widget))
            {
              GtkWindow *window;

              window = GTK_WINDOW (widget);
              if (window->focus_widget)
                {
                  /*
                   * If we already have a potential focus widget set this
                   * windows's focus widget to focus_before_menu so that 
                   * it will be reported when menu item is unset.
                   */
                  if (next_focus_widget)
                    {
                      if (GTK_IS_MENU_ITEM (next_focus_widget) &&
                          !focus_before_menu)
                        {
                          focus_before_menu = window->focus_widget;
                          g_object_add_weak_pointer (G_OBJECT (focus_before_menu), (gpointer *)&focus_before_menu);
                        }

                      return TRUE;
                    }
                  widget = window->focus_widget;
                }
              else if (window->type == GTK_WINDOW_POPUP)
                {
                  GtkWidget *child = gtk_bin_get_child (GTK_BIN (widget));

                  if (GTK_IS_WIDGET (child) && GTK_WIDGET_HAS_GRAB (child))
                    {
                      if (GTK_IS_MENU_SHELL (child))
                        {
                          if (GTK_MENU_SHELL (child)->active_menu_item)
                            {
                              /*
                               * We have a menu which has a menu item selected
                               * so we do not report focus on the menu.
                               */ 
                              return TRUE; 
                            }
                        }
                      widget = child;
                    } 
                }
            }
        }
      else
        {
          /* focus out */
          widget = NULL;
        }
    }
  else
    {
      if (event->type == GDK_MOTION_NOTIFY && GTK_WIDGET_HAS_FOCUS (widget))
        {
          if (widget == focus_widget)
            {
              return TRUE;
            }
        }
      else
        {
          return TRUE;
        }
    }
  /*
   * The widget may not yet be visible on the screen so we wait until it is.
   */
  gail_focus_notify_when_idle (widget);
  return TRUE; 
}

static gboolean
gail_select_watcher (GSignalInvocationHint *ihint,
                     guint                  n_param_values,
                     const GValue          *param_values,
                     gpointer               data)
{
  GObject *object;
  GtkWidget *widget;

  object = g_value_get_object (param_values + 0);
  g_return_val_if_fail (GTK_IS_WIDGET(object), FALSE);

  widget = GTK_WIDGET (object);

  if (!GTK_WIDGET_MAPPED (widget))
    {
      g_signal_connect (widget, "map",
                        G_CALLBACK (gail_map_cb),
                        NULL);
    }
  else
    gail_finish_select (widget);

  return TRUE;
}

static void
gail_finish_select (GtkWidget *widget)
{
  if (GTK_IS_MENU_ITEM (widget))
    {
      GtkMenuItem* menu_item;

      menu_item = GTK_MENU_ITEM (widget);
      if (menu_item->submenu &&
          !GTK_WIDGET_VISIBLE (menu_item->submenu))
        {
          /*
           * If the submenu is not visble, wait until it is before
           * reporting focus on the menu item.
           */
          gulong handler_id;

          handler_id = g_signal_handler_find (menu_item->submenu,
                                              G_SIGNAL_MATCH_FUNC,
                                              g_signal_lookup ("show",
                                                               GTK_TYPE_WINDOW),
                                              0,
                                              NULL,
                                              (gpointer) gail_show_cb,
                                              NULL); 
          if (!handler_id)
            g_signal_connect (menu_item->submenu, "show",
                              G_CALLBACK (gail_show_cb),
                              NULL);

          /*
           * If we are waiting to report focus on a menubar or a menu item
           * because of a previous deselect, cancel it.
           */
          if (focus_notify_handler &&
              next_focus_widget &&
              (GTK_IS_MENU_BAR (next_focus_widget) ||
               GTK_IS_MENU_ITEM (next_focus_widget)))
            {
              gtk_idle_remove (focus_notify_handler);
              g_object_remove_weak_pointer (G_OBJECT (next_focus_widget), (gpointer *)&next_focus_widget);
              focus_notify_handler = 0;
            }
          return;
        }
    } 
  /*
   * If previously focused widget is not a GtkMenuItem or a GtkMenu,
   * keep track of it so we can return to it after menubar is deactivated
   */
  if (focus_widget && 
      !GTK_IS_MENU_ITEM (focus_widget) && 
      !GTK_IS_MENU (focus_widget))
    {
      focus_before_menu = focus_widget;
      g_object_add_weak_pointer (G_OBJECT (focus_before_menu), (gpointer *)&focus_before_menu);

    } 
  gail_focus_notify_when_idle (widget);

  return; 
}

static void
gail_map_cb (GtkWidget *widget)
{
  gail_finish_select (widget);
}

static void
gail_show_cb (GtkWidget *widget)
{
  if (GTK_IS_MENU (widget))
    {
      if (GTK_MENU (widget)->parent_menu_item)
        gail_finish_select (GTK_MENU (widget)->parent_menu_item);
    }
}


static gboolean
gail_deselect_watcher (GSignalInvocationHint *ihint,
                       guint                  n_param_values,
                       const GValue          *param_values,
                       gpointer               data)
{
  GObject *object;
  GtkWidget *widget;
  GtkWidget *menu_shell;

  object = g_value_get_object (param_values + 0);
  g_return_val_if_fail (GTK_IS_WIDGET(object), FALSE);

  widget = GTK_WIDGET (object);

  if (!GTK_IS_MENU_ITEM (widget))
    return TRUE;

  menu_shell = gtk_widget_get_parent (widget);
  if (GTK_IS_MENU_SHELL (menu_shell))
    {
      GtkWidget *parent_menu_shell;

      parent_menu_shell = GTK_MENU_SHELL (menu_shell)->parent_menu_shell;
      if (parent_menu_shell)
        {
          GtkWidget *active_menu_item;

          active_menu_item = GTK_MENU_SHELL (parent_menu_shell)->active_menu_item;
          if (active_menu_item)
            {
              gail_focus_notify_when_idle (active_menu_item);
            }
        }
      else
        {
          gail_focus_notify_when_idle (menu_shell);
        }
    }
  return TRUE; 
}

static gboolean 
gail_switch_page_watcher (GSignalInvocationHint *ihint,
                          guint                  n_param_values,
                          const GValue          *param_values,
                          gpointer               data)
{
  GObject *object;
  GtkWidget *widget;
  GtkNotebook *notebook;

  object = g_value_get_object (param_values + 0);
  g_return_val_if_fail (GTK_IS_WIDGET(object), FALSE);

  widget = GTK_WIDGET (object);

  if (!GTK_IS_NOTEBOOK (widget))
    return TRUE;

  notebook = GTK_NOTEBOOK (widget);
  if (!notebook->focus_tab)
    return TRUE;

  gail_focus_notify_when_idle (widget);
  return TRUE;
}


static gint
gail_focus_idle_handler (gpointer data)
{
  focus_notify_handler = 0;
  /*
   * The widget which was to receive focus may have been removed
   */
  if (!next_focus_widget)
    {
      if (next_focus_widget != data)
        return FALSE;
    }
  else
    {
      g_object_remove_weak_pointer (G_OBJECT (next_focus_widget), (gpointer *)&next_focus_widget);
      next_focus_widget = NULL;
    }
    
  gail_focus_notify (data);

  return FALSE; 
}

static void
gail_focus_notify (GtkWidget *widget)
{
  AtkObject *atk_obj;
  gboolean transient;

  if (widget != focus_widget)
    {
      if (focus_widget)
        {
          g_object_remove_weak_pointer (G_OBJECT (focus_widget), (gpointer *)&focus_widget);
        }
      focus_widget = widget;
      if (focus_widget)
        {
          g_object_add_weak_pointer (G_OBJECT (focus_widget), (gpointer *)&focus_widget);
          /*
           * The UI may not have been updated yet; e.g. in gtkhtml2
           * html_view_layout() is called in a idle handler
           */
          if (focus_widget == focus_before_menu)
            {
              g_object_remove_weak_pointer (G_OBJECT (focus_before_menu), (gpointer *)&focus_before_menu);
              focus_before_menu = NULL;
            }
        }
      gail_focus_notify_when_idle (focus_widget);
    }
  else
    {
      if (focus_widget)
        atk_obj  = gail_get_accessible_for_widget (focus_widget, &transient);
      else
        atk_obj = NULL;
      atk_focus_tracker_notify (atk_obj);
      if (atk_obj && transient)
        g_object_unref (atk_obj);
    }
}

static void
gail_focus_notify_when_idle (GtkWidget *widget)
{
  if (focus_notify_handler)
    {
      if (widget)
        {
          gtk_idle_remove (focus_notify_handler);
          if (next_focus_widget)
            g_object_remove_weak_pointer (G_OBJECT (next_focus_widget), (gpointer *)&next_focus_widget);
        }
      else
        /*
         * Ignore if focus is being set to NULL and we are waiting to set focus
         */
        return;
    }

  if (widget)
    {
      next_focus_widget = widget;
      g_object_add_weak_pointer (G_OBJECT (next_focus_widget), (gpointer *)&next_focus_widget);
    }
  focus_notify_handler = gtk_idle_add (gail_focus_idle_handler, widget);
}

static gboolean
gail_deactivate_watcher (GSignalInvocationHint *ihint,
                         guint                  n_param_values,
                         const GValue          *param_values,
                         gpointer               data)
{
  GObject *object;
  GtkWidget *widget;
  GtkMenuShell *shell;
  GtkWidget *focus = NULL;

  object = g_value_get_object (param_values + 0);
  g_return_val_if_fail (GTK_IS_WIDGET(object), FALSE);
  widget = GTK_WIDGET (object);

  g_return_val_if_fail (GTK_IS_MENU_SHELL(widget), TRUE);
  shell = GTK_MENU_SHELL(widget);
  if (!shell->parent_menu_shell)
    focus = focus_before_menu;
      
  gail_focus_notify_when_idle (focus);

  return TRUE; 
}

static void
gail_focus_tracker_init ()
{
  static gboolean  emission_hooks_added = FALSE;

  if (!emission_hooks_added)
    {
      /*
       * We cannot be sure that the classes exist so we make sure that they do.
       */
      gtk_type_class (GTK_TYPE_WIDGET);
      gtk_type_class (GTK_TYPE_ITEM);
      gtk_type_class (GTK_TYPE_MENU_SHELL);
      gtk_type_class (GTK_TYPE_NOTEBOOK);

      /*
       * We listen for event_after signal and then check that the
       * event was a focus in event so we get called after the event.
       */
      g_signal_add_emission_hook (
             g_signal_lookup ("event-after", GTK_TYPE_WIDGET), 0,
             gail_focus_watcher, NULL, (GDestroyNotify) NULL);
      /*
       * A "select" signal is emitted when arrow key is used to
       * move to a list item in the popup window of a GtkCombo or
       * a menu item in a menu.
       */
      g_signal_add_emission_hook (
             g_signal_lookup ("select", GTK_TYPE_ITEM), 0,
             gail_select_watcher, NULL, (GDestroyNotify) NULL);

      /*
       * A "deselect" signal is emitted when arrow key is used to
       * move from a menu item in a menu to the parent menu.
       */
      g_signal_add_emission_hook (
             g_signal_lookup ("deselect", GTK_TYPE_ITEM), 0,
             gail_deselect_watcher, NULL, (GDestroyNotify) NULL);

      /*
       * We listen for deactivate signals on menushells to determine
       * when the "focus" has left the menus.
       */
      g_signal_add_emission_hook (
             g_signal_lookup ("deactivate", GTK_TYPE_MENU_SHELL), 0,
             gail_deactivate_watcher, NULL, (GDestroyNotify) NULL);

      /*
       * We listen for "switch-page" signal on a GtkNotebook to notify
       * when page has changed because of clicking on a notebook tab.
       */
      g_signal_add_emission_hook (
             g_signal_lookup ("switch-page", GTK_TYPE_NOTEBOOK), 0,
             gail_switch_page_watcher, NULL, (GDestroyNotify) NULL);
      emission_hooks_added = TRUE;
    }
}

/*
 *   These exported symbols are hooked by gnome-program
 * to provide automatic module initialization and shutdown.
 */
extern void gnome_accessibility_module_init     (void);
extern void gnome_accessibility_module_shutdown (void);

static int gail_initialized = FALSE;

static void
gail_accessibility_module_init (void)
{
  if (gail_initialized)
    {
      return;
    }
  gail_initialized = TRUE;

  fprintf (stderr, "GTK Accessibility Module initialized\n");

  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_WIDGET, gail_widget);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_CONTAINER, gail_container);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_BUTTON, gail_button);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_ITEM, gail_item);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_MENU_ITEM, gail_menu_item);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_TOGGLE_BUTTON, gail_toggle_button);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_IMAGE, gail_image);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_TEXT_VIEW, gail_text_view);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_COMBO, gail_combo);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_ENTRY, gail_entry);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_MENU_BAR, gail_menu_shell);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_MENU, gail_menu);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_WINDOW, gail_window);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_HANDLE_BOX, gail_window);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_RANGE, gail_range);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_CLIST, gail_clist);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_LABEL, gail_label);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_STATUSBAR, gail_statusbar);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_NOTEBOOK, gail_notebook);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_CALENDAR, gail_calendar);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_PROGRESS_BAR, gail_progress_bar);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_SPIN_BUTTON, gail_spin_button);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_TREE_VIEW, gail_tree_view);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_FRAME, gail_frame);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_CELL_RENDERER_TEXT, gail_text_cell);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_CELL_RENDERER_TOGGLE, gail_boolean_cell);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_CELL_RENDERER_PIXBUF, gail_image_cell);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_RADIO_BUTTON, gail_radio_button);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_ARROW, gail_arrow);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_PIXMAP, gail_pixmap);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_SEPARATOR, gail_separator);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_BOX, gail_box);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_SCROLLED_WINDOW, gail_scrolled_window);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_LIST, gail_list);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_PANED, gail_paned);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_SCROLLBAR, gail_scrollbar);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_OPTION_MENU, gail_option_menu);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_CHECK_MENU_ITEM, gail_check_menu_item);
  GAIL_WIDGET_SET_FACTORY (GTK_TYPE_RADIO_MENU_ITEM, gail_radio_menu_item);

  /* LIBGNOMECANVAS SUPPORT */
  GAIL_WIDGET_SET_FACTORY (GNOME_TYPE_CANVAS, gail_canvas);
  GAIL_WIDGET_SET_FACTORY (GNOME_TYPE_CANVAS_GROUP, gail_canvas_group);
  GAIL_WIDGET_SET_FACTORY (GNOME_TYPE_CANVAS_TEXT, gail_canvas_text);
  GAIL_WIDGET_SET_FACTORY (GNOME_TYPE_CANVAS_RICH_TEXT, gail_canvas_text);
  GAIL_WIDGET_SET_FACTORY (GNOME_TYPE_CANVAS_WIDGET, gail_canvas_widget);
  GAIL_WIDGET_SET_FACTORY (GNOME_TYPE_CANVAS_ITEM, gail_canvas_item);

  atk_focus_tracker_init (gail_focus_tracker_init);

  /* Initialize the GailUtility class */
  g_type_class_unref (g_type_class_ref (GAIL_TYPE_UTIL));
}

/**
 * gnome_accessibility_module_init:
 * @void: 
 * 
 *   This method is invoked by name from libgnome's
 * gnome-program.c to activate accessibility support.
 **/
void
gnome_accessibility_module_init (void)
{
  gail_accessibility_module_init ();
}

/**
 * gnome_accessibility_module_shutdown:
 * @void: 
 * 
 *   This method is invoked by name from libgnome's
 * gnome-program.c to de-activate accessibility support.
 **/
void
gnome_accessibility_module_shutdown (void)
{
  if (!gail_initialized)
    {
      return;
    }
  gail_initialized = FALSE;

  fprintf (stderr, "Gtk Accessibilty Module shutdown\n");

  /* FIXME: de-register the factory types so we can unload ? */
}

int
gtk_module_init (gint *argc, char** argv[])
{
  gail_accessibility_module_init ();

  return 0;
}
