#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gtklist.h>
#include "gtkwtree.h"
#include "gtkwtreeitem.h"

enum {
  SELECTION_CHANGED,
  SELECT_CHILD,
  UNSELECT_CHILD,
  LAST_SIGNAL
};

static void gtk_wtree_class_init      (GtkWTreeClass   *klass);
static void gtk_wtree_init            (GtkWTree        *wtree);
static void gtk_wtree_destroy         (GtkObject      *object);
static void gtk_wtree_map             (GtkWidget      *widget);
static void gtk_wtree_unmap           (GtkWidget      *widget);
static void gtk_wtree_realize         (GtkWidget      *widget);
static void gtk_wtree_draw            (GtkWidget      *widget,
				      GdkRectangle   *area);
static gint gtk_wtree_expose          (GtkWidget      *widget,
				      GdkEventExpose *event);
static gint gtk_wtree_motion_notify   (GtkWidget      *widget,
				      GdkEventMotion *event);
static gint gtk_wtree_button_press    (GtkWidget      *widget,
				      GdkEventButton *event);
static gint gtk_wtree_button_release  (GtkWidget      *widget,
				      GdkEventButton *event);
static void gtk_wtree_size_request    (GtkWidget      *widget,
				      GtkRequisition *requisition);
static void gtk_wtree_size_allocate   (GtkWidget      *widget,
				      GtkAllocation  *allocation);
static void gtk_wtree_add             (GtkContainer   *container,
				      GtkWidget      *widget);
static void gtk_wtree_forall          (GtkContainer   *container,
				      gboolean	      include_internals,
				      GtkCallback     callback,
				      gpointer        callback_data);

static void gtk_real_wtree_select_child   (GtkWTree       *wtree,
					  GtkWidget     *child);
static void gtk_real_wtree_unselect_child (GtkWTree       *wtree,
					  GtkWidget     *child);

static GtkType gtk_wtree_child_type  (GtkContainer   *container);

static GtkContainerClass *parent_class = NULL;
static guint wtree_signals[LAST_SIGNAL] = { 0 };

GtkType
gtk_wtree_get_type (void)
{
  static GtkType wtree_type = 0;
  
  if (!wtree_type)
    {
      static const GtkTypeInfo wtree_info =
      {
	"GtkWTree",
	sizeof (GtkWTree),
	sizeof (GtkWTreeClass),
	(GtkClassInitFunc) gtk_wtree_class_init,
	(GtkObjectInitFunc) gtk_wtree_init,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };
      
      wtree_type = gtk_type_unique (gtk_container_get_type (), &wtree_info);
    }
  
  return wtree_type;
}

static void
gtk_wtree_class_init (GtkWTreeClass *class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  GtkContainerClass *container_class;
  
  object_class = (GtkObjectClass*) class;
  widget_class = (GtkWidgetClass*) class;
  container_class = (GtkContainerClass*) class;
  
  parent_class = gtk_type_class (gtk_container_get_type ());
  
  wtree_signals[SELECTION_CHANGED] =
    gtk_signal_new ("selection_changed",
		    GTK_RUN_FIRST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (GtkWTreeClass, selection_changed),
		    gtk_marshal_NONE__NONE,
		    GTK_TYPE_NONE, 0);
  wtree_signals[SELECT_CHILD] =
    gtk_signal_new ("select_child",
		    GTK_RUN_FIRST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (GtkWTreeClass, select_child),
		    gtk_marshal_NONE__POINTER,
		    GTK_TYPE_NONE, 1,
		    GTK_TYPE_WIDGET);
  wtree_signals[UNSELECT_CHILD] =
    gtk_signal_new ("unselect_child",
		    GTK_RUN_FIRST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (GtkWTreeClass, unselect_child),
		    gtk_marshal_NONE__POINTER,
		    GTK_TYPE_NONE, 1,
		    GTK_TYPE_WIDGET);
  
  gtk_object_class_add_signals (object_class, wtree_signals, LAST_SIGNAL);
  
  object_class->destroy = gtk_wtree_destroy;
  
  widget_class->map = gtk_wtree_map;
  widget_class->unmap = gtk_wtree_unmap;
  widget_class->realize = gtk_wtree_realize;
  widget_class->draw = gtk_wtree_draw;
  widget_class->expose_event = gtk_wtree_expose;
  widget_class->motion_notify_event = gtk_wtree_motion_notify;
  widget_class->button_press_event = gtk_wtree_button_press;
  widget_class->button_release_event = gtk_wtree_button_release;
  widget_class->size_request = gtk_wtree_size_request;
  widget_class->size_allocate = gtk_wtree_size_allocate;
  
  container_class->add = gtk_wtree_add;
  container_class->remove = 
    (void (*)(GtkContainer *, GtkWidget *)) gtk_wtree_remove_item;
  container_class->forall = gtk_wtree_forall;
  container_class->child_type = gtk_wtree_child_type;
  
  class->selection_changed = NULL;
  class->select_child = gtk_real_wtree_select_child;
  class->unselect_child = gtk_real_wtree_unselect_child;
}

static GtkType
gtk_wtree_child_type (GtkContainer     *container)
{
  return GTK_TYPE_WTREE_ITEM;
}

static void
gtk_wtree_init (GtkWTree *wtree)
{
  wtree->children = NULL;
  wtree->root_wtree = NULL;
  wtree->selection = NULL;
  wtree->wtree_owner = NULL;
  wtree->selection_mode = GTK_SELECTION_SINGLE;
  wtree->indent_value = 9;
  wtree->current_indent = 0;
  wtree->level = 0;
  wtree->view_mode = GTK_WTREE_VIEW_LINE;
  wtree->view_line = 1;
}

GtkWidget*
gtk_wtree_new (void)
{
  return GTK_WIDGET (gtk_type_new (gtk_wtree_get_type ()));
}

void
gtk_wtree_append (GtkWTree   *wtree,
		 GtkWidget *wtree_item)
{
  g_return_if_fail (wtree != NULL);
  g_return_if_fail (GTK_IS_WTREE (wtree));
  g_return_if_fail (wtree_item != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (wtree_item));
  
  gtk_wtree_insert (wtree, wtree_item, -1);
}

void
gtk_wtree_prepend (GtkWTree   *wtree,
		  GtkWidget *wtree_item)
{
  g_return_if_fail (wtree != NULL);
  g_return_if_fail (GTK_IS_WTREE (wtree));
  g_return_if_fail (wtree_item != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (wtree_item));
  
  gtk_wtree_insert (wtree, wtree_item, 0);
}

void
gtk_wtree_insert (GtkWTree   *wtree,
		 GtkWidget *wtree_item,
		 gint       position)
{
  gint nchildren;
  
  g_return_if_fail (wtree != NULL);
  g_return_if_fail (GTK_IS_WTREE (wtree));
  g_return_if_fail (wtree_item != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (wtree_item));
  
  nchildren = g_list_length (wtree->children);
  
  if ((position < 0) || (position > nchildren))
    position = nchildren;
  
  if (position == nchildren)
    wtree->children = g_list_append (wtree->children, wtree_item);
  else
    wtree->children = g_list_insert (wtree->children, wtree_item, position);
  
  gtk_widget_set_parent (wtree_item, GTK_WIDGET (wtree));
  
  if (GTK_WIDGET_REALIZED (wtree_item->parent))
    gtk_widget_realize (wtree_item);

  if (GTK_WIDGET_VISIBLE (wtree_item->parent) && GTK_WIDGET_VISIBLE (wtree_item))
    {
      if (GTK_WIDGET_MAPPED (wtree_item->parent))
	gtk_widget_map (wtree_item);

      gtk_widget_queue_resize (wtree_item);
    }
}

static void
gtk_wtree_add (GtkContainer *container,
	      GtkWidget    *child)
{
  GtkWTree *wtree;
  
  g_return_if_fail (container != NULL);
  g_return_if_fail (GTK_IS_WTREE (container));
  g_return_if_fail (GTK_IS_WTREE_ITEM (child));
  
  wtree = GTK_WTREE (container);
  
  wtree->children = g_list_append (wtree->children, child);
  
  gtk_widget_set_parent (child, GTK_WIDGET (container));
  
  if (GTK_WIDGET_REALIZED (child->parent))
    gtk_widget_realize (child);

  if (GTK_WIDGET_VISIBLE (child->parent) && GTK_WIDGET_VISIBLE (child))
    {
      if (GTK_WIDGET_MAPPED (child->parent))
	gtk_widget_map (child);

      gtk_widget_queue_resize (child);
    }
  
  if (!wtree->selection && (wtree->selection_mode == GTK_SELECTION_BROWSE))
    gtk_wtree_select_child (wtree, child);
}

static gint
gtk_wtree_button_press (GtkWidget      *widget,
		       GdkEventButton *event)
{
  GtkWTree *wtree;
  GtkWidget *item;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_WTREE (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);
  
  wtree = GTK_WTREE (widget);
  item = gtk_get_event_widget ((GdkEvent*) event);
  
  while (item && !GTK_IS_WTREE_ITEM (item))
    item = item->parent;
  
  if (!item || (item->parent != widget))
    return FALSE;
  
  switch(event->button) 
    {
    case 1:
      gtk_wtree_select_child (wtree, item);
      break;
    case 2:
      if(GTK_WTREE_ITEM(item)->subwtree) gtk_wtree_item_expand(GTK_WTREE_ITEM(item));
      break;
    case 3:
      if(GTK_WTREE_ITEM(item)->subwtree) gtk_wtree_item_collapse(GTK_WTREE_ITEM(item));
      break;
    }
  
  return TRUE;
}

static gint
gtk_wtree_button_release (GtkWidget      *widget,
			 GdkEventButton *event)
{
  GtkWTree *wtree;
  GtkWidget *item;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_WTREE (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);
  
  wtree = GTK_WTREE (widget);
  item = gtk_get_event_widget ((GdkEvent*) event);
  
  return TRUE;
}

gint
gtk_wtree_child_position (GtkWTree   *wtree,
			 GtkWidget *child)
{
  GList *children;
  gint pos;
  
  
  g_return_val_if_fail (wtree != NULL, -1);
  g_return_val_if_fail (GTK_IS_WTREE (wtree), -1);
  g_return_val_if_fail (child != NULL, -1);
  
  pos = 0;
  children = wtree->children;
  
  while (children)
    {
      if (child == GTK_WIDGET (children->data)) 
	return pos;
      
      pos += 1;
      children = children->next;
    }
  
  
  return -1;
}

void
gtk_wtree_clear_items (GtkWTree *wtree,
		      gint     start,
		      gint     end)
{
  GtkWidget *widget;
  GList *clear_list;
  GList *tmp_list;
  guint nchildren;
  guint index;
  
  g_return_if_fail (wtree != NULL);
  g_return_if_fail (GTK_IS_WTREE (wtree));
  
  nchildren = g_list_length (wtree->children);
  
  if (nchildren > 0)
    {
      if ((end < 0) || (end > nchildren))
	end = nchildren;
      
      if (start >= end)
	return;
      
      tmp_list = g_list_nth (wtree->children, start);
      clear_list = NULL;
      index = start;
      while (tmp_list && index <= end)
	{
	  widget = tmp_list->data;
	  tmp_list = tmp_list->next;
	  index++;
	  
	  clear_list = g_list_prepend (clear_list, widget);
	}
      
      gtk_wtree_remove_items (wtree, clear_list);
    }
}

static void
gtk_wtree_destroy (GtkObject *object)
{
  GtkWTree *wtree;
  GtkWidget *child;
  GList *children;
  
  g_return_if_fail (object != NULL);
  g_return_if_fail (GTK_IS_WTREE (object));
  
  wtree = GTK_WTREE (object);
  
  children = wtree->children;
  while (children)
    {
      child = children->data;
      children = children->next;
      
      gtk_widget_ref (child);
      gtk_widget_unparent (child);
      gtk_widget_destroy (child);
      gtk_widget_unref (child);
    }
  
  g_list_free (wtree->children);
  wtree->children = NULL;
  
  if (wtree->root_wtree == wtree)
    {
      GList *node;
      for (node = wtree->selection; node; node = node->next)
	gtk_widget_unref ((GtkWidget *)node->data);
      g_list_free (wtree->selection);
      wtree->selection = NULL;
    }
  
  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
gtk_wtree_draw (GtkWidget    *widget,
	       GdkRectangle *area)
{
  GtkWTree *wtree;
  GtkWidget *subwtree;
  GtkWidget *child;
  GdkRectangle child_area;
  GList *children;
  
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_WTREE (widget));
  g_return_if_fail (area != NULL);
  
  if (GTK_WIDGET_DRAWABLE (widget))
    {
      wtree = GTK_WTREE (widget);
      
      children = wtree->children;
      while (children)
	{
	  child = children->data;
	  children = children->next;
	  
	  if (gtk_widget_intersect (child, area, &child_area))
	    gtk_widget_draw (child, &child_area);
	  
	  if((subwtree = GTK_WTREE_ITEM(child)->subwtree) &&
	     GTK_WIDGET_VISIBLE(subwtree) &&
	     gtk_widget_intersect (subwtree, area, &child_area))
	    gtk_widget_draw (subwtree, &child_area);
	}
    }
  
}

static gint
gtk_wtree_expose (GtkWidget      *widget,
		 GdkEventExpose *event)
{
  GtkWTree *wtree;
  GtkWidget *child;
  GdkEventExpose child_event;
  GList *children;
  
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_WTREE (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);
  
  if (GTK_WIDGET_DRAWABLE (widget))
    {
      wtree = GTK_WTREE (widget);
      
      child_event = *event;
      
      children = wtree->children;
      while (children)
	{
	  child = children->data;
	  children = children->next;
	  
	  if (GTK_WIDGET_NO_WINDOW (child) &&
	      gtk_widget_intersect (child, &event->area, &child_event.area))
	    gtk_widget_event (child, (GdkEvent*) &child_event);
	}
    }
  
  
  return FALSE;
}

static void
gtk_wtree_forall (GtkContainer *container,
		 gboolean      include_internals,
		 GtkCallback   callback,
		 gpointer      callback_data)
{
  GtkWTree *wtree;
  GtkWidget *child;
  GList *children;
  
  
  g_return_if_fail (container != NULL);
  g_return_if_fail (GTK_IS_WTREE (container));
  g_return_if_fail (callback != NULL);
  
  wtree = GTK_WTREE (container);
  children = wtree->children;
  
  while (children)
    {
      child = children->data;
      children = children->next;
      
      (* callback) (child, callback_data);
    }
}

static void
gtk_wtree_map (GtkWidget *widget)
{
  GtkWTree *wtree;
  GtkWidget *child;
  GList *children;
  
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_WTREE (widget));
  
  GTK_WIDGET_SET_FLAGS (widget, GTK_MAPPED);
  wtree = GTK_WTREE (widget);
  
  if(GTK_IS_WTREE(widget->parent)) 
    {
      /* set root wtree for this wtree */
      wtree->root_wtree = GTK_WTREE(widget->parent)->root_wtree;
      
      wtree->level = GTK_WTREE(GTK_WIDGET(wtree)->parent)->level+1;
      wtree->indent_value = GTK_WTREE(GTK_WIDGET(wtree)->parent)->indent_value;
      wtree->current_indent = GTK_WTREE(GTK_WIDGET(wtree)->parent)->current_indent + 
	wtree->indent_value;
      wtree->view_mode = GTK_WTREE(GTK_WIDGET(wtree)->parent)->view_mode;
      wtree->view_line = GTK_WTREE(GTK_WIDGET(wtree)->parent)->view_line;
    } 
  else
    wtree->root_wtree = wtree;
  
  children = wtree->children;
  while (children)
    {
      child = children->data;
      children = children->next;
      
      if (GTK_WIDGET_VISIBLE (child) &&
	  !GTK_WIDGET_MAPPED (child))
	gtk_widget_map (child);
      
      if (GTK_WTREE_ITEM (child)->subwtree)
	{
	  child = GTK_WIDGET (GTK_WTREE_ITEM (child)->subwtree);
	  
	  if (GTK_WIDGET_VISIBLE (child) && !GTK_WIDGET_MAPPED (child))
	    gtk_widget_map (child);
	}
    }

  gdk_window_show (widget->window);
}

static gint
gtk_wtree_motion_notify (GtkWidget      *widget,
			GdkEventMotion *event)
{
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_WTREE (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);
  
#ifdef WTREE_DEBUG
  g_message("gtk_wtree_motion_notify\n");
#endif /* WTREE_DEBUG */
  
  return FALSE;
}

static void
gtk_wtree_realize (GtkWidget *widget)
{
  GdkWindowAttr attributes;
  gint attributes_mask;
  
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_WTREE (widget));
  
  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
  
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);
  attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;
  
  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
  
  widget->window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attributes_mask);
  gdk_window_set_user_data (widget->window, widget);
  
  widget->style = gtk_style_attach (widget->style, widget->window);
  gdk_window_set_background (widget->window, 
			     &widget->style->bg[GTK_STATE_NORMAL]);
}

void
gtk_wtree_remove_item (GtkWTree      *container,
		      GtkWidget    *widget)
{
  GList *item_list;
  
  g_return_if_fail (container != NULL);
  g_return_if_fail (GTK_IS_WTREE (container));
  g_return_if_fail (widget != NULL);
  g_return_if_fail (container == GTK_WTREE (widget->parent));
  
  item_list = g_list_append (NULL, widget);
  
  gtk_wtree_remove_items (GTK_WTREE (container), item_list);
  
  g_list_free (item_list);
}

/* used by gtk_wtree_remove_items to make the function independant of
   order in list of items to remove.
   Sort item bu depth in wtree */
static gint 
gtk_wtree_sort_item_by_depth(GtkWidget* a, GtkWidget* b)
{
  if((GTK_WTREE(a->parent)->level) < (GTK_WTREE(b->parent)->level))
    return 1;
  if((GTK_WTREE(a->parent)->level) > (GTK_WTREE(b->parent)->level))
    return -1;
  
  return 0;
}

void
gtk_wtree_remove_items (GtkWTree *wtree,
		       GList   *items)
{
  GtkWidget *widget;
  GList *selected_widgets;
  GList *tmp_list;
  GList *sorted_list;
  GtkWTree *real_wtree;
  GtkWTree *root_wtree;
  
  g_return_if_fail (wtree != NULL);
  g_return_if_fail (GTK_IS_WTREE (wtree));
  
#ifdef WTREE_DEBUG
  g_message("+ gtk_wtree_remove_items [ wtree %#x items list %#x ]\n", (int)wtree, (int)items);
#endif /* WTREE_DEBUG */
  
  /* We may not yet be mapped, so we actively have to find our
   * root wtree
   */
  if (wtree->root_wtree)
    root_wtree = wtree->root_wtree;
  else
    {
      GtkWidget *tmp = GTK_WIDGET (wtree);
      while (tmp->parent && GTK_IS_WTREE (tmp->parent))
	tmp = tmp->parent;
      
      root_wtree = GTK_WTREE (tmp);
    }
  
  tmp_list = items;
  selected_widgets = NULL;
  sorted_list = NULL;
  widget = NULL;
  
#ifdef WTREE_DEBUG
  g_message("* sort list by depth\n");
#endif /* WTREE_DEBUG */
  
  while (tmp_list)
    {
      
#ifdef WTREE_DEBUG
      g_message ("* item [%#x] depth [%d]\n", 
		 (int)tmp_list->data,
		 (int)GTK_WTREE(GTK_WIDGET(tmp_list->data)->parent)->level);
#endif /* WTREE_DEBUG */
      
      sorted_list = g_list_insert_sorted(sorted_list,
					 tmp_list->data,
					 (GCompareFunc)gtk_wtree_sort_item_by_depth);
      tmp_list = g_list_next(tmp_list);
    }
  
#ifdef WTREE_DEBUG
  /* print sorted list */
  g_message("* sorted list result\n");
  tmp_list = sorted_list;
  while(tmp_list)
    {
      g_message("* item [%#x] depth [%d]\n", 
		(int)tmp_list->data,
		(int)GTK_WTREE(GTK_WIDGET(tmp_list->data)->parent)->level);
      tmp_list = g_list_next(tmp_list);
    }
#endif /* WTREE_DEBUG */
  
#ifdef WTREE_DEBUG
  g_message("* scan sorted list\n");
#endif /* WTREE_DEBUG */
  
  tmp_list = sorted_list;
  while (tmp_list)
    {
      widget = tmp_list->data;
      tmp_list = tmp_list->next;
      
#ifdef WTREE_DEBUG
      g_message("* item [%#x] subwtree [%#x]\n", 
		(int)widget, (int)GTK_WTREE_ITEM_SUBWTREE(widget));
#endif /* WTREE_DEBUG */
      
      /* get real owner of this widget */
      real_wtree = GTK_WTREE(widget->parent);
#ifdef WTREE_DEBUG
      g_message("* subwtree having this widget [%#x]\n", (int)real_wtree);
#endif /* WTREE_DEBUG */
      
      
      if (widget->state == GTK_STATE_SELECTED)
	{
	  selected_widgets = g_list_prepend (selected_widgets, widget);
#ifdef WTREE_DEBUG
	  g_message("* selected widget - adding it in selected list [%#x]\n",
		    (int)selected_widgets);
#endif /* WTREE_DEBUG */
	}
      
      /* remove this item from its real parent */
#ifdef WTREE_DEBUG
      g_message("* remove widget from its owner wtree\n");
#endif /* WTREE_DEBUG */
      real_wtree->children = g_list_remove (real_wtree->children, widget);
      
      /* remove subwtree associate at this item if it exist */      
      if(GTK_WTREE_ITEM(widget)->subwtree) 
	{
#ifdef WTREE_DEBUG
	  g_message("* remove subwtree associate at this item [%#x]\n",
		    (int) GTK_WTREE_ITEM(widget)->subwtree);
#endif /* WTREE_DEBUG */
	  if (GTK_WIDGET_MAPPED (GTK_WTREE_ITEM(widget)->subwtree))
	    gtk_widget_unmap (GTK_WTREE_ITEM(widget)->subwtree);
	  
	  gtk_widget_unparent (GTK_WTREE_ITEM(widget)->subwtree);
	  GTK_WTREE_ITEM(widget)->subwtree = NULL;
	}
      
      /* really remove widget for this item */
#ifdef WTREE_DEBUG
      g_message("* unmap and unparent widget [%#x]\n", (int)widget);
#endif /* WTREE_DEBUG */
      if (GTK_WIDGET_MAPPED (widget))
	gtk_widget_unmap (widget);
      
      gtk_widget_unparent (widget);
      
      /* delete subwtree if there is no children in it */
      if(real_wtree->children == NULL && 
	 real_wtree != root_wtree)
	{
#ifdef WTREE_DEBUG
	  g_message("* owner wtree don't have children ... destroy it\n");
#endif /* WTREE_DEBUG */
	  gtk_wtree_item_remove_subwtree(GTK_WTREE_ITEM(real_wtree->wtree_owner));
	}
      
#ifdef WTREE_DEBUG
      g_message("* next item in list\n");
#endif /* WTREE_DEBUG */
    }
  
  if (selected_widgets)
    {
#ifdef WTREE_DEBUG
      g_message("* scan selected item list\n");
#endif /* WTREE_DEBUG */
      tmp_list = selected_widgets;
      while (tmp_list)
	{
	  widget = tmp_list->data;
	  tmp_list = tmp_list->next;
	  
#ifdef WTREE_DEBUG
	  g_message("* widget [%#x] subwtree [%#x]\n", 
		    (int)widget, (int)GTK_WTREE_ITEM_SUBWTREE(widget));
#endif /* WTREE_DEBUG */
	  
	  /* remove widget of selection */
	  root_wtree->selection = g_list_remove (root_wtree->selection, widget);
	  
	  /* unref it to authorize is destruction */
	  gtk_widget_unref (widget);
	}
      
      /* emit only one selection_changed signal */
      gtk_signal_emit (GTK_OBJECT (root_wtree), 
		       wtree_signals[SELECTION_CHANGED]);
    }
  
#ifdef WTREE_DEBUG
  g_message("* free selected_widgets list\n");
#endif /* WTREE_DEBUG */
  g_list_free (selected_widgets);
  g_list_free (sorted_list);
  
  if (root_wtree->children && !root_wtree->selection &&
      (root_wtree->selection_mode == GTK_SELECTION_BROWSE))
    {
#ifdef WTREE_DEBUG
      g_message("* BROWSE mode, select another item\n");
#endif /* WTREE_DEBUG */
      widget = root_wtree->children->data;
      gtk_wtree_select_child (root_wtree, widget);
    }
  
  if (GTK_WIDGET_VISIBLE (root_wtree))
    {
#ifdef WTREE_DEBUG
      g_message("* query queue resizing for root_wtree\n");
#endif /* WTREE_DEBUG */      
      gtk_widget_queue_resize (GTK_WIDGET (root_wtree));
    }
}

void
gtk_wtree_select_child (GtkWTree   *wtree,
		       GtkWidget *wtree_item)
{
  g_return_if_fail (wtree != NULL);
  g_return_if_fail (GTK_IS_WTREE (wtree));
  g_return_if_fail (wtree_item != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (wtree_item));
  
  gtk_signal_emit (GTK_OBJECT (wtree), wtree_signals[SELECT_CHILD], wtree_item);
}

void
gtk_wtree_select_item (GtkWTree   *wtree,
		      gint       item)
{
  GList *tmp_list;
  
  g_return_if_fail (wtree != NULL);
  g_return_if_fail (GTK_IS_WTREE (wtree));
  
  tmp_list = g_list_nth (wtree->children, item);
  if (tmp_list)
    gtk_wtree_select_child (wtree, GTK_WIDGET (tmp_list->data));
  
}

static void
gtk_wtree_size_allocate (GtkWidget     *widget,
			GtkAllocation *allocation)
{
  GtkWTree *wtree;
  GtkWidget *child, *subwtree;
  GtkAllocation child_allocation;
  GList *children;
  
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_WTREE (widget));
  g_return_if_fail (allocation != NULL);
  
  wtree = GTK_WTREE (widget);
  
  widget->allocation = *allocation;
  if (GTK_WIDGET_REALIZED (widget))
    gdk_window_move_resize (widget->window,
			    allocation->x, allocation->y,
			    allocation->width, allocation->height);
  
  if (wtree->children)
    {
      child_allocation.x = GTK_CONTAINER (wtree)->border_width;
      child_allocation.y = GTK_CONTAINER (wtree)->border_width;
      child_allocation.width = MAX (1, (gint)allocation->width - 
				    child_allocation.x * 2);
      
      children = wtree->children;
      
      while (children)
	{
	  child = children->data;
	  children = children->next;
	  
	  if (GTK_WIDGET_VISIBLE (child))
	    {
	      GtkRequisition child_requisition;
	      gtk_widget_get_child_requisition (child, &child_requisition);
	      
	      child_allocation.height = child_requisition.height;
	      
	      gtk_widget_size_allocate (child, &child_allocation);
	      
	      child_allocation.y += child_allocation.height;
	      
	      if((subwtree = GTK_WTREE_ITEM(child)->subwtree))
		if(GTK_WIDGET_VISIBLE (subwtree))
		  {
		    child_allocation.height = subwtree->requisition.height;
		    gtk_widget_size_allocate (subwtree, &child_allocation);
		    child_allocation.y += child_allocation.height;
		  }
	    }
	}
    }
  
}

static void
gtk_wtree_size_request (GtkWidget      *widget,
		       GtkRequisition *requisition)
{
  GtkWTree *wtree;
  GtkWidget *child, *subwtree;
  GList *children;
  GtkRequisition child_requisition;
  
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_WTREE (widget));
  g_return_if_fail (requisition != NULL);
  
  wtree = GTK_WTREE (widget);
  requisition->width = 0;
  requisition->height = 0;
  
  children = wtree->children;
  while (children)
    {
      child = children->data;
      children = children->next;
      
      if (GTK_WIDGET_VISIBLE (child))
	{
	  gtk_widget_size_request (child, &child_requisition);
	  
	  requisition->width = MAX (requisition->width, child_requisition.width);
	  requisition->height += child_requisition.height;
	  
	  if((subwtree = GTK_WTREE_ITEM(child)->subwtree) &&
	     GTK_WIDGET_VISIBLE (subwtree))
	    {
	      gtk_widget_size_request (subwtree, &child_requisition);
	      
	      requisition->width = MAX (requisition->width, 
					child_requisition.width);
	      
	      requisition->height += child_requisition.height;
	    }
	}
    }
  
  requisition->width += GTK_CONTAINER (wtree)->border_width * 2;
  requisition->height += GTK_CONTAINER (wtree)->border_width * 2;
  
  requisition->width = MAX (requisition->width, 1);
  requisition->height = MAX (requisition->height, 1);
  
}

static void
gtk_wtree_unmap (GtkWidget *widget)
{
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_WTREE (widget));
  
  GTK_WIDGET_UNSET_FLAGS (widget, GTK_MAPPED);
  gdk_window_hide (widget->window);
  
}

void
gtk_wtree_unselect_child (GtkWTree   *wtree,
			 GtkWidget *wtree_item)
{
  g_return_if_fail (wtree != NULL);
  g_return_if_fail (GTK_IS_WTREE (wtree));
  g_return_if_fail (wtree_item != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (wtree_item));
  
  gtk_signal_emit (GTK_OBJECT (wtree), wtree_signals[UNSELECT_CHILD], wtree_item);
}

void
gtk_wtree_unselect_item (GtkWTree *wtree,
			gint     item)
{
  GList *tmp_list;
  
  g_return_if_fail (wtree != NULL);
  g_return_if_fail (GTK_IS_WTREE (wtree));
  
  tmp_list = g_list_nth (wtree->children, item);
  if (tmp_list)
    gtk_wtree_unselect_child (wtree, GTK_WIDGET (tmp_list->data));
  
}

static void
gtk_real_wtree_select_child (GtkWTree   *wtree,
			    GtkWidget *child)
{
  GList *selection, *root_selection;
  GList *tmp_list;
  GtkWidget *tmp_item;
  
  g_return_if_fail (wtree != NULL);
  g_return_if_fail (GTK_IS_WTREE (wtree));
  g_return_if_fail (child != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (child));

  if (!wtree->root_wtree)
    {
      g_warning ("gtk_real_wtree_select_child(): unable to select a child in a wtree prior to realization");
      return;
    }
  
  root_selection = wtree->root_wtree->selection;
  
  switch (wtree->root_wtree->selection_mode)
    {
    case GTK_SELECTION_SINGLE:
      
      selection = root_selection;
      
      /* remove old selection list */
      while (selection)
	{
	  tmp_item = selection->data;
	  
	  if (tmp_item != child)
	    {
	      gtk_wtree_item_deselect (GTK_WTREE_ITEM (tmp_item));
	      
	      tmp_list = selection;
	      selection = selection->next;
	      
	      root_selection = g_list_remove_link (root_selection, tmp_list);
	      gtk_widget_unref (tmp_item);
	      
	      g_list_free (tmp_list);
	    }
	  else
	    selection = selection->next;
	}
      
      if (child->state == GTK_STATE_NORMAL)
	{
	  gtk_wtree_item_select (GTK_WTREE_ITEM (child));
	  root_selection = g_list_prepend (root_selection, child);
	  gtk_widget_ref (child);
	}
      else if (child->state == GTK_STATE_SELECTED)
	{
	  gtk_wtree_item_deselect (GTK_WTREE_ITEM (child));
	  root_selection = g_list_remove (root_selection, child);
	  gtk_widget_unref (child);
	}
      
      wtree->root_wtree->selection = root_selection;
      
      gtk_signal_emit (GTK_OBJECT (wtree->root_wtree), 
		       wtree_signals[SELECTION_CHANGED]);
      break;
      
      
    case GTK_SELECTION_BROWSE:
      selection = root_selection;
      
      while (selection)
	{
	  tmp_item = selection->data;
	  
	  if (tmp_item != child)
	    {
	      gtk_wtree_item_deselect (GTK_WTREE_ITEM (tmp_item));
	      
	      tmp_list = selection;
	      selection = selection->next;
	      
	      root_selection = g_list_remove_link (root_selection, tmp_list);
	      gtk_widget_unref (tmp_item);
	      
	      g_list_free (tmp_list);
	    }
	  else
	    selection = selection->next;
	}
      
      wtree->root_wtree->selection = root_selection;
      
      if (child->state == GTK_STATE_NORMAL)
	{
	  gtk_wtree_item_select (GTK_WTREE_ITEM (child));
	  root_selection = g_list_prepend (root_selection, child);
	  gtk_widget_ref (child);
	  wtree->root_wtree->selection = root_selection;
	  gtk_signal_emit (GTK_OBJECT (wtree->root_wtree), 
			   wtree_signals[SELECTION_CHANGED]);
	}
      break;
      
    case GTK_SELECTION_MULTIPLE:
      if (child->state == GTK_STATE_NORMAL)
	{
	  gtk_wtree_item_select (GTK_WTREE_ITEM (child));
	  root_selection = g_list_prepend (root_selection, child);
	  gtk_widget_ref (child);
	  wtree->root_wtree->selection = root_selection;
	  gtk_signal_emit (GTK_OBJECT (wtree->root_wtree), 
			   wtree_signals[SELECTION_CHANGED]);
	}
      else if (child->state == GTK_STATE_SELECTED)
	{
	  gtk_wtree_item_deselect (GTK_WTREE_ITEM (child));
	  root_selection = g_list_remove (root_selection, child);
	  gtk_widget_unref (child);
	  wtree->root_wtree->selection = root_selection;
	  gtk_signal_emit (GTK_OBJECT (wtree->root_wtree), 
			   wtree_signals[SELECTION_CHANGED]);
	}
      break;
      
    case GTK_SELECTION_EXTENDED:
      break;
    }
}

static void
gtk_real_wtree_unselect_child (GtkWTree   *wtree,
			      GtkWidget *child)
{
  g_return_if_fail (wtree != NULL);
  g_return_if_fail (GTK_IS_WTREE (wtree));
  g_return_if_fail (child != NULL);
  g_return_if_fail (GTK_IS_WTREE_ITEM (child));
  
  switch (wtree->selection_mode)
    {
    case GTK_SELECTION_SINGLE:
    case GTK_SELECTION_MULTIPLE:
    case GTK_SELECTION_BROWSE:
      if (child->state == GTK_STATE_SELECTED)
	{
	  GtkWTree* root_wtree = GTK_WTREE_ROOT_WTREE(wtree);
	  gtk_wtree_item_deselect (GTK_WTREE_ITEM (child));
	  root_wtree->selection = g_list_remove (root_wtree->selection, child);
	  gtk_widget_unref (child);
	  gtk_signal_emit (GTK_OBJECT (wtree->root_wtree), 
			   wtree_signals[SELECTION_CHANGED]);
	}
      break;
      
    case GTK_SELECTION_EXTENDED:
      break;
    }
}

void
gtk_wtree_set_selection_mode (GtkWTree       *wtree,
			     GtkSelectionMode mode) 
{
  g_return_if_fail (wtree != NULL);
  g_return_if_fail (GTK_IS_WTREE (wtree));
  
  wtree->selection_mode = mode;
}

void
gtk_wtree_set_view_mode (GtkWTree       *wtree,
			GtkWTreeViewMode mode) 
{
  g_return_if_fail (wtree != NULL);
  g_return_if_fail (GTK_IS_WTREE (wtree));
  
  wtree->view_mode = mode;
}

void
gtk_wtree_set_view_lines (GtkWTree       *wtree,
			 guint          flag) 
{
  g_return_if_fail (wtree != NULL);
  g_return_if_fail (GTK_IS_WTREE (wtree));
  
  wtree->view_line = flag;
}
