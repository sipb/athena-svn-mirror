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
#include <gtk/gtktreeviewcolumn.h>
#include "gailtreeview.h"
#include "gailrenderercell.h"
#include "gailbooleancell.h"
#include "gailcontainercell.h"
#include "gailtextcell.h"
#include "gailcellparent.h"

typedef struct _GailTreeViewRowInfo    GailTreeViewRowInfo;
typedef struct _GailTreeViewCellInfo   GailTreeViewCellInfo;

static void             gail_tree_view_class_init       (GailTreeViewClass      *klass);
static void             gail_tree_view_real_initialize  (AtkObject              *obj,
                                                         gpointer               data);
static void             gail_tree_view_real_notify_gtk  (GObject		*obj,
                                                         GParamSpec		*pspec);
static void             gail_tree_view_finalize         (GObject                *object);

/* atkobject.h */

static gint             gail_tree_view_get_n_children   (AtkObject              *obj);
static AtkObject*       gail_tree_view_ref_child        (AtkObject              *obj,
                                                         gint                   i);

/* atkcomponent.h */

static void             atk_component_interface_init    (AtkComponentIface      *iface);

static AtkObject*       gail_tree_view_ref_accessible_at_point
                                                        (AtkComponent           *component,
                                                         gint                   x,
                                                         gint                   y,
                                                         AtkCoordType           coord_type);
           
/* atktable.h */

static void             atk_table_interface_init        (AtkTableIface          *iface);

static gint             gail_tree_view_get_index_at     (AtkTable               *table,
                                                         gint                   row,
                                                         gint                   column);
static gint             gail_tree_view_get_column_at_index
                                                        (AtkTable               *table,
                                                         gint                   index);
static gint             gail_tree_view_get_row_at_index (AtkTable               *table,
                                                         gint                   index);

static AtkObject*       gail_tree_view_table_ref_at     (AtkTable               *table,
                                                         gint                   row,
                                                         gint                   column);
static gint             gail_tree_view_get_n_rows       (AtkTable               *table);
static gint             gail_tree_view_get_n_columns    (AtkTable               *table);
static gint             get_n_actual_columns            (GtkTreeView            *tree_view);
static gboolean         gail_tree_view_is_row_selected  (AtkTable               *table,
                                                         gint                   row);
static gboolean         gail_tree_view_is_selected      (AtkTable               *table,
                                                         gint                   row,
                                                         gint                   column);
static gint             gail_tree_view_get_selected_rows 
                                                        (AtkTable               *table, 
                                                         gint                   **selected);
static gboolean         gail_tree_view_add_row_selection 
                                                        (AtkTable               *table, 
                                                         gint                   row);
static gboolean         gail_tree_view_remove_row_selection 
                                                        (AtkTable               *table, 
                                                         gint                   row);
static AtkObject*       gail_tree_view_get_row_header   (AtkTable               *table,
                                                         gint                   row);
static AtkObject*       gail_tree_view_get_column_header 
                                                        (AtkTable               *table,
                                                         gint                   column);
static void             gail_tree_view_set_row_header   (AtkTable               *table,
                                                         gint                   row,
                                                         AtkObject              *header);
static void             gail_tree_view_set_column_header 
                                                        (AtkTable               *table,
                                                         gint                   column,
                                                         AtkObject              *header);
static AtkObject*
                        gail_tree_view_get_caption      (AtkTable               *table);
static void             gail_tree_view_set_caption      (AtkTable               *table,
                                                         AtkObject              *caption);
static AtkObject*       gail_tree_view_get_summary      (AtkTable               *table);
static void             gail_tree_view_set_summary      (AtkTable               *table,
                                                         AtkObject              *accessible);
static G_CONST_RETURN gchar*
                        gail_tree_view_get_row_description 
                                                        (AtkTable               *table,
                                                         gint                   row);
static void             gail_tree_view_set_row_description 
                                                        (AtkTable               *table,
                                                         gint                   row,
                                                         const gchar            *description);
static G_CONST_RETURN gchar*
                        gail_tree_view_get_column_description
                                                        (AtkTable               *table,
                                                         gint                   column);
static void             gail_tree_view_set_column_description
                                                        (AtkTable               *table,
                                                         gint                   column,
                                                         const gchar            *description);

static void             set_row_data                    (AtkTable               *table,
                                                         gint                   row,
                                                         AtkObject              *header,
                                                         const gchar            *description,
                                                         gboolean               is_header);
static GailTreeViewRowInfo* 
                        get_row_info                    (AtkTable               *table,
                                                         gint                   row);

/* atkselection.h */

static void             atk_selection_interface_init    (AtkSelectionIface      *iface);
static gboolean         gail_tree_view_clear_selection  (AtkSelection           *selection);
static AtkObject*       gail_tree_view_ref_selection    (AtkSelection           *selection,
                                                         gint                   i);
static gint             gail_tree_view_get_selection_count 
                                                        (AtkSelection           *selection);
static gboolean         gail_tree_view_is_child_selected 
                                                        (AtkSelection           *selection,
                                                         gint                   i);

/* gailcellparent.h */

static void             gail_cell_parent_interface_init (GailCellParentIface    *iface);
static void             gail_tree_view_get_cell_extents (GailCellParent         *parent,
                                                         GailCell               *cell,
                                                         gint                   *x,
                                                         gint                   *y,
                                                         gint                   *width,
                                                         gint                   *height,
                                                         AtkCoordType           coord_type);
static void             gail_tree_view_get_cell_area    (GailCellParent         *parent,
                                                         GailCell               *cell,
                                                         GdkRectangle           *cell_rect);

/* signal handling */

static gboolean         gail_tree_view_expand_row_gtk   (GtkTreeView            *tree_view,
                                                         GtkTreeIter            *iter,
                                                         GtkTreePath            *path);
static gint             idle_expand_row                 (gpointer               data);
static gboolean         gail_tree_view_collapse_row_gtk (GtkTreeView            *tree_view,
                                                         GtkTreeIter            *iter,
                                                         GtkTreePath            *path);
static void             gail_tree_view_size_allocate_gtk (GtkWidget             *widget,
                                                         GtkAllocation          *allocation);
static void             gail_tree_view_changed_gtk      (GtkTreeSelection       *selection,
                                                         gpointer               data);

static void             columns_changed                 (GtkTreeView            *tree_view);
static void             cursor_changed                  (GtkTreeView            *tree_view);
static gint             idle_cursor_changed             (gpointer               data);

static void             model_row_changed               (GtkTreeModel           *tree_model,
                                                         GtkTreePath            *path,
                                                         GtkTreeIter            *iter,
                                                         gpointer               user_data);
static void             column_visibility_changed       (GObject                *object,
                                                         GParamSpec             *param,
                                                         gpointer               user_data);
static void             column_destroy                  (GtkObject              *obj); 
static void             model_row_inserted              (GtkTreeModel           *tree_model,
                                                         GtkTreePath            *path,
                                                         GtkTreeIter            *iter,
                                                         gpointer               user_data);
static void             model_row_deleted               (GtkTreeModel           *tree_model,
                                                         GtkTreePath            *path,
                                                         gpointer               user_data);
static void             destroy_count_func              (GtkTreeView            *tree_view,
                                                         GtkTreePath            *path,
                                                         gint                   count,
                                                         gpointer               user_data);
static void             model_rows_reordered            (GtkTreeModel           *tree_model,
                                                         GtkTreePath            *path,
                                                         GtkTreeIter            *iter,
                                                         gint                   *new_order,
                                                         gpointer               user_data);
static void             adjustment_changed              (GtkAdjustment          *adjustment,
                                                         GtkTreeView            *tree_view);

/* Misc */

static void             set_iter_nth_row                (GtkTreeView            *tree_view,
                                                         GtkTreeIter            *iter,
                                                         gint                   row);
static gint             get_row_from_tree_path          (GtkTreeView            *tree_view,
                                                         GtkTreePath            *path);
static GtkTreeViewColumn* get_column                    (GtkTreeView            *tree_view,
                                                         gint                   in_col);
static gint             get_actual_column_number        (GtkTreeView            *tree_view,
                                                         gint                   visible_column);
static gint             get_visible_column_number       (GtkTreeView            *tree_view,
                                                         gint                   actual_column);
static void		iterate_thru_children           (GtkTreeView            *tree_view,
                                                         GtkTreeModel           *tree_model,
                                                         GtkTreePath            *tree_path,
                                                         GtkTreePath            *orig,
                                                         gint                   *count,
                                                         gint                   depth);
static GtkTreeIter*     return_iter_nth_row             (GtkTreeView            *tree_view,
                                                         GtkTreeModel           *tree_model,
                                                         GtkTreeIter            *iter,
                                                         gint                   increment,
                                                         gint                   row);
static void             free_row_info                   (GArray                 *array,
                                                         gint                   array_idx,
                                                         gboolean               shift);
static void             clean_cell_info                 (GailTreeView           *tree_view,
                                                         GList                  *list); 
static void             clean_rows                      (GailTreeView           *tree_view);
static void             clean_cols                      (GailTreeView           *tree_view,
                                                         GtkTreeViewColumn      *tv_col);
static void             traverse_cells                  (GailTreeView           *tree_view,
                                                         GtkTreePath            *tree_path,
                                                         gboolean               set_stale,
                                                         gboolean               inc_row);
static gboolean         update_cell_value               (GailRendererCell       *renderer_cell,
                                                         GailTreeView           *gailview,
                                                         gboolean               emit_change_signal);
static void             set_cell_visibility             (GtkTreeView            *tree_view,
                                                         GailCell               *cell,
                                                         GtkTreeViewColumn      *tv_col,
                                                         GtkTreePath            *tree_path,
                                                         gboolean               emit_signal);
static gboolean         is_cell_showing                 (GtkTreeView            *tree_view,
                                                         GdkRectangle           *cell_rect);
static void             set_expand_state                (GtkTreeView            *tree_view,
                                                         GtkTreeModel           *tree_model,
                                                         GailTreeView           *gailview,
                                                         GtkTreePath            *tree_path,
                                                         gboolean               set_on_ancestor);
static void             add_cell_actions                (GailCell               *cell,
                                                         gboolean               editable);

static gboolean         toggle_cell_expanded            (GailCell               *cell);
static gboolean         toggle_cell_toggled             (GailCell               *cell);
static gboolean         edit_cell                       (GailCell               *cell);
static gboolean         activate_cell                   (GailCell               *cell);
static void             cell_destroyed                  (gpointer               data);
static void             cell_info_remove                (GailTreeView           *tree_view, 
                                                         GailCell               *cell);
static void             cell_info_get_index             (GtkTreeView            *tree_view, 
                                                         GailTreeViewCellInfo   *info,
                                                         gint                   *index);
static void             cell_info_new                   (GailTreeView           *gailview, 
                                                         GtkTreeModel           *tree_model,
                                                         GtkTreePath            *path,
                                                         GtkTreeViewColumn      *tv_col,
                                                         GailCell               *cell);
static GailCell*        find_cell                       (GailTreeView           *gailview, 
                                                         gint                   index);
static void             refresh_cell_index              (GailCell               *cell);
static void             get_selected_rows               (GtkTreeModel           *model,
                                                         GtkTreePath            *path,
                                                         GtkTreeIter            *iter,
                                                         gpointer               data);
static void             connect_model_signals           (GtkTreeView            *view,
                                                         GtkTreeModel           *model); 
static void             disconnect_model_signals        (GtkTreeModel           *model); 
static void             clear_cached_data               (GailTreeView           *view);
static gint             get_column_number               (GtkTreeView            *tree_view,
                                                         GtkTreeViewColumn      *column,
                                                         gboolean               visible); 
static gint             get_index                       (GtkTreeView            *tree_view,
                                                         GtkTreePath            *path,
                                                         gint                   actual_column);
static void             count_rows                      (GtkTreeModel           *model,
                                                         GtkTreeIter            *iter,
                                                         GtkTreePath            *end_path,
                                                         gint                   *count,
                                                         gint                   level,
                                                         gint                   depth);

static gboolean         get_next_node_with_child_at_depth 
                                                        (GtkTreeModel           *model,
                                                         GtkTreeIter            *iter,
                                                         GtkTreePath            **path,
                                                         gint                   level,
                                                         gint                   depth);
static gboolean         get_next_node_with_child        (GtkTreeModel           *model,
                                                         GtkTreePath            *path,
                                                         GtkTreePath            **return_path);
static gboolean         get_tree_path_from_row_index    (GtkTreeModel           *model,
                                                         gint                   row_index,
                                                         GtkTreePath            **tree_path);
static gint             get_row_count                   (GtkTreeModel           *model);
static gboolean         get_path_column_from_index      (GtkTreeView            *tree_view,
                                                         gint                   index,
                                                         GtkTreePath            **path,
                                                         GtkTreeViewColumn      **column);
static void             set_cell_expandable             (GailCell               *cell);

static GailTreeViewCellInfo* find_cell_info             (GailTreeView           *view,
                                                         GailCell               *cell,
                                                          GList**                list);

static GailWidgetClass *parent_class = NULL;
static GQuark quark_column_desc_object = 0;
static GQuark quark_column_header_object = 0;
static gboolean editing = FALSE;

struct _GailTreeViewRowInfo
{
  GtkTreeRowReference *row_ref;
  gchar *description;
  AtkObject *header;
};

struct _GailTreeViewCellInfo
{
  GailCell *cell;
  GtkTreeRowReference *cell_row_ref;
  GtkTreeViewColumn *cell_col_ref;
};

typedef struct _GailTreeViewIdleData   GailTreeViewIdleData;

struct _GailTreeViewIdleData
{
  GailTreeView *tree_view;
  GtkTreePath  *path;
};


GType
gail_tree_view_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo tinfo =
      {
        sizeof (GailTreeViewClass),
        (GBaseInitFunc) NULL, /* base init */
        (GBaseFinalizeFunc) NULL, /* base finalize */
        (GClassInitFunc) gail_tree_view_class_init, /* class init */
        (GClassFinalizeFunc) NULL, /* class finalize */
        NULL, /* class data */
        sizeof (GailTreeView), /* instance size */
        0, /* nb preallocs */
        (GInstanceInitFunc) NULL, /* instance init */
        NULL /* value table */
      };

      static const GInterfaceInfo atk_table_info =
      {
     	(GInterfaceInitFunc) atk_table_interface_init,
	(GInterfaceFinalizeFunc) NULL,
	NULL
      };

      static const GInterfaceInfo atk_selection_info =
      {
     	(GInterfaceInitFunc) atk_selection_interface_init,
	(GInterfaceFinalizeFunc) NULL,
	NULL
      };

      static const GInterfaceInfo atk_component_info =
      {
        (GInterfaceInitFunc) atk_component_interface_init,
        (GInterfaceFinalizeFunc) NULL,
        NULL
      };

      static const GInterfaceInfo gail_cell_parent_info =
      {
     	(GInterfaceInitFunc) gail_cell_parent_interface_init,
	(GInterfaceFinalizeFunc) NULL,
	NULL
      };

      type = g_type_register_static (GAIL_TYPE_CONTAINER,
                                     "GailTreeView", &tinfo, 0);

      g_type_add_interface_static (type, ATK_TYPE_TABLE,
                                   &atk_table_info);
      g_type_add_interface_static (type, ATK_TYPE_SELECTION,
                                   &atk_selection_info);
      g_type_add_interface_static (type, ATK_TYPE_COMPONENT,
                                   &atk_component_info);
      g_type_add_interface_static (type, GAIL_TYPE_CELL_PARENT,
                                   &gail_cell_parent_info);
    }

  return type;
}

static void
gail_tree_view_class_init (GailTreeViewClass *klass)
{
  AtkObjectClass *class = ATK_OBJECT_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GailWidgetClass *widget_class;

  widget_class = (GailWidgetClass*)klass;

  parent_class = g_type_class_peek_parent (klass);

  class->get_n_children = gail_tree_view_get_n_children;
  class->ref_child = gail_tree_view_ref_child;
  class->initialize = gail_tree_view_real_initialize;

  widget_class->notify_gtk = gail_tree_view_real_notify_gtk;

  gobject_class->finalize = gail_tree_view_finalize;

  quark_column_desc_object = g_quark_from_static_string ("gtk-column-object");
  quark_column_header_object = g_quark_from_static_string ("gtk-header-object");
}

static void
gail_tree_view_real_initialize (AtkObject *obj,
                                gpointer  data)
{
  GailTreeView *view;
  GtkTreeView *tree_view;
  GtkTreeModel *tree_model; 
  GtkAdjustment *adj;
  GList *tv_cols, *tmp_list;
  GtkWidget *widget;
  guint handler_id;

  ATK_OBJECT_CLASS (parent_class)->initialize (obj, data);

  /*
   * The children of a GtkTreeView are the buttons at the top of the columns
   * we do not represent these as children so we do not want to report
   * children added or deleted when these changed.
   */
  handler_id = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (obj), "gail-add-handler-id"));
  g_signal_handler_disconnect (data, handler_id);
  handler_id = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (obj), "gail-remove-handler-id"));
  g_signal_handler_disconnect (data, handler_id);

  view = GAIL_TREE_VIEW (obj);
  view->caption = NULL;
  view->summary = NULL;
  view->row_data = NULL;
  view->col_data = NULL;
  view->cell_data = NULL;

  view->n_children_deleted = 0;

  widget = GTK_WIDGET (data);
  g_signal_connect_after (widget,
                          "row-collapsed",
                          G_CALLBACK (gail_tree_view_collapse_row_gtk),
                          NULL);
  g_signal_connect (widget,
                    "row-expanded",
                    G_CALLBACK (gail_tree_view_expand_row_gtk),
                    NULL);
  g_signal_connect (widget,
                    "size-allocate",
                    G_CALLBACK (gail_tree_view_size_allocate_gtk),
                    NULL);

  tree_view = GTK_TREE_VIEW (widget);
  tree_model = gtk_tree_view_get_model (tree_view);

  /* Set up signal handling */

  g_signal_connect_data (gtk_tree_view_get_selection (tree_view),
                         "changed",
                         (GCallback) gail_tree_view_changed_gtk,
                      	 obj, NULL, 0);

  g_signal_connect_data (tree_view, "columns-changed",
    (GCallback) columns_changed, NULL, NULL, 0);
  g_signal_connect_data (tree_view, "cursor-changed",
    (GCallback) cursor_changed, NULL, NULL, 0);

  view->tree_model = tree_model;
  if (tree_model)
    {
      connect_model_signals (tree_view, tree_model);

      if (GTK_IS_TREE_STORE (tree_model))
        obj->role = ATK_ROLE_TREE_TABLE;
      else if (GTK_IS_LIST_STORE (tree_model))
        obj->role = ATK_ROLE_TABLE;
      else
        obj->role = ATK_ROLE_UNKNOWN;
    }
  else
    {
      obj->role = ATK_ROLE_UNKNOWN;
    }

  /* adjustment callbacks */

  g_object_get (tree_view, "hadjustment", &adj, NULL);
  g_signal_connect (adj, 
                    "value_changed",
                    G_CALLBACK (adjustment_changed),
                    tree_view);

  g_object_get (tree_view, "vadjustment", &adj, NULL);
  g_signal_connect (adj, 
                    "value_changed",
                    G_CALLBACK (adjustment_changed),
                    tree_view);

  view->col_data = g_array_sized_new (FALSE, TRUE, 
                                      sizeof(GtkTreeViewColumn *), 0);

  tv_cols = gtk_tree_view_get_columns (tree_view);

  for (tmp_list = tv_cols; tmp_list; tmp_list = tmp_list->next)
    {
      g_signal_connect_data (tmp_list->data, "notify::visible",
       (GCallback)column_visibility_changed, 
        tree_view, FALSE, FALSE);
      g_signal_connect_data (tmp_list->data, "destroy",
       (GCallback)column_destroy, 
        FALSE, FALSE, FALSE);
      g_array_append_val (view->col_data, tmp_list->data);
    }

  gtk_tree_view_set_destroy_count_func (tree_view, 
                                        destroy_count_func,
                                        NULL, NULL);
  g_list_free (tv_cols);
}

static void
gail_tree_view_real_notify_gtk (GObject             *obj,
                                GParamSpec          *pspec)
{
  if (strcmp (pspec->name, "model") == 0)
    {
      GtkWidget *widget = GTK_WIDGET (obj);
      AtkObject* atk_obj = gtk_widget_get_accessible (widget);
      GtkTreeModel *tree_model;
      GtkTreeView *tree_view;
      GailTreeView *gail_view;
      AtkRole role;

      tree_view = GTK_TREE_VIEW (widget);
      tree_model = gtk_tree_view_get_model (tree_view);
      gail_view = GAIL_TREE_VIEW (atk_obj);
      if (gail_view->tree_model)
        disconnect_model_signals (gail_view->tree_model);
      clear_cached_data (gail_view);
      gail_view->tree_model = tree_model;
      /*
       * if there is no model the GtkTreeView is probably being destroyed
       */
      if (tree_model)
        {
          connect_model_signals (tree_view, tree_model);

          if (GTK_IS_TREE_STORE (tree_model))
            role = ATK_ROLE_TREE_TABLE;
          else if (GTK_IS_LIST_STORE (tree_model))
            role = ATK_ROLE_TABLE;
          else
            role = ATK_ROLE_UNKNOWN;

        }
      else
        {
          role = ATK_ROLE_UNKNOWN;
        }
      atk_object_set_role (atk_obj, role);
      g_object_freeze_notify (G_OBJECT (atk_obj));
      g_signal_emit_by_name (atk_obj, "model_changed");
      g_signal_emit_by_name (atk_obj, "visible_data_changed");
      g_object_thaw_notify (G_OBJECT (atk_obj));
    }
  else
    parent_class->notify_gtk (obj, pspec);
}

AtkObject*
gail_tree_view_new (GtkWidget *widget)
{
  GObject *object;
  AtkObject *accessible;

  g_return_val_if_fail (GTK_IS_TREE_VIEW (widget), NULL);

  object = g_object_new (GAIL_TYPE_TREE_VIEW, NULL);

  accessible = ATK_OBJECT (object);
  atk_object_initialize (accessible, widget);

  /*
   * The role is set in gail_tree_view_real_initialize()
   */

  return accessible;
}

static void
gail_tree_view_finalize (GObject	    *object)
{
  GailTreeView *view = GAIL_TREE_VIEW (object);

  if (view->caption)
    g_object_unref (view->caption);
  if (view->summary)
    g_object_unref (view->summary);

  if (view->tree_model)
    disconnect_model_signals (view->tree_model);

  clear_cached_data (view);

  if (view->col_data)
    {
      GArray *array = view->col_data;

     /*
      * No need to free the contents of the array since it
      * just contains pointers to the GtkTreeViewColumn
      * objects that are in the GtkTreeView.
      */
      g_array_free (array, TRUE);
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

AtkObject *
gail_tree_view_ref_focus_cell (GtkTreeView *tree_view)
{
  /*
   * This function returns a reference to the accessible object for the cell
   * in the treeview which has focus, if a cell has focus.
   */
  AtkObject *focus_cell = NULL;
  AtkObject *atk_obj;
  GtkTreePath *focus_path;
  GtkTreeViewColumn *focus_column;

  atk_obj = gtk_widget_get_accessible (GTK_WIDGET (tree_view));
  gtk_tree_view_get_cursor (tree_view, &focus_path, &focus_column);
  if (focus_path && focus_column)
    {
      gint index;

      index = get_index (tree_view, focus_path,
                         get_column_number (tree_view, focus_column, FALSE));
      focus_cell = atk_object_ref_accessible_child (atk_obj, index);
    }

  if (focus_path)
    gtk_tree_path_free (focus_path);

  return focus_cell;
}

/* atkobject.h */

static gint
gail_tree_view_get_n_children (AtkObject *obj)
{
  GtkWidget *widget;
  GtkTreeView *tree_view;
  GtkTreeModel *tree_model;
  gint n_rows, n_cols;

  g_return_val_if_fail (GAIL_IS_TREE_VIEW (obj), 0);

  widget = GTK_ACCESSIBLE (obj)->widget;
  if (widget == NULL)
    /*
     * State is defunct
     */
    return 0;

  tree_view = GTK_TREE_VIEW (widget);
  tree_model = gtk_tree_view_get_model (tree_view);

  /*
   * We get the total number of rows including those which are collapsed
   */
  n_rows = get_row_count (tree_model);
  /*
   * We get the total number of columns including those which are not visible
   */
  n_cols = get_n_actual_columns (tree_view);
  return (n_rows * n_cols);
}

static AtkObject*
gail_tree_view_ref_child (AtkObject *obj, 
                          gint      i)
{
  GtkWidget *widget;
  GailTreeView *gailview;
  GailCell *cell;
  GtkTreeView *tree_view;
  GtkTreeModel *tree_model; 
  GtkCellRenderer *renderer;
  GtkTreeIter iter;
  GtkTreeViewColumn *tv_col;
  GtkTreeSelection *selection;
  GtkTreePath *path;
  AtkRegistry *default_registry;
  AtkObjectFactory *factory;
  AtkObject *child;
  AtkObject *parent;
  GtkTreeViewColumn *expander_tv;
  GList *renderer_list;
  GList *l;
  GailContainerCell *container = NULL;
  GailRendererCell *renderer_cell;
  gboolean is_expander, is_expanded, retval;
  gboolean editable = FALSE;

  g_return_val_if_fail (GAIL_IS_TREE_VIEW (obj), NULL);
  g_return_val_if_fail (i >= 0, NULL);

  widget = GTK_ACCESSIBLE (obj)->widget;
  if (widget == NULL)
    /*
     * State is defunct
     */
    return NULL;

  if (i >= gail_tree_view_get_n_children (obj))
    return NULL;

  tree_view = GTK_TREE_VIEW (widget);
  if (i < get_n_actual_columns (tree_view))
    {
      /* A column header is required */
      child = atk_table_get_column_header (ATK_TABLE (obj), i);
      if (child)
        g_object_ref (child);
      return child;
    }

  gailview = GAIL_TREE_VIEW (obj);
  /*
   * Check whether the child is cached
   */
  cell = find_cell (gailview, i);
  if (cell)
    {
      g_object_ref (cell);
      return ATK_OBJECT (cell);
    }

  /*
   * Find the TreePath and GtkTreeViewColumn for the index
   */
  if (!get_path_column_from_index (tree_view, i, &path, &tv_col))
    return NULL;
 
  tree_model = gtk_tree_view_get_model (tree_view);
  retval = gtk_tree_model_get_iter (tree_model, &iter, path);
  g_return_val_if_fail (retval, NULL);

  expander_tv = gtk_tree_view_get_expander_column (tree_view);
  is_expander = FALSE;
  is_expanded = FALSE;
  if (gtk_tree_model_iter_has_child (tree_model, &iter))
    {
      if (expander_tv == tv_col)
        {
          is_expander = TRUE;
          is_expanded = gtk_tree_view_row_expanded (tree_view, path);
        }
    } 
  gtk_tree_view_column_cell_set_cell_data (tv_col, tree_model, &iter, 
                                           is_expander, is_expanded);

  renderer_list = gtk_tree_view_column_get_cell_renderers (tv_col);

  /* If there are more than one renderer in the list, make a container */

  g_assert (renderer_list != NULL);
  if (renderer_list->next)
    {
      GailCell *container_cell;

      container = gail_container_cell_new ();
      g_return_val_if_fail (container, NULL);

      container_cell = GAIL_CELL (container);
      gail_cell_init (container_cell,
                      widget, ATK_OBJECT (gailview), 
                      i);
      /*
       * The GailTreeViewCellInfo structure for the container will be before
       * the ones for the cells so that the first one we find for a position
       * will be for the container
       */
      cell_info_new (gailview, tree_model, path, tv_col, container_cell);
      container_cell->refresh_index = refresh_cell_index;
      parent = ATK_OBJECT (container);
    }
  else
    parent = ATK_OBJECT (gailview);

  child = NULL;

  for (l = renderer_list; l; l = l->next)
    {
      renderer = GTK_CELL_RENDERER (l->data);
    
      if (GTK_IS_CELL_RENDERER_TEXT (renderer))
        g_object_get (G_OBJECT (renderer), "editable", &editable, NULL);

      default_registry = atk_get_default_registry ();
      factory = atk_registry_get_factory (default_registry,
                                          GTK_OBJECT_TYPE (renderer));
      child = atk_object_factory_create_accessible (factory,
                                                    G_OBJECT (renderer));
      g_return_val_if_fail (GAIL_IS_RENDERER_CELL (child), NULL);
      cell = GAIL_CELL (child);
      renderer_cell = GAIL_RENDERER_CELL (child);

      /* Create the GailTreeViewCellInfo structure for this cell */
      cell_info_new (gailview, tree_model, path, tv_col, cell);

      gail_cell_init (cell,
                      widget, parent, 
                      i);

      if (container)
        gail_container_cell_add_child (container, cell);
      else
        cell->refresh_index = refresh_cell_index;

      update_cell_value (renderer_cell, gailview, FALSE);
      /* Add the actions appropriate for this cell */
      add_cell_actions (cell, editable);
  
      /* set state if it is expandable */
      if (is_expander)
        {
          set_cell_expandable (cell);
          if (is_expanded)
            gail_cell_add_state (cell, 
                                 ATK_STATE_EXPANDED,
                                 FALSE);
        }
      /*
       * If the column is visible, sets the cell's state
       */
      if (gtk_tree_view_column_get_visible (tv_col))
        set_cell_visibility (tree_view, cell, tv_col, path, FALSE);
      /*
       * If the row is selected, all cells on the row are selected
       */
      selection = gtk_tree_view_get_selection (tree_view);

      if (gtk_tree_selection_path_is_selected (selection, path))
        gail_cell_add_state (cell, ATK_STATE_SELECTED, FALSE);
    }
  g_list_free (renderer_list); 
  if (container)
    child =  ATK_OBJECT (container);

  if (expander_tv == tv_col)
    {
      AtkRelationSet *relation_set;
      AtkObject *accessible_array[1];
      AtkRelation* relation;
      AtkObject *parent_node;

      relation_set = atk_object_ref_relation_set (ATK_OBJECT (child));

      gtk_tree_path_up (path);
      if (gtk_tree_path_get_depth (path) == 0)
        parent_node = obj;
      else
        {
          gint parent_index;
          gint n_columns;

          n_columns = get_n_actual_columns (tree_view);
          parent_index = get_index (tree_view, path, i % n_columns);
          parent_node = atk_object_ref_accessible_child (obj, parent_index);
        }
      accessible_array[0] = parent_node;
      relation = atk_relation_new (accessible_array, 1,
                                   ATK_RELATION_NODE_CHILD_OF);
      atk_relation_set_add (relation_set, relation);
      if (gtk_tree_path_get_depth (path) != 0)
        g_object_unref (parent_node);
      g_object_unref (relation);
      g_object_unref (relation_set);
    }
  gtk_tree_path_free (path);
  /*
   * We do not increase the reference count here; when g_object_unref() is 
   * called for the cell then cell_destroyed() is called and
   * this removes the cell from the cache.
   */
  return child;
}

/* atktable.h */

static void
atk_component_interface_init (AtkComponentIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->ref_accessible_at_point = gail_tree_view_ref_accessible_at_point;
}

static AtkObject*
gail_tree_view_ref_accessible_at_point (AtkComponent           *component,
                                        gint                   x,
                                        gint                   y,
                                        AtkCoordType           coord_type)
{
  GtkWidget *widget;
  GtkTreeView *tree_view;
  GtkTreePath *path;
  GtkTreeViewColumn *tv_column;
  gint x_pos, y_pos;
  gboolean ret_val;

  widget = GTK_ACCESSIBLE (component)->widget;
  if (widget == NULL)
    /* State is defunct */
    return NULL;

  tree_view = GTK_TREE_VIEW (widget);

  atk_component_get_extents (component, &x_pos, &y_pos, NULL, NULL, coord_type);
  ret_val = gtk_tree_view_get_path_at_pos (tree_view, 
                                           x - x_pos, y - y_pos, 
                                           &path, &tv_column, NULL, NULL);
  if (ret_val)
    {
      gint index, column;

      column = get_column_number (tree_view, tv_column, FALSE);
      index = get_index (tree_view, path, column);
      gtk_tree_path_free (path);

      return gail_tree_view_ref_child (ATK_OBJECT (component), index);
    } 
  else
    {
      g_warning ("gail_tree_view_ref_accessible_at_point: gtk_tree_view_get_path_at_pos () failed\n");
    }
  return NULL;
}
           
/* atktable.h */

static void 
atk_table_interface_init (AtkTableIface *iface)
{
  g_return_if_fail (iface != NULL);
  iface->ref_at = gail_tree_view_table_ref_at;
  iface->get_n_rows = gail_tree_view_get_n_rows;	
  iface->get_n_columns = gail_tree_view_get_n_columns;	
  iface->get_index_at = gail_tree_view_get_index_at;	
  iface->get_column_at_index = gail_tree_view_get_column_at_index;	
  iface->get_row_at_index = gail_tree_view_get_row_at_index;	
  iface->is_row_selected = gail_tree_view_is_row_selected;
  iface->is_selected = gail_tree_view_is_selected;
  iface->get_selected_rows = gail_tree_view_get_selected_rows;
  iface->add_row_selection = gail_tree_view_add_row_selection;
  iface->remove_row_selection = gail_tree_view_remove_row_selection;
  iface->get_column_extent_at = NULL;
  iface->get_row_extent_at = NULL;
  iface->get_row_header = gail_tree_view_get_row_header;
  iface->set_row_header = gail_tree_view_set_row_header;
  iface->get_column_header = gail_tree_view_get_column_header;
  iface->set_column_header = gail_tree_view_set_column_header;
  iface->get_caption = gail_tree_view_get_caption;
  iface->set_caption = gail_tree_view_set_caption;
  iface->get_summary = gail_tree_view_get_summary;
  iface->set_summary = gail_tree_view_set_summary;
  iface->get_row_description = gail_tree_view_get_row_description;
  iface->set_row_description = gail_tree_view_set_row_description;
  iface->get_column_description = gail_tree_view_get_column_description;
  iface->set_column_description = gail_tree_view_set_column_description;
}

static gint
gail_tree_view_get_index_at (AtkTable *table,
                             gint     row,
                             gint     column)
{
  GtkWidget *widget;
  GtkTreeView *tree_view;
  gint actual_column;
  gint n_cols, n_rows;
  GtkTreeIter iter;
  GtkTreePath *path;
  gint index;

  n_cols = atk_table_get_n_columns (table);
  n_rows = atk_table_get_n_rows (table);

  if (row >= n_rows ||
      column >= n_cols)
    return -1;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return -1;

  tree_view = GTK_TREE_VIEW (widget);
  actual_column = get_actual_column_number (tree_view, column);

  set_iter_nth_row (tree_view, &iter, row);
  path = gtk_tree_model_get_path (gtk_tree_view_get_model (tree_view), &iter);

  index = get_index (tree_view, path, actual_column);
  gtk_tree_path_free (path);

  return index;
}

static gint
gail_tree_view_get_column_at_index (AtkTable *table,
                                    gint     index)
{
  GtkWidget *widget;
  GtkTreeView *tree_view;
  gint n_columns;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return -1;

  tree_view = GTK_TREE_VIEW (widget);
  n_columns = get_n_actual_columns (tree_view);

  if (n_columns == 0)
    return 0;
  index = index % n_columns;

  return get_visible_column_number (tree_view, index);
}

static gint
gail_tree_view_get_row_at_index (AtkTable *table,
                                 gint     index)
{
  GtkWidget *widget;
  GtkTreeView *tree_view;
  GtkTreePath *path;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return -1;

  tree_view = GTK_TREE_VIEW (widget);
  if (get_path_column_from_index (tree_view, index, &path, NULL))
    {
      gint row = get_row_from_tree_path (tree_view, path);
      gtk_tree_path_free (path);
      return row;
    }
  else
    return -1;
}

static AtkObject* 
gail_tree_view_table_ref_at (AtkTable *table,
                             gint     row, 
                             gint     column)
{
  GtkWidget *widget;
  GtkTreeView *tree_view;
  gint actual_column, index;
  GtkTreeIter iter;
  GtkTreePath *path;

  index = gail_tree_view_get_index_at (table, row, column);
  if (index == -1)
    return NULL;
  
  return gail_tree_view_ref_child (ATK_OBJECT (table), index);
}

static gint 
gail_tree_view_get_n_rows (AtkTable *table)
{
  GtkWidget *widget;
  GtkTreeView *tree_view;
  GtkTreeModel *tree_model;
  gint n_rows;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return 0;

  tree_view = GTK_TREE_VIEW (widget);
  tree_model = gtk_tree_view_get_model (tree_view);

  if (gtk_tree_model_get_flags (tree_model) & GTK_TREE_MODEL_LIST_ONLY)
   /* 
    * If working with a LIST store, then this is a faster way
    * to get the number of rows.
    */
    n_rows = gtk_tree_model_iter_n_children (tree_model, NULL);
  else
    {
      GtkTreePath *root_tree;

      n_rows = 0;
      root_tree = gtk_tree_path_new_root ();
      iterate_thru_children (tree_view, tree_model,
                             root_tree, NULL, &n_rows, 0);
      g_free (root_tree);
    }

  return n_rows;
}

/*
 * The function get_n_actual_columns returns the number of columns in the 
 * GtkTreeView. i.e. it include both visible and non-visible columns.
 */
static gint 
get_n_actual_columns (GtkTreeView *tree_view)
{
  GList *columns;
  gint n_cols;

  columns = gtk_tree_view_get_columns (tree_view);
  n_cols = g_list_length (columns);
  g_list_free (columns);
  return n_cols;
}

static gint 
gail_tree_view_get_n_columns (AtkTable *table)
{
  GtkWidget *widget;
  GtkTreeView *tree_view;
  GtkTreeViewColumn *tv_col;
  gint n_cols = 0;
  gint i = 0;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return 0;

  tree_view = GTK_TREE_VIEW (widget);
  tv_col = gtk_tree_view_get_column (tree_view, i);

  while (tv_col != NULL) 
    {
      if (gtk_tree_view_column_get_visible (tv_col)) 
        n_cols++;

      i++;
      tv_col = gtk_tree_view_get_column (tree_view, i);
    }

  return n_cols;
}

static gboolean 
gail_tree_view_is_row_selected (AtkTable *table,
                                gint     row)
{
  GtkWidget *widget;
  GtkTreeView *tree_view;
  GtkTreeSelection *selection;
  GtkTreeIter iter;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return FALSE;

  if (row < 0)
    return FALSE;

  tree_view = GTK_TREE_VIEW (widget);

  selection = gtk_tree_view_get_selection (tree_view);

  set_iter_nth_row (tree_view, &iter, row);

  return (gtk_tree_selection_iter_is_selected (selection, &iter));
}

static gboolean 
gail_tree_view_is_selected (AtkTable *table, 
                            gint     row, 
                            gint     column)
{
  return gail_tree_view_is_row_selected (table, row);
}

static gint 
gail_tree_view_get_selected_rows (AtkTable *table,
                                  gint     **rows_selected)
{
  GtkWidget *widget;
  GtkTreeView *tree_view;
  GtkTreeModel *tree_model;
  GtkTreeIter iter;
  GtkTreeSelection *selection;
  GtkTreePath *tree_path;
  gint ret_val = 0;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return 0;

  tree_view = GTK_TREE_VIEW (widget);

  selection = gtk_tree_view_get_selection (tree_view);

  switch (selection->type)
    {
    case GTK_SELECTION_SINGLE:
    case GTK_SELECTION_BROWSE:
      if (gtk_tree_selection_get_selected (selection, &tree_model, &iter))
        {
          gint row;

          if (rows_selected)
            {
              *rows_selected = (gint *)g_malloc (sizeof(gint));
              tree_path = gtk_tree_model_get_path (tree_model, &iter);
              row = get_row_from_tree_path (tree_view, tree_path);
              gtk_tree_path_free (tree_path);

              /* shouldn't ever happen */
              g_return_val_if_fail (row != -1, 0);

              *rows_selected[0] = row;
            }
          ret_val = 1;
        }
      break;
    case GTK_SELECTION_MULTIPLE:
      {
        GPtrArray *array = g_ptr_array_new();

        gtk_tree_selection_selected_foreach (selection,
                                             get_selected_rows,
                                             array);
        ret_val = array->len;

        if (rows_selected && ret_val)
          {
            gint i;
            *rows_selected = (gint *) g_malloc (ret_val * sizeof (gint));

            for (i = 0; i < ret_val; i++)
              {
                gint row;

                tree_path = (GtkTreePath *) g_ptr_array_index (array, i);
                row = get_row_from_tree_path (tree_view, tree_path);
                gtk_tree_path_free (tree_path);
                (*rows_selected)[i] = row;
              }
          }
        g_ptr_array_free (array, FALSE);
      }
      break;
    case GTK_SELECTION_NONE:
      break; 
    }
  return ret_val;
}

static gboolean 
gail_tree_view_add_row_selection (AtkTable *table, 
                                  gint     row)
{
  GtkWidget *widget;
  GtkTreeView *tree_view;
  GtkTreeModel *tree_model;
  GtkTreeSelection *selection;
  GtkTreePath *tree_path;
  GtkTreeIter iter_to_row;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
  {
    /* State is defunct */
    return FALSE;
  }

  if (!gail_tree_view_is_row_selected (table, row))
    {
      tree_view = GTK_TREE_VIEW (widget);
      tree_model = gtk_tree_view_get_model (tree_view);
      selection = gtk_tree_view_get_selection (tree_view);

      if (gtk_tree_model_get_flags (tree_model) & GTK_TREE_MODEL_LIST_ONLY)
        {
          tree_path = gtk_tree_path_new ();
          gtk_tree_path_append_index (tree_path, row);
          gtk_tree_selection_select_path (selection,tree_path);
          gtk_tree_path_free (tree_path);
        }
      else
        { 
          set_iter_nth_row (tree_view, &iter_to_row, row);
          if (&iter_to_row != NULL)
            gtk_tree_selection_select_iter (selection, &iter_to_row);
          else
          return FALSE;
        }
    }

  return gail_tree_view_is_row_selected (table, row);
}

static gboolean 
gail_tree_view_remove_row_selection (AtkTable *table, 
                                     gint     row)
{
  GtkWidget *widget;
  GtkTreeView *tree_view;
  GtkTreeSelection *selection;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return FALSE;

  tree_view = GTK_TREE_VIEW (widget);

  selection = gtk_tree_view_get_selection (tree_view);

  if (gail_tree_view_is_row_selected (table, row)) 
    {
      gtk_tree_selection_unselect_all (selection);
      return TRUE;
    }
  else return FALSE;
}

static AtkObject* 
gail_tree_view_get_row_header (AtkTable *table, 
                               gint     row)
{
  GailTreeViewRowInfo *row_info;

  row_info = get_row_info (table, row);
  if (row_info)
    return row_info->header;
  else
    return NULL;
}

static void
gail_tree_view_set_row_header (AtkTable  *table, 
                               gint      row, 
                               AtkObject *header)
{
  set_row_data (table, row, header, NULL, TRUE);
}

static AtkObject* 
gail_tree_view_get_column_header (AtkTable *table, 
                                  gint     in_col)
{
  GtkWidget *widget, *header_widget;
  GtkTreeView *tree_view;
  GtkTreeViewColumn *tv_col;
  AtkObject *rc;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return NULL;

  tree_view = GTK_TREE_VIEW (widget);
  tv_col = get_column (tree_view, in_col);
  if (tv_col == NULL)
    return NULL;

  /* If the user has set a header object, use that */

  rc = g_object_get_qdata (G_OBJECT (tv_col), quark_column_header_object);

  if (rc == NULL)
    {
      /* If the user has not set a header object, grab the column */
      /* header object defined by the GtkTreeView */

      header_widget = tv_col->button;

      if (header_widget)
        {
          rc = gtk_widget_get_accessible (header_widget);
        }
      else
        rc = NULL;
    }
  return rc;
}

static void
gail_tree_view_set_column_header (AtkTable  *table, 
                                  gint      in_col,
                                  AtkObject *header)
{
  GtkWidget *widget;
  GtkTreeView *tree_view;
  GtkTreeViewColumn *tv_col;
  AtkObject *rc;
  AtkPropertyValues values = { 0, };

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return;

  tree_view = GTK_TREE_VIEW (widget);
  tv_col = get_column (tree_view, in_col);
  if (tv_col == NULL)
     return;

  rc = g_object_get_qdata (G_OBJECT (tv_col),
                          quark_column_header_object);
  if (rc)
    g_object_unref (rc);

  g_object_set_qdata (G_OBJECT (tv_col),
			quark_column_header_object,
			header);
  if (header)
    g_object_ref (header);
  g_value_init (&values.new_value, G_TYPE_INT);
  g_value_set_int (&values.new_value, in_col);

  values.property_name = "accessible_table_column_header";
  g_signal_emit_by_name (table, 
                         "property_change::accessible_table_column_header",
                         &values, NULL);
}

static AtkObject*
gail_tree_view_get_caption (AtkTable	*table)
{
  GailTreeView* obj = GAIL_TREE_VIEW (table);

  return obj->caption;
}

static void
gail_tree_view_set_caption (AtkTable	*table,
                            AtkObject   *caption)
{
  GailTreeView* obj = GAIL_TREE_VIEW (table);
  AtkPropertyValues values = { 0, };
  AtkObject *old_caption;

  old_caption = obj->caption;
  obj->caption = caption;
  if (obj->caption)
    g_object_ref (obj->caption);
  g_value_init (&values.old_value, G_TYPE_POINTER);
  g_value_set_pointer (&values.old_value, old_caption);
  g_value_init (&values.new_value, G_TYPE_POINTER);
  g_value_set_pointer (&values.new_value, obj->caption);

  values.property_name = "accessible_table_caption";
  g_signal_emit_by_name (table, 
                         "property_change::accessible_table_caption", 
                         &values, NULL);
  if (old_caption)
    g_object_unref (old_caption);
}

static G_CONST_RETURN gchar*
gail_tree_view_get_column_description (AtkTable	  *table,
                                       gint       in_col)
{
  GtkWidget *widget;
  GtkTreeView *tree_view;
  GtkTreeViewColumn *tv_col;
  gchar *rc;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return NULL;

  tree_view = GTK_TREE_VIEW (widget);
  tv_col = get_column (tree_view, in_col);
  if (tv_col == NULL)
     return NULL;

  rc = g_object_get_qdata (G_OBJECT (tv_col),
                           quark_column_desc_object);

  if (rc != NULL)
    return rc;
  else
    {
      gchar *title_text;

      g_object_get (tv_col, "title", &title_text, NULL);
      return title_text;
    }
}

static void
gail_tree_view_set_column_description (AtkTable	   *table,
                                       gint        in_col,
                                       const gchar *description)
{
  GtkWidget *widget;
  GtkTreeView *tree_view;
  GtkTreeViewColumn *tv_col;
  AtkPropertyValues values = { 0, };

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return;

  tree_view = GTK_TREE_VIEW (widget);
  tv_col = get_column (tree_view, in_col);
  if (tv_col == NULL)
     return;

  g_object_set_qdata (G_OBJECT (tv_col),
                      quark_column_desc_object,
                      g_strdup (description));
  g_value_init (&values.new_value, G_TYPE_INT);
  g_value_set_int (&values.new_value, in_col);

  values.property_name = "accessible_table_column_description";
  g_signal_emit_by_name (table, 
                         "property_change::accessible_table_column_description",
                         &values, NULL);
}

static G_CONST_RETURN gchar*
gail_tree_view_get_row_description (AtkTable    *table,
                                    gint        row)
{
  GailTreeViewRowInfo *row_info;

  row_info = get_row_info (table, row);
  if (row_info)
    return row_info->description;
  else
    return NULL;
}

static void
gail_tree_view_set_row_description (AtkTable    *table,
                                    gint        row,
                                    const gchar *description)
{
  set_row_data (table, row, NULL, description, FALSE);
}

static AtkObject*
gail_tree_view_get_summary (AtkTable	*table)
{
  GailTreeView* obj = GAIL_TREE_VIEW (table);

  return obj->summary;
}

static void
gail_tree_view_set_summary (AtkTable    *table,
                            AtkObject   *accessible)
{
  GailTreeView* obj = GAIL_TREE_VIEW (table);
  AtkPropertyValues values = { 0, };
  AtkObject *old_summary;

  old_summary = obj->summary;
  obj->summary = accessible;
  if (obj->summary)
    g_object_ref (obj->summary);
  g_value_init (&values.old_value, G_TYPE_POINTER);
  g_value_set_pointer (&values.old_value, old_summary);
  g_value_init (&values.new_value, G_TYPE_POINTER);
  g_value_set_pointer (&values.new_value, obj->summary);

  values.property_name = "accessible_table_summary";
  g_signal_emit_by_name (table, 
                         "property_change::accessible_table_summary",
                         &values, NULL);
  if (old_summary)
    g_object_unref (old_summary);
}

static void
set_row_data (AtkTable    *table, 
              gint        row, 
              AtkObject   *header,
              const gchar *description,
              gboolean    is_header)
{
  GtkWidget *widget;
  GtkTreeView *tree_view;
  GtkTreeModel *tree_model;
  GailTreeView* obj = GAIL_TREE_VIEW (table);
  GailTreeViewRowInfo* row_info;
  GtkTreePath *path;
  GtkTreeIter iter;
  GArray *array;
  gboolean found = FALSE;
  gint i;
  AtkPropertyValues values = { 0, };
  gchar *signal_name;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return;

  tree_view = GTK_TREE_VIEW (widget);
  tree_model = gtk_tree_view_get_model (tree_view);

  set_iter_nth_row (tree_view, &iter, row);
  path = gtk_tree_model_get_path (tree_model, &iter);

  if (obj->row_data == NULL)
    obj->row_data = g_array_sized_new (FALSE, TRUE,
                                       sizeof(GailTreeViewRowInfo *), 0);

  array = obj->row_data;

  for (i = 0; i < array->len; i++)
    {
      GtkTreePath *row_path;

      row_info = g_array_index (array, GailTreeViewRowInfo*, i);
      row_path = gtk_tree_row_reference_get_path (row_info->row_ref);

      if (row_path != NULL)
        {
          if (gtk_tree_path_compare (row_path, path) == 0)
            found = TRUE;

          gtk_tree_path_free (row_path);

          if (found)
            {
              if (is_header)
                {
                  if (row_info->header)
                    g_object_unref (row_info->header);
                  row_info->header = header;
                  if (row_info->header)
                    g_object_ref (row_info->header);
                }
              else
                {
                  g_free (row_info->description);
                  row_info->description = g_strdup (description);
                }
              break;
            }
        }
    }

  if (!found)
    {
      /* if not found */
      row_info = g_malloc (sizeof(GailTreeViewRowInfo));
      row_info->row_ref = gtk_tree_row_reference_new (tree_model, path);
      if (is_header)
        {
          row_info->header = header;
          if (row_info->header)
            g_object_ref (row_info->header);
          row_info->description = NULL;
        }
      else
        {
          row_info->header = NULL;
          row_info->description = g_strdup (description);
        }
      g_array_append_val (array, row_info);
    }
  g_value_init (&values.new_value, G_TYPE_INT);
  g_value_set_int (&values.new_value, row);

  if (is_header)
    {
      values.property_name = "accessible_table_row_header";
      signal_name = "property_change::accessible_table_row_header";
    }
  else
    {
      values.property_name = "accessible_table_row_description";
      signal_name = "property-change::accessible_table_row_description";
    }
  g_signal_emit_by_name (table, 
                         signal_name,
                         &values, NULL);

  gtk_tree_path_free (path);
}


static GailTreeViewRowInfo*
get_row_info (AtkTable    *table,
              gint        row)
{
  GtkWidget *widget;
  GtkTreeView *tree_view;
  GtkTreeModel *tree_model;
  GailTreeView* obj = GAIL_TREE_VIEW (table);
  GtkTreePath *path;
  GtkTreeIter iter;
  GArray *array;
  GailTreeViewRowInfo *rc = NULL;

  widget = GTK_ACCESSIBLE (table)->widget;
  if (widget == NULL)
    /* State is defunct */
    return NULL;

  tree_view = GTK_TREE_VIEW (widget);
  tree_model = gtk_tree_view_get_model (tree_view);

  set_iter_nth_row (tree_view, &iter, row);
  path = gtk_tree_model_get_path (tree_model, &iter);
  array = obj->row_data;

  if (array != NULL)
    {
      GailTreeViewRowInfo *row_info;
      GtkTreePath *row_path;
      gint i;

      for (i = 0; i < array->len; i++)
        {
          row_info = g_array_index (array, GailTreeViewRowInfo*, i);
          row_path = gtk_tree_row_reference_get_path (row_info->row_ref);

          if (row_path != NULL)
            {
              if (gtk_tree_path_compare (row_path, path) == 0)
                rc = row_info;

              gtk_tree_path_free (row_path);

              if (rc != NULL)
                break;
            }
        }
    }

  gtk_tree_path_free (path);
  return rc;
}
/* atkselection.h */

static void atk_selection_interface_init (AtkSelectionIface *iface)
{
  g_return_if_fail (iface != NULL);
  iface->clear_selection = gail_tree_view_clear_selection;
  iface->ref_selection = gail_tree_view_ref_selection;
  iface->get_selection_count = gail_tree_view_get_selection_count;
  iface->is_child_selected = gail_tree_view_is_child_selected;
}

static gboolean
gail_tree_view_clear_selection (AtkSelection *selection)
{
  GtkWidget *widget;
  GtkTreeView *tree_view;
  GtkTreeSelection *tree_selection;

  widget = GTK_ACCESSIBLE (selection)->widget;
  if (widget == NULL)
    /* State is defunct */
    return FALSE;
 
  tree_view = GTK_TREE_VIEW (widget);

  tree_selection = gtk_tree_view_get_selection (tree_view);
  gtk_tree_selection_unselect_all (tree_selection);  

  return TRUE;
}

static AtkObject*  
gail_tree_view_ref_selection (AtkSelection *selection, 
                              gint         i)
{
  AtkTable *table;
  gint row;
  gint n_selected;
  gint n_columns;
  gint *selected;

  table = ATK_TABLE (selection);
  n_columns = gail_tree_view_get_n_columns (table);
  n_selected = gail_tree_view_get_selected_rows (table, &selected);
  if (i >= n_columns * n_selected)
    return NULL;

  row = selected[i / n_columns];
  g_free (selected);

  return gail_tree_view_table_ref_at (table, row, i % n_columns);
}

static gint
gail_tree_view_get_selection_count (AtkSelection *selection)
{
  AtkTable *table;
  gint n_selected;

  table = ATK_TABLE (selection);
  n_selected = gail_tree_view_get_selected_rows (table, NULL);
  if (n_selected > 0)
    n_selected *= gail_tree_view_get_n_columns (table);
  return n_selected;
}

static gboolean
gail_tree_view_is_child_selected (AtkSelection *selection, 
                                  gint         i)
{
  GtkWidget *widget;
  gint row;

  widget = GTK_ACCESSIBLE (selection)->widget;
  if (widget == NULL)
    /* State is defunct */
    return FALSE;

  row = atk_table_get_row_at_index (ATK_TABLE (selection), i);

  return gail_tree_view_is_row_selected (ATK_TABLE (selection), row);
}


static void gail_cell_parent_interface_init (GailCellParentIface *iface)
{
  g_return_if_fail (iface);

  iface->get_cell_extents = gail_tree_view_get_cell_extents;
  iface->get_cell_area = gail_tree_view_get_cell_area;
}

static void
gail_tree_view_get_cell_extents (GailCellParent *parent,
                                 GailCell       *cell,
                                 gint           *x,
                                 gint           *y,
                                 gint           *width,
                                 gint           *height,
                                 AtkCoordType   coord_type)
{
  GtkWidget *widget;
  GtkTreeView *tree_view;
  GdkWindow *bin_window;
  GdkRectangle cell_rect;
  gint w_x, w_y;

  widget = GTK_ACCESSIBLE (parent)->widget;
  if (widget == NULL)
    /* State is defunct */
    return;

  tree_view = GTK_TREE_VIEW (widget);
  gail_tree_view_get_cell_area (parent, cell, &cell_rect);
  bin_window = gtk_tree_view_get_bin_window (tree_view);
  gdk_window_get_origin (bin_window, &w_x, &w_y);

  if (coord_type == ATK_XY_WINDOW)
    {
      GdkWindow *window;
      gint x_toplevel, y_toplevel;

      window = gdk_window_get_toplevel (bin_window);
      gdk_window_get_origin (window, &x_toplevel, &y_toplevel);

      w_x -= x_toplevel;
      w_y -= y_toplevel;
    }

  *width = cell_rect.width;
  *height = cell_rect.height;
  if (is_cell_showing (tree_view, &cell_rect))
    {
      *x = cell_rect.x + w_x;
      *y = cell_rect.y + w_y;
    }
  else
    {
      *x = G_MININT;
      *y = G_MININT;
    }
}

#define EXTRA_EXPANDER_PADDING 4

static void
gail_tree_view_get_cell_area (GailCellParent *parent,
                              GailCell       *cell,
                              GdkRectangle   *cell_rect)
{
  GtkWidget *widget;
  GtkTreeView *tree_view;
  GtkTreeViewColumn *tv_col;
  GtkTreePath *path;
  gint index;
  AtkObject *parent_cell;
  GailCell *top_cell;
  gint x, y, width, height;

  widget = GTK_ACCESSIBLE (parent)->widget;
  if (widget == NULL)
    /* State is defunct */
    return;

  tree_view = GTK_TREE_VIEW (widget);

  parent_cell = atk_object_get_parent (ATK_OBJECT (cell));
  if (parent_cell != ATK_OBJECT (parent))
    {
      /*
       * GailCell is in a GailContainerCell
       */
      top_cell = GAIL_CELL (parent_cell);
    }
  else
    {
      top_cell = cell;
    }
  index = atk_object_get_index_in_parent (ATK_OBJECT (top_cell));
  if (get_path_column_from_index (tree_view, index, &path, &tv_col))
    {
      GtkTreeViewColumn *expander_column;
      gint focus_line_width;

      gtk_tree_view_get_cell_area (tree_view, path, tv_col, cell_rect);
      expander_column = gtk_tree_view_get_expander_column (tree_view);
      if (expander_column == tv_col)
        {
          gint expander_size;

          gtk_widget_style_get (widget,
                                "expander_size", &expander_size,
                                NULL);

          cell_rect->x += expander_size + EXTRA_EXPANDER_PADDING;
          cell_rect->width -= expander_size + EXTRA_EXPANDER_PADDING;
        }
      gtk_widget_style_get (widget,
                            "focus-line-width", &focus_line_width,
                            NULL);

      cell_rect->x += focus_line_width;
      cell_rect->width -= 2 * focus_line_width; 

      gtk_tree_path_free (path);
    }
}

/* signal handling */

static gboolean
gail_tree_view_expand_row_gtk (GtkTreeView       *tree_view,
                               GtkTreeIter        *iter,
                               GtkTreePath        *path)
{
  AtkObject *atk_obj = gtk_widget_get_accessible (GTK_WIDGET (tree_view));
  GailTreeView *gailview = GAIL_TREE_VIEW (atk_obj);
  GailTreeViewIdleData *data;

  /*
   * The visible rectangle has not been updated when this signal is emitted
   * so we process the signal when the GTK processing is completed
   */
  data = g_new (GailTreeViewIdleData, 1);
  data->tree_view = gailview;
  data->path = gtk_tree_path_copy (path);
  gtk_idle_add (idle_expand_row, data);

  return FALSE;
}

static gint
idle_expand_row (gpointer data)
{
  GailTreeViewIdleData *idle_data = (GailTreeViewIdleData *) data;
  GailTreeView *gailview = idle_data->tree_view;
  GtkTreePath *path = idle_data->path;
  GtkTreeView *tree_view = GTK_TREE_VIEW (GTK_ACCESSIBLE (gailview)->widget);
  GtkTreeIter iter;
  GtkTreeModel *tree_model;
  gint n_inserted, row;

  tree_model = gtk_tree_view_get_model(tree_view);
  if (!gtk_tree_model_get_iter (tree_model, &iter, path))
    g_assert_not_reached ();

  /*
   * Update visibility of cells below expansion row
   */
  traverse_cells (gailview, path, FALSE, FALSE);
  /*
   * Figure out number of visible children, the following test
   * should not fail
   */
  if (gtk_tree_model_iter_has_child (tree_model, &iter))
    {
      GtkTreePath *path_copy;

     /*
      * By passing path into this function, we find the number of
      * visible children of path.
      */
      path_copy = gtk_tree_path_copy (path);
      gtk_tree_path_append_index(path_copy, 0);

      n_inserted = 0;
      iterate_thru_children (tree_view, tree_model,
                             path_copy, NULL, &n_inserted, 0);
      gtk_tree_path_free (path_copy);
    }
  else
    g_assert_not_reached ();

  /* Set expand state */
  set_expand_state (tree_view, tree_model, gailview, path, FALSE);

  row = get_row_from_tree_path (tree_view, path);

  /* shouldn't ever happen */
  if (row == -1)
    g_assert_not_reached ();

  /* Must add 1 because the "added rows" are below the row being expanded */
  row += 1;
  
  g_signal_emit_by_name (gailview, "row_inserted", row, n_inserted);

  gtk_tree_path_free (path);
  g_free (data);
  return FALSE;
}

static gboolean
gail_tree_view_collapse_row_gtk (GtkTreeView       *tree_view,
                                 GtkTreeIter        *iter,
                                 GtkTreePath        *path)
{
  GtkTreeModel *tree_model;
  AtkObject *atk_obj = gtk_widget_get_accessible (GTK_WIDGET (tree_view));
  GailTreeView *gailview = GAIL_TREE_VIEW (atk_obj);
  gint row;

  tree_model = gtk_tree_view_get_model (tree_view);

  clean_rows (gailview);

  /*
   * Update visibility of cells below collapsed row
   */
  traverse_cells (gailview, path, FALSE, FALSE);
  /* Set collapse state */
  set_expand_state (tree_view, tree_model, gailview, path, FALSE);

  g_return_val_if_fail (gailview->n_children_deleted, FALSE);
  row = get_row_from_tree_path (tree_view, path);
  g_return_val_if_fail (row != -1, FALSE);
  g_signal_emit_by_name (atk_obj, "row_deleted", row, 
                         gailview->n_children_deleted);
  gailview->n_children_deleted = 0;
  return FALSE;
}

static void
gail_tree_view_size_allocate_gtk (GtkWidget     *widget,
                                  GtkAllocation *allocation)
{
  AtkObject *atk_obj = gtk_widget_get_accessible (widget);
  GailTreeView *gailview = GAIL_TREE_VIEW (atk_obj);

  /*
   * If the size allocation changes, the visibility of cells may change so
   * update the cells visibility.
   */
  traverse_cells (gailview, FALSE, FALSE, FALSE);
}
static void
gail_tree_view_changed_gtk (GtkTreeSelection *selection,
                            gpointer         data)
{
  GailTreeView *gailview;
  GtkTreeView *tree_view;
  GtkWidget *widget;
  GList *cell_list;
  GList *l;
  GailTreeViewCellInfo *info;
  GtkTreeSelection *tree_selection;
  GtkTreePath *path;

  gailview = GAIL_TREE_VIEW (data);
  cell_list = gailview->cell_data;
  widget = GTK_ACCESSIBLE (gailview)->widget;
  if (widget == NULL)
    /*
     * destroy signal emitted for widget
     */
    return;
  tree_view = GTK_TREE_VIEW (widget);

  tree_selection = gtk_tree_view_get_selection (tree_view);

  for (l = cell_list; l; l = l->next)
    {
      info = (GailTreeViewCellInfo *) (l->data);

      gail_cell_remove_state (info->cell, ATK_STATE_SELECTED, TRUE); 

      path = gtk_tree_row_reference_get_path (info->cell_row_ref);
      if (gtk_tree_selection_path_is_selected (tree_selection, path))
        gail_cell_add_state (info->cell, ATK_STATE_SELECTED, TRUE); 
      gtk_tree_path_free (path);
    }
  g_signal_emit_by_name (gailview, "selection_changed");
}

static void
columns_changed (GtkTreeView *tree_view)
{
  AtkObject *atk_obj = gtk_widget_get_accessible (GTK_WIDGET(tree_view));
  GailTreeView *gailview = GAIL_TREE_VIEW (atk_obj);
  GList *tv_cols, *tmp_list;
  gboolean column_found;
  gboolean move_found = FALSE;
  gboolean stale_set = FALSE;
  gint column_count = 0;
  gint i;

 /*
  * This function must determine if the change is an add, delete or
  * a move based upon its cache of TreeViewColumns in
  * gailview->col_data
  */
  tv_cols = gtk_tree_view_get_columns (tree_view);

  /* check for adds or moves */
  for (tmp_list = tv_cols; tmp_list; tmp_list = tmp_list->next)
    {
      column_found = FALSE;

      for (i = 0; i < gailview->col_data->len; i++)
        {

          if ((GtkTreeViewColumn *)tmp_list->data ==
              (GtkTreeViewColumn *)g_array_index (gailview->col_data,
               GtkTreeViewColumn *, i))
            {
              column_found = TRUE;

              /* If the column isn't in the same position, a move happened */
              if (!move_found && i != column_count)
                {
                  if (!stale_set)
                    {
                      /* Set all rows to ATK_STATE_STALE */
                      traverse_cells (gailview, NULL, TRUE, FALSE);
                      stale_set = TRUE;
                    }
  
                  /* Just emit one column reordered signal when a move happens */
                  g_signal_emit_by_name (atk_obj, "column_reordered");
                  move_found = TRUE;
                }

              break;
            }
        }

     /*
      * If column_found is FALSE, then an insert happened for column
      * number column_count
      */
      if (!column_found)
        {
          gint n_cols, n_rows, row;

          if (!stale_set)
            {
              /* Set all rows to ATK_STATE_STALE */
              traverse_cells (gailview, NULL, TRUE, FALSE);
              stale_set = TRUE;
            }

          /* Generate column-inserted signal */
          g_signal_emit_by_name (atk_obj, "column_inserted", column_count, 1);

          /* Generate children-changed signals */
          n_rows = get_row_count (gtk_tree_view_get_model (tree_view));
          n_cols = get_n_actual_columns (tree_view);
          for (row = 0; row < n_rows; row++)
            {
             /*
              * Pass NULL as the child object, i.e. 4th argument.
              */
              g_signal_emit_by_name (atk_obj, "children_changed::add",
                                    ((row * n_cols) + column_count), NULL, NULL);
            }
        }

      column_count++;
    }

  /* check for deletes */
  for (i = 0; i < gailview->col_data->len; i++)
    {
      column_found = FALSE;

      for (tmp_list = tv_cols; tmp_list; tmp_list = tmp_list->next)
        {
            if ((GtkTreeViewColumn *)tmp_list->data ==
                (GtkTreeViewColumn *)g_array_index (gailview->col_data,
                 GtkTreeViewColumn *, i))
              {
                column_found = TRUE;
                break;
              }
        }

       /*
        * If column_found is FALSE, then a delete happened for column
        * number i
        */
      if (!column_found)
        {
          gint n_rows, n_cols, row;

          clean_cols (gailview,
                      (GtkTreeViewColumn *)g_array_index (gailview->col_data,
	              GtkTreeViewColumn *, i));

          if (!stale_set)
            {
              /* Set all rows to ATK_STATE_STALE */
              traverse_cells (gailview, NULL, TRUE, FALSE);
              stale_set = TRUE;
            }

          /* Generate column-deleted signal */
          g_signal_emit_by_name (atk_obj, "column_deleted", i, 1);

          /* Generate children-changed signals */
          n_rows = get_row_count (gtk_tree_view_get_model (tree_view));
          n_cols = get_n_actual_columns (tree_view);
          for (row = 0; row < n_rows; row++)
            {
             /*
              * Pass NULL as the child object, 4th argument.
              */
              g_signal_emit_by_name (atk_obj, "children_changed::remove",
                                    ((row * n_cols) + column_count), NULL, NULL);
            }
        }
    }
   
  /* rebuild the array */

  g_array_free (gailview->col_data, FALSE);
  gailview->col_data = g_array_sized_new (FALSE, TRUE,
    sizeof(GtkTreeViewColumn *), 0);

  for (tmp_list = tv_cols; tmp_list; tmp_list = tmp_list->next)
     g_array_append_val (gailview->col_data, tmp_list->data);
  g_list_free (tv_cols);
}

static void
cursor_changed (GtkTreeView *tree_view)
{
  /*
   * We notify the focus change in a idle handler so that the processing
   * of the cursor change is completed when the focus handler is called.
   * This will allow actions to be called in the focus handler
   */ 
  /*
   * This signal is not currently emitted when cursor is moved left or right.
   * See bug 64361.
   */
  gtk_idle_add (idle_cursor_changed, tree_view);
}

static gint
idle_cursor_changed (gpointer data)
{
  GtkTreeView *tree_view;
  GtkWidget *widget;
  GtkWidget *toplevel;
  AtkObject *atk_obj;

  tree_view = GTK_TREE_VIEW (data);
  widget = GTK_WIDGET (tree_view);
  toplevel = gtk_widget_get_toplevel (widget);

  /*
   * If another window has focus do not report focus in cell.
   */
  if (toplevel && GTK_IS_WINDOW (toplevel))
    if (!GTK_WINDOW (toplevel)->has_focus)
      return FALSE;

  atk_obj = gail_tree_view_ref_focus_cell (tree_view);
  if (atk_obj)
    {
      /*
       * Notify that the cell has focus
       */
      atk_focus_tracker_notify (atk_obj);
      g_object_unref (atk_obj);
    }

  return FALSE;
}

static void
model_row_changed (GtkTreeModel *tree_model,
                   GtkTreePath  *path, 
                   GtkTreeIter  *iter,
                   gpointer     user_data)
{
  GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
  GailTreeView *gailview;
  GtkTreePath *cell_path;
  GList *l;
  GailTreeViewCellInfo *cell_info;
 
  gailview = GAIL_TREE_VIEW (gtk_widget_get_accessible (GTK_WIDGET (tree_view)));

  /* Loop through our cached cells */
  /* Must loop through them all */
  for (l = gailview->cell_data; l; l = l->next)
    {
      cell_info = (GailTreeViewCellInfo *) l->data;
      cell_path = gtk_tree_row_reference_get_path (cell_info->cell_row_ref);

      if (cell_path != NULL)
        {
          if (gtk_tree_path_compare (cell_path, path) == 0)
            {
              if (GAIL_IS_RENDERER_CELL (cell_info->cell))
                {
                  update_cell_value (GAIL_RENDERER_CELL (cell_info->cell),
                                     gailview, TRUE);
                }
            }
          gtk_tree_path_free (cell_path);
        }
    }
  g_signal_emit_by_name (gailview, "visible-data-changed");
}

static void
column_visibility_changed (GObject    *object,
                           GParamSpec *pspec,
                           gpointer   user_data)
{
  if (strcmp (pspec->name, "visible") == 0)
    {
      /*
       * A column has been made visible or invisible
       *
       * We update our cache of cells and emit model_changed signal
       */ 
      GtkTreeView *tree_view = (GtkTreeView *)user_data;
      GailTreeView *gailview;
      GList *l;
      GailTreeViewCellInfo *cell_info;
      GtkTreeViewColumn *this_col = GTK_TREE_VIEW_COLUMN (object);
      GtkTreeViewColumn *tv_col;

      gailview = GAIL_TREE_VIEW (gtk_widget_get_accessible (GTK_WIDGET (tree_view))
);
      g_signal_emit_by_name (gailview, "model_changed");

      for (l = gailview->cell_data; l; l = l->next)
        {
          cell_info = (GailTreeViewCellInfo *) l->data;
          tv_col = cell_info->cell_col_ref;
          if (tv_col == this_col)
            {
              GtkTreePath *row_path;
      
              row_path = gtk_tree_row_reference_get_path (cell_info->cell_row_ref);
              if (GAIL_IS_RENDERER_CELL (cell_info->cell))
                {
                  if (gtk_tree_view_column_get_visible (tv_col))
                    set_cell_visibility (tree_view, 
                                         cell_info->cell, 
                                         tv_col, row_path, FALSE);
                  else
                    {
                      gail_cell_remove_state (cell_info->cell, 
                                              ATK_STATE_VISIBLE, TRUE);
                      gail_cell_remove_state (cell_info->cell, 
                                              ATK_STATE_SHOWING, TRUE);
                    }
                }
              gtk_tree_path_free (row_path);
            }
      
        }
    }
}

/*
 * This is the signal handler for the "destroy" signal for a GtkTreeViewColumn
 *
 * We check whether we have stored column description or column header
 * and if so we get rid of it.
 */
static void
column_destroy (GtkObject *obj)
{
  GtkTreeViewColumn *tv_col = GTK_TREE_VIEW_COLUMN (obj);
  AtkObject *header;
  gchar *desc;

  header = g_object_get_qdata (G_OBJECT (tv_col),
                          quark_column_header_object);
  if (header)
    g_object_unref (header);
  desc = g_object_get_qdata (G_OBJECT (tv_col),
                           quark_column_desc_object);
  g_free (desc); 
}

static void
model_row_inserted (GtkTreeModel *tree_model,
                    GtkTreePath  *path, 
                    GtkTreeIter  *iter, 
                    gpointer     user_data)
{
  GtkTreeView *tree_view = (GtkTreeView *)user_data;
  GtkTreePath *path_copy;
  AtkObject *atk_obj = gtk_widget_get_accessible (GTK_WIDGET (tree_view));
  GailTreeView *gailview = GAIL_TREE_VIEW (atk_obj);
  gint row, n_inserted, child_row;

  /* Check to see if row is visible */
  row = get_row_from_tree_path (tree_view, path);

 /*
  * A row insert is not necessarily visible.  For example,
  * a row can be draged & dropped into another row, which
  * causes an insert on the model that isn't visible in the
  * view.  Only generate a signal if the inserted row is
  * visible.
  */
  if (row != -1)
    {
      GtkTreeIter iter;
      gint n_cols, col;

      gtk_tree_model_get_iter (tree_model, &iter, path);

      /* Figure out number of visible children. */
      if (gtk_tree_model_iter_has_child (tree_model, &iter))
        {
         /*
          * By passing path into this function, we find the number of
          * visible children of path.
          */
          n_inserted = 0;
          iterate_thru_children (tree_view, tree_model,
                                 path, NULL, &n_inserted, 0);

          /* Must add one to include the row that is being added */
          n_inserted++;
        }
      else
      n_inserted = 1;

      /* Set rows below the inserted row to ATK_STATE_STALE */
      traverse_cells (gailview, path, TRUE, TRUE);

      /* Generate row-inserted signal */
      g_signal_emit_by_name (atk_obj, "row_inserted", row, n_inserted);

      /* Generate children-changed signals */
      n_cols = gail_tree_view_get_n_columns (ATK_TABLE (atk_obj));
      for (child_row = row; child_row < (row + n_inserted); child_row++)
        {
          for (col = 0; col < n_cols; col++)
            {
             /*
              * Pass NULL as the child object, i.e. 4th argument
              */
              g_signal_emit_by_name (atk_obj, "children_changed::add",
                                    ((row * n_cols) + col), NULL, NULL);
            }
        }
    }
  else
    {
     /*
      * The row has been inserted inside another row.  This can
      * cause a row that previously couldn't be expanded to now
      * be expandable.
      */
      path_copy = gtk_tree_path_copy (path);
      gtk_tree_path_up (path_copy);
      set_expand_state (tree_view, tree_model, gailview, path_copy, TRUE);
      gtk_tree_path_free (path_copy);
    }
}

static void
model_row_deleted (GtkTreeModel *tree_model,
                   GtkTreePath  *path, 
                   gpointer     user_data)
{
  GtkTreeView *tree_view = (GtkTreeView *)user_data;
  GtkTreePath *path_copy;
  AtkObject *atk_obj = gtk_widget_get_accessible (GTK_WIDGET (tree_view));
  GailTreeView *gailview = GAIL_TREE_VIEW (atk_obj);
  gint row;

  clean_rows (gailview);

  /* Set rows at or below the specified row to ATK_STATE_STALE */
  traverse_cells (gailview, path, TRUE, TRUE);

  /*
   * If deleting a row with a depth > 1, then this may affect the
   * expansion/contraction of its parent(s).  Make sure this is
   * handled.
   */
  if (gtk_tree_path_get_depth (path) > 1)
    {
      path_copy = gtk_tree_path_copy (path);
      gtk_tree_path_up (path_copy);
      set_expand_state (tree_view, tree_model, gailview, path_copy, TRUE);
      gtk_tree_path_free (path_copy);
    }
  row = get_row_from_tree_path (tree_view, path);
  /*
   * If the row which is deleted is not visible because it is a child of
   * a collapsed row then row will be -1
   */
  if (row > 0)
    g_signal_emit_by_name (atk_obj, "row_deleted", row, 
                           gailview->n_children_deleted + 1);
  gailview->n_children_deleted = 0;
}

/* 
 * This function gets called when a row is deleted or when rows are
 * removed from the view due to a collapse event.  Note that the
 * count is the number of visible *children* of the deleted row,
 * so it does not include the row being deleted.
 *
 * As this function is called before the rows are removed we just note the
 * number of rows and then deal with it when we get a notification that
 * rows were deleted or collapsed.
 */
static void
destroy_count_func (GtkTreeView *tree_view, 
                    GtkTreePath *path,
                    gint        count,
                    gpointer    user_data)
{
  AtkObject *atk_obj = gtk_widget_get_accessible (GTK_WIDGET (tree_view));
  GailTreeView *gailview = GAIL_TREE_VIEW (atk_obj);

  g_return_if_fail (gailview->n_children_deleted == 0);
  gailview->n_children_deleted = count;
}

static void 
model_rows_reordered (GtkTreeModel *tree_model,
                      GtkTreePath  *path, 
                      GtkTreeIter  *iter,
                      gint         *new_order, 
                      gpointer     user_data)
{
  GtkTreeView *tree_view = (GtkTreeView *)user_data;
  AtkObject *atk_obj = gtk_widget_get_accessible (GTK_WIDGET (tree_view));
  GailTreeView *gailview = GAIL_TREE_VIEW (atk_obj);

  traverse_cells (gailview, NULL, TRUE, FALSE);

  g_signal_emit_by_name (atk_obj, "row_reordered");
}

static void
adjustment_changed (GtkAdjustment *adjustment, 
                    GtkTreeView   *tree_view)
{
  AtkObject *atk_obj;
  GailTreeView* obj;

  /*
   * The scrollbars have changed
   */
  atk_obj = gtk_widget_get_accessible (GTK_WIDGET (tree_view));
  obj = GAIL_TREE_VIEW (atk_obj);

  traverse_cells (obj, NULL, FALSE, FALSE);
  g_signal_emit_by_name (atk_obj, "visible_data_changed");
}

static void
set_cell_visibility (GtkTreeView       *tree_view,
                     GailCell          *cell,
                     GtkTreeViewColumn *tv_col,
                     GtkTreePath       *tree_path,
                     gboolean          emit_signal)
{
  GdkRectangle cell_rect;

  /* Get these three values in tree coords */
  gtk_tree_view_get_cell_area (tree_view, tree_path, tv_col, &cell_rect);
  if (cell_rect.height > 0)
    {
      /*
       * The height will be zero for a cell for which an antecedent is not 
       * expanded
       */
      gail_cell_add_state (cell, ATK_STATE_VISIBLE, emit_signal);
      if (is_cell_showing (tree_view, &cell_rect))
        gail_cell_add_state (cell, ATK_STATE_SHOWING, emit_signal);
      else
        gail_cell_remove_state (cell, ATK_STATE_SHOWING, emit_signal);
    }
  else
    {
      gail_cell_remove_state (cell, ATK_STATE_VISIBLE, emit_signal);
      gail_cell_remove_state (cell, ATK_STATE_SHOWING, emit_signal);
    }
}

static gboolean 
is_cell_showing (GtkTreeView   *tree_view,
                 GdkRectangle  *cell_rect)
{
  GdkRectangle rect, *visible_rect;
 /*
  * Right now a cell is considered "SHOWING" if any part of the cell
  * is in the visible area.  Other ways we could do this is by a cell's
  * midpoint or if the cell is fully in the visible range.  Since the
  * cell_rect contains height/width/x/y positions of the cell, any of
  * these is easy to compute.
  *
  * It is assumed that both the GtkTreeView's visible rectangle and the
  * cell's rectangle are in tree window coordinates.
  */
  visible_rect = &rect;
  gtk_tree_view_get_visible_rect (tree_view, visible_rect);
  if (((cell_rect->x + cell_rect->width) < visible_rect->x) ||
     ((cell_rect->y + cell_rect->height) < (visible_rect->y)) ||
     (cell_rect->x > (visible_rect->x + visible_rect->width)) ||
     (cell_rect->y > (visible_rect->y + visible_rect->height)))
    return FALSE;
  else
    return TRUE;
}

/* Misc Public */

/*
 * This function is called when a cell's flyweight is created in
 * gail_tree_view_table_ref_at with emit_change_signal set to FALSE
 * and in model_row_changed() on receipt of "row-changed" signal when 
 * emit_change_signal is set to TRUE
 */
static gboolean
update_cell_value (GailRendererCell *renderer_cell,
                   GailTreeView     *gailview,
                   gboolean         emit_change_signal)
{
  GailTreeViewCellInfo *cell_info;
  GtkTreeView *tree_view;
  GtkTreeModel *tree_model;
  GtkTreePath *path;
  GtkTreeIter iter;
  GList *renderers, *cur_renderer;
  GParamSpec *spec;
  GailRendererCellClass *gail_renderer_cell_class;
  GtkCellRendererClass *gtk_cell_renderer_class;
  GailCell *cell;
  gchar **prop_list;
  AtkObject *parent;
  gboolean is_expander, is_expanded;
  
  gail_renderer_cell_class = GAIL_RENDERER_CELL_GET_CLASS (renderer_cell);
  gtk_cell_renderer_class = GTK_CELL_RENDERER_GET_CLASS (renderer_cell->renderer);
  prop_list = gail_renderer_cell_class->property_list;

  cell = GAIL_CELL (renderer_cell);
  cell_info = find_cell_info (gailview, cell, NULL);
  g_return_val_if_fail (cell_info, FALSE);
  g_return_val_if_fail (cell_info->cell_col_ref, FALSE);
  g_return_val_if_fail (cell_info->cell_row_ref, FALSE);

  if (emit_change_signal)
    {
      tree_view = GTK_TREE_VIEW (GTK_ACCESSIBLE (gailview)->widget);
      tree_model = gtk_tree_view_get_model (tree_view);
      path = gtk_tree_row_reference_get_path (cell_info->cell_row_ref);
      if (path == NULL)
        return FALSE;

      gtk_tree_model_get_iter (tree_model, &iter, path);
      is_expander = FALSE;
      is_expanded = FALSE;
      if (gtk_tree_model_iter_has_child (tree_model, &iter))
        {
          GtkTreeViewColumn *expander_tv;

          expander_tv = gtk_tree_view_get_expander_column (tree_view);
          if (expander_tv == cell_info->cell_col_ref)
            {
              is_expander = TRUE;
              is_expanded = gtk_tree_view_row_expanded (tree_view, path);
            }
        } 
      gtk_tree_path_free (path);
      gtk_tree_view_column_cell_set_cell_data (cell_info->cell_col_ref,
                                  tree_model, &iter, is_expander, is_expanded);
    }
  renderers = gtk_tree_view_column_get_cell_renderers (cell_info->cell_col_ref);
  g_return_val_if_fail (renderers, FALSE);

  /*
   * If the cell is in a container, it's index is used to find the renderer 
   * in the list
   */
  parent = atk_object_get_parent (ATK_OBJECT (cell));
  if (GAIL_IS_CONTAINER_CELL (parent))
    cur_renderer = g_list_nth (renderers, cell->index);
  
  /*
   * Otherwise, we assume that the cell is represented by the first renderer 
   * in the list
   */

  else
    cur_renderer = renderers;

  g_return_val_if_fail (cur_renderer != NULL, FALSE);

  while (*prop_list)
    {
      spec = g_object_class_find_property
                       (G_OBJECT_CLASS (gtk_cell_renderer_class), *prop_list);

      if (spec != NULL)
        {
          GValue value = { 0, };

          g_value_init (&value, spec->value_type);
          g_object_get_property (cur_renderer->data, *prop_list, &value);
          g_object_set_property (G_OBJECT (renderer_cell->renderer),
                                 *prop_list, &value);
        }
      else
        g_warning ("Invalid property: %s\n", *prop_list);
      prop_list++;
    }
  g_list_free (renderers);
  return gail_renderer_cell_update_cache (renderer_cell, emit_change_signal);
}

static void 
set_iter_nth_row (GtkTreeView *tree_view, 
                  GtkTreeIter *iter, 
                  gint        row)
{
  GtkTreeModel *tree_model;
  
  tree_model = gtk_tree_view_get_model (tree_view);
  gtk_tree_model_get_iter_root (tree_model, iter);
  iter = return_iter_nth_row (tree_view, tree_model, iter, 0 , row);
}

static gint 
get_row_from_tree_path (GtkTreeView *tree_view,
                        GtkTreePath *path)
{
  GtkTreeModel *tree_model;
  GtkTreePath *root_tree;
  gint row;

  tree_model = gtk_tree_view_get_model (tree_view);

  if (gtk_tree_model_get_flags (tree_model) & GTK_TREE_MODEL_LIST_ONLY)
    row = gtk_tree_path_get_indices (path)[0];
  else
    {
      root_tree = gtk_tree_path_new_root ();
      row = 0;
      iterate_thru_children (tree_view, tree_model, root_tree, path, &row, 0);
      gtk_tree_path_free (root_tree);
    }

  return row;
}

/* Misc Private */

/*
 * Get the specified GtkTreeViewColumn in the GtkTreeView.
 * Only visible columns are considered.
 */
static GtkTreeViewColumn* 
get_column (GtkTreeView *tree_view, 
            gint        in_col)
{
  GtkTreeViewColumn *tv_col;
  gint n_cols = -1;
  gint i = 0;
 
  if (in_col < 0)
    {
       g_warning ("Request for invalid column %d\n", in_col);
       return NULL;
    }

  tv_col = gtk_tree_view_get_column (tree_view, i);

  while (tv_col != NULL)
    {
      if (gtk_tree_view_column_get_visible (tv_col)) 
        n_cols++;
      if (in_col == n_cols)
        break;
      tv_col = gtk_tree_view_get_column (tree_view, ++i);
    }

  if (in_col != n_cols)
    {
       g_warning ("Request for invalid column %d\n", in_col);
       return NULL;
    }
  return tv_col;
}

static gint
get_actual_column_number (GtkTreeView *tree_view,
                          gint        visible_column)
{
  GtkTreeViewColumn *tv_col;
  gint actual_column = 0;
  gint visible_columns = -1;
  /*
   * This function calculates the column number which corresponds to the
   * specified visible column number
   */
  tv_col = gtk_tree_view_get_column (tree_view, actual_column);

  while (tv_col != NULL)
    {
      if (gtk_tree_view_column_get_visible (tv_col)) 
        visible_columns++;
      if (visible_columns == visible_column)
        return actual_column;
      tv_col = gtk_tree_view_get_column (tree_view, ++actual_column);
    }
  g_warning ("get_actual_column_number failed for %d\n", visible_column);
  return -1;
}

static gint
get_visible_column_number (GtkTreeView *tree_view,
                           gint        actual_column)
{
  GtkTreeViewColumn *tv_col;
  gint column = 0;
  gint visible_columns = -1;
  /*
   * This function calculates the visible column number which corresponds to the
   * specified actual column number
   */
  tv_col = gtk_tree_view_get_column (tree_view, column);

  while (tv_col != NULL)
    {
      if (gtk_tree_view_column_get_visible (tv_col)) 
        {
          visible_columns++;
          if (actual_column == column)
            return visible_columns;
        }
      else
        if (actual_column == column)
          return -1;
      tv_col = gtk_tree_view_get_column (tree_view, ++column);
    }
  g_warning ("get_visible_column_number failed for %d\n", actual_column);
  return -1;
}

/**
 * Helper recursive function that returns GtkTreeIter pointer to nth row.
 **/
static GtkTreeIter* 
return_iter_nth_row(GtkTreeView  *tree_view,
                    GtkTreeModel *tree_model, 
                    GtkTreeIter  *iter, 
                    gint         increment,
                    gint         row)
{
  GtkTreePath *current_path = gtk_tree_model_get_path (tree_model, iter);
  GtkTreeIter new_iter;
  gboolean row_expanded;

  if (increment == row)
    return iter;

  row_expanded = gtk_tree_view_row_expanded (tree_view, current_path);
  gtk_tree_path_free (current_path);

  new_iter = *iter;
  if ((row_expanded && gtk_tree_model_iter_children (tree_model, iter, &new_iter)) ||
      (gtk_tree_model_iter_next (tree_model, iter)) ||
      (gtk_tree_model_iter_parent (tree_model, iter, &new_iter) &&
          (gtk_tree_model_iter_next (tree_model, iter))))
    return return_iter_nth_row (tree_view, tree_model, iter,
      ++increment, row);

  return NULL;	 	
}

/**
 * Recursively called until the row specified by orig is found.
 *
 * *count will be set to the visible row number of the child
 * relative to the row that was initially passed in as tree_path.
 *
 * *count will be -1 if orig is not found as a child (a row that is
 * not visible will not be found, e.g. if the row is inside a
 * collapsed row).  If NULL is passed in as orig, *count will
 * be a count of the visible children.
 *
 * NOTE: the value for depth must be 0 when this recursive function
 * is initially called, or it may not function as expected.
 **/
static void 
iterate_thru_children(GtkTreeView  *tree_view,
                      GtkTreeModel *tree_model,
                      GtkTreePath  *tree_path,
                      GtkTreePath  *orig,
                      gint         *count,
                      gint         depth)
{
  GtkTreeIter iter;

  gtk_tree_model_get_iter (tree_model, &iter, tree_path);

  if (orig != NULL && !gtk_tree_path_compare (tree_path, orig)) 
    /* Found it! */
    return;

  if (orig != NULL && gtk_tree_path_compare (tree_path, orig) > 0)
    {
      /* Past it, so return -1 */
      *count = -1;
      return;
    }
  else if (gtk_tree_view_row_expanded (tree_view, tree_path) && 
    gtk_tree_model_iter_has_child (tree_model, &iter)) 
    {
      (*count)++;
      gtk_tree_path_append_index (tree_path, 0);
      iterate_thru_children (tree_view, tree_model, tree_path,
                             orig, count, (depth + 1));
      return;
    }
  else if (gtk_tree_model_iter_next (tree_model, &iter)) 
    {
      (*count)++;
      tree_path = gtk_tree_model_get_path (tree_model, &iter);
      iterate_thru_children (tree_view, tree_model, tree_path,
                             orig, count, depth); 
      gtk_tree_path_free (tree_path);
      return;
  }
  else if (gtk_tree_path_up (tree_path))
    {
      GtkTreeIter temp_iter;
      gboolean exit_loop = FALSE;
      gint new_depth = depth - 1;

      (*count)++;

     /*
      * Make sure that we back up until we find a row
      * where gtk_tree_path_next does not return NULL.
      */
      while (!exit_loop)
        {
          if (gtk_tree_path_get_depth (tree_path) == 0)
              /* depth is now zero so */
            return;
          gtk_tree_path_next (tree_path);	

          /* Verify that the next row is a valid row! */
          exit_loop = gtk_tree_model_get_iter (tree_model, &temp_iter, tree_path);

          if (!exit_loop)
            {
              /* Keep going up until we find a row that has a valid next */
              if (gtk_tree_path_get_depth(tree_path) > 1)
                {
                  new_depth--;
                  gtk_tree_path_up (tree_path);
                }
              else
                {
                 /*
                  * If depth is 1 and gtk_tree_model_get_iter returns FALSE,
                  * then we are at the last row, so just return.
                  */ 
                  if (orig != NULL)
                    *count = -1;

                  return;
                }
            }
        }

     /*
      * This guarantees that we will stop when we hit the end of the
      * children.
      */
      if (new_depth < 0)
        return;

      iterate_thru_children (tree_view, tree_model, tree_path,
                            orig, count, new_depth);
      return;
    }

 /*
  * If it gets here, then the path wasn't found.  Situations
  * that would cause this would be if the path passed in is
  * invalid or contained within the last row, but not visible
  * because the last row is not expanded.  If NULL was passed
  * in then a row count is desired, so only set count to -1
  * if orig is not NULL.
  */
  if (orig != NULL)
    *count = -1;

  return;
}

static void
clean_cell_info (GailTreeView *gailview,
                 GList        *list) 
{
  GailTreeViewCellInfo *cell_info = list->data;

  gail_cell_add_state (cell_info->cell, ATK_STATE_DEFUNCT, TRUE);
  if (cell_info->cell_row_ref != NULL)
    gtk_tree_row_reference_free (cell_info->cell_row_ref);
  g_object_weak_unref (G_OBJECT (cell_info->cell), (GWeakNotify) cell_destroyed, cell_info->cell);
  gailview->cell_data = g_list_remove_link (gailview->cell_data, list);
  g_free (cell_info); 
}

static void 
clean_rows (GailTreeView *gailview)
{
  GArray *array;

  /* Clean GailTreeViewRowInfo data */

  array = gailview->row_data;
  if (array != NULL)
    {
      GailTreeViewRowInfo *row_info;
      GtkTreePath *row_path;
      gint i;

     /*
      * Loop backwards so that calls to free_row_info
      * do not affect the index numbers 
      */
      for (i = (array->len - 1); i >= 0; i  --)
        {
          row_info = g_array_index (array, GailTreeViewRowInfo*, i);
          row_path = gtk_tree_row_reference_get_path (row_info->row_ref);

          /* Remove any rows that have become invalid */
          if (row_path == NULL)
            free_row_info (array, i, TRUE);
          else
            gtk_tree_path_free (row_path);
        }
    }

  /* Clean GailTreeViewCellInfo data */

  if (gailview->cell_data != NULL)
    {
      GailTreeViewCellInfo *cell_info;
      GtkTreePath *row_path;
      GList *cur_list;
      GList *temp_list;

      temp_list = gailview->cell_data;

      /* Must loop through them all */
      while (temp_list != NULL)
        {
          cur_list = temp_list;
          cell_info = temp_list->data;
          temp_list = temp_list->next;
          row_path = gtk_tree_row_reference_get_path (cell_info->cell_row_ref);

         /*
          * If the cell has become invalid because the row has been removed, 
          * then set the cell's state to ATK_STATE_DEFUNCT and remove the cell
          * from gailview->cell_data.  If row_path is NULL then the row has
          * been removed.
          */
          if (row_path == NULL)
            {
              clean_cell_info (gailview, cur_list);
            }
          else
            {
              gtk_tree_path_free (row_path);
            }
        }
    }
}

static void 
clean_cols (GailTreeView      *gailview,
            GtkTreeViewColumn *tv_col)
{
  /* Clean GailTreeViewCellInfo data */

  if (gailview->cell_data != NULL)
    {
      GailTreeViewCellInfo *cell_info;
      GList *cur_list, *temp_list;

      temp_list = gailview->cell_data;

      while (temp_list != NULL)
        {
          cur_list = temp_list;
          cell_info = temp_list->data;
          temp_list = temp_list->next;

         /*
          * If the cell has become invalid because the column tv_col
          * has been removed, then set the cell's state to ATK_STATE_DEFUNCT
          * and remove the cell from gailview->cell_data. 
          */
          if (cell_info->cell_col_ref == tv_col)
            {
              clean_cell_info (gailview, cur_list);
            }
        }
    }
}

/**
 * If tree_path is passed in as NULL, then all cells are acted on.
 * Otherwise, just act on those cells that are on a row greater than 
 * the specified tree_path. If inc_row is passed in as TRUE, then rows 
 * greater and equal to the specified tree_path are acted on.
 *
 * if set_stale is set the ATK_STATE_STALE is set on cells which are to be
 * acted on. 
 *
 * The function set_cell_visibility() is called on all cells to be
 * acted on to update the visibility of the cell.
 **/
static void 
traverse_cells (GailTreeView *tree_view,
                GtkTreePath  *tree_path,
                gboolean     set_stale,
                gboolean     inc_row)
{
  if (tree_view->cell_data != NULL)
    {
      GailTreeViewCellInfo *cell_info;
      GtkTreeView *gtk_tree_view;
      GList *temp_list;

      gtk_tree_view = GTK_TREE_VIEW (GTK_ACCESSIBLE (tree_view)->widget);
      temp_list = tree_view->cell_data;

      /* Must loop through them all */
      while (temp_list != NULL)
        {
          GtkTreePath *row_path;
          gboolean act_on_cell;

          cell_info = temp_list->data;
          temp_list = temp_list->next;

          row_path = gtk_tree_row_reference_get_path (cell_info->cell_row_ref);
          g_assert (row_path != NULL);
          if (tree_path == NULL)
            act_on_cell = TRUE;
          else 
            {
              gint comparison;

              comparison =  gtk_tree_path_compare (row_path, tree_path);
              if ((comparison > 0) ||
                  (comparison == 0 && inc_row))
                act_on_cell = TRUE;
              else
                act_on_cell = FALSE;
            }
          if (act_on_cell)
            {
              if (set_stale)
                gail_cell_add_state (cell_info->cell, ATK_STATE_STALE, TRUE);
              set_cell_visibility (gtk_tree_view,
                                   cell_info->cell,
                                   cell_info->cell_col_ref,
                                   row_path, TRUE);
            }
          gtk_tree_path_free (row_path);
        }
    }
  g_signal_emit_by_name (tree_view, "visible-data-changed");
}

static void
free_row_info (GArray   *array,
               gint     array_idx,
               gboolean shift)
{
  GailTreeViewRowInfo* obj;

  obj = g_array_index (array, GailTreeViewRowInfo*, array_idx);

  g_free (obj->description);
  if (obj->row_ref != NULL)
    gtk_tree_row_reference_free (obj->row_ref);
  if (obj->header)
    g_object_unref (obj->header);
  g_free (obj);

  if (shift)
    g_array_remove_index (array, array_idx);
}

/*
 * If the tree_path passed in has children, then
 * ATK_STATE_EXPANDABLE is set.  If the row is expanded
 * ATK_STATE_EXPANDED is turned on.  If the row is 
 * collapsed, then ATK_STATE_EXPANDED is removed.
 * 
 * If the tree_path passed in has no children, then
 * ATK_STATE_EXPANDABLE and ATK_STATE_EXPANDED are removed.
 *
 * If set_on_ancestor is TRUE, then this function will also
 * update all cells that are ancestors of the tree_path.
 */
static void
set_expand_state (GtkTreeView  *tree_view,
                  GtkTreeModel *tree_model,
                  GailTreeView *gailview,
                  GtkTreePath  *tree_path,
                  gboolean     set_on_ancestor)
{
  if (gailview->cell_data != NULL)
    {
      GtkTreeViewColumn *expander_tv;
      GailTreeViewCellInfo *cell_info;
      GList *temp_list;
      GtkTreePath *cell_path;
      GtkTreeIter iter;
      gboolean found;

      temp_list = gailview->cell_data;

      while (temp_list != NULL)
        {
          cell_info = temp_list->data;
          temp_list = temp_list->next;
          cell_path = gtk_tree_row_reference_get_path (cell_info->cell_row_ref);
          found = FALSE;

          if (cell_path != NULL)
            {
              GailCell *cell  = GAIL_CELL (cell_info->cell);

              expander_tv = gtk_tree_view_get_expander_column (tree_view);

             /*
              * Only set state for the cell that is in the column with the
              * expander toggle
              */
              if (expander_tv == cell_info->cell_col_ref)
                {
                  if (gtk_tree_path_compare (cell_path, tree_path) == 0)
                    found = TRUE;
                  else if (set_on_ancestor &&
                           gtk_tree_path_get_depth (cell_path) <
                           gtk_tree_path_get_depth (tree_path) && 
                           gtk_tree_path_is_ancestor (cell_path, tree_path) == 0)
                  /* Only set if set_on_ancestor was passed in as TRUE */
                    found = TRUE;
                }

               /*
                * Set ATK_STATE_EXPANDABLE and ATK_STATE_EXPANDED
                * for ancestors and found cells.
                */
              if (found)
                {
                 /*
                  * Must check against cell_path since cell_path
                  * can be equal to or an ancestor of tree_path.
                  */
                  gtk_tree_model_get_iter (tree_model, &iter, cell_path);

                  /* Set or unset ATK_STATE_EXPANDABLE as appropriate */
                  if (gtk_tree_model_iter_has_child (tree_model, &iter)) 
                    {
                      set_cell_expandable (cell);

                      if (gtk_tree_view_row_expanded (tree_view, cell_path))
                        gail_cell_add_state (cell, ATK_STATE_EXPANDED, TRUE);
                      else
                        gail_cell_remove_state (cell, 
                                                ATK_STATE_EXPANDED, TRUE);
                    }
                  else
                    {
                      gail_cell_remove_state (cell, 
                                              ATK_STATE_EXPANDED, TRUE);
                      if (gail_cell_remove_state (cell,
                                                  ATK_STATE_EXPANDABLE, TRUE))
	                gail_cell_remove_action_by_name (cell,
                                                         "expand or contract");
                    }

                  /*
                   * We assume that each cell in the cache once and
                   * a container cell is before its child cells so we are 
                   * finished if set_on_ancestor is not set to TRUE.
                   */
                  if (!set_on_ancestor)
                    break;
                }
            }
          gtk_tree_path_free (cell_path);
        }
    }
}


static void
add_cell_actions (GailCell *cell,
                  gboolean editable)
{
  if (GAIL_IS_BOOLEAN_CELL (cell))
    gail_cell_add_action (cell,
	"toggle",
	"toggles the cell", /* action description */
	NULL,
	toggle_cell_toggled);
  if (editable)
    gail_cell_add_action (cell,
	"edit",
	"creates a widget in which the contents of the cell can be edited", 
	NULL,
	edit_cell);
  gail_cell_add_action (cell,
	"activate",
	"activate the cell", 
	NULL,
	activate_cell);
}

static gboolean
toggle_cell_expanded (GailCell *cell)
{
  GailTreeViewCellInfo *cell_info;
  GtkTreeView *tree_view;
  GtkTreePath *path;
  AtkObject *parent;
  AtkStateSet *stateset;
  
  parent = atk_object_get_parent (ATK_OBJECT (cell));
  if (GAIL_IS_CONTAINER_CELL (parent))
    parent = atk_object_get_parent (parent);

  cell_info = find_cell_info (GAIL_TREE_VIEW (parent), cell, NULL);
  g_return_val_if_fail (cell_info, FALSE);
  g_return_val_if_fail (cell_info->cell_col_ref, FALSE);
  g_return_val_if_fail (cell_info->cell_row_ref, FALSE);

  tree_view = GTK_TREE_VIEW (GTK_ACCESSIBLE (parent)->widget);
  path = gtk_tree_row_reference_get_path (cell_info->cell_row_ref);
  g_return_val_if_fail (path, FALSE);

  stateset = atk_object_ref_state_set (ATK_OBJECT (cell));
  if (atk_state_set_contains_state (stateset, ATK_STATE_EXPANDED))
    gtk_tree_view_collapse_row (tree_view, path);
  else
      gtk_tree_view_expand_row (tree_view, path, TRUE);
  g_object_unref (stateset);
  gtk_tree_path_free (path);
  return TRUE;
}

static gboolean
toggle_cell_toggled (GailCell *cell)
{
  GailTreeViewCellInfo *cell_info;
  GtkTreeView *tree_view;
  GtkTreePath *path;
  gchar *pathstring;
  GList *renderers, *cur_renderer;
  AtkObject *parent;
  gboolean is_container_cell = FALSE;

  parent = atk_object_get_parent (ATK_OBJECT (cell));
  if (GAIL_IS_CONTAINER_CELL (parent))
    {
      is_container_cell = TRUE;
      parent = atk_object_get_parent (parent);
    }

  cell_info = find_cell_info (GAIL_TREE_VIEW (parent), cell, NULL);
  g_return_val_if_fail (cell_info, FALSE);
  g_return_val_if_fail (cell_info->cell_col_ref, FALSE);
  g_return_val_if_fail (cell_info->cell_row_ref, FALSE);

  tree_view = GTK_TREE_VIEW (GTK_ACCESSIBLE (parent)->widget);
  path = gtk_tree_row_reference_get_path (cell_info->cell_row_ref);
  g_return_val_if_fail (path, FALSE);
  pathstring = gtk_tree_path_to_string (path);

  renderers = gtk_tree_view_column_get_cell_renderers (cell_info->cell_col_ref);
  g_return_val_if_fail (renderers, FALSE);

  /* 
   * if the cell is in a container, it's index is used to find the 
   * renderer in the list
   */

  if (is_container_cell)
      cur_renderer = g_list_nth (renderers, cell->index);
  else
  /*
   * Otherwise, we assume that the cell is represented by the first 
   * renderer in the list 
   */

    cur_renderer = renderers;

  g_return_val_if_fail (cur_renderer != NULL, FALSE);

  g_signal_emit_by_name (cur_renderer, "toggle", pathstring);
  g_list_free (renderers);
  g_free (pathstring);
  gtk_tree_path_free (path);
  return TRUE;
}

static gboolean
edit_cell (GailCell *cell)
{
  GailTreeViewCellInfo *cell_info;
  GtkTreeView *tree_view;
  GtkTreePath *path;
  AtkObject *parent;
  gboolean is_container_cell = FALSE;

  editing = TRUE;
  parent = atk_object_get_parent (ATK_OBJECT (cell));
  if (GAIL_IS_CONTAINER_CELL (parent))
    {
      is_container_cell = TRUE;
      parent = atk_object_get_parent (parent);
    }

  cell_info = find_cell_info (GAIL_TREE_VIEW (parent), cell, NULL);
  g_return_val_if_fail (cell_info, FALSE);
  g_return_val_if_fail (cell_info->cell_col_ref, FALSE);
  g_return_val_if_fail (cell_info->cell_row_ref, FALSE);

  tree_view = GTK_TREE_VIEW (GTK_ACCESSIBLE (parent)->widget);
  path = gtk_tree_row_reference_get_path (cell_info->cell_row_ref);
  g_return_val_if_fail (path, FALSE);
  gtk_tree_view_set_cursor (tree_view, path, cell_info->cell_col_ref, TRUE);
  gtk_tree_path_free (path);
  return TRUE;
}

static gboolean
activate_cell (GailCell *cell)
{
  GailTreeViewCellInfo *cell_info;
  GtkTreeView *tree_view;
  GtkTreePath *path;
  AtkObject *parent;
  gboolean is_container_cell = FALSE;

  editing = TRUE;
  parent = atk_object_get_parent (ATK_OBJECT (cell));
  if (GAIL_IS_CONTAINER_CELL (parent))
    {
      is_container_cell = TRUE;
      parent = atk_object_get_parent (parent);
    }

  cell_info = find_cell_info (GAIL_TREE_VIEW (parent), cell, NULL);
  g_return_val_if_fail (cell_info, FALSE);
  g_return_val_if_fail (cell_info->cell_col_ref, FALSE);
  g_return_val_if_fail (cell_info->cell_row_ref, FALSE);

  tree_view = GTK_TREE_VIEW (GTK_ACCESSIBLE (parent)->widget);
  path = gtk_tree_row_reference_get_path (cell_info->cell_row_ref);
  g_return_val_if_fail (path, FALSE);
  gtk_tree_view_row_activated (tree_view, path, cell_info->cell_col_ref);
  gtk_tree_path_free (path);
  return TRUE;
}

static void
cell_destroyed (gpointer data)
{
  GailCell *cell = GAIL_CELL(data);
  AtkObject *parent;

  g_return_if_fail (GAIL_IS_CELL (cell));
  parent = atk_object_get_parent (ATK_OBJECT (cell));

  if (GAIL_IS_CONTAINER_CELL (cell))
    {
      /*
       * We remove GailTreeViewCellInfo entries for the children
       */
      GList *list;

      list = GAIL_CONTAINER_CELL (cell)->children;
      while (list)
        {
          GailCell *child_cell;

          child_cell = GAIL_CELL (list->data);
          cell_info_remove (GAIL_TREE_VIEW (parent), child_cell);
          list = list->next;
        }
    }
  if (GAIL_IS_CONTAINER_CELL (parent))
    /*
     * We do not remove the cell from the cache here.
     * We do it when the ContainerCell is removed.
     */
    return;

  g_return_if_fail (GAIL_IS_TREE_VIEW (parent));
  cell_info_remove (GAIL_TREE_VIEW (parent), cell);
}

static void
cell_info_remove (GailTreeView *tree_view, 
                  GailCell     *cell)
{
  GailTreeViewCellInfo *info;
  GList *temp_list;

  info = find_cell_info (tree_view, cell, &temp_list);
  if (info)
    {
      tree_view->cell_data = g_list_remove_link (tree_view->cell_data, 
                                                 temp_list);
      if (info->cell_row_ref)
        gtk_tree_row_reference_free (info->cell_row_ref);
      g_free (info);
      return;
    }
  g_warning ("No cell removed in cell_info_remove\n");
}

static void
cell_info_get_index (GtkTreeView            *tree_view, 
                     GailTreeViewCellInfo   *info,
                     gint                   *index)
{
  GtkTreePath *path;
  gint column_number;

  path = gtk_tree_row_reference_get_path (info->cell_row_ref);
  g_return_if_fail (path);

  column_number = get_column_number (tree_view, info->cell_col_ref, FALSE);
  *index = get_index (tree_view, path, column_number);
  gtk_tree_path_free (path);
}

static void
cell_info_new (GailTreeView      *gailview, 
               GtkTreeModel      *tree_model, 
               GtkTreePath       *path,
               GtkTreeViewColumn *tv_col,
               GailCell          *cell )
{
  GailTreeViewCellInfo *cell_info;

  cell_info = g_new (GailTreeViewCellInfo, 1);
  cell_info->cell_row_ref = gtk_tree_row_reference_new (tree_model, path);

  cell_info->cell_col_ref = tv_col;
  cell_info->cell = cell;
  gailview->cell_data = g_list_append (gailview->cell_data, cell_info);
      
  /* Setup weak reference notification */

  g_object_weak_ref (G_OBJECT (cell),
                     (GWeakNotify) cell_destroyed,
                     cell);
}

static GailCell*
find_cell (GailTreeView *gailview, 
           gint         index)
{
  GailTreeViewCellInfo *info;
  GtkTreeView *tree_view;
  GList *cell_list;
  GList *l;
  gint real_index;

  tree_view = GTK_TREE_VIEW (GTK_ACCESSIBLE (gailview)->widget);
  cell_list = gailview->cell_data;

  for (l = cell_list; l; l = l->next)
    {
      info = (GailTreeViewCellInfo *) (l->data);

      cell_info_get_index (tree_view, info, &real_index);
      if (index == real_index)
        return info->cell;
    }
  return NULL;
}

static void
refresh_cell_index (GailCell *cell)
{
  GailTreeViewCellInfo *info;
  AtkObject *parent;
  GtkTreeView *tree_view;
  gint index;

  parent = atk_object_get_parent (ATK_OBJECT (cell));
  g_return_if_fail (GAIL_IS_TREE_VIEW (parent));

  tree_view = GTK_TREE_VIEW (GTK_ACCESSIBLE (parent)->widget);

  /* Find this cell in the GailTreeView's cache */

  info = find_cell_info (GAIL_TREE_VIEW (parent), cell, NULL);
  g_return_if_fail (info);
  
  cell_info_get_index (tree_view, info, &index); 
  cell->index = index;
}

static void
get_selected_rows (GtkTreeModel *model,
                   GtkTreePath  *path,
                   GtkTreeIter  *iter,
                   gpointer     data)
{
  GPtrArray *array = (GPtrArray *)data;

  g_ptr_array_add (array, gtk_tree_path_copy (path));
}

static void
connect_model_signals (GtkTreeView  *view,
                       GtkTreeModel *model)
{
  GObject *obj;

  obj = G_OBJECT (model);
  g_signal_connect_data (obj, "row-changed",
                         (GCallback) model_row_changed, view, NULL, 0);
  g_signal_connect_data (obj, "row-inserted",
                         (GCallback) model_row_inserted, view, NULL, 
                         G_CONNECT_AFTER);
  g_signal_connect_data (obj, "row-deleted",
                         (GCallback) model_row_deleted, view, NULL, 
                         G_CONNECT_AFTER);
  g_signal_connect_data (obj, "rows-reordered",
                         (GCallback) model_rows_reordered, view, NULL, 
                         G_CONNECT_AFTER);
  g_object_ref (obj);
}

static void
disconnect_model_signals (GtkTreeModel *model) 
{
  GObject *obj;

  obj = G_OBJECT (model);
  g_signal_handlers_disconnect_by_func (obj, (gpointer) model_row_changed, NULL);
  g_signal_handlers_disconnect_by_func (obj, (gpointer) model_row_inserted, NULL);
  g_signal_handlers_disconnect_by_func (obj, (gpointer) model_row_deleted, NULL);
  g_signal_handlers_disconnect_by_func (obj, (gpointer) model_rows_reordered, NULL);
  g_object_unref (obj);
}

static void
clear_cached_data (GailTreeView  *view)
{
  GailTreeViewCellInfo *cell_info;
  GList *temp_list;

  if (view->row_data)
    {
      GArray *array = view->row_data;
      gint i;

     /*
      * Since the third argument to free_row_info is FALSE, we don't remove 
      * the element.  Therefore it is safe to loop forward.
      */
      for (i = 0; i < array->len; i++)
        free_row_info (array, i, FALSE);

      g_array_free (array, TRUE);

      view->row_data = NULL;
    }

  if (view->cell_data)
    {
      /* Must loop through them all */
      for (temp_list = view->cell_data; temp_list; temp_list = temp_list->next)
        {
          cell_info = (GailTreeViewCellInfo *) temp_list->data;
          if (cell_info->cell_row_ref != NULL)
            gtk_tree_row_reference_free (cell_info->cell_row_ref);
          g_free (temp_list->data);
        }
      g_list_free (view->cell_data);

      view->cell_data = NULL;
    }
}

/*
 * Returns the column number of the specified GtkTreeViewColumn
 *
 * If visible is set, the value returned will be the visible column number, 
 * i.e. suitable for use in AtkTable function. If visible is not set, the
 * value returned is the actual column number, which is suitable for use in 
 * getting an index value.
 */
static gint
get_column_number (GtkTreeView       *tree_view,
                   GtkTreeViewColumn *column,
                   gboolean          visible)
{
  GList *temp_list, *column_list;
  GtkTreeViewColumn *tv_column;
  gint ret_val;

  column_list = gtk_tree_view_get_columns (tree_view);
  ret_val = 0;
  for (temp_list = column_list; temp_list; temp_list = temp_list->next)
    {
      tv_column = GTK_TREE_VIEW_COLUMN (temp_list->data);
      if (tv_column == column)
        break;
      if (!visible || gtk_tree_view_column_get_visible (tv_column))
        ret_val++;
    }
  if (temp_list == NULL)
    {
      ret_val = -1;
    }
  g_list_free (column_list);
  return ret_val;
} 

static gint
get_index (GtkTreeView       *tree_view,
           GtkTreePath       *path,
           gint              actual_column)
{
  gint depth;
  gint index = 1;
  gint *indices;

  depth = gtk_tree_path_get_depth (path);
  indices = gtk_tree_path_get_indices (path);

  if (depth > 1)
    {
      GtkTreePath *copy_path;
      GtkTreeModel *model;

      model = gtk_tree_view_get_model (tree_view);
      copy_path = gtk_tree_path_copy (path);
      gtk_tree_path_up (copy_path);
      count_rows (model, NULL, copy_path, &index, 0, depth);
      gtk_tree_path_free (copy_path);
    }

  index += indices[depth-1];
  index *= get_n_actual_columns (tree_view);
  index +=  actual_column;
  return index;
}

/*
 * The function count_rows counts the number of rows starting at iter and ending
 * at end_path. The value of level is the depth of iter and the value of depth
 * is the depth of end_path. Rows at depth before end_path are counted.
 * This functions counts rows which are not visible because an ancestor is 
 * collapsed.
 */
static void 
count_rows (GtkTreeModel *model,
            GtkTreeIter *iter,
            GtkTreePath *end_path,
            gint        *count,
            gint        level,
            gint        depth)
{
  GtkTreeIter child_iter;
  
  if (!model)
    return;

  level++;

  *count += gtk_tree_model_iter_n_children (model, iter);

#if 0
  g_print ("count_rows : %d level: %d depth: %d\n", *count, level, depth);
  if (iter != NULL)
    g_print ("path: %s\n",
            gtk_tree_path_to_string (gtk_tree_model_get_path (model, iter)));
#endif

  if (level >= depth)
    return;

  if (gtk_tree_model_iter_children (model, &child_iter, iter))
    {
      gboolean ret_val = TRUE;

      while (ret_val)
        {
          if (level == depth - 1)
            {
              GtkTreePath *iter_path; 
              gboolean finished = FALSE;

              iter_path = gtk_tree_model_get_path (model, &child_iter);
              if (end_path && gtk_tree_path_compare (iter_path, end_path) >= 0)
                finished = TRUE;
              gtk_tree_path_free (iter_path);
              if (finished)
                break;
            }
          if (gtk_tree_model_iter_has_child (model, &child_iter))
            count_rows (model, &child_iter, end_path, count, level, depth);
          ret_val = gtk_tree_model_iter_next (model, &child_iter);
        }
    }
}

/*
 * Find the next node, which has children, at the specified depth below
 * the specified iter. The level is the depth of the current iter.
 * The position of the node is returned in path and the return value of TRUE 
 * means that a node was found.
 */

gboolean get_next_node_with_child_at_depth (GtkTreeModel *model,
                                            GtkTreeIter  *iter,
                                            GtkTreePath  **path,
                                            gint         level,
                                            gint         depth)
{
  GtkTreeIter child_iter;

  *path = NULL;

  if (gtk_tree_model_iter_children (model, &child_iter, iter))
    {
      level++;

      while (TRUE)
        {
          while (!gtk_tree_model_iter_has_child (model, &child_iter))
            {
              if (!gtk_tree_model_iter_next (model, &child_iter))
                return FALSE;
            }

          if (level == depth)
          /* We have found what we were looking for */
            {
              *path = gtk_tree_model_get_path (model, &child_iter);
              return TRUE;
            }

          if (get_next_node_with_child_at_depth (model, &child_iter, path,
                                                 level, depth))
            return TRUE;

          if (!gtk_tree_model_iter_next (model, &child_iter))
            return FALSE;
        }
    }
  return FALSE;
}

/*
 * Find the next node, which has children, at the same depth as 
 * the specified GtkTreePath.
 */
static gboolean 
get_next_node_with_child (GtkTreeModel *model,
                          GtkTreePath  *path,
                          GtkTreePath  **return_path)
{
  GtkTreeIter iter;
  gint depth;

  gtk_tree_model_get_iter (model, &iter, path);

  while (gtk_tree_model_iter_next (model, &iter))
    {
      if (gtk_tree_model_iter_has_child (model, &iter))
        {
          *return_path = gtk_tree_model_get_path (model, &iter);
          return TRUE;
        }
    }
  depth = gtk_tree_path_get_depth (path);
  while (gtk_tree_path_up (path))
    {
      if (gtk_tree_path_get_depth (path) == 0)
        break;

      gtk_tree_model_get_iter (model, &iter, path);
      while (gtk_tree_model_iter_next (model, &iter))
        if (get_next_node_with_child_at_depth (model, &iter, return_path,
                                         gtk_tree_path_get_depth (path), depth))
          return TRUE;
    }
  *return_path = NULL;
  return FALSE;
}

static gboolean 
get_tree_path_from_row_index (GtkTreeModel *model,
                              gint         row_index,
                              GtkTreePath  **tree_path)
{
  GtkTreeIter iter;
  gint count;
  gint depth;

  count = gtk_tree_model_iter_n_children (model, NULL);
  if (count > row_index)
    {
      if (gtk_tree_model_iter_nth_child (model, &iter, NULL, row_index))
        {
          *tree_path = gtk_tree_model_get_path (model, &iter);
          return TRUE;
        }
      else
        g_assert_not_reached ();
    }
  else
     row_index -= count;

  depth = 0;
  while (TRUE)
    {
      depth++;

      if (get_next_node_with_child_at_depth (model, NULL, tree_path, 0, depth))
        {
          GtkTreePath *next_path;

          while (TRUE)
            {
              gtk_tree_model_get_iter (model, &iter, *tree_path);
              count = gtk_tree_model_iter_n_children (model, &iter);
              if (count > row_index)
                {
                  gtk_tree_path_append_index (*tree_path, row_index);
                  return TRUE;
                }
              else
                row_index -= count;

              if (!get_next_node_with_child (model,  *tree_path, &next_path))
                break;
           
              gtk_tree_path_free (*tree_path);
              *tree_path = next_path;
            }
        }
      else
        {
          g_warning ("Index value is too large\n");
          gtk_tree_path_free (*tree_path);
           *tree_path = NULL;
          return FALSE;
        }
    }  
}

/*
 * This function returns the number of rows, including those which are collapsed
 */
static gint
get_row_count (GtkTreeModel *model)
{
  gint n_rows = 1;

  count_rows (model, NULL, NULL, &n_rows, 0, G_MAXINT);

  return n_rows;
}

static gboolean
get_path_column_from_index (GtkTreeView       *tree_view,
                            gint              index,
                            GtkTreePath       **path,
                            GtkTreeViewColumn **column)
{
  GtkTreeModel *tree_model;
  gint n_columns;

  tree_model = gtk_tree_view_get_model (tree_view);
  n_columns = get_n_actual_columns (tree_view);
  if (n_columns == 0)
    return FALSE;
  /* First row is the column headers */
  index -= n_columns;
  if (index < 0)
    return FALSE;

  if (path)
    {
      gint row_index;
      gboolean retval;

      row_index = index / n_columns;
      retval = get_tree_path_from_row_index (tree_model, row_index, path);
      g_return_val_if_fail (retval, FALSE);
      g_return_val_if_fail (*path, FALSE);
    }    

  if (column)
    {
      *column = gtk_tree_view_get_column (tree_view, index % n_columns);
      if (*column == NULL)
        {
          gtk_tree_path_free (*path);
          return FALSE;
        }
  }
  return TRUE;
}

static void
set_cell_expandable (GailCell *cell)
{
  if (gail_cell_add_state (cell, 
                           ATK_STATE_EXPANDABLE,
                           FALSE))
    gail_cell_add_action (cell,
                          "expand or contract", /* action name */
                          "expands or contracts the row in the tree view "
                          "containing this cell", /* description */
                          NULL, /* Keybinding */
                          toggle_cell_expanded);
}

static GailTreeViewCellInfo*
find_cell_info (GailTreeView *view,
                GailCell     *cell,
                GList**      list)
{
  GList *temp_list;
  GailTreeViewCellInfo *cell_info;

  for (temp_list = view->cell_data; temp_list; temp_list = temp_list->next)
    {
      cell_info = (GailTreeViewCellInfo *) temp_list->data;
      if (cell_info->cell == cell)
        {
          if (list)
            *list = temp_list;
          return cell_info;
        }
    }
  return NULL;
}
