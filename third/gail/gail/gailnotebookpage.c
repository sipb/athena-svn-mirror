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
#include "gailnotebookpage.h"

static void      gail_notebook_page_class_init      (GailNotebookPageClass     *klass);

static void                  gail_notebook_page_finalize       (GObject   *object);

static G_CONST_RETURN gchar* gail_notebook_page_get_name       (AtkObject *accessible);
static AtkObject*            gail_notebook_page_get_parent     (AtkObject *accessible);
static gint                  gail_notebook_page_get_n_children (AtkObject *accessible);
static AtkObject*            gail_notebook_page_ref_child      (AtkObject *accessible,
                                                                gint      i); 
static gint                  gail_notebook_page_get_index_in_parent
                                                               (AtkObject *accessible);
static AtkStateSet*          gail_notebook_page_ref_state_set  (AtkObject *accessible);

static void                  atk_component_interface_init      (AtkComponentIface *iface);

static AtkObject*            gail_notebook_page_ref_accessible_at_point 
                                                               (AtkComponent *component,
                                                                gint         x,
                                                                gint         y,
                                                                AtkCoordType coord_type);

static void                  gail_notebook_page_get_extents    (AtkComponent *component,
                                                                gint         *x,
                                                                gint         *y,
                                                                gint         *width,
                                                                gint         *height,
                                                                AtkCoordType coord_type);

static AtkObject*            _gail_notebook_page_get_tab_label (GailNotebookPage *page);

static gpointer parent_class = NULL;

GType
gail_notebook_page_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo tinfo = 
      {
        sizeof (GailNotebookPageClass),
        (GBaseInitFunc) NULL, /* base init */
        (GBaseFinalizeFunc) NULL, /* base finalize */
        (GClassInitFunc) gail_notebook_page_class_init, /* class init */
        (GClassFinalizeFunc) NULL, /* class finalize */
        NULL, /* class data */
        sizeof (GailNotebookPage), /* instance size */
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

      type = g_type_register_static (ATK_TYPE_OBJECT,
                                     "GailNotebookPage", &tinfo, 0);

      g_type_add_interface_static (type, ATK_TYPE_COMPONENT,
                                   &atk_component_info);
    }
  return type;
}

static void
gail_notebook_page_class_init (GailNotebookPageClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  AtkObjectClass *class = ATK_OBJECT_CLASS (klass);
  
  parent_class = g_type_class_peek_parent (klass);
  
  class->get_name = gail_notebook_page_get_name;
  class->get_parent = gail_notebook_page_get_parent;
  class->get_n_children = gail_notebook_page_get_n_children;
  class->ref_child = gail_notebook_page_ref_child;
  class->ref_state_set = gail_notebook_page_ref_state_set;
  class->get_index_in_parent = gail_notebook_page_get_index_in_parent;

  gobject_class->finalize = gail_notebook_page_finalize;
}

AtkObject*
gail_notebook_page_new (GtkNotebook *notebook, 
                        gint        pagenum)
{
  GObject *object;
  AtkObject *atk_object, *atk_child;
  GailNotebookPage *page;
  GtkWidget *child;
  
  g_return_val_if_fail (GTK_IS_NOTEBOOK (notebook), NULL);

  child = gtk_notebook_get_nth_page (notebook, pagenum);

  if (!child)
    return NULL;

  object = g_object_new (GAIL_TYPE_NOTEBOOK_PAGE, NULL);
  g_return_val_if_fail (object != NULL, NULL);

  page = GAIL_NOTEBOOK_PAGE (object);
  page->notebook = notebook;
  g_object_add_weak_pointer (G_OBJECT (page->notebook), (gpointer *)&page->notebook);
  page->index = pagenum;
  
  atk_object = ATK_OBJECT (page);
  atk_object->role = ATK_ROLE_PAGE_TAB;
  atk_object->layer = ATK_LAYER_WIDGET;

  atk_child = gtk_widget_get_accessible (child);
  atk_object_set_parent (atk_child, atk_object);
  
  return atk_object;
}

static void
gail_notebook_page_finalize (GObject *object)
{
  GailNotebookPage *page = GAIL_NOTEBOOK_PAGE (object);

  if (page->notebook)
    g_object_remove_weak_pointer (G_OBJECT (page->notebook), (gpointer *)&page->notebook);

  G_OBJECT_CLASS (parent_class)->finalize (object);

}


static G_CONST_RETURN gchar*
gail_notebook_page_get_name (AtkObject *accessible)
{
  g_return_val_if_fail (GAIL_IS_NOTEBOOK_PAGE (accessible), NULL);
  
  if (accessible->name != NULL)
    return accessible->name;
  else
    {
      GtkWidget *child;
      GtkNotebook *notebook;
      GailNotebookPage *page;
      G_CONST_RETURN gchar *label;
	
      page = GAIL_NOTEBOOK_PAGE (accessible);
	
      notebook = page->notebook;
      if (!notebook)
        return NULL;

      child = gtk_notebook_get_nth_page (notebook, page->index);
      g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
	
      label = gtk_notebook_get_tab_label_text (notebook, child);
      /*
       * bugzilla.gnome.org Bug #57995 Gtk+-1.3.x
       * GtkNotebook: Can't get tab label for pages created with default label.
       * label will be NULL for pages created with no tab label argument
       * until this bug is resolved.
       */
	 
      return label;
    }
}

static AtkObject*
gail_notebook_page_get_parent (AtkObject *accessible)
{
  GailNotebookPage *page;
  
  g_return_val_if_fail (GAIL_IS_NOTEBOOK_PAGE (accessible), NULL);
  
  page = GAIL_NOTEBOOK_PAGE (accessible);

  if (!page->notebook)
    return NULL;

  return gtk_widget_get_accessible (GTK_WIDGET (page->notebook));
}

static gint
gail_notebook_page_get_n_children (AtkObject *accessible)
{
  /* Notebook page has only one child */
  g_return_val_if_fail (GAIL_IS_NOTEBOOK_PAGE (accessible), 0);

  return 1;
}

static AtkObject*
gail_notebook_page_ref_child (AtkObject *accessible,
                              gint i)
{
  GtkWidget *child;
  AtkObject *child_obj;
  GailNotebookPage *page = NULL;
   
  g_return_val_if_fail (GAIL_IS_NOTEBOOK_PAGE (accessible), NULL);
  if (i != 0)
    return NULL;
   
  page = GAIL_NOTEBOOK_PAGE (accessible);
  if (!page->notebook)
    return NULL;

  child = gtk_notebook_get_nth_page (page->notebook, page->index);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
   
  child_obj = gtk_widget_get_accessible (child);
  g_object_ref (child_obj);
  return child_obj;
}

static gint
gail_notebook_page_get_index_in_parent (AtkObject *accessible)
{
  GailNotebookPage *page;

  g_return_val_if_fail (GAIL_IS_NOTEBOOK_PAGE (accessible), -1);
  page = GAIL_NOTEBOOK_PAGE (accessible);

  return page->index;
}

static AtkStateSet*
gail_notebook_page_ref_state_set (AtkObject *accessible)
{
  AtkStateSet *state_set, *label_state_set, *merged_state_set;
  AtkObject *atk_label;

  g_return_val_if_fail (GAIL_NOTEBOOK_PAGE (accessible), NULL);

  state_set = ATK_OBJECT_CLASS (parent_class)->ref_state_set (accessible);

  atk_label = _gail_notebook_page_get_tab_label (GAIL_NOTEBOOK_PAGE (accessible));
  if (atk_label)
    {
      label_state_set = atk_object_ref_state_set (atk_label);
      merged_state_set = atk_state_set_or_sets (state_set, label_state_set);
      g_object_unref (label_state_set);
      g_object_unref (state_set);
    }
  else
    {
      merged_state_set = state_set;
    }
  return merged_state_set;
}


static void
atk_component_interface_init (AtkComponentIface *iface)
{
  g_return_if_fail (iface != NULL);

  /*
   * We use the default implementations for contains, get_position, get_size
   */
  iface->ref_accessible_at_point = gail_notebook_page_ref_accessible_at_point;
  iface->get_extents = gail_notebook_page_get_extents;
}

static AtkObject*
gail_notebook_page_ref_accessible_at_point (AtkComponent *component,
                                            gint         x,
                                            gint         y,
                                            AtkCoordType coord_type)
{
  /*
   * There is only one child so we return it.
   */
  AtkObject* child;

  g_return_val_if_fail (ATK_IS_OBJECT (component), NULL);

  child = atk_object_ref_accessible_child (ATK_OBJECT (component), 0);
  return child;
}

static void
gail_notebook_page_get_extents (AtkComponent *component,
                                gint         *x,
                                gint         *y,
                                gint         *width,
                                gint         *height,
                                AtkCoordType coord_type)
{
  AtkObject *atk_label;

  g_return_if_fail (GAIL_IS_NOTEBOOK_PAGE (component));

  atk_label = _gail_notebook_page_get_tab_label (GAIL_NOTEBOOK_PAGE (component));

  if (!atk_label)
    {
      AtkObject *child;

      *width = 0;
      *height = 0;

      child = atk_object_ref_accessible_child (ATK_OBJECT (component), 0);
      g_return_if_fail (child);

      atk_component_get_position (ATK_COMPONENT (child), x, y, coord_type);
    }
  else
    {
      atk_component_get_extents (ATK_COMPONENT (atk_label), 
                                 x, y, width, height, coord_type);
    }
  return; 
}

static AtkObject*
_gail_notebook_page_get_tab_label (GailNotebookPage *page)
{
  GtkWidget *label, *child;

  if (!page->notebook)
    return NULL;

  child = gtk_notebook_get_nth_page (page->notebook, page->index);
  label = gtk_notebook_get_tab_label (page->notebook, child);
  if (label)
    return gtk_widget_get_accessible (label);
  else
    return NULL;
}
