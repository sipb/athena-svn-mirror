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
#include "gailnotebook.h"
#include "gailnotebookpage.h"

static void         gail_notebook_class_init          (GailNotebookClass *klass);
static void         gail_notebook_object_init         (GailNotebook      *notebook);
static void         gail_notebook_finalize            (GObject           *object);
static void         gail_notebook_real_initialize     (AtkObject         *obj,
                                                       gpointer          data);

static void         gail_notebook_real_notify_gtk     (GObject           *obj,
                                                       GParamSpec        *pspec);

static AtkObject*   gail_notebook_ref_child           (AtkObject      *obj,
                                                       gint           i);
static void         atk_selection_interface_init      (AtkSelectionIface *iface);
static gboolean     gail_notebook_add_selection       (AtkSelection   *selection,
                                                       gint           i);
static AtkObject*   gail_notebook_ref_selection       (AtkSelection   *selection,
                                                       gint           i);
static gint         gail_notebook_get_selection_count (AtkSelection   *selection);
static gboolean     gail_notebook_is_child_selected   (AtkSelection   *selection,
                                                       gint           i);
static AtkObject*   find_child_in_list                (GList          *list,
                                                       gint           index);
static gboolean     gail_notebook_focus_cb            (GtkWidget      *widget,
                                                       GtkDirectionType type);
static gboolean     gail_notebook_check_focus_tab     (gpointer       data);
static void         gail_notebook_destroyed           (gpointer       data);

static gpointer parent_class = NULL;

GType
gail_notebook_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo tinfo =
      {
        sizeof (GailNotebookClass),
        (GBaseInitFunc) NULL, /* base init */
        (GBaseFinalizeFunc) NULL, /* base finalize */
        (GClassInitFunc) gail_notebook_class_init, /* class init */
        (GClassFinalizeFunc) NULL, /* class finalize */
        NULL, /* class data */
        sizeof (GailNotebook), /* instance size */
        0, /* nb preallocs */
        (GInstanceInitFunc) gail_notebook_object_init, /* instance init */
        NULL /* value table */
      };

      static const GInterfaceInfo atk_selection_info = 
      {
        (GInterfaceInitFunc) atk_selection_interface_init,
        (GInterfaceFinalizeFunc) NULL,
        NULL
      };
	
      type = g_type_register_static (GAIL_TYPE_CONTAINER,
                                     "GailNotebook", &tinfo, 0);
      g_type_add_interface_static (type, ATK_TYPE_SELECTION,
                                   &atk_selection_info);
    }
  return type;
}

static void
gail_notebook_class_init (GailNotebookClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  AtkObjectClass  *class = ATK_OBJECT_CLASS (klass);
  GailWidgetClass *widget_class;

  widget_class = (GailWidgetClass*)klass;

  parent_class = g_type_class_peek_parent (klass);
  
  gobject_class->finalize = gail_notebook_finalize;

  widget_class->notify_gtk = gail_notebook_real_notify_gtk;

  class->ref_child = gail_notebook_ref_child;
  class->initialize = gail_notebook_real_initialize;
  /*
   * We do not provide an implementation of get_n_children
   * as the implementation in GailContainer returns the correct
   * number of children.
   */
}

static void
gail_notebook_object_init (GailNotebook      *notebook)
{
  notebook->page_cache = NULL;
  notebook->selected_page = -1;
  notebook->focus_tab_page = -1;
  notebook->idle_focus_id = 0;
}

AtkObject* 
gail_notebook_new (GtkWidget *widget)
{
  GObject *object;
  AtkObject *accessible;

  g_return_val_if_fail (GTK_IS_NOTEBOOK (widget), NULL);

  object = g_object_new (GAIL_TYPE_NOTEBOOK, NULL);

  accessible = ATK_OBJECT (object);
  atk_object_initialize (accessible, widget);

  accessible->role = ATK_ROLE_PAGE_TAB_LIST;

  return accessible;
}

static AtkObject*
gail_notebook_ref_child (AtkObject      *obj,
                         gint           i)
{
  AtkObject *accessible = NULL;
  GailNotebook *gail_notebook;
  GtkNotebook *gtk_notebook;
  GtkWidget *widget;
 
  widget = GTK_ACCESSIBLE (obj)->widget;
  if (widget == NULL)
    /*
     * State is defunct
     */
    return NULL;

  gail_notebook = GAIL_NOTEBOOK (obj);
  
  gtk_notebook = GTK_NOTEBOOK (widget);
  
  accessible = find_child_in_list (gail_notebook->page_cache, i);

  if (accessible == NULL)
    { 
    /*
     * We attempt to create the cache of pages when the GailNotebook
     * is created but we do not get notifications of pages being added or
     * removed subsequently.
     *
     * So this code should be reached only for a page which was added 
     * subsequently.
     *
     * We have a problem if pages are removed in that we cannot remove
     * the corresponding accessible from the cache.
     * See bug 58674
     */
      accessible = gail_notebook_page_new (gtk_notebook, i);
      if (!accessible)
        return NULL;
      gail_notebook->page_cache = g_list_append (gail_notebook->page_cache, 
                                                 accessible);
      g_return_val_if_fail (accessible, NULL);
    }
  g_object_ref (accessible);
  return accessible;
}

static void
gail_notebook_real_initialize (AtkObject *obj,
                               gpointer  data)
{
  GailNotebook *notebook = GAIL_NOTEBOOK (obj);
  GtkNotebook *gtk_notebook = GTK_NOTEBOOK (data);
  GtkWidget *page;
  gint i;

  ATK_OBJECT_CLASS (parent_class)->initialize (obj, data);

  i = 0;
  page = gtk_notebook_get_nth_page (gtk_notebook, i);
  while (page)
    {
      AtkObject *obj;

      obj = gail_notebook_page_new (gtk_notebook, i);
      notebook->page_cache = g_list_append (notebook->page_cache, obj);
      page = gtk_notebook_get_nth_page (gtk_notebook, ++i);
    }
  notebook->selected_page = gtk_notebook_get_current_page (gtk_notebook);
  if (gtk_notebook->focus_tab && gtk_notebook->focus_tab->data)
    {
      notebook->focus_tab_page = g_list_index (gtk_notebook->children, gtk_notebook->focus_tab->data);
    }
  g_signal_connect (gtk_notebook,
                    "focus",
                    G_CALLBACK (gail_notebook_focus_cb),
                    NULL);
  g_object_weak_ref (G_OBJECT(gtk_notebook),
                     (GWeakNotify) gail_notebook_destroyed,
                     obj);                     
}

static void
gail_notebook_real_notify_gtk (GObject           *obj,
                               GParamSpec        *pspec)
{
  GtkWidget *widget = GTK_WIDGET (obj);
  AtkObject* atk_obj = gtk_widget_get_accessible (widget);

  if (strcmp (pspec->name, "page") == 0)
    {
      gint page_num, old_page_num;
      gint focus_page_num, old_focus_page_num;
      GailNotebook *gail_notebook = GAIL_NOTEBOOK (atk_obj);
      GtkNotebook *gtk_notebook = GTK_NOTEBOOK (widget);
     
      /*
       * Notify SELECTED state change for old and new page
       */
      old_page_num = gail_notebook->selected_page;
      page_num = gtk_notebook_get_current_page (gtk_notebook);
      gail_notebook->selected_page = page_num;
      old_focus_page_num = gail_notebook->focus_tab_page;
      focus_page_num = g_list_index (gtk_notebook->children, gtk_notebook->focus_tab->data);
      gail_notebook->focus_tab_page = focus_page_num;
    
      if (page_num != old_page_num)
        {
          AtkObject *obj;

          g_signal_emit_by_name (atk_obj, "selection_changed");
          g_signal_emit_by_name (atk_obj, "visible_data_changed");

          obj = find_child_in_list (gail_notebook->page_cache, old_page_num);
          if (obj)
            atk_object_notify_state_change (obj,
                                            ATK_STATE_SELECTED,
                                            FALSE);
          obj = find_child_in_list (gail_notebook->page_cache, page_num);
          if (obj)
            {
              atk_object_notify_state_change (obj,
                                              ATK_STATE_SELECTED,
                                              TRUE);
              /*
               * The page which is being displayed has changed but there is
               * no need to tell the focus tracker as the focus page will also 
               * change or a widget in the page will receive focus if the
               * Notebook does not have tabs.
               */
            }
        }
      if (gtk_notebook_get_show_tabs (gtk_notebook) &&
         (focus_page_num != old_focus_page_num))
        {
          if (gail_notebook->idle_focus_id)
            gtk_idle_remove (gail_notebook->idle_focus_id);
          gail_notebook->idle_focus_id = gtk_idle_add (gail_notebook_check_focus_tab, atk_obj);
        }
    }
  else
    GAIL_WIDGET_CLASS (parent_class)->notify_gtk (obj, pspec);
}

static void
gail_notebook_finalize (GObject            *object)
{
  GailNotebook *notebook = GAIL_NOTEBOOK (object);
  GList *list;

  /*
   * Get rid of the GailNotebookPage objects which we have cached.
   */
  list = notebook->page_cache;
  if (list != NULL)
    {
      while (list)
        {
          g_object_unref (list->data);
          list = list->next;
        }
    }

  g_list_free (notebook->page_cache);
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
atk_selection_interface_init (AtkSelectionIface *iface)
{
  g_return_if_fail (iface != NULL);
  
  iface->add_selection = gail_notebook_add_selection;
  iface->ref_selection = gail_notebook_ref_selection;
  iface->get_selection_count = gail_notebook_get_selection_count;
  iface->is_child_selected = gail_notebook_is_child_selected;
  /*
   * The following don't make any sense for GtkNotebook widgets.
   * Unsupported AtkSelection interfaces:
   * clear_selection();
   * remove_selection();
   * select_all_selection();
   */
}

/*
 * GtkNotebook only supports the selection of one page at a time. 
 * Selecting a page unselects any previous selection, so this 
 * changes the current selection instead of adding to it.
 */
static gboolean
gail_notebook_add_selection (AtkSelection *selection,
                             gint         i)
{
  GtkNotebook *notebook;
  GtkWidget *widget;
  
  widget =  GTK_ACCESSIBLE (selection)->widget;
  if (widget == NULL)
    /*
     * State is defunct
     */
    return FALSE;
  
  notebook = GTK_NOTEBOOK (widget);
  gtk_notebook_set_current_page (notebook, i);
  return TRUE;
}

static AtkObject*
gail_notebook_ref_selection (AtkSelection *selection,
                             gint i)
{
  AtkObject *accessible;
  GtkWidget *widget;
  GtkNotebook *notebook;
  gint pagenum;
  
  /*
   * A note book can have only one selection.
   */
  g_return_val_if_fail (i == 0, NULL);
  g_return_val_if_fail (GAIL_IS_NOTEBOOK (selection), NULL);
  
  widget = GTK_ACCESSIBLE (selection)->widget;
  if (widget == NULL)
    /* State is defunct */
	return NULL;
  
  notebook = GTK_NOTEBOOK (widget);
  pagenum = gtk_notebook_get_current_page (notebook);
  g_return_val_if_fail (pagenum != -1, NULL);
  accessible = gail_notebook_ref_child (ATK_OBJECT (selection), pagenum);

  return accessible;
}

/*
 * Always return 1 because there can only be one page
 * selected at any time
 */
static gint
gail_notebook_get_selection_count (AtkSelection *selection)
{
  GtkWidget *widget;
  GtkNotebook *notebook;
  
  widget = GTK_ACCESSIBLE (selection)->widget;
  if (widget == NULL)
    /*
     * State is defunct
     */
    return 0;

  notebook = GTK_NOTEBOOK (widget);
  if (notebook == NULL)
    return 0;
  else
    return 1;
}

static gboolean
gail_notebook_is_child_selected (AtkSelection *selection,
                                 gint i)
{
  GtkWidget *widget;
  GtkNotebook *notebook;
  gint pagenumber;

  widget = GTK_ACCESSIBLE (selection)->widget;
  if (widget == NULL)
    /* 
     * State is defunct
     */
    return FALSE;

  
  notebook = GTK_NOTEBOOK (widget);
  pagenumber = gtk_notebook_get_current_page(notebook);

  if (pagenumber == i)
    return TRUE;
  else
    return FALSE; 
}

static AtkObject*
find_child_in_list (GList *list,
                    gint  index)
{
  AtkObject *obj = NULL;

  while (list)
    {
      if (GAIL_NOTEBOOK_PAGE (list->data)->index == index)
        {
          obj = ATK_OBJECT (list->data);
          break;
        }
      list = list->next;
    }
  return obj;
}

static gboolean
gail_notebook_focus_cb (GtkWidget      *widget,
                        GtkDirectionType type)
{
  AtkObject *atk_obj = gtk_widget_get_accessible (widget);
  GailNotebook *gail_notebook = GAIL_NOTEBOOK (atk_obj);

  switch (type)
    {
    case GTK_DIR_LEFT:
    case GTK_DIR_RIGHT:
      if (gail_notebook->idle_focus_id)
        gtk_idle_remove (gail_notebook->idle_focus_id);
      gail_notebook->idle_focus_id = gtk_idle_add (gail_notebook_check_focus_tab, atk_obj);
      break;
    default:
      break;
    }
  return FALSE;
}

static gboolean
gail_notebook_check_focus_tab (gpointer data)
{
  GtkWidget *widget;
  AtkObject *atk_obj;
  gint focus_page_num, old_focus_page_num;
  GailNotebook *gail_notebook;
  GtkNotebook *gtk_notebook;

  atk_obj = ATK_OBJECT (data);
  gail_notebook = GAIL_NOTEBOOK (atk_obj);
  widget = GTK_ACCESSIBLE (atk_obj)->widget;

  gtk_notebook = GTK_NOTEBOOK (widget);

  gail_notebook->idle_focus_id = 0;

  if (!gtk_notebook->focus_tab)
    return FALSE;

  old_focus_page_num = gail_notebook->focus_tab_page;
  focus_page_num = g_list_index (gtk_notebook->children, gtk_notebook->focus_tab->data);
  gail_notebook->focus_tab_page = focus_page_num;
  if (old_focus_page_num != focus_page_num)
    {
      AtkObject *obj;

      obj = atk_object_ref_accessible_child (atk_obj, focus_page_num);
      atk_focus_tracker_notify (obj);
      g_object_unref (obj);
    }
  return FALSE;
}

static void
gail_notebook_destroyed (gpointer data)
{
  GailNotebook *gail_notebook = GAIL_NOTEBOOK (data);

  if (gail_notebook->idle_focus_id)
    gtk_idle_remove (gail_notebook->idle_focus_id);
}
