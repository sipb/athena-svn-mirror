/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright 1999, 2000 Helix Code, Inc.

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

#include <config.h>
#include <ctype.h>

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkprivate.h>
#include <gtk/gtk.h>
#include <string.h>
#include <gal/widgets/e-unicode.h>

#include <gnome.h>

#include "htmlcolorset.h"
#include "htmlcursor.h"
#include "htmldrawqueue.h"
#include "htmlengine-edit.h"
#include "htmlengine-edit-clueflowstyle.h"
#include "htmlengine-edit-cut-and-paste.h"
#include "htmlengine-edit-fontstyle.h"
#include "htmlengine-edit-rule.h"
#include "htmlengine-edit-movement.h"
#include "htmlengine-edit-cursor.h"
#include "htmlengine-edit-table.h"
#include "htmlengine-edit-tablecell.h"
#include "htmlengine-edit-text.h"
#include "htmlengine-edit-selection-updater.h"
#include "htmlengine-print.h"
#include "htmlengine-save.h"
#include "htmlform.h"
#include "htmlframe.h"
#include "htmliframe.h"
#include "htmlimage.h"
#include "htmlinterval.h"
#include "htmllinktext.h"
#include "htmlplainpainter.h"
#include "htmlsettings.h"
#include "htmltable.h"
#include "htmltext.h"
#include "htmltextslave.h"
#include "htmlselection.h"
#include "htmlundo.h"

#include "gtkhtml.h"
#include "gtkhtml-embedded.h"
#include "gtkhtml-im.h"
#include "gtkhtml-keybinding.h"
#include "gtkhtml-search.h"
#include "gtkhtml-stream.h"
#include "gtkhtml-private.h"
#include "gtkhtml-properties.h"
#include "math.h"
#include <libgnome/gnome-util.h>

enum DndTargetType {
	DND_TARGET_TYPE_TEXT_URI_LIST,
	DND_TARGET_TYPE__NETSCAPE_URL,
	DND_TARGET_TYPE_TEXT_PLAIN,
	DND_TARGET_TYPE_STRING,
};
static GtkTargetEntry dnd_link_sources [] = {
	{ "text/uri-list", 0, DND_TARGET_TYPE_TEXT_URI_LIST },
	{ "_NETSCAPE_URL", 0, DND_TARGET_TYPE__NETSCAPE_URL },
	{ "text/plain", 0, DND_TARGET_TYPE_TEXT_PLAIN },
	{ "STRING", 0, DND_TARGET_TYPE_STRING },
};
#define DND_LINK_SOURCES sizeof (dnd_link_sources) / sizeof (GtkTargetEntry)

#define GNOME_SPELL_GCONF_DIR "/GNOME/Spell"

#define d_s(x)

static GtkLayoutClass *parent_class = NULL;

#ifdef GTKHTML_HAVE_GCONF
GConfClient *gconf_client = NULL;
GError      *gconf_error  = NULL;
#endif

enum {
	TITLE_CHANGED,
	URL_REQUESTED,
	LOAD_DONE,
	LINK_CLICKED,
	SET_BASE,
	SET_BASE_TARGET,
	ON_URL,
	REDIRECT,
	SUBMIT,
	OBJECT_REQUESTED,
	CURRENT_PARAGRAPH_STYLE_CHANGED,
	CURRENT_PARAGRAPH_INDENTATION_CHANGED,
	CURRENT_PARAGRAPH_ALIGNMENT_CHANGED,
	INSERTION_FONT_STYLE_CHANGED,
	INSERTION_COLOR_CHANGED,
	SIZE_CHANGED,
	IFRAME_CREATED,
	/* keybindings signals */
	SCROLL,
	CURSOR_MOVE,
	COMMAND,
	/* now only last signal */
	LAST_SIGNAL
};
static guint signals [LAST_SIGNAL] = { 0 };

static void
gtk_html_update_scrollbars_on_resize (GtkHTML *html,
				      gdouble old_doc_width, gdouble old_doc_height,
				      gdouble old_width, gdouble old_height,
				      gboolean *changed_x, gboolean *changed_y);

/* keybindings signal hadlers */
static void     scroll                 (GtkHTML *html, GtkOrientation orientation, GtkScrollType scroll_type, gfloat position);
static void     cursor_move            (GtkHTML *html, GtkDirectionType dir_type, GtkHTMLCursorSkipType skip);
static gboolean command                (GtkHTML *html, GtkHTMLCommandType com_type);
static gint     mouse_change_pos       (GtkWidget *widget, GdkWindow *window, gint x, gint y);
static void     load_keybindings       (GtkHTMLClass *klass);
static void     set_editor_keybindings (GtkHTML *html, gboolean editable);
static gchar *  get_value_nick         (GtkHTMLCommandType com_type);


/* Values for selection information.  FIXME: what about COMPOUND_STRING and
   TEXT?  */
enum _TargetInfo {
	TARGET_HTML,
	TARGET_UTF8_STRING,
	TARGET_UTF8,
	TARGET_COMPOUND_TEXT,
	TARGET_STRING,
	TARGET_TEXT
};

typedef enum _TargetInfo TargetInfo;

/* Interval for scrolling during selection.  */
#define SCROLL_TIMEOUT_INTERVAL 10


GtkHTMLParagraphStyle
clueflow_style_to_paragraph_style (HTMLClueFlowStyle style, HTMLListType item_type)
{
	switch (style) {
	case HTML_CLUEFLOW_STYLE_NORMAL:
		return GTK_HTML_PARAGRAPH_STYLE_NORMAL;
	case HTML_CLUEFLOW_STYLE_H1:
		return GTK_HTML_PARAGRAPH_STYLE_H1;
	case HTML_CLUEFLOW_STYLE_H2:
		return GTK_HTML_PARAGRAPH_STYLE_H2;
	case HTML_CLUEFLOW_STYLE_H3:
		return GTK_HTML_PARAGRAPH_STYLE_H3;
	case HTML_CLUEFLOW_STYLE_H4:
		return GTK_HTML_PARAGRAPH_STYLE_H4;
	case HTML_CLUEFLOW_STYLE_H5:
		return GTK_HTML_PARAGRAPH_STYLE_H5;
	case HTML_CLUEFLOW_STYLE_H6:
		return GTK_HTML_PARAGRAPH_STYLE_H6;
	case HTML_CLUEFLOW_STYLE_ADDRESS:
		return GTK_HTML_PARAGRAPH_STYLE_ADDRESS;
	case HTML_CLUEFLOW_STYLE_PRE:
		return GTK_HTML_PARAGRAPH_STYLE_PRE;
	case HTML_CLUEFLOW_STYLE_LIST_ITEM:
		switch (item_type) {
		case HTML_LIST_TYPE_UNORDERED:
			return GTK_HTML_PARAGRAPH_STYLE_ITEMDOTTED;
		case HTML_LIST_TYPE_ORDERED_ARABIC:
			return GTK_HTML_PARAGRAPH_STYLE_ITEMDIGIT;
		case HTML_LIST_TYPE_ORDERED_LOWER_ROMAN:
		case HTML_LIST_TYPE_ORDERED_UPPER_ROMAN:
			return GTK_HTML_PARAGRAPH_STYLE_ITEMROMAN;
		case HTML_LIST_TYPE_ORDERED_LOWER_ALPHA:
		case HTML_LIST_TYPE_ORDERED_UPPER_ALPHA:
			return GTK_HTML_PARAGRAPH_STYLE_ITEMALPHA;
		default:
			return GTK_HTML_PARAGRAPH_STYLE_ITEMDOTTED;
		}
	default:		/* This should not really happen, though.  */
		return GTK_HTML_PARAGRAPH_STYLE_NORMAL;
	}
}

void
paragraph_style_to_clueflow_style (GtkHTMLParagraphStyle style, HTMLClueFlowStyle *flow_style, HTMLListType *item_type)
{
	*item_type = HTML_LIST_TYPE_BLOCKQUOTE;
	*flow_style = HTML_CLUEFLOW_STYLE_LIST_ITEM;

	switch (style) {
	case GTK_HTML_PARAGRAPH_STYLE_NORMAL:
		*flow_style = HTML_CLUEFLOW_STYLE_NORMAL;
		break;
	case GTK_HTML_PARAGRAPH_STYLE_H1:
		*flow_style = HTML_CLUEFLOW_STYLE_H1;
		break;
	case GTK_HTML_PARAGRAPH_STYLE_H2:
		*flow_style = HTML_CLUEFLOW_STYLE_H2;
		break;
	case GTK_HTML_PARAGRAPH_STYLE_H3:
		*flow_style = HTML_CLUEFLOW_STYLE_H3;
		break;
	case GTK_HTML_PARAGRAPH_STYLE_H4:
		*flow_style = HTML_CLUEFLOW_STYLE_H4;
		break;
	case GTK_HTML_PARAGRAPH_STYLE_H5:
		*flow_style = HTML_CLUEFLOW_STYLE_H5;
		break;
	case GTK_HTML_PARAGRAPH_STYLE_H6:
		*flow_style = HTML_CLUEFLOW_STYLE_H6;
		break;
	case GTK_HTML_PARAGRAPH_STYLE_ADDRESS:
		*flow_style = HTML_CLUEFLOW_STYLE_ADDRESS;
		break;
	case GTK_HTML_PARAGRAPH_STYLE_PRE:
		*flow_style = HTML_CLUEFLOW_STYLE_PRE;
		break;
	case GTK_HTML_PARAGRAPH_STYLE_ITEMDOTTED:
		*item_type = HTML_LIST_TYPE_UNORDERED;
		break;
	case GTK_HTML_PARAGRAPH_STYLE_ITEMROMAN:
		*item_type = HTML_LIST_TYPE_ORDERED_UPPER_ROMAN;
		break;
	case GTK_HTML_PARAGRAPH_STYLE_ITEMALPHA:
		*item_type = HTML_LIST_TYPE_ORDERED_UPPER_ALPHA;
		break;
	case GTK_HTML_PARAGRAPH_STYLE_ITEMDIGIT:
		*item_type = HTML_LIST_TYPE_ORDERED_ARABIC;
		break;
	default:		/* This should not really happen, though.  */
		*flow_style = HTML_CLUEFLOW_STYLE_NORMAL;
	}
}

HTMLHAlignType
paragraph_alignment_to_html (GtkHTMLParagraphAlignment alignment)
{
	switch (alignment) {
	case GTK_HTML_PARAGRAPH_ALIGNMENT_LEFT:
		return HTML_HALIGN_LEFT;
	case GTK_HTML_PARAGRAPH_ALIGNMENT_RIGHT:
		return HTML_HALIGN_RIGHT;
	case GTK_HTML_PARAGRAPH_ALIGNMENT_CENTER:
		return HTML_HALIGN_CENTER;
	default:
		return HTML_HALIGN_LEFT;
	}
}

GtkHTMLParagraphAlignment
html_alignment_to_paragraph (HTMLHAlignType alignment)
{
	switch (alignment) {
	case HTML_HALIGN_LEFT:
		return GTK_HTML_PARAGRAPH_ALIGNMENT_LEFT;
	case HTML_HALIGN_CENTER:
		return GTK_HTML_PARAGRAPH_ALIGNMENT_CENTER;
	case HTML_HALIGN_RIGHT:
		return GTK_HTML_PARAGRAPH_ALIGNMENT_RIGHT;
	default:
		return GTK_HTML_PARAGRAPH_ALIGNMENT_LEFT;
	}
}

void
gtk_html_update_styles (GtkHTML *html)
{
	GtkHTMLParagraphStyle paragraph_style;
	GtkHTMLParagraphAlignment alignment;
	HTMLEngine *engine;
	HTMLClueFlowStyle flow_style;
	HTMLListType item_type;
	guint indentation;

	/* printf ("gtk_html_update_styles called\n"); */

	if (! html_engine_get_editable (html->engine))
		return;

	engine          = html->engine;
	html_engine_get_current_clueflow_style (engine, &flow_style, &item_type);
	paragraph_style = clueflow_style_to_paragraph_style (flow_style, item_type);

	if (paragraph_style != html->priv->paragraph_style) {
		html->priv->paragraph_style = paragraph_style;
		gtk_signal_emit (GTK_OBJECT (html), signals [CURRENT_PARAGRAPH_STYLE_CHANGED],
				 paragraph_style);
	}

	indentation = html_engine_get_current_clueflow_indentation (engine);
	if (indentation != html->priv->paragraph_indentation) {
		html->priv->paragraph_indentation = indentation;
		gtk_signal_emit (GTK_OBJECT (html), signals [CURRENT_PARAGRAPH_INDENTATION_CHANGED], indentation);
	}

	alignment = html_alignment_to_paragraph (html_engine_get_current_clueflow_alignment (engine));
 	if (alignment != html->priv->paragraph_alignment) {
		html->priv->paragraph_alignment = alignment;
		gtk_signal_emit (GTK_OBJECT (html), signals [CURRENT_PARAGRAPH_ALIGNMENT_CHANGED], alignment);
	}

	if (html_engine_update_insertion_font_style (engine))
		gtk_signal_emit (GTK_OBJECT (html), signals [INSERTION_FONT_STYLE_CHANGED], engine->insertion_font_style);
	if (html_engine_update_insertion_color (engine))
		gtk_signal_emit (GTK_OBJECT (html), signals [INSERTION_COLOR_CHANGED], engine->insertion_color);

	/* TODO add insertion_url_or_targed_changed signal */
	html_engine_update_insertion_url_and_target (engine);
}


/* GTK+ idle loop handler.  */

static gint
idle_handler (gpointer data)
{
	GtkHTML *html;
	HTMLEngine *engine;

	html = GTK_HTML (data);
	engine = html->engine;

	if (html->priv->scroll_timeout_id == 0  &&
	    html->engine->thaw_idle_id == 0  &&
	    !html_engine_frozen (html->engine))
		html_engine_make_cursor_visible (engine);

	gtk_adjustment_set_value (GTK_LAYOUT (html)->hadjustment, (gfloat) engine->x_offset);
	gtk_adjustment_set_value (GTK_LAYOUT (html)->vadjustment, (gfloat) engine->y_offset);

	gtk_html_private_calc_scrollbars (html, NULL, NULL);

	if (html->engine->thaw_idle_id == 0 && !html_engine_frozen (html->engine))
		html_engine_flush_draw_queue (engine);

 	html->priv->idle_handler_id = 0;
	return FALSE;
}

static void
queue_draw (GtkHTML *html)
{
	if (html->priv->idle_handler_id == 0)
		html->priv->idle_handler_id = gtk_idle_add (idle_handler, html);
}


/* HTMLEngine callbacks.  */

static void
html_engine_title_changed_cb (HTMLEngine *engine, gpointer data)
{
	GtkHTML *gtk_html;

	gtk_html = GTK_HTML (data);
	gtk_signal_emit (GTK_OBJECT (gtk_html), signals[TITLE_CHANGED], engine->title->str);
}

static void
html_engine_set_base_cb (HTMLEngine *engine, const gchar *base, gpointer data)
{
	GtkHTML *gtk_html;

	gtk_html = GTK_HTML (data);
	gtk_signal_emit (GTK_OBJECT (gtk_html), signals[SET_BASE], base);
}

static void
html_engine_set_base_target_cb (HTMLEngine *engine, const gchar *base_target, gpointer data)
{
	GtkHTML *gtk_html;

	gtk_html = GTK_HTML (data);
	gtk_signal_emit (GTK_OBJECT (gtk_html), signals[SET_BASE_TARGET], base_target);
}

static void
html_engine_load_done_cb (HTMLEngine *engine, gpointer data)
{
	GtkHTML *gtk_html;

	gtk_html = GTK_HTML (data);
	gtk_signal_emit (GTK_OBJECT (gtk_html), signals[LOAD_DONE]);
}

static void
html_engine_url_requested_cb (HTMLEngine *engine,
			      const gchar *url,
			      GtkHTMLStream *handle,
			      gpointer data)
{
	GtkHTML *gtk_html;
	char *expanded = NULL;
	gtk_html = GTK_HTML (data);

	expanded = gtk_html_get_url_base_relative (gtk_html, url);
	gtk_signal_emit (GTK_OBJECT (gtk_html), signals[URL_REQUESTED], expanded, handle);
	g_free (expanded);
}

static void
html_engine_draw_pending_cb (HTMLEngine *engine,
			     gpointer data)
{
	GtkHTML *html;

	html = GTK_HTML (data);
	queue_draw (html);
}

static void
html_engine_redirect_cb (HTMLEngine *engine,
			 const gchar *url,
			 int delay,
			 gpointer data)
{
	GtkHTML *gtk_html;

	gtk_html = GTK_HTML (data);

	gtk_signal_emit (GTK_OBJECT (gtk_html), signals[REDIRECT], url, delay);
}

static void
html_engine_submit_cb (HTMLEngine *engine,
		       const gchar *method,
		       const gchar *url,
		       const gchar *encoding,
		       gpointer data)
{
	GtkHTML *gtk_html;

	gtk_html = GTK_HTML (data);

	gtk_signal_emit (GTK_OBJECT (gtk_html), signals[SUBMIT], method, url, encoding);
}

static gboolean
html_engine_object_requested_cb (HTMLEngine *engine,
		       GtkHTMLEmbedded *eb,
		       gpointer data)
{
	GtkHTML *gtk_html;
	gboolean object_found = FALSE;

	gtk_html = GTK_HTML (data);

	object_found = FALSE;
	gtk_signal_emit (GTK_OBJECT (gtk_html), signals[OBJECT_REQUESTED], eb, &object_found);
	return object_found;
}


/* GtkAdjustment handling.  */

static void
scroll_update_mouse (GtkWidget *widget)
{
	gint x, y;

	if (GTK_WIDGET_REALIZED (widget)) {
		gdk_window_get_pointer (GTK_LAYOUT (widget)->bin_window, &x, &y, NULL);
		mouse_change_pos (widget, widget->window, x, y);
	}
}

static void
vertical_scroll_cb (GtkAdjustment *adjustment, gpointer data)
{
	GtkHTML *html = GTK_HTML (data);

	/* check if adjustment is valid, it's changed in
	   Layout::size_allocate and we can't do anything about it,
	   because it uses private fields we cannot access, so we have
	   to use it*/
	if (html->engine->height != adjustment->page_increment)
		return;
		
	html->engine->y_offset = (gint)adjustment->value;
	scroll_update_mouse (GTK_WIDGET (data));
}

static void
horizontal_scroll_cb (GtkAdjustment *adjustment, gpointer data)
{
	GtkHTML *html = GTK_HTML (data);

	/* check if adjustment is valid, it's changed in
	   Layout::size_allocate and we can't do anything about it,
	   because it uses private fields we cannot access, so we have
	   to use it*/
	if (html->engine->width != adjustment->page_increment)
		return;
		
	html->engine->x_offset = (gint)adjustment->value;
	scroll_update_mouse (GTK_WIDGET (data));
}

static void
connect_adjustments (GtkHTML *html,
		     GtkAdjustment *hadj,
		     GtkAdjustment *vadj)
{
	GtkLayout *layout;

	layout = GTK_LAYOUT (html);

	if (html->hadj_connection != 0)
		gtk_signal_disconnect (GTK_OBJECT(layout->hadjustment),
				       html->hadj_connection);

	if (html->vadj_connection != 0)
		gtk_signal_disconnect (GTK_OBJECT(layout->vadjustment),
				       html->vadj_connection);

	if (vadj != NULL)
		html->vadj_connection =
			gtk_signal_connect (GTK_OBJECT (vadj), "value_changed",
					    GTK_SIGNAL_FUNC (vertical_scroll_cb), (gpointer)html);
	else
		html->vadj_connection = 0;
	
	if (hadj != NULL)
		html->hadj_connection =
			gtk_signal_connect (GTK_OBJECT (hadj), "value_changed",
					    GTK_SIGNAL_FUNC (horizontal_scroll_cb), (gpointer)html);
	else
		html->hadj_connection = 0;
}


/* Scroll timeout handling.  */

static void
inc_adjustment (GtkAdjustment *adj, gint doc_width, gint alloc_width, gint inc)
{
	gfloat value;
	gint max;

	value = adj->value + (gfloat) inc;
	
	if (doc_width > alloc_width)
		max = doc_width - alloc_width;
	else
		max = 0;

	if (value > (gfloat) max)
		value = (gfloat) max;
	else if (value < 0)
		value = 0.0;

	gtk_adjustment_set_value (adj, value);
}

static gint
scroll_timeout_cb (gpointer data)
{
	GtkWidget *widget;
	GtkHTML *html;
	GtkLayout *layout;
	gint x_scroll, y_scroll;
	gint x, y;

	GDK_THREADS_ENTER ();

	widget = GTK_WIDGET (data);
	html = GTK_HTML (data);

	gdk_window_get_pointer (widget->window, &x, &y, NULL);

	if (x < 0) {
		x_scroll = x;
		x = 0;
	} else if (x >= widget->allocation.width) {
		x_scroll = x - widget->allocation.width + 1;
		x = widget->allocation.width;
	} else {
		x_scroll = 0;
	}
	x_scroll /= 2;

	if (y < 0) {
		y_scroll = y;
		y = 0;
	} else if (y >= widget->allocation.height) {
		y_scroll = y - widget->allocation.height + 1;
		y = widget->allocation.height;
	} else {
		y_scroll = 0;
	}
	y_scroll /= 2;

	if (html->in_selection && (x_scroll != 0 || y_scroll != 0)) {
		HTMLEngine *engine;

		engine = html->engine;
		html_engine_select_region (engine,
					   html->selection_x1, html->selection_y1,
					   x + engine->x_offset, y + engine->y_offset);
	}

	layout = GTK_LAYOUT (widget);

	inc_adjustment (layout->hadjustment, html_engine_get_doc_width (html->engine),
			widget->allocation.width, x_scroll);
	inc_adjustment (layout->vadjustment, html_engine_get_doc_height (html->engine),
			widget->allocation.height, y_scroll);

	GDK_THREADS_LEAVE ();

	return TRUE;
}

static void
setup_scroll_timeout (GtkHTML *html)
{
	if (html->priv->scroll_timeout_id != 0)
		return;

	html->priv->scroll_timeout_id = gtk_timeout_add (SCROLL_TIMEOUT_INTERVAL,
						   scroll_timeout_cb, html);

	GDK_THREADS_LEAVE();
	scroll_timeout_cb (html);
	GDK_THREADS_ENTER();
}

static void
remove_scroll_timeout (GtkHTML *html)
{
	if (html->priv->scroll_timeout_id == 0)
		return;

	gtk_timeout_remove (html->priv->scroll_timeout_id);
	html->priv->scroll_timeout_id = 0;
}


/* GtkObject methods.  */

static void
destroy (GtkObject *object)
{
	GtkHTML *html;

	html = GTK_HTML (object);

	g_free (html->pointer_url);
	gdk_cursor_destroy (html->hand_cursor);
	gdk_cursor_destroy (html->arrow_cursor);
	gdk_cursor_destroy (html->ibeam_cursor);

	connect_adjustments (html, NULL, NULL);

	if (html->priv->idle_handler_id != 0)
		gtk_idle_remove (html->priv->idle_handler_id);

	if (html->priv->scroll_timeout_id != 0)
		gtk_timeout_remove (html->priv->scroll_timeout_id);
	
#ifdef GTKHTML_HAVE_GCONF
	if (html->priv->set_font_id)
		g_source_remove (html->priv->set_font_id);

	if (html->priv->notify_id)
		gconf_client_notify_remove (gconf_client, html->priv->notify_id);
	if (html->priv->notify_spell_id)
		gconf_client_notify_remove (gconf_client, html->priv->notify_spell_id);
#endif

	g_free (html->priv->content_type);
	g_free (html->priv);
	html->priv = NULL;

	gtk_object_unref (GTK_OBJECT (html->engine));

	if (GTK_OBJECT_CLASS (parent_class)->destroy != NULL)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}


/* GtkWidget methods.  */
static void
style_set (GtkWidget *widget,
	   GtkStyle  *previous_style)
{
	HTMLEngine *engine = GTK_HTML (widget)->engine;
       
	html_colorset_set_style (engine->defaultSettings->color_set,
				 widget->style);
	html_colorset_set_unchanged (engine->settings->color_set,
				     engine->defaultSettings->color_set);
	html_engine_schedule_update (engine);

#ifdef GTK_HTML_USE_XIM
	if (previous_style)
		gtk_html_im_style_set (GTK_HTML (widget));
#endif
}

static gint
key_press_event (GtkWidget *widget,
		 GdkEventKey *event)
{
	GtkHTML *html = GTK_HTML (widget);
	gboolean retval, update = TRUE;

	html->binding_handled = FALSE;
	html->priv->update_styles = FALSE;
	html->priv->event_time = event->time;
	if (html->editor_bindings && html_engine_get_editable (html->engine))
		gtk_binding_set_activate (html->editor_bindings, 
					  event->keyval, 
					  event->state, 
					  GTK_OBJECT (widget));

	if (!html->binding_handled)
		gtk_bindings_activate (GTK_OBJECT (widget), 
				       event->keyval,
				       event->state);
	
	retval = html->binding_handled;
	update = html->priv->update_styles;

	if (! retval
	    && html_engine_get_editable (html->engine)
	    && ! (event->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK))) {
		gchar *str;

		/*
		printf ("event length: %d s[0]: %d string: '%s'\n", 
			event->length, event->string [0], event->string); 
		*/

		str = e_utf8_from_gtk_event_key (widget, event->keyval, event->string);
		/*
		printf ("len: %d str: %s\n", str ? g_utf8_strlen (str, -1) : -1, str);
		*/

		if (str) {
			html_engine_paste_text (html->engine, str, g_utf8_strlen (str, -1));
			retval = TRUE;
		} else if (event->length == 1 && event->string
			   && ((guchar)event->string [0]) > 0x20 
			   && ((guchar)event->string [0]) < 0x80) {
			html_engine_paste_text (html->engine, event->string, 1);
			retval = TRUE;
		}

		g_free (str);
		update = FALSE;
	}

	if (retval && html_engine_get_editable (html->engine))
		html_engine_reset_blinking_cursor (html->engine);

	if (retval && update)
		gtk_html_update_styles (html);

	html->priv->event_time = 0;
	
	/*
	printf ("editable: %d keyval %d len: %d\n string: %s", 
		html_engine_get_editable (html->engine), 
		event->keyval, event->length, event->string);
	*/
	if (!html_engine_get_editable (html->engine)) {
		switch (event->keyval) {
		case GDK_Return:
		case GDK_KP_Enter:
			if (html->engine->focus_object) {
				gchar *url;
				url = html_object_get_complete_url (html->engine->focus_object);
				if (url) {
					/* printf ("link clicked: %s\n", url); */
					gtk_signal_emit (GTK_OBJECT (html), signals [LINK_CLICKED], url);
					g_free (url);
				}
			}
			break;
		default:
			;
		}
	}

	/* printf ("retval: %d\n", retval); */

	return retval;
}

static void
realize (GtkWidget *widget)
{
	GtkHTML *html;
	GtkLayout *layout;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_HTML (widget));

	html = GTK_HTML (widget);
	layout = GTK_LAYOUT (widget);

	if (GTK_WIDGET_CLASS (parent_class)->realize)
		(* GTK_WIDGET_CLASS (parent_class)->realize) (widget);

	widget->style = gtk_style_attach (widget->style, widget->window);
	gdk_window_set_events (html->layout.bin_window,
			       (gdk_window_get_events (html->layout.bin_window)
				| GDK_EXPOSURE_MASK | GDK_POINTER_MOTION_MASK
				| GDK_ENTER_NOTIFY_MASK
				| GDK_BUTTON_PRESS_MASK 
				| GDK_BUTTON_RELEASE_MASK
				| GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK));

	html_engine_realize (html->engine, html->layout.bin_window);

	gdk_window_set_cursor (widget->window, html->arrow_cursor);

	/* This sets the backing pixmap to None, so that scrolling does not
           erase the newly exposed area, thus making the thing smoother.  */
	gdk_window_set_back_pixmap (html->layout.bin_window, NULL, FALSE);

	/* If someone was silly enough to stick us in something that doesn't 
	 * have adjustments, go ahead and create them now
	 */
#if 0
	if (layout->hadjustment == NULL)
		gtk_layout_set_hadjustment (layout, NULL);

	if (layout->vadjustment == NULL)
		gtk_layout_set_vadjustment (layout, NULL);
#else 
	if (layout->hadjustment == NULL) {
		layout->hadjustment = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0));

		gtk_object_ref (GTK_OBJECT (layout->hadjustment));
		gtk_object_sink (GTK_OBJECT (layout->hadjustment));
	}

	if (layout->vadjustment == NULL) {
		layout->vadjustment = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0));
		
		gtk_object_ref (GTK_OBJECT (layout->vadjustment));
		gtk_object_sink (GTK_OBJECT (layout->vadjustment));	
	}
#endif
		
#ifdef GTK_HTML_USE_XIM
	gtk_html_im_realize (html);
#endif /* GTK_HTML_USE_XIM */

#ifdef GTK_HTML_DND_DEST
	gtk_drag_dest_set (widget, GTK_DEST_DEFAULT_ALL,
			   dnd_link_sources, DND_LINK_SOURCES,
			   GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK);
#endif
	
}

static void
unrealize (GtkWidget *widget)
{
	GtkHTML *html = GTK_HTML (widget);
	
	html_engine_unrealize (html->engine);

#ifdef GTK_HTML_USE_XIM	
	gtk_html_im_unrealize (html);
#endif
	if (GTK_WIDGET_CLASS (parent_class)->unrealize)
		(* GTK_WIDGET_CLASS (parent_class)->unrealize) (widget);
}

static gint
expose (GtkWidget *widget, GdkEventExpose *event)
{
	/* printf ("expose x: %d y: %d\n", GTK_HTML (widget)->engine->x_offset, GTK_HTML (widget)->engine->y_offset); */

	html_engine_draw (GTK_HTML (widget)->engine,
			  event->area.x, event->area.y,
			  event->area.width, event->area.height);

	if (GTK_WIDGET_CLASS (parent_class)->expose_event)
		(* GTK_WIDGET_CLASS (parent_class)->expose_event) (widget, event);

	/* printf ("expose END\n"); */

	return TRUE;
}

static void
draw (GtkWidget *widget, GdkRectangle *area)
{
	GtkHTML *html = GTK_HTML (widget);
	HTMLPainter *painter = html->engine->painter;

	html_painter_clear (painter);

	html_engine_draw (GTK_HTML (widget)->engine,
			  area->x, area->y,
			  area->width, area->height);

	if (GTK_WIDGET_CLASS (parent_class)->draw)
		(* GTK_WIDGET_CLASS (parent_class)->draw) (widget, area);
}

static void
size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	GtkHTML *html;
	gboolean changed_x = FALSE, changed_y = FALSE;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_HTML (widget));
	g_return_if_fail (allocation != NULL);
	
	if (GTK_WIDGET_CLASS (parent_class)->size_allocate)
		( *GTK_WIDGET_CLASS (parent_class)->size_allocate) (widget, allocation);

	/* printf ("size allocate\n"); */

	html = GTK_HTML (widget);

	if (html->engine->width != allocation->width
	    || html->engine->height != allocation->height) {
		HTMLEngine *e = html->engine;
		gint old_doc_width, old_doc_height, old_width, old_height;

		old_doc_width = html_engine_get_doc_width (html->engine);
		old_doc_height = html_engine_get_doc_height (html->engine);
		old_width = e->width;
		old_height = e->height;

		/* printf ("allocate %d x %d\n", allocation->width, allocation->height); */

		e->width  = allocation->width;
		e->height = allocation->height;

		html_engine_calc_size (html->engine, FALSE);
		gtk_html_update_scrollbars_on_resize (html, old_doc_width, old_doc_height, old_width, old_height,
						      &changed_x, &changed_y);
	}
	gtk_html_private_calc_scrollbars (html, &changed_x, &changed_y);

	if (changed_x)
		gtk_adjustment_value_changed (GTK_LAYOUT (html)->hadjustment);
	if (changed_y)
		gtk_adjustment_value_changed (GTK_LAYOUT (html)->vadjustment);

#ifdef GTK_HTML_USE_XIM
	gtk_html_im_size_allocate (html);
#endif

}

static void
set_pointer_url (GtkHTML *html, const char *url)
{
	if (url == html->pointer_url)
		return;

	if (url && html->pointer_url && !strcmp (url, html->pointer_url))
		return;
		
	g_free (html->pointer_url);
	html->pointer_url = url ? g_strdup (url) : NULL;
	gtk_signal_emit (GTK_OBJECT (html), 
			 signals[ON_URL], html->pointer_url);
}

static void
dnd_link_set (GtkWidget *widget, HTMLObject *o)
{
	if (!html_engine_get_editable (GTK_HTML (widget)->engine)) {
		/* printf ("dnd_link_set %p\n", o); */

		gtk_drag_source_set (widget, GDK_BUTTON1_MASK,
				     dnd_link_sources, DND_LINK_SOURCES,
				     GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK);
		GTK_HTML (widget)->priv->dnd_object = o;
	}
}

static void
dnd_link_unset (GtkWidget *widget)
{
	if (!html_engine_get_editable (GTK_HTML (widget)->engine)) {
		/* printf ("dnd_link_unset\n"); */

		gtk_drag_source_unset (widget);
		GTK_HTML (widget)->priv->dnd_object = NULL;
	}
}

static void
on_object (GtkWidget *widget, GdkWindow *window, HTMLObject *obj)
{
	GtkHTML *html = GTK_HTML (widget);

	if (obj) {
		gchar *url;
		url = gtk_html_get_url_object_relative (html, obj, 
							html_object_get_url (obj));
		if (url != NULL) {
			set_pointer_url (html, url);
			dnd_link_set (widget, obj);
			
			if (html->engine->editable)
				gdk_window_set_cursor (window, html->ibeam_cursor);
			else {
				gdk_window_set_cursor (window, html->hand_cursor);
			}
		} else {
			set_pointer_url (html, NULL);
			dnd_link_unset (widget);			

			if (html_object_is_text (obj) && html->allow_selection)
				gdk_window_set_cursor (window, html->ibeam_cursor);
			else
				gdk_window_set_cursor (window, html->arrow_cursor);
		}

		g_free (url);
	} else {
		set_pointer_url (html, NULL);
		dnd_link_unset (widget);			

		gdk_window_set_cursor (window, html->arrow_cursor);
	}
}

#define HTML_DIST(x,y) sqrt(x*x + y*y)

static gint
mouse_change_pos (GtkWidget *widget, GdkWindow *window, gint x, gint y)
{
	GtkHTML *html;
	HTMLEngine *engine;
	HTMLObject *obj;
	HTMLType type;

	if (!GTK_WIDGET_REALIZED (widget))
		return FALSE;

	html   = GTK_HTML (widget);
	engine = html->engine;
	obj    = html_engine_get_object_at (engine, x + engine->x_offset, y + engine->y_offset, NULL, FALSE);

	if ((html->in_selection || html->in_selection_drag) && html->allow_selection) {
		gboolean need_scroll;

		if (obj) {
			type = HTML_OBJECT_TYPE (obj);

			/* FIXME this is broken */

			if (type == HTML_TYPE_BUTTON ||
			    type ==  HTML_TYPE_CHECKBOX ||
			    type ==  HTML_TYPE_EMBEDDED ||
			    type ==  HTML_TYPE_HIDDEN ||
			    type ==  HTML_TYPE_IMAGEINPUT ||
			    type ==  HTML_TYPE_RADIO ||
			    type ==  HTML_TYPE_SELECT ||
			    type ==  HTML_TYPE_TEXTAREA ||
			    type ==  HTML_TYPE_TEXTINPUT ) {
				return FALSE;
			}
		}

		if (HTML_DIST ((x + engine->x_offset - html->selection_x1),
			       (y + engine->y_offset - html->selection_y1)) 
		    > html_painter_get_space_width (engine->painter, 
						    GTK_HTML_FONT_STYLE_SIZE_3,  
						    NULL)) {
			html->in_selection = TRUE;
			html->in_selection_drag = TRUE;
		}

		need_scroll = FALSE;

		if (x < 0) {
			x = 0;
			need_scroll = TRUE;
		} else if (x >= widget->allocation.width) {
			x = widget->allocation.width - 1;
			need_scroll = TRUE;
		}

		if (y < 0) {
			y = 0;
			need_scroll = TRUE;
		} else if (y >= widget->allocation.height) {
			y = widget->allocation.height - 1;
			need_scroll = TRUE;
		}

		if (need_scroll)
			setup_scroll_timeout (html);
		else
			remove_scroll_timeout (html);

		/* This will put the mark at the position of the
                   previous click.  */
		if (engine->mark == NULL && engine->editable)
			html_engine_set_mark (engine);

		html_engine_select_region (engine, html->selection_x1,
					   html->selection_y1,
					   x + engine->x_offset, 
					   y + engine->y_offset);
	}

	on_object (widget, window, obj);

	return TRUE;
}

static const char *
skip_host (const char *url)
{
	const char *host;
	
	host = url;
	while (*host && (*host != '/') && (*host != ':'))
	       host++;

	if (*host == ':') {
		url = host++;

		if (*host == '/') 
			host++;

		url = host;

		if (*host == '/') {
			host++;

			while (*host && (*host != '/'))
				host++;
			
			url = host;
		}
	}		
	
	return url;
}
	
static size_t
path_len (const char *base, gboolean absolute)
{
	const char *last;
	const char *cur;
	const char *start;

	start = last = skip_host (base);
	if (!absolute) {
		cur = strrchr (start, '/');
		
		if (cur)
			last = cur;
	}

	return last - base;
}

#if 0
char *
collapse_path (char *url)
{
	char *start;
	char *end;
	char *cur;
	size_t len;

	start = skip_host (url);

	cur = start;
	while ((cur = strstr (cur, "/../"))) {
		end = cur + 3;
		
		/* handle the case of a rootlevel /../ specialy */
		if (cur == start) {
			len = strlen (end);
			memmove (cur, end, len + 1);
		}
			
		while (cur > start) {
			cur--;
			if ((*cur == '/') || (cur == start)) {
				len = strlen (end);
				memmove (cur, end, len + 1);
				break;
			}
		}
	}
	return url;
}
#endif

static char *
expand_relative (const char *base, const char *url)
{
	char *new_url = NULL;
	size_t base_len, url_len;
	gboolean absolute = FALSE;

	if (!base || (url && strstr (url, ":"))) {
		/*
		  g_warning ("base = %s url = %s new_url = %s",
		  base, url, new_url);
		*/
		return g_strdup (url);
	}		

	if (*url == '/') {
		absolute = TRUE;;
	}
	
	base_len = path_len (base, absolute);
	url_len = strlen (url);

	new_url = g_malloc (base_len + url_len + 2);
	
	if (base_len) {
		memcpy (new_url, base, base_len);

		if (base[base_len - 1] != '/')
			new_url[base_len++] = '/';
		if (absolute)
			url++;
	}
	
	memcpy (new_url + base_len, url, url_len);
	new_url[base_len + url_len] = '\0';
	
	/* 
	   g_warning ("base = %s url = %s new_url = %s", 
	   base, url, new_url);
	*/
	return new_url;
}

char *
gtk_html_get_url_base_relative (GtkHTML *html, const char *url)
{
	return expand_relative (gtk_html_get_base (html), url);
}

static char *
expand_frame_url (GtkHTML *html, const char *url)
{
	char *new_url;

	new_url = gtk_html_get_url_base_relative (html, url);
	while (html->iframe_parent) {
		char *expanded;

		expanded = gtk_html_get_url_base_relative (GTK_HTML (html->iframe_parent), 
						       new_url);
		g_free (new_url);
		new_url = expanded;

		html = GTK_HTML (html->iframe_parent);
	}
	return new_url;
}

char *
gtk_html_get_url_object_relative (GtkHTML *html, HTMLObject *o, const char *url)
{
	HTMLEngine *e;
	HTMLObject *parent;

	g_return_val_if_fail (GTK_IS_HTML (html), NULL);

	/* start at the top always */
	while (html->iframe_parent)
		html = GTK_HTML (html->iframe_parent);
	
	parent = o;
	while (parent->parent) {
		parent = parent->parent;
		if ((HTML_OBJECT_TYPE (parent) == HTML_TYPE_FRAME)
		    || (HTML_OBJECT_TYPE (parent) == HTML_TYPE_IFRAME))
			break;
	}

	e = html_object_get_engine (parent, html->engine);
	
	if (!e) {
		g_warning ("Cannot find object for url");
		return NULL;
	}

	/*
	if (e == html->engine)
		g_warning ("engine matches engine");
	*/
        return url ? expand_frame_url (e->widget, url) : NULL;
}
	
static GtkWidget *
shift_to_iframe_parent (GtkWidget *widget, gint *x, gint *y)
{
	while (GTK_HTML (widget)->iframe_parent) {
		if (x)
			*x += widget->allocation.x;
		if (y)
			*y += widget->allocation.y;

		widget = GTK_HTML (widget)->iframe_parent;
		
			
	}

	return widget;
}

static gint
motion_notify_event (GtkWidget *widget,
		     GdkEventMotion *event)
{
	GdkWindow *window = widget->window;
	HTMLEngine *engine;
	gint x, y;

	g_return_val_if_fail (widget != NULL, 0);
	g_return_val_if_fail (GTK_IS_HTML (widget), 0);
	g_return_val_if_fail (event != NULL, 0);

	/* printf ("motion_notify_event\n"); */

	if (GTK_HTML (widget)->priv->dnd_in_progress)
		return TRUE;

	if (!event->is_hint) {
		x = event->x;
		y = event->y;
	}
	widget = shift_to_iframe_parent (widget, &x, &y);

	if (event->is_hint) {
		gdk_window_get_pointer (GTK_LAYOUT (widget)->bin_window, &x, &y, NULL);
	}

	if (!mouse_change_pos (widget, window, x, y))
		return FALSE;

	engine = GTK_HTML (widget)->engine;
	if (GTK_HTML (widget)->in_selection_drag && html_engine_get_editable (engine))
		html_engine_jump_at (engine,
				     x + engine->x_offset,
				     y + engine->y_offset);
	return TRUE;
}

static gint
button_press_event (GtkWidget *widget,
		    GdkEventButton *event)
{
	GtkHTML *html;
	GtkWidget *orig_widget = widget;
	HTMLEngine *engine;
	gint value, x, y;

	/* printf ("button_press_event\n"); */

	x = event->x;
	y = event->y;

	widget = shift_to_iframe_parent (widget, &x, &y);
	html   = GTK_HTML (widget);
	engine = html->engine;

	if (event->button == 1 || ((event->button == 2 || event->button == 3)
				   && html_engine_get_editable (engine)))
		gtk_widget_grab_focus (widget);

	if (event->type == GDK_BUTTON_PRESS) {
		GtkAdjustment *vadj;

		vadj   = GTK_LAYOUT (widget)->vadjustment;
		
		switch (event->button) {
		case 4:
			/* Mouse wheel scroll up.  */
			if (event->state & GDK_CONTROL_MASK)
				gtk_html_command (html, "zoom-out");
			else {
				value = vadj->value - vadj->step_increment * 3;
			
				if (value < vadj->lower)
					value = vadj->lower;
			
				gtk_adjustment_set_value (vadj, value);
			}
			return TRUE;
			break;
		case 5:
			/* Mouse wheel scroll down.  */
			if (event->state & GDK_CONTROL_MASK) 
				gtk_html_command (html, "zoom-in");
			else {
				value = vadj->value + vadj->step_increment * 3;
			
				if (value > (vadj->upper - vadj->page_size))
					value = vadj->upper - vadj->page_size;
			
				gtk_adjustment_set_value (vadj, value);
			}
			return TRUE;
			break;
		case 2:
			if (html_engine_get_editable (engine)) {
				html_engine_disable_selection (html->engine);
				html_engine_jump_at (engine,
						     x + engine->x_offset,
						     y + engine->y_offset);
				gtk_html_update_styles (html);
				gtk_html_request_paste (html, GDK_SELECTION_PRIMARY, 
							event->state & GDK_CONTROL_MASK ? 1 : 0,
							event->time, event->state & GDK_SHIFT_MASK);
				return TRUE;
			}
			break;
		case 1:
			html->in_selection_drag = TRUE;
			if (html_engine_get_editable (engine)) {
				if (html->allow_selection)
					if (!(event->state & GDK_SHIFT_MASK)
					    || (!engine->mark && event->state & GDK_SHIFT_MASK))
						html_engine_set_mark (engine);
				html_engine_jump_at (engine, x + engine->x_offset, y + engine->y_offset);
			} else {
				HTMLObject *obj;
				HTMLEngine *orig_e;

				orig_e = GTK_HTML (orig_widget)->engine;
				obj = html_engine_get_object_at (engine, x + engine->x_offset, y + engine->y_offset,
								 NULL, FALSE);
				if (obj && ((HTML_IS_IMAGE (obj) && HTML_IMAGE (obj)->url && *HTML_IMAGE (obj)->url)
					    || HTML_IS_LINK_TEXT (obj)))
					html_engine_set_focus_object (orig_e, obj);
				else
					html_engine_set_focus_object (orig_e, NULL);
  			}
			if (html->allow_selection) {
				if (event->state & GDK_SHIFT_MASK)
					html_engine_select_region (engine,
								   html->selection_x1, html->selection_y1,
								   x + engine->x_offset, y + engine->y_offset);
				else {
					html_engine_disable_selection (engine);
					if (gdk_pointer_grab (GTK_LAYOUT (widget)->bin_window, FALSE,
							      (GDK_BUTTON_RELEASE_MASK
							       | GDK_BUTTON_MOTION_MASK
							       | GDK_POINTER_MOTION_HINT_MASK),
							      NULL, NULL, 0) == 0) {
						html->selection_x1 = x + engine->x_offset;
						html->selection_y1 = y + engine->y_offset;
					}
				}
			}

			engine->selection_mode = FALSE;
			if (html_engine_get_editable (engine))
				gtk_html_update_styles (html);
			break;
		default:
			break;
		}
	} else if (event->button == 1 && html->allow_selection) {
		if (event->type == GDK_2BUTTON_PRESS) {
			html->in_selection_drag = FALSE;
			gtk_html_select_word (html);
			html->in_selection = TRUE;
		}
		else if (event->type == GDK_3BUTTON_PRESS) {
			html->in_selection_drag = FALSE;
			gtk_html_select_line (html);
			html->in_selection = TRUE;
		}
	}

	return TRUE;
}

static gint
button_release_event (GtkWidget *initial_widget,
		      GdkEventButton *event)
{
	GtkWidget *widget;
	GtkHTML *html;
	HTMLEngine *engine;
	gint x, y;

	/* printf ("button_release_event\n"); */

	x = event->x;
	y = event->y;
	widget = shift_to_iframe_parent (initial_widget, &x, &y);
	html   = GTK_HTML (widget);

	remove_scroll_timeout (html);
	gtk_grab_remove (widget);
	gdk_pointer_ungrab (0);
	
	engine = html->engine;

	if (html->in_selection) {
		if (html->in_selection_drag)
			html_engine_select_region (engine, html->selection_x1, html->selection_y1,
						   x + engine->x_offset, y + engine->y_offset);
		html_engine_update_selection_active_state (engine, html->priv->event_time);
		gtk_html_update_styles (html);
		queue_draw (html);
	}

	if (event->button == 1) {
	
		if (html->in_selection_drag && html_engine_get_editable (engine)) 
			html_engine_jump_at (engine, x + engine->x_offset, y + engine->y_offset); 

		html->in_selection_drag = FALSE;
		
		if (!html->priv->dnd_in_progress
		    && html->pointer_url != NULL && ! html->in_selection)
			gtk_signal_emit (GTK_OBJECT (widget), 
					 signals[LINK_CLICKED], 
					 html->pointer_url);
	}

	html->in_selection = FALSE;

	return TRUE;
}

static gint
focus_in_event (GtkWidget *widget,
		GdkEventFocus *event)
{
	GtkHTML *html = GTK_HTML (widget);

	/* printf ("focus in\n"); */
	if (!html->iframe_parent) {
		GTK_WIDGET_SET_FLAGS (widget, GTK_HAS_FOCUS);
		html_engine_set_focus (html->engine, TRUE);
	} else {
		GtkWidget *window = gtk_widget_get_ancestor (widget, gtk_window_get_type ());
		if (window)
			gtk_window_set_focus (GTK_WINDOW (window), html->iframe_parent);
	}

#ifdef GTK_HTML_USE_XIM
	gtk_html_im_focus_in (html);
#endif

	return FALSE;
}

static gint
focus_out_event (GtkWidget *widget,
		 GdkEventFocus *event)
{
	GtkHTML *html = GTK_HTML (widget);

	html_painter_set_focus (html->engine->painter, FALSE);
	html_engine_redraw_selection (html->engine);
	/* printf ("focus out\n"); */
	if (!html->iframe_parent) {
		GTK_WIDGET_UNSET_FLAGS (widget, GTK_HAS_FOCUS);
		html_engine_set_focus (html->engine, FALSE);
	}

#ifdef GTK_HTML_USE_XIM
	gtk_html_im_focus_out (html);
#endif

	return FALSE;
}

static gint
enter_notify_event (GtkWidget *widget, GdkEventCrossing *event)
{
	gint x, y;

	x = event->x;
	y = event->y;
	widget = shift_to_iframe_parent (widget, &x, &y);

	mouse_change_pos (widget, widget->window, x, y);

	return TRUE;
}


/* X11 selection support.  */

static guchar *
replace_nbsp (const guchar *text)
{
	guchar *nt, *ntext = g_new (guchar, strlen (text) + 1);
	gint pos = 0;

	nt = ntext;
	while (*text) {
		if (*text == 0xc2 && pos == 0)
			pos = 1;
		else if (*text == 0xa0 && pos == 1) {
			*nt = ' ';
			nt ++;
			pos = 0;
		} else {
			if (pos) {
				*nt = 0xc2;
				nt ++;
				pos = 0;
			}
			*nt = *text;
			nt ++;
		}
		text ++;
	}

	*nt = 0;

	return ntext;
}

static char *
ucs2_order (gboolean swap)
{
	gboolean be;

	be = G_BYTE_ORDER == G_BIG_ENDIAN;
	be = swap ? be : !be;	

	if (be)
		return "ucs-2be";
	else 
		return "ucs-2le";
	
}

static gint
ucs2_len (guchar *buffer) {
	guchar *p;
	
	for (p = buffer; p != 0; p += 2)
		if (*p == 0 && *(p + 1) == 0)
			return p - buffer;

	/* not reached */
	return 0;
}

static void
string_append_unichar (GString *string, gunichar uc)
{
	guchar bits[7], *p;
	gint   len;
	
	len = g_unichar_to_utf8 (uc, bits);

	p = bits;
	while (len--)
		g_string_append_c (string, *p++);
}

static void
selection_get (GtkWidget        *widget, 
	       GtkSelectionData *selection_data,
	       guint             info,
	       guint             time)
{
	GtkHTML *html;
	gchar *selection_string = NULL;
	gchar *localized_string = NULL;
	HTMLObject *selection_object = NULL;
	guint selection_object_len = 0;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_HTML (widget));
	
	html = GTK_HTML (widget);
	if (selection_data->selection == GDK_SELECTION_PRIMARY) {
		if (html->engine->primary) {
			selection_object = html->engine->primary;
			selection_object_len = html->engine->primary_len;
		}			
	} else	/* CLIPBOARD */ {
		if (html->engine->clipboard) {
			selection_object = html->engine->clipboard;
			selection_object_len = html->engine->clipboard_len;
		}
	}

	/*
	 * FIXME we should make e_utf8_to/from_string_target and 
	 * e_utf8_to/from_compound_string in e-unicode.c but I'll wait until
	 * more severe bugs have been fixed.  NOTE: by the conventions we should
	 * follow (gtk+-2.0) STRING should _allways_ be iso-8859-1 and COMPOUND_TEXT
	 * should be localized.
	 */
	
	if (info == TARGET_HTML) {
		if (selection_object) {
			HTMLEngineSaveState *state;
			GString *buffer;

			state = html_engine_save_buffer_new (html->engine, TRUE);
			buffer = (GString *)state->user_data;

			/* prepend a byte order marker (ZWNBSP) to the selection */
			string_append_unichar (buffer, 0xfeff);
			html_object_save (selection_object, state);
			
			d_s (g_warning ("text/html buffer = %s", buffer->str);)
			selection_string = e_utf8_to_charset_string_sized ("ucs-2", buffer->str, buffer->len);
			
			if (selection_string)
				gtk_selection_data_set (selection_data,
							gdk_atom_intern ("text/html", FALSE), 16,
							selection_string,
							ucs2_len (selection_string));
			
			html_engine_save_buffer_free (state);
		}				
	} else {
		if (selection_object)
			selection_string = html_object_get_selection_string (selection_object, html->engine);

		if (selection_string != NULL) {
			if (info == TARGET_UTF8_STRING) {
				/* printf ("UTF8_STRING\n"); */
				gtk_selection_data_set (selection_data,
							gdk_atom_intern ("UTF8_STRING", FALSE), 8,
							(const guchar *) selection_string,
							strlen (selection_string));
			} else if (info == TARGET_UTF8) {
				/* printf ("UTF-8\n"); */
				gtk_selection_data_set (selection_data,
							gdk_atom_intern ("UTF-8", FALSE), 8,
							(const guchar *) selection_string,
							strlen (selection_string));
			} else if (info == TARGET_STRING || info == TARGET_TEXT || info == TARGET_COMPOUND_TEXT) {
				gchar *to_be_freed;
				
				to_be_freed = selection_string;
				selection_string = replace_nbsp (selection_string);
				g_free (to_be_freed);
				localized_string = e_utf8_to_gtk_string (widget,
								 selection_string);
				
				if (info == TARGET_STRING) {
					/* printf ("STRING\n"); */
					gtk_selection_data_set (selection_data,
								GDK_SELECTION_TYPE_STRING, 8,
								(const guchar *) localized_string, 
								strlen (localized_string));
				} else {
					guchar *text;
					GdkAtom encoding;
					gint format;
					gint new_length;
					
					/* printf ("TEXT or COMPOUND_TEXT\n"); */
					gdk_string_to_compound_text (localized_string, 
								     &encoding, &format,
								     &text, &new_length);
					
					gtk_selection_data_set (selection_data,
								encoding, format,
								text, new_length);
					gdk_free_compound_text (text);
				}
				
			}
		}
		g_free (selection_string);
		g_free (localized_string);
	}
}

/* receive a selection */
/* Signal handler called when the selections owner returns the data */
static void
selection_received (GtkWidget *widget,
		    GtkSelectionData *selection_data, 
		    guint time)
{
	HTMLEngine *e;
	gboolean as_cite;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_HTML (widget));
	g_return_if_fail (selection_data != NULL);
	
	/* printf ("got selection from system\n"); */

	e = GTK_HTML (widget)->engine;
	as_cite = GTK_HTML (widget)->priv->selection_as_cite;

	/* If the Widget is editable,
	** and we are the owner of the atom requested
	** and we are not pasting as a citation 
	** then we are pasting between ourself and we
	** need not do all the conversion.
	*/
	if (html_engine_get_editable (e)
	    && widget->window == gdk_selection_owner_get (selection_data->selection)
	    && !as_cite) {

		/* Check which atom was requested (PRIMARY or CLIPBOARD) */
		if (selection_data->selection == gdk_atom_intern ("CLIPBOARD", FALSE)
		    && e->clipboard) {

			html_engine_paste (e);
			return;

		} else if (selection_data->selection == GDK_SELECTION_PRIMARY
			   && e->primary) {
			HTMLObject *copy;
			guint len = 0;

			copy = html_object_op_copy (e->primary, NULL, e, NULL, NULL, &len);
			html_engine_paste_object (e, copy, len);

			return;
		}
	}

	/* **** IMPORTANT **** Check to see if retrieval succeeded  */
	/* If we have more selection types we can ask for, try the next one,
	   until there are none left */
	if (selection_data->length < 0) {
		gint type = GTK_HTML (widget)->priv->selection_type;

		/* now, try again with next selection type */
		if (!gtk_html_request_paste (GTK_HTML (widget), selection_data->selection, type + 1, time, as_cite))
			g_warning ("Selection retrieval failed\n");
		return;
	}

	/* Make sure we got the data in the expected form */
	if ((selection_data->type != gdk_atom_intern ("UTF8_STRING", FALSE))
	    && (selection_data->type != GDK_SELECTION_TYPE_STRING)
	    && (selection_data->type != gdk_atom_intern ("UTF-8", FALSE))
	    && (selection_data->type != gdk_atom_intern ("text/html", FALSE))) {
		g_warning ("Selection \"STRING\" was not returned as strings!\n");
	} else if (selection_data->length > 0) {
		if (selection_data->type == gdk_atom_intern ("text/html", FALSE)) {
			guint    len  = (guint)selection_data->length;
			guchar  *data = selection_data->data;
			gchar   *utf8 = NULL;

      			/* 
			 * FIXME This hack decides the charset of the selection.  It seems that
			 * mozilla/netscape alway use ucs2 for text/html
			 * and openoffice.org seems to always use utf8 so we try to validate
			 * the string as utf8 and if that fails we assume it is ucs2 
			 */

			if (len > 1 && 
			    !g_utf8_validate (data, len - 1, NULL)) {
				char    *tocode = NULL;
				guint16 c;

				/*
				 * Unicode Techinical Report 20 ( http://www.unicode.org/unicode/reports/tr20/ )
				 * says to treat an initial 0xfeff (ZWNBSP) as a byte order indicator so that is
				 * what we do.  If there is no indicator assume it is in the default order
				 */
				memcpy (&c, data, 2);
				switch (c) {
				case 0xfeff:
				case 0xfffe:
					tocode = ucs2_order (c == 0xfeff);
					data += 2;
					len  -= 2;
					break;
				default:
					tocode = ucs2_order(1);
					break;
				}
				
				utf8 = e_utf8_from_charset_string_sized (tocode,
									 data,
									 len);

				d_s (g_warning ("UCS2 selection = %s", utf8);)
			} else if (len > 0) {
				d_s (g_warning ("UTF-8 selection (%d) = %s", len, data);)

				utf8 = g_malloc0 (len + 1);
				memcpy (utf8, data, len);
			} else {
				g_warning ("unable to determine selection charset");
				return;
			}

			if (as_cite) {
				char *cite;

				cite = g_strdup_printf ("<br><blockquote type=\"cite\">%s</blockquote>", utf8);

				g_free (utf8);
				utf8 = cite;
			}
			gtk_html_insert_html (GTK_HTML (widget), utf8);

			g_free (utf8);			
		} else  {
			char *text = NULL;

			if (selection_data->type != GDK_SELECTION_TYPE_STRING) {
				text = g_malloc0 (selection_data->length + 1);
				memcpy (text, selection_data->data, selection_data->length);
			} else {
				text = e_utf8_from_gtk_string_sized (widget, 
								     selection_data->data,
								     (guint) selection_data->length);
			}

			if (as_cite) {
				char *encoded;
				
				/* FIXME there has to be a cleaner way to do this */
				encoded = html_encode_entities (text, g_utf8_strlen (text, -1), NULL);
				g_free (text);
				text = g_strdup_printf ("<br><blockquote type=\"cite\"><pre>%s</pre></blockquote>", 
							encoded);
				g_free (encoded);
				gtk_html_insert_html (GTK_HTML (widget), text);
			} else {
				html_engine_paste_text (e, text, g_utf8_strlen (text, -1));
			}
			g_free (text);

			if (HTML_IS_TEXT (e->cursor->object))
				html_text_magic_link (HTML_TEXT (e->cursor->object), e,
						      html_object_get_length (e->cursor->object));
		}
		return;
	}

	if (html_engine_get_editable (e))
		html_engine_paste (e);
}

gint
gtk_html_request_paste (GtkHTML *html, GdkAtom selection, gint type, gint32 time, gboolean as_cite)
{
	GdkAtom format_atom;
	static char *formats[] = {"text/html", "UTF8_STRING", "UTF-8", "STRING"};

	if (type >= sizeof (formats) / sizeof (formats[0])) {
		/* we have now tried all the slection types we support */
		html->priv->selection_type = -1;
		if (html_engine_get_editable (html->engine))
			html_engine_paste (html->engine);
		return FALSE;
	}
	
	html->priv->selection_type = type;
	html->priv->selection_as_cite = as_cite;

	format_atom = gdk_atom_intern (formats [type], FALSE);
	
	if (format_atom == GDK_NONE) {
		g_warning("Could not get requested atom\n");
	}
	/* And request the format target for the required selection */
	gtk_selection_convert (GTK_WIDGET (html), selection, format_atom,
			       time);
	return TRUE;
}


static gint
selection_clear_event (GtkWidget *widget,
		       GdkEventSelection *event)
{
	GtkHTML *html;

	if (! gtk_selection_clear (widget, event))
		return FALSE;

	html = GTK_HTML (widget);

	if (!html_engine_get_editable (html->engine)) {
		html_engine_disable_selection (html->engine);
		html->in_selection = FALSE;
	}

	return TRUE;
}


static void
set_adjustments (GtkLayout     *layout,
		 GtkAdjustment *hadj,
		 GtkAdjustment *vadj)
{
	GtkHTML *html = GTK_HTML(layout);

	connect_adjustments (html, hadj, vadj);
	
	if (parent_class->set_scroll_adjustments)
		(* parent_class->set_scroll_adjustments) (layout, hadj, vadj);
}


/* Initialization.  */

static gint
set_fonts_idle (GtkHTML *html)
{
	GtkHTMLClassProperties *prop = GTK_HTML_CLASS (GTK_OBJECT (html)->klass)->properties;

	if (html->engine) {
		html_font_manager_set_default (html_engine_gdk_font_manager (html->engine),
					       prop->font_var,      prop->font_fix,
					       prop->font_var_size, prop->font_var_points,
					       prop->font_fix_size, prop->font_fix_points);

		if (html->engine->painter->mag_fm)
			html_font_manager_set_default (html->engine->painter->mag_fm,
						       prop->font_var,      prop->font_fix,
						       prop->font_var_size, prop->font_var_points,
						       prop->font_fix_size, prop->font_fix_points);

		if (html->engine->clue) {
			html_object_reset (html->engine->clue);
			html_object_change_set_down (html->engine->clue, HTML_CHANGE_ALL);
			html_engine_calc_size (html->engine, FALSE);
			html_engine_schedule_update (html->engine);
		}
	}
#ifdef GTKHTML_HAVE_GCONF
	html->priv->set_font_id = 0;
#endif

	return FALSE;
}

#ifdef GTKHTML_HAVE_GCONF

static void
set_fonts (GtkHTML *html)
{
#ifdef GTKHTML_HAVE_GCONF
	if (!html->priv->set_font_id)
		html->priv->set_font_id = gtk_idle_add ((GtkFunction) set_fonts_idle, html);
#else
	set_fonts_idle (html);
#endif
}

static void
client_notify_widget (GConfClient* client,
		      guint cnxn_id,
		      GConfEntry* entry,
		      gpointer user_data)
{
	GtkHTML *html = (GtkHTML *) user_data;
	GtkHTMLClass *klass = GTK_HTML_CLASS (GTK_OBJECT (html)->klass);
	GtkHTMLClassProperties *prop = klass->properties;	
	gchar *tkey;
	GdkColor tcolor;

	g_assert (client == gconf_client);
	g_assert (entry->key);
	tkey = strrchr (entry->key, '/');
	g_assert (tkey);

	if (!strcmp (tkey, "/font_variable")) {
		g_free (prop->font_var);
		prop->font_var = gconf_client_get_string (client, entry->key, NULL);
		set_fonts (html);
	} else if (!strcmp (tkey, "/font_fixed")) {
		g_free (prop->font_fix);
		prop->font_fix = gconf_client_get_string (client, entry->key, NULL);
		set_fonts (html);
	} else if (!strcmp (tkey, "/font_variable_points")) {
		prop->font_var_points = gconf_client_get_bool (client, entry->key, NULL);
	} else if (!strcmp (tkey, "/font_fixed_points")) {
		prop->font_fix_points = gconf_client_get_bool (client, entry->key, NULL);
	} else if (!strcmp (tkey, "/font_variable_size")) {
		prop->font_var_size = gconf_client_get_int (client, entry->key, NULL);
		set_fonts (html);
	} else if (!strcmp (tkey, "/font_fixed_size")) {
		prop->font_fix_size = gconf_client_get_int (client, entry->key, NULL);
		set_fonts (html);
	} else if (!strcmp (tkey, "/link_color")) {
		g_free (prop->link_color);
		prop->link_color = g_strdup (gconf_client_get_string (client, entry->key, NULL));
		gdk_color_parse (prop->link_color, &tcolor);

		html_colorset_set_color (html->engine->defaultSettings->color_set, &tcolor, HTMLLinkColor);
		html_colorset_set_color (html->engine->settings->color_set, &tcolor, HTMLLinkColor);

		gtk_widget_queue_draw (GTK_WIDGET (html));
	} else if (!strcmp (tkey, "/alink_color")) {
		g_free (prop->alink_color);
		prop->alink_color = g_strdup (gconf_client_get_string (client, entry->key, NULL));
		gdk_color_parse (prop->alink_color, &tcolor);

		html_colorset_set_color (html->engine->defaultSettings->color_set, &tcolor, HTMLALinkColor);
		html_colorset_set_color (html->engine->settings->color_set, &tcolor, HTMLALinkColor);

		gtk_widget_queue_draw (GTK_WIDGET (html));
	} else if (!strcmp (tkey, "/vlink_color")) {
		g_free (prop->vlink_color);
		prop->vlink_color = g_strdup (gconf_client_get_string (client, entry->key, NULL));
		gdk_color_parse (prop->vlink_color, &tcolor);

		html_colorset_set_color (html->engine->defaultSettings->color_set, &tcolor, HTMLVLinkColor);
		html_colorset_set_color (html->engine->settings->color_set, &tcolor, HTMLVLinkColor);

		gtk_widget_queue_draw (GTK_WIDGET (html));
	} else if (!strcmp (tkey, "/live_spell_check")) {
		prop->live_spell_check = gconf_client_get_bool (client, entry->key, NULL);
		if (html_engine_get_editable (html->engine)) {
			if (prop->live_spell_check)
				html_engine_spell_check (html->engine);
			else
				html_engine_clear_spell_check (html->engine);
		}
	} else if (!strcmp (tkey, "/keybindings_theme")) {
		gchar *theme = gconf_client_get_string (client, entry->key, NULL);
		if (strcmp (theme, prop->keybindings_theme)) {
			g_free (prop->keybindings_theme);
			prop->keybindings_theme = theme;
			load_keybindings (klass);
		} else
			g_free (theme);
		set_editor_keybindings (html, html_engine_get_editable (html->engine));
	}
}

static void
client_notify_spell_widget (GConfClient* client, guint cnxn_id, GConfEntry* entry, gpointer user_data)
{
	GtkHTML *html = (GtkHTML *) user_data;
	GtkHTMLClass *klass = GTK_HTML_CLASS (GTK_OBJECT (html)->klass);
	GtkHTMLClassProperties *prop = klass->properties;	
	gchar *tkey;

	g_assert (client == gconf_client);
	g_assert (entry->key);
	tkey = strrchr (entry->key, '/');
	g_assert (tkey);

	if (!strcmp (tkey, "/spell_error_color_red")) {
		prop->spell_error_color.red = gconf_client_get_int (client, entry->key, NULL);
	} else if (!strcmp (tkey, "/spell_error_color_green")) {
		prop->spell_error_color.green = gconf_client_get_int (client, entry->key, NULL);
	} else if (!strcmp (tkey, "/spell_error_color_blue")) {
		prop->spell_error_color.blue = gconf_client_get_int (client, entry->key, NULL);
		html_colorset_set_color (html->engine->defaultSettings->color_set,
					 &prop->spell_error_color, HTMLSpellErrorColor);
		html_colorset_set_color (html->engine->settings->color_set,
					 &prop->spell_error_color, HTMLSpellErrorColor);
		if (html_engine_get_editable (html->engine) && !strcmp (tkey, "/spell_error_color_blue"))
			gtk_widget_queue_draw (GTK_WIDGET (html));
	} else if (!strcmp (tkey, "/language")) {
		g_free (prop->language);
		prop->language = g_strdup (gconf_client_get_string (client, entry->key, NULL));
		if (!html->engine->language)
			gtk_html_api_set_language (html);
	}
}

static void
client_notify_class (GConfClient* client,
		     guint cnxn_id,
		     GConfEntry* entry,
		     gpointer user_data)
{
	GtkHTMLClass *klass = (GtkHTMLClass *) user_data;
	GtkHTMLClassProperties *prop = klass->properties;	
	gchar *tkey;

	g_assert (client == gconf_client);
	g_assert (entry->key);
	tkey = strrchr (entry->key, '/');
	g_assert (tkey);

	if (!strcmp (tkey, "/magic_links")) {
		prop->magic_links = gconf_client_get_bool (client, entry->key, NULL);
	}
	if (!strcmp (tkey, "/magic_smileys")) {
		prop->magic_smileys = gconf_client_get_bool (client, entry->key, NULL);
	}
}

#endif

static GtkHTMLClassProperties *
get_class_properties (GtkHTML *html)
{
	GtkHTMLClass *klass;

	klass = GTK_HTML_CLASS (GTK_OBJECT (html)->klass);

	if (!klass->properties) {
		klass->properties = gtk_html_class_properties_new ();

#ifdef GTKHTML_HAVE_GCONF
		if (!gconf_is_initialized ()) {
		char *argv[] = { "gtkhtml", NULL };
		
		g_warning ("gconf is not initialized, please call gconf_init before using GtkHTML library. "
			   "Meanwhile it's initialized by gtkhtml itself.");
		gconf_init (1, argv, &gconf_error);
		if (gconf_error)
			g_error ("gconf error: %s\n", gconf_error->message);
		}
		
		gconf_client = gconf_client_get_default ();
		if (!gconf_client)
			g_error ("cannot create gconf_client\n");
		gconf_client_add_dir (gconf_client, GTK_HTML_GCONF_DIR, GCONF_CLIENT_PRELOAD_ONELEVEL, &gconf_error);
		if (gconf_error)
			g_error ("gconf error: %s\n", gconf_error->message);
		gconf_client_add_dir (gconf_client, GNOME_SPELL_GCONF_DIR, GCONF_CLIENT_PRELOAD_ONELEVEL, &gconf_error);
		if (gconf_error)
		g_error ("gconf error: %s\n", gconf_error->message);
		gtk_html_class_properties_load (klass->properties, gconf_client);
#else
		gtk_html_class_properties_load (klass->properties);
#endif
		load_keybindings (klass);
#ifdef GTKHTML_HAVE_GCONF
		gconf_client_notify_add (gconf_client, GTK_HTML_GCONF_DIR, client_notify_class, klass, NULL, &gconf_error);
		if (gconf_error)
			g_warning ("gconf error: %s\n", gconf_error->message);
#endif
	}

	return klass->properties;
}

static void
set_focus_child (GtkContainer *containter, GtkWidget *w)
{
	HTMLObject *o = NULL;

	while (w && !(o = gtk_object_get_data (GTK_OBJECT (w), "embeddedelement")))
		w = w->parent;

	if (o && !html_object_is_frame (o))
		html_engine_set_focus_object (GTK_HTML (containter)->engine, o);

	(*GTK_CONTAINER_CLASS (parent_class)->set_focus_child) (containter, w);
}

static gboolean
focus (GtkContainer *w, GtkDirectionType direction)
{
	HTMLEngine *e = GTK_HTML (w)->engine;

	if (html_engine_get_editable (e)) {
		gboolean rv;

		rv = (*GTK_CONTAINER_CLASS (parent_class)->focus) (w, direction);
		html_engine_set_focus (GTK_HTML (w)->engine, rv);
		return rv;
	}

	if (html_engine_focus (e, direction) && e->focus_object) {
		HTMLObject *cur, *obj = html_engine_get_focus_object (e);
		gint x1, y1, x2, y2, xo, yo;

		xo = e->x_offset;
		yo = e->y_offset;

		html_object_calc_abs_position (obj, &x1, &y1);
		x1 += e->leftBorder;
		y1 += e->topBorder;
		y2 = y1 + obj->descent;
		x2 = x1 + obj->width;
		y1 -= obj->ascent;

		/* correct coordinates for text slaves */
		if (html_object_is_text (obj) && obj->next) {
			cur = obj->next;
			while (cur && HTML_IS_TEXT_SLAVE (cur)) {
				gint xa, ya;
				html_object_calc_abs_position (cur, &xa, &ya);
				xa += e->leftBorder + cur->width;
				if (xa > x2)
					x2 = xa;
				ya += e->topBorder + cur->descent;
				if (ya > y2)
					y2 = ya;
				cur = cur->next;
			}
		}

		/* printf ("child pos: %d,%d x %d,%d\n", x1, y1, x2, y2); */

		if (x2 > e->x_offset + e->width)
			e->x_offset = x2 - e->width;
		if (x1 < e->x_offset)
			e->x_offset = x1;
		if (e->width > 2*RIGHT_BORDER && e->x_offset == x2 - e->width)
			e->x_offset = MIN (x2 - e->width + RIGHT_BORDER + 1,
					   html_engine_get_doc_width (e) - e->width);
		if (e->width > 2*LEFT_BORDER && e->x_offset >= x1)
			e->x_offset = MAX (x1 - LEFT_BORDER, 0);

		if (y2 >= e->y_offset + e->height)
			e->y_offset = y2 - e->height + 1;
		if (y1 < e->y_offset)
			e->y_offset = y1;
		if (e->height > 2*BOTTOM_BORDER && e->y_offset == y2 - e->height + 1)
			e->y_offset = MIN (y2 - e->height + BOTTOM_BORDER + 1,
					   html_engine_get_doc_height (e) - e->height);
		if (e->height > 2*TOP_BORDER && e->y_offset >= y1)
			e->y_offset = MAX (y1 - TOP_BORDER, 0);

		if (e->x_offset != xo)
			gtk_adjustment_set_value (GTK_LAYOUT (w)->hadjustment, (gfloat) e->x_offset);
		if (e->y_offset != yo)
			gtk_adjustment_set_value (GTK_LAYOUT (w)->vadjustment, (gfloat) e->y_offset);
		/* printf ("engine pos: %d,%d x %d,%d\n",
		   e->x_offset, e->y_offset, e->x_offset + e->width, e->y_offset + e->height); */

		if (!html_object_is_embedded (obj))
			gtk_widget_grab_focus (GTK_WIDGET (w));

		return TRUE;
	}

	gtk_widget_grab_focus (GTK_WIDGET (w));

	return FALSE;
}

/* dnd begin */

static void
drag_begin (GtkWidget *widget, GdkDragContext *context)
{
	HTMLInterval *i;
	HTMLObject *o;

	/* printf ("drag_begin\n"); */
	GTK_HTML (widget)->priv->dnd_real_object = o = GTK_HTML (widget)->priv->dnd_object;
	GTK_HTML (widget)->priv->dnd_in_progress = TRUE;

	i = html_interval_new (o, o, 0, html_object_get_length (o));
	html_engine_select_interval (GTK_HTML (widget)->engine, i);
}

static void
drag_end (GtkWidget *widget, GdkDragContext *context)
{
	/* printf ("drag_end\n"); */
	GTK_HTML (widget)->priv->dnd_in_progress = FALSE;
}

static void
drag_data_get (GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data, guint info, guint time)
{
	/* printf ("drag_data_get\n"); */
	switch (info) {
	case DND_TARGET_TYPE_TEXT_URI_LIST:
	case DND_TARGET_TYPE__NETSCAPE_URL: 
	case DND_TARGET_TYPE_TEXT_PLAIN:
	case DND_TARGET_TYPE_STRING: {
		HTMLObject *obj = GTK_HTML (widget)->priv->dnd_real_object;
		const gchar *url, *target;
		gchar *complete_url;

		/* printf ("\ttext/plain\n"); */
		if (obj) {
			/* printf ("obj %p\n", obj); */
			url = html_object_get_url (obj);
			target = html_object_get_target (obj);
			if (url && *url) {

				complete_url = g_strconcat (url, target && *target ? "#" : NULL, 
							    target, NULL);

				gtk_selection_data_set (selection_data, selection_data->target, 8,
							complete_url, strlen (complete_url));
				/* printf ("complete URL %s\n", complete_url); */
				GTK_HTML (widget)->priv->dnd_url = complete_url;
			}
		}
	}
	break;
	}
}

static void
drag_data_delete (GtkWidget *widget, GdkDragContext *context)
{
	g_free (GTK_HTML (widget)->priv->dnd_url);
	GTK_HTML (widget)->priv->dnd_url = NULL;
}

static gchar *
next_uri (guchar **uri_list, gint *len, gint *list_len)
{
	guchar *uri, *begin;

	begin = *uri_list;
	*len = 0;
	while (**uri_list && **uri_list != '\n' && **uri_list != '\r' && *list_len) {
		(*uri_list) ++;
		(*len) ++;
		(*list_len) --;
	}

	uri = g_strndup (begin, *len);

	while ((!**uri_list || **uri_list == '\n' || **uri_list == '\r') && *list_len) {
		(*uri_list) ++;
		(*list_len) --;
	}	

	return uri;
}

static gchar *pic_extensions [] = {
	".png",
	".gif",
	".jpg",
	".xbm",
	".xpm",
	".bmp",
	NULL
};

static gchar *known_protocols [] = {
	"http://",
	"ftp://",
	"nntp://",
	"news://",
	"mailto:",
	"file:",
	NULL
};

static HTMLObject *
new_obj_from_uri (HTMLEngine *e, gchar *uri, gint len)
{
	gint i;

	if (!strncmp (uri, "file:", 5)) {

		for (i = 0; pic_extensions [i]; i++) {
			if (!strcmp (uri + len - strlen (pic_extensions [i]), pic_extensions [i])) {
				return html_image_new (e->image_factory, uri,
						       NULL, NULL, -1, -1, FALSE, FALSE, 0,
						       html_colorset_get_color (e->settings->color_set, HTMLTextColor),
						       HTML_VALIGN_BOTTOM, TRUE);
			}
		}
	}

	for (i = 0; known_protocols [i]; i++) {
		if (!strncmp (uri, known_protocols [i], strlen (known_protocols [i]))) {
			return html_engine_new_link (e, uri, len, uri);
		}
	}	

	return NULL;
}

static void
move_before_paste (GtkWidget *widget, gint x, gint y)
{
	HTMLEngine *engine = GTK_HTML (widget)->engine;

	if (html_engine_is_selection_active (engine)) {
		HTMLObject *obj;
		guint offset;

		obj = html_engine_get_object_at (engine,
						 x + engine->x_offset,
						 y + engine->y_offset,
						 &offset, FALSE);
		if (!html_engine_point_in_selection (engine, obj, offset)) {
			html_engine_disable_selection (engine);
			html_engine_edit_selection_updater_update_now (engine->selection_updater);
		}
	}
	if (!html_engine_is_selection_active (engine)) {

		html_engine_jump_at (engine, x + engine->x_offset, y + engine->y_offset);
		gtk_html_update_styles (GTK_HTML (widget));
	}
}

static void
drag_data_received (GtkWidget *widget, GdkDragContext *context,
		    gint x, gint y, GtkSelectionData *selection_data, guint info, guint time)
{
	HTMLEngine *engine = GTK_HTML (widget)->engine;

	/* printf ("drag data received at %d,%d\n", x, y); */

	if (!selection_data->data || selection_data->length < 0 || !html_engine_get_editable (engine))
		return;

	move_before_paste (widget, x, y);

	switch (info) {
	case DND_TARGET_TYPE_TEXT_PLAIN:
	case DND_TARGET_TYPE_STRING:
		/* printf ("\ttext/plain\n"); */
		html_engine_paste_object (engine, html_engine_new_text (engine, selection_data->data, selection_data->length),
					  selection_data->length);
		break;
	case DND_TARGET_TYPE_TEXT_URI_LIST:
	case DND_TARGET_TYPE__NETSCAPE_URL: {
		HTMLObject *obj;
		gint list_len, len;
		gchar *uri;

		html_undo_level_begin (engine->undo, "Dropped URI(s)", "Remove Dropped URI(s)");
		list_len = selection_data->length;
		do {
			uri = next_uri (&selection_data->data, &len, &list_len);
			move_before_paste (widget, x, y);
			obj = new_obj_from_uri (engine, uri, len);
			if (obj)
				html_engine_paste_object (engine, obj, len);
			else {
				gtk_drag_finish (context, FALSE, FALSE, time);
				break;
			}
		} while (list_len);
		html_undo_level_end (engine->undo);
	}
	break;
	}
}

/* dnd end */

typedef void (*GtkSignal_NONE__INT_INT_FLOAT) (GtkObject * object,
					       gint arg1, gint arg2,
					       gfloat arg3, gpointer user_data);
static void
gtk_marshal_NONE__INT_INT_FLOAT (GtkObject * object,
				 GtkSignalFunc func,
				 gpointer func_data, GtkArg * args)
{
	GtkSignal_NONE__INT_INT_FLOAT rfunc;
	rfunc = (GtkSignal_NONE__INT_INT_FLOAT) func;
	(*rfunc) (object,
		  GTK_VALUE_INT (args[0]), GTK_VALUE_INT (args[1]), GTK_VALUE_FLOAT (args[2]), func_data);
}

static void
class_init (GtkHTMLClass *klass)
{
	GtkHTMLClass      *html_class;
	GtkWidgetClass    *widget_class;
	GtkObjectClass    *object_class;
	GtkLayoutClass    *layout_class;
	GtkContainerClass *container_class;
	
	html_class = (GtkHTMLClass *)klass;
	widget_class = (GtkWidgetClass *) klass;
	object_class = (GtkObjectClass *) klass;
	layout_class = (GtkLayoutClass *) klass;
	container_class = (GtkContainerClass *) klass;

	object_class->destroy = destroy;

	parent_class = gtk_type_class (GTK_TYPE_LAYOUT);

	signals [TITLE_CHANGED] = 
	  gtk_signal_new ("title_changed",
			  GTK_RUN_FIRST,
			  object_class->type,
			  GTK_SIGNAL_OFFSET (GtkHTMLClass, title_changed),
			  gtk_marshal_NONE__STRING,
			  GTK_TYPE_NONE, 1,
			  GTK_TYPE_STRING);
	signals [URL_REQUESTED] =
	  gtk_signal_new ("url_requested",
			  GTK_RUN_FIRST,
			  object_class->type,
			  GTK_SIGNAL_OFFSET (GtkHTMLClass, url_requested),
			  gtk_marshal_NONE__POINTER_POINTER,
			  GTK_TYPE_NONE, 2,
			  GTK_TYPE_STRING,
			  GTK_TYPE_POINTER);
	signals [LOAD_DONE] = 
	  gtk_signal_new ("load_done",
			  GTK_RUN_FIRST,
			  object_class->type,
			  GTK_SIGNAL_OFFSET (GtkHTMLClass, load_done),
			  gtk_marshal_NONE__NONE,
			  GTK_TYPE_NONE, 0);
	signals [LINK_CLICKED] =
	  gtk_signal_new ("link_clicked",
			  GTK_RUN_FIRST,
			  object_class->type,
			  GTK_SIGNAL_OFFSET (GtkHTMLClass, link_clicked),
			  gtk_marshal_NONE__STRING,
			  GTK_TYPE_NONE, 1,
			  GTK_TYPE_STRING);
	signals [SET_BASE] =
		gtk_signal_new ("set_base",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkHTMLClass, set_base),
				gtk_marshal_NONE__STRING,
				GTK_TYPE_NONE, 1,
				GTK_TYPE_STRING);
	signals [SET_BASE_TARGET] =
		gtk_signal_new ("set_base_target",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkHTMLClass, set_base_target),
				gtk_marshal_NONE__STRING,
				GTK_TYPE_NONE, 1,
				GTK_TYPE_STRING);
	
	signals [ON_URL] =
		gtk_signal_new ("on_url",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkHTMLClass, on_url),
				gtk_marshal_NONE__STRING,
				GTK_TYPE_NONE, 1,
				GTK_TYPE_STRING);
	
	signals [REDIRECT] =
		gtk_signal_new ("redirect",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkHTMLClass, redirect),
				gtk_marshal_NONE__POINTER_INT,
				GTK_TYPE_NONE, 2,
				GTK_TYPE_STRING,
				GTK_TYPE_INT);
	
	signals [SUBMIT] =
		gtk_signal_new ("submit",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkHTMLClass, submit),
				gtk_marshal_NONE__POINTER_POINTER_POINTER,
				GTK_TYPE_NONE, 3,
				GTK_TYPE_STRING,
				GTK_TYPE_STRING,
				GTK_TYPE_STRING);

	signals [OBJECT_REQUESTED] =
		gtk_signal_new ("object_requested",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkHTMLClass, object_requested),
				gtk_marshal_BOOL__POINTER,
				GTK_TYPE_BOOL, 1,
				GTK_TYPE_OBJECT);
	
	signals [CURRENT_PARAGRAPH_STYLE_CHANGED] =
		gtk_signal_new ("current_paragraph_style_changed",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkHTMLClass, current_paragraph_style_changed),
				gtk_marshal_NONE__INT,
				GTK_TYPE_NONE, 1,
				GTK_TYPE_INT);

	signals [CURRENT_PARAGRAPH_INDENTATION_CHANGED] =
		gtk_signal_new ("current_paragraph_indentation_changed",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkHTMLClass, current_paragraph_indentation_changed),
				gtk_marshal_NONE__INT,
				GTK_TYPE_NONE, 1,
				GTK_TYPE_INT);

	signals [CURRENT_PARAGRAPH_ALIGNMENT_CHANGED] =
		gtk_signal_new ("current_paragraph_alignment_changed",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkHTMLClass, current_paragraph_alignment_changed),
				gtk_marshal_NONE__INT,
				GTK_TYPE_NONE, 1,
				GTK_TYPE_INT);

	signals [INSERTION_FONT_STYLE_CHANGED] =
		gtk_signal_new ("insertion_font_style_changed",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkHTMLClass, insertion_font_style_changed),
				gtk_marshal_NONE__INT,
				GTK_TYPE_NONE, 1,
				GTK_TYPE_INT);
	
	signals [INSERTION_COLOR_CHANGED] =
		gtk_signal_new ("insertion_color_changed",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkHTMLClass, insertion_color_changed),
				gtk_marshal_NONE__POINTER,
				GTK_TYPE_NONE, 1,
				GTK_TYPE_POINTER);
	
	signals [SIZE_CHANGED] = 
		gtk_signal_new ("size_changed",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkHTMLClass, size_changed),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0);
	signals [IFRAME_CREATED] = 
		gtk_signal_new ("iframe_created",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkHTMLClass, iframe_created),
				gtk_marshal_NONE__POINTER,
				GTK_TYPE_NONE, 1,
				GTK_TYPE_HTML);

	/* signals for keybindings */
	signals [SCROLL] =
		gtk_signal_new ("scroll",
				GTK_RUN_LAST | GTK_RUN_ACTION,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkHTMLClass, scroll),
				gtk_marshal_NONE__INT_INT_FLOAT,
				GTK_TYPE_NONE, 3,
				GTK_TYPE_ORIENTATION,
				GTK_TYPE_SCROLL_TYPE, GTK_TYPE_FLOAT);

	signals [CURSOR_MOVE] =
		gtk_signal_new ("cursor_move",
				GTK_RUN_LAST | GTK_RUN_ACTION,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkHTMLClass, cursor_move),
				gtk_marshal_NONE__INT_INT,
				GTK_TYPE_NONE, 2, GTK_TYPE_DIRECTION_TYPE, GTK_TYPE_HTML_CURSOR_SKIP);

	signals [COMMAND] =
		gtk_signal_new ("command",
				GTK_RUN_LAST | GTK_RUN_ACTION,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkHTMLClass, command),
				gtk_marshal_NONE__ENUM,
				GTK_TYPE_NONE, 1, GTK_TYPE_HTML_COMMAND);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	object_class->destroy = destroy;
	
	widget_class->realize = realize;
	widget_class->unrealize = unrealize;
	widget_class->style_set = style_set;
	widget_class->draw = draw;
	widget_class->key_press_event = key_press_event;
	widget_class->expose_event  = expose;
	widget_class->size_allocate = size_allocate;
	widget_class->motion_notify_event = motion_notify_event;
	widget_class->button_press_event = button_press_event;
	widget_class->button_release_event = button_release_event;
	widget_class->focus_in_event = focus_in_event;
	widget_class->focus_out_event = focus_out_event;
	widget_class->enter_notify_event = enter_notify_event;
	widget_class->selection_get = selection_get;
	widget_class->selection_received = selection_received;
	widget_class->selection_clear_event = selection_clear_event;
	widget_class->drag_data_get = drag_data_get;
	widget_class->drag_data_delete = drag_data_delete;
	widget_class->drag_begin = drag_begin;
	widget_class->drag_end = drag_end;
	widget_class->drag_data_received = drag_data_received;

	container_class->focus = focus;
	container_class->set_focus_child = set_focus_child;

	layout_class->set_scroll_adjustments = set_adjustments;

	html_class->scroll            = scroll;
	html_class->cursor_move       = cursor_move;
	html_class->command           = command;

	gdk_rgb_init ();
}

static void
init_properties_widget (GtkHTML *html)
{
	GdkColor tcolor;
	GtkHTMLClassProperties *prop;


	prop = get_class_properties (html);

	set_fonts_idle (html);
	html_colorset_set_color (html->engine->defaultSettings->color_set, &prop->spell_error_color, HTMLSpellErrorColor);

	gdk_color_parse (prop->link_color, &tcolor);
	html_colorset_set_color (html->engine->defaultSettings->color_set, &tcolor, HTMLLinkColor);
	/* html_colorset_set_color (html->engine->settings->color_set, &tcolor, HTMLLinkColor); */
	gdk_color_parse (prop->alink_color, &tcolor);
	html_colorset_set_color (html->engine->defaultSettings->color_set, &tcolor, HTMLALinkColor);
	/* html_colorset_set_color (html->engine->settings->color_set, &tcolor, HTMLALinkColor); */
	gdk_color_parse (prop->vlink_color, &tcolor);
	html_colorset_set_color (html->engine->defaultSettings->color_set, &tcolor, HTMLVLinkColor);
	/* html_colorset_set_color (html->engine->settings->color_set, &tcolor, HTMLVLinkColor); */

#ifdef GTKHTML_HAVE_GCONF

	html->priv->notify_id = gconf_client_notify_add (gconf_client, GTK_HTML_GCONF_DIR,
							 client_notify_widget, html, NULL, &gconf_error);
	if (gconf_error) {
		g_warning ("gconf error: %s\n", gconf_error->message);
		html->priv->notify_id = 0;
	}

	html->priv->notify_spell_id = gconf_client_notify_add (gconf_client, GNOME_SPELL_GCONF_DIR,
							       client_notify_spell_widget, html, NULL, &gconf_error);
	if (gconf_error) {
		g_warning ("gconf error: %s\n", gconf_error->message);
		html->priv->notify_spell_id = 0;
	}
#endif
}

static void
init (GtkHTML* html)
{
	static const GtkTargetEntry targets[] = {
		{ "text/html", 0, TARGET_HTML },
		{ "UTF8_STRING", 0, TARGET_UTF8_STRING },
		{ "UTF-8", 0, TARGET_UTF8 },
		{ "COMPOUND_TEXT", 0, TARGET_COMPOUND_TEXT },
		{ "STRING", 0, TARGET_STRING },
		{ "TEXT",   0, TARGET_TEXT }
	};
	static const gint n_targets = sizeof(targets) / sizeof(targets[0]);

	GTK_WIDGET_SET_FLAGS (GTK_WIDGET (html), GTK_CAN_FOCUS);
	GTK_WIDGET_SET_FLAGS (GTK_WIDGET (html), GTK_APP_PAINTABLE);

	html->editor_bindings = NULL;
	html->editor_api = NULL;
	html->debug = FALSE;
	html->allow_selection = TRUE;

	html->pointer_url = NULL;
	html->hand_cursor = gdk_cursor_new (GDK_HAND2);
	html->arrow_cursor = gdk_cursor_new (GDK_LEFT_PTR);
	html->ibeam_cursor = gdk_cursor_new (GDK_XTERM);
	html->hadj_connection = 0;
	html->vadj_connection = 0;

	html->selection_x1 = 0;
	html->selection_y1 = 0;

	html->in_selection = FALSE;
	html->in_selection_drag = FALSE;

	html->priv = g_new0 (GtkHTMLPrivate, 1);
	html->priv->idle_handler_id = 0;
	html->priv->scroll_timeout_id = 0;
	html->priv->paragraph_style = GTK_HTML_PARAGRAPH_STYLE_NORMAL;
	html->priv->paragraph_alignment = GTK_HTML_PARAGRAPH_ALIGNMENT_LEFT;
	html->priv->paragraph_indentation = 0;
	html->priv->insertion_font_style = GTK_HTML_FONT_STYLE_DEFAULT;
	html->priv->selection_type = -1;
	html->priv->selection_as_cite = FALSE;
	html->priv->content_type = NULL;
	html->priv->search_input_line = NULL;

#ifdef GTKHTML_HAVE_GCONF
	html->priv->set_font_id = 0;
	html->priv->notify_id = 0;
#endif
#ifdef GTK_HTML_USE_XIM
	html->priv->ic_attr = NULL;
	html->priv->ic = NULL;
#endif

	gtk_selection_add_targets (GTK_WIDGET (html),
				   GDK_SELECTION_PRIMARY,
				   targets, n_targets);
	gtk_selection_add_targets (GTK_WIDGET (html),
				   gdk_atom_intern ("CLIPBOARD", FALSE),
				   targets, n_targets);
}


guint
gtk_html_get_type (void)
{
	static guint html_type = 0;

	if (!html_type) {
		static const GtkTypeInfo html_info = {
			"GtkHTML",
			sizeof (GtkHTML),
			sizeof (GtkHTMLClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};
		
		html_type = gtk_type_unique (GTK_TYPE_LAYOUT, &html_info);
	}

	return html_type;
}

/**
 * gtk_html_new:
 * @void:
 *
 * GtkHTML widget contructor. It creates an empty GtkHTML widget.
 *
 * Return value: A GtkHTML widget, newly created and empty.
 **/

GtkWidget *
gtk_html_new (void)
{
	GtkWidget *html;

	html = gtk_type_new (gtk_html_get_type ());
	gtk_html_construct (html);
	return html;
}

/**
 * gtk_html_new_from_string:
 * @str: A string containing HTML source.
 * @len: A length of @str, if @len == -1 then it will be computed using strlen.
 *
 * GtkHTML widget constructor. It creates an new GtkHTML widget and loads HTML source from @str.
 * It is intended for simple creation. For more complicated loading you probably want to use
 * #GtkHTMLStream. See #gtk_html_begin.
 *
 * Return value: A GtkHTML widget, newly created, containing document loaded from input @str.
 **/

GtkWidget *
gtk_html_new_from_string (const gchar *str, gint len)
{
	GtkWidget *html;

	html = gtk_type_new (gtk_html_get_type ());
	gtk_html_construct  (html);
	gtk_html_load_from_string (GTK_HTML (html), str, len);

	return html;
}

void
gtk_html_construct (GtkWidget *widget)
{
	GtkHTML *html;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_HTML (widget));

	html = GTK_HTML (widget);

	html->engine        = html_engine_new (widget);
	html->iframe_parent = NULL;
	
	gtk_signal_connect (GTK_OBJECT (html->engine), "title_changed",
			    GTK_SIGNAL_FUNC (html_engine_title_changed_cb), html);
	gtk_signal_connect (GTK_OBJECT (html->engine), "set_base",
			    GTK_SIGNAL_FUNC (html_engine_set_base_cb), html);
	gtk_signal_connect (GTK_OBJECT (html->engine), "set_base_target",
			    GTK_SIGNAL_FUNC (html_engine_set_base_target_cb), html);
	gtk_signal_connect (GTK_OBJECT (html->engine), "load_done",
			    GTK_SIGNAL_FUNC (html_engine_load_done_cb), html);
	gtk_signal_connect (GTK_OBJECT (html->engine), "url_requested",
			    GTK_SIGNAL_FUNC (html_engine_url_requested_cb), html);
	gtk_signal_connect (GTK_OBJECT (html->engine), "draw_pending",
			    GTK_SIGNAL_FUNC (html_engine_draw_pending_cb), html);
	gtk_signal_connect (GTK_OBJECT (html->engine), "redirect",
			    GTK_SIGNAL_FUNC (html_engine_redirect_cb), html);
	gtk_signal_connect (GTK_OBJECT (html->engine), "submit",
			    GTK_SIGNAL_FUNC (html_engine_submit_cb), html);
	gtk_signal_connect (GTK_OBJECT (html->engine), "object_requested",
			    GTK_SIGNAL_FUNC (html_engine_object_requested_cb), html);

	init_properties_widget (html);
}


void
gtk_html_enable_debug (GtkHTML *html,
		       gboolean debug)
{
	g_return_if_fail (html != NULL);
	g_return_if_fail (GTK_IS_HTML (html));

	html->debug = debug;
}


void
gtk_html_allow_selection (GtkHTML *html,
			  gboolean allow)
{
	g_return_if_fail (html != NULL);
	g_return_if_fail (GTK_IS_HTML (html));

	html->allow_selection = allow;
}


/**
 * gtk_html_begin:
 * @html: the html widget to operate on.
 * 
 * Opens a new stream to load new content into the GtkHTML widget @html.
 *
 * Returns: a new GtkHTMLStream to store new content.
 **/
GtkHTMLStream *
gtk_html_begin (GtkHTML *html)
{
	g_return_val_if_fail (GTK_IS_HTML (html), NULL);

	return gtk_html_begin_content (html, html->priv->content_type);
}

/**
 * gtk_html_begin_content:
 * @html: the html widget to operate on.
 * @content_type: a string listing the type of content to expect on the stream.
 *
 * Opens a new stream to load new content of type @content_type into 
 * the GtkHTML widget given in @html.
 *
 * Returns: a new GtkHTMLStream to store new content.
 **/
GtkHTMLStream *
gtk_html_begin_content (GtkHTML *html, gchar *content_type)
{
	GtkHTMLStream *handle;

	g_return_val_if_fail (! gtk_html_get_editable (html), NULL);

	handle = html_engine_begin (html->engine, content_type);
	if (handle == NULL)
		return NULL;

	html_engine_parse (html->engine);

	return handle;
}

/**
 * gtk_html_write:
 * @html: the GtkHTML widget the stream belongs to (unused)
 * @handle: the GkHTMLStream to write to.
 * @buffer: the data to write to the stream.
 * @size: the length of data to read from @buffer
 * 
 * Writes @size bytes of @buffer to the stream pointed to by @stream.
 **/
void
gtk_html_write (GtkHTML *html,
		GtkHTMLStream *handle,
		const gchar *buffer,
		size_t size)
{
	gtk_html_stream_write (handle, buffer, size);
}

/**
 * gtk_html_end:
 * @html: the GtkHTML widget the stream belongs to.
 * @handle: the GtkHTMLStream to close.
 * @status: the GtkHTMLStreamStatus representing the state of the stream when closed.
 *
 * Close the GtkHTMLStream represented by @stream and notify @html that is 
 * should not expect any more content from that stream.
 **/
void
gtk_html_end (GtkHTML *html,
	      GtkHTMLStream *handle,
	      GtkHTMLStreamStatus status)
{
	gtk_html_stream_close (handle, status);
}


/**
 * gtk_html_get_title:
 * @html: The GtkHTML widget.
 *
 * Retrieve the title of the document currently loaded in the GtkHTML widget.
 *
 * Returns: the title of the current document
 **/
const gchar *
gtk_html_get_title (GtkHTML *html)
{
	g_return_val_if_fail (html != NULL, NULL);
	g_return_val_if_fail (GTK_IS_HTML (html), NULL);

	if (html->engine->title == NULL)
		return NULL;

	return html->engine->title->str;
}

/**
 * gtk_html_set_title:
 * @html: The GtkHTML widget.
 *
 * Set the title of the document currently loaded in the GtkHTML widget.
 *
 **/
void
gtk_html_set_title (GtkHTML *html, const char *title)
{
	g_return_if_fail (html != NULL);
	g_return_if_fail (GTK_IS_HTML (html));

	html_engine_set_title (html->engine, title); 
}


/**
 * gtk_html_jump_to_anchor:
 * @html: the GtkHTML widget.
 * @anchor: a string containing the name of the anchor.
 *
 * Scroll the document display to show the HTML anchor listed in @anchor
 *
 * Returns: TRUE if the anchor is found, FALSE otherwise.
 **/
gboolean
gtk_html_jump_to_anchor (GtkHTML *html,
			 const gchar *anchor)
{
	g_return_val_if_fail (html != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_HTML (html), FALSE);
	
	return html_engine_goto_anchor (html->engine, anchor);
}


gboolean
gtk_html_save (GtkHTML *html,
	       GtkHTMLSaveReceiverFn receiver,
	       gpointer data)
{
	g_return_val_if_fail (html != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_HTML (html), FALSE);
	g_return_val_if_fail (receiver != NULL, FALSE);
	
	return html_engine_save (html->engine, receiver, data);
}

/**
 * gtk_html_export:
 * @html: the GtkHTML widget
 * @content_type: the expected content_type 
 * @receiver: 
 * @user_data: pointer to maintain user state. 
 *
 * Export the current document into the content type given by @content_type,
 * by calling the function listed in @receiver data becomes avaiable.  When @receiver is 
 * called @user_data is passed in as the user_data parameter.
 * 
 * Returns: TRUE if the export was successfull, FALSE otherwise.
 **/
gboolean
gtk_html_export (GtkHTML *html,
		 const char *content_type,
		 GtkHTMLSaveReceiverFn receiver,
		 gpointer user_data)
{
	g_return_val_if_fail (html != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_HTML (html), FALSE);
	g_return_val_if_fail (receiver != NULL, FALSE);
	
	if (strcmp (content_type, "text/html") == 0) {
		return html_engine_save (html->engine, receiver, user_data);
	} else if (strcmp (content_type, "text/plain") == 0) {
		return html_engine_save_plain (html->engine, receiver,
					       user_data);  
	} else {
		return FALSE;
	}
}



static void
gtk_html_update_scrollbars_on_resize (GtkHTML *html,
				      gdouble old_doc_width, gdouble old_doc_height,
				      gdouble old_width, gdouble old_height,
				      gboolean *changed_x, gboolean *changed_y)
{
	GtkLayout *layout;
	GtkAdjustment *vadj, *hadj;
	gdouble doc_width, doc_height;

	/* printf ("update on resize\n"); */

	layout = GTK_LAYOUT (html);
	hadj = layout->hadjustment;
	vadj = layout->vadjustment;

	doc_height = html_engine_get_doc_height (html->engine);
	doc_width  = html_engine_get_doc_width (html->engine);

	if (old_doc_width - old_width > 0) {
		html->engine->x_offset = (gint) (hadj->value * (doc_width - html->engine->width)
						 / (old_doc_width - old_width));

		/* hacky part, I set layout {x,y}offset by hand to avoid unneccessary flicker */
		if (layout->xoffset != html->engine->x_offset) {
			layout->xoffset = html->engine->x_offset;
			if (changed_x)
				*changed_x = TRUE;
		}
		gtk_adjustment_set_value (hadj, html->engine->x_offset);
	}

	if (old_doc_height - old_height > 0) {
		html->engine->y_offset = (gint) (vadj->value * (doc_height - html->engine->height)
						 / (old_doc_height - old_height));
		if (layout->yoffset != html->engine->y_offset) {
			layout->yoffset = html->engine->y_offset;
			if (changed_y)
				*changed_y = TRUE;
		}
		gtk_adjustment_set_value (vadj, html->engine->y_offset);
	}
}

void
gtk_html_private_calc_scrollbars (GtkHTML *html, gboolean *changed_x, gboolean *changed_y)
{
	GtkLayout *layout;
	GtkAdjustment *vadj, *hadj;
	gint width, height;

	/* printf ("calc scrollbars\n"); */

	height = html_engine_get_doc_height (html->engine);
	width = html_engine_get_doc_width (html->engine);

	layout = GTK_LAYOUT (html);
	hadj = layout->hadjustment;
	vadj = layout->vadjustment;

	if (!GTK_WIDGET_REALIZED (html))
		return;

	vadj->lower = 0;
	vadj->upper = height;
	vadj->page_size = html->engine->height;
	vadj->step_increment = 14; /* FIXME */
	vadj->page_increment = html->engine->height;
	if (vadj->value > height - html->engine->height) {
		gtk_adjustment_set_value (vadj, height - html->engine->height);
		if (changed_y)
			*changed_y = TRUE;
	}

	hadj->lower = 0.0;
	hadj->upper = width;
	hadj->page_size = html->engine->width;
	hadj->step_increment = 14; /* FIXME */
	hadj->page_increment = html->engine->width;
	
	if (hadj->value > width - html->engine->width 
	    || hadj->value > MAX_WIDGET_WIDTH - html->engine->width) {
		
		gtk_adjustment_set_value (hadj, MIN (width - html->engine->width, MAX_WIDGET_WIDTH - html->engine->width));
		if (changed_x)
			*changed_x = TRUE;
	}

	if ((width != layout->width) || (height != layout->height)) {
		/* printf ("set size\n"); */
		gtk_signal_emit (GTK_OBJECT (html), signals[SIZE_CHANGED]);
		gtk_layout_set_size (layout, width, height);
	}
}


void
gtk_html_set_editable (GtkHTML *html,
		       gboolean editable)
{
	g_return_if_fail (html != NULL);
	g_return_if_fail (GTK_IS_HTML (html));

	html_engine_set_editable (html->engine, editable);
	set_editor_keybindings (html, editable);

	if (editable)
		gtk_html_update_styles (html);
}

gboolean
gtk_html_get_editable  (const GtkHTML *html)
{
	g_return_val_if_fail (html != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_HTML (html), FALSE);

	return html_engine_get_editable (html->engine);
}

void
gtk_html_load_empty (GtkHTML *html)
{
	g_return_if_fail (html != NULL);
	g_return_if_fail (GTK_IS_HTML (html));

	html_engine_load_empty (html->engine);
}

void
gtk_html_load_from_string  (GtkHTML *html, const gchar *str, gint len)
{
	GtkHTMLStream *stream;

	stream = gtk_html_begin_content (html, "text/html; charset=utf-8");
	gtk_html_stream_write (stream, str, (len == -1) ? strlen (str) : len);
	gtk_html_stream_close (stream, GTK_HTML_STREAM_OK);
}

void
gtk_html_set_base (GtkHTML *html, const char *url)
{
	GtkHTMLPrivate *priv;

	g_return_if_fail (GTK_IS_HTML (html));
	
	priv = html->priv;

	g_free (html->priv->base_url);
	html->priv->base_url = g_strdup (url);
}

const char *
gtk_html_get_base (GtkHTML *html)
{
	g_return_val_if_fail (GTK_IS_HTML (html), NULL);
	
	return html->priv->base_url;
}


/* Printing.  */

void
gtk_html_print (GtkHTML *html,
		GnomePrintContext *print_context)
{
	g_return_if_fail (html != NULL);
	g_return_if_fail (GTK_IS_HTML (html));

	html_engine_print (html->engine, print_context);
}

void
gtk_html_print_with_header_footer (GtkHTML *html, GnomePrintContext *print_context,
				   gdouble header_height, gdouble footer_height,
				   GtkHTMLPrintCallback header_print, GtkHTMLPrintCallback footer_print, gpointer user_data)
{
	g_return_if_fail (html != NULL);
	g_return_if_fail (GTK_IS_HTML (html));

	html_engine_print_with_header_footer (html->engine, print_context,
					      header_height, footer_height, header_print, footer_print, user_data);
}


/* Editing.  */

void
gtk_html_set_paragraph_style (GtkHTML *html,
			      GtkHTMLParagraphStyle style)
{
	HTMLClueFlowStyle current_style;
	HTMLClueFlowStyle clueflow_style;
	HTMLListType item_type;
	HTMLListType cur_item_type;

	g_return_if_fail (html != NULL);
	g_return_if_fail (GTK_IS_HTML (html));

	/* FIXME precondition: check if it's a valid style.  */

	paragraph_style_to_clueflow_style (style, &clueflow_style, &item_type);

	html_engine_get_current_clueflow_style (html->engine, &current_style, &cur_item_type);
	if (current_style == clueflow_style
	    && (current_style != HTML_CLUEFLOW_STYLE_LIST_ITEM || item_type == cur_item_type))
		return;

	if (! html_engine_set_clueflow_style (html->engine, clueflow_style, item_type, 0, 0, NULL,
					      HTML_ENGINE_SET_CLUEFLOW_STYLE, HTML_UNDO_UNDO, TRUE))
		return;

	html->priv->paragraph_style = style;

	gtk_signal_emit (GTK_OBJECT (html), signals[CURRENT_PARAGRAPH_STYLE_CHANGED],
			 style);
	queue_draw (html);
}

GtkHTMLParagraphStyle
gtk_html_get_paragraph_style (GtkHTML *html)
{
	HTMLClueFlowStyle style;
	HTMLListType item_type;

	html_engine_get_current_clueflow_style (html->engine, &style, &item_type);

	return clueflow_style_to_paragraph_style (style, item_type);
}

guint
gtk_html_get_paragraph_indentation (GtkHTML *html)
{
	return html_engine_get_current_clueflow_indentation (html->engine);
}

void
gtk_html_set_indent (GtkHTML *html,
		     GByteArray *levels)
{
	g_return_if_fail (html != NULL);
	g_return_if_fail (GTK_IS_HTML (html));

	html_engine_set_clueflow_style (html->engine, 0, 0, 0, 
					levels ? levels->len : 0, 
					levels ? levels->data : NULL,
					HTML_ENGINE_SET_CLUEFLOW_INDENTATION, HTML_UNDO_UNDO, TRUE);

	gtk_html_update_styles (html);
}

static void
gtk_html_modify_indent_by_delta (GtkHTML *html,
				 gint delta, 
				 guint8 *levels)
{
	g_return_if_fail (html != NULL);
	g_return_if_fail (GTK_IS_HTML (html));

	html_engine_set_clueflow_style (html->engine, 0, 0, 0, delta, levels,
					HTML_ENGINE_SET_CLUEFLOW_INDENTATION_DELTA, HTML_UNDO_UNDO, TRUE);

	gtk_html_update_styles (html);
}

void
gtk_html_indent_push_level (GtkHTML *html, HTMLListType level_type)
{
	guint8 type = (guint8)level_type;
	gtk_html_modify_indent_by_delta (html, +1, &type);
}

void
gtk_html_indent_pop_level (GtkHTML *html)
{
	gtk_html_modify_indent_by_delta (html, -1, NULL);
}

void
gtk_html_set_font_style (GtkHTML *html,
			 GtkHTMLFontStyle and_mask,
			 GtkHTMLFontStyle or_mask)
{
	g_return_if_fail (html != NULL);
	g_return_if_fail (GTK_IS_HTML (html));

	if (html_engine_set_font_style (html->engine, and_mask, or_mask))
		gtk_signal_emit (GTK_OBJECT (html), signals [INSERTION_FONT_STYLE_CHANGED],
				 html->engine->insertion_font_style);
}

void
gtk_html_set_color (GtkHTML *html, HTMLColor *color)
{
	g_return_if_fail (html != NULL);
	g_return_if_fail (GTK_IS_HTML (html));

	if (html_engine_set_color (html->engine, color))
		gtk_signal_emit (GTK_OBJECT (html), signals [INSERTION_COLOR_CHANGED],
				 html->engine->insertion_font_style);
}

void
gtk_html_toggle_font_style (GtkHTML *html,
			    GtkHTMLFontStyle style)
{
	g_return_if_fail (html != NULL);
	g_return_if_fail (GTK_IS_HTML (html));

	if (html_engine_toggle_font_style (html->engine, style))
		gtk_signal_emit (GTK_OBJECT (html), signals [INSERTION_FONT_STYLE_CHANGED],
				 html->engine->insertion_font_style);
}

GtkHTMLParagraphAlignment
gtk_html_get_paragraph_alignment (GtkHTML *html)
{
	return paragraph_alignment_to_html (html_engine_get_current_clueflow_alignment (html->engine));
}

void
gtk_html_set_paragraph_alignment (GtkHTML *html,
				  GtkHTMLParagraphAlignment alignment)
{
	HTMLHAlignType align;

	g_return_if_fail (html != NULL);
	g_return_if_fail (GTK_IS_HTML (html));

	align = paragraph_alignment_to_html (alignment);

	if (html_engine_set_clueflow_style (html->engine, 0, 0, align, 0, NULL,
					    HTML_ENGINE_SET_CLUEFLOW_ALIGNMENT, HTML_UNDO_UNDO, TRUE)) {
		html->priv->paragraph_alignment = alignment;
		gtk_signal_emit (GTK_OBJECT (html), 
				 signals [CURRENT_PARAGRAPH_ALIGNMENT_CHANGED],
				 alignment);
	}
}


/* Clipboard operations.  */

void
gtk_html_cut (GtkHTML *html)
{
	g_return_if_fail (html != NULL);
	g_return_if_fail (GTK_IS_HTML (html));

	html_engine_cut (html->engine);
	gtk_selection_owner_set (GTK_WIDGET (html), gdk_atom_intern ("CLIPBOARD", FALSE), html_selection_current_time ());
}

void
gtk_html_copy (GtkHTML *html)
{
	g_return_if_fail (html != NULL);
	g_return_if_fail (GTK_IS_HTML (html));

	html_engine_copy (html->engine);
	gtk_selection_owner_set (GTK_WIDGET (html), gdk_atom_intern ("CLIPBOARD", FALSE), html_selection_current_time ());
}

void
gtk_html_paste (GtkHTML *html, gboolean as_cite)
{
	g_return_if_fail (html != NULL);
	g_return_if_fail (GTK_IS_HTML (html));

	gtk_html_request_paste (html, gdk_atom_intern ("CLIPBOARD", FALSE), 
				0, html_selection_current_time (), as_cite);
}


/* Undo/redo.  */

void
gtk_html_undo (GtkHTML *html)
{
	g_return_if_fail (html != NULL);
	g_return_if_fail (GTK_IS_HTML (html));

	html_engine_undo (html->engine);
	gtk_html_update_styles (html);
}

void
gtk_html_redo (GtkHTML *html)
{
	g_return_if_fail (html != NULL);
	g_return_if_fail (GTK_IS_HTML (html));

	html_engine_redo (html->engine);
	gtk_html_update_styles (html);
}

/* misc utils */

void
gtk_html_set_default_background_color (GtkHTML *html, GdkColor *c)
{
	html_colorset_set_color (html->engine->defaultSettings->color_set, c, HTMLBgColor);
}

void
gtk_html_set_default_content_type (GtkHTML *html, gchar *content_type)
{
	gchar *lower;

	g_free (html->priv->content_type);	

	if (content_type) {
		lower = g_strdup (content_type);
		g_strdown (lower);
		html->priv->content_type = lower;
	} else
		html->priv->content_type = NULL;
}

gpointer
gtk_html_get_object_by_id (GtkHTML *html, const gchar *id)
{
	g_return_val_if_fail (html, NULL);
	g_return_val_if_fail (id, NULL);
	g_return_val_if_fail (GTK_IS_HTML (html), NULL);
	g_return_val_if_fail (html->engine, NULL);

	return html_engine_get_object_by_id (html->engine, id);
}

/*******************************************

   keybindings

*/

static gint
get_line_height (GtkHTML *html)
{
	HTMLPainter *p;

	if (!html->engine || !html->engine->painter)
		return 0;

	p = html->engine->painter;

	return html_painter_calc_ascent (p, GTK_HTML_FONT_STYLE_SIZE_3, NULL)
		+ html_painter_calc_descent (p, GTK_HTML_FONT_STYLE_SIZE_3, NULL);
}

static void
scroll (GtkHTML *html,
	GtkOrientation orientation,
	GtkScrollType  scroll_type,
	gfloat         position)
{
	GtkAdjustment *adj;
	gint line_height;
	gfloat delta;

	/* we dont want scroll in editable (move cursor instead) */
	if (html_engine_get_editable (html->engine))
		return;

	adj = (orientation == GTK_ORIENTATION_VERTICAL)
		? gtk_layout_get_vadjustment (GTK_LAYOUT (html)) : gtk_layout_get_hadjustment (GTK_LAYOUT (html));


	line_height = (html->engine && adj->page_increment > (3 * get_line_height (html)))
		? get_line_height (html)
		: 0;

	switch (scroll_type) {
	case GTK_SCROLL_STEP_FORWARD:
		delta = adj->step_increment;
		break;
	case GTK_SCROLL_STEP_BACKWARD:
		delta = -adj->step_increment;
		break;
	case GTK_SCROLL_PAGE_FORWARD:
		delta = adj->page_increment - line_height;
		break;
	case GTK_SCROLL_PAGE_BACKWARD:
		delta = -adj->page_increment + line_height;
		break;
	default:
		g_warning ("invalid scroll parameters: %d %d %f\n", orientation, scroll_type, position);
		delta = 0.0;
		return;
	}

	gtk_adjustment_set_value (adj, CLAMP (adj->value + delta, adj->lower, MAX (0.0, adj->upper - adj->page_size)));

	html->binding_handled = TRUE;
}

static void
scroll_by_amount (GtkHTML *html, gint amount)
{
	GtkAdjustment *adj;

	adj = GTK_LAYOUT (html)->vadjustment;
	gtk_adjustment_set_value (adj,
				  CLAMP (adj->value + (gfloat) amount, adj->lower, MAX (0.0, adj->upper - adj->page_size)));
}

static void
cursor_move (GtkHTML *html, GtkDirectionType dir_type, GtkHTMLCursorSkipType skip)
{
	gint amount;

	if (!html_engine_get_editable (html->engine))
		return;

	if (html->engine->selection_mode) {
		if (!html->engine->mark)
			html_engine_set_mark (html->engine);
	} else if (html->engine->shift_selection || html->engine->mark) {
		html_engine_disable_selection (html->engine);
		html_engine_edit_selection_updater_schedule (html->engine->selection_updater);
		html->engine->shift_selection = FALSE;
	}
	switch (skip) {
	case GTK_HTML_CURSOR_SKIP_ONE:
		switch (dir_type) {
		case GTK_DIR_LEFT:
			html_engine_move_cursor (html->engine, HTML_ENGINE_CURSOR_LEFT, 1);
			break;
		case GTK_DIR_RIGHT:
			html_engine_move_cursor (html->engine, HTML_ENGINE_CURSOR_RIGHT, 1);
			break;
		case GTK_DIR_UP:
			html_engine_move_cursor (html->engine, HTML_ENGINE_CURSOR_UP, 1);
			break;
		case GTK_DIR_DOWN:
			html_engine_move_cursor (html->engine, HTML_ENGINE_CURSOR_DOWN, 1);
			break;
		default:
			g_warning ("invalid cursor_move parameters\n");
		}
		break;
	case GTK_HTML_CURSOR_SKIP_WORD:
		switch (dir_type) {
		case GTK_DIR_UP:
		case GTK_DIR_LEFT:
			html_engine_backward_word (html->engine);
			break;
		case GTK_DIR_DOWN:
		case GTK_DIR_RIGHT:
			html_engine_forward_word (html->engine);
			break;
		default:
			g_warning ("invalid cursor_move parameters\n");
		}
		break;
	case GTK_HTML_CURSOR_SKIP_PAGE: {
		gint line_height;

		line_height =  GTK_WIDGET (html)->allocation.height > (3 * get_line_height (html))
			? get_line_height (html) : 0;


		switch (dir_type) {
		case GTK_DIR_UP:
		case GTK_DIR_LEFT:
			if ((amount = html_engine_scroll_up (html->engine,
							     GTK_WIDGET (html)->allocation.height - line_height)) > 0)
				scroll_by_amount (html, - amount);
			break;
		case GTK_DIR_DOWN:
		case GTK_DIR_RIGHT:
			if ((amount = html_engine_scroll_down (html->engine,
							       GTK_WIDGET (html)->allocation.height - line_height)) > 0)
				scroll_by_amount (html, amount);
			break;
		default:
			g_warning ("invalid cursor_move parameters\n");
		}
		break;
	}
	case GTK_HTML_CURSOR_SKIP_ALL:
		switch (dir_type) {
		case GTK_DIR_LEFT:
			html_engine_beginning_of_line (html->engine);
			break;
		case GTK_DIR_RIGHT:
			html_engine_end_of_line (html->engine);
			break;
		case GTK_DIR_UP:
			html_engine_beginning_of_document (html->engine);
			break;
		case GTK_DIR_DOWN:
			html_engine_end_of_document (html->engine);
			break;
		default:
			g_warning ("invalid cursor_move parameters\n");
		}
		break;
	default:
		g_warning ("invalid cursor_move parameters\n");
	}

	html->binding_handled = TRUE;
	html->priv->update_styles = TRUE;
	gtk_html_edit_make_cursor_visible (html);
	html_engine_update_selection_active_state (html->engine, html->priv->event_time);
}

static gboolean
move_selection (GtkHTML *html, GtkHTMLCommandType com_type)
{
	gboolean rv;
	gint amount;

	if (!html_engine_get_editable (html->engine))
		return FALSE;

	html->engine->shift_selection = TRUE;
	if (!html->engine->mark)
		html_engine_set_mark (html->engine);
	switch (com_type) {
	case GTK_HTML_COMMAND_MODIFY_SELECTION_UP:
		rv = html_engine_move_cursor (html->engine, HTML_ENGINE_CURSOR_UP, 1) > 0 ? TRUE : FALSE;
		break;
	case GTK_HTML_COMMAND_MODIFY_SELECTION_DOWN:
		rv = html_engine_move_cursor (html->engine, HTML_ENGINE_CURSOR_DOWN, 1) > 0 ? TRUE : FALSE;
		break;
	case GTK_HTML_COMMAND_MODIFY_SELECTION_LEFT:
		rv = html_engine_move_cursor (html->engine, HTML_ENGINE_CURSOR_LEFT, 1) > 0 ? TRUE : FALSE;
		break;
	case GTK_HTML_COMMAND_MODIFY_SELECTION_RIGHT:
		rv = html_engine_move_cursor (html->engine, HTML_ENGINE_CURSOR_RIGHT, 1) > 0 ? TRUE : FALSE;
		break;
	case GTK_HTML_COMMAND_MODIFY_SELECTION_BOL:
		rv = html_engine_beginning_of_line (html->engine);
		break;
	case GTK_HTML_COMMAND_MODIFY_SELECTION_EOL:
		rv = html_engine_end_of_line (html->engine);
		break;
	case GTK_HTML_COMMAND_MODIFY_SELECTION_BOD:
		html_engine_beginning_of_document (html->engine);
		rv = TRUE;
		break;
	case GTK_HTML_COMMAND_MODIFY_SELECTION_EOD:
		html_engine_end_of_document (html->engine);
		rv = TRUE;
		break;
	case GTK_HTML_COMMAND_MODIFY_SELECTION_PREV_WORD:
		rv = html_engine_backward_word (html->engine);
		break;
	case GTK_HTML_COMMAND_MODIFY_SELECTION_NEXT_WORD:
		rv = html_engine_forward_word (html->engine);
		break;
	case GTK_HTML_COMMAND_MODIFY_SELECTION_PAGEUP:
		if ((amount = html_engine_scroll_up (html->engine, GTK_WIDGET (html)->allocation.height)) > 0) {
			scroll_by_amount (html, - amount);
			rv = TRUE;
		} else
			rv = FALSE;
		break;
	case GTK_HTML_COMMAND_MODIFY_SELECTION_PAGEDOWN:
		if ((amount = html_engine_scroll_down (html->engine, GTK_WIDGET (html)->allocation.height)) > 0) {
			scroll_by_amount (html, amount);
			rv = TRUE;
		} else
			rv = FALSE;
		break;
	default:
		g_warning ("invalid move_selection parameters\n");
		rv = FALSE;
	}

	html->binding_handled = TRUE;
	html->priv->update_styles = TRUE;

	html_engine_update_selection_active_state (html->engine, html->priv->event_time);

	return rv;
}

inline static void
delete_one (HTMLEngine *e, gboolean forward)
{
	if (e->cursor->object && html_object_is_container (e->cursor->object)
	    && ((forward && !e->cursor->offset) || (!forward && e->cursor->offset)))
		html_engine_delete_container (e);
	else
		html_engine_delete_n (e, 1, forward);
}

inline static gboolean
insert_tab_or_next_cell (GtkHTML *html)
{
	HTMLEngine *e = html->engine;
	if (!html_engine_next_cell (e, TRUE)) {
		if (html_clueflow_tabs (HTML_CLUEFLOW (e->cursor->object->parent), e->painter))
			html_engine_paste_text (e, "\t", 1);
		else
			html_engine_paste_text (e, "\xc2\xa0\xc2\xa0\xc2\xa0\xc2\xa0", 4);
		return TRUE;
	}

	return TRUE;
}

inline static void
indent_more_or_next_cell (GtkHTML *html)
{
	if (!html_engine_next_cell (html->engine, TRUE))
		gtk_html_indent_push_level (html, HTML_LIST_TYPE_BLOCKQUOTE);
}

static gboolean
command (GtkHTML *html, GtkHTMLCommandType com_type)
{
	HTMLEngine *e = html->engine;
	gboolean rv = TRUE;

	/* printf ("command %d %s\n", com_type, get_value_nick (com_type)); */
	html->binding_handled = TRUE;

	/* non-editable + editable commands */
	switch (com_type) {
	case GTK_HTML_COMMAND_ZOOM_IN:
		gtk_html_zoom_in (html);
		break;
	case GTK_HTML_COMMAND_ZOOM_OUT:
		gtk_html_zoom_out (html);
		break;
	case GTK_HTML_COMMAND_ZOOM_RESET:
		gtk_html_zoom_reset (html);
		break;
	case GTK_HTML_COMMAND_SEARCH_INCREMENTAL_FORWARD:
		gtk_html_isearch (html, TRUE);
		break;
	case GTK_HTML_COMMAND_SEARCH_INCREMENTAL_BACKWARD:
		gtk_html_isearch (html, FALSE);
		break;
	case GTK_HTML_COMMAND_FOCUS_FORWARD:
		html->binding_handled = gtk_container_focus (GTK_CONTAINER (html), GTK_DIR_TAB_FORWARD);
		break;
	case GTK_HTML_COMMAND_FOCUS_BACKWARD:
		html->binding_handled = gtk_container_focus (GTK_CONTAINER (html), GTK_DIR_TAB_BACKWARD);
		break;
	case GTK_HTML_COMMAND_SCROLL_BOD:
		gtk_adjustment_set_value (gtk_layout_get_vadjustment (GTK_LAYOUT (html)), 0);
		break;
	case GTK_HTML_COMMAND_SCROLL_EOD: {
		GtkAdjustment *vadj = gtk_layout_get_vadjustment (GTK_LAYOUT (html));
		gtk_adjustment_set_value (vadj, vadj->upper - vadj->page_size);
		break;
	case GTK_HTML_COMMAND_COPY:
		gtk_html_copy (html);
		break;
	}
	default:
		html->binding_handled = FALSE;
	}

	if (!html_engine_get_editable (e) || html->binding_handled)
		return rv;

	html->binding_handled = TRUE;
	html->priv->update_styles = FALSE;

	/* editable commands only */
	switch (com_type) {
	case GTK_HTML_COMMAND_UNDO:
		gtk_html_undo (html);
		break;
	case GTK_HTML_COMMAND_REDO:
		gtk_html_redo (html);
		break;
	case GTK_HTML_COMMAND_COPY_AND_DISABLE_SELECTION:
		gtk_html_copy (html);
		html_engine_disable_selection (e);
		html_engine_edit_selection_updater_schedule (e->selection_updater);
		e->selection_mode = FALSE;
		break;
	case GTK_HTML_COMMAND_CUT:
		gtk_html_cut (html);
		html->priv->update_styles = TRUE;
		break;
	case GTK_HTML_COMMAND_CUT_LINE:
		html_engine_cut_line (e);
		html->priv->update_styles = TRUE;
		break;
	case GTK_HTML_COMMAND_PASTE:
		gtk_html_paste (html, FALSE);
		html->priv->update_styles = TRUE;
		break;
	case GTK_HTML_COMMAND_INSERT_RULE:
		html_engine_insert_rule (e, 0, 100, 2, TRUE, HTML_HALIGN_LEFT);
		break;
	case GTK_HTML_COMMAND_INSERT_PARAGRAPH:
		html_engine_delete (e);

		/* stop inserting links after newlines */
		html_engine_set_insertion_link (e, NULL, NULL);

		html_engine_insert_empty_paragraph (e);
		html->priv->update_styles = TRUE;
		break;
	case GTK_HTML_COMMAND_DELETE:
		if (e->mark != NULL
		    && e->mark->position != e->cursor->position)
			html_engine_delete (e);
		else
			delete_one (e, TRUE);
		html->priv->update_styles = TRUE;
		break;
	case GTK_HTML_COMMAND_DELETE_BACK:
		if (html_engine_is_selection_active (e))
			html_engine_delete (e);
		else
			delete_one (e, FALSE);
		html->priv->update_styles = TRUE;
		break;
	case GTK_HTML_COMMAND_DELETE_BACK_OR_INDENT_DEC:
		if (html_engine_is_selection_active (e))
			html_engine_delete (e);
		else if (html_engine_cursor_on_bop (e) && html_engine_get_indent (e) > 0
			 && e->cursor->object->parent && HTML_IS_CLUEFLOW (e->cursor->object->parent)
			 && HTML_CLUEFLOW (e->cursor->object->parent)->style != HTML_CLUEFLOW_STYLE_LIST_ITEM)
			gtk_html_indent_pop_level (html);
		else
			delete_one (e, FALSE);
		html->priv->update_styles = TRUE;
		break;
	case GTK_HTML_COMMAND_DELETE_TABLE:
		html_engine_delete_table (e);
		html->priv->update_styles = TRUE;
		break;
	case GTK_HTML_COMMAND_DELETE_TABLE_ROW:
		html_engine_delete_table_row (e);
		html->priv->update_styles = TRUE;
		break;
	case GTK_HTML_COMMAND_DELETE_TABLE_COLUMN:
		html_engine_delete_table_column (e);
		html->priv->update_styles = TRUE;
		break;
	case GTK_HTML_COMMAND_DELETE_TABLE_CELL_CONTENTS:
		html_engine_delete_table_cell_contents (e);
		html->priv->update_styles = TRUE;
		break;
	case GTK_HTML_COMMAND_SELECTION_MODE:
		e->selection_mode = TRUE;
		break;
	case GTK_HTML_COMMAND_DISABLE_SELECTION:
		html_engine_disable_selection (e);
		html_engine_edit_selection_updater_schedule (e->selection_updater);
		e->selection_mode = FALSE;
		break;
	case GTK_HTML_COMMAND_BOLD_ON:
		gtk_html_set_font_style (html, GTK_HTML_FONT_STYLE_MAX, GTK_HTML_FONT_STYLE_BOLD);
		break;
	case GTK_HTML_COMMAND_BOLD_OFF:
		gtk_html_set_font_style (html, ~GTK_HTML_FONT_STYLE_BOLD, 0);
		break;
	case GTK_HTML_COMMAND_BOLD_TOGGLE:
		gtk_html_toggle_font_style (html, GTK_HTML_FONT_STYLE_BOLD);
		break;
	case GTK_HTML_COMMAND_ITALIC_ON:
		gtk_html_set_font_style (html, GTK_HTML_FONT_STYLE_MAX, GTK_HTML_FONT_STYLE_ITALIC);
		break;
	case GTK_HTML_COMMAND_ITALIC_OFF:
		gtk_html_set_font_style (html, ~GTK_HTML_FONT_STYLE_ITALIC, 0);
		break;
	case GTK_HTML_COMMAND_ITALIC_TOGGLE:
		gtk_html_toggle_font_style (html, GTK_HTML_FONT_STYLE_ITALIC);
		break;
	case GTK_HTML_COMMAND_STRIKEOUT_ON:
		gtk_html_set_font_style (html, GTK_HTML_FONT_STYLE_MAX, GTK_HTML_FONT_STYLE_STRIKEOUT);
		break;
	case GTK_HTML_COMMAND_STRIKEOUT_OFF:
		gtk_html_set_font_style (html, ~GTK_HTML_FONT_STYLE_STRIKEOUT, 0);
		break;
	case GTK_HTML_COMMAND_STRIKEOUT_TOGGLE:
		gtk_html_toggle_font_style (html, GTK_HTML_FONT_STYLE_STRIKEOUT);
		break;
	case GTK_HTML_COMMAND_UNDERLINE_ON:
		gtk_html_set_font_style (html, GTK_HTML_FONT_STYLE_MAX, GTK_HTML_FONT_STYLE_UNDERLINE);
		break;
	case GTK_HTML_COMMAND_UNDERLINE_OFF:
		gtk_html_set_font_style (html, ~GTK_HTML_FONT_STYLE_UNDERLINE, 0);
		break;
	case GTK_HTML_COMMAND_UNDERLINE_TOGGLE:
		gtk_html_toggle_font_style (html, GTK_HTML_FONT_STYLE_UNDERLINE);
		break;
	case GTK_HTML_COMMAND_SIZE_MINUS_2:
		gtk_html_set_font_style (html, ~GTK_HTML_FONT_STYLE_SIZE_MASK, GTK_HTML_FONT_STYLE_SIZE_1);
		break;
	case GTK_HTML_COMMAND_SIZE_MINUS_1:
		gtk_html_set_font_style (html, ~GTK_HTML_FONT_STYLE_SIZE_MASK, GTK_HTML_FONT_STYLE_SIZE_2);
		break;
	case GTK_HTML_COMMAND_SIZE_PLUS_0:
		gtk_html_set_font_style (html, ~GTK_HTML_FONT_STYLE_SIZE_MASK, GTK_HTML_FONT_STYLE_SIZE_3);
		break;
	case GTK_HTML_COMMAND_SIZE_PLUS_1:
		gtk_html_set_font_style (html, ~GTK_HTML_FONT_STYLE_SIZE_MASK, GTK_HTML_FONT_STYLE_SIZE_4);
		break;
	case GTK_HTML_COMMAND_SIZE_PLUS_2:
		gtk_html_set_font_style (html, ~GTK_HTML_FONT_STYLE_SIZE_MASK, GTK_HTML_FONT_STYLE_SIZE_5);
		break;
	case GTK_HTML_COMMAND_SIZE_PLUS_3:
		gtk_html_set_font_style (html, ~GTK_HTML_FONT_STYLE_SIZE_MASK, GTK_HTML_FONT_STYLE_SIZE_6);
		break;
	case GTK_HTML_COMMAND_SIZE_PLUS_4:
		gtk_html_set_font_style (html, ~GTK_HTML_FONT_STYLE_SIZE_MASK, GTK_HTML_FONT_STYLE_SIZE_7);
		break;
	case GTK_HTML_COMMAND_SIZE_INCREASE:
		html_engine_font_size_inc_dec (e, TRUE);
		break;
	case GTK_HTML_COMMAND_SIZE_DECREASE:
		html_engine_font_size_inc_dec (e, FALSE);
		break;
	case GTK_HTML_COMMAND_ALIGN_LEFT:
		gtk_html_set_paragraph_alignment (html, GTK_HTML_PARAGRAPH_ALIGNMENT_LEFT);
		break;
	case GTK_HTML_COMMAND_ALIGN_CENTER:
		gtk_html_set_paragraph_alignment (html, GTK_HTML_PARAGRAPH_ALIGNMENT_CENTER);
		break;
	case GTK_HTML_COMMAND_ALIGN_RIGHT:
		gtk_html_set_paragraph_alignment (html, GTK_HTML_PARAGRAPH_ALIGNMENT_RIGHT);
		break;
	case GTK_HTML_COMMAND_INDENT_ZERO:
		gtk_html_set_indent (html, NULL);
		break;
	case GTK_HTML_COMMAND_INDENT_INC:
		gtk_html_indent_push_level (html, HTML_LIST_TYPE_BLOCKQUOTE);
		break;
	case GTK_HTML_COMMAND_INDENT_INC_OR_NEXT_CELL:
		indent_more_or_next_cell (html);
		break;
	case GTK_HTML_COMMAND_INSERT_TAB:
		if (!html_engine_is_selection_active (e)
		    && html_clueflow_tabs (HTML_CLUEFLOW (e->cursor->object->parent), e->painter))
			html_engine_insert_text (e, "\t", 1);
		break;
	case GTK_HTML_COMMAND_INSERT_TAB_OR_INDENT_MORE:
		if (!html_engine_is_selection_active (e)
		    && html_clueflow_tabs (HTML_CLUEFLOW (e->cursor->object->parent), e->painter))
			html_engine_insert_text (e, "\t", 1);
		else
			gtk_html_indent_push_level (html, HTML_LIST_TYPE_BLOCKQUOTE);
		break;
	case GTK_HTML_COMMAND_INSERT_TAB_OR_NEXT_CELL:
		html->binding_handled = insert_tab_or_next_cell (html);
		break;
	case GTK_HTML_COMMAND_INDENT_DEC:
		gtk_html_indent_pop_level (html);
		break;
	case GTK_HTML_COMMAND_PREV_CELL:
		html->binding_handled = html_engine_prev_cell (html->engine);
		break;
	case GTK_HTML_COMMAND_INDENT_PARAGRAPH:
		html_engine_indent_paragraph (e);
		break;
	case GTK_HTML_COMMAND_BREAK_AND_FILL_LINE:
		html_engine_break_and_fill_line (e);
		break;
	case GTK_HTML_COMMAND_SPACE_AND_FILL_LINE:
		html_engine_space_and_fill_line (e);
		break;
	case GTK_HTML_COMMAND_PARAGRAPH_STYLE_NORMAL:
		gtk_html_set_paragraph_style (html, GTK_HTML_PARAGRAPH_STYLE_NORMAL);
		break;
	case GTK_HTML_COMMAND_PARAGRAPH_STYLE_H1:
		gtk_html_set_paragraph_style (html, GTK_HTML_PARAGRAPH_STYLE_H1);
		break;
	case GTK_HTML_COMMAND_PARAGRAPH_STYLE_H2:
		gtk_html_set_paragraph_style (html, GTK_HTML_PARAGRAPH_STYLE_H2);
		break;
	case GTK_HTML_COMMAND_PARAGRAPH_STYLE_H3:
		gtk_html_set_paragraph_style (html, GTK_HTML_PARAGRAPH_STYLE_H3);
		break;
	case GTK_HTML_COMMAND_PARAGRAPH_STYLE_H4:
		gtk_html_set_paragraph_style (html, GTK_HTML_PARAGRAPH_STYLE_H4);
		break;
	case GTK_HTML_COMMAND_PARAGRAPH_STYLE_H5:
		gtk_html_set_paragraph_style (html, GTK_HTML_PARAGRAPH_STYLE_H5);
		break;
	case GTK_HTML_COMMAND_PARAGRAPH_STYLE_H6:
		gtk_html_set_paragraph_style (html, GTK_HTML_PARAGRAPH_STYLE_H6);
		break;
	case GTK_HTML_COMMAND_PARAGRAPH_STYLE_PRE:
		gtk_html_set_paragraph_style (html, GTK_HTML_PARAGRAPH_STYLE_PRE);
		break;
	case GTK_HTML_COMMAND_PARAGRAPH_STYLE_ADDRESS:
		gtk_html_set_paragraph_style (html, GTK_HTML_PARAGRAPH_STYLE_ADDRESS);
		break;
	case GTK_HTML_COMMAND_PARAGRAPH_STYLE_ITEMDOTTED:
		gtk_html_set_paragraph_style (html, GTK_HTML_PARAGRAPH_STYLE_ITEMDOTTED);
		break;
	case GTK_HTML_COMMAND_PARAGRAPH_STYLE_ITEMROMAN:
		gtk_html_set_paragraph_style (html, GTK_HTML_PARAGRAPH_STYLE_ITEMROMAN);
		break;
	case GTK_HTML_COMMAND_PARAGRAPH_STYLE_ITEMDIGIT:
		gtk_html_set_paragraph_style (html, GTK_HTML_PARAGRAPH_STYLE_ITEMDIGIT);
		break;
	case GTK_HTML_COMMAND_PARAGRAPH_STYLE_ITEMALPHA:
		gtk_html_set_paragraph_style (html, GTK_HTML_PARAGRAPH_STYLE_ITEMALPHA);
		break;
	case GTK_HTML_COMMAND_MODIFY_SELECTION_UP:
	case GTK_HTML_COMMAND_MODIFY_SELECTION_DOWN:
	case GTK_HTML_COMMAND_MODIFY_SELECTION_LEFT:
	case GTK_HTML_COMMAND_MODIFY_SELECTION_RIGHT:
	case GTK_HTML_COMMAND_MODIFY_SELECTION_BOL:
	case GTK_HTML_COMMAND_MODIFY_SELECTION_EOL:
	case GTK_HTML_COMMAND_MODIFY_SELECTION_BOD:
	case GTK_HTML_COMMAND_MODIFY_SELECTION_EOD:
	case GTK_HTML_COMMAND_MODIFY_SELECTION_PAGEUP:
	case GTK_HTML_COMMAND_MODIFY_SELECTION_PAGEDOWN:
	case GTK_HTML_COMMAND_MODIFY_SELECTION_PREV_WORD:
	case GTK_HTML_COMMAND_MODIFY_SELECTION_NEXT_WORD:
		rv = move_selection (html, com_type);
		break;
	case GTK_HTML_COMMAND_SELECT_WORD:
		gtk_html_select_word (html);
		break;
	case GTK_HTML_COMMAND_SELECT_LINE:
		gtk_html_select_line (html);
		break;
	case GTK_HTML_COMMAND_SELECT_PARAGRAPH:
		gtk_html_select_paragraph (html);
		break;
	case GTK_HTML_COMMAND_SELECT_PARAGRAPH_EXTENDED:
		gtk_html_select_paragraph_extended (html);
		break;
	case GTK_HTML_COMMAND_SELECT_ALL:
		gtk_html_select_all (html);
		break;
	case GTK_HTML_COMMAND_CURSOR_POSITION_SAVE:
		html_engine_edit_cursor_position_save (html->engine);
		break;
	case GTK_HTML_COMMAND_CURSOR_POSITION_RESTORE:
		html_engine_edit_cursor_position_restore (html->engine);
		break;
	case GTK_HTML_COMMAND_CAPITALIZE_WORD:
		html_engine_capitalize_word (e);
		break;
	case GTK_HTML_COMMAND_UPCASE_WORD:
		html_engine_upcase_downcase_word (e, TRUE);
		break;
	case GTK_HTML_COMMAND_DOWNCASE_WORD:
		html_engine_upcase_downcase_word (e, FALSE);
		break;
	case GTK_HTML_COMMAND_SPELL_SUGGEST:
		if (html->editor_api && !html_engine_spell_word_is_valid (e))
			(*html->editor_api->suggestion_request) (html, html_engine_get_spell_word (e), html->editor_data);
		break;
	case GTK_HTML_COMMAND_SPELL_PERSONAL_DICTIONARY_ADD:
	case GTK_HTML_COMMAND_SPELL_SESSION_DICTIONARY_ADD: {
		gchar *word;
		word = html_engine_get_spell_word (e);

		if (word && html->editor_api) {
			if (com_type == GTK_HTML_COMMAND_SPELL_PERSONAL_DICTIONARY_ADD)
				(*html->editor_api->add_to_personal) (html, word, html->editor_data);
			else
				(*html->editor_api->add_to_session) (html, word, html->editor_data);
			g_free (word);
			html_engine_spell_check (e);
			gtk_widget_queue_draw (GTK_WIDGET (html));
		}
		break;
	}
	case GTK_HTML_COMMAND_CURSOR_FORWARD:
		rv = html_cursor_forward (html->engine->cursor, html->engine);
		break;
	case GTK_HTML_COMMAND_CURSOR_BACKWARD:
		rv = html_cursor_backward (html->engine->cursor, html->engine);
		break;
	case GTK_HTML_COMMAND_INSERT_TABLE_1_1:
		html_engine_insert_table_1_1 (e);
		break;
	case GTK_HTML_COMMAND_TABLE_INSERT_COL_BEFORE:
		html_engine_insert_table_column (e, FALSE);
		break;
	case GTK_HTML_COMMAND_TABLE_INSERT_COL_AFTER:
		html_engine_insert_table_column (e, TRUE);
		break;
	case GTK_HTML_COMMAND_TABLE_DELETE_COL:
		html_engine_delete_table_column (e);
		break;
	case GTK_HTML_COMMAND_TABLE_INSERT_ROW_BEFORE:
		html_engine_insert_table_row (e, FALSE);
		break;
	case GTK_HTML_COMMAND_TABLE_INSERT_ROW_AFTER:
		html_engine_insert_table_row (e, TRUE);
		break;
	case GTK_HTML_COMMAND_TABLE_DELETE_ROW:
		html_engine_delete_table_row (e);
		break;
	case GTK_HTML_COMMAND_TABLE_BORDER_WIDTH_INC:
		html_engine_table_set_border_width (e, html_engine_get_table (e), 1, TRUE);
		break;
	case GTK_HTML_COMMAND_TABLE_BORDER_WIDTH_DEC:
		html_engine_table_set_border_width (e, html_engine_get_table (e), -1, TRUE);
		break;
	case GTK_HTML_COMMAND_TABLE_BORDER_WIDTH_ZERO:
		html_engine_table_set_border_width (e, html_engine_get_table (e), 0, FALSE);
		break;
	case GTK_HTML_COMMAND_TABLE_SPACING_INC:
		html_engine_table_set_spacing (e, html_engine_get_table (e), 1, TRUE);
		break;
	case GTK_HTML_COMMAND_TABLE_SPACING_DEC:
		html_engine_table_set_spacing (e, html_engine_get_table (e), -1, TRUE);
		break;
	case GTK_HTML_COMMAND_TABLE_SPACING_ZERO:
		html_engine_table_set_spacing (e, html_engine_get_table (e), 0, FALSE);
		break;
	case GTK_HTML_COMMAND_TABLE_PADDING_INC:
		html_engine_table_set_padding (e, html_engine_get_table (e), 1, TRUE);
		break;
	case GTK_HTML_COMMAND_TABLE_PADDING_DEC:
		html_engine_table_set_padding (e, html_engine_get_table (e), -1, TRUE);
		break;
	case GTK_HTML_COMMAND_TABLE_PADDING_ZERO:
		html_engine_table_set_padding (e, html_engine_get_table (e), 0, FALSE);
		break;
	case GTK_HTML_COMMAND_TEXT_SET_DEFAULT_COLOR:
		html_engine_set_color (e, NULL);
		break;
	case GTK_HTML_COMMAND_CURSOR_BOD:
		html_engine_beginning_of_document (e);		
		break;
	case GTK_HTML_COMMAND_CURSOR_EOD:
		html_engine_end_of_document (e);
		break;
	case GTK_HTML_COMMAND_BLOCK_REDRAW:
		html_engine_block_redraw (e);
		break;
	case GTK_HTML_COMMAND_UNBLOCK_REDRAW:
		html_engine_unblock_redraw (e);
		break;
	case GTK_HTML_COMMAND_GRAB_FOCUS:
		gtk_widget_grab_focus (GTK_WIDGET (html));
		break;
	case GTK_HTML_COMMAND_KILL_WORD:
	case GTK_HTML_COMMAND_KILL_WORD_BACKWARD:
		html_engine_disable_selection (e);
		html_engine_edit_selection_updater_schedule (e->selection_updater);
		html_engine_set_mark (html->engine);
		rv = com_type == GTK_HTML_COMMAND_KILL_WORD
			? html_engine_forward_word (html->engine)
			: html_engine_backward_word (html->engine);
		html_engine_edit_selection_updater_update_now (e->selection_updater);
		html_draw_queue_clear (e->draw_queue);
		if (rv)
			gtk_html_cut (html);
		html_engine_disable_selection (e);
		break;
	case GTK_HTML_COMMAND_SAVE_DATA_ON:
		html->engine->save_data = TRUE;
		break;
	case GTK_HTML_COMMAND_SAVE_DATA_OFF:
		html->engine->save_data = FALSE;
		break;
	case GTK_HTML_COMMAND_SAVED:
		html_engine_saved (html->engine);
		break;
	case GTK_HTML_COMMAND_IS_SAVED:
		rv = html_engine_is_saved (html->engine);
		break;
	default:
		html->binding_handled = FALSE;
	}

	if (!html->binding_handled && html->editor_api)
		html->binding_handled = (* html->editor_api->command) (html, com_type, html->editor_data);

	return rv;
}

/*
  default keybindings:

  Viewer:

  Up/Down ............................. scroll one line up/down
  PageUp/PageDown ..................... scroll one page up/down
  Space/BackSpace ..................... scroll one page up/down

  Left/Right .......................... scroll one char left/right
  Shift Left/Right .................... scroll more left/right

  Editor:

  Up/Down ............................. move cursor one line up/down
  PageUp/PageDown ..................... move cursor one page up/down

  Return .............................. insert paragraph
  Delete .............................. delete one char
  BackSpace ........................... delete one char backwards

*/

static void
clean_bindings_set (GtkBindingSet *binding_set)
{
	GtkBindingEntry *cur;
	GList *mods, *vals, *cm, *cv;

	if (!binding_set)
		return;

	mods = NULL;
	vals = NULL;
	cur = binding_set->entries;
	while (cur) {
		mods = g_list_prepend (mods, GUINT_TO_POINTER (cur->modifiers));
		vals = g_list_prepend (vals, GUINT_TO_POINTER (cur->keyval));
		cur = cur->set_next;
	}
	cm = mods; cv = vals;
	while (cm) {
		gtk_binding_entry_remove (binding_set, GPOINTER_TO_UINT (cv->data), GPOINTER_TO_UINT (cm->data));
		cv = cv->next; cm = cm->next;
	}
	g_list_free (mods);
	g_list_free (vals);
}

static void
set_editor_keybindings (GtkHTML *html, gboolean editable)
{
	if (editable) {		
		gchar *name;
		GtkHTMLClassProperties *prop;
		
		prop = get_class_properties (html);
		name = g_strconcat ("gtkhtml-bindings-", prop->keybindings_theme, NULL);

		html->editor_bindings = gtk_binding_set_find (name);
		if (!html->editor_bindings)
			g_warning ("cannot find %s bindings", name);
		g_free (name);
	} else
		html->editor_bindings = NULL;
}

static void
load_bindings_from_file (gboolean from_share, gchar *name)
{
	gchar *rcfile;

	rcfile = g_strconcat ((from_share ? PREFIX "/share/gtkhtml-" GTKHTML_RELEASE "/" : gnome_util_user_home ()),
			      (from_share ? "" : "/.gnome/"), name, NULL);

	if (g_file_test (rcfile, G_FILE_TEST_ISFILE))
		gtk_rc_parse (rcfile);
	g_free (rcfile);
}

static void
load_keybindings (GtkHTMLClass *klass)
{
	GtkBindingSet *binding_set;

	/* FIXME add to gtk gtk_binding_set_clear & gtk_binding_set_remove_path */
	clean_bindings_set (gtk_binding_set_by_class (klass));
	clean_bindings_set (gtk_binding_set_find ("gtkhtml-bindings-emacs"));
	clean_bindings_set (gtk_binding_set_find ("gtkhtml-bindings-xemacs"));
	clean_bindings_set (gtk_binding_set_find ("gtkhtml-bindings-ms"));
	clean_bindings_set (gtk_binding_set_find ("gtkhtml-bindings-custom"));

	/* ensure enums are defined */
	gtk_html_cursor_skip_get_type ();
	gtk_html_command_get_type ();

	load_bindings_from_file (TRUE,  "keybindingsrc.emacs");
	load_bindings_from_file (TRUE,  "keybindingsrc.xemacs");
	load_bindings_from_file (TRUE,  "keybindingsrc.ms");
	load_bindings_from_file (FALSE, "gtkhtml-bindings-custom");

	binding_set = gtk_binding_set_by_class (klass);

	/* layout scrolling */
#define BSCROLL(m,key,orient,sc) \
	gtk_binding_entry_add_signal (binding_set, GDK_ ## key, m, \
				      "scroll", 3, \
				      GTK_TYPE_ORIENTATION, GTK_ORIENTATION_ ## orient, \
				      GTK_TYPE_SCROLL_TYPE, GTK_SCROLL_ ## sc, \
				      GTK_TYPE_FLOAT, 0.0); \

	BSCROLL (0, Up, VERTICAL, STEP_BACKWARD);
	BSCROLL (0, KP_Up, VERTICAL, STEP_BACKWARD);
	BSCROLL (0, Down, VERTICAL, STEP_FORWARD);
	BSCROLL (0, KP_Down, VERTICAL, STEP_FORWARD);

	BSCROLL (0, Left, HORIZONTAL, STEP_BACKWARD);
	BSCROLL (0, KP_Left, HORIZONTAL, STEP_BACKWARD);
	BSCROLL (0, Right, HORIZONTAL, STEP_FORWARD);
	BSCROLL (0, KP_Right, HORIZONTAL, STEP_FORWARD);

	BSCROLL (0, Page_Up, VERTICAL, PAGE_BACKWARD);
	BSCROLL (0, KP_Page_Up, VERTICAL, PAGE_BACKWARD);
	BSCROLL (0, Page_Down, VERTICAL, PAGE_FORWARD);
	BSCROLL (0, KP_Page_Down, VERTICAL, PAGE_FORWARD);
	BSCROLL (0, BackSpace, VERTICAL, PAGE_BACKWARD);
	BSCROLL (0, space, VERTICAL, PAGE_FORWARD);

	BSCROLL (GDK_SHIFT_MASK, Left, HORIZONTAL, PAGE_BACKWARD);
	BSCROLL (GDK_SHIFT_MASK, KP_Left, HORIZONTAL, PAGE_BACKWARD);
	BSCROLL (GDK_SHIFT_MASK, Right, HORIZONTAL, PAGE_FORWARD);
	BSCROLL (GDK_SHIFT_MASK, KP_Right, HORIZONTAL, PAGE_FORWARD);

	/* editing */

#define BMOVE(m,key,dir,sk) \
	gtk_binding_entry_add_signal (binding_set, GDK_ ## key, m, \
				      "cursor_move", 2, \
				      GTK_TYPE_DIRECTION_TYPE, GTK_DIR_ ## dir, \
				      GTK_TYPE_HTML_CURSOR_SKIP, GTK_HTML_CURSOR_SKIP_ ## sk);

	BMOVE (0, Left,     LEFT,  ONE);
	BMOVE (0, KP_Left,  LEFT,  ONE);
	BMOVE (0, Right,    RIGHT, ONE);
	BMOVE (0, KP_Right, RIGHT, ONE);
	BMOVE (0, Up,       UP  ,  ONE);
	BMOVE (0, KP_Up,    UP  ,  ONE);
	BMOVE (0, Down,     DOWN,  ONE);
	BMOVE (0, KP_Down,  DOWN,  ONE);

	BMOVE (GDK_CONTROL_MASK, KP_Left,  LEFT,  WORD);
	BMOVE (GDK_CONTROL_MASK, Left,     LEFT,  WORD);
	BMOVE (GDK_MOD1_MASK,    Left,     LEFT,  WORD);

	BMOVE (GDK_CONTROL_MASK, KP_Right, RIGHT, WORD);
	BMOVE (GDK_CONTROL_MASK, Right,    RIGHT, WORD);
	BMOVE (GDK_MOD1_MASK,    Right,    RIGHT, WORD);

	BMOVE (0, Page_Up,       UP,   PAGE);
	BMOVE (0, KP_Page_Up,    UP,   PAGE);
	BMOVE (0, Page_Down,     DOWN, PAGE);
	BMOVE (0, KP_Page_Down,  DOWN, PAGE);

#define BCOM(m,key,com) \
	gtk_binding_entry_add_signal (binding_set, GDK_ ## key, m, \
				      "command", 1, \
				      GTK_TYPE_HTML_COMMAND, GTK_HTML_COMMAND_ ## com);
	BCOM (0, Home, SCROLL_BOD);
	BCOM (0, KP_Home, SCROLL_BOD);
	BCOM (0, End, SCROLL_EOD);
	BCOM (0, KP_End, SCROLL_EOD);

	BCOM (GDK_CONTROL_MASK, c, COPY);

	BCOM (0, Return, INSERT_PARAGRAPH);
	BCOM (0, KP_Enter, INSERT_PARAGRAPH);
	BCOM (0, BackSpace, DELETE_BACK_OR_INDENT_DEC);
	BCOM (GDK_SHIFT_MASK, BackSpace, DELETE_BACK_OR_INDENT_DEC);
	BCOM (0, Delete, DELETE);
	BCOM (0, KP_Delete, DELETE);

	BCOM (GDK_CONTROL_MASK | GDK_SHIFT_MASK, plus, ZOOM_IN);
	BCOM (GDK_CONTROL_MASK, plus, ZOOM_IN);
	BCOM (GDK_CONTROL_MASK, equal, ZOOM_IN);
	BCOM (GDK_CONTROL_MASK, KP_Add, ZOOM_IN);
	BCOM (GDK_CONTROL_MASK, minus, ZOOM_OUT);
	BCOM (GDK_CONTROL_MASK, KP_Subtract, ZOOM_OUT);
	BCOM (GDK_CONTROL_MASK, 8, ZOOM_RESET);
	BCOM (GDK_CONTROL_MASK, KP_Multiply, ZOOM_RESET);
	BCOM (GDK_CONTROL_MASK, space, SELECTION_MODE);
}

gint
gtk_html_set_iframe_parent (GtkHTML *html, GtkWidget *parent, HTMLObject *frame)
{
	gint depth = 0;
	g_assert (GTK_IS_HTML (parent));

	html->iframe_parent = parent;
	html->frame = frame;
	gtk_signal_emit (GTK_OBJECT (html_engine_get_top_html_engine (html->engine)->widget),
			 signals [IFRAME_CREATED], html);

	while (html->iframe_parent) {
		depth++;
	        html = GTK_HTML (html->iframe_parent);
	}
	
	return depth;
}

void
gtk_html_select_word (GtkHTML *html)
{
	HTMLEngine *e;

	if (!html->allow_selection)
		return;

	e = html->engine;
	if (html_engine_get_editable (e))
		html_engine_select_word_editable (e);
	else
		html_engine_select_word (e);

	html_engine_update_selection_active_state (html->engine, html->priv->event_time);
}

void
gtk_html_select_line (GtkHTML *html)
{
	HTMLEngine *e;

	if (!html->allow_selection)
		return;

	e = html->engine;
	if (html_engine_get_editable (e))
		html_engine_select_line_editable (e);
	else
		html_engine_select_line (e);

	html_engine_update_selection_active_state (html->engine, html->priv->event_time);
}

void
gtk_html_select_paragraph (GtkHTML *html)
{
	HTMLEngine *e;

	if (!html->allow_selection)
		return;

	e = html->engine;
	if (html_engine_get_editable (e))
		html_engine_select_paragraph_editable (e);
	/* FIXME: does anybody need this? if so bother me. rodo
	   else
	   html_engine_select_paragraph (e); */

	html_engine_update_selection_active_state (html->engine, html->priv->event_time);
}

void
gtk_html_select_paragraph_extended (GtkHTML *html)
{
	HTMLEngine *e;

	if (!html->allow_selection)
		return;

	e = html->engine;
	if (html_engine_get_editable (e))
		html_engine_select_paragraph_extended (e);
	/* FIXME: does anybody need this? if so bother me. rodo
	   else
	   html_engine_select_paragraph (e); */

	html_engine_update_selection_active_state (html->engine, html->priv->event_time);
}

void
gtk_html_select_all (GtkHTML *html)
{
	HTMLEngine *e;

	if (!html->allow_selection)
		return;

	e = html->engine;

	if (html_engine_get_editable (e))
		html_engine_select_all_editable (e);
	else {
		html_engine_select_all (e);
	}

	html_engine_update_selection_active_state (html->engine, html->priv->event_time);
}

void
gtk_html_api_set_language (GtkHTML *html)
{
	g_assert (GTK_IS_HTML (html));

	if (html->editor_api) {
		gchar *language;

		/* printf ("set language through API to '%s'\n",
		   GTK_HTML_CLASS (GTK_OBJECT (html)->klass)->properties->language); */

		language = html->engine->language
			? html->engine->language
			: GTK_HTML_CLASS (GTK_OBJECT (html)->klass)->properties->language;
		html->editor_api->set_language (html, language, html->editor_data);
		html_engine_spell_check (html->engine);
	}
}

void
gtk_html_set_editor_api (GtkHTML *html, GtkHTMLEditorAPI *api, gpointer data)
{
	html->editor_api  = api;
	html->editor_data = data;

	gtk_html_api_set_language (html);
}

static gchar *
get_value_nick (GtkHTMLCommandType com_type)
{
	GtkEnumValue *val;

	val = gtk_type_enum_get_values (GTK_TYPE_HTML_COMMAND);
	while (val->value_name) {
		if (val->value == com_type)
			return val->value_nick;
		val++;
	}

	g_warning ("Invalid GTK_TYPE_HTML_COMMAND enum value %d\n", com_type);

	return NULL;
}

void
gtk_html_editor_event_command (GtkHTML *html, GtkHTMLCommandType com_type, gboolean before)
{
	GtkArg *args [1];

	args [0] = gtk_arg_new (GTK_TYPE_STRING);

	GTK_VALUE_STRING (*args [0]) = get_value_nick (com_type);
	/* printf ("sending %s\n", GTK_VALUE_STRING (*args [0])); */
	gtk_html_editor_event (html, before ? GTK_HTML_EDITOR_EVENT_COMMAND_BEFORE : GTK_HTML_EDITOR_EVENT_COMMAND_AFTER, args);

	gtk_arg_free (args [0], FALSE);
}

void
gtk_html_editor_event (GtkHTML *html, GtkHTMLEditorEventType event, GtkArg **args)
{
	if (html->editor_api && !html->engine->block_events)
		(*html->editor_api->event) (html, event, args, html->editor_data);
}

gboolean
gtk_html_command (GtkHTML *html, const gchar *command_name)
{
	GtkEnumValue *val;

	val = gtk_type_enum_find_value (GTK_TYPE_HTML_COMMAND, command_name);
	if (val) {
		if (command (html, val->value)) {
			if (html->priv->update_styles)
				gtk_html_update_styles (html);
			return TRUE;
		}
	}

	return FALSE;
}

gboolean
gtk_html_edit_make_cursor_visible (GtkHTML *html)
{
	gboolean rv = FALSE;

	g_return_val_if_fail (GTK_IS_HTML (html), rv);

	html_engine_hide_cursor (html->engine);
	if (html_engine_make_cursor_visible (html->engine)) {
		gtk_adjustment_set_value (GTK_LAYOUT (html)->hadjustment, (gfloat) html->engine->x_offset);
		gtk_adjustment_set_value (GTK_LAYOUT (html)->vadjustment, (gfloat) html->engine->y_offset);
		rv = TRUE;
	}
	html_engine_show_cursor (html->engine);

	return rv;
}

gboolean
gtk_html_build_with_gconf ()
{
#ifdef GTKHTML_HAVE_GCONF
	return TRUE;
#else
	return FALSE;
#endif
}

static void
update_embedded_object_parent (HTMLObject *o, HTMLEngine *e, gpointer data)
{
	HTMLEngine *toplevel = data;
 
	if (html_object_is_embedded (o)) {
		html_embedded_reparent (HTML_EMBEDDED (o), GTK_WIDGET (toplevel->widget));
	}
}


static void
gtk_html_insert_html_generic (GtkHTML *html, GtkHTML *tmp, const gchar *html_src, gboolean obj_only)
{
	GtkWidget *window, *sw;
	HTMLObject *o;

	html_engine_freeze (html->engine);
	html_engine_delete (html->engine);
	if (!tmp)
		tmp = GTK_HTML (gtk_html_new_from_string (html_src, -1));

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	sw     = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (sw));
	gtk_container_add (GTK_CONTAINER (sw), GTK_WIDGET (tmp));
	gtk_widget_realize (GTK_WIDGET (tmp));
	html_image_factory_move_images (html->engine->image_factory, tmp->engine->image_factory);	
	html_object_forall (tmp->engine->clue, tmp->engine, update_embedded_object_parent, html->engine);
	
	/* copy the forms */
	g_list_foreach (tmp->engine->formList, (GFunc)html_form_set_engine, html->engine);

	if (tmp->engine->formList && html->engine->formList) {
		GList *form_last;
		
		form_last = g_list_last (html->engine->formList);
		tmp->engine->formList->prev = form_last;
		form_last->next = tmp->engine->formList;
	} else if (tmp->engine->formList) {
		html->engine->formList = tmp->engine->formList;
	}
	tmp->engine->formList = NULL;

	/* copy the objects */
	if (obj_only) {
		HTMLObject *next;
		g_return_if_fail (tmp->engine->clue && HTML_CLUE (tmp->engine->clue)->head
				  && HTML_CLUE (HTML_CLUE (tmp->engine->clue)->head)->head);

		html_undo_level_begin (html->engine->undo, "Append HTML", "Remove appended HTML");
		o = HTML_CLUE (tmp->engine->clue)->head;
		for (; o; o = next) {
			next = o->next;
			html_object_remove_child (o->parent, o);
			html_engine_append_flow (html->engine, o, html_object_get_recursive_length (o));
		}
		html_undo_level_end (html->engine->undo);
	} else {
		g_return_if_fail (tmp->engine->clue);

		o = tmp->engine->clue;
		tmp->engine->clue = NULL;
		html_engine_insert_object (html->engine, o,
					   html_object_get_recursive_length (o),
					   html_object_get_insert_level (o));
	}
	gtk_widget_destroy (window);
	html_engine_thaw (html->engine);
}

void
gtk_html_insert_html (GtkHTML *html, const gchar *html_src)
{
	g_return_if_fail (GTK_IS_HTML (html));

	gtk_html_insert_html_generic (html, NULL, html_src, FALSE);
}

void
gtk_html_insert_gtk_html (GtkHTML *html, GtkHTML *to_be_destroyed)
{
	g_return_if_fail (GTK_IS_HTML (html));

	gtk_html_insert_html_generic (html, to_be_destroyed, NULL, FALSE);
}

void
gtk_html_append_html (GtkHTML *html, const gchar *html_src)
{
	g_return_if_fail (GTK_IS_HTML (html));

	gtk_html_insert_html_generic (html, NULL, html_src, TRUE);
}

static void
set_magnification (HTMLObject *o, HTMLEngine *e, gpointer data)
{
	if (HTML_IS_FRAME (o)) {
		GtkHTML *html =  GTK_HTML (HTML_FRAME (o)->html);
		html_painter_set_magnification (html->engine->painter, html, *(gdouble *) data);
	} else if (HTML_IS_IFRAME (o)) {
		GtkHTML *html =  GTK_HTML (HTML_IFRAME (o)->html);
		html_painter_set_magnification (html->engine->painter, html, *(gdouble *) data);
	}
}

void
gtk_html_set_magnification (GtkHTML *html, gdouble magnification)
{
	g_return_if_fail (GTK_IS_HTML (html));

	if (magnification > 0.05 && magnification < 20.0) {
		html_object_forall (html->engine->clue, html->engine, set_magnification, &magnification);
		html_painter_set_magnification (html->engine->painter, html, magnification);
		html_object_change_set_down (html->engine->clue, HTML_CHANGE_ALL);
		html_engine_schedule_update (html->engine);
	}
}

#define MAG_STEP 1.1

void
gtk_html_zoom_in (GtkHTML *html)
{
	g_return_if_fail (GTK_IS_HTML (html));

	gtk_html_set_magnification (html, html->engine->painter->magnification * MAG_STEP);
}

void
gtk_html_zoom_out (GtkHTML *html)
{
	g_return_if_fail (GTK_IS_HTML (html));
	g_return_if_fail (HTML_IS_ENGINE (html->engine));

	gtk_html_set_magnification (html, html->engine->painter->magnification * (1.0/MAG_STEP));
}

void
gtk_html_zoom_reset (GtkHTML *html)
{
	g_return_if_fail (GTK_IS_HTML (html));

	gtk_html_set_magnification (html, 1.0);
}

void 
gtk_html_set_allow_frameset (GtkHTML *html, gboolean allow)
{
	g_return_if_fail (GTK_IS_HTML (html));
	g_return_if_fail (HTML_IS_ENGINE (html->engine));

	html->engine->allow_frameset = allow;
}

gboolean
gtk_html_get_allow_frameset (GtkHTML *html)
{
	g_return_val_if_fail (GTK_IS_HTML (html), FALSE);
	g_return_val_if_fail (HTML_IS_ENGINE (html->engine), FALSE);

	return html->engine->allow_frameset;	
}

void
gtk_html_print_set_master (GtkHTML *html, GnomePrintMaster *print_master)
{
	html->priv->print_master = print_master;
}

void
gtk_html_images_ref (GtkHTML *html)
{
	html_image_factory_ref_all_images (HTML_IMAGE_FACTORY (html->engine->image_factory));
}

void
gtk_html_images_unref (GtkHTML *html)
{
	html_image_factory_unref_all_images (HTML_IMAGE_FACTORY (html->engine->image_factory));
}

void
gtk_html_image_ref (GtkHTML *html, const gchar *url)
{
	html_image_factory_ref_image_ptr (HTML_IMAGE_FACTORY (html->engine->image_factory), url);
}

void
gtk_html_image_unref (GtkHTML *html, const gchar *url)
{
	html_image_factory_unref_image_ptr (HTML_IMAGE_FACTORY (html->engine->image_factory), url);
}

void
gtk_html_image_preload (GtkHTML *html, const gchar *url)
{
	html_image_factory_register (HTML_IMAGE_FACTORY (html->engine->image_factory), NULL, url, FALSE);
}

void
gtk_html_set_blocking (GtkHTML *html, gboolean block)
{
	html->engine->block = block;
}

gint
gtk_html_print_get_pages_num (GtkHTML *html, GnomePrintContext *print_context, gdouble header_height, gdouble footer_height)
{
	return html_engine_print_get_pages_num (html->engine, print_context, header_height, footer_height);
}

gboolean
gtk_html_has_undo (GtkHTML *html)
{
	return html_undo_has_undo_steps (html->engine->undo);
}

void
gtk_html_drop_undo (GtkHTML *html)
{
	html_undo_reset (html->engine->undo);
}