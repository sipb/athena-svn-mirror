/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
   Copyright (C) 2000-2001 CodeFactory AB
   Copyright (C) 2000-2001 Jonas Borgström <jonas@codefactory.se>
   Copyright (C) 2000-2001 Anders Carlsson <andersca@codefactory.se>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "document/htmlfocusiterator.h"
#include "document/htmlparser.h"
#include "dom/core/dom-node.h"
#include "dom/core/dom-element.h"
#include "dom/views/dom-abstractview.h"
#include "dom/traversal/dom-traversal-utils.h"
#include "layout/html/htmlboxembedded.h"
#include "layout/htmlboxroot.h"
#include "layout/htmlboxfactory.h"
#include "layout/htmlbox.h"
#include "layout/htmlboxinline.h"
#include "layout/htmlboxtext.h"
#include "layout/htmlstyle.h"
#include "graphics/htmlgdkpainter.h"
#include "util/htmlmarshal.h"
#include "htmlevent.h"
#include "htmlview.h"
#include "config.h"

#ifdef ENABLE_ACCESSIBILITY
#include "layout/htmlboxtable.h"
#include "a11y/htmlviewaccessiblefactory.h"
#include "a11y/htmlaccessiblefactory.h"
#include "a11y/htmlboxblockaccessible.h"
#include "a11y/htmlboxembeddedaccessible.h"
#include "a11y/htmlboxaccessible.h"
#include "a11y/htmlboxtableaccessible.h"
#include "a11y/htmlboxtextaccessible.h"
#endif

enum {
	MOVE_CURSOR,
	REQUEST_OBJECT,
	ON_URL,
	ACTIVATE,
	MOVE_FOCUS_OUT,
	LAST_SIGNAL
};

static guint view_signals [LAST_SIGNAL] = { 0 };

#define RELAYOUT_TIMEOUT_INTERVAL 1000

static GtkLayoutClass *parent_class = NULL;

static gboolean html_view_remove_layout_box (HtmlView *view, DomNode *node);
static void html_view_try_jump (HtmlView *view);
static void html_view_set_saved_focus (HtmlView *view);
static DomElement* html_view_get_and_unset_saved_focus (HtmlView *view);
static void html_view_focus_element (HtmlView *view);

#ifdef ENABLE_ACCESSIBILITY
static AtkObject *  html_view_get_accessible (GtkWidget *widget);

HTML_ACCESSIBLE_FACTORY (HTML_TYPE_BOX_BLOCK_ACCESSIBLE, html_box_block_accessible)
HTML_ACCESSIBLE_FACTORY (HTML_TYPE_BOX_EMBEDDED_ACCESSIBLE, html_box_embedded_accessible)
HTML_ACCESSIBLE_FACTORY (HTML_TYPE_BOX_ACCESSIBLE, html_box_accessible)
HTML_ACCESSIBLE_FACTORY (HTML_TYPE_BOX_TABLE_ACCESSIBLE, html_box_table_accessible)
HTML_ACCESSIBLE_FACTORY (HTML_TYPE_BOX_TEXT_ACCESSIBLE, html_box_text_accessible)
#endif


static gboolean
set_adjustment_clamped (GtkAdjustment *adj, gdouble val)
{
  /* We don't really want to clamp to upper; we want to clamp to
     upper - page_size which is the highest value the scrollbar
     will let us reach. */
  if (val > (adj->upper - adj->page_size))
    val = adj->upper - adj->page_size;

  if (val < adj->lower)
    val = adj->lower;

  if (val != adj->value)
    {
      gtk_adjustment_set_value (adj, val);
      return TRUE;
    }
  else
    return FALSE;
}

void
html_view_scroll_to_node (HtmlView *view, DomNode *node, HtmlViewScrollToType type)
{
	HtmlBox *box;
	GtkAdjustment *adj = GTK_LAYOUT (view)->vadjustment;
	gint y;
	
	box = html_view_find_layout_box (view, node, FALSE);

	if (box == NULL) {
		return;
	}

	if (HTML_IS_BOX_INLINE (box) && box->children) {
		box = box->children;
	}

	y = html_box_get_absolute_y (box);

	if (!(adj->value < y && adj->value + adj->page_size > y)) {
		switch (type) {
		case HTML_VIEW_SCROLL_TO_TOP:
			set_adjustment_clamped (adj, y);
			break;
		case HTML_VIEW_SCROLL_TO_BOTTOM:
			/* FIXME: Handle box height better here */
			set_adjustment_clamped (adj, y - adj->page_size +  box->height);
			break;
		default:
			break;
		}
	}


#if 0
	g_print ("%d %d %d\n", (gint)adj->value, y, (gint)(adj->value + adj->page_size));
	
	if (adj->value < y || adj->value + adj->page_size > y) 
#endif
}

static void
html_view_set_adjustments (HtmlView *view)
{
	GtkLayout *layout;
	GtkAdjustment *vertical, *horizontal;

	if (view->root) {
		layout = GTK_LAYOUT (view);
		vertical = layout->vadjustment;
		horizontal = layout->hadjustment;
		
		vertical->lower = 0;
		vertical->upper = view->root->height; 
		vertical->page_size = (gfloat)GTK_WIDGET (view)->allocation.height;
		vertical->step_increment = GTK_WIDGET (view)->allocation.height / 10.0;
		vertical->page_increment = GTK_WIDGET (view)->allocation.height * 0.9;

		horizontal->lower = 0.0;
		horizontal->upper = view->root->width;
		horizontal->page_size = GTK_WIDGET (view)->allocation.width;
		horizontal->step_increment = GTK_WIDGET (view)->allocation.width / 10.0;
		horizontal->page_increment = GTK_WIDGET (view)->allocation.width * 0.9;

		gtk_layout_set_size (layout, horizontal->upper, vertical->upper);
		
		gtk_adjustment_changed (vertical);
		gtk_adjustment_changed (horizontal);
	}
}

static void
html_view_relayout (HtmlView *view)
{
	/* Tell the root layout box to relayout itself */
	if (view->painter && view->root) {
		HtmlRelayout *relayout;

		view->root->x = 0;
		view->root->y = 0;
		
		HTML_BOX_ROOT (view->root)->min_width = GTK_WIDGET (view)->allocation.width;
		HTML_BOX_ROOT (view->root)->min_height = GTK_WIDGET (view)->allocation.height;

		relayout = html_relayout_new ();
		relayout->type = HTML_RELAYOUT_INCREMENTAL;
		
		relayout->root = view->root;
		relayout->painter = view->painter;

		relayout->magnification = view->magnification;
		relayout->magnification_modified = view->magnification_modified;
		
		html_box_relayout (view->root, relayout);

		relayout->magnification_modified = FALSE;

		html_relayout_destroy (relayout);

		/* Set adjustments */
		html_view_set_adjustments (view);

		gtk_widget_queue_draw (GTK_WIDGET (view));

		if (view->jump_to_anchor)
			html_view_try_jump (view);
	}
	if (view->relayout_timeout_id != 0) {
		gtk_timeout_remove (view->relayout_timeout_id);
		view->relayout_timeout_id = 0;
	}
	if (view->relayout_idle_id != 0) {
		gtk_idle_remove (view->relayout_idle_id);
		view->relayout_idle_id = 0;
	}

	if (GTK_WIDGET_HAS_FOCUS (view)) {
		if (view->document->focus_element == NULL) {
			html_document_update_focus_element (view->document, html_focus_iterator_next_element (view->document->dom_document, NULL));
			html_view_focus_element (view);
		}
	}
}

static gint
relayout_timeout_callback (gpointer data)
{
	HtmlView *view = HTML_VIEW (data);

        html_view_relayout (view);
	
	view->relayout_timeout_id = 0;

	if (view->relayout_idle_id != 0) {
		gtk_idle_remove (view->relayout_idle_id);
		view->relayout_idle_id = 0;
	}
        return FALSE;

}

static gint
relayout_idle_callback (gpointer data)
{
        HtmlView *view = HTML_VIEW (data);

        html_view_relayout (view);

	view->relayout_idle_id = 0;

	if (view->relayout_timeout_id != 0) {
		gtk_timeout_remove (view->relayout_timeout_id);
		view->relayout_timeout_id = 0;
	}
        return FALSE;
}

static void
html_view_relayout_when_idle (HtmlView *view)
{
	if (view->relayout_idle_id == 0)
		view->relayout_idle_id = gtk_idle_add (relayout_idle_callback, view);
}

static void
html_view_relayout_after_timeout (HtmlView *view)
{
	if (view->relayout_timeout_id == 0)
		view->relayout_timeout_id = gtk_timeout_add (RELAYOUT_TIMEOUT_INTERVAL, relayout_timeout_callback, view);
}

static void
html_view_layout_tree_free (HtmlView *view, HtmlBox *root)
{
	HtmlBox *box;

	box = root;

	while (box) {
		HtmlBox *tmp_box;
		
		if (box->children)
			html_view_layout_tree_free (view, box->children);

		tmp_box = box;

		/* Remove the box from the table */
		html_view_remove_layout_box (view, tmp_box->dom_node);

		/* Check if we're trying to remove the root box */
		if (tmp_box == view->root)
			view->root = NULL;

		box = box->next;

		while (HTML_IS_BOX_TEXT (box) && !HTML_BOX_TEXT (box)->master)
			box = box->next;
		
		html_box_remove (tmp_box);
		g_object_unref (tmp_box);
	}
}


static void
html_view_paint (HtmlView *view, GdkRectangle *area)
{
	/* Tell the root layout box to repaint itself */
	if (view->painter && view->root) {
		/* Check that the document has not been deleted */
		if (view->root->dom_node) {
			html_box_paint (view->root, view->painter, area, 0, 0);
		}
	}
}

static void
html_view_add_layout_box (HtmlView *view, DomNode *node, HtmlBox *box)
{
	g_hash_table_insert (view->node_table, node, box);
}

static gboolean
html_view_remove_layout_box (HtmlView *view, DomNode *node)
{
	return g_hash_table_remove (view->node_table, node);
}

static void
html_view_insert_node (HtmlView *view, DomNode *node)
{
	HtmlBox *parent_box, *new_box;
	DomNode *n;
	
	if ((n = dom_Node__get_parentNode (node)))
		parent_box = html_view_find_layout_box (view, n, TRUE);
	else
		parent_box = NULL;

	/* Don't create a box if any of the parents have display: none */
	while (n) {
		if (n->style && n->style->display == HTML_DISPLAY_NONE)
			return;
		n = dom_Node__get_parentNode (n);
	}

	g_assert (node->style != NULL);

	new_box = html_box_factory_new_box (view, node);
	
	if (new_box) {

		new_box->dom_node = node;
		g_object_add_weak_pointer (G_OBJECT (node), (gpointer *) &(new_box->dom_node));

		html_box_handle_html_properties (new_box, node->xmlnode);
		
		if (parent_box == NULL) {
			if (!HTML_IS_BOX_ROOT (new_box))
				parent_box = view->root;
		}
		if (parent_box == NULL) {
			html_view_layout_tree_free (view, view->root);
			if (view->document && view->document->focus_element)
				html_document_update_focus_element (view->document, NULL);
			view->root = new_box;
		 } else {
			html_box_append_child (parent_box, new_box);

			/* Mark the boxes that need to be relayouted */
			html_box_set_unrelayouted_up (new_box);		
		}
		
		html_view_add_layout_box (view, node, new_box);
	}

}

static void
html_view_build_tree (HtmlView *view, DomNode *root)
{
	DomNode *node;
	
	node = root;

	for (node = root; node; node = dom_Node__get_nextSibling (node)) {
		html_view_insert_node (view, node);
		
		if (dom_Node_hasChildNodes (node)) 
			html_view_build_tree (view, dom_Node__get_firstChild (node));
	}
}

static void
html_view_inserted (HtmlDocument *document, DomNode *node, HtmlView *view)
{
	html_view_build_tree (view, node);

	if (document->state == HTML_DOCUMENT_STATE_PARSING) {
		html_view_relayout_after_timeout (view);
	}
	else {
		html_view_relayout_when_idle (view);
	}
}

static void
html_view_removed (HtmlDocument *document, DomNode *node, HtmlView *view)
{
	HtmlBox *box = html_view_find_layout_box (view, node, FALSE);

	/* Mark the boxes that need to be relayouted */
	if (box && box->parent)
		html_box_set_unrelayouted_up (box->parent);		

	if (box) {
		if (box->children)
			html_view_layout_tree_free (view, box->children);

		/* Remove the box from the table */
		html_view_remove_layout_box (view, box->dom_node);

		/* Check if we're trying to remove the root box */
		if (box == view->root)
			view->root = NULL;

		html_box_remove (box);
		g_object_unref (G_OBJECT (box));
	}
	
	html_view_relayout_when_idle (view);
}

static void
html_view_text_updated (HtmlDocument *document, DomNode *node, HtmlView *view)
{
	HtmlBox *box = html_view_find_layout_box (view, node, FALSE);

	if (box) {
		/* FIXME: perhaps use g_object_set here? */
		html_box_text_set_text (HTML_BOX_TEXT (box), node->xmlnode->content);
		html_box_set_unrelayouted_up (box);
	}
	else {
		g_error ("talk to the box factory here!");
	}

	html_view_relayout_when_idle (view);
}

static void
html_view_relayout_callback (HtmlDocument *document, DomNode *node, HtmlView *view)
{
	HtmlBox *box = html_view_find_layout_box (view, node, FALSE);

	/* Mark the boxes that need to be relayouted */
	if (box)
		html_box_set_unrelayouted_up (box);		

	
	if (document->state == HTML_DOCUMENT_STATE_PARSING) {
		html_view_relayout_after_timeout (view);
	}
	else {
		html_view_relayout_when_idle (view);
	}
}

static void
html_view_box_repaint_traverser (HtmlBox *box, gint *x, gint *y, gint *width, gint *height)
{
	*x = box->x;
	*y = box->y;

	/* Inline box special case */
	if (HTML_IS_BOX_INLINE (box)) {
		HtmlBox *child = box->children;
		gint x1 = G_MAXINT, y1 = G_MAXINT;
		gint x2 = *x, y2 = *y;

		while (child) {
			gint child_x, child_y, child_width, child_height;

			html_view_box_repaint_traverser (child, &child_x, &child_y, &child_width, &child_height);

			x1 = MIN (x1, *x + child_x);
			y1 = MIN (y1, *y + child_y);
			x2 = MAX (x2, *x + child_x + child_width);
			y2 = MAX (y2, *y + child_y + child_height);

			child = child->next;
		}
		*x = x1;
		*y = y1;
		*width = x2 - x1;
		*height = y2 - y1;
	}
	else {
		*width  = box->width;
		*height = box->height;
	}
}

static void
html_view_repaint_callback (HtmlDocument *document, DomNode *node, HtmlView *view)
{
	HtmlBox *box = html_view_find_layout_box (view, node, FALSE);
	gint x, y, width, height;

	g_return_if_fail (box != NULL);

	html_view_box_repaint_traverser (box, &x, &y, &width, &height);

	x += html_box_get_absolute_x (box);
	y += html_box_get_absolute_y (box);

	x -= (gint) (GTK_LAYOUT (view)->hadjustment->value);
	y -= (gint) (GTK_LAYOUT (view)->vadjustment->value);

	/* The + 3 is because we else might not paint underlines of text
	   with descent < 3 */
	gtk_widget_queue_draw_area (GTK_WIDGET (view), x, y, width, height + 3);
}

static void
html_view_style_updated (HtmlDocument *document, DomNode *node, HtmlStyleChange style_change, HtmlView *view)
{
	HtmlBox *box = html_view_find_layout_box (view, node, FALSE);
	DomNode *child_node;

	for (child_node = dom_Node__get_firstChild (node); child_node; child_node = dom_Node__get_nextSibling (child_node)) {
		html_view_style_updated (document, child_node, style_change, view);
	}

	if (!box)
		return;

	switch (style_change) {
	case HTML_STYLE_CHANGE_REPAINT:
		html_view_repaint_callback (document, node, view);
		break;
	case HTML_STYLE_CHANGE_RELAYOUT:
		html_view_relayout_callback (document, node, view);
		break;
	case HTML_STYLE_CHANGE_NONE:
		break;
	default:
		g_error ("fix me!");
	}
}

static void
html_view_setup_document (HtmlView *view)
{
	g_signal_connect (G_OBJECT (view->document),
			  "node_inserted",
			  G_CALLBACK (html_view_inserted),
			  view);

	g_signal_connect (G_OBJECT (view->document),
			   "node_removed",
			   (GCallback) html_view_removed,
			   view);

	g_signal_connect (G_OBJECT (view->document),
			  "text_updated",
			  (GCallback) html_view_text_updated,
			  view);
	
	g_signal_connect (G_OBJECT (view->document),
			  "style_updated",
			  G_CALLBACK (html_view_style_updated),
			  view);

	g_signal_connect (G_OBJECT (view->document),
			  "relayout_node",
			  G_CALLBACK (html_view_relayout_callback),
			  view);

	g_signal_connect (G_OBJECT (view->document),
			  "repaint_node",
			  G_CALLBACK (html_view_repaint_callback),
			  view);

	if (view->document->dom_document)
		html_view_build_tree (view, dom_Node_mkref (xmlDocGetRootElement ((xmlDoc *)DOM_NODE (view->document->dom_document)->xmlnode)));
}

static void
html_view_disconnect_document (HtmlView *view, HtmlDocument *document)
{
	g_signal_handlers_disconnect_matched (G_OBJECT (view->document),
					      G_SIGNAL_MATCH_FUNC,
					      0, 0, NULL,
					      html_view_inserted, NULL);

	g_signal_handlers_disconnect_matched (G_OBJECT (view->document),
					      G_SIGNAL_MATCH_FUNC,
					      0, 0, NULL,
					      html_view_removed, NULL);

	g_signal_handlers_disconnect_matched (G_OBJECT (view->document),
					      G_SIGNAL_MATCH_FUNC,
					      0, 0, NULL,
					      html_view_text_updated, NULL);

	g_signal_handlers_disconnect_matched (G_OBJECT (view->document),
					      G_SIGNAL_MATCH_FUNC,
					      0, 0, NULL,
					      html_view_style_updated, NULL);

	g_signal_handlers_disconnect_matched (G_OBJECT (view->document),
					      G_SIGNAL_MATCH_FUNC,
					      0, 0, NULL,
					      html_view_relayout_callback, 
					      NULL);

	g_signal_handlers_disconnect_matched (G_OBJECT (view->document),
					      G_SIGNAL_MATCH_FUNC,
					      0, 0, NULL,
					      html_view_repaint_callback, 
					      NULL);
}

static void
html_view_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	HtmlView *view = HTML_VIEW (widget);

	if (GTK_WIDGET_CLASS (parent_class)->size_allocate)
		( *GTK_WIDGET_CLASS (parent_class)->size_allocate) (widget, allocation);

	view = HTML_VIEW (widget);

	/* Relayout the view */
	html_view_relayout (view);
}

static void
html_view_realize (GtkWidget *widget)
{
	HtmlView *view;
	gfloat fsize;
	gint isize;

	view = HTML_VIEW (widget);

	/*
	 * GtkLayout uses the bg color for background but we want
	 * to use base color.
	 */	
	widget->style = gtk_style_copy (widget->style);
	widget->style->bg[GTK_STATE_NORMAL] = 
		widget->style->base[GTK_STATE_NORMAL];
	/*
	 * Store the font size so we can adjust size of HtmlFontSpecification
	 * if the size changes.
	 */
	fsize = pango_font_description_get_size (widget->style->font_desc) / 
			(gfloat) PANGO_SCALE;
	isize = (gint) fsize;
	g_object_set_data (G_OBJECT (widget), "html-view-font-size",
			   GINT_TO_POINTER (isize));
	
	if (GTK_WIDGET_CLASS (parent_class)->realize)
		(* GTK_WIDGET_CLASS (parent_class)->realize) (widget);
	
	gdk_window_set_events (GTK_LAYOUT (view)->bin_window,
			       gdk_window_get_events (GTK_LAYOUT (view)->bin_window) |
			       GDK_POINTER_MOTION_HINT_MASK | GDK_POINTER_MOTION_MASK |
			       GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
			       GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
			       GDK_KEY_PRESS_MASK |
			       GDK_EXPOSURE_MASK);
	
	/* Create the HtmlPainter */
	view->painter = html_gdk_painter_new ();

	html_gdk_painter_set_window (HTML_GDK_PAINTER (view->painter), GTK_LAYOUT (view)->bin_window);

	/* If we've got a document assigned, relayout the view */
	if (view->document)
		html_view_relayout_when_idle (view);
}


static gboolean
html_view_enter_notify (GtkWidget *widget, GdkEventCrossing *event)
{
	return TRUE;
}

static gboolean
html_view_leave_notify (GtkWidget *widget, GdkEventCrossing *event)
{
	/* Update the hover node to be NULL */
	html_document_update_hover_node (HTML_VIEW (widget)->document, NULL);
	
	return TRUE;
}

static gboolean
html_view_expose (GtkWidget *widget, GdkEventExpose *event)
{
	HtmlView *view;

	view = HTML_VIEW (widget);

	if (!GTK_WIDGET_DRAWABLE (widget) || (event->window != GTK_LAYOUT (view)->bin_window))
		return FALSE;

	html_view_paint (view, &event->area);
	return GTK_WIDGET_CLASS (parent_class)->expose_event (widget, event);
}


static gboolean
html_view_motion_notify (GtkWidget *widget, GdkEventMotion *event)
{
	gint x, y;
	GdkModifierType mask;
	
	if (event->window != GTK_LAYOUT (widget)->bin_window)
		return FALSE;

	gdk_window_get_pointer (widget->window, &x, &y, &mask);

	html_event_mouse_move (HTML_VIEW (widget), event);

	return FALSE;
}

static gboolean
html_view_button_press (GtkWidget *widget, GdkEventButton *event)
{
	if (!GTK_WIDGET_HAS_FOCUS (widget))
		gtk_widget_grab_focus (widget);
	
	if (event->window != GTK_LAYOUT (widget)->bin_window)
		return FALSE;

	html_event_button_press (HTML_VIEW (widget), event);
	
	return FALSE;
}

static gboolean
html_view_button_release (GtkWidget *widget, GdkEventButton *event)
{
	if (event->window != GTK_LAYOUT (widget)->bin_window)
		return FALSE;

	html_event_button_release (HTML_VIEW (widget), event);
	
	return FALSE;
}

static gboolean
html_view_focus_in (GtkWidget *widget, GdkEventFocus *event)
{
	HtmlView *view;
	DomElement *element;

	view = HTML_VIEW (widget);

	element = html_view_get_and_unset_saved_focus (view);
	if (element) {
		html_document_update_focus_element (view->document, element);
	}
	return GTK_WIDGET_CLASS (parent_class)->focus_in_event (widget, event);
}

static gboolean
html_view_focus_out (GtkWidget *widget, GdkEventFocus *event)
{
	HtmlView *view;

	view = HTML_VIEW (widget);

	if (view->document->focus_element) {
		html_view_set_saved_focus (view);
		if (GTK_CONTAINER (widget)->focus_child == NULL)
			html_document_update_focus_element (view->document, NULL);
	}
	return GTK_WIDGET_CLASS (parent_class)->focus_out_event (widget, event);
}

#if 0
static gboolean
html_view_key_press (GtkWidget *widget, GdkEventKey *event)
{

	return html_event_key_press (HTML_VIEW (widget), event);
}
#endif

static void
focus_element_destroyed (gpointer data)
{
	g_object_set_data (G_OBJECT (data), "saved-focus", NULL);
}

/* We must release all references to other objects here. But be careful as
   it can be called multiple times. */
static void
html_view_destroy (GtkObject *object)
{
	HtmlView *view = HTML_VIEW (object);

	gpointer saved_focus;

	if (view->relayout_timeout_id != 0) {
		gtk_timeout_remove (view->relayout_timeout_id);
		view->relayout_timeout_id = 0;
	}

	if (view->relayout_idle_id != 0) {
		gtk_idle_remove (view->relayout_idle_id);
		view->relayout_idle_id = 0;
	}

	saved_focus = g_object_get_data (G_OBJECT (view), "saved-focus");
	if (saved_focus) {
		g_object_weak_unref (G_OBJECT (saved_focus),
			   	     (GWeakNotify) focus_element_destroyed,
				     view);
		g_object_set_data (G_OBJECT (view), "saved-focus", NULL);
	}

	if (view->document) {
		g_object_unref (view->document);
		view->document = NULL;
	}

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
html_view_finalize (GObject *object)
{
	/*HtmlView *view = HTML_VIEW (object);*/

	if (G_OBJECT_CLASS (parent_class)->finalize)
		(G_OBJECT_CLASS (parent_class)->finalize) (object);
}

static gboolean
html_view_focus (GtkWidget *widget, GtkDirectionType direction)
{
	HtmlView *view = HTML_VIEW (widget);
	HtmlBox *box;
	DomElement *new_focus_element = NULL;

        if (view->document == NULL || view->document->dom_document == NULL) {
		return FALSE;
	}

	if (direction == GTK_DIR_TAB_FORWARD) {
		new_focus_element = html_focus_iterator_next_element (view->document->dom_document, view->document->focus_element);
		/* If end is reached cycle around */
		if (new_focus_element == NULL)
			new_focus_element = html_focus_iterator_next_element (view->document->dom_document, NULL);
	}
	else if (direction == GTK_DIR_TAB_BACKWARD) {
		new_focus_element = html_focus_iterator_prev_element (view->document->dom_document, view->document->focus_element);
		/* If start is reached cycle around */
		if (new_focus_element == NULL)
			new_focus_element = html_focus_iterator_prev_element (view->document->dom_document, NULL);
	}

	/* Handle special case when we haven't any focusable elements */
	if (new_focus_element == NULL) {
		if (!GTK_WIDGET_HAS_FOCUS (widget)) {
			gtk_widget_grab_focus (widget);
			return TRUE;
		} 
	} else { 
		html_document_update_focus_element (view->document, new_focus_element);

		html_view_focus_element (view);
		return TRUE;
	}

	return FALSE;
}

static void
html_view_set_focus_child (GtkContainer *container, GtkWidget *child)
{
	HtmlView *view;

	view = HTML_VIEW (container);
	
	if (child) {
		HtmlBox *box = g_object_get_data (G_OBJECT (child), "box");

		if (view->document->focus_element != DOM_ELEMENT (box->dom_node)) {
			html_document_update_focus_element (view->document,
							    DOM_ELEMENT (box->dom_node));
			
		}
	}
	
	GTK_CONTAINER_CLASS (parent_class)->set_focus_child (container, child);
}

static void
html_view_real_move_cursor (HtmlView *html_view, GtkMovementStep step, gint count, gboolean extend_selection)
{
	GtkAdjustment *vertical, *horizontal;

	vertical = GTK_LAYOUT (html_view)->vadjustment;
	horizontal = GTK_LAYOUT (html_view)->hadjustment;
	
	switch (step) {
	case GTK_MOVEMENT_VISUAL_POSITIONS:
		set_adjustment_clamped (horizontal, horizontal->value + horizontal->step_increment * count);
		break;
	case GTK_MOVEMENT_DISPLAY_LINES:
		set_adjustment_clamped (vertical, vertical->value + vertical->step_increment * count);
		break;
	case GTK_MOVEMENT_PAGES:
		set_adjustment_clamped (vertical, vertical->value + vertical->page_increment * count);
		break;
	case GTK_MOVEMENT_BUFFER_ENDS:
		if (count == -1) {
			set_adjustment_clamped (vertical, vertical->lower);
		}
		else {
			set_adjustment_clamped (vertical, vertical->upper);
		}
		break;
	default:
		g_warning ("unknown step!\n");
	}
}

static void
html_view_real_activate (HtmlView *view)
{
	html_event_activate (view);
}

static void html_view_real_move_focus_out (HtmlView *view, GtkDirectionType direction_type)
{
	GtkWidget *widget = GTK_WIDGET (view);

	html_document_update_focus_element (view->document, NULL);
	if (!GTK_WIDGET_HAS_FOCUS (widget))
		gtk_widget_grab_focus (widget);
	gtk_widget_queue_draw (widget);
}

static void
html_view_add_move_binding (GtkBindingSet *binding_set, guint keyval, guint modmask, GtkMovementStep step, gint count)
{
	gtk_binding_entry_add_signal (binding_set, keyval, modmask,
				      "move_cursor", 3,
				      GTK_TYPE_ENUM, step,
				      GTK_TYPE_INT, count,
				      GTK_TYPE_BOOL, FALSE);
}

static void
html_view_add_tab_binding (GtkBindingSet *binding_set, GdkModifierType modifiers, GtkDirectionType direction)
{
	gtk_binding_entry_add_signal (binding_set, GDK_Tab, modifiers,
				      "move_focus_out", 1,
				      GTK_TYPE_DIRECTION_TYPE, direction);
	gtk_binding_entry_add_signal (binding_set, GDK_KP_Tab, modifiers,
				      "move_focus_out", 1,
				      GTK_TYPE_DIRECTION_TYPE, direction);
}

void
html_view_set_document (HtmlView *view, HtmlDocument *document)
{
	g_return_if_fail (view != NULL);
	g_return_if_fail (HTML_IS_VIEW (view));

	if (view->document == document)
		return;

	if (document != NULL)
		g_object_ref (document);
	
	if (view->document != NULL) {
		html_view_disconnect_document (view, view->document);
		g_object_unref (view->document);
		html_view_layout_tree_free (view, view->root);
	}
	
	view->document = document;

	if (view->document != NULL) {
		html_view_setup_document (view);
	}

	gtk_widget_queue_resize (GTK_WIDGET (view));
}

HtmlBox *
html_view_find_layout_box (HtmlView *view, DomNode *node, gboolean find_parent)
{
	if (!find_parent)
		return g_hash_table_lookup (view->node_table, node);

	while (node) {
		HtmlBox *box = g_hash_table_lookup (view->node_table, node);

		if (box)
			return box;

		node = dom_Node__get_parentNode (node);
	}

	return NULL;
}

static void
html_view_try_jump (HtmlView *view)
{
	DomNode *node = html_document_find_anchor (view->document, view->jump_to_anchor);
	
	if (node != NULL) {
		
		html_view_scroll_to_node (view, node, HTML_VIEW_SCROLL_TO_TOP);
		g_free (view->jump_to_anchor);
		view->jump_to_anchor = NULL;
	}
}

/**
 * html_view_jump_to_anchor:
 * @view: 
 * @anchor: The name of the anchor to jump to
 * 
 * Scroll the @view so that the anchor with the
 * name @anchor is visible
 **/
void
html_view_jump_to_anchor (HtmlView *view, const gchar *anchor)
{
	g_return_if_fail (view != NULL);
	g_return_if_fail (HTML_IS_VIEW (view));
	g_return_if_fail (anchor != NULL);

	if (view->jump_to_anchor)
		g_free (view->jump_to_anchor);

	view->jump_to_anchor = g_strdup (anchor);
	html_view_try_jump (view);
}

gdouble
html_view_get_magnification (HtmlView *view)
{
	return view->magnification;
}

void
html_view_set_magnification (HtmlView *view, gdouble magnification)
{
	g_return_if_fail (view != NULL);
	g_return_if_fail (HTML_IS_VIEW (view));

	if (magnification < 0.05 || magnification > 20)
		return;

	if (view->root == NULL)
		return;

	if (magnification != view->magnification) {

		view->magnification = magnification;
		view->magnification_modified = TRUE;
		
		html_box_set_unrelayouted_down (view->root);
		
		html_view_relayout (view);
	}
}

#define MAG_STEP 1.1

void
html_view_zoom_in (HtmlView *view)
{
	html_view_set_magnification (view, view->magnification * MAG_STEP);
}

void
html_view_zoom_out (HtmlView *view)
{
	html_view_set_magnification (view, view->magnification * (1.0 / MAG_STEP));
}

void
html_view_zoom_reset (HtmlView *view)
{
	html_view_set_magnification (view, 1.0);
}

static void
html_view_class_init (HtmlViewClass *klass)
{
	GObjectClass *object_class = (GObjectClass *)klass;
	GtkObjectClass *gtkobject_class = (GtkObjectClass *)klass;
	GtkWidgetClass *widget_class = (GtkWidgetClass *)klass;
	GtkContainerClass *container_class = (GtkContainerClass *)klass;
	GtkBindingSet *binding_set;

	binding_set = gtk_binding_set_by_class (klass);
	parent_class = gtk_type_class (GTK_TYPE_LAYOUT);

	object_class->finalize = html_view_finalize;

	gtkobject_class->destroy = html_view_destroy;

	widget_class->focus = html_view_focus;
	
	widget_class->size_allocate = html_view_size_allocate;
	widget_class->realize = html_view_realize;
	widget_class->expose_event = html_view_expose;
	widget_class->motion_notify_event = html_view_motion_notify;
	widget_class->button_press_event = html_view_button_press;
	widget_class->button_release_event = html_view_button_release;

	widget_class->focus_in_event = html_view_focus_in;
	widget_class->focus_out_event = html_view_focus_out;
	
	widget_class->enter_notify_event = html_view_enter_notify;
	widget_class->leave_notify_event = html_view_leave_notify;

#ifdef ENABLE_ACCESSIBILITY
	widget_class->get_accessible = html_view_get_accessible;
#endif

	container_class->set_focus_child = html_view_set_focus_child;
	
	klass->move_cursor = html_view_real_move_cursor;
	klass->activate = html_view_real_activate;
	klass->move_focus_out = html_view_real_move_focus_out;
	
	view_signals [MOVE_CURSOR] =
		g_signal_new ("move_cursor",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (HtmlViewClass, move_cursor),
			      NULL, NULL,
			      html_marshal_VOID__ENUM_INT_BOOLEAN,
			      G_TYPE_NONE, 3,
			      GTK_TYPE_MOVEMENT_STEP,
			      G_TYPE_INT,
			      G_TYPE_BOOLEAN);
	
	view_signals [REQUEST_OBJECT] =
		g_signal_new ("request_object",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (HtmlViewClass, request_object),
			      NULL, NULL,
			      html_marshal_BOOLEAN__OBJECT,
			      G_TYPE_BOOLEAN, 1,
			      G_TYPE_OBJECT);
	
	view_signals [ON_URL] =
		g_signal_new ("on_url",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (HtmlViewClass, on_url),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__STRING,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_STRING);

	view_signals [ACTIVATE] =
		g_signal_new ("activate",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (HtmlViewClass, activate),
			      NULL, NULL,
			      html_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

	view_signals [MOVE_FOCUS_OUT] =
		g_signal_new ("move_focus_out",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (HtmlViewClass, move_focus_out),
			      NULL, NULL,
			      html_marshal_VOID__ENUM,
			      G_TYPE_NONE, 1,
			      GTK_TYPE_DIRECTION_TYPE);

	widget_class->activate_signal = view_signals[ACTIVATE];

	html_view_add_move_binding (binding_set, GDK_Page_Down, 0,
				    GTK_MOVEMENT_PAGES, 1);
	html_view_add_move_binding (binding_set, GDK_Page_Up, 0,
				    GTK_MOVEMENT_PAGES, -1);
	html_view_add_move_binding (binding_set, GDK_Home, 0,
				    GTK_MOVEMENT_BUFFER_ENDS, -1);
	html_view_add_move_binding (binding_set, GDK_End, 0,
				    GTK_MOVEMENT_BUFFER_ENDS, 1);

	html_view_add_move_binding (binding_set, GDK_Down, 0,
				    GTK_MOVEMENT_DISPLAY_LINES, 1);
	html_view_add_move_binding (binding_set, GDK_Up, 0,
				    GTK_MOVEMENT_DISPLAY_LINES, -1);

	html_view_add_move_binding (binding_set, GDK_Right, 0,
				    GTK_MOVEMENT_VISUAL_POSITIONS, 1);
	html_view_add_move_binding (binding_set, GDK_Left, 0,
				    GTK_MOVEMENT_VISUAL_POSITIONS, -1);
	
	html_view_add_tab_binding (binding_set, GDK_CONTROL_MASK, GTK_DIR_TAB_FORWARD);
	html_view_add_tab_binding (binding_set, GDK_CONTROL_MASK | GDK_SHIFT_MASK, GTK_DIR_TAB_BACKWARD);
	
}

static void
html_view_abstract_view_init (DomAbstractViewIface *iface)
{
	
}

static void
html_view_init (HtmlView *view)
{
	GTK_WIDGET_SET_FLAGS (view, GTK_CAN_FOCUS);
	  
	view->node_table = g_hash_table_new (g_direct_hash, g_direct_equal);
	view->document = NULL;
	view->root = NULL;
	view->relayout_idle_id = 0;
	view->relayout_timeout_id = 0;
	view->magnification = 1.0;
}

GType
html_view_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info= {
			sizeof (HtmlViewClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) html_view_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (HtmlView),
			16,   /* n_preallocs */
			(GInstanceInitFunc) html_view_init,
		};

		static const GInterfaceInfo dom_abstract_view_info = {
			(GInterfaceInitFunc) html_view_abstract_view_init,
			NULL,
			NULL
		};

		type = g_type_register_static (GTK_TYPE_LAYOUT, "HtmlView", &info, 0);
		g_type_add_interface_static (type,
					     DOM_TYPE_ABSTRACT_VIEW,
					     &dom_abstract_view_info);
	}

	return type;
}

static void 
html_view_update_box_style_size (HtmlBox *root, gfloat adjust, GPtrArray *done)
{
	HtmlBox *box;

	for (box = root; box; box = box->next) {
		HtmlStyle *style;

		style = HTML_BOX_GET_STYLE (box);
		if (style) {
			HtmlFontSpecification *font_spec;

			font_spec = style->inherited->font_spec;
			if (font_spec) {
				gint i;
				gboolean found = FALSE;

				for (i = 0; i < done->len; i++) {
					if (g_ptr_array_index (done, i) == font_spec) {
						found = TRUE;
						break;
					}
				}
				if (!found) {
					g_ptr_array_add (done, font_spec);
					font_spec->size = font_spec->size * adjust;
				}
			}
		}
		if (box->children) {
			html_view_update_box_style_size (box->children, adjust, done);
		}
	}
}

static void
html_view_style_set (GtkWidget *widget, GtkStyle *previous_style)
{
	if (previous_style) {
		gfloat fsize;
		gint new_isize;
		gint old_isize;

 		widget->style->bg[GTK_STATE_NORMAL] =
 			widget->style->base[GTK_STATE_NORMAL];
		fsize = pango_font_description_get_size (widget->style->font_desc) / (gfloat) PANGO_SCALE;
		new_isize = (gint) fsize;
		old_isize = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), "html-view-font-size"));
		if (old_isize && old_isize != new_isize) {
			/*
			 * The font size for the HtmlView has changed so we
			 * update the size in each HtmlFontSpecification.
			 * We traverse the document but we only want to
			 * update the HtmlFontSpecification once. We maintain
			 * an array of HtmlFontSpecification which have already
			 * been updated. As a HtmlFontSpecification may be used
			 * more than than one HtmlView we use a static array
			 * which is cleared when we determine that a different
			 * size change has occurred.
			 */
			HtmlView *view;

			view = HTML_VIEW (widget);
			g_object_set_data (G_OBJECT (widget), "html-view-font-size",
					   GINT_TO_POINTER (new_isize));
			if (view->root) {
				static gint old_size = 0;
				static gint new_size = 0;
				static GPtrArray *done;

				if (old_size != old_isize ||
				    new_size != new_isize) {
					if (old_size || new_size) {
						g_ptr_array_free (done, TRUE);
					}
					done = g_ptr_array_new ();
					old_size = old_isize;
					new_size = new_isize;
				}
				html_view_update_box_style_size (view->root, (gfloat) new_isize / (gfloat) old_isize, done);
			}
		}
	}
}

GtkWidget *
html_view_new (void)
{
	HtmlView *view = HTML_VIEW (g_object_new (HTML_TYPE_VIEW, NULL));	

	/* This creates the adjustments needed */
 	gtk_layout_set_hadjustment (GTK_LAYOUT (view), NULL);
 	gtk_layout_set_vadjustment (GTK_LAYOUT (view), NULL);

	g_signal_connect (G_OBJECT (view),
			  "style-set",
			  G_CALLBACK (html_view_style_set),
			  NULL);

	return GTK_WIDGET (view);
}

static void
html_view_set_saved_focus (HtmlView *view)
{
	gpointer saved_focus;

	saved_focus = g_object_get_data (G_OBJECT (view), "saved-focus");
	if (saved_focus) {
		g_object_weak_unref (G_OBJECT (saved_focus),
				     (GWeakNotify) focus_element_destroyed,
				     view);
	}
	g_object_weak_ref (G_OBJECT (view->document->focus_element),
			   (GWeakNotify) focus_element_destroyed,
			   view);
	g_object_set_data (G_OBJECT (view), "saved-focus", view->document->focus_element);
}

static DomElement* 
html_view_get_and_unset_saved_focus (HtmlView *view)
{
	gpointer saved_focus;
	
	saved_focus = g_object_get_data (G_OBJECT (view), "saved-focus");
	if (saved_focus) {
		g_object_weak_unref (G_OBJECT (saved_focus),
				     (GWeakNotify) focus_element_destroyed,
				     view);
		g_object_set_data (G_OBJECT (view), "saved-focus", NULL);
		return DOM_ELEMENT (saved_focus);
	} else
		return NULL;
}

static void
html_view_focus_element (HtmlView *view)
{
	HtmlBox *box;

	if (view->document->focus_element) {
		html_view_scroll_to_node (view, DOM_NODE (view->document->focus_element), HTML_VIEW_SCROLL_TO_BOTTOM);

		box = html_view_find_layout_box (view, DOM_NODE (view->document->focus_element), FALSE);
		if (box && HTML_IS_BOX_EMBEDDED (box)) 
			gtk_widget_child_focus (HTML_BOX_EMBEDDED (box)->widget, GTK_DIR_TAB_FORWARD);
		else
			gtk_widget_grab_focus (GTK_WIDGET (view));
	} else {
		/* No element focused to scroll to top */
		GtkAdjustment *adj = GTK_LAYOUT (view)->vadjustment;

		set_adjustment_clamped (adj, 0);
		gtk_widget_grab_focus (GTK_WIDGET (view));
	}
	gtk_widget_queue_draw (GTK_WIDGET (view));
}

#ifdef ENABLE_ACCESSIBILITY

static AtkObject *
html_view_get_accessible (GtkWidget *widget)
{
        static gboolean first_time = TRUE;

	if (first_time) {
		AtkObjectFactory *factory;
		AtkRegistry *registry;
		GType derived_type;
		GType derived_atk_type;

		/*
		 * Figure out whether accessibility is enabled by looking at the
		 * type of the accessible object which would be created for 
		 * the parent type of HtmlView.
		 */
		derived_type = g_type_parent (HTML_TYPE_VIEW);

		registry = atk_get_default_registry ();
		factory = atk_registry_get_factory (registry,
						    derived_type);
		derived_atk_type = atk_object_factory_get_accessible_type (factory);
		if (g_type_is_a (derived_atk_type, GTK_TYPE_ACCESSIBLE)) {
			/*
			 * Specify what factory to use to create accessible
			 * objects
			 */
			HTML_ACCESSIBLE_SET_FACTORY (HTML_TYPE_VIEW, html_view_accessible);
			HTML_ACCESSIBLE_SET_FACTORY (HTML_TYPE_BOX_BLOCK, html_box_block_accessible);
			HTML_ACCESSIBLE_SET_FACTORY (HTML_TYPE_BOX_EMBEDDED, html_box_embedded_accessible);
			HTML_ACCESSIBLE_SET_FACTORY (HTML_TYPE_BOX, html_box_accessible);
			HTML_ACCESSIBLE_SET_FACTORY (HTML_TYPE_BOX_TABLE, html_box_table_accessible);
			HTML_ACCESSIBLE_SET_FACTORY (HTML_TYPE_BOX_TEXT, html_box_text_accessible);
		}
		first_time = FALSE;
	}
	return GTK_WIDGET_CLASS (parent_class)->get_accessible (widget);
}

#endif /* ENABLE_ACCESSIBILITY */
