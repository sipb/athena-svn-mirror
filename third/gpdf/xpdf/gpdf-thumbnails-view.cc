/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-indent-level: 8; c-basic-offset: 8;  -*- */
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
#  include <glib.h>
#  include <glib/ghash.h>
#  include <gdk-pixbuf/gdk-pixbuf.h>
#  include <gtk/gtk.h>
#  include <libgnome/gnome-i18n.h>
#  include <libgnomeui/gnome-app-helper.h>
#  include <libgnomecanvas/libgnomecanvas.h>
#  include "gpdf-stock-icons.h"
#  include "gpdf-util.h"
#  include "gpdf-marshal.h"
#include <gpdf-g-switch.h>

#include <aconf.h>
#include "goo/GList.h"
#include "goo/GString.h"
#include "Object.h"
#include "Catalog.h"
#include "Dict.h"
#include "Stream.h"
#include "Page.h"
#include "UnicodeMap.h"
#include "GlobalParams.h"
#include "GfxState.h"
#include "Thumb.h"
#include "gpdf-view.h"
#include "gpdf-control-private.h"

BEGIN_EXTERN_C

#include "gpdf-thumbnails-view.h"
#include "gpdf-control-private.h"

/* FIXME: Put this values in preferences ? */
#define GPDF_THUMBNAILS_PER_ROW		2
#define GPDF_THUMBNAILS_ENLIGTH_FACTOR	35.0
#define GPDF_DEFAULT_THUMB_WIDTH	76

typedef struct _GPdfThumbnail {
	GnomeCanvasItem *group;
	GdkPixbuf *pixbuf;
	GnomeCanvasItem *item;
	GnomeCanvasItem *outerborder_sel;
	GnomeCanvasItem *shadow;
	GnomeCanvasItem *text;
	GnomeCanvasItem *outline;
	gboolean generated; 
} GPdfThumbnail;

struct _GPdfThumbnailsViewPrivate {

	GPdfView *gpdf_view; 
	PDFDoc *pdf_doc;

	GtkWidget *canvas_widget; 
	GnomeCanvas *canvas;
	GnomeCanvasGroup *root;

	guint page_per_row;
	guint num_pages;
	guint n_pixbufs;

	guint max_page_width;
	guint max_page_height;

	GPdfThumbnail *thumbs;

	GnomeUIInfo *popup_menu_uiinfo;
	GtkWidget *popup_menu;
	
	gint current_page;
	double zoom;
	guint current_gen_thumb;
	guint idle_id; 
	gulong idle_dcon_id; 

	gint clicked_x, clicked_y;

	gboolean thumbs_initialized;
	gboolean ignore_page_changed;
	gboolean ignore_zoom_changed;

	GPdfControl *parent;
};

typedef enum _GPdfThumbnailsDragAction {
	GPDF_DRAG_ACTION_NONE = 0,
	GPDF_DRAG_ACTION_RESIZE_TL,
	GPDF_DRAG_ACTION_RESIZE_BR,
	GPDF_DRAG_ACTION_MOVE	
} GPdfThumbnailsDragAction; 

static void	gpdf_thumbnails_view_class_init	      		     (GPdfThumbnailsViewClass*); 
static void	gpdf_thumbnails_view_dispose	      		     (GObject*); 
static void	gpdf_thumbnails_view_finalize	      		     (GObject*); 
static void	gpdf_thumbnails_view_instance_init    		     (GPdfThumbnailsView*);
static void	gpdf_thumbnails_view_get_page_canvas_regions         (GPdfThumbnailsView *thumbnails_view,
							              double *px1, double *py1,
							              double *px2, double *py2,
							              int *cx, int *cy,
							              double *vw, double *vh); 
#if CAN_GENERATE_THUMBNAILS
static void	gpdf_thumbnails_view_popup_menu_item_generate_cb     (GtkMenuItem*,
								      GPdfThumbnailsView*); 
static void	gpdf_thumbnails_view_popup_menu_item_generate_all_cb (GtkMenuItem*,
								      GPdfThumbnailsView*);
#endif

#define GPDF_IS_NON_NULL_THUMBNAILS_VIEW(obj) \
		(((obj) != NULL) && (GPDF_IS_THUMBNAILS_VIEW ((obj))))

enum {
	THUMBNAIL_SELECTED_SIGNAL = 0,
	READY_SIGNAL, 
	LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_VIEW,
	PROP_CONTROL
};

#define POPUP_MENU_GENERATE_ITEM		"Generate Thumbnail"
#define POPUP_MENU_GENERATE_ITEM_TIP		"Generate Thumbnail image for current page"
#define POPUP_MENU_GENERATE_ALL_ITEM		"Generate All Thumbnails"
#define POPUP_MENU_GENERATE_ALL_ITEM_TIP	"Generate Thumbnails images for every pages"

#define POPUP_MENU_GENERATE_INDEX		0
#define POPUP_MENU_GENERATE_ALL_INDEX		1

static GnomeUIInfo tools_popup_menu_items_init[] =
{
#if CAN_GENERATE_THUMBNAILS
	GNOMEUIINFO_ITEM_NONE (N_ (POPUP_MENU_GENERATE_ITEM),
			       N_ (POPUP_MENU_GENERATE_ITEM_TIP),
			       gpdf_thumbnails_view_popup_menu_item_generate_cb),
	GNOMEUIINFO_ITEM_NONE (N_ (POPUP_MENU_GENERATE_ALL_ITEM),
			       N_ (POPUP_MENU_GENERATE_ALL_ITEM_TIP),
			       gpdf_thumbnails_view_popup_menu_item_generate_all_cb),
#endif
	GNOMEUIINFO_END
};

static guint gpdf_thumbnails_view_signals [LAST_SIGNAL];

GPDF_CLASS_BOILERPLATE(GPdfThumbnailsView, gpdf_thumbnails_view, GtkScrolledWindow, GTK_TYPE_SCROLLED_WINDOW); 

static void
gpdf_thumbnails_view_set_property (GObject *object, guint param_id,
				  const GValue *value, GParamSpec *pspec)
{
	GPdfThumbnailsView *thumbnails_view;

	g_return_if_fail (GPDF_IS_THUMBNAILS_VIEW (object));

	thumbnails_view = GPDF_THUMBNAILS_VIEW (object);

	switch (param_id) {
	case PROP_VIEW:
		thumbnails_view->priv->gpdf_view = 
		  GPDF_VIEW (g_value_get_object (value));
		break;
	case PROP_CONTROL:
		thumbnails_view->priv->parent = 
		  GPDF_CONTROL (g_value_get_object (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
gpdf_thumbnails_view_class_init (GPdfThumbnailsViewClass *klass)
{
	GObjectClass *gobject_class;
	
	gobject_class = G_OBJECT_CLASS (klass);

	parent_class = GTK_SCROLLED_WINDOW_CLASS (g_type_class_peek_parent (klass));

	gobject_class->dispose = gpdf_thumbnails_view_dispose;
	gobject_class->finalize = gpdf_thumbnails_view_finalize;
	gobject_class->set_property = gpdf_thumbnails_view_set_property;

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

	gpdf_thumbnails_view_signals [THUMBNAIL_SELECTED_SIGNAL] =
	  g_signal_new (
	    "thumbnail_selected",
	    G_TYPE_FROM_CLASS (gobject_class),
	    G_SIGNAL_RUN_LAST,
	    G_STRUCT_OFFSET (GPdfThumbnailsViewClass, thumbnail_selected),
	    NULL, NULL,
	    gpdf_marshal_VOID__INT_INT_INT_INT_INT,
	    G_TYPE_NONE, 5,
	    G_TYPE_INT, G_TYPE_INT, /* x, y */
	    G_TYPE_INT, G_TYPE_INT, /* w, h */
	    G_TYPE_INT); /* page */

	gpdf_thumbnails_view_signals [READY_SIGNAL] =
	  g_signal_new (
	    "ready",
	    G_TYPE_FROM_CLASS (gobject_class),
	    G_SIGNAL_RUN_LAST,
	    G_STRUCT_OFFSET (GPdfThumbnailsViewClass, ready),
	    NULL, NULL,
	    gpdf_marshal_VOID__VOID,
	    G_TYPE_NONE, 0);
}

static void
gpdf_thumbnails_view_emit_thumbnail_selected (GPdfThumbnailsView *thumbnails_view,
					      guint x, guint y,
					      guint w, guint h,
					      guint page)
{
	g_signal_emit (G_OBJECT (thumbnails_view),
		       gpdf_thumbnails_view_signals [THUMBNAIL_SELECTED_SIGNAL],
		       0, x, y, w, h, page);
}

static void
gpdf_thumbnails_view_emit_ready (GPdfThumbnailsView *thumbnails_view)
{
	g_signal_emit (G_OBJECT (thumbnails_view),
		       gpdf_thumbnails_view_signals [READY_SIGNAL],
		       0);
}

static void
gpdf_thumbnails_view_construct (GPdfThumbnailsView *thumbnails_view)
{
	GPdfThumbnailsViewPrivate *priv;

	g_return_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view)); 

	priv = thumbnails_view->priv;

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (thumbnails_view),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (thumbnails_view),
					     GTK_SHADOW_IN);

	priv->canvas_widget = gnome_canvas_new_aa ();
	priv->canvas = GNOME_CANVAS (priv->canvas_widget); 
	gtk_widget_show (priv->canvas_widget);
	gtk_container_add (GTK_CONTAINER (thumbnails_view), priv->canvas_widget);
	
	priv->root = gnome_canvas_root (priv->canvas); 
}

#define GPDF_THUMBNAILS_MARGIN_LEFT		10
#define GPDF_THUMBNAILS_MARGIN_RIGHT		10
#define GPDF_THUMBNAILS_MARGIN_TOP		10
#define GPDF_THUMBNAILS_MARGIN_BOTTOM		10
#define GPDF_THUMBNAILS_INNER_MARGIN_LEFT	8
#define GPDF_THUMBNAILS_INNER_MARGIN_RIGHT	8
#define GPDF_THUMBNAILS_INNER_MARGIN_TOP	8
#define GPDF_THUMBNAILS_INNER_MARGIN_BOTTOM	8
#define GPDF_THUMBNAILS_SHADOW_WIDTH		2
#define GPDF_THUMBNAILS_SHADOW_HEIGHT		2
#define GPDF_THUMBNAILS_SHADOW_COLOR		0x303030ff
#define GPDF_THUMBNAILS_TEXT_HEIGHT		14
#define GPDF_THUMBNAILS_ROW_SEP			10
#define GPDF_THUMBNAILS_COL_SEP			10
#define GPDF_THUMBNAILS_FONT			"Sans 11"
#define GPDF_THUMBNAILS_SEL_FONT		"Sans Bold 11"
#define GPDF_THUMBNAIL_OUTLINE_CORNER_SIZE	3

static void
gpdf_thumbnails_view_page_box_coord(GPdfThumbnailsView *thumbnails_view,
				    guint page, 
				    double *x, double *y,
				    double *w, double *h)
{
	GPdfThumbnailsViewPrivate *priv;
	guint real_width, real_height; 
    
	g_return_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view));
	g_return_if_fail (x || y || w || h); 

	priv = thumbnails_view->priv;

	real_width = gdk_pixbuf_get_width (priv->thumbs[page -1].pixbuf); 
	real_height = gdk_pixbuf_get_height (priv->thumbs[page -1].pixbuf); 

	/* Update page size if guess was wrong */
	if (priv->max_page_width < real_width)
		priv->max_page_width = real_width; 
	if (priv->max_page_height < real_height)
		priv->max_page_height = real_height; 

	if (x) {
		*x = GPDF_THUMBNAILS_MARGIN_LEFT +
			((page -1) % priv->page_per_row) *
			(priv->max_page_width + GPDF_THUMBNAILS_COL_SEP +
			 GPDF_THUMBNAILS_INNER_MARGIN_RIGHT +
			 GPDF_THUMBNAILS_INNER_MARGIN_LEFT +
			 GPDF_THUMBNAILS_SHADOW_WIDTH);

		*x += (priv->max_page_width -real_width)/2;
	}

	if (y) {
		*y = GPDF_THUMBNAILS_MARGIN_TOP +
			(uint)((page -1) / priv->page_per_row) *
			(priv->max_page_height + GPDF_THUMBNAILS_ROW_SEP +
			 GPDF_THUMBNAILS_TEXT_HEIGHT +
			 GPDF_THUMBNAILS_INNER_MARGIN_BOTTOM +
			 GPDF_THUMBNAILS_INNER_MARGIN_TOP +
			 GPDF_THUMBNAILS_SHADOW_HEIGHT);

		*y += (priv->max_page_height -real_height)/2;
	}

	if (w) *w = real_width;
	if (h) *h = real_height; 
}

/*
 * Origin is always (0, 0)
 */
static void
gpdf_thumbnails_view_scroll_region_coord(GPdfThumbnailsView *thumbnails_view,
					 guint page, 
					 double *x, double *y)
{
	GPdfThumbnailsViewPrivate *priv;
	gint npage_h;
	
	g_return_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view));
	g_return_if_fail (x && y); 

	priv = thumbnails_view->priv;

	*x = GPDF_THUMBNAILS_MARGIN_LEFT + GPDF_THUMBNAILS_MARGIN_RIGHT +
		priv->page_per_row * (priv->max_page_width +
				      GPDF_THUMBNAILS_INNER_MARGIN_LEFT +
				      GPDF_THUMBNAILS_SHADOW_WIDTH +
				      GPDF_THUMBNAILS_INNER_MARGIN_RIGHT) +
		(priv->page_per_row -1) * GPDF_THUMBNAILS_COL_SEP;

	npage_h = (labs(page / priv->page_per_row) + (page % priv->page_per_row ? 1 : 0));
	*y = GPDF_THUMBNAILS_MARGIN_TOP + GPDF_THUMBNAILS_MARGIN_BOTTOM +
		npage_h * (priv->max_page_height +
			   GPDF_THUMBNAILS_INNER_MARGIN_TOP +
			   GPDF_THUMBNAILS_SHADOW_HEIGHT +
			   GPDF_THUMBNAILS_TEXT_HEIGHT +
			   GPDF_THUMBNAILS_INNER_MARGIN_BOTTOM) +
		(npage_h - 1) * GPDF_THUMBNAILS_ROW_SEP; 
}


static void
gpdf_thumbnails_view_set_scroll_region(GPdfThumbnailsView *thumbnails_view)
{
	GPdfThumbnailsViewPrivate *priv;
	guint page; 
	double x, y;
    
	g_return_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view));

	priv = thumbnails_view->priv;

	for (page = 1; page <= priv->current_gen_thumb; page++) {
		gpdf_thumbnails_view_page_box_coord (thumbnails_view,
						     page,
						     &x, &y,
						     NULL, NULL);
		gnome_canvas_item_set (priv->thumbs[page -1].group,
				       "x", x,
				       "y", y,
				       NULL); 
	}
	
	gpdf_thumbnails_view_scroll_region_coord(thumbnails_view,
						 priv->num_pages,
						 &x, &y);
	gnome_canvas_set_scroll_region(priv->canvas, 0, 0, x, y);
	gnome_canvas_set_center_scroll_region(priv->canvas, FALSE);
}


static void
gpdf_thumbnails_view_get_page_outline_coords (GPdfThumbnailsView *thumbnails_view,
					      guint page, 
					      double *x1, double *y1,
					      double *x2, double *y2)
{
	GPdfThumbnailsViewPrivate *priv;
	GPdfThumbnail *thumb;
	double px1, px2, py1, py2;
	double vw = 0.0, vh = 0.0;
	int cx, cy;
	double page_width, page_height;
	int width, height; 
	
	g_return_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view));
	
	priv = thumbnails_view->priv;

	/* Get a pointer on the current thumb */
	thumb = &priv->thumbs[page -1];

	/* Get page view canvas status */ 
	gpdf_thumbnails_view_get_page_canvas_regions (thumbnails_view,
						      &px1, &py1, &px2, &py2,
						      &cx, &cy, &vw, &vh);

	/* Get pixbuf size */
	width = gdk_pixbuf_get_width (thumb->pixbuf); 
	height = gdk_pixbuf_get_height (thumb->pixbuf);

	/* Compute page size in canvas units */
	page_width = (px2 -px1 +0.5) * priv->zoom;
	page_height = (py2 -py1 +0.5) * priv->zoom;

	/* Compute outline upper/left coords */
	*x1 = ((cx * width) / page_width); 
	*y1 = ((cy * height) / page_height);
	
	/* Compute outline lower/right coords */
	*x2 = *x1 + ((vw * width) / page_width); 
	*y2 = *y1 + ((vh * height) / page_height);

	*x1 += GPDF_THUMBNAILS_INNER_MARGIN_LEFT;
	*y1 += GPDF_THUMBNAILS_INNER_MARGIN_TOP; 

	*x2 = MIN (*x2, width) + GPDF_THUMBNAILS_INNER_MARGIN_LEFT;
	//*x2 = ((*x2 > width) ? width : *x2) + GPDF_THUMBNAILS_INNER_MARGIN_LEFT;
	*y2 = MIN (*y2, height) + GPDF_THUMBNAILS_INNER_MARGIN_TOP; 
	//*y2 = ((*y2 > height) ? height : *y2) + GPDF_THUMBNAILS_INNER_MARGIN_TOP; 
}

static void
gpdf_thumbnails_view_update_page_outline (GPdfThumbnailsView *thumbnails_view,
					  guint page)
{
	GPdfThumbnailsViewPrivate *priv;
	GPdfThumbnail *thumb;
	double view_x1, view_y1; 
	double view_x2, view_y2;
	
	g_return_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view));
	
	priv = thumbnails_view->priv;

	/* Get a pointer on the current thumb */
	thumb = &priv->thumbs[page -1];

	/* Get page outline coords */
	gpdf_thumbnails_view_get_page_outline_coords (thumbnails_view,
						      page, 
						      &view_x1, &view_y1, 
						      &view_x2, &view_y2);
	
	/* Update outline view canvas item */
	gnome_canvas_item_set (thumb->outline,
			       "x1", view_x1, "y1", view_y1,
			       "x2", view_x2, "y2", view_y2, 
			       NULL); 
}

static void
gpdf_thumbnails_view_set_selected_page (GPdfThumbnailsView *thumbnails_view,
					guint page)
{
	GPdfThumbnailsViewPrivate *priv;
	GPdfThumbnail *thumb;
	GdkPixbuf *enlighten; 

	g_return_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view));
	
	priv = thumbnails_view->priv;

	/* Get a pointer on the current thumb (old page) */
	thumb = &priv->thumbs[priv->current_page -1];

	/* Update text canvas item */
	gnome_canvas_item_set (thumb->text, 
			       "font", GPDF_THUMBNAILS_FONT,
			       NULL);

	/* Hide/Show items of old page */
	gnome_canvas_item_hide (thumb->outline);
	gnome_canvas_item_hide (thumb->outerborder_sel);

	/* Darken thumbnail */
	gnome_canvas_item_set (thumb->item,
			       "pixbuf", thumb->pixbuf,
			       NULL); 

	/* Keep new page as current one */
	priv->current_page = page;

	/* Get thumb for new page */
	thumb = &priv->thumbs[page -1];

	/* Update text canvas item for new page */
	gnome_canvas_item_set (thumb->text, 
			       "font", GPDF_THUMBNAILS_SEL_FONT,
			       NULL);

	/* Update page outline view */
	gpdf_thumbnails_view_update_page_outline (thumbnails_view, page); 

	/* Hide/Show items according to current page */
	gnome_canvas_item_show (thumb->outline);
	gnome_canvas_item_show (thumb->outerborder_sel);

	/* Enlighten pixbuf */
	enlighten = gdk_pixbuf_new (gdk_pixbuf_get_colorspace (thumb->pixbuf),
				    gdk_pixbuf_get_has_alpha (thumb->pixbuf),
				    gdk_pixbuf_get_bits_per_sample (thumb->pixbuf),
				    gdk_pixbuf_get_width (thumb->pixbuf),
				    gdk_pixbuf_get_height (thumb->pixbuf)); 
	gdk_pixbuf_saturate_and_pixelate (thumb->pixbuf,
					  enlighten,
					  GPDF_THUMBNAILS_ENLIGTH_FACTOR,
					  FALSE);
	gnome_canvas_item_set (thumb->item,
			       "pixbuf", enlighten,
			       NULL);
	gdk_pixbuf_unref (enlighten); 
}

static void
gpdf_thumbnails_view_page_changed_cb (GPdfView *gpdf_view,
				      int page,
				      gpointer user_data)
{
	GPdfThumbnailsView *thumbnails_view;
	GPdfThumbnailsViewPrivate *priv;
	double item_x, item_y;

	thumbnails_view = GPDF_THUMBNAILS_VIEW (user_data);

	g_return_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view));
	g_return_if_fail (GPDF_IS_VIEW (gpdf_view));

	priv = thumbnails_view->priv;

	if (priv->ignore_page_changed || !priv->thumbs_initialized) return;

	/* Get coord for new thumb */
	gpdf_thumbnails_view_page_box_coord (thumbnails_view,
					     page,
					     &item_x, &item_y,
					     NULL, NULL);

	/* Scroll to ensure the thumb is visible */
	gnome_canvas_scroll_to (priv->canvas, (int)item_x, (int)item_y);

	/* Do thumb stuff necessary to update current page status in thumbs */
	gpdf_thumbnails_view_set_selected_page (thumbnails_view, page);
}

static void
gpdf_thumbnails_view_zoom_changed_cb (GPdfView *gpdf_view,
				      double zoom,
				      gpointer user_data)
{
	GPdfThumbnailsView *thumbnails_view;
	GPdfThumbnailsViewPrivate *priv;

	thumbnails_view = GPDF_THUMBNAILS_VIEW (user_data);

	g_return_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view));
	g_return_if_fail (GPDF_IS_VIEW (gpdf_view));

	priv = thumbnails_view->priv;

	priv->zoom = zoom;
	
	if (priv->ignore_zoom_changed || !priv->thumbs_initialized) return;

	/* Do thumb stuff necessary to update current page status in thumbs */
	gpdf_thumbnails_view_set_selected_page (thumbnails_view, priv->current_page);
}

GtkWidget *
gpdf_thumbnails_view_new (GPdfControl *parent, GtkWidget *gpdf_view)
{
	GPdfThumbnailsView *thumbnails_view;

	thumbnails_view =
	  GPDF_THUMBNAILS_VIEW (g_object_new (
		  GPDF_TYPE_THUMBNAILS_VIEW, 
		  "view", (gpointer)gpdf_view,
		  "control", (gpointer)parent, 
		  NULL));

	g_object_ref (G_OBJECT (thumbnails_view->priv->gpdf_view)); 
	g_object_ref (G_OBJECT (thumbnails_view->priv->parent)); 
	gpdf_thumbnails_view_construct (thumbnails_view); 
	
	g_signal_connect (G_OBJECT (gpdf_view), "page_changed",
			  G_CALLBACK (gpdf_thumbnails_view_page_changed_cb),
			  thumbnails_view);

	g_signal_connect (G_OBJECT (gpdf_view), "zoom_changed",
			  G_CALLBACK (gpdf_thumbnails_view_zoom_changed_cb),
			  thumbnails_view);

	return GTK_WIDGET (thumbnails_view);
}

static void
gpdf_thumbnails_view_dispose (GObject *object)
{
	GPdfThumbnailsView *thumbnails_view;
	GPdfThumbnailsViewPrivate *priv;
	guint n; 
    
	g_return_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (object));
    
	thumbnails_view = GPDF_THUMBNAILS_VIEW (object);
	priv = thumbnails_view->priv;
    
	if (priv->gpdf_view) {
		g_object_unref (priv->gpdf_view);
		priv->gpdf_view = NULL; 
	}
	if (priv->thumbs) {
		for (n = 0; n < priv->n_pixbufs; n++)
		  gdk_pixbuf_unref (priv->thumbs[n].pixbuf); 
		g_free (priv->thumbs);
		priv->thumbs = NULL; 
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
gpdf_thumbnails_view_finalize (GObject *object)
{
	GPdfThumbnailsView *thumbnails_view;
	GPdfThumbnailsViewPrivate *priv;
    
	g_return_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (object));
    
	thumbnails_view = GPDF_THUMBNAILS_VIEW (object);
	priv = thumbnails_view->priv;
    
	thumbnails_view->priv->parent = NULL;
	
	if (thumbnails_view->priv) {
		g_free (thumbnails_view->priv);
		thumbnails_view->priv = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gpdf_thumbnails_view_instance_init (GPdfThumbnailsView *thumbnails_view)
{
	thumbnails_view->priv = g_new0 (GPdfThumbnailsViewPrivate, 1);
	thumbnails_view->priv->current_page = 1;
	thumbnails_view->priv->zoom = (double)1.0;
	thumbnails_view->priv->page_per_row = GPDF_THUMBNAILS_PER_ROW;
	thumbnails_view->priv->popup_menu_uiinfo =
	  (GnomeUIInfo*)g_memdup (tools_popup_menu_items_init,
				  sizeof (tools_popup_menu_items_init));	  
}

static void
gpdf_thumbnails_view_get_page_canvas_regions (GPdfThumbnailsView *thumbnails_view,
					      double *px1, double *py1,
					      double *px2, double *py2,
					      int *cx, int *cy,
					      double *vw, double *vh)
{
	GPdfView *gpdf_view;

	g_return_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view));

	gpdf_view = thumbnails_view->priv->gpdf_view;

	g_return_if_fail (GPDF_IS_VIEW (gpdf_view));

	gnome_canvas_get_scroll_region (GNOME_CANVAS (gpdf_view),
					px1, py1, px2, py2);
	gnome_canvas_get_scroll_offsets (GNOME_CANVAS (gpdf_view),
					 cx, cy);

	if (vw) *vw = GTK_LAYOUT (gpdf_view)->hadjustment->page_size;
	if (vh) *vh = GTK_LAYOUT (gpdf_view)->vadjustment->page_size; 
}

static void
gpdf_thumbnails_view_page_layout_adjustment_changed (GtkAdjustment *adjustment,
						     GPdfThumbnailsView *thumbnails_view)
{
	GPdfThumbnailsViewPrivate *priv;
	
	g_return_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view));
	
	priv = thumbnails_view->priv;
	
	gpdf_thumbnails_view_update_page_outline (thumbnails_view, priv->current_page);
}

static void
gpdf_thumbnails_view_canvas_item_event_cb (GnomeCanvasItem *item,
					   GdkEvent *event,
					   gpointer data)
{
	GPdfThumbnailsView *thumbnails_view;
	GPdfThumbnailsViewPrivate *priv;
	double item_x, item_y;
	double world_x, world_y;
	guint page;

	thumbnails_view = GPDF_THUMBNAILS_VIEW (data);

	g_return_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view));

	priv = thumbnails_view->priv;

	world_x = item_x = event->button.x;
	world_y = item_y = event->button.y;
	gnome_canvas_item_w2i (item->parent, &item_x, &item_y);
	
	for (page = 1; page <= priv->num_pages; page++) {
		if (item == priv->thumbs[page -1].item) break;
	}

	g_return_if_fail (page <= priv->num_pages);

	switch (event->type) {

	    case GDK_BUTTON_PRESS:
		if (event->button.button == 1) {
			priv->clicked_x = (gint)world_x;
			priv->clicked_y = (gint)world_y;

			priv->ignore_page_changed = TRUE;
			gpdf_thumbnails_view_set_selected_page (thumbnails_view, page);
			gpdf_thumbnails_view_update_page_outline (thumbnails_view, page); 
			gpdf_thumbnails_view_emit_thumbnail_selected (thumbnails_view,
								      (int)item_x, (int)item_y,
								      0, 0,
								      page);
			priv->ignore_page_changed = FALSE;
		}
		break;

	      default:
		break;
	}
	
}

static void
gpdf_thumbnails_view_canvas_outline_event_cb (GnomeCanvasItem *outline,
					      GdkEvent *event,
					      gpointer data)
{
	GPdfThumbnailsView *thumbnails_view;
	GPdfThumbnailsViewPrivate *priv;
	gint ptr_x, ptr_y;
	GdkModifierType mod;
	double outline_x, outline_y;
	double world_x, world_y;
	guint page;
	double view_x1, view_y1; 
	double view_x2, view_y2;
	double view_width, view_height;
	GdkDisplay *display;
	GdkCursor *cursor;
	GdkWindow *gdk_window;
	static double x, y;
	static gboolean dragging;
	static GPdfThumbnailsDragAction action = GPDF_DRAG_ACTION_NONE; 
	GtkAdjustment *hadj, *vadj; 
	double hval, vval;
	double zoom_w, zoom_h; 

	thumbnails_view = GPDF_THUMBNAILS_VIEW (data);

	g_return_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view));

	priv = thumbnails_view->priv;

	if (event->type == GDK_MOTION_NOTIFY && ((GdkEventMotion *)event)->is_hint) {
		gdk_window_get_pointer (gtk_widget_get_parent_window (priv->canvas_widget),
					&ptr_x, &ptr_y, &mod);
		world_x = outline_x = (double)ptr_x;
		world_y = outline_y = (double)ptr_y;
	}
	else {
		world_x = outline_x = event->button.x;
		world_y = outline_y = event->button.y;
	}
	gnome_canvas_item_w2i (outline->parent, &outline_x, &outline_y);
	
	for (page = 1; page <= priv->num_pages; page++) {
		if (outline == priv->thumbs[page -1].outline) 
			break;
	}

	view_width  = gdk_pixbuf_get_width  (priv->thumbs[page -1].pixbuf); 
	view_height = gdk_pixbuf_get_height (priv->thumbs[page -1].pixbuf);
	
	g_return_if_fail (page <= priv->num_pages);

	/* Get page outline coords */
	gpdf_thumbnails_view_get_page_outline_coords (thumbnails_view,
						      page, 
						      &view_x1, &view_y1, 
						      &view_x2, &view_y2);
	switch (event->type) {
		
	case GDK_LEAVE_NOTIFY:

		/* When leaving area restore cursor */
		display = gtk_widget_get_display (priv->canvas_widget);
		cursor = gdk_cursor_new_for_display (display, GDK_LEFT_PTR);
		gdk_window = GTK_WIDGET (priv->canvas_widget)->window;
		gdk_window_set_cursor (gdk_window, cursor);
		gdk_cursor_unref (cursor);
		gdk_flush();
		
		break;
		
	case GDK_BUTTON_PRESS:

		display = gtk_widget_get_display (priv->canvas_widget);
		if (event->button.button == 1) {
			/*
			 * Upper Left or Lower Right corner: Resize
			 */
			/* Upper Left */
			if (view_x1 <= outline_x && outline_x <= (view_x1+GPDF_THUMBNAIL_OUTLINE_CORNER_SIZE) &&
			    view_y1 <= outline_y && outline_y <= (view_y1+GPDF_THUMBNAIL_OUTLINE_CORNER_SIZE)) {
				cursor = gdk_cursor_new_for_display (display, GDK_TOP_LEFT_CORNER);
				action = GPDF_DRAG_ACTION_RESIZE_TL; 
			}
			else if ((view_x2-GPDF_THUMBNAIL_OUTLINE_CORNER_SIZE) <= outline_x && outline_x <= view_x2 &&
				 (view_y2-GPDF_THUMBNAIL_OUTLINE_CORNER_SIZE) <= outline_y && outline_y <= view_y2) {
				cursor = gdk_cursor_new_for_display (display, GDK_BOTTOM_RIGHT_CORNER);
				action = GPDF_DRAG_ACTION_RESIZE_BR; 
			}
		
			/*
			 * Upper Right or Lower Left corner: Move
			 */
			/* Upper Right */
			else if (((view_x2-GPDF_THUMBNAIL_OUTLINE_CORNER_SIZE) <= outline_x && outline_x <= view_x2 &&
				  view_y1 <= outline_y && outline_y <= (view_y1+GPDF_THUMBNAIL_OUTLINE_CORNER_SIZE)) ||
				 (view_x1 <= outline_x && outline_x <= (view_x1+GPDF_THUMBNAIL_OUTLINE_CORNER_SIZE) &&
				  (view_y2-GPDF_THUMBNAIL_OUTLINE_CORNER_SIZE) <= outline_y && outline_y <= view_y2)) {
				cursor = gdk_cursor_new_for_display (display, GDK_FLEUR);
				action = GPDF_DRAG_ACTION_MOVE; 
			}
			else {
				cursor = gdk_cursor_new_for_display (display, GDK_DRAPED_BOX);
				action = GPDF_DRAG_ACTION_MOVE; 
			}
			
			x = outline_x;
			y = outline_y;
			
			gnome_canvas_item_grab (outline,
						GDK_POINTER_MOTION_MASK | 
						GDK_POINTER_MOTION_HINT_MASK | 
						GDK_BUTTON_RELEASE_MASK,
						cursor,
						event->button.time);
			gdk_cursor_unref (cursor);
			gdk_flush();
			dragging = TRUE;
		}		
		break; 

	case GDK_MOTION_NOTIFY:
		display = gtk_widget_get_display (priv->canvas_widget);
		/*
		 * Upper Left or Lower Right corner: Resize
		 */
		/* Upper Left */
		if (view_x1 <= outline_x && outline_x <= (view_x1+GPDF_THUMBNAIL_OUTLINE_CORNER_SIZE) &&
		    view_y1 <= outline_y && outline_y <= (view_y1+GPDF_THUMBNAIL_OUTLINE_CORNER_SIZE)) {
			cursor = gdk_cursor_new_for_display (display, GDK_TOP_LEFT_CORNER);
		}
		else if ((view_x2-GPDF_THUMBNAIL_OUTLINE_CORNER_SIZE) <= outline_x && outline_x <= view_x2 &&
			 (view_y2-GPDF_THUMBNAIL_OUTLINE_CORNER_SIZE) <= outline_y && outline_y <= view_y2) {
			cursor = gdk_cursor_new_for_display (display, GDK_BOTTOM_RIGHT_CORNER);
		}
		
		/*
		 * Upper Right or Lower Left corner: Move
		 */
		/* Upper Right */
		else if (((view_x2-GPDF_THUMBNAIL_OUTLINE_CORNER_SIZE) <= outline_x && outline_x <= view_x2 &&
			  view_y1 <= outline_y && outline_y <= (view_y1+GPDF_THUMBNAIL_OUTLINE_CORNER_SIZE)) ||
			 (view_x1 <= outline_x && outline_x <= (view_x1+GPDF_THUMBNAIL_OUTLINE_CORNER_SIZE) &&
			  (view_y2-GPDF_THUMBNAIL_OUTLINE_CORNER_SIZE) <= outline_y && outline_y <= view_y2)) {
			cursor = gdk_cursor_new_for_display (display, GDK_FLEUR);
		}
		else {
			cursor = gdk_cursor_new_for_display (display, GDK_LEFT_PTR);
		}
		gdk_window = GTK_WIDGET (priv->canvas_widget)->window;
		gdk_window_set_cursor (gdk_window, cursor); 
		gdk_cursor_unref (cursor);
		gdk_flush();
		
		if (dragging &&
			 ((event->motion.state & GDK_BUTTON2_MASK) || 
			  ((event->motion.state & GDK_BUTTON1_MASK) && action == GPDF_DRAG_ACTION_MOVE))) {

			hadj = GTK_LAYOUT (priv->gpdf_view)->hadjustment; 
			vadj = GTK_LAYOUT (priv->gpdf_view)->vadjustment; 
			
			hval = gtk_adjustment_get_value(hadj) +
				((outline_x -x) * (hadj->upper -hadj->lower) / view_width); 
			vval = gtk_adjustment_get_value(vadj) +
				((outline_y -y) * (vadj->upper -vadj->lower) / view_height);

			gtk_adjustment_set_value(hadj, MIN (MAX (hval, hadj->lower), hadj->upper - hadj->page_size));
			gtk_adjustment_set_value(vadj, MIN (MAX (vval, vadj->lower), vadj->upper - vadj->page_size)); 
				
			x = outline_x;
			y = outline_y;
		}
		else if (dragging && (event->motion.state & GDK_BUTTON1_MASK) &&
			 action == GPDF_DRAG_ACTION_RESIZE_TL) {

			hadj = GTK_LAYOUT (priv->gpdf_view)->hadjustment; 
			vadj = GTK_LAYOUT (priv->gpdf_view)->vadjustment; 

			zoom_w = priv->zoom +
				((2 * (outline_x -x)) *
				 (hadj->upper -hadj->lower) /
				 (view_width * hadj->page_size)); 
			zoom_h = priv->zoom +
				((2 * (outline_y -y)) *
				 (vadj->upper -vadj->lower) /
				 (view_height * vadj->page_size));
			
			gpdf_view_zoom(priv->gpdf_view, MAX(zoom_w, zoom_h), FALSE); 

			x = outline_x;
			y = outline_y;
		}
		else if (dragging && (event->motion.state & GDK_BUTTON1_MASK) &&
			 action == GPDF_DRAG_ACTION_RESIZE_BR) {

			hadj = GTK_LAYOUT (priv->gpdf_view)->hadjustment; 
			vadj = GTK_LAYOUT (priv->gpdf_view)->vadjustment; 

			zoom_w = priv->zoom + ((2 * (x -outline_x)) *
					       (hadj->upper -hadj->lower) /
					       (view_width * hadj->page_size)); 
			zoom_h = priv->zoom + ((2 * (y -outline_y)) *
					       (vadj->upper -vadj->lower) /
					       (view_height * vadj->page_size));
			
			gpdf_view_zoom(priv->gpdf_view, MAX(zoom_w, zoom_h), FALSE); 

			x = outline_x;
			y = outline_y;
		}
		
		break;
		
	    case GDK_BUTTON_RELEASE:
		    if (dragging) {
			    gnome_canvas_item_ungrab (outline, event->button.time);
			    dragging = FALSE; 
		    }
				    
		break;

	      default:
		break;
	}
}

/*
 * This func could be the gpdf_thumbnail_construct method of
 * a GpdfThumbnail widget.
 */
static gboolean
gpdf_thumbnails_view_create_thumbnail (GPdfThumbnailsView *thumbnails_view,
				       gint page,
				       GdkPixbuf *pixbuf,
				       GPdfThumbnail *thumbnail)
{
	GPdfThumbnailsViewPrivate *priv; 
	GnomeCanvasItem *group;
	GnomeCanvasItem *item;
	double dx, dy, dw, dh;
	int width, height; 
	double outerwidth, outerheight;
	double shadow_x, shadow_y;
	gchar *text;

	g_return_val_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view),
			      FALSE); 
	
	priv = thumbnails_view->priv;

	thumbnail->pixbuf = pixbuf;

	width = gdk_pixbuf_get_width (pixbuf); 
	height = gdk_pixbuf_get_height (pixbuf); 

	/*
	 * Just a guess for now, in order to have
	 * an approximate layout while creating
	 * thumbs
	 */
	dw = (double)width;
	dh = (double)height;
	gpdf_thumbnails_view_page_box_coord(thumbnails_view,
					    page, 
					    &dx, &dy,
					    &dw, &dh);
	group = gnome_canvas_item_new(priv->root,
				      gnome_canvas_group_get_type(),
				      "x", dx,
				      "y", dy, 
				      NULL);
	thumbnail->group = group;

	outerwidth = width +
	  	GPDF_THUMBNAILS_INNER_MARGIN_LEFT +
		GPDF_THUMBNAILS_INNER_MARGIN_RIGHT +
		GPDF_THUMBNAILS_SHADOW_WIDTH;
	outerheight = height +
	  	GPDF_THUMBNAILS_INNER_MARGIN_TOP +
		GPDF_THUMBNAILS_INNER_MARGIN_BOTTOM +
	        GPDF_THUMBNAILS_SHADOW_HEIGHT +
		GPDF_THUMBNAILS_TEXT_HEIGHT;
	
	thumbnail->outerborder_sel =
	  gnome_canvas_item_new(GNOME_CANVAS_GROUP (group),
				gnome_canvas_rect_get_type(),
				"x1",(double)0,
				"y1",(double)0, 
				"x2", outerwidth, 
				"y2", outerheight,
				"outline_color", "white",
				NULL);
	if (priv->current_page == page)
	  gnome_canvas_item_show (thumbnail->outerborder_sel);
	else
	  gnome_canvas_item_hide (thumbnail->outerborder_sel);

	shadow_x = GPDF_THUMBNAILS_INNER_MARGIN_LEFT + GPDF_THUMBNAILS_SHADOW_WIDTH;
	shadow_y = GPDF_THUMBNAILS_INNER_MARGIN_TOP + GPDF_THUMBNAILS_SHADOW_HEIGHT;
	thumbnail->shadow =
	  gnome_canvas_item_new(GNOME_CANVAS_GROUP (group),
				gnome_canvas_rect_get_type(),
				"x1", shadow_x,
				"y1", shadow_y,
				"x2", shadow_x+width,
				"y2", shadow_y+height,
				"fill_color_rgba",
				GPDF_THUMBNAILS_SHADOW_COLOR,
				NULL);
	gnome_canvas_item_show (thumbnail->shadow);

	item =
	  gnome_canvas_item_new (GNOME_CANVAS_GROUP (group),
				 gnome_canvas_pixbuf_get_type(),
				 "pixbuf",	     pixbuf,
				 "x",		     (double)GPDF_THUMBNAILS_INNER_MARGIN_LEFT,
				 "x_in_pixels",	     TRUE, 
				 "y",		     (double)GPDF_THUMBNAILS_INNER_MARGIN_TOP,
				 "y_in_pixels",	     TRUE, 
				 "width",	     (double)width,
				 "width_in_pixels",  TRUE, 
				 "height",	     (double)height,
				 "height_in_pixels", TRUE, 
				 NULL);

	text = g_strdup_printf("%d", page);
	thumbnail->text =
	  gnome_canvas_item_new (GNOME_CANVAS_GROUP (group),
				 gnome_canvas_text_get_type (),
				 "text", text, 
				 "x", (double)(GPDF_THUMBNAILS_INNER_MARGIN_LEFT + width/2),
				 "y", (double)(GPDF_THUMBNAILS_INNER_MARGIN_TOP + height + 1), 
				 "font",
				 page == priv->current_page ? GPDF_THUMBNAILS_SEL_FONT : GPDF_THUMBNAILS_FONT,
				 "anchor", GTK_ANCHOR_N,
				 "fill_color", "black",
				 NULL);
	g_free (text);
	
	g_signal_connect (item, "event",
			  G_CALLBACK (gpdf_thumbnails_view_canvas_item_event_cb),
			  thumbnails_view);	
	thumbnail->item = item;

	thumbnail->outline =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP (group),
				       gnome_canvas_rect_get_type(),
				       "x1", 0.0, 
				       "y1", 0.0, 
				       "x2", 1.0, 
				       "y2", 1.0, 
				       "fill_color_rgba", 0x80808040,
				       "outline_color_rgba", 0x808080ff, 
				       NULL);
	gpdf_thumbnails_view_update_page_outline (thumbnails_view, page);
	if (priv->current_page == page)
		gnome_canvas_item_show (thumbnail->outline);
	else
		gnome_canvas_item_hide (thumbnail->outline);
	g_signal_connect (thumbnail->outline, "event",
			  G_CALLBACK (gpdf_thumbnails_view_canvas_outline_event_cb),
			  thumbnails_view);

	return TRUE; 
}

/*
 * This function create the thumbnail image from the page because
 * no thumbnail data is available in the PDF file.
 */
static void
gpdf_thumbnails_view_create_thumbnail_image (GPdfThumbnailsView *thumbnails_view,
					     Page *pdf_page,
					     gint page,
					     gint *width, gint *height)
{
	GPdfThumbnailsViewPrivate *priv; 
	gint lwidth = GPDF_DEFAULT_THUMB_WIDTH;
	GdkColorspace gdk_colorspace = GDK_COLORSPACE_RGB;
	gboolean has_alpha = FALSE; 
	gint bitsPerComponent = 8;
	GdkPixbuf *pixbuf;
	gint dest_width, dest_height;
	gdouble page_ratio;
	
	g_return_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view)); 
	
	priv = thumbnails_view->priv;

	gdk_colorspace = GDK_COLORSPACE_RGB;
	has_alpha = FALSE;
	page_ratio = pdf_page->getHeight () / pdf_page->getWidth ();
	dest_width = lwidth;
	dest_height = (gint)(lwidth*page_ratio);

	pixbuf = gdk_pixbuf_new (gdk_colorspace, has_alpha, bitsPerComponent, dest_width, dest_height);
	gdk_pixbuf_fill (pixbuf, 0xffffffff);
	
	/* Create the empty thumbnail itself */
	if (gpdf_thumbnails_view_create_thumbnail (thumbnails_view,
						   page,
						   pixbuf,
						   &priv->thumbs[page -1])) {
		priv->thumbs[page -1].generated = FALSE; 
		priv->n_pixbufs++;
	}

	if (width) *width = dest_width; 
	if (height) *height = dest_height; 
}

/*
 * The PDF file contains thumbnails images data and we need to render them
 * here.
 *
 * FIXME: Should take care of what kind of colorspace the pixbuf data
 * we got from Thumb object is, in order to transform data if not
 * RGB.
 * At now Thumb object only encode data as RGB images.
 */
static void
gpdf_thumbnails_view_render_thumbnail_image (GPdfThumbnailsView *thumbnails_view,
					     gint page,
					     gint width, gint height,
					     GfxColorSpace *colorSpace,
					     gint bitsPerComponent,
					     Thumb *thumb)
{
	GPdfThumbnailsViewPrivate *priv; 
	guchar *data;
	GdkPixbuf *pixbuf;

	g_return_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view)); 
	
	priv = thumbnails_view->priv;

	data = thumb->getPixbufData();
	
	/* FIXME: Should use colorSpace (GfxColorSpace *) here !! */

	pixbuf = gdk_pixbuf_new_from_data(data,
					  GDK_COLORSPACE_RGB,
					  FALSE,
					  8,
					  width, height,
					  width * 3,
					  NULL, NULL);

	/* Create the thumbnail itself */
	if (gpdf_thumbnails_view_create_thumbnail (thumbnails_view,
						   page,
						   pixbuf,
						   &priv->thumbs[page -1])) {
		priv->thumbs[page -1].generated = TRUE; 
		priv->n_pixbufs++;
	}
		
}

static gboolean
gpdf_thumbnails_view_populate_idle (GPdfThumbnailsView *thumbnails_view)
{
	GPdfThumbnailsViewPrivate *priv;
	Page *the_page;
	Object the_thumb;
	Thumb *thumb = NULL;
	gboolean have_ethumbs = FALSE;
	gchar *status = NULL;
	double ratio = 0.0; 
	
	g_return_val_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view),
			      TRUE);
	
	priv = thumbnails_view->priv;
	ratio = ((double)priv->current_gen_thumb/(double)priv->num_pages); 
	status = g_strdup_printf ("Processing thumbnail #%d/%d: %3d %%",
				  priv->current_gen_thumb,
				  priv->num_pages, 
				  (int)(ratio*100.0));
	gpdf_control_private_set_status (priv->parent, status);
	gpdf_control_private_set_fraction (priv->parent, ratio); 
	g_free (status); 

	/* Get the PDF objects for fetching thumb */
	the_page = priv->pdf_doc->getCatalog ()->getPage (++(priv->current_gen_thumb));
	the_page->getThumb(&the_thumb);
	
	/*
	 * If no thumb object available in PDF, render each page in
	 * thumbnail mode, else fetch thumbnail object and render image
	 * data.
	 */
	if (!(the_thumb.isNull () || the_thumb.isNone())) {
		/* Build the thumbnail object */
		thumb = new Thumb(priv->pdf_doc->getXRef (),
				  &the_thumb);
		
		have_ethumbs = thumb->ok();
	}
	
	if (have_ethumbs) {
		/* Keep maximum width/height to compute layout later */
		priv->max_page_width = MAX ((guint)thumb->getWidth(), priv->max_page_width);
		priv->max_page_height = MAX ((guint)thumb->getHeight(), priv->max_page_height);
		
		gpdf_thumbnails_view_render_thumbnail_image (thumbnails_view,
							     priv->current_gen_thumb, 
							     thumb->getWidth(),
							     thumb->getHeight(),
							     thumb->getColorSpace(),
							     thumb->getBitsPerComponent(),
							     thumb);
	}
	else {
		GtkWidget *item;
		guint width, height;
		
		/*
		 * FIXME: Could perhaps render only displayed thumbs. This process may
		 * be time consuming.
		 */
		gpdf_thumbnails_view_create_thumbnail_image (thumbnails_view, the_page,
							     priv->current_gen_thumb, 
							     (gint*)&width, (gint*)&height);
		
		/* Keep maximum width/height to compute layout later */
		priv->max_page_width = MAX (width, priv->max_page_width);
		priv->max_page_height = MAX (height, priv->max_page_height);
		
#if CAN_GENERATE_THUMBNAILS
		item = priv->popup_menu_uiinfo[POPUP_MENU_GENERATE_INDEX].widget;
		if (GTK_IS_WIDGET (item))
		  gtk_widget_set_sensitive (item, TRUE); 
		item = priv->popup_menu_uiinfo[POPUP_MENU_GENERATE_ALL_INDEX].widget;
		if (GTK_IS_WIDGET (item))
		  gtk_widget_set_sensitive (item, TRUE); 
#endif
	}

	/* Width and height are best known, set scroll region and relayout */
	gpdf_thumbnails_view_set_scroll_region(thumbnails_view);
		
	if (priv->current_gen_thumb == priv->num_pages)
	{
		GdkDisplay *display;
		GdkWindow *parent_window;
		
		/* All thumbs are setup, let listen page canvas adjustments to update page outline */
		g_signal_connect (GTK_LAYOUT (priv->gpdf_view)->hadjustment, "value_changed",
				  G_CALLBACK (gpdf_thumbnails_view_page_layout_adjustment_changed),
				  thumbnails_view); 
		g_signal_connect (GTK_LAYOUT (priv->gpdf_view)->hadjustment, "changed",
				  G_CALLBACK (gpdf_thumbnails_view_page_layout_adjustment_changed),
				  thumbnails_view); 
		g_signal_connect (GTK_LAYOUT (priv->gpdf_view)->vadjustment, "value_changed",
				  G_CALLBACK (gpdf_thumbnails_view_page_layout_adjustment_changed),
				  thumbnails_view); 
		g_signal_connect (GTK_LAYOUT (priv->gpdf_view)->vadjustment, "changed",
				  G_CALLBACK (gpdf_thumbnails_view_page_layout_adjustment_changed),
				  thumbnails_view); 
		
		/* Select first page */
		gpdf_thumbnails_view_set_selected_page (thumbnails_view, 1);
		
		priv->thumbs_initialized = TRUE;
		
		display = gtk_widget_get_display (priv->canvas_widget);
		parent_window = GTK_WIDGET (priv->canvas_widget)->window;
		if (GDK_IS_WINDOW (parent_window))
			gdk_window_set_cursor (parent_window, NULL); 
		gdk_flush();

		gpdf_control_private_clear_stack (priv->parent);
		gpdf_control_private_set_fraction (priv->parent, 0.0); 
		g_signal_handler_disconnect (G_OBJECT (priv->parent), priv->idle_dcon_id); 
		gpdf_thumbnails_view_emit_ready (thumbnails_view);
	}
	else {
		GdkDisplay *display;
		GdkCursor *cursor;
		GdkWindow *parent_window;
		
		/* Set watch cursor while view not ready */
		display = gtk_widget_get_display (priv->canvas_widget);
		cursor = gdk_cursor_new_for_display (display, GDK_WATCH);
		parent_window = GTK_WIDGET (priv->canvas_widget)->window;
		if (GDK_IS_WINDOW (parent_window))
			gdk_window_set_cursor (parent_window, cursor); 
		gdk_cursor_unref (cursor);
		gdk_flush();
	}

	return (priv->current_gen_thumb != priv->num_pages); 
}

static void
disconnected_handler (gpointer control, gpointer data)
{
	g_source_remove (GPOINTER_TO_UINT (data));
}

void
gpdf_thumbnails_view_set_pdf_doc (GPdfThumbnailsView *thumbnails_view,
				  PDFDoc *pdf_doc)
{
    GPdfThumbnailsViewPrivate *priv;

    g_return_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view));

    priv = thumbnails_view->priv;

    if (pdf_doc != priv->pdf_doc)
    {
	    priv->pdf_doc = pdf_doc;

	    /* Get number of pages */
	    priv->num_pages = priv->pdf_doc->getNumPages ();

	    /* Allocate thumbs table */
	    priv->thumbs =(GPdfThumbnail *)g_new0 (GPdfThumbnail, priv->num_pages);

	    /*
	     * Init page max sizes: We try a guess in order to have thumbs created
	     * in an approximate place. When READY signal will be emitted the whole
	     * layout will be recomputed with exact values.
	     */
	    priv->max_page_width = 45;
	    priv->max_page_height = 90;

	    /* Init index of last generated thumb */
	    priv->current_gen_thumb = 0;

	    /*
	     * As thumb computation may be a long process
	     * we make it as backgroud task.
	     */
	    priv->idle_id = g_idle_add_full (G_PRIORITY_DEFAULT_IDLE +1,
					     (GSourceFunc)gpdf_thumbnails_view_populate_idle,
					     thumbnails_view, 
					     NULL);

	    priv->idle_dcon_id =
		    g_signal_connect (G_OBJECT (priv->parent), "disconnected",
				      G_CALLBACK (disconnected_handler),
				      GUINT_TO_POINTER (priv->idle_id));
    }
}

#if CAN_GENERATE_THUMBNAILS
static void
gpdf_thumbnails_view_popup_menu_item_generate_cb (GtkMenuItem *item,
						  GPdfThumbnailsView *thumbnails_view)
{
	GPdfThumbnailsViewPrivate *priv;
	
	g_return_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view));
	
	priv = thumbnails_view->priv;
	
	gpdf_control_private_warn_dialog (priv->parent, 
					  _("Not yet implemented!"),
					  _("Thumbnail Generation feature not yet implemented... Sorry."),
					  TRUE);					  
}

static void
gpdf_thumbnails_view_popup_menu_item_generate_all_cb (GtkMenuItem *item,
						      GPdfThumbnailsView *thumbnails_view)
{
	GPdfThumbnailsViewPrivate *priv;
	
	g_return_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view));
	
	priv = thumbnails_view->priv;
	
	gpdf_control_private_warn_dialog (priv->parent, 
					  _("Not yet implemented!"),
					  _("All Thumbnails Generation feature not yet implemented... Sorry."),
					  TRUE);					  
}
#endif

GtkWidget*
gpdf_thumbnails_view_get_tools_menu (GPdfThumbnailsView *thumbnails_view)
{
#if CAN_GENERATE_THUMBNAILS
	GPdfThumbnailsViewPrivate *priv;
	GtkWidget *item; 
	
	g_return_val_if_fail (GPDF_IS_NON_NULL_THUMBNAILS_VIEW (thumbnails_view), NULL);
	
	priv = thumbnails_view->priv;
	
	if (!priv->popup_menu) {
		priv->popup_menu = gtk_menu_new ();
		gnome_app_fill_menu_with_data (GTK_MENU_SHELL (priv->popup_menu),
					       priv->popup_menu_uiinfo,
					       NULL, FALSE, 0,
					       (gpointer)thumbnails_view);
		item = priv->popup_menu_uiinfo[POPUP_MENU_GENERATE_INDEX].widget;
		gtk_widget_set_sensitive (item, FALSE); 

		item = priv->popup_menu_uiinfo[POPUP_MENU_GENERATE_ALL_INDEX].widget;
		gtk_widget_set_sensitive (item, FALSE); 
	}
	
	return priv->popup_menu; 
#else
	return NULL;
#endif
}

END_EXTERN_C
