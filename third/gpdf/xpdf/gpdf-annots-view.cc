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
#  include <glade/glade.h>
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
#include "Annot.h"
#include "UnicodeMap.h"
#include "GlobalParams.h"

BEGIN_EXTERN_C

#include "gpdf-annots-view.h"
#include "gpdf-control-private.h"

typedef enum _GPdfAnnotationType {
    GPDF_ANNOT_TYPE_TEXT = 0,
    GPDF_ANNOT_TYPE_LINK, 
    GPDF_ANNOT_TYPE_FREETEXT, 
    GPDF_ANNOT_TYPE_LINE, 
    GPDF_ANNOT_TYPE_SQUARE, 
    GPDF_ANNOT_TYPE_CIRCLE, 
    GPDF_ANNOT_TYPE_HIGHLIGHT, 
    GPDF_ANNOT_TYPE_UNDERLINE, 
    GPDF_ANNOT_TYPE_STRIKEOUT, 
    GPDF_ANNOT_TYPE_STAMP, 
    GPDF_ANNOT_TYPE_INK, 
    GPDF_ANNOT_TYPE_POPUP, 
    GPDF_ANNOT_TYPE_FILEATTACHMENT, 
    GPDF_ANNOT_TYPE_SOUND, 
    GPDF_ANNOT_TYPE_MOVIE, 
    GPDF_ANNOT_TYPE_WIDGET, 
    GPDF_ANNOT_TYPE_TRAPNET,
    GPDF_ANNOT_TYPE_UNKNOWN,
    NUM_ANNOT_TYPES
} GPdfAnnotationType;

typedef struct _GPdfAnnotData {
        GPdfAnnotationType type; 
        gint page_nr;
        Annot *annot;
	gint refnum;
	gint refgen;
} GPdfAnnotData;

static const gchar * const gpdf_annotation_types_str[NUM_ANNOT_TYPES] = {
        "Text",
        "Link", 
        "FreeText", 
        "Line", 
        "Square", 
        "Circle", 
        "Highlight", 
        "Underline", 
        "StrikeOut", 
        "Stamp", 
        "Ink", 
        "Popup", 
        "FileAttachment", 
        "Sound", 
        "Movie", 
        "Widget", 
        "TrapNet",
        NULL,
};

static const gchar * const gpdf_annotation_icon_ids[NUM_ANNOT_TYPES] = {
        GPDF_STOCK_ANNOT_TEXT,
        GPDF_STOCK_ANNOT_LINK,
        GPDF_STOCK_ANNOT_FREETEXT,
        GPDF_STOCK_ANNOT_LINE,
        GPDF_STOCK_ANNOT_SQUARE,
        GPDF_STOCK_ANNOT_CIRCLE,
        GPDF_STOCK_ANNOT_HIGHLIGHT,
        GPDF_STOCK_ANNOT_UNDERLINE,
        GPDF_STOCK_ANNOT_STRIKEOUT,
        GPDF_STOCK_ANNOT_STAMP,
        GPDF_STOCK_ANNOT_INK,
        GPDF_STOCK_ANNOT_POPUP,
        GPDF_STOCK_ANNOT_FILEATTACHMENT,
        GPDF_STOCK_ANNOT_SOUND,
        GPDF_STOCK_ANNOT_MOVIE,
        GPDF_STOCK_ANNOT_WIDGET,
        GPDF_STOCK_ANNOT_TRAPNET,
        GPDF_STOCK_ANNOT_UNKNOWN
};

struct _GPdfAnnotsViewPrivate {

	GPdfView *gpdf_view; 
	PDFDoc *pdf_doc;

	GtkWidget *listview;

	GdkPixbuf *stock_annots_pixbuf[NUM_ANNOT_TYPES];

	GtkListStore *model;

	GnomeUIInfo *popup_menu_uiinfo;
	GtkWidget *popup_menu;

	guint total_annots;
	UnicodeMap *umap;

	gint current_page;

	GPdfControl *parent;
};

static void	gpdf_annots_view_class_init	   	     (GPdfAnnotsViewClass*); 
static void	gpdf_annots_view_dispose	   	     (GObject*); 
static void	gpdf_annots_view_finalize	   	     (GObject*); 
static void	gpdf_annots_view_instance_init     	     (GPdfAnnotsView*);
static void 	gpdf_annots_view_popup_menu_item_show_all_cb (GtkMenuItem*,
							      GPdfAnnotsView*); 
static void 	gpdf_annots_view_popup_menu_item_hide_all_cb (GtkMenuItem*,
							      GPdfAnnotsView*); 
static void 	gpdf_annots_view_popup_menu_item_filter_cb   (GtkMenuItem*,
							      GPdfAnnotsView*); 
static void 	gpdf_annots_view_popup_menu_item_plugins_cb  (GtkMenuItem*,
							      GPdfAnnotsView*); 

#define GPDF_IS_NON_NULL_ANNOTS_VIEW(obj) \
		(((obj) != NULL) && (GPDF_IS_ANNOTS_VIEW ((obj))))

enum {
    GPDF_ANNVIEW_COLUMN1 = 0, 	/* Pixbuf for annot type */
    GPDF_ANNVIEW_COLUMN2, 	/* Annot title */
    GPDF_ANNVIEW_COLUMN3, 	/* Boolean toggle (displayed) */
    GPDF_ANNVIEW_COLUMN4, 	/* Hidden column for Annot data */
    NUM_COLUMNS
};

enum {
	ANNOT_SELECTED_SIGNAL = 0,
	ANNOT_TOGGLED_SIGNAL,
	READY_SIGNAL,
	LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_VIEW,
	PROP_CONTROL
};

#define POPUP_MENU_SHOW_ALL_ITEM	"Show All"
#define POPUP_MENU_SHOW_ALL_ITEM_TIP	"Show All annotations availables of any kind"
#define POPUP_MENU_HIDE_ALL_ITEM	"Hide All"
#define POPUP_MENU_HIDE_ALL_ITEM_TIP	"Hide All annotations availables of any kind"
#define POPUP_MENU_FILTER_ITEM		"Filter"
#define POPUP_MENU_FILTER_ITEM_TIP	"Filter displayed annotations according to their types"
#define POPUP_MENU_PLUGINS_ITEM		"Annotation Plug-Ins"
#define POPUP_MENU_PLUGINS_ITEM_TIP	"Manage active Annotation Plug-Ins"

#define POPUP_MENU_SHOW_ALL_INDEX	0
#define POPUP_MENU_HIDE_ALL_INDEX	1
#define POPUP_MENU_FILTER_INDEX		2
#define POPUP_MENU_PLUGINS_INDEX	3

static GnomeUIInfo tools_popup_menu_items_init[] =
{
	GNOMEUIINFO_ITEM_NONE (N_ (POPUP_MENU_SHOW_ALL_ITEM),
			       N_ (POPUP_MENU_SHOW_ALL_ITEM_TIP),
			       gpdf_annots_view_popup_menu_item_show_all_cb), 
	GNOMEUIINFO_ITEM_NONE (N_ (POPUP_MENU_HIDE_ALL_ITEM),
			       N_ (POPUP_MENU_HIDE_ALL_ITEM_TIP),
			       gpdf_annots_view_popup_menu_item_hide_all_cb), 
	GNOMEUIINFO_ITEM_NONE (N_ (POPUP_MENU_FILTER_ITEM),
			       N_ (POPUP_MENU_FILTER_ITEM_TIP),
			       gpdf_annots_view_popup_menu_item_filter_cb), 
	GNOMEUIINFO_ITEM_NONE (N_ (POPUP_MENU_PLUGINS_ITEM),
			       N_ (POPUP_MENU_PLUGINS_ITEM_TIP),
			       gpdf_annots_view_popup_menu_item_plugins_cb), 
	GNOMEUIINFO_END
};

static guint gpdf_annots_view_signals [LAST_SIGNAL];

static GPdfAnnotationType
gpdf_annots_view_get_annot_subtype (Annot *annot)
{
        GPdfAnnotationType subtype_num = (GPdfAnnotationType)0; 
        Object subtype;
        gint i; 
 
        annot->getSubtype (&subtype);
        for (i = 0; i < NUM_ANNOT_TYPES; i++) {
                if (gpdf_annotation_types_str[i] &&
                    subtype.isName ((gchar *)gpdf_annotation_types_str[i])) {
                        subtype_num = (GPdfAnnotationType)i;
                        break; 
                }
        }

        return subtype_num; 
}

GPDF_CLASS_BOILERPLATE(GPdfAnnotsView, gpdf_annots_view, GtkScrolledWindow, GTK_TYPE_SCROLLED_WINDOW); 

static void
gpdf_annots_view_set_property (GObject *object, guint param_id,
				  const GValue *value, GParamSpec *pspec)
{
	GPdfAnnotsView *annots_view;

	g_return_if_fail (GPDF_IS_ANNOTS_VIEW (object));

	annots_view = GPDF_ANNOTS_VIEW (object);

	switch (param_id) {
	case PROP_VIEW:
		annots_view->priv->gpdf_view = 
		  GPDF_VIEW (g_value_get_object (value));
		break;
	case PROP_CONTROL:
		annots_view->priv->parent = 
		  GPDF_CONTROL (g_value_get_object (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
gpdf_annots_view_class_init (GPdfAnnotsViewClass *klass)
{
	GObjectClass *gobject_class;
	
	gobject_class = G_OBJECT_CLASS (klass);

	parent_class = GTK_SCROLLED_WINDOW_CLASS (g_type_class_peek_parent (klass));

	gobject_class->dispose = gpdf_annots_view_dispose;
	gobject_class->finalize = gpdf_annots_view_finalize;
	gobject_class->set_property = gpdf_annots_view_set_property;

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
			_("Parent control"),
			_("Parent control"),
			GPDF_TYPE_CONTROL, 
		        (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE)));

	gpdf_annots_view_signals [ANNOT_SELECTED_SIGNAL] =
	  g_signal_new (
	    "annotation_selected",
	    G_TYPE_FROM_CLASS (gobject_class),
	    G_SIGNAL_RUN_LAST,
	    G_STRUCT_OFFSET (GPdfAnnotsViewClass, annot_selected),
	    NULL, NULL,
	    gpdf_marshal_VOID__POINTER_INT,
	    G_TYPE_NONE, 2,
            G_TYPE_POINTER,
            G_TYPE_INT);

	gpdf_annots_view_signals [ANNOT_TOGGLED_SIGNAL] =
	  g_signal_new (
	    "annotation_toggled",
	    G_TYPE_FROM_CLASS (gobject_class),
	    G_SIGNAL_RUN_LAST,
	    G_STRUCT_OFFSET (GPdfAnnotsViewClass, annot_toggled),
	    NULL, NULL,
	    gpdf_marshal_VOID__POINTER_INT_BOOLEAN,
	    G_TYPE_NONE, 3,
            G_TYPE_POINTER,
            G_TYPE_INT,
	    G_TYPE_BOOLEAN);

	gpdf_annots_view_signals [READY_SIGNAL] =
	  g_signal_new (
	    "ready",
	    G_TYPE_FROM_CLASS (gobject_class),
	    G_SIGNAL_RUN_LAST,
	    G_STRUCT_OFFSET (GPdfAnnotsViewClass, ready),
	    NULL, NULL,
	    gpdf_marshal_VOID__VOID,
	    G_TYPE_NONE, 0);
}

static void
gpdf_annots_view_emit_annot_selected (GPdfAnnotsView *annots_view,
                                      Annot          *annot,
                                      gint            page_nr)
{
	g_signal_emit (G_OBJECT (annots_view),
		       gpdf_annots_view_signals [ANNOT_SELECTED_SIGNAL],
		       0, annot, page_nr);
}

static void
gpdf_annots_view_emit_annot_toggled (GPdfAnnotsView *annots_view,
				     Annot          *annot,
				     gint            page_nr,
				     gboolean	     active)
{
	g_signal_emit (G_OBJECT (annots_view),
		       gpdf_annots_view_signals [ANNOT_TOGGLED_SIGNAL],
		       0, annot, page_nr, active);
}

static void
gpdf_annots_view_emit_ready (GPdfAnnotsView *annots_view)
{
	g_signal_emit (G_OBJECT (annots_view),
		       gpdf_annots_view_signals [READY_SIGNAL], 0);
}

#define GPDF_RENDER_STOCK_ICON(widget, dest, id, size) 					\
{											\
	GtkIconSet *icon_set;								\
	icon_set = gtk_style_lookup_icon_set (gtk_widget_get_style (widget),		\
					      gpdf_annotation_icon_ids[id]);		\
	if (icon_set != NULL)								\
		 dest = gtk_icon_set_render_icon (icon_set,				\
						  gtk_widget_get_style (widget),	\
						  gtk_widget_get_direction (widget),	\
						  GTK_STATE_NORMAL,			\
						  size,					\
						  NULL, NULL);				\
}

static void
gpdf_annots_view_construct (GPdfAnnotsView *annots_view)
{
	GPdfAnnotsViewPrivate *priv;
	GtkWidget *annwidget;
        gint i; 

	g_return_if_fail (GPDF_IS_NON_NULL_ANNOTS_VIEW (annots_view)); 

	priv = annots_view->priv;
	
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (annots_view),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (annots_view),
					     GTK_SHADOW_NONE);

	priv->listview = gtk_tree_view_new ();
	gtk_widget_show (priv->listview);
	gtk_container_add (GTK_CONTAINER (annots_view), priv->listview);
	
	annwidget = GTK_WIDGET (annots_view);

	/* Init annots icons */
        for (i = 0; i < NUM_ANNOT_TYPES; i++)
          GPDF_RENDER_STOCK_ICON(annwidget,
                                 priv->stock_annots_pixbuf[i], 
                                 i,
                                 GTK_ICON_SIZE_SMALL_TOOLBAR); 
}

gboolean
gpdf_annots_view_update_annvisual (GtkTreeModel *model, 
                                   GtkTreePath *path, 
                                   GtkTreeIter *iter,
                                   gpointer data)
{
	GPdfAnnotsView *annots_view;
	GPdfAnnotsViewPrivate *priv;
        GPdfAnnotData *annotdata_ptr; 
	GValue annot_data = {0, };
	gboolean is_oncurrentpage; 
	
	annots_view = GPDF_ANNOTS_VIEW (data);

	g_return_val_if_fail (GPDF_IS_NON_NULL_ANNOTS_VIEW (annots_view),
			      FALSE);

	priv = annots_view->priv; 
	gtk_tree_model_get_value (model,
				  iter,
				  GPDF_ANNVIEW_COLUMN4,
				  &annot_data);
        annotdata_ptr =
          (GPdfAnnotData *)g_value_get_pointer ((const GValue *)&annot_data);

        g_return_val_if_fail (annotdata_ptr, FALSE); 

	is_oncurrentpage = (annotdata_ptr->page_nr == priv->current_page);
        /*gtk_widget_set_sensitive (GTK_WIDGET (annotdata_ptr->toggle_renderer),
          is_oncurrentpage); */ 
	if (is_oncurrentpage)
            gtk_tree_view_scroll_to_cell (
		GTK_TREE_VIEW (priv->listview),
		path,
		gtk_tree_view_get_column(
			GTK_TREE_VIEW (priv->listview),
			GPDF_ANNVIEW_COLUMN2),
		TRUE, 0.0, 0.0); 
	
	return FALSE; 
}

static void
gpdf_annots_view_page_changed_cb (GPdfView *gpdf_view,
                                  int page,
                                  gpointer user_data)
{
	GPdfAnnotsView *annots_view;
	GtkTreeModel *model;

	annots_view = GPDF_ANNOTS_VIEW (user_data);

	g_return_if_fail (GPDF_IS_NON_NULL_ANNOTS_VIEW (annots_view));
	g_return_if_fail (GPDF_IS_VIEW (gpdf_view));

	if (!annots_view->priv->model) return;
	model = GTK_TREE_MODEL (annots_view->priv->model);

	annots_view->priv->current_page = page; 
	gtk_tree_model_foreach (model,
				gpdf_annots_view_update_annvisual,
				annots_view); 
}

GtkWidget *
gpdf_annots_view_new (GPdfControl *parent, GtkWidget *gpdf_view)
{
	GPdfAnnotsView *annots_view;

	annots_view =
	  GPDF_ANNOTS_VIEW (g_object_new (
		  GPDF_TYPE_ANNOTS_VIEW, 
		  "view", GPDF_VIEW (gpdf_view),
		  "control", parent, 
                  NULL));

	g_object_ref (G_OBJECT (annots_view->priv->gpdf_view)); 
	g_object_ref (G_OBJECT (annots_view->priv->parent)); 
	gpdf_annots_view_construct (annots_view);
	
	g_signal_connect (G_OBJECT (gpdf_view), "page_changed",
			  G_CALLBACK (gpdf_annots_view_page_changed_cb),
			  annots_view);

	return GTK_WIDGET (annots_view);
}


static void
gpdf_annots_view_dispose (GObject *object)
{
	GPdfAnnotsView *annots_view;
	GPdfAnnotsViewPrivate *priv;
    
	g_return_if_fail (GPDF_IS_NON_NULL_ANNOTS_VIEW (object));
    
	annots_view = GPDF_ANNOTS_VIEW (object);
	priv = annots_view->priv;

	if (priv->model) {
		gtk_list_store_clear (priv->model); 
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
	if (priv->popup_menu) {
		gtk_widget_destroy (priv->popup_menu);
		priv->popup_menu = NULL;
		g_free (priv->popup_menu_uiinfo);
	}
	if (priv->parent) {
		g_object_unref (priv->parent);
		priv->parent = NULL; 
	}

	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
gpdf_annots_view_finalize (GObject *object)
{
	GPdfAnnotsView *annots_view;
	GPdfAnnotsViewPrivate *priv;
    
	g_return_if_fail (GPDF_IS_NON_NULL_ANNOTS_VIEW (object));
    
	annots_view = GPDF_ANNOTS_VIEW (object);
	priv = annots_view->priv;
    
	annots_view->priv->parent = NULL;

	if (annots_view->priv) {
		g_free (annots_view->priv);
		annots_view->priv = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gpdf_annots_view_instance_init (GPdfAnnotsView *annots_view)
{
	annots_view->priv = g_new0 (GPdfAnnotsViewPrivate, 1);
	annots_view->priv->popup_menu_uiinfo =
	  (GnomeUIInfo*)g_memdup (tools_popup_menu_items_init,
				  sizeof (tools_popup_menu_items_init));
}

static gchar *
unicode_to_char (Object *aString, 
		 UnicodeMap *uMap)
{
	GString gstr;
	gchar buf[8]; /* 8 is enough for mapping an unicode char to a string */
	int i, n;
	
	for (i = 0; i < aString->getString()->getLength(); ++i) {
		n = uMap->mapUnicode(aString->getString()->getChar(i),
                                     buf, sizeof(buf));
		gstr.append(buf, n);
	}

	return g_strdup (gstr.getCString ());
}

static gboolean
gpdf_annots_view_annot_select_func (GtkTreeSelection *selection,
                                    GtkTreeModel     *model,
                                    GtkTreePath	     *path,
                                    gboolean	      path_currently_selected,
                                    gpointer	      data)
{
	GPdfAnnotsView *annots_view;
	GPdfAnnotsViewPrivate *priv;
        GPdfAnnotData *annot_data; 
	GValue selection_item = {0,};
	GtkTreeIter iterator;
	gboolean ret = FALSE;

	annots_view = GPDF_ANNOTS_VIEW (data);
	
	g_return_val_if_fail (GPDF_IS_NON_NULL_ANNOTS_VIEW (annots_view),
			      FALSE);

	do {
		priv = annots_view->priv;

		gtk_tree_model_get_iter (GTK_TREE_MODEL (priv->model),
					 &iterator,
					 path);

		/* handle only if is about to be selected */
		if (!gtk_tree_selection_get_selected (selection, 
						      &model,
						      &iterator)) {
			gtk_tree_model_get_value (GTK_TREE_MODEL (priv->model),
						  &iterator,
						  GPDF_ANNVIEW_COLUMN4,
						  &selection_item);
			annot_data =
                          (GPdfAnnotData *)g_value_peek_pointer (
                                  (const GValue*) &selection_item);
			
			gpdf_annots_view_emit_annot_selected (annots_view,
                                                              annot_data->annot, 
                                                              annot_data->page_nr);
		}
		
		ret = TRUE;
	}
	while (0); 

	return ret;
}

static void
gpdf_annots_view_toggled_callback (GtkCellRendererToggle *celltoggle,
				   gchar                 *path_string,
				   GPdfAnnotsView        *annots_view)
{
	GPdfAnnotsViewPrivate *priv;
        GPdfAnnotData *annot_data;
	GtkTreePath *path;
	GtkTreeIter iter;
	gboolean active = FALSE;
	
	g_return_if_fail (GPDF_IS_NON_NULL_ANNOTS_VIEW (annots_view));

	priv = annots_view->priv;

	path = gtk_tree_path_new_from_string (path_string);
	if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (priv->model),
				      &iter, path))
		g_warning ("%s: bad path", G_STRLOC);
	else
	{
		gtk_tree_model_get (GTK_TREE_MODEL (priv->model), &iter,
				    GPDF_ANNVIEW_COLUMN3, &active,
				    GPDF_ANNVIEW_COLUMN4, &annot_data,
				    -1);
		active = !active;
		gtk_list_store_set (priv->model, &iter,
				    GPDF_ANNVIEW_COLUMN3, active,
				    -1);
		gpdf_annots_view_emit_annot_toggled (annots_view,
						     annot_data->annot,
						     annot_data->page_nr,
						     active);
		
	}
	gtk_tree_path_free (path);
	
}

gboolean
gpdf_annots_view_is_annot_displayed (Annot *annot, gpointer user_data)
{
	GPdfAnnotsView *annots_view = GPDF_ANNOTS_VIEW (user_data);
	GPdfAnnotsViewPrivate *priv;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean active = FALSE;
        GPdfAnnotData *annot_data;
	gboolean found = FALSE;

	g_return_val_if_fail (GPDF_IS_NON_NULL_ANNOTS_VIEW (annots_view),
			      gTrue);

	priv = annots_view->priv;

	if (!GTK_IS_TREE_MODEL (priv->model))
	  return gTrue;

	model = GTK_TREE_MODEL (priv->model);
	
	if (gtk_tree_model_get_iter_first (model, &iter))
	{
		do
		{
			gtk_tree_model_get (model, &iter,
					    GPDF_ANNVIEW_COLUMN3, &active,
					    GPDF_ANNVIEW_COLUMN4, &annot_data,
					    -1);
			if (annot_data->refnum == annot->getRefNum () &&
			    annot_data->refgen == annot->getRefGen ()) {
				found = TRUE;
				break;
			}
		}
		while (gtk_tree_model_iter_next (model, &iter));

		/* Just to be warned */
		g_assert (found);
	}

//	g_message ("gpdf_annots_view_is_annot_displayed: display annotation = %s",
//		   ((found && active) || !found) ? "Yes" : "No");
	
	return (((found && active) || !found) ? gTrue : gFalse);
}

static gboolean
gpdf_annots_view_have_annotations (GPdfAnnotsView *annots_view)
{
	GPdfAnnotsViewPrivate *priv;
	
	g_return_val_if_fail
	  (GPDF_IS_NON_NULL_ANNOTS_VIEW (annots_view),
	   FALSE);
	
	priv = annots_view->priv;
	
	return (priv->total_annots > 0);
}

static void
gpdf_annots_view_update_annots_list (GPdfAnnotsView *annots_view,
                                     GtkTreeIter    *iter, 
                                     Annots         *annots,
                                     int page_nr, int cur_page)
{
	GPdfAnnotsViewPrivate *priv;
        GPdfAnnotData *annot_data; 
        int num_annots, annot_nr = 0;
        Annot *annot;
        Object subtype;
        Dict *dict;
        Object objT, objName, objOpen, objDest, objContents;
        gboolean is_opened = TRUE;
        char *item = ""; 
	
	g_return_if_fail (GPDF_IS_NON_NULL_ANNOTS_VIEW (annots_view));
	
	priv = annots_view->priv;

        num_annots = annots->getNumAnnots ();

        //g_message ("Page #%d have %d annots", page_nr, num_annots);

        if (!num_annots) return;
	      
	priv->total_annots += num_annots;
        for (annot_nr = 0, annot = annots->getAnnot (0);
             annot_nr < num_annots;
             annot = annots->getAnnot (++annot_nr)) {

                dict = annot->getDict();

		/* Allocate annotation assiociated data */
                annot_data = g_new0 (GPdfAnnotData, 1);
                annot_data->page_nr = page_nr;
                annot_data->annot = annot;
                annot_data->type = gpdf_annots_view_get_annot_subtype (annot);
		annot_data->refnum = annot->getRefNum ();
		annot_data->refgen = annot->getRefGen ();

                //g_message ("Processing annot #%d of type '%s' in page #%d",
		//	   annot_nr, 
		//	   gpdf_annotation_types_str[annot_data->type], 
		//	   page_nr);

		/* FIXME TODO:
		 *   As suggested in PDF reference, we should handle
		 *   different kind of annotations subtypes with loadable
		 *   modules. Each module could contains an hash with
		 *   annot fields names/field types.
		 *   One or two modules could be added immediatly for
		 *   handling Widget & Stamp annotations. Others may be
		 *   added later ...
		 */
                switch (annot_data->type) {
                      case GPDF_ANNOT_TYPE_TEXT:
                        dict->lookup ("T", &objName);
                        item = unicode_to_char (&objName, priv->umap); 
                        dict->lookup ("Open", &objOpen);
                        if (!objOpen.isNull ())
                          is_opened = objOpen.getBool ();
                        break; 
                      case GPDF_ANNOT_TYPE_LINK:
                        dict->lookup ("Dest", &objDest);
                        if (!objDest.isNull ())
                          item = unicode_to_char (&objDest, priv->umap); 
			is_opened = TRUE;
                        break; 
                      case GPDF_ANNOT_TYPE_FREETEXT: 
                        dict->lookup ("T", &objT);
                        if (!objT.isNull ())
                          item = unicode_to_char (&objT, priv->umap); 
			is_opened = TRUE;
                        break; 
                      case GPDF_ANNOT_TYPE_LINE: 
                      case GPDF_ANNOT_TYPE_SQUARE: 
                      case GPDF_ANNOT_TYPE_CIRCLE: 
                      case GPDF_ANNOT_TYPE_HIGHLIGHT: 
                      case GPDF_ANNOT_TYPE_UNDERLINE: 
                      case GPDF_ANNOT_TYPE_STRIKEOUT: 
                      case GPDF_ANNOT_TYPE_STAMP: 
                      case GPDF_ANNOT_TYPE_INK:
                        dict->lookup ("Contents", &objContents);
                        if (!objContents.isNull ())
                          item = unicode_to_char (&objContents, priv->umap); 
			is_opened = TRUE;
                        break; 
                      case GPDF_ANNOT_TYPE_POPUP: 
                      case GPDF_ANNOT_TYPE_FILEATTACHMENT: 
                      case GPDF_ANNOT_TYPE_SOUND: 
                      case GPDF_ANNOT_TYPE_MOVIE: 
                      case GPDF_ANNOT_TYPE_WIDGET: 
                      case GPDF_ANNOT_TYPE_TRAPNET:
                      case GPDF_ANNOT_TYPE_UNKNOWN:
                      default:
			is_opened = TRUE;
                        break; 
                }

		/* Add an item to the list model */
                gtk_list_store_append (GTK_LIST_STORE (priv->model), iter);
		gtk_list_store_set (GTK_LIST_STORE (priv->model), iter,
				    GPDF_ANNVIEW_COLUMN1, priv->stock_annots_pixbuf[annot_data->type], 
				    GPDF_ANNVIEW_COLUMN2, item, 
				    GPDF_ANNVIEW_COLUMN3, is_opened,
				    GPDF_ANNVIEW_COLUMN4, (gpointer)annot_data, 
				    -1);
        }
}

static gboolean
gpdf_annots_view_populate_idle (GPdfAnnotsView *annots_view)
{
	GPdfAnnotsViewPrivate *priv;
	GString *enc; 
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;
        GtkTreeIter iter; 
        gint num_pages;
        gint page;

	g_return_val_if_fail (GPDF_IS_NON_NULL_ANNOTS_VIEW (annots_view),
			      TRUE);
	
	priv = annots_view->priv;

        /* Allocate list model */
	if (priv->model) {
		gtk_list_store_clear(priv->model);
		g_object_unref (priv->model);
		priv->model = NULL;
	}
	
	priv->model = gtk_list_store_new (NUM_COLUMNS,
					  GDK_TYPE_PIXBUF,
					  G_TYPE_STRING,
					  G_TYPE_BOOLEAN,
					  G_TYPE_POINTER);        

	/*
	 * FIXME:
	 *   There might be a way to get the used for
	 *   annotations.
	 */
	if (!priv->umap) {
		enc = new GString("UTF-8");
		priv->umap = globalParams->getUnicodeMap(enc);
		priv->umap->incRefCnt (); 
		delete enc;
	}
        
        /* Navigate through all pages to get annotations */
	num_pages = priv->pdf_doc->getNumPages ();
        for (page = 1; page < num_pages; page++) {
		Page *the_page;
		Object annots_obj;
		Annots *annots;
		XRef *xref;

		the_page = priv->pdf_doc->getCatalog ()->getPage (page);
                xref = priv->pdf_doc->getXRef();
		annots = new Annots (xref, the_page->getAnnots (&annots_obj));
		
                priv->current_page = gpdf_view_get_current_page (priv->gpdf_view);
		
                gpdf_annots_view_update_annots_list (annots_view,
                                                     &iter, 
                                                     annots,
                                                     page,
                                                     priv->current_page);
		delete annots;
        }
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->listview),
				 GTK_TREE_MODEL (priv->model));

	if (gpdf_annots_view_have_annotations (annots_view)) {
	
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->listview));
		gtk_tree_selection_set_select_function (selection,
							gpdf_annots_view_annot_select_func,
							annots_view, NULL);
		
		gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (priv->listview), TRUE);
		
		renderer = gtk_cell_renderer_pixbuf_new ();
		column = gtk_tree_view_column_new_with_attributes (NULL, renderer,
								   "pixbuf", GPDF_ANNVIEW_COLUMN1, 
								   NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (priv->listview), column);
		
		renderer = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Owner/Title"), renderer,
								   "text", GPDF_ANNVIEW_COLUMN2,
								   NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (priv->listview), column);
		
		renderer = gtk_cell_renderer_toggle_new ();
		g_signal_connect (renderer, "toggled",
				  G_CALLBACK (gpdf_annots_view_toggled_callback),
				  (gpointer)annots_view);
		column = gtk_tree_view_column_new_with_attributes (_("Opened"), renderer,
								   "active", GPDF_ANNVIEW_COLUMN3,
								   NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (priv->listview), column);
		
		/* Activate popup menu items */
		if (GTK_IS_WIDGET (priv->popup_menu_uiinfo[POPUP_MENU_SHOW_ALL_INDEX].widget))
		  gtk_widget_set_sensitive (priv->popup_menu_uiinfo[POPUP_MENU_SHOW_ALL_INDEX].widget,
					    TRUE); 
		
		if (GTK_IS_WIDGET (priv->popup_menu_uiinfo[POPUP_MENU_HIDE_ALL_INDEX].widget))
		  gtk_widget_set_sensitive (priv->popup_menu_uiinfo[POPUP_MENU_HIDE_ALL_INDEX].widget, 
					    TRUE); 
		
		if (GTK_IS_WIDGET (priv->popup_menu_uiinfo[POPUP_MENU_FILTER_INDEX].widget))
		  gtk_widget_set_sensitive (priv->popup_menu_uiinfo[POPUP_MENU_FILTER_INDEX].widget, 
					    FALSE); 
		
		if (GTK_IS_WIDGET (priv->popup_menu_uiinfo[POPUP_MENU_PLUGINS_INDEX].widget))
		  gtk_widget_set_sensitive (priv->popup_menu_uiinfo[POPUP_MENU_PLUGINS_INDEX].widget, 
					    FALSE); 
	}

	gpdf_annots_view_emit_ready (annots_view);

        return FALSE; 
}

void
gpdf_annots_view_set_pdf_doc (GPdfAnnotsView *annots_view,
                              PDFDoc *pdf_doc)
{
    GPdfAnnotsViewPrivate *priv;

    g_return_if_fail (GPDF_IS_NON_NULL_ANNOTS_VIEW (annots_view));

    priv = annots_view->priv;

    if (pdf_doc != priv->pdf_doc)
    {
	    priv->pdf_doc = pdf_doc;

            /*
             * Annotations are attached to pages, so need to
             * scan all pages. Can be time comsuming so run
              * this as idle func.
             */ 
	    g_idle_add ((GSourceFunc)gpdf_annots_view_populate_idle,
			annots_view);
    }
}

static void
gpdf_annots_view_popup_menu_item_show_all_cb (GtkMenuItem *item,
					      GPdfAnnotsView *annots_view)
{
	GPdfAnnotsViewPrivate *priv;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GPdfAnnotData *annot_data;
	gboolean need_update = FALSE;
	
	g_return_if_fail (GPDF_IS_NON_NULL_ANNOTS_VIEW (annots_view));
	
	priv = annots_view->priv;
	
	if (!annots_view->priv->model) return;
	model = GTK_TREE_MODEL (annots_view->priv->model);

	if (gtk_tree_model_get_iter_first (model, &iter))
	{
		do
		{
			gtk_tree_model_get (model, &iter,
					    GPDF_ANNVIEW_COLUMN4, &annot_data,
					    -1);
			if (annot_data->page_nr == priv->current_page)
			  need_update = TRUE;
			gtk_list_store_set (GTK_LIST_STORE (model), &iter,
					    GPDF_ANNVIEW_COLUMN3, TRUE,
					    -1);
		}
		while (gtk_tree_model_iter_next (model, &iter));
	}

	/* We emit only one signal if current page have annots */
	if (need_update)
	  gpdf_annots_view_emit_annot_toggled (annots_view,
					       NULL,
					       priv->current_page,
					       TRUE);
}

static void
gpdf_annots_view_popup_menu_item_hide_all_cb (GtkMenuItem *item,
					      GPdfAnnotsView *annots_view)
{
	GPdfAnnotsViewPrivate *priv;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GPdfAnnotData *annot_data;
	gboolean need_update = FALSE;
	
	g_return_if_fail (GPDF_IS_NON_NULL_ANNOTS_VIEW (annots_view));
	
	priv = annots_view->priv;
	
	if (!annots_view->priv->model) return;
	model = GTK_TREE_MODEL (annots_view->priv->model);

	if (gtk_tree_model_get_iter_first (model, &iter))
	{
		do
		{
			gtk_tree_model_get (GTK_TREE_MODEL (priv->model), &iter,
					    GPDF_ANNVIEW_COLUMN4, &annot_data,
					    -1);
			if (annot_data->page_nr == priv->current_page)
			  need_update = TRUE;
			gtk_list_store_set (GTK_LIST_STORE (model), &iter,
					    GPDF_ANNVIEW_COLUMN3, FALSE,
					    -1);
		}
		while (gtk_tree_model_iter_next (model, &iter));
	}

	/* We emit only one signal if current page have annots */
	if (need_update)
	  gpdf_annots_view_emit_annot_toggled (annots_view,
					       NULL,
					       priv->current_page,
					       FALSE);
}

static void
gpdf_annots_view_popup_menu_item_filter_cb (GtkMenuItem *item,
					    GPdfAnnotsView *annots_view)
{
	GPdfAnnotsViewPrivate *priv;
	
	g_return_if_fail (GPDF_IS_NON_NULL_ANNOTS_VIEW (annots_view));
	
	priv = annots_view->priv;
	
	gpdf_control_private_warn_dialog (priv->parent, 
					  _("Not yet implemented!"),
					  _("Annotations Filter feature not yet implemented... Sorry."),
					  TRUE);					  
}

static void
gpdf_annots_view_popup_menu_item_plugins_cb (GtkMenuItem *item,
					     GPdfAnnotsView *annots_view)
{
	GPdfAnnotsViewPrivate *priv;
	
	g_return_if_fail (GPDF_IS_NON_NULL_ANNOTS_VIEW (annots_view));
	
	priv = annots_view->priv;
	
	gpdf_control_private_warn_dialog (priv->parent, 
					  _("Not yet implemented!"),
					  _("Annotations Plug-ins management feature not yet implemented... Sorry."),
					  TRUE);					  
}

GtkWidget*
gpdf_annots_view_get_tools_menu (GPdfAnnotsView *annots_view)
{
	GPdfAnnotsViewPrivate *priv;
	GtkWidget *item; 
	
	g_return_val_if_fail (GPDF_IS_NON_NULL_ANNOTS_VIEW (annots_view), NULL);
	
	priv = annots_view->priv;

	if (!gpdf_annots_view_have_annotations (annots_view))
	  return NULL;
	
	if (!priv->popup_menu) {
		priv->popup_menu = gtk_menu_new ();
		gnome_app_fill_menu_with_data (GTK_MENU_SHELL (priv->popup_menu),
					       priv->popup_menu_uiinfo,
					       NULL, FALSE, 0,
					       (gpointer)annots_view);

		item = priv->popup_menu_uiinfo[POPUP_MENU_SHOW_ALL_INDEX].widget;
		gtk_widget_set_sensitive (item, TRUE); 
		item = priv->popup_menu_uiinfo[POPUP_MENU_HIDE_ALL_INDEX].widget;
		gtk_widget_set_sensitive (item, TRUE); 
		item = priv->popup_menu_uiinfo[POPUP_MENU_FILTER_INDEX].widget;
		gtk_widget_set_sensitive (item, FALSE); 
		item = priv->popup_menu_uiinfo[POPUP_MENU_PLUGINS_INDEX].widget;
		gtk_widget_set_sensitive (item, FALSE); 
	}
	
	return priv->popup_menu; 
}

END_EXTERN_C
