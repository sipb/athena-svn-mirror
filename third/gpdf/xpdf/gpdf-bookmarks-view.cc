/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-indent-level: 8; c-basic-offset: 8 -*- */
/*  PDF view widget
 *
 *  Copyright (C) 2003 Remi Cohen-Scali
 *
 *  Author:
 *    Remi Cohen-Scali <Remi@Cohen-Scali.com>
 *
 * GPdf is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPdf is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include <gpdf-g-switch.h>
#  include <gdk-pixbuf/gdk-pixbuf.h>
#  include <gtk/gtk.h>
#  include <libgnome/gnome-i18n.h>
#  include <libgnomeui/gnome-app-helper.h>
#  include "gpdf-stock-icons.h"
#  include "gpdf-util.h"
#  include "gpdf-marshal.h"
#include <gpdf-g-switch.h>

#include <aconf.h>
#include "goo/GList.h"
#include "Object.h"
#include "Catalog.h"
#include "Link.h"
#include "Dict.h"
#include "Stream.h"
#include "Page.h"
#include "Outline.h"
#include "UnicodeMap.h"
#include "GlobalParams.h"
#include "gpdf-control-private.h"

BEGIN_EXTERN_C

#include "gpdf-bookmarks-view.h"

struct _GPdfBookmarksGenState {
	GList *items;
	int i;
	GtkTreeIter *parent_iter;
	int level; 
	struct _GPdfBookmarksGenState *next; 
};
typedef struct _GPdfBookmarksGenState GPdfBookmarksGenState; 

struct _GPdfBookmarksViewPrivate {

	GPdfView *gpdf_view; 
	PDFDoc *pdf_doc;

	GtkWidget *treeview;

	gulong expand_id;
	gulong collapse_id;
	gulong selection_id;

	GdkPixbuf *stock_book_closed_pixbuf;
	GdkPixbuf *stock_book_opened_pixbuf;
	GdkPixbuf *stock_book_closed_mark_pixbuf;
	GdkPixbuf *stock_book_opened_mark_pixbuf;
	GdkPixbuf *stock_bookmarks_pixbuf;

	GdkPixbuf *stock_first_pixbuf; 
	GdkPixbuf *stock_last_pixbuf; 
	GdkPixbuf *stock_prev_pixbuf; 
	GdkPixbuf *stock_next_pixbuf; 
	GdkPixbuf *stock_close_pixbuf; 
	GdkPixbuf *stock_quit_pixbuf;
	GdkPixbuf *stock_net_pixbuf;

	GnomeUIInfo *popup_menu_uiinfo;
	GtkWidget *popup_menu;

	GtkTreeStore *model;

	Outline *outlines;
	UnicodeMap *umap;

	gint current_page;

	gboolean generation_terminated;
	GPdfBookmarksGenState *generation_state, *generation_head;
	guint idle_id; 
	gulong idle_dcon_id;

	GPdfControl *parent; 
};

static void	gpdf_bookmarks_view_class_init	      (GPdfBookmarksViewClass*); 
static void	gpdf_bookmarks_view_dispose	      (GObject*); 
static void	gpdf_bookmarks_view_finalize	      (GObject*); 
static void	gpdf_bookmarks_view_instance_init     (GPdfBookmarksView*);
static void     gpdf_bookmarks_view_row_expanded_cb   (GtkTreeView*, GtkTreeIter*,
						       GtkTreePath*, gpointer); 
static void     gpdf_bookmarks_view_row_collapsed_cb  (GtkTreeView*, GtkTreeIter*,
						       GtkTreePath*, gpointer);
static void	gpdf_bookmarks_view_page_changed_cb   (GPdfView *gpdf_view,
						       int 	 page,
						       gpointer  user_data);
static void	gpdf_bookmarks_view_popup_menu_item_expand_cb 	    (GtkMenuItem*,
								     GPdfBookmarksView*); 
static void	gpdf_bookmarks_view_popup_menu_item_expand_all_cb   (GtkMenuItem*,
								     GPdfBookmarksView*); 
static void	gpdf_bookmarks_view_popup_menu_item_collapse_cb     (GtkMenuItem*,
								     GPdfBookmarksView*); 
static void	gpdf_bookmarks_view_popup_menu_item_collapse_all_cb (GtkMenuItem*,
								     GPdfBookmarksView*); 

#define GPDF_IS_NON_NULL_BOOKMARKS_VIEW(obj) \
		(((obj) != NULL) && (GPDF_IS_BOOKMARKS_VIEW ((obj))))

enum {
	GPDF_BKVIEW_COLUMN1 = 0, 
	GPDF_BKVIEW_COLUMN2, 
	GPDF_BKVIEW_COLUMN3,
	GPDF_BKVIEW_COLUMN4,
	GPDF_BKVIEW_COLUMN5,
	NUM_COLUMNS
};

enum {
	BOOKMARK_SELECTED_SIGNAL = 0,
	READY_SIGNAL, 
	LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_VIEW,
	PROP_CONTROL
};

#define POPUP_MENU_EXPAND_ITEM			"Expand"
#define POPUP_MENU_EXPAND_ITEM_TIP		"Expand all bookmarks under this one"
#define POPUP_MENU_COLLAPSE_ITEM		"Collapse"
#define POPUP_MENU_COLLAPSE_ITEM_TIP		"Collapse all bookmarks under this one"
#define POPUP_MENU_EXPAND_ALL_ITEM		"Expand All"
#define POPUP_MENU_EXPAND_ALL_ITEM_TIP		"Expand All bookmarks in bookmarks tree"
#define POPUP_MENU_COLLAPSE_ALL_ITEM		"Collapse All"
#define POPUP_MENU_COLLAPSE_ALL_ITEM_TIP	"Collapse All bookmarks in bookmarks tree"

#define POPUP_MENU_EXPAND_INDEX			0
#define POPUP_MENU_COLLAPSE_INDEX		1
#define POPUP_MENU_EXPAND_ALL_INDEX		2
#define POPUP_MENU_COLLAPSE_ALL_INDEX		3

static GnomeUIInfo tools_popup_menu_items_init[] =
{
      GNOMEUIINFO_ITEM_NONE (N_ (POPUP_MENU_EXPAND_ITEM),
			     N_ (POPUP_MENU_EXPAND_ITEM_TIP),
			     gpdf_bookmarks_view_popup_menu_item_expand_cb),
      GNOMEUIINFO_ITEM_NONE (N_ (POPUP_MENU_COLLAPSE_ITEM),
			     N_ (POPUP_MENU_COLLAPSE_ITEM_TIP),
			     gpdf_bookmarks_view_popup_menu_item_collapse_cb),
      GNOMEUIINFO_ITEM_NONE (N_ (POPUP_MENU_EXPAND_ALL_ITEM),
			     N_ (POPUP_MENU_EXPAND_ALL_ITEM_TIP),
			     gpdf_bookmarks_view_popup_menu_item_expand_all_cb),
      GNOMEUIINFO_ITEM_NONE (N_ (POPUP_MENU_COLLAPSE_ALL_ITEM),
			     N_ (POPUP_MENU_COLLAPSE_ALL_ITEM_TIP),
			     gpdf_bookmarks_view_popup_menu_item_collapse_all_cb), 
      GNOMEUIINFO_END
};

static guint gpdf_bookmarks_view_signals [LAST_SIGNAL];

GPDF_CLASS_BOILERPLATE(GPdfBookmarksView, gpdf_bookmarks_view, GtkScrolledWindow, GTK_TYPE_SCROLLED_WINDOW); 

static void
gpdf_bookmarks_view_set_property (GObject *object, guint param_id,
				  const GValue *value, GParamSpec *pspec)
{
	GPdfBookmarksView *bookmarks_view;

	g_return_if_fail (GPDF_IS_BOOKMARKS_VIEW (object));

	bookmarks_view = GPDF_BOOKMARKS_VIEW (object);

	switch (param_id) {
	case PROP_VIEW:
		bookmarks_view->priv->gpdf_view = 
		  GPDF_VIEW (g_value_get_object (value));
		break;
	case PROP_CONTROL:
		bookmarks_view->priv->parent = 
		  GPDF_CONTROL (g_value_get_object (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
gpdf_bookmarks_view_class_init (GPdfBookmarksViewClass *klass)
{
	GObjectClass *gobject_class;
	
	gobject_class = G_OBJECT_CLASS (klass);

	parent_class = GTK_SCROLLED_WINDOW_CLASS (g_type_class_peek_parent (klass));

	gobject_class->dispose = gpdf_bookmarks_view_dispose;
	gobject_class->finalize = gpdf_bookmarks_view_finalize;
	gobject_class->set_property = gpdf_bookmarks_view_set_property;

	g_object_class_install_property (
		gobject_class, PROP_VIEW,
		g_param_spec_object (
			"view",
			"Parent view",
			"Parent view",
			GPDF_TYPE_VIEW, 
		        (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE)));

	g_object_class_install_property (
		gobject_class, PROP_CONTROL,
		g_param_spec_object (
			"control",
			"Parent control",
			"Parent control",
			GPDF_TYPE_CONTROL, 
		        (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE)));

	gpdf_bookmarks_view_signals [BOOKMARK_SELECTED_SIGNAL] =
	  g_signal_new (
	    "bookmark_selected",
	    G_TYPE_FROM_CLASS (gobject_class),
	    G_SIGNAL_RUN_LAST,
	    G_STRUCT_OFFSET (GPdfBookmarksViewClass, bookmark_selected),
	    NULL, NULL,
	    gpdf_marshal_VOID__POINTER,
	    G_TYPE_NONE, 1, G_TYPE_POINTER);

	gpdf_bookmarks_view_signals [READY_SIGNAL] =
	  g_signal_new (
	    "ready",
	    G_TYPE_FROM_CLASS (gobject_class),
	    G_SIGNAL_RUN_LAST,
	    G_STRUCT_OFFSET (GPdfBookmarksViewClass, ready),
	    NULL, NULL,
	    gpdf_marshal_VOID__VOID,
	    G_TYPE_NONE, 0);
}

static void
gpdf_bookmarks_view_emit_bookmark_selected (GPdfBookmarksView *bookmarks_view,
					    LinkAction	      *link)
{
	g_signal_emit (G_OBJECT (bookmarks_view),
		       gpdf_bookmarks_view_signals [BOOKMARK_SELECTED_SIGNAL],
		       0, link);
}

static void
gpdf_bookmarks_view_emit_ready (GPdfBookmarksView *bookmarks_view)
{
	g_signal_emit (G_OBJECT (bookmarks_view),
		       gpdf_bookmarks_view_signals [READY_SIGNAL],
		       0);
}

/*
 * This macro simplify iconb rendering code and increase lisibility
 */
#define GPDF_RENDER_STOCK_ICON(widget, dest, id, size) 					\
{											\
	GtkIconSet *icon_set;								\
	icon_set = gtk_style_lookup_icon_set (gtk_widget_get_style (widget),		\
					      id);					\
	if (icon_set != NULL)								\
		 dest = gtk_icon_set_render_icon (icon_set,				\
						  gtk_widget_get_style (widget),	\
						  gtk_widget_get_direction (widget),	\
						  GTK_STATE_NORMAL,			\
						  size,					\
						  NULL, NULL);				\
}

static void
gpdf_bookmarks_view_construct (GPdfBookmarksView *bookmarks_view)
{
	GPdfBookmarksViewPrivate *priv;
	GtkWidget *bkwidget; 

	g_return_if_fail (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (bookmarks_view)); 

	priv = bookmarks_view->priv;

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (bookmarks_view),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (bookmarks_view),
					     GTK_SHADOW_NONE);
	
	/* Exprim interrest in page changed signal */
	g_signal_connect (G_OBJECT (priv->gpdf_view), "page_changed",
			  G_CALLBACK (gpdf_bookmarks_view_page_changed_cb),
			  bookmarks_view);

	/* Create tree view */
	priv->treeview = gtk_tree_view_new ();
	gtk_widget_show (priv->treeview);
	gtk_container_add (GTK_CONTAINER (bookmarks_view), priv->treeview);
	
	priv->expand_id =
	  g_signal_connect (G_OBJECT (priv->treeview), "row_expanded",
			    G_CALLBACK (gpdf_bookmarks_view_row_expanded_cb),
			    bookmarks_view);

	priv->collapse_id =
	  g_signal_connect (G_OBJECT (priv->treeview), "row_collapsed",
			    G_CALLBACK (gpdf_bookmarks_view_row_collapsed_cb),
			    bookmarks_view);
	
	bkwidget = GTK_WIDGET (bookmarks_view);

	/* Toolbar bookmarks icon */
	GPDF_RENDER_STOCK_ICON(bkwidget,
			       priv->stock_bookmarks_pixbuf, 
			       GPDF_STOCK_BOOKMARKS,
			       GTK_ICON_SIZE_SMALL_TOOLBAR); 

	/* Tree closed book w/o mark */
	GPDF_RENDER_STOCK_ICON(bkwidget,
			       priv->stock_book_closed_pixbuf, 
			       GPDF_STOCK_BOOK_CLOSED,
			       GTK_ICON_SIZE_SMALL_TOOLBAR); 
	/* Tree opened book w/o mark */
	GPDF_RENDER_STOCK_ICON(bkwidget,
			       priv->stock_book_opened_pixbuf, 
			       GPDF_STOCK_BOOK_OPENED,
			       GTK_ICON_SIZE_SMALL_TOOLBAR);

	/* Tree closed book w/ mark */
	GPDF_RENDER_STOCK_ICON(bkwidget,
			       priv->stock_book_closed_mark_pixbuf, 
			       GPDF_STOCK_BOOK_CLOSED_MARK,
			       GTK_ICON_SIZE_SMALL_TOOLBAR); 
	/* Tree opened book w/ mark */
	GPDF_RENDER_STOCK_ICON(bkwidget,
			       priv->stock_book_opened_mark_pixbuf, 
			       GPDF_STOCK_BOOK_OPENED_MARK,
			       GTK_ICON_SIZE_SMALL_TOOLBAR);

	/* Stock first */
	GPDF_RENDER_STOCK_ICON(bkwidget,
			       priv->stock_first_pixbuf, 
			       GTK_STOCK_GOTO_FIRST,
			       GTK_ICON_SIZE_SMALL_TOOLBAR);
	/* Stock last */
	GPDF_RENDER_STOCK_ICON(bkwidget,
			       priv->stock_last_pixbuf, 
			       GTK_STOCK_GOTO_LAST,
			       GTK_ICON_SIZE_SMALL_TOOLBAR); 

	/* Stock previous */
	GPDF_RENDER_STOCK_ICON(bkwidget,
			       priv->stock_prev_pixbuf, 
			       GTK_STOCK_GO_BACK,
			       GTK_ICON_SIZE_SMALL_TOOLBAR); 
	/* Stock next */
	GPDF_RENDER_STOCK_ICON(bkwidget,
			       priv->stock_next_pixbuf, 
			       GTK_STOCK_GO_FORWARD,
			       GTK_ICON_SIZE_SMALL_TOOLBAR); 

	/* Stock close */
	GPDF_RENDER_STOCK_ICON(bkwidget,
			       priv->stock_close_pixbuf, 
			       GTK_STOCK_CLOSE,
			       GTK_ICON_SIZE_SMALL_TOOLBAR); 

	/* Stock quit */
	GPDF_RENDER_STOCK_ICON(bkwidget,
			       priv->stock_quit_pixbuf, 
			       GTK_STOCK_QUIT,
			       GTK_ICON_SIZE_SMALL_TOOLBAR);

	/* Stock URI */
	GPDF_RENDER_STOCK_ICON(bkwidget,
			       priv->stock_net_pixbuf, 
			       GTK_STOCK_NETWORK,
			       GTK_ICON_SIZE_SMALL_TOOLBAR);

}

static void
gpdf_bookmarks_view_row_expanded_cb (GtkTreeView *treeview,
				     GtkTreeIter *iter,
				     GtkTreePath *path,
				     gpointer user_data)
{
	GPdfBookmarksView *bookmarks_view = GPDF_BOOKMARKS_VIEW (user_data);
	GPdfBookmarksViewPrivate *priv;
	GValue page_nr_value = {0, };
	gint page_nr; 
	GdkPixbuf *bookmark_pixbuf = NULL;
	gboolean page_displayed; 
	
	g_return_if_fail (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (bookmarks_view));
	priv = bookmarks_view->priv;

	if (!priv->generation_terminated) return; 

	/* Get bookmark page nr */
	gtk_tree_model_get_value (GTK_TREE_MODEL(priv->model),
				  iter,
				  GPDF_BKVIEW_COLUMN5,
				  &page_nr_value);
	page_nr = g_value_get_int ((const GValue *)&page_nr_value);

	/* Is its page displayed */
	page_displayed = (page_nr == priv->current_page);
	
	/* Change appearance accordingly */
	if (page_displayed)
		bookmark_pixbuf = priv->stock_book_opened_mark_pixbuf; 
	else
		bookmark_pixbuf = priv->stock_book_opened_pixbuf;
	
	gtk_tree_store_set (GTK_TREE_STORE (priv->model), iter,
			    GPDF_BKVIEW_COLUMN1, bookmark_pixbuf,
			    -1);
	
}

static void
gpdf_bookmarks_view_row_collapsed_cb (GtkTreeView *treeview,
				      GtkTreeIter *iter,
				      GtkTreePath *path,
				      gpointer user_data)
{
	GPdfBookmarksView *bookmarks_view = GPDF_BOOKMARKS_VIEW (user_data);
	GPdfBookmarksViewPrivate *priv; 
	GValue page_nr_value = {0, };
	gint page_nr; 
	GdkPixbuf *bookmark_pixbuf = NULL;
	gboolean page_displayed; 
	
	g_return_if_fail (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (bookmarks_view));
	priv = bookmarks_view->priv; 

	if (!priv->generation_terminated) return; 

	/* Get bookmark page nr */
	gtk_tree_model_get_value (GTK_TREE_MODEL (priv->model),
				  iter,
				  GPDF_BKVIEW_COLUMN5,
				  &page_nr_value);
	page_nr = g_value_get_int ((const GValue *)&page_nr_value);

	/* Is its page displayed */
	page_displayed = (page_nr == priv->current_page);

	/* Change appearance accordingly */
	if (page_displayed)
		bookmark_pixbuf = priv->stock_book_closed_mark_pixbuf; 
	else
		bookmark_pixbuf = priv->stock_book_closed_pixbuf; 

	gtk_tree_store_set (GTK_TREE_STORE (priv->model), iter,
			    GPDF_BKVIEW_COLUMN1, bookmark_pixbuf,
			    -1);
	
}

gboolean
gpdf_bookmarks_view_update_bkvisual (GtkTreeModel *model, 
				     GtkTreePath *path, 
				     GtkTreeIter *iter,
				     gpointer data)
{
	GPdfBookmarksView *bookmarks_view;
	GPdfBookmarksViewPrivate *priv;
	GValue page_nr_value = {0, };
	OutlineItem *anItem;
	GValue an_item_value = {0, };
	gint page_nr;
	GdkPixbuf *bookmark_pixbuf = NULL;
	gboolean page_displayed, is_expanded; 
	
	bookmarks_view = GPDF_BOOKMARKS_VIEW (data);

	g_return_val_if_fail (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (bookmarks_view),
			      FALSE);

	priv = bookmarks_view->priv;

	/* Get item to access action kind */
	gtk_tree_model_get_value (model,
				  iter,
				  GPDF_BKVIEW_COLUMN4,
				  &an_item_value);
	anItem = (OutlineItem *) g_value_get_pointer ((const GValue *)&an_item_value);

	/* anItem is NULL if no action was associated with the node,
	 * see also gpdf_bookmarks_view_flat_recurse_outlines */
	if (anItem && anItem->getAction ()->getKind () == actionGoTo)
	{
		/* Get this bookmark page nr */
		gtk_tree_model_get_value (model,
					  iter,
					  GPDF_BKVIEW_COLUMN5,
					  &page_nr_value);
		page_nr = g_value_get_int ((const GValue *)&page_nr_value);
		
		/* Get its displayed/expanded states */
		page_displayed = (page_nr == priv->current_page);
		is_expanded = (gtk_tree_view_row_expanded (GTK_TREE_VIEW (priv->treeview),
							   path));
		
		/* And take pixbuf accordingly to set in store */
		if (page_displayed) {
			if (is_expanded)
			  bookmark_pixbuf = priv->stock_book_opened_mark_pixbuf; 
			else
			  bookmark_pixbuf = priv->stock_book_closed_mark_pixbuf; 
		}
		else {
			if (is_expanded)
			  bookmark_pixbuf = priv->stock_book_opened_pixbuf; 
			else
			  bookmark_pixbuf = priv->stock_book_closed_pixbuf; 
		}
		gtk_tree_store_set (GTK_TREE_STORE (model), iter,
				    GPDF_BKVIEW_COLUMN1, bookmark_pixbuf,
				    -1);

		/* For convinience, expand and scroll to current page bookmarks */
		if (page_displayed)
		{
			gtk_tree_view_expand_to_path (
				GTK_TREE_VIEW (priv->treeview),
				path);
			
			gtk_tree_view_scroll_to_cell (
				GTK_TREE_VIEW (priv->treeview),
				path,
				gtk_tree_view_get_column(
					GTK_TREE_VIEW (priv->treeview),
					GPDF_BKVIEW_COLUMN2),
				TRUE, 0.0, 0.0); 
		}
	}
		
	return FALSE; 
}

static void
gpdf_bookmarks_view_page_changed_cb (GPdfView *gpdf_view,
				     int page,
				     gpointer user_data)
{
	GPdfBookmarksView *bookmarks_view;
	GtkTreeModel *model;

	bookmarks_view = GPDF_BOOKMARKS_VIEW (user_data);

	g_return_if_fail (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (bookmarks_view));
	g_return_if_fail (GPDF_IS_VIEW (gpdf_view));

	/* Just store current page */
	bookmarks_view->priv->current_page = page;

	/* If view not yet constructed */
	if (!bookmarks_view->priv->model || 
	    !bookmarks_view->priv->generation_terminated) return;

	/* Else update  bookmarks tree status */
	model = GTK_TREE_MODEL (bookmarks_view->priv->model);
	gtk_tree_model_foreach (model,
				gpdf_bookmarks_view_update_bkvisual,
				bookmarks_view); 
}

GtkWidget *
gpdf_bookmarks_view_new (GPdfControl *control, GtkWidget *gpdf_view)
{
	GPdfBookmarksView *bookmarks_view;

	bookmarks_view =
	  GPDF_BOOKMARKS_VIEW (g_object_new (
		  GPDF_TYPE_BOOKMARKS_VIEW,
		  "view", GPDF_VIEW (gpdf_view),
		  "control", control, 
		  NULL));

	g_object_ref (G_OBJECT (bookmarks_view->priv->gpdf_view));
	g_object_ref (G_OBJECT (bookmarks_view->priv->parent));
	
	gpdf_bookmarks_view_construct (bookmarks_view);
	
	return GTK_WIDGET (bookmarks_view);
}


static void
gpdf_bookmarks_view_dispose (GObject *object)
{
	GPdfBookmarksView *bookmarks_view;
	GPdfBookmarksViewPrivate *priv;
    
	g_return_if_fail (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (object));
    
	bookmarks_view = GPDF_BOOKMARKS_VIEW (object);
	priv = bookmarks_view->priv;

	if (priv->expand_id) {
		g_signal_handler_disconnect (priv->treeview, priv->expand_id);
		priv->expand_id = (gulong)0L;
	}
	if (priv->collapse_id) {
		g_signal_handler_disconnect (priv->treeview, priv->collapse_id);
		priv->collapse_id = (gulong)0L;
	}
	if (priv->selection_id) {
		g_signal_handler_disconnect
		  (gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview)),
		   priv->selection_id);
		priv->selection_id = (gulong)0L;
	}
    
	if (priv->model) {
		gtk_tree_store_clear (priv->model); 
		g_object_unref (priv->model);
		priv->model = NULL; 
	}
	if (priv->umap) {
		priv->umap->decRefCnt ();
		priv->umap = NULL;
	}
	if (priv->gpdf_view) {
		g_object_unref (priv->gpdf_view);
		priv->gpdf_view = NULL; 
	}
	if (priv->parent) {
		g_object_unref (priv->parent);
		priv->parent = NULL; 
	}
	if (priv->popup_menu) {
		gtk_widget_destroy (priv->popup_menu);
		priv->popup_menu = NULL;
		g_free (priv->popup_menu_uiinfo);
	}

	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
gpdf_bookmarks_view_finalize (GObject *object)
{
	GPdfBookmarksView *bookmarks_view;
	GPdfBookmarksViewPrivate *priv;
    
	g_return_if_fail (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (object));
    
	bookmarks_view = GPDF_BOOKMARKS_VIEW (object);
	priv = bookmarks_view->priv;
    
	if (bookmarks_view->priv) {
		g_free (bookmarks_view->priv);
		bookmarks_view->priv = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gpdf_bookmarks_view_instance_init (GPdfBookmarksView *bookmarks_view)
{
	bookmarks_view->priv = g_new0 (GPdfBookmarksViewPrivate, 1);
	bookmarks_view->priv->popup_menu_uiinfo =
	  (GnomeUIInfo*)g_memdup (tools_popup_menu_items_init,
				  sizeof (tools_popup_menu_items_init));
}

static gchar *
unicode_to_char (OutlineItem *outline_item, 
		 UnicodeMap *uMap)
{
	GString gstr;
	gchar buf[8]; /* 8 is enough for mapping an unicode char to a string */
	int i, n;
	
	for (i = 0; i < outline_item->getTitleLength(); ++i) {
		n = uMap->mapUnicode(outline_item->getTitle()[i], buf, sizeof(buf));
		gstr.append(buf, n);
	}

	return g_strdup (gstr.getCString ());
}

static gboolean
gpdf_bookmarks_view_have_outline_items (GPdfBookmarksView *bookmarks_view)
{
	GPdfBookmarksViewPrivate *priv;

	g_return_val_if_fail
	  (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (bookmarks_view),
	   FALSE);
	
	priv = bookmarks_view->priv;
	
	return (priv->outlines != NULL &&
		priv->outlines->getItems () !=  NULL &&
		priv->outlines->getItems ()->getLength () > 0);
}

static void
gpdf_bookmarks_view_update_popup_actions (GPdfBookmarksView *bookmarks_view)
{
	GPdfBookmarksViewPrivate *priv;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter; 
	G_List *selrows;
	GtkTreePath *path; 
	GtkWidget *item; 
	gboolean is_expandable = FALSE, is_expanded = FALSE;

	g_return_if_fail (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (bookmarks_view));

	if (!gpdf_bookmarks_view_have_outline_items (bookmarks_view))
	  return;
	
	priv = bookmarks_view->priv;
	model = GTK_TREE_MODEL (priv->model); 

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
	selrows = gtk_tree_selection_get_selected_rows (selection, &model);

	if (g_list_length (selrows) == 1) {

		path = (GtkTreePath *)g_list_nth_data (selrows, 0);
		if (gtk_tree_model_get_iter (model, &iter, path)) {

			is_expandable = gtk_tree_model_iter_has_child(GTK_TREE_MODEL (priv->model),
								      &iter); 
			is_expanded = gtk_tree_view_row_expanded (GTK_TREE_VIEW (priv->treeview),
								  path);
		}
	}

	if (gpdf_bookmarks_view_have_outline_items (bookmarks_view)) {
		item = priv->popup_menu_uiinfo[POPUP_MENU_EXPAND_INDEX].widget;
		if (GTK_IS_WIDGET (item))
			gtk_widget_set_sensitive (item, is_expandable && !is_expanded); 
	
		item = priv->popup_menu_uiinfo[POPUP_MENU_COLLAPSE_INDEX].widget;
		if (GTK_IS_WIDGET (item))
			gtk_widget_set_sensitive (item, is_expandable && is_expanded);	
	}
}

static gboolean
gpdf_bookmarks_view_bookmark_select_func (GtkTreeSelection  *selection,
					  GtkTreeModel	    *model,
					  GtkTreePath	    *path,
					  gboolean	     path_currently_selected,
					  gpointer	     data)
{
	GPdfBookmarksView *bookmarks_view;
	GPdfBookmarksViewPrivate *priv;	
	GValue selection_item = {0,};
	GtkTreeIter iterator;
	OutlineItem *item;
	gboolean ret = FALSE;

	bookmarks_view = GPDF_BOOKMARKS_VIEW (data);
	
	g_return_val_if_fail (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (bookmarks_view),
			      FALSE);

	do {
		priv = bookmarks_view->priv;

		gtk_tree_model_get_iter (GTK_TREE_MODEL (priv->model),
					 &iterator,
					 path);

		/* handle only if is about to be selected */
		if (!gtk_tree_selection_get_selected (selection, 
						      &model,
						      &iterator)) {
			gtk_tree_model_get_value (GTK_TREE_MODEL (priv->model),
						  &iterator,
						  GPDF_BKVIEW_COLUMN4,
						  &selection_item);
			item = (OutlineItem *)g_value_peek_pointer ((const GValue*) &selection_item);

			gpdf_bookmarks_view_emit_bookmark_selected (bookmarks_view,
								    item ? item->getAction () : NULL);
		}
		
		ret = TRUE;
	}
	while (0); 

	return ret;
}

static void
gpdf_bookmarks_view_flat_recurse_outlines (GPdfBookmarksView *bookmarks_view)
{
	GPdfBookmarksViewPrivate *priv;
	GdkPixbuf *bookmark_pixbuf = NULL; 
	LinkAction *link_action = NULL;
	LinkNamed *link_named = NULL;
	LinkGoTo *link_goto = NULL;
	LinkDest *link_dest = NULL;
	LinkURI *link_uri = NULL;
	GString *named_dest; 
	Ref page_ref;
	gchar *action_name; 
	int i, page = 0;
	gchar *pagestr = NULL;
	gboolean is_expanded; 
	gchar *status = NULL;
	double ratio = 0.0; 
	
	g_return_if_fail (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (bookmarks_view)); 

	priv = bookmarks_view->priv; 

	i = priv->generation_state->i;
	if (i < priv->generation_state->items->getLength())
	{
		GtkTreeIter child_iter;
		OutlineItem *anItem;
		Unicode *title; 
		
		anItem = (OutlineItem *)priv->generation_state->items->get(i);
		priv->generation_state->i++;
		
		link_action = anItem->getAction ();
		title = anItem->getTitle (); 
		if (link_action) {
			gtk_tree_store_append (GTK_TREE_STORE (priv->model),
					       &child_iter,
					       priv->generation_state->parent_iter);
			
			is_expanded =
				(gtk_tree_view_row_expanded (
					GTK_TREE_VIEW (priv->treeview),
					gtk_tree_model_get_path (
						GTK_TREE_MODEL (priv->model),
						&child_iter)));

			switch (link_action->getKind ()) {

			case actionGoTo:
				link_goto = dynamic_cast <LinkGoTo *> (link_action);
				link_dest = link_goto->getDest ();
				named_dest = link_goto->getNamedDest (); 
				if (link_dest != NULL) {
					link_dest = link_dest->copy ();
				} else if (named_dest != NULL) {
					named_dest = named_dest->copy ();
					link_dest = priv->pdf_doc->findDest (named_dest);
					delete named_dest;
				}
				if (link_dest != NULL) {
					if (link_dest->isPageRef ()) {
						page_ref = link_dest->getPageRef ();
						page = priv->pdf_doc->findPage (page_ref.num, page_ref.gen);
					} else {
						page = link_dest->getPageNum ();
					}
					
					delete link_dest;
				}
				pagestr = g_strdup_printf ("%d", page); 
				
				if (page == priv->current_page) {
					if (is_expanded)
						bookmark_pixbuf = priv->stock_book_opened_mark_pixbuf;
					else
						bookmark_pixbuf = priv->stock_book_closed_mark_pixbuf;
				}
				else {
					if (is_expanded)
						bookmark_pixbuf = priv->stock_book_opened_pixbuf;
					else
						bookmark_pixbuf = priv->stock_book_closed_pixbuf;
				}
				break;
				
			case actionNamed:
				link_named = dynamic_cast <LinkNamed *> (link_action); 
				action_name = link_named->getName ()->getCString ();
				if (strcmp (action_name, "NextPage") == 0) {
					page = priv->current_page +1;
					bookmark_pixbuf = priv->stock_next_pixbuf; 
				} else if (strcmp (action_name, "PrevPage") == 0) {
					page = priv->current_page -1; 
					bookmark_pixbuf = priv->stock_prev_pixbuf; 
				} else if (strcmp (action_name, "FirstPage") == 0) {
					page = 1; 
					bookmark_pixbuf = priv->stock_first_pixbuf; 
				} else if (strcmp (action_name, "LastPage") == 0) {
					page = priv->pdf_doc->getNumPages (); 
					bookmark_pixbuf = priv->stock_last_pixbuf; 
				} else if (strcmp (action_name, "Close") == 0) {
					page = -1; 
					bookmark_pixbuf = priv->stock_close_pixbuf; 
				} else if (strcmp (action_name, "Quit") == 0) {
					page = -1; 
					bookmark_pixbuf = priv->stock_quit_pixbuf; 
				} else {
					g_warning ("Unimplemented named link action: %s", action_name);
					pagestr = g_strdup_printf (_("** Unknown %s **"), action_name); 
				}
				if (page == -1)
					pagestr = g_strdup (action_name); 
				else if (page > 0)
					pagestr = g_strdup_printf ("%d", page);
				break;

			case actionURI:
				link_uri = dynamic_cast <LinkURI *> (link_action);
				pagestr = g_strdup (link_uri->getURI ()->getCString ());
				bookmark_pixbuf = priv->stock_net_pixbuf;
				break;

			default:
				pagestr = g_strdup ("** Unknown action **"); 
				g_warning ("Unkown link action type: %d", link_action->getKind ());
			}

			ratio = ((double)page/(double)priv->pdf_doc->getNumPages ()); 
			status = g_strdup_printf ("Processing bookmarks on page %d/%d: %2d %%",
						  page, 
						  priv->pdf_doc->getNumPages (), 
						  (int)(ratio*100.0));
			gpdf_control_private_set_status (priv->parent, status);
			gpdf_control_private_set_fraction (priv->parent, ratio); 
			g_free (status);
			
			gtk_tree_store_set (GTK_TREE_STORE (priv->model), &child_iter,
					    GPDF_BKVIEW_COLUMN1, bookmark_pixbuf, 
					    GPDF_BKVIEW_COLUMN2, unicode_to_char (anItem, priv->umap), 
					    GPDF_BKVIEW_COLUMN3, pagestr,
					    GPDF_BKVIEW_COLUMN4, (gpointer)anItem,
					    GPDF_BKVIEW_COLUMN5, page, 
					    -1);
		}
		else if (title) {
			gtk_tree_store_append (GTK_TREE_STORE (priv->model),
					       &child_iter,
					       priv->generation_state->parent_iter);
			
			is_expanded =
				(gtk_tree_view_row_expanded (
					GTK_TREE_VIEW (priv->treeview),
					gtk_tree_model_get_path (
						GTK_TREE_MODEL (priv->model),
						&child_iter)));

			if (is_expanded)
				bookmark_pixbuf = priv->stock_book_opened_mark_pixbuf;
			else
				bookmark_pixbuf = priv->stock_book_closed_mark_pixbuf;
			
			gtk_tree_store_set (GTK_TREE_STORE (priv->model), &child_iter,
					    GPDF_BKVIEW_COLUMN1, bookmark_pixbuf, 
					    GPDF_BKVIEW_COLUMN2, unicode_to_char (anItem, priv->umap), 
					    GPDF_BKVIEW_COLUMN3, "0",
					    GPDF_BKVIEW_COLUMN4, (gpointer)NULL,
					    GPDF_BKVIEW_COLUMN5, 0, 
					    -1);
		}
		
		anItem->open ();
		if (anItem->hasKids () && anItem->getKids ())
		{
			GPdfBookmarksGenState *next_state = priv->generation_state;
			
			priv->generation_state = g_new0 (GPdfBookmarksGenState, 1);
			priv->generation_state->items = anItem->getKids ();
			priv->generation_state->parent_iter = gtk_tree_iter_copy (&child_iter); 
			priv->generation_state->level = next_state->level +1; 
			priv->generation_state->next = next_state;
		}
	}
	else {
		GPdfBookmarksGenState *prev_state = priv->generation_state->next;

		g_free (priv->generation_state->parent_iter); 
		g_free (priv->generation_state);
		priv->generation_state = prev_state; 
	}
}

static void
gpdf_bookmarks_view_update_bookmarks_tree (GPdfBookmarksView *bookmarks_view)
{
	GPdfBookmarksViewPrivate *priv;
	GList *items;
	GString *enc; 
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;
	
	g_return_if_fail (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (bookmarks_view));
	
	priv = bookmarks_view->priv;
	if (priv->generation_state == NULL)
	{
		items = priv->outlines->getItems(); 
	
		if (priv->model) {
			gtk_tree_store_clear(priv->model);
			g_object_unref (priv->model);
			priv->model = NULL;
		}
	
		priv->model = gtk_tree_store_new (NUM_COLUMNS,
						  GDK_TYPE_PIXBUF,
						  G_TYPE_STRING,
						  G_TYPE_STRING,
						  G_TYPE_POINTER,
						  G_TYPE_INT);

		if (!priv->umap) {
			enc = new GString("UTF-8");
			priv->umap = globalParams->getUnicodeMap(enc);
			priv->umap->incRefCnt (); 
			delete enc;
		}

		gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview),
					 GTK_TREE_MODEL (priv->model));
		
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
		gtk_tree_selection_set_select_function (selection,
							gpdf_bookmarks_view_bookmark_select_func,
							bookmarks_view, NULL);
		
		gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (priv->treeview), TRUE);
		
		renderer = gtk_cell_renderer_pixbuf_new ();
		column = gtk_tree_view_column_new_with_attributes (NULL, renderer,
								   "pixbuf", GPDF_BKVIEW_COLUMN1, 
								   NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview), column);
		
		renderer = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Title"), renderer,
								   "text", GPDF_BKVIEW_COLUMN2,
								   NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview), column);
		
		renderer = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Page"), renderer,
								   "text", GPDF_BKVIEW_COLUMN3,
								   NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview), column);

		priv->generation_state = g_new0 (GPdfBookmarksGenState, 1);
		priv->generation_head = priv->generation_state; 
		priv->generation_state->items = items;
		priv->generation_state->level = 0; 
	}

	if (priv->generation_state == priv->generation_head && \
	    priv->generation_head->i == priv->generation_head->items->getLength())
		priv->generation_terminated = TRUE;
	else
		gpdf_bookmarks_view_flat_recurse_outlines (bookmarks_view); 
}

static void
gpdf_bookmarks_view_selection_changed_cb (GtkTreeSelection *selection, gpointer data)
{
	GPdfBookmarksView *bookmarks_view = GPDF_BOOKMARKS_VIEW (data);

	g_return_if_fail (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (bookmarks_view));
	
	if (!bookmarks_view->priv->generation_terminated) return;
		
	gpdf_bookmarks_view_update_popup_actions (bookmarks_view);
}

static void
gpdf_bookmarks_view_expand_unique_root (GPdfBookmarksView *bookmarks_view)
{
	GPdfBookmarksViewPrivate *priv;
	GtkTreeModel *model; 
	GtkTreeIter iter;
	GtkTreePath *path;

	g_return_if_fail (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (bookmarks_view));
	
	priv = bookmarks_view->priv;
	model = GTK_TREE_MODEL (priv->model); 

	if (gtk_tree_model_get_iter_root (model, &iter)) {
		if (!gtk_tree_model_iter_next (model, &iter))
		{
			path = gtk_tree_model_get_path (model, &iter);
			gtk_tree_view_expand_row (GTK_TREE_VIEW (priv->treeview),
						  path,
						  FALSE); 
		}
	}
}

static gboolean
gpdf_bookmarks_view_populate_idle (GPdfBookmarksView *bookmarks_view)
{
	GPdfBookmarksViewPrivate *priv;
	GtkTreeSelection *selection; 
	
	g_return_val_if_fail (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (bookmarks_view), TRUE);
	
	priv = bookmarks_view->priv;

	/* Create bookmarks tree */
	if (gpdf_bookmarks_view_have_outline_items (bookmarks_view)) {
		
		gpdf_bookmarks_view_update_bookmarks_tree (bookmarks_view);

		if (priv->generation_terminated)
		{
			/* Add listener for popup actions sensitivity */
			selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview)); 
			priv->selection_id =
				g_signal_connect (G_OBJECT (selection), "changed",
						  G_CALLBACK (gpdf_bookmarks_view_selection_changed_cb), 
						  bookmarks_view);
			
			/* If root is unique, let's expand it */
			gpdf_bookmarks_view_expand_unique_root (bookmarks_view);
		}
	}
	else {
		priv->generation_terminated = TRUE; 
	}

	if (priv->generation_terminated) {
		GdkDisplay *display;
		GdkWindow *parent_window;

		display = gtk_widget_get_display (priv->treeview);
		parent_window = GTK_WIDGET (priv->treeview)->window;
		if (GDK_IS_WINDOW (parent_window))
			gdk_window_set_cursor (parent_window, NULL); 
		gdk_flush();
		if (priv->idle_dcon_id)
		  g_signal_handler_disconnect (G_OBJECT (priv->parent), priv->idle_dcon_id); 
		gpdf_control_private_clear_stack (priv->parent);
		gpdf_control_private_set_fraction (priv->parent, 0.0); 
		gpdf_bookmarks_view_emit_ready (bookmarks_view);
	}
	else {
		GdkDisplay *display;
		GdkCursor *cursor;
		GdkWindow *parent_window;
		
		/* Set watch cursor while view not ready */
		display = gtk_widget_get_display (priv->treeview);
		cursor = gdk_cursor_new_for_display (display, GDK_WATCH);
		parent_window = GTK_WIDGET (priv->treeview)->window;
		if (GDK_IS_WINDOW (parent_window))
			gdk_window_set_cursor (parent_window, cursor); 
		gdk_cursor_unref (cursor);
		gdk_flush();

	}

	return (!priv->generation_terminated);
}

static void
disconnected_handler (gpointer control, gpointer data)
{
	g_source_remove (GPOINTER_TO_UINT (data));
}

void
gpdf_bookmarks_view_set_pdf_doc (GPdfBookmarksView *bookmarks_view,
				 PDFDoc *pdf_doc)
{
	GPdfBookmarksViewPrivate *priv;
	
	g_return_if_fail (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (bookmarks_view));
	
	priv = bookmarks_view->priv;
	
	if (pdf_doc != priv->pdf_doc)
	{		
		priv->pdf_doc = pdf_doc;
		priv->outlines = pdf_doc->getOutline ();
		/* Current page set through 'page_changed' signal */
		
		priv->idle_id = g_idle_add ((GSourceFunc)gpdf_bookmarks_view_populate_idle,
					    bookmarks_view);

		g_signal_connect (G_OBJECT (priv->parent), "disconnected", 
				  G_CALLBACK (disconnected_handler),
				  GUINT_TO_POINTER (priv->idle_id));
	}
}

static void
gpdf_bookmarks_view_popup_menu_item_expand_cb (GtkMenuItem       *menuitem,
					       GPdfBookmarksView *bookmarks_view)
{
	GPdfBookmarksViewPrivate *priv;
	GtkTreeSelection *selection;
	GtkTreeModel *model; 
	G_List *selrows;
	GtkTreePath *path; 
	
	g_return_if_fail (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (bookmarks_view));
	
	priv = bookmarks_view->priv;
	model = GTK_TREE_MODEL (priv->model); 
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
	selrows = gtk_tree_selection_get_selected_rows (selection, &model);

	if (g_list_length (selrows) == 1) {
		path = (GtkTreePath *)g_list_nth_data (selrows, 0);
		(void)gtk_tree_view_expand_row (GTK_TREE_VIEW (priv->treeview),
						path, FALSE); 
		gpdf_bookmarks_view_update_popup_actions (bookmarks_view);
	}
}

static void
gpdf_bookmarks_view_popup_menu_item_expand_all_cb (GtkMenuItem       *menuitem,
						   GPdfBookmarksView *bookmarks_view)
{
	GPdfBookmarksViewPrivate *priv;
	
	g_return_if_fail (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (bookmarks_view));
	
	priv = bookmarks_view->priv;

	gtk_tree_view_expand_all (GTK_TREE_VIEW (priv->treeview)); 

	gpdf_bookmarks_view_update_popup_actions (bookmarks_view); 
}

static void
gpdf_bookmarks_view_popup_menu_item_collapse_cb (GtkMenuItem       *menuitem,
						 GPdfBookmarksView *bookmarks_view)
{
	GPdfBookmarksViewPrivate *priv;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	G_List *selrows;
	GtkTreePath *path; 
	
	g_return_if_fail (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (bookmarks_view));
	
	priv = bookmarks_view->priv;
	model = GTK_TREE_MODEL (priv->model); 
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
	selrows = gtk_tree_selection_get_selected_rows (selection, &model);

	if (g_list_length (selrows) == 1) {
		path = (GtkTreePath *)g_list_nth_data (selrows, 0);
		(void)gtk_tree_view_collapse_row (GTK_TREE_VIEW (priv->treeview), path); 

		gpdf_bookmarks_view_update_popup_actions (bookmarks_view);
	}
}

static void
gpdf_bookmarks_view_popup_menu_item_collapse_all_cb (GtkMenuItem       *menuitem,
						     GPdfBookmarksView *bookmarks_view)
{
	GPdfBookmarksViewPrivate *priv;
	
	g_return_if_fail (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (bookmarks_view));
	
	priv = bookmarks_view->priv;
	
	gtk_tree_view_collapse_all (GTK_TREE_VIEW (priv->treeview)); 

	gpdf_bookmarks_view_update_popup_actions (bookmarks_view); 
}

GtkWidget*
gpdf_bookmarks_view_get_tools_menu (GPdfBookmarksView *bookmarks_view)
{
	GPdfBookmarksViewPrivate *priv;
	GtkWidget *item; 
	
	g_return_val_if_fail (GPDF_IS_NON_NULL_BOOKMARKS_VIEW (bookmarks_view), NULL);
	
	priv = bookmarks_view->priv;

	if (!gpdf_bookmarks_view_have_outline_items (bookmarks_view))
	  return NULL;
	
	if (!priv->popup_menu) {
		priv->popup_menu = gtk_menu_new ();
		gnome_app_fill_menu_with_data (GTK_MENU_SHELL (priv->popup_menu),
					       priv->popup_menu_uiinfo,
					       NULL, FALSE, 0,
					       (gpointer)bookmarks_view);

		item = priv->popup_menu_uiinfo[POPUP_MENU_EXPAND_INDEX].widget;
		gtk_widget_set_sensitive (item, FALSE);

		item = priv->popup_menu_uiinfo[POPUP_MENU_COLLAPSE_INDEX].widget;
		gtk_widget_set_sensitive (item, FALSE); 

		item = priv->popup_menu_uiinfo[POPUP_MENU_EXPAND_ALL_INDEX].widget;
		gtk_widget_set_sensitive (item, TRUE); 

		item = priv->popup_menu_uiinfo[POPUP_MENU_COLLAPSE_ALL_INDEX].widget;
		gtk_widget_set_sensitive (item, TRUE); 
	}
	
	return priv->popup_menu; 
}

END_EXTERN_C
