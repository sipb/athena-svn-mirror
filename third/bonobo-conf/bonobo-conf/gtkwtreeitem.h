#ifndef __GTK_WTREE_ITEM_H__
#define __GTK_WTREE_ITEM_H__


#include <gdk/gdk.h>
#include <gtk/gtkitem.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_TYPE_WTREE_ITEM              (gtk_wtree_item_get_type ())
#define GTK_WTREE_ITEM(obj)              (GTK_CHECK_CAST ((obj), GTK_TYPE_WTREE_ITEM, GtkWTreeItem))
#define GTK_WTREE_ITEM_CLASS(klass)      (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_WTREE_ITEM, GtkWTreeItemClass))
#define GTK_IS_WTREE_ITEM(obj)           (GTK_CHECK_TYPE ((obj), GTK_TYPE_WTREE_ITEM))
#define GTK_IS_WTREE_ITEM_CLASS(klass)   (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_WTREE_ITEM))

#define GTK_WTREE_ITEM_SUBWTREE(obj)      (GTK_WTREE_ITEM(obj)->subwtree)


typedef struct _GtkWTreeItem       GtkWTreeItem;
typedef struct _GtkWTreeItemClass  GtkWTreeItemClass;

struct _GtkWTreeItem
{
	GtkItem item;

	gchar     *label;

	GtkWidget *subwtree;
	GtkWidget *pixmaps_box;
	GtkWidget *plus_pix_widget, *minus_pix_widget;

	GList *pixmaps;	       

	guint expanded : 1;
};

struct _GtkWTreeItemClass
{
  GtkItemClass parent_class;

  void (* expand)   (GtkWTreeItem *wtree_item);
  void (* collapse) (GtkWTreeItem *wtree_item);
};


GtkType    gtk_wtree_item_get_type        (void);
GtkWidget* gtk_wtree_item_new             (void);
GtkWidget* gtk_wtree_item_new_with_label  (const gchar *label);
GtkWidget* gtk_wtree_item_new_with_widget (const gchar *label, 
					   GtkWidget *widget);

void       gtk_wtree_item_set_subwtree    (GtkWTreeItem *wtree_item,
					   GtkWidget   *subwtree);
void       gtk_wtree_item_remove_subwtree (GtkWTreeItem *wtree_item);
void       gtk_wtree_item_select          (GtkWTreeItem *wtree_item);
void       gtk_wtree_item_deselect        (GtkWTreeItem *wtree_item);
void       gtk_wtree_item_expand          (GtkWTreeItem *wtree_item);
void       gtk_wtree_item_collapse        (GtkWTreeItem *wtree_item);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_WTREE_ITEM_H__ */
