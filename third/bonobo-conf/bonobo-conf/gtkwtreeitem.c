#include <gtk/gtklabel.h>
#include <gtk/gtkeventbox.h>
#include <gtk/gtkpixmap.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>

#include "gtkwtree.h"
#include "gtkwtreeitem.h"

/* XPM */
static char *tree_plus[] = {
/* width height num_colors chars_per_pixel */
"     9     9        2            1",
/* colors */
". c #000000",
"# c #f8fcf8",
/* pixels */
".........",
".#######.",
".###.###.",
".###.###.",
".#.....#.",
".###.###.",
".###.###.",
".#######.",
"........."
};

/* XPM */
static char *tree_minus[] = {
/* width height num_colors chars_per_pixel */
"     9     9        2            1",
/* colors */
". c #000000",
"# c #f8fcf8",
/* pixels */
".........",
".#######.",
".#######.",
".#######.",
".#.....#.",
".#######.",
".#######.",
".#######.",
"........."
};

#define DEFAULT_DELTA 9

enum {
  COLLAPSE_WTREE,
  EXPAND_WTREE,
  LAST_SIGNAL
};

typedef struct _GtkWTreePixmaps GtkWTreePixmaps;

struct _GtkWTreePixmaps {
  gint refcount;
  GdkColormap *colormap;
  
  GdkPixmap *pixmap_plus;
  GdkPixmap *pixmap_minus;
  GdkBitmap *mask_plus;
  GdkBitmap *mask_minus;
};

static GList *pixmaps = NULL;

static void gtk_wtree_item_class_init (GtkWTreeItemClass *klass);
static void gtk_wtree_item_init       (GtkWTreeItem      *wtree_item);
static void gtk_wtree_item_realize       (GtkWidget        *widget);
static void gtk_wtree_item_size_request  (GtkWidget        *widget,
					 GtkRequisition   *requisition);
static void gtk_wtree_item_size_allocate (GtkWidget        *widget,
					 GtkAllocation    *allocation);
static void gtk_wtree_item_draw          (GtkWidget        *widget,
					 GdkRectangle     *area);
static void gtk_wtree_item_draw_focus    (GtkWidget        *widget);
static void gtk_wtree_item_paint         (GtkWidget        *widget,
					 GdkRectangle     *area);
static gint gtk_wtree_item_button_press  (GtkWidget        *widget,
					 GdkEventButton   *event);
static gint gtk_wtree_item_expose        (GtkWidget        *widget,
					 GdkEventExpose   *event);
static gint gtk_wtree_item_focus_in      (GtkWidget        *widget,
					 GdkEventFocus    *event);
static gint gtk_wtree_item_focus_out     (GtkWidget        *widget,
					 GdkEventFocus    *event);
static void gtk_wtree_item_forall        (GtkContainer    *container,
					 gboolean         include_internals,
					 GtkCallback      callback,
					 gpointer         callback_data);

static void gtk_real_wtree_item_select   (GtkItem          *item);
static void gtk_real_wtree_item_deselect (GtkItem          *item);
static void gtk_real_wtree_item_toggle   (GtkItem          *item);
static void gtk_real_wtree_item_expand   (GtkWTreeItem      *item);
static void gtk_real_wtree_item_collapse (GtkWTreeItem      *item);
static void gtk_real_wtree_item_expand   (GtkWTreeItem      *item);
static void gtk_real_wtree_item_collapse (GtkWTreeItem      *item);
static void gtk_wtree_item_destroy        (GtkObject *object);
static void gtk_wtree_item_subwtree_button_click (GtkWidget *widget);
static void gtk_wtree_item_subwtree_button_changed_state (GtkWidget *widget);

static void gtk_wtree_item_map(GtkWidget*);
static void gtk_wtree_item_unmap(GtkWidget*);

static void gtk_wtree_item_add_pixmaps    (GtkWTreeItem       *wtree_item);
static void gtk_wtree_item_remove_pixmaps (GtkWTreeItem       *wtree_item);

static GtkItemClass *parent_class = NULL;
static guint wtree_item_signals[LAST_SIGNAL] = { 0 };

GtkType
gtk_wtree_item_get_type (void)
{
  static GtkType wtree_item_type = 0;

  if (!wtree_item_type)
    {
      static const GtkTypeInfo wtree_item_info =
      {
	"GtkWTreeItem",
	sizeof (GtkWTreeItem),
	sizeof (GtkWTreeItemClass),
	(GtkClassInitFunc) gtk_wtree_item_class_init,
	(GtkObjectInitFunc) gtk_wtree_item_init,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      wtree_item_type = gtk_type_unique (gtk_item_get_type (), &wtree_item_info);
    }

  return wtree_item_type;
}

static void
gtk_wtree_item_class_init (GtkWTreeItemClass *class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  GtkContainerClass *container_class;
  GtkItemClass *item_class;

  object_class = (GtkObjectClass*) class;
  widget_class = (GtkWidgetClass*) class;
  item_class = (GtkItemClass*) class;
  container_class = (GtkContainerClass*) class;

  parent_class = gtk_type_class (gtk_item_get_type ());
  
  wtree_item_signals[EXPAND_WTREE] =
    gtk_signal_new ("expand",
		    GTK_RUN_FIRST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (GtkWTreeItemClass, expand),
		    gtk_marshal_NONE__NONE,
		    GTK_TYPE_NONE, 0);
  wtree_item_signals[COLLAPSE_WTREE] =
    gtk_signal_new ("collapse",
		    GTK_RUN_FIRST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (GtkWTreeItemClass, collapse),
		    gtk_marshal_NONE__NONE,
		    GTK_TYPE_NONE, 0);

  gtk_object_class_add_signals (object_class, wtree_item_signals, LAST_SIGNAL);

  object_class->destroy = gtk_wtree_item_destroy;

  widget_class->realize = gtk_wtree_item_realize;
  widget_class->size_request = gtk_wtree_item_size_request;
  widget_class->size_allocate = gtk_wtree_item_size_allocate;
  widget_class->draw = gtk_wtree_item_draw;
  widget_class->draw_focus = gtk_wtree_item_draw_focus;
  widget_class->button_press_event = gtk_wtree_item_button_press;
  widget_class->expose_event = gtk_wtree_item_expose;
  widget_class->focus_in_event = gtk_wtree_item_focus_in;
  widget_class->focus_out_event = gtk_wtree_item_focus_out;
  widget_class->map = gtk_wtree_item_map;
  widget_class->unmap = gtk_wtree_item_unmap;

  container_class->forall = gtk_wtree_item_forall;

  item_class->select = gtk_real_wtree_item_select;
  item_class->deselect = gtk_real_wtree_item_deselect;
  item_class->toggle = gtk_real_wtree_item_toggle;

  class->expand = gtk_real_wtree_item_expand;
  class->collapse = gtk_real_wtree_item_collapse;
}

/* callback for event box mouse event */
static void 
gtk_wtree_item_subwtree_button_click (GtkWidget *widget)
{
  GtkWTreeItem* item;
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_EVENT_BOX (widget));
  
  item = (GtkWTreeItem*) gtk_object_get_user_data (GTK_OBJECT (widget));
  if (!GTK_WIDGET_IS_SENSITIVE (item))
    return;
  
  if (item->expanded)
    gtk_wtree_item_collapse (item);
  else
    gtk_wtree_item_expand (item);
}

/* callback for event box state changed */
static void
gtk_wtree_item_subwtree_button_changed_state (GtkWidget *widget)
{
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_EVENT_BOX (widget));
  
  if (GTK_WIDGET_VISIBLE (widget))
    {

      if (widget->state == GTK_STATE_NORMAL)
	gdk_window_set_background (widget->window, &widget->style->base[widget->state]);
      else
	gdk_window_set_background (widget->window, &widget->style->bg[widget->state]);
	   
      if (GTK_WIDGET_DRAWABLE (widget))
	gdk_window_clear_area (widget->window, 0, 0, 
			       widget->allocation.width, widget->allocation.height);
	    
    }
}

static void
gtk_wtree_item_init (GtkWTreeItem *wtree_item)
{
  GtkWidget *eventbox, *pixmapwid;
  
  g_return_if_fail (wtree_item != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (wtree_item));

  wtree_item->expanded = FALSE;
  wtree_item->subwtree = NULL;
  GTK_WIDGET_SET_FLAGS (wtree_item, GTK_CAN_FOCUS);
  
  /* create an event box containing one pixmaps */
  eventbox = gtk_event_box_new();
  gtk_widget_set_events (eventbox, GDK_BUTTON_PRESS_MASK);
  gtk_signal_connect(GTK_OBJECT(eventbox), "state_changed",
		     (GtkSignalFunc)gtk_wtree_item_subwtree_button_changed_state, 
		     (gpointer)NULL);
  gtk_signal_connect(GTK_OBJECT(eventbox), "realize",
		     (GtkSignalFunc)gtk_wtree_item_subwtree_button_changed_state, 
		     (gpointer)NULL);
  gtk_signal_connect(GTK_OBJECT(eventbox), "button_press_event",
		     (GtkSignalFunc)gtk_wtree_item_subwtree_button_click,
		     (gpointer)NULL);
  gtk_object_set_user_data(GTK_OBJECT(eventbox), wtree_item);
  wtree_item->pixmaps_box = eventbox;

  /* create pixmap for button '+' */
  pixmapwid = gtk_type_new (gtk_pixmap_get_type ());
  if (!wtree_item->expanded) 
    gtk_container_add (GTK_CONTAINER (eventbox), pixmapwid);
  gtk_widget_show (pixmapwid);
  wtree_item->plus_pix_widget = pixmapwid;
  gtk_widget_ref (wtree_item->plus_pix_widget);
  gtk_object_sink (GTK_OBJECT (wtree_item->plus_pix_widget));
  
  /* create pixmap for button '-' */
  pixmapwid = gtk_type_new (gtk_pixmap_get_type ());
  if (wtree_item->expanded) 
    gtk_container_add (GTK_CONTAINER (eventbox), pixmapwid);
  gtk_widget_show (pixmapwid);
  wtree_item->minus_pix_widget = pixmapwid;
  gtk_widget_ref (wtree_item->minus_pix_widget);
  gtk_object_sink (GTK_OBJECT (wtree_item->minus_pix_widget));
  
  gtk_widget_set_parent (eventbox, GTK_WIDGET (wtree_item));
}


GtkWidget*
gtk_wtree_item_new (void)
{
  GtkWidget *wtree_item;

  wtree_item = GTK_WIDGET (gtk_type_new (gtk_wtree_item_get_type ()));

  return wtree_item;
}

GtkWidget*
gtk_wtree_item_new_with_label (const gchar *label)
{
  GtkWidget *wtree_item;
  GtkWidget *label_widget;

  wtree_item = gtk_wtree_item_new ();
  GTK_WTREE_ITEM (wtree_item)->label = g_strdup (label);

  label_widget = gtk_label_new (label);
  gtk_misc_set_alignment (GTK_MISC (label_widget), 0.0, 0.5);

  gtk_container_add (GTK_CONTAINER (wtree_item), label_widget);
  gtk_widget_show (label_widget);


  return wtree_item;
}

GtkWidget*
gtk_wtree_item_new_with_widget (const gchar *label, GtkWidget *widget)
{
  GtkWidget *wtree_item;

  g_return_val_if_fail (label != NULL, NULL);
  g_return_val_if_fail (widget != NULL, NULL);

  wtree_item = gtk_wtree_item_new ();
  GTK_WTREE_ITEM (wtree_item)->label = g_strdup (label);

  gtk_container_add (GTK_CONTAINER (wtree_item), widget);
  gtk_widget_show (widget);

  return wtree_item;
}

void
gtk_wtree_item_set_subwtree (GtkWTreeItem *wtree_item,
			   GtkWidget   *subwtree)
{
  g_return_if_fail (wtree_item != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (wtree_item));
  g_return_if_fail (subwtree != NULL);
  g_return_if_fail (GTK_IS_WTREE (subwtree));

  if (wtree_item->subwtree)
    {
      g_warning("there is already a subwtree for this wtree item\n");
      return;
    }

  wtree_item->subwtree = subwtree; 
  GTK_WTREE (subwtree)->wtree_owner = GTK_WIDGET (wtree_item);

  /* show subwtree button */
  if (wtree_item->pixmaps_box)
    gtk_widget_show (wtree_item->pixmaps_box);

  if (wtree_item->expanded)
    gtk_widget_show (subwtree);
  else
    gtk_widget_hide (subwtree);

  gtk_widget_set_parent (subwtree, GTK_WIDGET (wtree_item)->parent);

  if (GTK_WIDGET_REALIZED (subwtree->parent))
    gtk_widget_realize (subwtree);

  if (GTK_WIDGET_VISIBLE (subwtree->parent) && GTK_WIDGET_VISIBLE (subwtree))
    {
      if (GTK_WIDGET_MAPPED (subwtree->parent))
	gtk_widget_map (subwtree);

      gtk_widget_queue_resize (subwtree);
    }
}

void
gtk_wtree_item_select (GtkWTreeItem *wtree_item)
{
  g_return_if_fail (wtree_item != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (wtree_item));

  gtk_item_select (GTK_ITEM (wtree_item));
}

void
gtk_wtree_item_deselect (GtkWTreeItem *wtree_item)
{
  g_return_if_fail (wtree_item != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (wtree_item));

  gtk_item_deselect (GTK_ITEM (wtree_item));
}

void
gtk_wtree_item_expand (GtkWTreeItem *wtree_item)
{
  g_return_if_fail (wtree_item != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (wtree_item));

  gtk_signal_emit (GTK_OBJECT (wtree_item), wtree_item_signals[EXPAND_WTREE], NULL);
}

void
gtk_wtree_item_collapse (GtkWTreeItem *wtree_item)
{
  g_return_if_fail (wtree_item != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (wtree_item));

  gtk_signal_emit (GTK_OBJECT (wtree_item), wtree_item_signals[COLLAPSE_WTREE], NULL);
}

static void
gtk_wtree_item_add_pixmaps (GtkWTreeItem *wtree_item)
{
  GList *tmp_list;
  GdkColormap *colormap;
  GtkWTreePixmaps *pixmap_node = NULL;

  g_return_if_fail (wtree_item != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (wtree_item));

  if (wtree_item->pixmaps)
    return;

  colormap = gtk_widget_get_colormap (GTK_WIDGET (wtree_item));

  tmp_list = pixmaps;
  while (tmp_list)
    {
      pixmap_node = (GtkWTreePixmaps *)tmp_list->data;

      if (pixmap_node->colormap == colormap)
	break;
      
      tmp_list = tmp_list->next;
    }

  if (tmp_list)
    {
      pixmap_node->refcount++;
      wtree_item->pixmaps = tmp_list;
    }
  else
    {
      pixmap_node = g_new (GtkWTreePixmaps, 1);

      pixmap_node->colormap = colormap;
      gdk_colormap_ref (colormap);

      pixmap_node->refcount = 1;

      /* create pixmaps for plus icon */
      pixmap_node->pixmap_plus = 
	gdk_pixmap_create_from_xpm_d (GTK_WIDGET (wtree_item)->window,
				      &pixmap_node->mask_plus,
				      NULL,
				      tree_plus);
      
      /* create pixmaps for minus icon */
      pixmap_node->pixmap_minus = 
	gdk_pixmap_create_from_xpm_d (GTK_WIDGET (wtree_item)->window,
				      &pixmap_node->mask_minus,
				      NULL,
				      tree_minus);

      wtree_item->pixmaps = pixmaps = g_list_prepend (pixmaps, pixmap_node);
    }
  
  gtk_pixmap_set (GTK_PIXMAP (wtree_item->plus_pix_widget), 
		  pixmap_node->pixmap_plus, pixmap_node->mask_plus);
  gtk_pixmap_set (GTK_PIXMAP (wtree_item->minus_pix_widget), 
		  pixmap_node->pixmap_minus, pixmap_node->mask_minus);
}

static void
gtk_wtree_item_remove_pixmaps (GtkWTreeItem *wtree_item)
{
  g_return_if_fail (wtree_item != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (wtree_item));

  if (wtree_item->pixmaps)
    {
      GtkWTreePixmaps *pixmap_node = (GtkWTreePixmaps *)wtree_item->pixmaps->data;
      
      g_assert (pixmap_node->refcount > 0);
      
      if (--pixmap_node->refcount == 0)
	{
	  gdk_colormap_unref (pixmap_node->colormap);
	  gdk_pixmap_unref (pixmap_node->pixmap_plus);
	  gdk_bitmap_unref (pixmap_node->mask_plus);
	  gdk_pixmap_unref (pixmap_node->pixmap_minus);
	  gdk_bitmap_unref (pixmap_node->mask_minus);
	  
	  pixmaps = g_list_remove_link (pixmaps, wtree_item->pixmaps);
	  g_list_free_1 (wtree_item->pixmaps);
	  g_free (pixmap_node);
	}

      wtree_item->pixmaps = NULL;
    }
}

static void
gtk_wtree_item_realize (GtkWidget *widget)
{    
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (widget));

  if (GTK_WIDGET_CLASS (parent_class)->realize)
    (* GTK_WIDGET_CLASS (parent_class)->realize) (widget);
  
  gdk_window_set_background (widget->window, 
			     &widget->style->base[GTK_STATE_NORMAL]);

  gtk_wtree_item_add_pixmaps (GTK_WTREE_ITEM (widget));
}

static int
calc_indent (GtkWTree *wtree)
{
  GList *children;
  GtkWTreeItem *child;
  int width = 0, tw;

  children = wtree->children;
  while (children) {
      child = GTK_WTREE_ITEM (children->data);
      children = children->next;
      if (child->subwtree)
	      tw = calc_indent (GTK_WTREE (child->subwtree));
      else {
	      tw = gdk_text_width (GTK_WIDGET(child)->style->font,
				   child->label, strlen (child->label));
	      tw += GTK_WTREE(GTK_WIDGET(child)->parent)->current_indent*2;
	      tw += GTK_WTREE(GTK_WIDGET(child)->parent)->indent_value*2;
      }
      width = MAX (width, tw);
  }
  return width;
}

static void
gtk_wtree_item_size_request (GtkWidget      *widget,
			    GtkRequisition *requisition)
{
  GtkBin *bin;
  GtkWTreeItem *item;
  GtkWTree *wtree, *root;
  GtkRequisition child_requisition;
  int th;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (widget));
  g_return_if_fail (requisition != NULL);

  bin = GTK_BIN (widget);
  item = GTK_WTREE_ITEM(widget);
  wtree = GTK_WTREE(widget->parent);
  root = wtree->root_wtree;

  if (root)
	  wtree->col_width = calc_indent (root) + DEFAULT_DELTA;

  requisition->width = (GTK_CONTAINER (widget)->border_width +
			widget->style->klass->xthickness) * 2 +
	  wtree->col_width;
  requisition->height = GTK_CONTAINER (widget)->border_width * 2;

  th = GTK_CONTAINER (widget)->border_width * 2 + 8 +
	  widget->style->font->ascent + widget->style->font->descent;

  if (bin->child && GTK_WIDGET_VISIBLE (bin->child))
    {
      GtkRequisition pix_requisition;
      
      gtk_widget_size_request (bin->child, &child_requisition);

      requisition->width += child_requisition.width;

      gtk_widget_size_request (item->pixmaps_box, 
			       &pix_requisition);
      requisition->width += pix_requisition.width + DEFAULT_DELTA + 
	GTK_WTREE (widget->parent)->current_indent;

      requisition->height += MAX (MAX (child_requisition.height,
				       pix_requisition.height), th) + 4;
    }
}

static void
gtk_wtree_item_size_allocate (GtkWidget     *widget,
			     GtkAllocation *allocation)
{
  GtkBin *bin;
  GtkWTreeItem* item;
  GtkWTree* wtree;
  GtkAllocation child_allocation;
  gint border_width;
  int temp;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (widget));
  g_return_if_fail (allocation != NULL);

  widget->allocation = *allocation;
  if (GTK_WIDGET_REALIZED (widget))
    gdk_window_move_resize (widget->window,
			    allocation->x, allocation->y,
			    allocation->width, allocation->height);

  bin = GTK_BIN (widget);
  item = GTK_WTREE_ITEM(widget);
  wtree = GTK_WTREE(widget->parent);

  if (bin->child)
    {
      border_width = (GTK_CONTAINER (widget)->border_width +
		      widget->style->klass->xthickness);

      
      child_allocation.x = border_width + wtree->current_indent*2 + 2;
      
      child_allocation.y = GTK_CONTAINER (widget)->border_width - 2;

      child_allocation.width = item->pixmaps_box->requisition.width;
      child_allocation.height = item->pixmaps_box->requisition.height;
      
      temp = allocation->height - child_allocation.height;
      child_allocation.y += ( temp / 2 ) + ( temp % 2 );

      gtk_widget_size_allocate (item->pixmaps_box, &child_allocation);

      child_allocation.y = GTK_CONTAINER (widget)->border_width - 2;
      child_allocation.height = MAX (1, (gint)allocation->height - child_allocation.y * 2);
      
      if (wtree->root_wtree) {
	      child_allocation.x += item->pixmaps_box->requisition.width
		      + DEFAULT_DELTA + wtree->root_wtree->col_width;
      
	      child_allocation.x = wtree->root_wtree->col_width;

      }

      child_allocation.width = 
	MAX (1, (gint)allocation->width - ((gint)child_allocation.x + border_width));

      gtk_widget_size_allocate (bin->child, &child_allocation);
    }
}

static void
gtk_wtree_item_paint (GtkWidget    *widget,
		      GdkRectangle *area)
{
  GtkBin *bin;
  GdkRectangle item_area;
  GtkWTreeItem* wtree_item;
  GtkWTree* wtree;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (widget));
  g_return_if_fail (area != NULL);

  /* FIXME: We should honor wtree->view_mode, here - I think
   * the desired effect is that when the mode is VIEW_ITEM,
   * only the subitem is drawn as selected, not the entire
   * line. (Like the way that the wtree in Windows Explorer
   * works).
   */
  if (GTK_WIDGET_DRAWABLE (widget))
    {
      bin = GTK_BIN (widget);
      wtree_item = GTK_WTREE_ITEM(widget);
      wtree = GTK_WTREE(widget->parent);

      if (widget->state == GTK_STATE_NORMAL)
	{
	  gdk_window_set_back_pixmap (widget->window, NULL, TRUE);
	  gdk_window_clear_area (widget->window, area->x, area->y, 
				 area->width, area->height);
	}
      else 
	{
	  if (!GTK_WIDGET_IS_SENSITIVE (widget)) 
	    gtk_paint_flat_box(widget->style, widget->window,
			       widget->state, GTK_STATE_INSENSITIVE,
			       area, widget, "treeitem",
			       0, 0, -1, -1);
	  else
	    gtk_paint_flat_box(widget->style, widget->window,
			       widget->state, GTK_SHADOW_ETCHED_OUT,
			       area, widget, "treeitem",
			       0, 0, -1, -1);
	}


      if (wtree == wtree->root_wtree) {
      } else {
	      gdk_draw_rectangle (widget->window, 
			      widget->style->base_gc[GTK_STATE_NORMAL],
			      TRUE, wtree->indent_value*2, 0,
			      widget->allocation.width,
			      widget->allocation.height);
      }
      /* draw left size of wtree item */
      item_area.x = 0;
      item_area.y = 0;
      item_area.width = (wtree_item->pixmaps_box->allocation.width + DEFAULT_DELTA +
			 GTK_WTREE (widget->parent)->current_indent + 2);
      item_area.height = widget->allocation.height;

      if (wtree_item->pixmaps_box && 
	  GTK_WIDGET_VISIBLE(wtree_item->pixmaps_box))
	      gtk_widget_draw (wtree_item->pixmaps_box, &item_area);

      if (wtree_item->label) {
	      int th;

	      th = widget->style->font->ascent + widget->style->font->descent;

	      gdk_draw_text (widget->window, widget->style->font,
			     widget->style->text_gc[GTK_STATE_NORMAL],
			     wtree->current_indent*2 + DEFAULT_DELTA*2, 
			     th, 
			     wtree_item->label, 
			     strlen (wtree_item->label));
			     

      }


      if (GTK_WIDGET_HAS_FOCUS (widget))
	gtk_paint_focus (widget->style, widget->window,
			 NULL, widget, "wtreeitem",
			 0, 0,
			 widget->allocation.width - 1,
			 widget->allocation.height - 1);
      
    }

      if (GTK_BIN(widget)->child)
	      gtk_widget_draw (GTK_BIN(widget)->child, NULL);

}

static void
gtk_wtree_item_draw (GtkWidget    *widget,
		    GdkRectangle *area)
{
  GtkBin *bin;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (widget));
  g_return_if_fail (area != NULL);

  if (GTK_WIDGET_DRAWABLE (widget))
    {
      bin = GTK_BIN (widget);

      gtk_wtree_item_paint (widget, area);
     
    }
}

static void
gtk_wtree_item_draw_focus (GtkWidget *widget)
{
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (widget));

  gtk_widget_draw(widget, NULL);
}

static gint
gtk_wtree_item_button_press (GtkWidget      *widget,
			    GdkEventButton *event)
{

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_WTREE_ITEM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  if (event->type == GDK_BUTTON_PRESS
	&& GTK_WIDGET_IS_SENSITIVE(widget)
     	&& !GTK_WIDGET_HAS_FOCUS (widget))
      gtk_widget_grab_focus (widget);

  return FALSE;
}

static gint
gtk_wtree_item_expose (GtkWidget      *widget,
		      GdkEventExpose *event)
{
  GdkEventExpose child_event;
  GtkBin *bin;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_WTREE_ITEM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  if (GTK_WIDGET_DRAWABLE (widget))
    {
      bin = GTK_BIN (widget);
      
      gtk_wtree_item_paint (widget, &event->area);

      child_event = *event;
      if (bin->child && GTK_WIDGET_NO_WINDOW (bin->child) &&
	  gtk_widget_intersect (bin->child, &event->area, &child_event.area))
	gtk_widget_event (bin->child, (GdkEvent*) &child_event);
   }

  return FALSE;
}

static gint
gtk_wtree_item_focus_in (GtkWidget     *widget,
			GdkEventFocus *event)
{
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_WTREE_ITEM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  GTK_WIDGET_SET_FLAGS (widget, GTK_HAS_FOCUS);
  gtk_widget_draw_focus (widget);

  return FALSE;
}

static gint
gtk_wtree_item_focus_out (GtkWidget     *widget,
			 GdkEventFocus *event)
{
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_WTREE_ITEM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  GTK_WIDGET_UNSET_FLAGS (widget, GTK_HAS_FOCUS);
  gtk_widget_draw_focus (widget);


  return FALSE;
}

static void
gtk_real_wtree_item_select (GtkItem *item)
{    
  GtkWTreeItem *wtree_item;
  GtkWidget *widget;

  g_return_if_fail (item != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (item));

  wtree_item = GTK_WTREE_ITEM (item);
  widget = GTK_WIDGET (item);

/*  gtk_widget_set_state (GTK_WIDGET (item), GTK_STATE_SELECTED); */

  if (!widget->parent || GTK_WTREE (widget->parent)->view_mode == GTK_WTREE_VIEW_LINE)
    gtk_widget_set_state (GTK_WTREE_ITEM (item)->pixmaps_box, GTK_STATE_SELECTED);
}

static void
gtk_real_wtree_item_deselect (GtkItem *item)
{
  GtkWTreeItem *wtree_item;
  GtkWidget *widget;

  g_return_if_fail (item != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (item));

  wtree_item = GTK_WTREE_ITEM (item);
  widget = GTK_WIDGET (item);

  gtk_widget_set_state (widget, GTK_STATE_NORMAL);

  if (!widget->parent || GTK_WTREE (widget->parent)->view_mode == GTK_WTREE_VIEW_LINE)
    gtk_widget_set_state (wtree_item->pixmaps_box, GTK_STATE_NORMAL);
}

static void
gtk_real_wtree_item_toggle (GtkItem *item)
{
  g_return_if_fail (item != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (item));

  if(!GTK_WIDGET_IS_SENSITIVE(item))
    return;

  if (GTK_WIDGET (item)->parent && GTK_IS_WTREE (GTK_WIDGET (item)->parent))
    gtk_wtree_select_child (GTK_WTREE (GTK_WIDGET (item)->parent),
			   GTK_WIDGET (item));
  else
    {
      /* Should we really bother with this bit? A listitem not in a list?
       * -Johannes Keukelaar
       * yes, always be on the safe side!
       * -timj
       */
      if (GTK_WIDGET (item)->state == GTK_STATE_SELECTED)
	gtk_widget_set_state (GTK_WIDGET (item), GTK_STATE_NORMAL);
      else
	gtk_widget_set_state (GTK_WIDGET (item), GTK_STATE_SELECTED);
    }
}

static void
gtk_real_wtree_item_expand (GtkWTreeItem *wtree_item)
{
  GtkWTree* wtree;
  
  g_return_if_fail (wtree_item != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (wtree_item));
  
  if (wtree_item->subwtree && !wtree_item->expanded)
    {
      wtree = GTK_WTREE (GTK_WIDGET (wtree_item)->parent); 
      
      /* hide subwtree widget */
      gtk_widget_show (wtree_item->subwtree);
      
      /* hide button '+' and show button '-' */
      if (wtree_item->pixmaps_box)
	{
	  gtk_container_remove (GTK_CONTAINER (wtree_item->pixmaps_box), 
				wtree_item->plus_pix_widget);
	  gtk_container_add (GTK_CONTAINER (wtree_item->pixmaps_box), 
			     wtree_item->minus_pix_widget);
	}
      if (wtree->root_wtree)
	gtk_widget_queue_resize (GTK_WIDGET (wtree->root_wtree));
      wtree_item->expanded = TRUE;
    }
}

static void
gtk_real_wtree_item_collapse (GtkWTreeItem *wtree_item)
{
  GtkWTree* wtree;
  
  g_return_if_fail (wtree_item != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (wtree_item));
  
  if (wtree_item->subwtree && wtree_item->expanded) 
    {
      wtree = GTK_WTREE (GTK_WIDGET (wtree_item)->parent);
      
      /* hide subwtree widget */
      gtk_widget_hide (wtree_item->subwtree);
      
      /* hide button '-' and show button '+' */
      if (wtree_item->pixmaps_box)
	{
	  gtk_container_remove (GTK_CONTAINER (wtree_item->pixmaps_box), 
				wtree_item->minus_pix_widget);
	  gtk_container_add (GTK_CONTAINER (wtree_item->pixmaps_box), 
			     wtree_item->plus_pix_widget);
	}
      if (wtree->root_wtree)
	gtk_widget_queue_resize (GTK_WIDGET (wtree->root_wtree));
      wtree_item->expanded = FALSE;
    }
}

static void
gtk_wtree_item_destroy (GtkObject *object)
{
  GtkWTreeItem* item;
  GtkWidget* child;

  g_return_if_fail (object != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (object));

#ifdef WTREE_DEBUG
  g_message("+ gtk_wtree_item_destroy [object %#x]\n", (int)object);
#endif /* WTREE_DEBUG */

  item = GTK_WTREE_ITEM(object);

  /* free sub wtree if it exist */
  child = item->subwtree;
  if (child)
    {
      gtk_widget_ref (child);
      gtk_widget_unparent (child);
      gtk_widget_destroy (child);
      gtk_widget_unref (child);
      item->subwtree = NULL;
    }
  
  /* free pixmaps box */
  child = item->pixmaps_box;
  if (child)
    {
      gtk_widget_ref (child);
      gtk_widget_unparent (child);
      gtk_widget_destroy (child);
      gtk_widget_unref (child);
      item->pixmaps_box = NULL;
    }
  
  
  /* destroy plus pixmap */
  if (item->plus_pix_widget)
    {
      gtk_widget_destroy (item->plus_pix_widget);
      gtk_widget_unref (item->plus_pix_widget);
      item->plus_pix_widget = NULL;
    }
  
  /* destroy minus pixmap */
  if (item->minus_pix_widget)
    {
      gtk_widget_destroy (item->minus_pix_widget);
      gtk_widget_unref (item->minus_pix_widget);
      item->minus_pix_widget = NULL;
    }
  
  /* By removing the pixmaps here, and not in unrealize, we depend on
   * the fact that a widget can never change colormap or visual.
   */
  gtk_wtree_item_remove_pixmaps (item);
  
  GTK_OBJECT_CLASS (parent_class)->destroy (object);
  
#ifdef WTREE_DEBUG
  g_message("- gtk_wtree_item_destroy\n");
#endif /* WTREE_DEBUG */
}

void
gtk_wtree_item_remove_subwtree (GtkWTreeItem* item) 
{
  g_return_if_fail (item != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM(item));
  g_return_if_fail (item->subwtree != NULL);
  
  if (GTK_WTREE (item->subwtree)->children)
    {
      /* The following call will remove the children and call
       * gtk_wtree_item_remove_subwtree() again. So we are done.
       */
      gtk_wtree_remove_items (GTK_WTREE (item->subwtree), 
			     GTK_WTREE (item->subwtree)->children);
      return;
    }

  if (GTK_WIDGET_MAPPED (item->subwtree))
    gtk_widget_unmap (item->subwtree);
      
  gtk_widget_unparent (item->subwtree);
  
  if (item->pixmaps_box)
    gtk_widget_hide (item->pixmaps_box);
  
  item->subwtree = NULL;

  if (item->expanded)
    {
      item->expanded = FALSE;
      if (item->pixmaps_box)
	{
	  gtk_container_remove (GTK_CONTAINER (item->pixmaps_box), 
				item->minus_pix_widget);
	  gtk_container_add (GTK_CONTAINER (item->pixmaps_box), 
			     item->plus_pix_widget);
	}
    }
}

static void
gtk_wtree_item_map (GtkWidget *widget)
{
  GtkBin *bin;
  GtkWTreeItem* item;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (widget));

  bin = GTK_BIN (widget);
  item = GTK_WTREE_ITEM(widget);

  GTK_WIDGET_SET_FLAGS (widget, GTK_MAPPED);

  if(item->pixmaps_box &&
     GTK_WIDGET_VISIBLE (item->pixmaps_box) &&
     !GTK_WIDGET_MAPPED (item->pixmaps_box))
    gtk_widget_map (item->pixmaps_box);

  if (bin->child &&
      GTK_WIDGET_VISIBLE (bin->child) &&
      !GTK_WIDGET_MAPPED (bin->child))
    gtk_widget_map (bin->child);

  gdk_window_show (widget->window);
}

static void
gtk_wtree_item_unmap (GtkWidget *widget)
{
  GtkBin *bin;
  GtkWTreeItem* item;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (widget));

  GTK_WIDGET_UNSET_FLAGS (widget, GTK_MAPPED);
  bin = GTK_BIN (widget);
  item = GTK_WTREE_ITEM(widget);

  gdk_window_hide (widget->window);

  if(item->pixmaps_box &&
     GTK_WIDGET_VISIBLE (item->pixmaps_box) &&
     GTK_WIDGET_MAPPED (item->pixmaps_box))
    gtk_widget_unmap (bin->child);

  if (bin->child &&
      GTK_WIDGET_VISIBLE (bin->child) &&
      GTK_WIDGET_MAPPED (bin->child))
    gtk_widget_unmap (bin->child);
}

static void
gtk_wtree_item_forall (GtkContainer *container,
		      gboolean      include_internals,
		      GtkCallback   callback,
		      gpointer      callback_data)
{
  GtkBin *bin;
  GtkWTreeItem *wtree_item;

  g_return_if_fail (container != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (container));
  g_return_if_fail (callback != NULL);

  bin = GTK_BIN (container);
  wtree_item = GTK_WTREE_ITEM (container);

  if (bin->child)
    (* callback) (bin->child, callback_data);
  if (include_internals && wtree_item->subwtree)
    (* callback) (wtree_item->subwtree, callback_data);
}
