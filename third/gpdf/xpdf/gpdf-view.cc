/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; c-indent-level: 8 -*- */
/*  PDF view widget
 *
 *  Copyright (C) 2002 Martin Kretzschmar
 *
 *  Author:
 *    Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
 *
 * GPdf is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPdf is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include <unistd.h>
#include <aconf.h>
#include <math.h>
#include "gpdf-view.h"
#include "gpdf-util.h"
#include "gpdf-marshal.c"
#include "gpdf-g-switch.h"
#  include <glib/gi18n.h>
#  include <gdk/gdkkeysyms.h>
#  include <libgnome/gnome-macros.h>
#  include <libgnomeprint/gnome-print.h>
#  include <libgnomeprintui/gnome-print-preview.h>
#  include <libgnomevfs/gnome-vfs-mime.h>
#  include <libgnomevfs/gnome-vfs-mime-handlers.h>
#  include <gtk/gtkwidget.h>
#  include <ggv-document.h>
#  include "gtkgesture.h"
#  include <glade/glade.h>
#include "gpdf-g-switch.h"
#include "gpdf-control.h"
#include "gpdf-control-private.h"
#include "GPOutputDev.h"
#include "PDFDoc.h"
#include "Link.h"
#include "gpdf-links-canvas-layer.h"
#ifdef USE_ANNOTS_VIEW
#include "gpdf-annots-view.h"
#endif

BEGIN_EXTERN_C

struct _GPdfViewPrivate {
	GnomePrintContext *print_context;
	GPOutputDev *output_dev;
	PDFDoc *pdf_doc;

	GPdfLinksCanvasLayer *links_layer;

	GnomeCanvasItem *page_background;
	GnomeCanvasItem *page_shadow;

	GtkGestureHandler *gesture_handler;

	gint requested_page;
	gint current_page;
	gdouble zoom;
	guint dragging : 1;
	gint drag_anchor_x, drag_anchor_y;
	gint drag_offset_x, drag_offset_y;

	G_List *forward_history;
	G_List *backward_history;

#ifdef USE_ANNOTS_VIEW
	gboolean (*annots_cb)(Annot *annot, gpointer data);
	GPdfAnnotsView *annots_view;
#endif

	guint status_to_id;
	
	GPdfControl *parent;
};

#define PARENT_TYPE GNOME_TYPE_CANVAS
static GnomeCanvasClass *parent_class = NULL;

#define GPDF_IS_NON_NULL_VIEW(obj) (((obj) != NULL) && (GPDF_IS_VIEW ((obj))))
#define GPDF_IS_VIEW_WITH_DOC(obj) \
(GPDF_IS_NON_NULL_VIEW (obj) && ((GPdfView *)(obj))->priv->pdf_doc)

enum {
	ZOOM_CHANGED_SIGNAL,
	PAGE_CHANGED_SIGNAL,
	CLOSE_REQUESTED_SIGNAL,
	QUIT_REQUESTED_SIGNAL,
	LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_CONTROL,
	PROP_PDFDOC	  
};

static guint gpdf_view_signals [LAST_SIGNAL];


#define PAGE_PAD ((gdouble)3.0)

static void
gpdf_view_page_background_show (GPdfView *gpdf_view)
{
	g_return_if_fail (GPDF_IS_NON_NULL_VIEW (gpdf_view));

	gnome_canvas_item_show (gpdf_view->priv->page_background);
	gnome_canvas_item_show (gpdf_view->priv->page_shadow);
}

static void
gpdf_view_page_background_hide (GPdfView *gpdf_view)
{
	g_return_if_fail (GPDF_IS_NON_NULL_VIEW (gpdf_view));

	gnome_canvas_item_hide (gpdf_view->priv->page_background);
	gnome_canvas_item_hide (gpdf_view->priv->page_shadow);
}

static void
gpdf_view_page_background_resize (GPdfView *gpdf_view,
				  gdouble width, gdouble height)
{
	g_return_if_fail (GPDF_IS_NON_NULL_VIEW (gpdf_view));

	gnome_canvas_item_set (gpdf_view->priv->page_background,
			       "x2", width, "y2", height, NULL);
	gnome_canvas_item_set (gpdf_view->priv->page_shadow,
			       "x2", width + PAGE_PAD - 1.0,
			       "y2", height + PAGE_PAD - 1.0,
			       NULL);
}

#ifdef USE_ANNOTS_VIEW
static GPdfAnnotsView *
gpdf_view_get_annots_view (GPdfView *gpdf_view)
{
	g_return_val_if_fail (GPDF_IS_NON_NULL_VIEW (gpdf_view),
			      NULL);

	if (!gpdf_view->priv->annots_view)
	  gpdf_view->priv->annots_view = GPDF_ANNOTS_VIEW 
	    (gpdf_control_private_get_annots_view (gpdf_view->priv->parent)); 

	return gpdf_view->priv->annots_view; 
}
#endif

static void
gpdf_view_setup_page_transform (gdouble *transform,
				gdouble x1, gdouble y1,
				gdouble x2, gdouble y2,
				gint angle)
{
	double width;
	double height;
	double rotate [6];
	double translate [6];

	width = x2 - x1;
	height = y2 - y1;

	art_affine_identity (transform);
	art_affine_flip (transform, transform, FALSE, TRUE);
	art_affine_rotate (rotate, angle);
	art_affine_multiply (transform, transform, rotate);

	if (angle == 90) {
		transform [4] = 0.0;
		transform [5] = 0.0;
	} else if (angle == 180) {
		transform [4] = width;
		transform [5] = 0.0;
	} else if (angle == 270) {
		transform [4] = height;
		transform [5] = width;
	} else /* if (angle == 0) */ {
		transform [4] = 0.0;
		transform [5] = height;
	}

	art_affine_translate (translate, -x1, -y1);
	art_affine_multiply (transform, translate, transform);
}

static gint
gpdf_view_canonical_multiple_of_90 (gint n)
{
	while (n >= 360) {
		n -= 360;
	}
	while (n < 0) {
		n += 360;
	}

	return 90 * (gint)((n / 90.0) + .5);
}

static void
gpdf_free_history_node (gpointer node, gpointer user_data)
{
	g_free (node);
}

static void
gpdf_view_history_stack_page (GPdfView *gpdf_view)
{
	GPdfViewPrivate *priv = gpdf_view->priv;
	GPdfViewHistory *hnode;

	/* First delete forward history if one exists */
	if (priv->forward_history) {
		g_list_foreach (priv->forward_history,
				gpdf_free_history_node,
				NULL);
		g_list_free (priv->forward_history);
		priv->forward_history = (G_List *)NULL;
	}

	/* Prepend history node */
	hnode = (GPdfViewHistory *)g_malloc (sizeof(GPdfViewHistory));
	hnode->zoom = priv->zoom;
	hnode->page = priv->current_page;
	hnode->hval = gtk_adjustment_get_value(GTK_LAYOUT (gpdf_view)->hadjustment); 
	hnode->vval = gtk_adjustment_get_value(GTK_LAYOUT (gpdf_view)->vadjustment); 
	priv->backward_history = g_list_prepend (priv->backward_history, (gpointer)hnode);
}

void
gpdf_view_clear_history (GPdfView *gpdf_view)
{
	GPdfViewPrivate *priv;

	g_return_if_fail (GPDF_IS_NON_NULL_VIEW (gpdf_view));

	priv = gpdf_view->priv;

	if (priv->forward_history) {
		g_list_foreach (priv->forward_history,
				gpdf_free_history_node,
				NULL);
		g_list_free (priv->forward_history);
		priv->forward_history = NULL;
	}

	if (priv->backward_history) {
		g_list_foreach (priv->backward_history,
				gpdf_free_history_node,
				NULL);
		g_list_free (priv->backward_history);
		priv->backward_history = NULL;
	}	
}

static void
gpdf_view_link_action_goto (GPdfView *gpdf_view, LinkGoTo *link_goto)
{
	PDFDoc *pdf_doc;
	LinkDest *dest;
	GString *named_dest;

	g_return_if_fail (GPDF_IS_VIEW_WITH_DOC (gpdf_view));
	g_return_if_fail (dynamic_cast <LinkGoTo *> (link_goto) != NULL);

	pdf_doc = gpdf_view->priv->pdf_doc;
	dest = link_goto->getDest ();
	named_dest = link_goto->getNamedDest ();

	if (dest != NULL) {
		dest = dest->copy();
	} else if (named_dest != NULL) {
		named_dest = named_dest->copy();
		dest = pdf_doc->findDest (named_dest);
		delete named_dest;
	}

	if (dest != NULL) {
		Ref page_ref;
		int page;

		if (dest->isPageRef ()) {
			page_ref = dest->getPageRef ();
			page = pdf_doc->findPage (page_ref.num, page_ref.gen);
		} else {
			page = dest->getPageNum ();
		}

		delete dest;

		if (page != gpdf_view->priv->requested_page) {
			gpdf_view_goto_page (gpdf_view, page);
			gpdf_view_scroll_to_top (gpdf_view);
		}
	}
}

static void
gpdf_view_link_action_named (GPdfView *gpdf_view, LinkNamed *link_named)
{
	const char *action_name;

	g_return_if_fail (GPDF_IS_VIEW_WITH_DOC (gpdf_view));
	g_return_if_fail (dynamic_cast <LinkNamed *> (link_named) != NULL);

	action_name = link_named->getName ()->getCString ();

	if (strcmp (action_name, "NextPage") == 0) {
		gpdf_view_page_next (gpdf_view);
	} else if (strcmp (action_name, "PrevPage") == 0) {
		gpdf_view_page_prev (gpdf_view);
	} else if (strcmp (action_name, "FirstPage") == 0) {
		gpdf_view_page_first (gpdf_view);
	} else if (strcmp (action_name, "LastPage") == 0) {
		gpdf_view_page_last (gpdf_view);
	} else if (strcmp (action_name, "Close") == 0) {
		g_signal_emit (
			G_OBJECT (gpdf_view), CLOSE_REQUESTED_SIGNAL, 0);
	} else if (strcmp (action_name, "Quit") == 0) {
		g_signal_emit (G_OBJECT (gpdf_view), QUIT_REQUESTED_SIGNAL, 0);
	} else {
		g_message ("Unimplemented named link action: %s", action_name);
	}
}

static void
gpdf_view_link_action_uri (GPdfView *gpdf_view, LinkURI *link_uri)
{
	gchar *uri; 
	GnomeVFSMimeAction *action; 
	GnomeVFSMimeApplication *app;
	GnomeVFSURI *vfsuri; 
	G_List *uris = NULL;
	const gchar *mime;

	g_message ("gpdf_view_link_action_uri: enters ..."); 

	g_return_if_fail (GPDF_IS_VIEW_WITH_DOC (gpdf_view));
	g_return_if_fail (dynamic_cast <LinkURI *> (link_uri) != NULL);

	uri = g_strdup (link_uri->getURI ()->getCString ()); 
	g_message ("gpdf_view_link_action_uri: uri = '%s'", uri); 
	uris = g_list_append (uris, uri);

	vfsuri = gnome_vfs_uri_new ((const gchar *)uris->data); 
	mime = gnome_vfs_get_mime_type_common (vfsuri);

	g_message ("gpdf_view_link_action_uri: mime = '%s'", mime); 

	if (!mime) {
		mime = "text/html";
		g_message ("gpdf_view_link_action_uri: using default mime = '%s'", mime); 
	}

	action = gnome_vfs_mime_get_default_action (mime); 
	if (gnome_vfs_mime_action_launch (action, uris) != GNOME_VFS_OK) {
		app = gnome_vfs_mime_get_default_application (mime); 
		
		if (!app || gnome_vfs_mime_application_launch (app, uris) != GNOME_VFS_OK) {

			/* Fallback: No component or app was found. Use a browser */
			mime = "text/html";
			action = gnome_vfs_mime_get_default_action (mime);
			if (gnome_vfs_mime_action_launch (action, uris) != GNOME_VFS_OK) {
				app = gnome_vfs_mime_get_default_application (mime); 
				if (!app) {
					gchar *msg = 
					  g_strdup_printf(_("No application was found to open the URI: %s"),
							  (const gchar *)uris->data);
					gpdf_control_private_error_dialog (gpdf_view->priv->parent, 
									   _("Application Launch Failure!"),
									   msg,
									   TRUE,
									   FALSE);
					g_free (msg);
				} else if (gnome_vfs_mime_application_launch (app, uris) != GNOME_VFS_OK) {
					gchar *msg = 
					  g_strdup_printf(_("Failed to launch %s for URI: %s"),
							  app->name,
							  (const gchar *)uris->data);
					gpdf_control_private_error_dialog (gpdf_view->priv->parent, 
									   _("Application Launch Failure!"),
									   msg,
									   TRUE,
									   FALSE);
					g_free (msg);
				}
			}
		} else {
			gchar *msg = g_strdup_printf ("URI '%s' launched with %s",
						      (const gchar *)uris->data,
						      app->name);
			gpdf_control_private_info_dialog (gpdf_view->priv->parent, 
							  _("Application Launch notification!"),
							  msg,
							  TRUE);
			g_free (msg);			
		}

		gnome_vfs_mime_application_free (app); 
	}
	g_list_free (uris); 
	gnome_vfs_uri_unref (vfsuri); 
	delete uri; 
}

static void
gpdf_view_link_clicked_cb (GPdfLinksCanvasLayer *links_layer,
			   Link                 *link,
			   gpointer              user_data)
{
	GPdfView *gpdf_view;
	LinkAction *link_action;

	g_return_if_fail (GPDF_IS_VIEW (user_data));
	g_return_if_fail (dynamic_cast <Link *> (link) != NULL);

	gpdf_view = GPDF_VIEW (user_data);
	link_action = link->getAction ();

	switch (link_action->getKind ()) {
	case actionGoTo:
		gpdf_view_link_action_goto (
			gpdf_view, dynamic_cast <LinkGoTo *> (link_action));
		break;
	case actionNamed:
		gpdf_view_link_action_named (
			gpdf_view, dynamic_cast <LinkNamed *> (link_action));
		break;
	case actionURI:
		gpdf_view_link_action_uri (
			gpdf_view, dynamic_cast <LinkURI *> (link_action));
		break; 
	default:
		g_message ("Click on link: unimplemented Action");
		break;
	}
}

static gboolean
gpdf_view_reset_link_status_cb (gpointer user_data)
{
	GPdfView *gpdf_view;
	GPdfViewPrivate *priv;
	
	g_return_val_if_fail (GPDF_IS_VIEW (user_data), FALSE);
	
	gpdf_view = GPDF_VIEW (user_data);
	priv = gpdf_view->priv;
	
	gpdf_control_private_clear_stack (priv->parent);
	priv->status_to_id = 0;
	
	return FALSE;
}

static void
gpdf_view_link_entered_cb (GPdfLinksCanvasLayer *links_layer,
			   Link                 *link,
			   gpointer              user_data)
{
	GPdfView *gpdf_view;
	GPdfViewPrivate *priv;

	g_return_if_fail (GPDF_IS_VIEW (user_data));
	g_return_if_fail (dynamic_cast <Link *> (link) != NULL);

	gpdf_view = GPDF_VIEW (user_data);
	priv = gpdf_view->priv;

	if (link) {
	    LinkAction *action = link->getAction ();
	    gchar *text = NULL;
	    
	    switch (action->getKind ()) {
	      case actionGoTo:
	      {
		  LinkGoTo *link_goto = NULL;
		  LinkDest *link_dest = NULL;
		  GString *named_dest;
		  Ref page_ref;
		  guint page = 0;
		  
		  link_goto = dynamic_cast <LinkGoTo *> (action);
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
		  text = g_strdup_printf (_("Go to page #%d"), page);
		  break;
	      }
	      case actionGoToR:
	      {
		  LinkGoToR *link_gotor = NULL;
		  LinkDest *link_dest = NULL;
		  GString *named_dest;
		  Ref page_ref;
		  guint page = 0;
		  GString *filename;
		  
		  link_gotor = dynamic_cast <LinkGoToR *> (action);
		  link_dest = link_gotor->getDest ();
		  named_dest = link_gotor->getNamedDest ();
		  filename = link_gotor->getNamedDest ();
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
		  text = g_strdup_printf (_("Go to file %s:#%d"), filename->getCString (), page); 
		  break;
	      }		
	      case actionLaunch:
	      {
		  LinkLaunch *link_launch = NULL;
		  
		  link_launch = dynamic_cast <LinkLaunch *> (link_launch);
		  text = g_strdup_printf (_("Launch: %s %s"),
					  link_launch->getFileName ()->getCString (), 
					  link_launch->getParams ()->getCString ());
		  break;
	      }
	      case actionURI:
	      {
		  LinkURI *link_uri = NULL;
		  
		  link_uri = dynamic_cast <LinkURI *> (action);
		  text = g_strdup_printf (_("Browse %s"), link_uri->getURI ()->getCString ());
		  break; 
	      }
	      case actionNamed:
	      {
		  LinkNamed *link_named = NULL;
		  
		  link_named = dynamic_cast <LinkNamed *> (action);
		  text = g_strdup_printf (_("Action: %s"), link_named->getName ()->getCString ());
		  break;
	      }
	      case actionMovie:
	      {
		  LinkMovie *link_movie = NULL;
		  
		  link_movie = dynamic_cast <LinkMovie *> (action);
		  text = g_strdup_printf (_("Movie: %s"),
					  link_movie->getTitle ()->getCString ());
		  break;
	      }
	      case actionUnknown:
	      default:
	      {
		  LinkUnknown *link_unknown = NULL;
		  
		  link_unknown = dynamic_cast <LinkUnknown *> (action);
		  g_message ("Click on link: unknown Action");
		  text = g_strdup_printf (_("Unknown action: %s !"),
					  link_unknown->getAction ()->getCString ());
		  break;
	      }
	    }

	    if (text) {
		gpdf_control_private_set_status (priv->parent, text);
		if (priv->status_to_id)
		  g_source_remove (priv->status_to_id);
		priv->status_to_id =
		  g_timeout_add (3000,
				 (GSourceFunc)gpdf_view_reset_link_status_cb,
				 gpdf_view);
		g_free (text);
	    }
	}	
}

static void
gpdf_view_link_leaved_cb (GPdfLinksCanvasLayer *links_layer,
			  Link                 *link,
			  gpointer              user_data)
{
	GPdfView *gpdf_view;
	GPdfViewPrivate *priv;

	g_return_if_fail (GPDF_IS_VIEW (user_data));

	gpdf_view = GPDF_VIEW (user_data);
	priv = gpdf_view->priv;

	gpdf_control_private_pop (priv->parent);
	if (priv->status_to_id) {
	    g_source_remove (priv->status_to_id);
	    priv->status_to_id = 0;
	}
}

static void
gpdf_view_render_page (GPdfView *gpdf_view)
{
	GPdfViewPrivate *priv;
	GnomePrintConfig *config;
	PDFDoc *pdf_doc;
	Page *pdf_page;
	const PDFRectangle *rect;
	gint page;
	gint page_rotate;
	gdouble page_width_rot, page_height_rot;
	ArtDRect region;
	gdouble page_transform [6];
	Object obj;

	g_return_if_fail (GPDF_IS_VIEW_WITH_DOC (gpdf_view));

	priv = gpdf_view->priv;

	if (priv->requested_page == priv->current_page)
		return;

	gpdf_control_private_set_wait_cursor (priv->parent);
	
	page = priv->requested_page;

	if (priv->print_context) {
		g_object_unref (priv->print_context);
		priv->print_context = NULL;
	}

	pdf_doc = priv->pdf_doc;
	pdf_page = pdf_doc->getCatalog ()->getPage (page);
	rect = pdf_page->getBox ();

	page_rotate = gpdf_view_canonical_multiple_of_90 (pdf_doc->getPageRotate (page));

	gpdf_view_setup_page_transform (page_transform,
					rect->x1, rect->y1, rect->x2, rect->y2,
					page_rotate);

	if (page_rotate == 90 || page_rotate == 270) {
		page_width_rot = pdf_doc->getPageHeight (page);
		page_height_rot = pdf_doc->getPageWidth (page);
	} else /* if (page_rotate == 0 || page_rotate == 180) */ {
		page_width_rot = pdf_doc->getPageWidth (page);
		page_height_rot = pdf_doc->getPageHeight (page);
	}

	/* Set scrolling region */
	region.x0 = region.y0 = 0.0 - PAGE_PAD;
	region.x1 = page_width_rot + PAGE_PAD;
	region.y1 = page_height_rot + PAGE_PAD;

	gpdf_view_page_background_resize (gpdf_view,
					  page_width_rot, page_height_rot);

	config = gnome_print_config_default ();
	priv->print_context = gnome_print_preview_new_full (
		config, GNOME_CANVAS (gpdf_view), page_transform, &region);
	gnome_print_config_unref (config);

	priv->output_dev->setPrintContext (priv->print_context);
#ifdef USE_ANNOTS_VIEW
	priv->pdf_doc->displayPage (priv->output_dev,
				    page,
				    72.0, 72.0, 
				    0,
				    gTrue, gFalse,
				    NULL, NULL, 
				    priv->annots_cb,
				    gpdf_view_get_annots_view (gpdf_view));
#else
	priv->pdf_doc->displayPage (priv->output_dev,
				    page,
				    72.0, 72.0,
				    0,
				    gTrue, gFalse,
				    NULL, NULL, 
				    NULL, NULL);
#endif

	priv->current_page = page;

	if (priv->links_layer) {
		g_object_unref (G_OBJECT (priv->links_layer));
		gtk_object_destroy (GTK_OBJECT (priv->links_layer));
		priv->links_layer = NULL;
	}

	priv->links_layer = GPDF_LINKS_CANVAS_LAYER (gnome_canvas_item_new (
		gnome_canvas_root (GNOME_CANVAS (gpdf_view)),
		GPDF_TYPE_LINKS_CANVAS_LAYER,
		"links",
		new Links (pdf_page->getAnnots (&obj),
			   pdf_doc->getCatalog ()->getBaseURI ()),
		NULL));
	g_object_ref (G_OBJECT (priv->links_layer));
	gtk_object_sink (GTK_OBJECT (priv->links_layer));

	obj.free ();

	gnome_canvas_item_affine_relative (
		GNOME_CANVAS_ITEM (priv->links_layer), page_transform);

	gnome_canvas_item_raise_to_top (GNOME_CANVAS_ITEM (priv->links_layer));

	g_signal_connect (G_OBJECT (priv->links_layer), "link_clicked",
			  G_CALLBACK (gpdf_view_link_clicked_cb),
			  gpdf_view);
	g_signal_connect (G_OBJECT (priv->links_layer), "link_entered",
			  G_CALLBACK (gpdf_view_link_entered_cb),
			  gpdf_view);
	g_signal_connect (G_OBJECT (priv->links_layer), "link_leaved",
			  G_CALLBACK (gpdf_view_link_leaved_cb),
			  gpdf_view);

	gpdf_control_private_reset_cursor (priv->parent);
	return;
}

static void
gpdf_view_emit_page_changed (GPdfView *gpdf_view, gint page)
{
	g_signal_emit (G_OBJECT (gpdf_view),
		       gpdf_view_signals [PAGE_CHANGED_SIGNAL],
		       0, page);
}

static void
gpdf_view_goto_page_no_history (GPdfView *gpdf_view, gint page)
{
	GPdfViewPrivate *priv;
	gint last_page;

	priv = gpdf_view->priv;
	last_page = priv->pdf_doc->getNumPages ();

	priv->requested_page = CLAMP (page, 1, last_page);
	gpdf_view_emit_page_changed (gpdf_view, priv->requested_page);
	gpdf_view_render_page (gpdf_view);
}

void
gpdf_view_goto_page (GPdfView *gpdf_view, gint page)
{
	g_return_if_fail (GPDF_IS_VIEW_WITH_DOC (gpdf_view));

	gpdf_view_history_stack_page (gpdf_view);
	gpdf_view_goto_page_no_history (gpdf_view, page);
}

void
gpdf_view_scroll_to_top (GPdfView *gpdf_view)
{
	GnomeCanvas *canvas;
	gint x;

	g_return_if_fail (GPDF_IS_NON_NULL_VIEW (gpdf_view));

	canvas = GNOME_CANVAS (gpdf_view);
	gnome_canvas_get_scroll_offsets (canvas, &x, NULL);

	gdk_window_set_back_pixmap (GTK_WIDGET (canvas)->window, NULL, FALSE);
	gdk_window_hide (canvas->layout.bin_window);

	gnome_canvas_scroll_to (canvas, x, 0);

	gdk_window_show (canvas->layout.bin_window);
	gtk_style_set_background (GTK_WIDGET (canvas)->style, GTK_WIDGET (canvas)->window, GTK_STATE_NORMAL);
}

void
gpdf_view_scroll_to_bottom (GPdfView *gpdf_view)
{
	GnomeCanvas *canvas;
	gint x, y;

	g_return_if_fail (GPDF_IS_NON_NULL_VIEW (gpdf_view));

	canvas = GNOME_CANVAS (gpdf_view);
	gnome_canvas_get_scroll_offsets (canvas, &x, NULL);

	y = canvas->layout.height - GTK_WIDGET (gpdf_view)->allocation.height;

	gdk_window_set_back_pixmap (GTK_WIDGET (canvas)->window, NULL, FALSE);
	gdk_window_hide (canvas->layout.bin_window);

	gnome_canvas_scroll_to (canvas, x, y);

	gdk_window_show (canvas->layout.bin_window);
	gtk_style_set_background (GTK_WIDGET (canvas)->style, GTK_WIDGET (canvas)->window, GTK_STATE_NORMAL);
}

gint
gpdf_view_get_current_page (GPdfView *gpdf_view)
{
	GPdfViewPrivate *priv;
	
	g_return_val_if_fail (GPDF_IS_VIEW_WITH_DOC (gpdf_view), 0);
	priv = gpdf_view->priv;
	return priv->requested_page != priv->current_page ?
	  priv->requested_page : priv->current_page; 
}

void
gpdf_view_page_prev (GPdfView *gpdf_view)
{
	g_return_if_fail (GPDF_IS_VIEW_WITH_DOC (gpdf_view));

	gpdf_view_goto_page (gpdf_view, gpdf_view->priv->requested_page - 1);
	gpdf_view_scroll_to_top (gpdf_view);
}

void
gpdf_view_page_next (GPdfView *gpdf_view)
{
	g_return_if_fail(GPDF_IS_VIEW_WITH_DOC (gpdf_view));

	gpdf_view_goto_page (gpdf_view, gpdf_view->priv->requested_page + 1);
	gpdf_view_scroll_to_top (gpdf_view);
}

void
gpdf_view_page_first (GPdfView *gpdf_view)
{
	g_return_if_fail (GPDF_IS_VIEW_WITH_DOC (gpdf_view));

	gpdf_view_goto_page (gpdf_view, 1);
	gpdf_view_scroll_to_top (gpdf_view);
}

void
gpdf_view_page_last (GPdfView *gpdf_view)
{
	g_return_if_fail (GPDF_IS_VIEW_WITH_DOC (gpdf_view));

	gpdf_view_goto_page (gpdf_view,
			     gpdf_view->priv->pdf_doc->getNumPages ());
	gpdf_view_scroll_to_top (gpdf_view);
}

gboolean
gpdf_view_is_first_page (GPdfView *gpdf_view)
{
	g_return_val_if_fail (GPDF_IS_VIEW_WITH_DOC (gpdf_view), FALSE);

	return gpdf_view->priv->requested_page == 1;
}

gboolean
gpdf_view_is_last_page (GPdfView *gpdf_view)
{
	gint last_page;

	g_return_val_if_fail (GPDF_IS_VIEW_WITH_DOC (gpdf_view), FALSE);

	last_page = gpdf_view->priv->pdf_doc->getNumPages ();

	return gpdf_view->priv->requested_page == last_page;
}

void
gpdf_view_back_history (GPdfView *gpdf_view)
{
	GPdfViewPrivate *priv;
	GPdfViewHistory *hnode;
	int target_page = 0;
	double target_zoom = (double)0.0;
	double target_hval, target_vval; 

	g_return_if_fail (GPDF_IS_VIEW_WITH_DOC (gpdf_view));
	priv = gpdf_view->priv;

	g_return_if_fail (priv->backward_history != (G_List *)NULL);
	g_return_if_fail (priv->backward_history->data != (gpointer)NULL);

	hnode = (GPdfViewHistory *)priv->backward_history->data;
	target_page = hnode->page;
	target_zoom = hnode->zoom;
	target_hval = hnode->hval; 
	target_vval = hnode->vval; 
	priv->backward_history = g_list_remove (priv->backward_history,
						(gpointer)hnode);
	if (!g_list_length (priv->backward_history)) {
		g_list_free (priv->backward_history);
		priv->backward_history = (G_List *)NULL;
	}
	hnode->page = priv->current_page;
	hnode->zoom = priv->zoom;
	hnode->hval = gtk_adjustment_get_value(GTK_LAYOUT (gpdf_view)->hadjustment); 
	hnode->vval = gtk_adjustment_get_value(GTK_LAYOUT (gpdf_view)->vadjustment); 
	priv->forward_history = g_list_prepend (priv->forward_history, hnode);

	gpdf_view_goto_page_no_history (gpdf_view, target_page);
	gpdf_view_zoom (gpdf_view, target_zoom, FALSE); 
	gtk_adjustment_set_value(GTK_LAYOUT (gpdf_view)->hadjustment, target_hval);
	gtk_adjustment_set_value(GTK_LAYOUT (gpdf_view)->vadjustment, target_vval);
}

void
gpdf_view_forward_history (GPdfView *gpdf_view)
{
	GPdfViewPrivate *priv;
	GPdfViewHistory *hnode;
	int target_page = 0;
	double target_zoom = (double)0.0;
	double target_hval, target_vval; 

	g_return_if_fail (GPDF_IS_VIEW_WITH_DOC (gpdf_view));
	priv = gpdf_view->priv;

	g_return_if_fail (priv->forward_history != (G_List *)NULL);
	g_return_if_fail (priv->forward_history->data != (gpointer)NULL);
	hnode = (GPdfViewHistory *)priv->forward_history->data;
	target_page = hnode->page;
	target_zoom = hnode->zoom;
	target_hval = hnode->hval; 
	target_vval = hnode->vval; 
	priv->forward_history = g_list_remove (priv->forward_history,
					       (gpointer)hnode);
	if (!g_list_length (priv->forward_history)) {
		g_list_free (priv->forward_history);
		priv->forward_history = (G_List *)NULL;
	}
	hnode->page = priv->current_page;
	hnode->zoom = priv->zoom;
	hnode->hval = gtk_adjustment_get_value(GTK_LAYOUT (gpdf_view)->hadjustment); 
	hnode->vval = gtk_adjustment_get_value(GTK_LAYOUT (gpdf_view)->vadjustment); 
	priv->backward_history = g_list_prepend (priv->backward_history, hnode);

	gpdf_view_goto_page_no_history (gpdf_view, target_page);
	gpdf_view_zoom (gpdf_view, target_zoom, FALSE); 
	gtk_adjustment_set_value(GTK_LAYOUT (gpdf_view)->hadjustment, target_hval);
	gtk_adjustment_set_value(GTK_LAYOUT (gpdf_view)->vadjustment, target_vval);
}

gboolean
gpdf_view_is_first_history(GPdfView *gpdf_view)
{
	GPdfViewPrivate *priv;

	g_return_val_if_fail (GPDF_IS_VIEW_WITH_DOC (gpdf_view), TRUE);
	priv = gpdf_view->priv;

	return (priv->backward_history == (G_List *)NULL);
}

gboolean
gpdf_view_is_last_history (GPdfView *gpdf_view)
{
	GPdfViewPrivate *priv;

	g_return_val_if_fail (GPDF_IS_VIEW_WITH_DOC (gpdf_view), TRUE);
	priv = gpdf_view->priv;

	return (priv->forward_history == (G_List *)NULL);
}

static int
gpdf_view_get_page_count (GgvDocument *document)
{
	g_return_val_if_fail (GPDF_IS_VIEW_WITH_DOC (document), -1);

	return GPDF_VIEW (document)->priv->pdf_doc->getNumPages ();
}

static char **
gpdf_view_get_page_names (GgvDocument *document)
{
	int page_count;
	int i;
	char **result;

	g_return_val_if_fail (GPDF_IS_VIEW_WITH_DOC (document), NULL);

	page_count = gpdf_view_get_page_count (document);

	result = g_new0 (char *, page_count + 1);
	for (i = 0; i < page_count; ++i)
		result [i] = g_strdup_printf ("%d", i+1);

	return result;
}


#define MAGSTEP  1.2
#define IMAGSTEP 0.8333333333333

#define MIN_ZOOM 0.05409
#define MAX_ZOOM 18.4884

#define ZOOM_IN_FACTOR  MAGSTEP
#define ZOOM_OUT_FACTOR IMAGSTEP

static void
gpdf_view_emit_zoom_changed (GPdfView *gpdf_view, gdouble zoom)
{
	g_signal_emit (G_OBJECT (gpdf_view),
		       gpdf_view_signals [ZOOM_CHANGED_SIGNAL],
		       0, zoom);
}

void
gpdf_view_zoom (GPdfView *gpdf_view, gdouble factor, gboolean relative)
{
	GPdfViewPrivate *priv;
	gdouble zoom;

	g_return_if_fail (GPDF_IS_VIEW_WITH_DOC (gpdf_view));

	priv = gpdf_view->priv;

	if (relative) {
		zoom = priv->zoom * factor;
	} else {
		zoom = factor;
	}

	priv->zoom = CLAMP (zoom, MIN_ZOOM, MAX_ZOOM);

	gdk_window_set_back_pixmap (GTK_WIDGET (gpdf_view)->window, NULL,
				    FALSE);
 	gdk_window_hide (GNOME_CANVAS (gpdf_view)->layout.bin_window);

	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (gpdf_view),
					  priv->zoom);

 	gdk_window_show (GNOME_CANVAS (gpdf_view)->layout.bin_window);
	gtk_style_set_background (GTK_WIDGET (gpdf_view)->style,
				  GTK_WIDGET (gpdf_view)->window,
				  GTK_STATE_NORMAL);

	gpdf_view_emit_zoom_changed (gpdf_view, priv->zoom);
}

void
gpdf_view_zoom_default (GPdfView *gpdf_view)
{
	gpdf_view_zoom (gpdf_view, 1.0, FALSE);
}

void
gpdf_view_zoom_in (GPdfView *gpdf_view)
{
	gpdf_view_zoom (gpdf_view, ZOOM_IN_FACTOR, TRUE);
}

void
gpdf_view_zoom_out (GPdfView *gpdf_view)
{
	gpdf_view_zoom (gpdf_view, ZOOM_OUT_FACTOR, TRUE);
}

#define DOUBLE_EQUAL(a,b) (fabs (a - b) < 1e-6)
void
gpdf_view_zoom_fit (GPdfView *gpdf_view, gint width, gint height)
{
	gdouble x1, y1, x2, y2;
	gdouble page_w, page_h, zoom_w, zoom_h;

	g_return_if_fail (GPDF_IS_VIEW_WITH_DOC (gpdf_view));

	gnome_canvas_get_scroll_region (GNOME_CANVAS (gpdf_view),
					&x1, &y1, &x2, &y2);

	page_w = x2 - x1;
	page_h = y2 - y1;

	if (DOUBLE_EQUAL (page_w, 0.0) || DOUBLE_EQUAL (page_h, 0.0))
		return;

	zoom_w = width / page_w;
	zoom_h = height / page_h;

	gpdf_view_zoom (gpdf_view, (zoom_w < zoom_h) ? zoom_w : zoom_h, FALSE);
}

void
gpdf_view_zoom_fit_width (GPdfView *gpdf_view, gint width)
{
	gdouble x1, y1, x2, y2;
	gdouble page_w;

	g_return_if_fail (GPDF_IS_VIEW_WITH_DOC (gpdf_view));

	gnome_canvas_get_scroll_region (GNOME_CANVAS (gpdf_view),
					&x1, &y1, &x2, &y2);
	page_w = x2 - x1 + 0.5;
	if (DOUBLE_EQUAL (page_w, 0.0))
		return;

	gpdf_view_zoom (gpdf_view, width / page_w, FALSE);
}

void
gpdf_view_thumbnail_selected (GPdfView   *gpdf_view,
			      gint x, gint y,
			      gint w, gint h,
			      gint page)
{
	gpdf_view_goto_page(gpdf_view, page);
}

void
gpdf_view_annotation_selected(GPdfView *gpdf_view,
			      Annot    *annot, 
			      gint      page)
{
	gpdf_view_goto_page (gpdf_view, page);
}

void
gpdf_view_annotation_toggled(GPdfView   *gpdf_view,
			     Annot      *annot, 
			     gint        page,
			     gboolean	 active)
{
	if (gpdf_view->priv->current_page != page)
	  gpdf_view_goto_page (gpdf_view, page);
	else {
		gpdf_view->priv->requested_page = page; 
		gpdf_view->priv->current_page = 0; 
		gpdf_view_render_page (gpdf_view);
	}
}

void
gpdf_view_bookmark_selected (GPdfView   *gpdf_view,
			     LinkAction *link_action)
{
	g_return_if_fail (GPDF_IS_VIEW (gpdf_view));

	/* Title have null link_action: goto page 1 */
	if (!link_action)
		gpdf_view_goto_page (gpdf_view, 1);
	else
		switch (link_action->getKind ()) {
		case actionGoTo:
			gpdf_view_link_action_goto (
				gpdf_view, dynamic_cast <LinkGoTo *> (link_action));
			break;
		case actionNamed:
			gpdf_view_link_action_named (
				gpdf_view, dynamic_cast <LinkNamed *> (link_action));
			break;
		 case actionURI:
			gpdf_view_link_action_uri (
				gpdf_view, dynamic_cast <LinkURI *> (link_action));
			break;

		default:
			g_message ("Bookmark selection: unimplemented Action");
			break;
		}
}


static gboolean
gpdf_view_key_press_event (GtkWidget *widget, GdkEventKey *event)
{
	GPdfView *gpdf_view;
	gboolean big_step;
	gboolean do_scroll;
	gint height, width;
	gint x, y;

	g_return_val_if_fail (GPDF_IS_NON_NULL_VIEW (widget), FALSE);

	gpdf_view = GPDF_VIEW (widget);
	gnome_canvas_get_scroll_offsets (GNOME_CANVAS (gpdf_view), &x, &y);
	do_scroll = FALSE;
	height = widget->allocation.height;
	width = widget->allocation.width;
	big_step = event->state & GDK_SHIFT_MASK;

	switch (event->keyval) {
	case GDK_plus:
	case GDK_KP_Add:
	case GDK_equal:
		gpdf_view_zoom_in (gpdf_view);
		break;
	case GDK_minus:
	case GDK_KP_Subtract:
	case GDK_underscore:
		gpdf_view_zoom_out (gpdf_view);
		break;
	case GDK_KP_Right:
	case GDK_Right:
		do_scroll = TRUE;
		if (big_step)
			x += width;
		else
			x += 10;
		break;
	case GDK_KP_Left:
	case GDK_Left:
		do_scroll = TRUE;
		if (big_step)
			x -= width;
		else
			x -= 10;
		break;
	case GDK_KP_Up:
	case GDK_Up:
		if (big_step) {
			goto prev_screen;
		} else {
			do_scroll = TRUE;
			y -= 10;
		}
		break;
	case GDK_KP_Down:
	case GDK_Down:
		if (big_step) {
			goto next_screen;
		} else {
			do_scroll = TRUE;
			y += 10;
		}
		break;
	case GDK_KP_Page_Up:
	case GDK_Page_Up:
		if (gpdf_view->priv->current_page > 1)
			gpdf_view_page_prev (gpdf_view);
		break;
	case GDK_Delete:
	case GDK_KP_Delete:
	case GDK_BackSpace:
	prev_screen:
		if (y <= 0) {
			if (gpdf_view->priv->current_page <= 1)
				break;

			gpdf_view_goto_page (
				gpdf_view,
				gpdf_view->priv->requested_page - 1);
			gpdf_view_scroll_to_bottom (gpdf_view);
		} else {
			do_scroll = TRUE;
			y -= height;
		}
		break;
	case GDK_KP_Page_Down:
	case GDK_Page_Down:
		if (gpdf_view->priv->current_page <
		    gpdf_view->priv->pdf_doc->getNumPages ())
			gpdf_view_page_next (gpdf_view);
		break;
	case GDK_space:
	next_screen:
		if (y >= (gint)(GTK_LAYOUT (widget)->height) - height) {
			if (gpdf_view->priv->current_page >=
			    gpdf_view->priv->pdf_doc->getNumPages ())
				break;
			
			gpdf_view_goto_page (
				gpdf_view,
				gpdf_view->priv->requested_page + 1);
			gpdf_view_scroll_to_top (gpdf_view);
		} else {
			do_scroll = TRUE;
			y += height;
		}
		break;
	default:
		return FALSE;
	}

	if (do_scroll)
		gnome_canvas_scroll_to (GNOME_CANVAS (widget), x, y);

	return TRUE;
}

static gboolean
gpdf_view_button_press_event (GtkWidget *widget, GdkEventButton *event)
{
	GPdfViewPrivate *priv;

	g_return_val_if_fail (GPDF_IS_NON_NULL_VIEW (widget), FALSE);

	priv = GPDF_VIEW (widget)->priv;

	if (GNOME_CALL_PARENT_WITH_DEFAULT (GTK_WIDGET_CLASS,
					    button_press_event,
					    (widget, event), FALSE) == TRUE) {
		return TRUE;
	}

	if (!GTK_WIDGET_HAS_FOCUS (widget))
		gtk_widget_grab_focus (widget);

	if (priv->dragging)
		return FALSE;

	switch (event->button) {
	case 1:
		GdkCursor *cursor;

		priv->dragging = TRUE;

		gnome_canvas_get_scroll_offsets (GNOME_CANVAS (widget),
						 &priv->drag_offset_x,
						 &priv->drag_offset_y);

		priv->drag_anchor_x = (gint)(event->x - priv->drag_offset_x);
		priv->drag_anchor_y = (gint)(event->y - priv->drag_offset_y);

		cursor = gdk_cursor_new (GDK_FLEUR);
		gdk_pointer_grab (widget->window, FALSE,
				  (GdkEventMask)
				  (GDK_POINTER_MOTION_MASK |
				   GDK_POINTER_MOTION_HINT_MASK |
				   GDK_BUTTON_RELEASE_MASK),
				  NULL, cursor, event->time);
		gdk_cursor_unref (cursor);
		return TRUE;
		
	default:
		return FALSE;
	}
}

static gboolean
gpdf_view_button_release_event (GtkWidget *widget, GdkEventButton *event)
{
	GPdfViewPrivate *priv;

	g_return_val_if_fail (GPDF_IS_NON_NULL_VIEW (widget), FALSE);

	priv = GPDF_VIEW (widget)->priv;

	if (!priv->dragging || event->button != 1)
		return FALSE;

	priv->dragging = FALSE;
	gdk_pointer_ungrab (event->time);

	return TRUE;
}

static void
gpdf_view_handle_drag_motion (GPdfView *gpdf_view, GdkEventMotion *event)
{
	GPdfViewPrivate *priv;
	gint x, y, dx, dy;
	GdkModifierType mod;

	g_return_if_fail (GPDF_IS_NON_NULL_VIEW (gpdf_view));

	priv = gpdf_view->priv;

	gnome_canvas_get_scroll_offsets (GNOME_CANVAS (gpdf_view),
			&priv->drag_offset_x, &priv->drag_offset_y);

	if (event->is_hint) {
		gdk_window_get_pointer (GTK_WIDGET (gpdf_view)->window,
					&x, &y, &mod);
	} else {
		/* subtract drag_offset because event->x is relative to backing window
		 * of the canvas and not the clipping window of the widget */
		x = (gint)event->x - priv->drag_offset_x;
		y = (gint)event->y - priv->drag_offset_y;
	}

	dx = priv->drag_anchor_x - x;
	dy = priv->drag_anchor_y - y;

	gnome_canvas_scroll_to (GNOME_CANVAS (gpdf_view),
				priv->drag_offset_x + dx,
				priv->drag_offset_y + dy);

	/* reset the anchor to the new position of the mouse */
	priv->drag_anchor_x = x;
	priv->drag_anchor_y = y;

}

static gboolean
gpdf_view_motion_notify_event (GtkWidget *widget, GdkEventMotion *event)
{

	g_return_val_if_fail (GPDF_IS_NON_NULL_VIEW (widget), FALSE);

	if (GPDF_VIEW (widget)->priv->dragging)
		gpdf_view_handle_drag_motion (GPDF_VIEW (widget), event);

	return GNOME_CALL_PARENT_WITH_DEFAULT (
		GTK_WIDGET_CLASS, motion_notify_event, (widget, event), TRUE);
}

static void
gesture_page_next_event_cb (GtkGestureHandler *gh,
			    GPdfView *gpdf_view, gpointer p)
{
	gpdf_view_page_next (gpdf_view);
}

static void
gesture_page_prev_event_cb (GtkGestureHandler *gh,
			    GPdfView *gpdf_view, gpointer p)
{
	gpdf_view_page_prev (gpdf_view);
}

static void
gesture_page_first_event_cb (GtkGestureHandler *gh,
			     GPdfView *gpdf_view, gpointer p)
{
	gpdf_view_page_first (gpdf_view);
}

static void
gesture_page_last_event_cb (GtkGestureHandler *gh,
			    GPdfView *gpdf_view, gpointer p)
{
	gpdf_view_page_last (gpdf_view);
}

static void
gesture_zoom_in_event_cb (GtkGestureHandler *gh,
			  GPdfView *gpdf_view, gpointer p)
{
	gpdf_view_zoom_in (gpdf_view);
}

static void
gesture_zoom_out_event_cb (GtkGestureHandler *gh,
			   GPdfView *gpdf_view, gpointer p)
{
	gpdf_view_zoom_out (gpdf_view);
}


void
gpdf_view_set_pdf_doc (GPdfView *gpdf_view, PDFDoc *pdf_doc)
{
	GPdfViewPrivate *priv;

	g_return_if_fail (GPDF_IS_NON_NULL_VIEW (gpdf_view));

	priv = gpdf_view->priv;

	gpdf_view_clear_history (gpdf_view);

	g_object_set (G_OBJECT (gpdf_view),
		      "pdf_doc", pdf_doc,
		      NULL);

	priv->current_page = 0;
	priv->output_dev->startDoc (pdf_doc->getXRef());
	gpdf_view_page_background_show (gpdf_view);
	gpdf_view_goto_page_no_history (gpdf_view, 1);
}

/* Expects pathname in on-disk encoding (like Xpdf's saveAs) */
gboolean
gpdf_view_save_as (GPdfView *gpdf_view, const gchar *pathname)
{
	GPdfViewPrivate *priv; 
	GString *filename = new GString(pathname); 
	gboolean retval = FALSE;

	g_return_val_if_fail (GPDF_IS_NON_NULL_VIEW (gpdf_view), FALSE);

	priv = gpdf_view->priv;

	gpdf_control_private_set_wait_cursor (priv->parent);
	
	retval = priv->pdf_doc->saveAs (filename);

	gpdf_control_private_reset_cursor (priv->parent);

	return retval;
}


static void
gpdf_view_dispose (GObject *object)
{
	GPdfViewPrivate *priv;

	g_return_if_fail (GPDF_IS_NON_NULL_VIEW (object));

	priv = GPDF_VIEW (object)->priv;

	if (priv->output_dev) {
		delete priv->output_dev;
		priv->output_dev = NULL;
	}

	if (priv->print_context) {
		g_object_unref (priv->print_context);
		priv->print_context = NULL;
	}

	if (priv->links_layer) {
		g_object_unref (priv->links_layer);
		priv->links_layer = NULL;
	}

	if (priv->gesture_handler) {
		gtk_gesture_handler_destroy (priv->gesture_handler);
		priv->gesture_handler = NULL;
	}
#ifdef USE_ANNOTS_VIEW
	if (priv->annots_view) {
		gtk_widget_unref (GTK_WIDGET (priv->annots_view));
		priv->annots_view = NULL; 
	}
#endif

	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
gpdf_view_finalize (GObject *object)
{
	GPdfView *gpdf_view;

	g_return_if_fail (GPDF_IS_NON_NULL_VIEW (object));

	gpdf_view = GPDF_VIEW (object);

	if (gpdf_view->priv) {
		g_free (gpdf_view->priv);
		gpdf_view->priv = NULL;
	}

	GNOME_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

static void
gpdf_view_get_property (GObject *object, guint param_id,
                        GValue *value, GParamSpec *pspec)
{
	GPdfView *gpdf_view;

	g_return_if_fail (GPDF_IS_VIEW (object));

	gpdf_view = GPDF_VIEW (object);

	switch (param_id)
	{
	      case PROP_PDFDOC:
		g_value_set_pointer (value,
				     (gpointer)gpdf_view->priv->pdf_doc);
		break;
	      default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
gpdf_view_set_property (GObject *object, guint param_id,
			const GValue *value, GParamSpec *pspec)
{
	GPdfView *gpdf_view;

	g_return_if_fail (GPDF_IS_VIEW (object));

	gpdf_view = GPDF_VIEW (object);

	switch (param_id)
{
	      case PROP_CONTROL:
		gpdf_view->priv->parent =
		  GPDF_CONTROL (g_value_get_object (value));
		break;
	      case PROP_PDFDOC:
		/*
		 * All PDFDocs in GPdf code are owned by a GPdfPersistStream,
		 * so don't delete our old pdf_doc
		 */
		gpdf_view->priv->pdf_doc =
		  (PDFDoc *)(g_value_get_pointer (value));
		break;
	      default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
gpdf_view_setup_at (GPdfView *gpdf_view)
{
	AtkObject *accessible;

	accessible = gtk_widget_get_accessible (GTK_WIDGET (gpdf_view));
	atk_object_set_role (accessible, ATK_ROLE_DRAWING_AREA);
	atk_object_set_name (accessible, _("Document View"));
}

static void
gpdf_view_setup_gesture_handler (GPdfView *gpdf_view)
{
	GtkGestureHandler *gh;

	g_return_if_fail (GPDF_IS_NON_NULL_VIEW (gpdf_view));

	gh = gtk_gesture_handler_new (GTK_WIDGET (gpdf_view));
	gpdf_view->priv->gesture_handler = gh;

	gtk_gesture_add_callback (
		gh, "012",
		GTK_GESTURE_FUNC (gesture_page_next_event_cb),
		gpdf_view, NULL);
	gtk_gesture_add_callback (
		gh, "210",
		GTK_GESTURE_FUNC (gesture_page_prev_event_cb),
		gpdf_view, NULL);
	gtk_gesture_add_callback (
		gh, "630",
		GTK_GESTURE_FUNC (gesture_page_first_event_cb),
		gpdf_view, NULL);
	gtk_gesture_add_callback (
		gh, "036",
		GTK_GESTURE_FUNC (gesture_page_last_event_cb),
		gpdf_view, NULL);

	gtk_gesture_add_callback (
		gh, "048",
		GTK_GESTURE_FUNC (gesture_zoom_in_event_cb),
		gpdf_view, NULL);
	gtk_gesture_add_callback (
		gh, "840",
		GTK_GESTURE_FUNC (gesture_zoom_out_event_cb),
		gpdf_view, NULL);
}

static void
gpdf_view_setup_page_background (GPdfView *gpdf_view)
{
	GPdfViewPrivate *priv;

	g_return_if_fail (GPDF_IS_NON_NULL_VIEW (gpdf_view));

	priv = gpdf_view->priv;

	priv->page_background = gnome_canvas_item_new (
		gnome_canvas_root (GNOME_CANVAS (gpdf_view)),
		GNOME_TYPE_CANVAS_RECT,
		"x1", 0.0, "y1", 0.0, "x2", 72.0, "y2", 72.0,
		"fill_color", "white",
		"outline_color", "black",
		"width_pixels", 1, NULL);

	gnome_canvas_item_lower_to_bottom (priv->page_background);

	priv->page_shadow = gnome_canvas_item_new (
		gnome_canvas_root (GNOME_CANVAS (gpdf_view)),
		GNOME_TYPE_CANVAS_RECT,
		"x1", (gdouble)PAGE_PAD -1.0,
		"y1", (gdouble)PAGE_PAD -1.0,
		"x2", 72.0 + (gdouble)PAGE_PAD -1.0,
		"y2", 72.0 + (gdouble)PAGE_PAD -1.0,
		"fill_color", "black", NULL);

	gnome_canvas_item_lower_to_bottom (priv->page_shadow);

	gpdf_view_page_background_hide (gpdf_view);
}

static GObject *
gpdf_view_constructor (GType type,
		       guint n_construct_properties,
		       GObjectConstructParam *construct_params)
{
	GObject *object;
	GPdfView *gpdf_view;
	GPdfViewPrivate *priv;

	object = G_OBJECT_CLASS (parent_class)->constructor (
		type, n_construct_properties, construct_params);

	gpdf_view = GPDF_VIEW (object);
	priv = gpdf_view->priv;

	g_return_val_if_fail (GPDF_IS_NON_NULL_VIEW (gpdf_view), NULL);

	gpdf_view_setup_page_background (gpdf_view);
	gpdf_view_setup_gesture_handler (gpdf_view);
	gpdf_view_setup_at (gpdf_view);

	priv = gpdf_view->priv;
	priv->output_dev = new GPOutputDev ();
	priv->zoom = 1.0;

#ifdef USE_ANNOTS_VIEW
	priv->annots_cb = gpdf_annots_view_is_annot_displayed;
#endif
	
	return object;
}

static void
gpdf_view_class_init (GPdfViewClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->constructor = gpdf_view_constructor;
	object_class->dispose = gpdf_view_dispose;
	object_class->finalize = gpdf_view_finalize;
	object_class->set_property = gpdf_view_set_property;
	object_class->get_property = gpdf_view_get_property;

	widget_class->button_press_event = gpdf_view_button_press_event;
	widget_class->button_release_event = gpdf_view_button_release_event;
	widget_class->motion_notify_event = gpdf_view_motion_notify_event;
	widget_class->key_press_event = gpdf_view_key_press_event;

	g_object_class_install_property (
		object_class, PROP_CONTROL,
		g_param_spec_object (
			"control",
			"Parent control",
			"Parent control",
			GPDF_TYPE_CONTROL, 
		        (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE)));

	g_object_class_install_property (
		object_class, PROP_PDFDOC,
		g_param_spec_pointer (
			"pdf_doc",
			"PDF Document",
			"PDF Document",
		        (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));

	gpdf_view_signals [ZOOM_CHANGED_SIGNAL] = g_signal_new (
		"zoom_changed",
		G_TYPE_FROM_CLASS (object_class),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (GPdfViewClass, zoom_changed),
		NULL, NULL,
		gpdf_marshal_VOID__DOUBLE,
		G_TYPE_NONE, 1, G_TYPE_DOUBLE);

	gpdf_view_signals [PAGE_CHANGED_SIGNAL] = g_signal_new (
		"page_changed",
		G_TYPE_FROM_CLASS (object_class),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (GPdfViewClass, page_changed),
		NULL, NULL,
		gpdf_marshal_VOID__INT,
		G_TYPE_NONE, 1, G_TYPE_INT);

	gpdf_view_signals [CLOSE_REQUESTED_SIGNAL] = g_signal_new (
		"close_requested",
		G_TYPE_FROM_CLASS (object_class),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (GPdfViewClass, close_requested),
		NULL, NULL,
		gpdf_marshal_VOID__VOID,
		G_TYPE_NONE, 0);

	gpdf_view_signals [QUIT_REQUESTED_SIGNAL] = g_signal_new (
		"quit_requested",
		G_TYPE_FROM_CLASS (object_class),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (GPdfViewClass, quit_requested),
		NULL, NULL,
		gpdf_marshal_VOID__VOID,
		G_TYPE_NONE, 0);
}

static void
ggv_document_interface_init (GgvDocumentClass *klass, gpointer iface_data)
{
	klass->get_page_count = gpdf_view_get_page_count;
	klass->get_page_names = gpdf_view_get_page_names;
}

static void
gpdf_view_instance_init (GPdfView *gpdf_view)
{
	gpdf_view->priv = g_new0 (GPdfViewPrivate, 1);
}

static void
gpdf_view_class_init_trampoline (gpointer klass, gpointer data)
{
	parent_class = (GnomeCanvasClass *)g_type_class_ref (PARENT_TYPE);
	gpdf_view_class_init ((GPdfViewClass *)klass);
}

GType
gpdf_view_get_type (void)
{
	static GType object_type = 0;
	if (object_type == 0) {
		static const GTypeInfo object_info = {
		    sizeof (GPdfViewClass),
		    NULL,		/* base_init */
		    NULL,		/* base_finalize */
		    gpdf_view_class_init_trampoline,
		    NULL,		/* class_finalize */
		    NULL,               /* class_data */
		    sizeof (GPdfView),
		    0,                  /* n_preallocs */
		    (GInstanceInitFunc) gpdf_view_instance_init
		};

		static const GInterfaceInfo ggv_document_info = {
			(GInterfaceInitFunc) ggv_document_interface_init,
			NULL,
			NULL
		};

		object_type = g_type_register_static (PARENT_TYPE, "GPdfView",
						      &object_info,
						      (GTypeFlags)0);

		g_type_add_interface_static (object_type,
					     GGV_DOCUMENT_TYPE,
					     &ggv_document_info);
	}
	return object_type;
}

END_EXTERN_C
