#ifndef __GTK_WTREE_H__
#define __GTK_WTREE_H__

/* set this flag to enable tree debugging output */
/* #define WTREE_DEBUG */

#include <gdk/gdk.h>
#include <gtk/gtkcontainer.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_TYPE_WTREE                  (gtk_wtree_get_type ())
#define GTK_WTREE(obj)                  (GTK_CHECK_CAST ((obj), GTK_TYPE_WTREE, GtkWTree))
#define GTK_WTREE_CLASS(klass)          (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_WTREE, GtkWTreeClass))
#define GTK_IS_WTREE(obj)               (GTK_CHECK_TYPE ((obj), GTK_TYPE_WTREE))
#define GTK_IS_WTREE_CLASS(klass)       (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_WTREE))

#define GTK_IS_ROOT_WTREE(obj)   ((GtkObject*) GTK_WTREE(obj)->root_wtree == (GtkObject*)obj)
#define GTK_WTREE_ROOT_WTREE(obj) (GTK_WTREE(obj)->root_wtree ? GTK_WTREE(obj)->root_wtree : GTK_WTREE(obj))
#define GTK_WTREE_SELECTION(obj) (GTK_WTREE_ROOT_WTREE(obj)->selection)

typedef enum 
{
  GTK_WTREE_VIEW_LINE,  /* default view mode */
  GTK_WTREE_VIEW_ITEM
} GtkWTreeViewMode;

typedef struct _GtkWTree       GtkWTree;
typedef struct _GtkWTreeClass  GtkWTreeClass;

struct _GtkWTree
{
  GtkContainer container;
  
  GList *children;
  
  GtkWTree* root_wtree; /* owner of selection list */
  GtkWidget* wtree_owner;
  GList *selection;
  guint level;
  guint indent_value;
  guint col_width;
  guint current_indent;
  guint selection_mode : 2;
  guint view_mode : 1;
  guint view_line : 1;
};

struct _GtkWTreeClass
{
  GtkContainerClass parent_class;
  
  void (* selection_changed) (GtkWTree   *wtree);
  void (* select_child)      (GtkWTree   *wtree,
			      GtkWidget *child);
  void (* unselect_child)    (GtkWTree   *wtree,
			      GtkWidget *child);
};


GtkType    gtk_wtree_get_type           (void);
GtkWidget* gtk_wtree_new                (void);
void       gtk_wtree_append             (GtkWTree          *wtree,
				        GtkWidget        *wtree_item);
void       gtk_wtree_prepend            (GtkWTree          *wtree,
				        GtkWidget        *wtree_item);
void       gtk_wtree_insert             (GtkWTree          *wtree,
				        GtkWidget        *wtree_item,
				        gint              position);
void       gtk_wtree_remove_items       (GtkWTree          *wtree,
				        GList            *items);
void       gtk_wtree_clear_items        (GtkWTree          *wtree,
				        gint              start,
				        gint              end);
void       gtk_wtree_select_item        (GtkWTree          *wtree,
				        gint              item);
void       gtk_wtree_unselect_item      (GtkWTree          *wtree,
				        gint              item);
void       gtk_wtree_select_child       (GtkWTree          *wtree,
				        GtkWidget        *wtree_item);
void       gtk_wtree_unselect_child     (GtkWTree          *wtree,
				        GtkWidget        *wtree_item);
gint       gtk_wtree_child_position     (GtkWTree          *wtree,
				        GtkWidget        *child);
void       gtk_wtree_set_selection_mode (GtkWTree          *wtree,
				        GtkSelectionMode  mode);
void       gtk_wtree_set_view_mode      (GtkWTree          *wtree,
				        GtkWTreeViewMode   mode); 
void       gtk_wtree_set_view_lines     (GtkWTree          *wtree,
					guint            flag);

/* deprecated function, use gtk_container_remove instead.
 */
void       gtk_wtree_remove_item        (GtkWTree          *wtree,
					 GtkWidget        *child);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_WTREE_H__ */
