/*  zvtterm.c - Zed's Virtual Terminal
 *  Copyright (C) 1998  Michael Zucchi
 *
 *  The zvtterm widget.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <config.h>

/* please use K&R style indenting if you make changes.
   2, 4, or 8 character tabs are fine - MPZ */

/* need for 'gcc -ansi -pedantic' under GNU/Linux */
#ifndef _POSIX_SOURCE
#  define _POSIX_SOURCE 1
#endif
#include <sys/types.h>

/* we need GdkFont calls for now */
#ifdef GDK_DISABLE_DEPRECATED
#undef GDK_DISABLE_DEPRECATED
#include <gdk/gdkfont.h>
#include <gdk/gdkx.h>
#define GDK_DISABLE_DEPRECATED
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkclipboard.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkselection.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkbindings.h>

#include <libzvt/libzvt.h>

#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>

#include "zvt-marshal.h"
#include "zvt-accessible.h"
#include "zvt-accessible-factory.h"

#define READ_CONDITION (G_IO_IN | G_IO_HUP | G_IO_ERR)
#define WRITE_CONDITION (G_IO_OUT | G_IO_ERR)

/* define to 'x' to enable copious debug output */
#define d(x)

/* default font */
#define DEFAULT_FONT "-misc-fixed-medium-r-semicondensed--13-120-75-75-c-60-iso8859-1"

#define PADDING 0

/* forward declararations */
static void zvt_term_init (ZvtTerm *term);
static void zvt_term_class_init (ZvtTermClass *class);
static void zvt_term_init (ZvtTerm *term);
static void zvt_term_destroy (GtkObject *object);
static void zvt_term_realize (GtkWidget *widget);
static void zvt_term_unrealize (GtkWidget *widget);
static void zvt_term_map (GtkWidget *widget);
static void zvt_term_unmap (GtkWidget *widget);
static void zvt_term_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void zvt_term_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static gint zvt_term_expose (GtkWidget *widget, GdkEventExpose *event);
static gint zvt_term_button_press (GtkWidget *widget, GdkEventButton *event);
static gint zvt_term_button_release (GtkWidget *widget, GdkEventButton *event);
static gint zvt_term_motion_notify (GtkWidget *widget, GdkEventMotion *event);
static gint zvt_term_key_press (GtkWidget *widget, GdkEventKey *event);
static gint zvt_term_focus_in (GtkWidget *widget, GdkEventFocus *event);
static gint zvt_term_focus_out (GtkWidget *widget, GdkEventFocus *event);
static gint zvt_term_scroll_event (GtkWidget *widget, GdkEventScroll *event);
static gint zvt_term_selection_clear (GtkWidget *widget, GdkEventSelection *event);
static void zvt_term_style_set (GtkWidget *widget, GtkStyle *previous_style);
static void zvt_term_selection_get (GtkWidget *widget,
				    GtkSelectionData *selection_data_ptr, 
				    guint info, 
				    guint time);
static void zvt_term_child_died (ZvtTerm *term);
static void zvt_term_title_changed (ZvtTerm *term, VTTITLE_TYPE type, const char *str);
static void zvt_term_title_changed_raise (void *user_data, char *str, VTTITLE_TYPE type);
static void zvt_term_got_output (ZvtTerm *term, const gchar *buffer, gint count);
static gint zvt_term_cursor_blink (gpointer data);
static void zvt_term_scrollbar_moved (GtkAdjustment *adj, GtkWidget *widget);
static gboolean zvt_term_readdata (GIOChannel *source, GIOCondition condition, gpointer data);
static gboolean zvt_term_readmsg (GIOChannel *source, GIOCondition condition, gpointer data);
static void zvt_term_fix_scrollbar (ZvtTerm *term);
static void vtx_unrender_selection (struct _vtx *vx);
static void zvt_term_scroll (ZvtTerm *term, int n);
static void zvt_term_scroll_by_lines (ZvtTerm *term, int n);
static int vt_cursor_state(void *user_data, int state);
static gboolean zvt_term_writemore (GIOChannel *source, GIOCondition condition, gpointer data);
static void zvt_term_updated(ZvtTerm *term, int mode);
static void clone_col(unsigned short **dest, unsigned short *from);
static void zvt_term_real_copy_clipboard (ZvtTerm *term);
static void zvt_term_real_paste_clipboard (ZvtTerm *term);
static void zvt_term_real_selection_changed (ZvtTerm *term);
static AtkObject *zvt_term_get_accessible (GtkWidget *widget);

static void zvt_term_hierarchy_changed (GtkWidget *widget,
                                        GtkWidget *previous_toplevel);

/* callbacks from update layer */
void vt_draw_text(void *user_data, struct vt_line *line, int row, int col, int len, int attr);
void vt_scroll_area(void *user_data, int firstrow, int count, int offset, int fill);
static void vt_selection_changed(void *user_data);
gint zvt_input_add (gint source, GIOCondition condition, GIOFunc function, gpointer data);

/* static data */

/* The first 16 values are the ansi colors, the last
 * two are the default foreground and default background
 */
static gushort default_red[] = 
{0x0000,0xaaaa,0x0000,0xaaaa,0x0000,0xaaaa,0x0000,0xaaaa,
 0x5555,0xffff,0x5555,0xffff,0x5555,0xffff,0x5555,0xffff,
 0x0000, 0xffff};

static gushort default_grn[] = 
{0x0000,0x0000,0xaaaa,0x5555,0x0000,0x0000,0xaaaa,0xaaaa,
 0x5555,0x5555,0xffff,0xffff,0x5555,0x5555,0xffff,0xffff,
 0x0000, 0xffff};

static gushort default_blu[] = 
{0x0000,0x0000,0x0000,0x0000,0xaaaa,0xaaaa,0xaaaa,0xaaaa,
 0x5555,0x5555,0x5555,0x5555,0xffff,0xffff,0xffff,0xffff,
 0x0000, 0xffff};


/* GTK signals */
enum 
{
  CHILD_DIED,
  TITLE_CHANGED,
  GOT_OUTPUT,
  COPY_CLIPBOARD,
  PASTE_CLIPBOARD,
  SCROLL,
  SELECTION_CHANGED,
  LAST_SIGNAL
};
static guint term_signals[LAST_SIGNAL] = { 0 };


/* GTK parent class */
static GtkWidgetClass *parent_class = NULL;

gint
zvt_input_add (gint             source,
	       GIOCondition 	condition,
	       GIOFunc  	function,
	       gpointer         data)
{
  guint result;
  GIOChannel *channel;
      
  channel = g_io_channel_unix_new (source);
  result = g_io_add_watch (channel, condition, function, data);
  g_io_channel_unref (channel);

  return result;
}

GType
zvt_term_get_type (void)
{
  static GType term_type = 0;
  
  if (!term_type) {
      GTypeInfo term_info = {
	sizeof (ZvtTermClass),
	(GBaseInitFunc)0,
	(GBaseFinalizeFunc)0,
	(GClassInitFunc) zvt_term_class_init,
	(GClassFinalizeFunc)0,
	NULL,
	sizeof (ZvtTerm),
	0,
	(GInstanceInitFunc) zvt_term_init,
	NULL
      };
      
      term_type = g_type_register_static (gtk_widget_get_type (), 
					  "ZvtTerm",
					  &term_info,
					  0);
    }

  return term_type;
}


static void
zvt_term_class_init (ZvtTermClass *class)
{
  GObjectClass *object_class;
  GtkWidgetClass *widget_class;
  ZvtTermClass *term_class;
  GtkBindingSet  *binding_set;

  object_class = (GObjectClass*) class;
  widget_class = (GtkWidgetClass*) class;
  term_class = (ZvtTermClass*) class;

  parent_class = gtk_type_class (gtk_widget_get_type ());

  term_signals[CHILD_DIED] =
    g_signal_new ("child_died",
		  G_TYPE_FROM_CLASS (object_class),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (ZvtTermClass, child_died),
		  NULL,
		  NULL,
		  zvt_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  term_signals[TITLE_CHANGED] =
    g_signal_new ("title_changed",
		  G_TYPE_FROM_CLASS (object_class),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (ZvtTermClass, title_changed),
		  NULL,
		  NULL,
		  zvt_marshal_VOID__INT_POINTER,
		  G_TYPE_NONE, 2,
		  G_TYPE_INT,
		  G_TYPE_POINTER);
  
  term_signals[GOT_OUTPUT] =
      g_signal_new ("got_output",
		    G_TYPE_FROM_CLASS (object_class),	
		    G_SIGNAL_RUN_FIRST,
		    G_STRUCT_OFFSET (ZvtTermClass, got_output),
		    NULL,
		    NULL,
		    zvt_marshal_VOID__POINTER_INT,
		    G_TYPE_NONE, 2,
		    G_TYPE_POINTER,
		    G_TYPE_INT);
      
  term_signals[COPY_CLIPBOARD] = 
      g_signal_new ("copy_clipboard",
		    G_TYPE_FROM_CLASS (object_class),	
		    G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		    G_STRUCT_OFFSET (ZvtTermClass, copy_clipboard),
		    NULL,
		    NULL,
		    zvt_marshal_VOID__VOID,
		    G_TYPE_NONE, 0);

    term_signals[PASTE_CLIPBOARD] =
    g_signal_new ("paste_clipboard",
		    G_TYPE_FROM_CLASS (object_class),	
		    G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		    G_STRUCT_OFFSET (ZvtTermClass, paste_clipboard),
		    NULL,
		    NULL,
		    zvt_marshal_VOID__VOID,
		    G_TYPE_NONE, 0);

    term_signals[SCROLL] =
    g_signal_new ("scroll",
		  G_TYPE_FROM_CLASS (object_class), 
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (ZvtTermClass,scroll),
		  NULL,
		  NULL,
		  zvt_marshal_VOID__INT,
		  G_TYPE_NONE, 1, 
		  G_TYPE_INT);

    term_signals[SELECTION_CHANGED] =
    g_signal_new ("selection_changed",
		  G_TYPE_FROM_CLASS (object_class),
		  G_SIGNAL_RUN_LAST,	
		  G_STRUCT_OFFSET (ZvtTermClass, selection_changed),
		  NULL,
		  NULL,
		  zvt_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  GTK_OBJECT_CLASS (object_class)->destroy = zvt_term_destroy;

  widget_class->realize = zvt_term_realize;
  widget_class->unrealize = zvt_term_unrealize;
  widget_class->map = zvt_term_map;
  widget_class->unmap = zvt_term_unmap;
  widget_class->expose_event = zvt_term_expose;
  widget_class->focus_in_event = zvt_term_focus_in;
  widget_class->focus_out_event = zvt_term_focus_out;
  widget_class->size_request = zvt_term_size_request;
  widget_class->size_allocate = zvt_term_size_allocate;
  widget_class->key_press_event = zvt_term_key_press;
  widget_class->button_press_event = zvt_term_button_press;
  widget_class->button_release_event = zvt_term_button_release;
  widget_class->motion_notify_event = zvt_term_motion_notify;
  widget_class->get_accessible = zvt_term_get_accessible;
  widget_class->style_set = zvt_term_style_set;
  
  widget_class->scroll_event = zvt_term_scroll_event;

  widget_class->selection_clear_event = zvt_term_selection_clear;
  widget_class->selection_get = zvt_term_selection_get;

  widget_class->hierarchy_changed = zvt_term_hierarchy_changed;
  
  term_class->child_died = zvt_term_child_died;
  term_class->title_changed = zvt_term_title_changed;
  term_class->got_output = zvt_term_got_output;
  term_class->copy_clipboard = zvt_term_real_copy_clipboard;
  term_class->paste_clipboard = zvt_term_real_paste_clipboard;
  term_class->scroll = zvt_term_scroll;
  term_class->selection_changed = zvt_term_real_selection_changed;
  
  /* Setup default key bindings */
  binding_set = gtk_binding_set_by_class(term_class);
#if defined(sparc) || defined(__sparc__)  
  gtk_binding_entry_add_signal(binding_set,GDK_F16,0,"copy-clipboard",0);
  gtk_binding_entry_add_signal(binding_set,GDK_F20,0,"copy-clipboard",0);
  gtk_binding_entry_add_signal(binding_set,GDK_F18,0,"paste-clipboard",0);
  gtk_binding_entry_add_signal(binding_set,GDK_Page_Up,0,"scroll",1,G_TYPE_INT,-1);
  gtk_binding_entry_add_signal(binding_set,GDK_Page_Down,0,"scroll",1,G_TYPE_INT,1);
#endif
}

static const GtkTargetEntry target_table[] = {
  { "STRING" },
  { "TEXT" },
  { "COMPOUND_TEXT" },
#ifdef ZVT_UTF8
  { "UTF8_STRING" },
#endif
};

static void
zvt_term_init (ZvtTerm *term)
{
  struct _zvtprivate *zp;

  GTK_WIDGET_SET_FLAGS (term, GTK_CAN_FOCUS);

  /* create and configure callbacks for the teminal back-end */
  term->vx = vtx_new (80, 24, term);
  term->vx->vt.ring_my_bell = zvt_term_bell;
  term->vx->vt.change_my_name = zvt_term_title_changed_raise;

  /* set rendering callbacks */
  term->vx->draw_text = vt_draw_text;
  term->vx->scroll_area = vt_scroll_area;
  term->vx->selection_changed = vt_selection_changed;
term->vx->cursor_state = vt_cursor_state;

  term->shadow_type = GTK_SHADOW_NONE;
  term->term_window = NULL;
  term->cursor_bar = NULL;
  term->cursor_dot = NULL;
  term->cursor_current = NULL;
  term->font = NULL;
  term->font_bold = NULL;
  term->scroll_gc = NULL;
  term->fore_gc = NULL;
  term->back_gc = NULL;
  term->fore_last = 0;
  term->back_last = 0;
  /* Deprecated */
  /*
  term->color_ctx = 0;
  term->ic = NULL;
  */

  /* grid sizing */
  term->grid_width = term->vx->vt.width;
  term->grid_height = term->vx->vt.height;

  /* input handlers */
  term->input_id = -1;
  term->msg_id = -1;
  term->timeout_id = -1;

  /* bitfield flags */
  term->cursor_on = 0;
  term->cursor_filled = 0;
  term->cursor_blink_state = 0;
  term->scroll_on_keystroke = 0;
  term->scroll_on_output = 0;
  term->blink_enabled = 1;
  /* Deprecated */
  /*
  term->ic = NULL;
  */
  term->in_expose = 0;
  term->transparent = 0;
  term->shaded = 0;

  /* The correct defaults are backspace=ASCII_DEL
   * delete=ESCAPE_SEQUENCE, but we have this for backward compat.
   */
  term->backspace_binding = ZVT_ERASE_CONTROL_H;
  term->delete_binding = ZVT_ERASE_ASCII_DEL;
  
  /* private data - set before calling functions */
  zp = g_malloc(sizeof(*zp));
  zp->scrollselect_id = -1;
  zp->text_expand = 0;
  zp->text_expandlen = 0;
  zp->scroll_position = 0;
  zp->fonttype=0;
  zp->default_char=0;
  zp->bold_save = 0;
  zp->paste_id = -1;
  zp->paste = 0;
  
  zp->auto_hint = TRUE;
  
  /* background stuff */
  zp->background = 0;
  zp->background_queue = 0;
  
  /* for queuing colours till we can set them */
  zp->queue_red = 0;
  zp->queue_green = 0;
  zp->queue_blue = 0;

  zp->background_watch_window = NULL;
  zp->background_pixmap = NULL;
    
  g_object_set_data(G_OBJECT (term), "_zvtprivate", zp);

  /* charwidth, charheight set here */
  zvt_term_set_font_name (term, DEFAULT_FONT);

  /* scrollback position adjustment */
  term->adjustment =
    GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 1.0, 1.0, 1.0, 1.0));
  g_object_ref (G_OBJECT (term->adjustment));
  gtk_object_sink (GTK_OBJECT (term->adjustment));

  g_signal_connect (
      G_OBJECT (term->adjustment),
      "value_changed",
      G_CALLBACK (zvt_term_scrollbar_moved),
      term);

  /* selection received */
  gtk_selection_add_targets (
      GTK_WIDGET (term),
      GDK_SELECTION_PRIMARY,
      target_table, 
      G_N_ELEMENTS (target_table));
}

/**
 * zvt_term_set_blink:
 * @term: A &ZvtTerm widget.
 * @state: The blinking state.  If %TRUE, the cursor will blink.
 *
 * Use this to control the way the cursor is displayed (blinking/solid)
 */
void
zvt_term_set_blink (ZvtTerm *term, int state)
{
  g_return_if_fail (term != NULL);                     
  g_return_if_fail (ZVT_IS_TERM (term));

  if (!(term->blink_enabled ^ (state?1:0)))
    return;

  if (term->blink_enabled) {
    if (term->timeout_id != -1){
      gtk_timeout_remove (term->timeout_id);
      term->timeout_id = -1;
    }
    
    if (GTK_WIDGET_REALIZED (term))
      vt_cursor_state (GTK_WIDGET (term), 1);
    
    term->blink_enabled = 0;
  } else {
    term->timeout_id = gtk_timeout_add (500, zvt_term_cursor_blink, term);
    term->blink_enabled = 1;
  }
}

/**
 * zvt_term_set_scroll_on_keystroke:
 * @term: A &ZvtTerm widget.
 * @state: Desired state.
 * 
 * If @state is %TRUE, forces the terminal to jump out of the
 * scrollback buffer whenever a keypress is received.
 **/
void 
zvt_term_set_scroll_on_keystroke(ZvtTerm *term, int state)
{
  term->scroll_on_keystroke = (state != 0);
}

/**
 * zvt_term_set_scroll_on_output:
 * @term: A &ZvtTerm widget.
 * @state: Desired state.
 *
 * If @state is %TRUE, forces the terminal to scroll on output
 * being generated by a child process or by zvt_term_feed().
 */
void 
zvt_term_set_scroll_on_output   (ZvtTerm *term, int state)
{
  g_return_if_fail (term != NULL);                     
  g_return_if_fail (ZVT_IS_TERM (term));

  term->scroll_on_output = (state != 0);
}

/**
 * zvt_term_set_auto_window_hint:
 * @term: A &ZvtTerm widget.
 * @state: Desired state.
 *
 * If @state is %TRUE, then window hints will automatically
 * be set when the font is set or resized.  This function
 * should be called before the widget is realized, otherwise
 * it will not have an effect until the next font size change.
 *
 * Note that the default is %TRUE.
 */
void 
zvt_term_set_auto_window_hint (ZvtTerm *term, int state)
{
  struct _zvtprivate *zp;
  g_return_if_fail (term != NULL);                     
  g_return_if_fail (ZVT_IS_TERM (term));

  zp = _ZVT_PRIVATE(term);
  zp->auto_hint = state;
}

/**
 * zvt_term_set_backspace_binding:
 * @term: a #ZvtTerm
 * @binding: what the backspace key should send
 *
 * Determines the effect of the backspace key. The correct setting is
 * #ZVT_ERASE_ASCII_DEL. The other settings are all broken and only
 * provided for compatibility with broken applications that may be
 * running inside the terminal.
 *
 * #ZvtTerm defaults to #ZVT_ERASE_CONTROL_H for Backspace, which
 * is wrong, so all apps using #ZvtTerm need to call this function
 * to fix it.
 * 
 **/
void
zvt_term_set_backspace_binding (ZvtTerm *term, ZvtEraseBinding binding)
{
  g_return_if_fail (ZVT_IS_TERM (term));

  term->backspace_binding = binding;
}

/**
 * zvt_term_set_delete_binding:
 * @term: a #ZvtTerm
 * @binding: what the delete key should send
 *
 * Determines the effect of the delete key. The correct setting is
 * #ZVT_ERASE_ESCAPE_SEQUENCE. The other settings are all broken and
 * only provided for compatibility with broken applications that may
 * be running inside the terminal.
 *
 * #ZvtTerm defaults to #ZVT_ERASE_ASCII_DEL for Delete, which
 * is wrong, so all apps using #ZvtTerm need to call this function
 * to fix it.
 * 
 **/
void
zvt_term_set_delete_binding (ZvtTerm *term, ZvtEraseBinding binding)
{
  g_return_if_fail (ZVT_IS_TERM (term));

  term->delete_binding = binding;
}

/**
 * zvt_term_set_wordclass:
 * @term: A &ZvtTerm widget.
 * @class: A string of characters to consider a "word" character.
 *
 * Sets the list of characters (character class) that are considered
 * part of a word, when selecting by word.  The @class is defined
 * the same was as a regular expression character class (as normally
 * defined using []'s, but without those included).  A leading or trailing
 * hypen (-) is used to include a hyphen in the character class.
 *
 * Passing a %NULL @class restores the default behaviour of alphanumerics
 * plus "_"  (i.e. "A-Za-z0-9_").
 */
void zvt_term_set_wordclass(ZvtTerm *term, unsigned char *class)
{
  g_return_if_fail (term != NULL);                     
  g_return_if_fail (ZVT_IS_TERM (term));

  vt_set_wordclass(term->vx, class);
}

static void
term_force_size(ZvtTerm *term)
{
  struct _zvtprivate *zp = _ZVT_PRIVATE(term);
  if (GTK_WIDGET_REALIZED (term)) {

    /* ok, if we update the window hints ourselves, then we
       use that to implicitly resize the window, otherwise
       we do it explicitly */
    if (zp->auto_hint) {
      GdkGeometry hints;
      GtkWidget *app;
      
      app = gtk_widget_get_toplevel(GTK_WIDGET(term));
      g_assert (app != NULL);

      hints.base_width = (GTK_WIDGET (term)->style->xthickness * 2) + PADDING;
      hints.base_height =  (GTK_WIDGET (term)->style->ythickness * 2);
      
      hints.width_inc = term->charwidth;
      hints.height_inc = term->charheight;
      hints.min_width = hints.base_width + hints.width_inc;
      hints.min_height = hints.base_height + hints.height_inc;

      d(printf("setting window hints:\n  min %dx%d\n  base %dx%d\n  inc %dx%d\n",
	       hints.min_width, hints.min_height, hints.base_width, hints.base_height,
	       hints.width_inc, hints.height_inc));

      gtk_window_set_geometry_hints(GTK_WINDOW(app),
				    GTK_WIDGET(term),
				    &hints,
				    GDK_HINT_RESIZE_INC|GDK_HINT_MIN_SIZE|GDK_HINT_BASE_SIZE);
    }

    d(printf("forcing term size to %d, %d\n", term->grid_width, term->grid_height));
    d(printf("                   = %d, %d\n",
	     (term->grid_width * term->charwidth) + 
	     (GTK_WIDGET(term)->style->xthickness * 2) + PADDING,
	     (term->grid_height * term->charheight) + 
	     (GTK_WIDGET(term)->style->ythickness * 2)));
  }
}

/**
 * zvt_term_new_with_size:
 * @cols: Number of columns required.
 * @rows: Number of rows required.
 *
 * Creates a new ZVT Terminal widget of the given character dimentions.
 * If the encompassing widget is resizable, then this size may change
 * afterwards, but should be correct at realisation time.
 *
 * Return Value: A pointer to a &ZvtTerm widget is returned, or %NULL
 * on error.
 */
GtkWidget*
zvt_term_new_with_size (int cols, int rows)
{
  ZvtTerm *term;
  term = g_object_new (ZVT_TYPE_TERM, NULL);

  /* fudge the pixel size, not (really) used anyway */
  vt_resize (&term->vx->vt, cols, rows, cols*8, rows*8);

  term->grid_width = cols;
  term->grid_height = rows;

  return GTK_WIDGET (term);
}


/**
 * zvt_term_new:
 *
 * Creates a new ZVT Terminal widget.  By default the terminal will be
 * setup as 80 colmns x 24 rows, but it will size automatically to its
 * encompassing widget, and may be smaller or larger upon realisation.
 *
 * Return Value: A pointer to a &ZvtTerm widget is returned, or %NULL
 * on error.
 */
GtkWidget*
zvt_term_new (void)
{
  ZvtTerm *term;
  term = g_object_new (ZVT_TYPE_TERM, NULL);
  return GTK_WIDGET (term);
}

static void
zvt_term_destroy (GtkObject *object)
{
  ZvtTerm *term;
  struct _zvtprivate *zp;

  g_return_if_fail (ZVT_IS_TERM (object));

  term = ZVT_TERM (object);
  zp = _ZVT_PRIVATE(term);

  if (!zp)
    goto chain_destroy;
  
  if (term->timeout_id != -1) {
    gtk_timeout_remove(term->timeout_id);
    term->timeout_id = -1;
  }
  
  zvt_term_closepty (term);

  zvt_term_update_toplevel_watch (term, TRUE);
  
  vtx_destroy (term->vx);
  term->vx = NULL;

  if (term->font) {
    gdk_font_unref (term->font);
    term->font = NULL;
  }

  if (term->font_bold) {
    gdk_font_unref (term->font_bold);
    term->font_bold = NULL;
  }

  /* release the adjustment */
  if (term->adjustment) {
    g_signal_handlers_disconnect_matched (term->adjustment, G_SIGNAL_MATCH_DATA,
					  0, 0, NULL, NULL, term);
    g_object_unref (term->adjustment);
    term->adjustment = NULL;
  }

  /* Deprecated */
  /*
  if (term->ic) {
    gdk_ic_destroy(term->ic);
    term->ic = NULL;
  }
  */

  /* release private data */
  if (zp) {
    if (zp->text_expand)
      g_free(zp->text_expand);
    if (zp->bold_save)
      g_object_unref(zp->bold_save);
    if (zp->paste)
      g_free(zp->paste);
    if (zp->paste_id != -1)
      g_source_remove(zp->paste_id);

    if (zp->queue_red)
	    g_free(zp->queue_red);
    if (zp->queue_green)
	    g_free(zp->queue_green);
    if (zp->queue_blue)
	    g_free(zp->queue_blue);

    if (zp->background)
	zvt_term_background_unload (term);
    
    g_free(zp);
   g_object_set_data (G_OBJECT (term), "_zvtprivate", 0);
  }

 chain_destroy:
  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

/**
 * zvt_term_reset:
 * @term: A &ZvtTerm widget.
 * @hard: If %TRUE, then perform a HARD reset.
 * 
 * Performs a complete reset on the terminal.  Resets all
 * attributes, and if @hard is %TRUE, also clears the screen.
 **/
void
zvt_term_reset (ZvtTerm *term, int hard)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));

  vt_cursor_state (term, 0);
  vt_reset_terminal(&term->vx->vt, hard);
  vt_update (term->vx, UPDATE_CHANGES);
  vt_cursor_state (term, 1);
  zvt_term_updated(term, 2);
}

static void
clone_col(unsigned short **dest, unsigned short *from)
{
  if (*dest)
    g_free(*dest);
  if (from) {
    *dest = g_malloc(18 * sizeof(unsigned short));
    memcpy(*dest, from, 18 * sizeof(unsigned short));
  } else {
    *dest = 0;
  }
}

/**
 * zvt_term_set_color_scheme:
 * @term: A &ZvtTerm widget.
 * @red:  pointer to a gushort array of 18 elements with red values.
 * @grn:  pointer to a gushort array of 18 elements with green values.
 * @blu:  pointer to a gushort array of 18 elements with blue values.
 *
 * This function sets the colour palette for the terminal @term.  Each
 * pointer points to a gushort array of 18 elements.  White is 0xffff
 * in all elements.
 *
 * The elements 0 trough 15 are the first 16 colours for the terminal,
 * with element 16 and 17 the default foreground and background colour
 * respectively.
 */
void
zvt_term_set_color_scheme (ZvtTerm *term, gushort *red, gushort *grn, gushort *blu)
{
  GdkColor c;
  struct _zvtprivate *zp;
  gboolean success[18];
  gint i;
  
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));
  g_return_if_fail (red != NULL);
  g_return_if_fail (grn != NULL);
  g_return_if_fail (blu != NULL);

  zp = _ZVT_PRIVATE(term);

  /* Deprecated */
  /*
  if (term->color_ctx == NULL) {
    clone_col(&zp->queue_red, red);
    clone_col(&zp->queue_green, grn);
    clone_col(&zp->queue_blue, blu);
    return;
  }
  */

  memset (term->colors, 0, sizeof (term->colors));

  for(i=0;i<18;i++) {
	term->colors[i].red=  red[i];
	term->colors[i].green= grn[i];
	term->colors[i].blue= blu[i];
  }
  gdk_colormap_alloc_colors(term->color_map, term->colors, 18, FALSE, TRUE, success);

  /* Deprecated */
  /*
  gdk_color_context_get_pixels (term->color_ctx, red, grn, blu, 18, 
				term->colors, &nallocated);
  */
  c.pixel = term->colors [17].pixel;
  gdk_window_set_background (GTK_WIDGET (term)->window, &c);
  gdk_window_clear (GTK_WIDGET (term)->window);
  gtk_widget_queue_draw (GTK_WIDGET (term));

  /* Need to unset these so the GC's get updated properly */
  term->back_last = -1;
  term->fore_last = -1;

  /* If we don't have a background pixmap, we need to update the
   * background gc.
   */
  if (!zp->background || zp->background->type == ZVT_BGTYPE_NONE) {
         if (term->back_gc) {
	        GdkColor pen;

		pen.pixel = term->colors[17].pixel;
		gdk_gc_set_foreground (term->back_gc, &pen);
	  }
  }
  
  /* always clear up any old queued values */
  clone_col(&zp->queue_red, 0);
  clone_col(&zp->queue_green, 0);
  clone_col(&zp->queue_blue, 0);
}

/**
 * zvt_term_set_default_color_scheme:
 * @term: A &ZvtTerm widget.
 *
 * Resets the color values to the default color scheme.
 */
void
zvt_term_set_default_color_scheme (ZvtTerm *term)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));
  
  zvt_term_set_color_scheme (term, default_red, default_grn, default_blu);
}

/**
 * zvt_term_set_size:
 * @term: A &ZvtTerm widget.
 * @width: Width of terminal, in columns.
 * @height: Height of terminal, in rows.
 * 
 * Causes the terminal to attempt to resize to the absolute character
 * size of @width rows by @height columns.
 **/
void
zvt_term_set_size (ZvtTerm *term, guint width, guint height)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));

  term->grid_width = width;
  term->grid_height = height;
  term_force_size(term);
}

static void
zvt_term_style_set (GtkWidget *widget, GtkStyle *previous_style)
{
  GdkColor c;

  if (GTK_WIDGET_CLASS (parent_class)->style_set)
    parent_class->style_set (widget, previous_style);

  if (GTK_WIDGET_REALIZED (widget))
    {
      c.pixel = ZVT_TERM (widget)->colors [17].pixel;  
      gdk_window_set_background (widget->window, &c);
    }
}

static void
zvt_term_realize (GtkWidget *widget)
{
  ZvtTerm *term;
  GdkWindowAttr attributes;
  GdkPixmap *cursor_dot_pm;
  gint attributes_mask;
  struct _zvtprivate *zp;
  GdkColor c;
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
  term = ZVT_TERM (widget);
  zp = _ZVT_PRIVATE(term);

  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width - (2 * widget->style->xthickness) - PADDING;
  attributes.height = widget->allocation.height - (2 * widget->style->ythickness);
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.event_mask = gtk_widget_get_events (widget) |
    GDK_EXPOSURE_MASK | 
    GDK_BUTTON_PRESS_MASK | 
    GDK_BUTTON_MOTION_MASK |
    GDK_POINTER_MOTION_MASK | 
    GDK_BUTTON_RELEASE_MASK | 
    GDK_KEY_PRESS_MASK;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

  /* main window */
  widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
				   &attributes, attributes_mask);
  widget->style = gtk_style_attach (widget->style, widget->window);
  gdk_window_set_user_data (widget->window, widget);
#if 1
  c.pixel = term->colors [17].pixel;  
  gdk_window_set_background (widget->window, &c);
#else
  gdk_window_set_back_pixmap(widget->window, 0, 1);
#endif

  /* this should never have been created *sigh* */
  /* keep for compatability */
  term->term_window = widget->window;

  /* create pixmaps for this window */
  cursor_dot_pm = 
    gdk_pixmap_create_from_data(widget->window,
				"\0", 1, 1, 1,
				&widget->style->fg[GTK_STATE_ACTIVE],
				&widget->style->bg[GTK_STATE_ACTIVE]);

  /* Get I beam cursor, and also create a blank one based on the blank image */
  term->cursor_bar = gdk_cursor_new(GDK_XTERM);
  term->cursor_dot = 
    gdk_cursor_new_from_pixmap(cursor_dot_pm, cursor_dot_pm,
			       &widget->style->fg[GTK_STATE_ACTIVE],
			       &widget->style->bg[GTK_STATE_ACTIVE],
			       0, 0);
  gdk_window_set_cursor(widget->window, term->cursor_bar);
  g_object_unref (cursor_dot_pm);
  zp->cursor_hand = gdk_cursor_new(GDK_HAND2);
  term->cursor_current = term->cursor_bar;

  /* setup scrolling gc */
  term->scroll_gc = gdk_gc_new (widget->window);
  gdk_gc_set_exposures (term->scroll_gc, TRUE);

  /* Colors */
  term->fore_gc = gdk_gc_new (widget->window);
  term->back_gc = gdk_gc_new (widget->window);
  /* Create the colormap */
  term->color_map=gtk_widget_get_colormap(GTK_WIDGET (term));


  /* Deprecated */
  /*
  term->color_ctx = 
      gdk_color_context_new (gtk_widget_get_visual (GTK_WIDGET (term)),
			     gtk_widget_get_colormap (GTK_WIDGET (term)));
  */

  /* Allocate default or requested colour set */
  if (zp->queue_red != NULL && zp->queue_green != NULL && zp->queue_blue != NULL) {
    zvt_term_set_color_scheme(term, zp->queue_red, zp->queue_green, zp->queue_blue);
  } else {
    zvt_term_set_default_color_scheme (term);
  }
  
  /* set the initial colours */
  term->back_last = -1;
  term->fore_last = -1;

  /* and the initial size ... */
  term_force_size(term);

  /* input context */
#ifdef UNDEF
  if (gdk_im_ready () && !term->ic) {
    GdkICAttr attr;
    
    /* FIXME: do we have any window yet? */
    attr.style = GDK_IM_PREEDIT_NOTHING | GDK_IM_STATUS_NOTHING;
    attr.client_window = attr.focus_window = widget->window;
    term->ic = gdk_ic_new(&attr, GDK_IC_ALL_REQ);
    
    if (!term->ic) {
      g_warning("Can't create input context.");
    }
  }
#endif /* UNDEF */
  /* if we have a delayed background set, set it here? */
  if (zp->background_queue) {
    zvt_term_background_load(term, zp->background_queue);
    zvt_term_background_unref(zp->background_queue);
    zp->background_queue = 0;
  }
}

static void
zvt_term_unrealize (GtkWidget *widget)
{
  ZvtTerm *term;
  struct _zvtprivate *zp;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));

  term = ZVT_TERM (widget);
  zp = _ZVT_PRIVATE(term);

  /* free resources */
  gdk_cursor_unref (term->cursor_bar);
  term->cursor_bar = NULL;

  gdk_cursor_unref (term->cursor_dot);
  term->cursor_dot = NULL;

  gdk_cursor_unref (zp->cursor_hand);
  zp->cursor_hand = NULL;

  term->cursor_current = NULL;

  /* Deprecated */
  /*
  gdk_color_context_free (term->color_ctx);
  term->color_ctx = NULL;
  */
  g_object_unref(term->color_map);


  g_object_unref (term->scroll_gc);
  term->scroll_gc = NULL;

  g_object_unref (term->back_gc);
  term->back_gc = NULL;

  g_object_unref (term->fore_gc);
  term->fore_gc = NULL;

  if (GTK_WIDGET_CLASS (parent_class)->unrealize)
    (*GTK_WIDGET_CLASS (parent_class)->unrealize) (widget);
}


static void
zvt_term_map (GtkWidget *widget)
{
  ZvtTerm *term;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));

  term = ZVT_TERM (widget);

  if (!GTK_WIDGET_MAPPED (widget)) {
    GTK_WIDGET_SET_FLAGS (widget, GTK_MAPPED);
    
    gdk_window_show (widget->window);
  }
}


static void
zvt_term_unmap (GtkWidget *widget)
{
  ZvtTerm *term;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));

  term = ZVT_TERM (widget);

  if (GTK_WIDGET_MAPPED (widget)) {
    GTK_WIDGET_UNSET_FLAGS (widget, GTK_MAPPED);
    
    gdk_window_hide (widget->window);
  }
}


static gint
zvt_term_focus_in(GtkWidget *widget, GdkEventFocus *event)
{
  ZvtTerm *term;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (ZVT_IS_TERM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  term = ZVT_TERM (widget);
  GTK_WIDGET_SET_FLAGS (widget, GTK_HAS_FOCUS);

  vt_cursor_state(term, 0);
  term->cursor_filled = 1;
  vt_cursor_state(term, 1);

  /* setup blinking cursor */
  if (term->blink_enabled && term->timeout_id == -1)
    term->timeout_id = gtk_timeout_add (500, zvt_term_cursor_blink, term);

  /* Deprecated */
  /*
  if (term->ic)
    gdk_im_begin (term->ic, widget->window);
  */

  return FALSE;
}


static gint
zvt_term_focus_out(GtkWidget *widget, GdkEventFocus *event)
{
  ZvtTerm *term;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (ZVT_IS_TERM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  term = ZVT_TERM (widget);
  GTK_WIDGET_UNSET_FLAGS (widget, GTK_HAS_FOCUS);

  vt_cursor_state(term, 0);
  term->cursor_filled = 0;
  vt_cursor_state(term, 1);

  /* setup blinking cursor */
  if (term->blink_enabled && term->timeout_id != -1) {
    gtk_timeout_remove(term->timeout_id);
    term->timeout_id = -1;
  }
  
  /* Deprecated */
  /*
  if (term->ic)
    gdk_im_end ();
  */

  return FALSE;
}

static void 
zvt_term_size_request (GtkWidget      *widget,
		       GtkRequisition *requisition)
{
  ZvtTerm *term;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));
  g_return_if_fail (requisition != NULL);

  term = ZVT_TERM (widget);

  requisition->width = term->charwidth + widget->style->xthickness * 2 + PADDING;
  requisition->height = term->charheight + widget->style->ythickness * 2;

  /* debug ouput */
  d( printf("zvt_term_size_request x=%d y=%d\n", grid_width, grid_height) );
  d( printf("   requestion size w=%d, h=%d\n", requisition->width, requisition->height) );
}

static void
zvt_term_size_allocate (GtkWidget     *widget,
			GtkAllocation *allocation)
{
  gint term_width, term_height;
  guint grid_width, grid_height;
  ZvtTerm *term;
  struct _zvtprivate *zp;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));
  g_return_if_fail (allocation != NULL);

  widget->allocation = *allocation;

  if (GTK_WIDGET_REALIZED (widget)) {
      term = ZVT_TERM (widget);
      zp = _ZVT_PRIVATE(term);

      d( printf("zvt_term_size_allocate old grid x=%d y=%d\n", 
		term->vx->vt.width,
		term->vx->vt.height) );
      
      d(printf("old size w=%d h=%d\n", widget->allocation.width, widget->allocation.height));
      d(printf("new allocation w=%d h=%d\n", allocation->width, allocation->height));

      gdk_window_move_resize (widget->window,
			      allocation->x,
			      allocation->y,
			      allocation->width,
			      allocation->height);

      term_width = allocation->width - (2 * widget->style->xthickness) - PADDING;
      term_height = allocation->height - (2 * widget->style->ythickness);

      /* resize the virtual terminal buffer, if its size has changed, minimal size is 1x1 */
      grid_width = MAX(term_width / term->charwidth, 1);
      grid_height = MAX(term_height / term->charheight, 1);
      if (grid_width != term->charwidth
	  || grid_height != term->charheight) {
	/* turn off the selection */
	term->vx->selstartx = term->vx->selendx;
	term->vx->selstarty = term->vx->selendy;
	term->vx->selected = 0;

	vt_resize (&term->vx->vt, grid_width, grid_height, term_width, term_height);
	vt_update (term->vx, UPDATE_REFRESH|UPDATE_SCROLLBACK);
        
	d(printf("zvt_term_size_allocate grid calc x=%d y=%d\n", 
		 grid_width,
		 grid_height) );

	term->grid_width = grid_width;
	term->grid_height = grid_height;
      }

      /* resize the scrollbar */
      zvt_term_fix_scrollbar (term);
      zvt_term_updated(term, 2);

      d( printf("zvt_term_size_allocate new grid x=%d y=%d\n", 
		term->vx->vt.width,
		term->vx->vt.height) );
  }
}

static gint
zvt_term_expose (GtkWidget      *widget,
		 GdkEventExpose *event)
{
  ZvtTerm *term;
  int offx, offy;
  int fill;
  struct _zvtprivate *zp;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (ZVT_IS_TERM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  offx = widget->style->xthickness + PADDING;
  offy = widget->style->ythickness;

  d(printf("exposed!  (%d,%d) %dx%d\n",
	   event->area.x, 
	   event->area.y,
	   event->area.width, 
	   event->area.height));

  if (GTK_WIDGET_DRAWABLE (widget)) {
    term = ZVT_TERM (widget);
    zp = _ZVT_PRIVATE(widget);
    
    /* FIXME: may update 1 more line/char than needed */
    term->in_expose = 1;

    if(zp->background) {
      gdk_draw_rectangle (widget->window,
			  term->back_gc, 1,
			  event->area.x, 
			  event->area.y,
			  event->area.width, 
			  event->area.height);
    }
    fill = 17;

    vt_update_rect (term->vx, fill,
		    (event->area.x-offx) / term->charwidth,
		    (event->area.y-offy) / term->charheight,
		    (event->area.x + event->area.width) / term->charwidth+1,
		    (event->area.y + event->area.height) / term->charheight+1);

    term->in_expose = 0;

    if (term->shadow_type != GTK_SHADOW_NONE)
      gtk_paint_shadow (widget->style, widget->window,
		        GTK_STATE_NORMAL, term->shadow_type, 
			NULL, widget, NULL, 0, 0, 
		        widget->allocation.width,
		        widget->allocation.height); 
  }

  return FALSE;
}


/**
 * zvt_term_show_pointer:
 * @term: A &ZvtTerm widget.
 *
 * Show the default I beam pointer.
 */
void
zvt_term_show_pointer (ZvtTerm *term)
{
  g_return_if_fail (term != NULL);

  if (term->cursor_current == term->cursor_dot) {
    gdk_window_set_cursor(GTK_WIDGET(term)->window, term->cursor_bar);
    term->cursor_current = term->cursor_bar;
  }
}

static void zvt_term_set_pointer(ZvtTerm *term, GdkCursor *c)
{
  if (term->cursor_current != c) {
    gdk_window_set_cursor(GTK_WIDGET(term)->window, c);
    term->cursor_current = c;
  }
}

/**
 * zvt_term_show_pointer:
 * @term: A &ZvtTerm widget.
 *
 * Hide the pointer.  In reality the pointer is changed to a
 * single-pixel black dot.
 */
void
zvt_term_hide_pointer (ZvtTerm *term)
{
  g_return_if_fail (term != NULL);

  if (term->cursor_current != term->cursor_dot) {
    gdk_window_set_cursor(GTK_WIDGET(term)->window, term->cursor_dot);
    term->cursor_current = term->cursor_dot;
  }
}

/**
 * zvt_term_set_scrollback:
 * @term: A &ZvtTerm widget.
 * @lines: Number of lines desired for the scrollback buffer.
 *
 * Set the maximum number of scrollback lines for the widget @term to
 * @lines lines.
 */
void
zvt_term_set_scrollback (ZvtTerm *term, int lines)
{
  g_return_if_fail (term != NULL);

  vt_scrollback_set (&term->vx->vt, lines);
  zvt_term_fix_scrollbar (term);
}


/* Load a set of fonts into the terminal.
 * These fonts should be the same size, otherwise it could get messy ...
i to * if font_bold is NULL, then the font is emboldened manually (overstrike)
 */
static void
zvt_term_set_fonts_internal(ZvtTerm *term, GdkFont *font, GdkFont *font_bold)
{
  struct _zvtprivate *zp;

  /* ignore no-font setting */
  if (font==NULL)
    return;

  zp = _ZVT_PRIVATE(term);

  /* get the font/fontset size, and set the font type */
  switch (font->type) {
  case GDK_FONT_FONT: {
    XFontStruct *xfont;
    xfont = (XFontStruct *)GDK_FONT_XFONT(font);
    term->charwidth = xfont->max_bounds.width;
    term->charheight = font->ascent + font->descent;
    if ((xfont->min_byte1 == 0) && (xfont->max_byte1 == 0))
      zp->fonttype = ZVT_FONT_1BYTE;
    else
      zp->fonttype = ZVT_FONT_2BYTE;
    break;
  }
  case GDK_FONT_FONTSET: {
    XFontSet fontset = (XFontSet) GDK_FONT_XFONT(font);
    XFontSetExtents *extents = XExtentsOfFontSet(fontset);
    term->charwidth = extents->max_logical_extent.width;
    term->charheight = extents->max_logical_extent.height;
    zp->fonttype = ZVT_FONT_FONTSET;
  }
  }
  d(printf("fonttype = %d\n", zp->fonttype));

  /* set the desired size, and force a resize */
  term->grid_width = term->vx->vt.width;
  term->grid_height = term->vx->vt.height;
  term_force_size(term);

  if (term->font)
    gdk_font_unref (term->font);
  term->font = font;

  if (term->font_bold)
    gdk_font_unref (term->font_bold);
  term->font_bold = font_bold;

  /* setup bold_save pixmap, for when drawing bold, we need to save
     what was at the end of the text, so we can restore it.  Affects
     tiny fonts only - what a mess */
  if (zp && term->font_bold==0) {
    int depth;

    if (zp->bold_save)
      g_object_unref(zp->bold_save);
    gdk_window_get_geometry(GTK_WIDGET(term)->window,NULL,NULL,NULL,NULL,&depth);

    zp->bold_save = gdk_pixmap_new(GTK_WIDGET(term)->window,
				   1, term->charheight, depth);
  }

  gtk_widget_queue_resize (GTK_WIDGET (term));
}

/**
 * zvt_term_set_fonts:
 * @term: A &ZvtTerm widget.
 * @font: Font used for regular text.
 * @font_bold: Font used for bold text.  May be null, in which case the bold
 * font is rendered by over-striking.
 *
 * Load a set of fonts into the terminal.
 * 
 * These fonts should be the same size, otherwise it could get messy ...
 */
void
zvt_term_set_fonts (ZvtTerm *term, GdkFont *font, GdkFont *font_bold)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));
  g_return_if_fail (font != NULL);

  gdk_font_ref (font);
  if (font_bold)
      gdk_font_ref (font_bold);
  
  zvt_term_set_fonts_internal (term, font, font_bold);
}

/**
 * zvt_term_set_font_name:
 * @term: A &ZvtTerm widget.
 * @name: A full X11 font name string.
 *
 * Set a font by name only.  If font aliases such as 'fixed' or
 * '10x20' are passed to this function, then both the bold and
 * non-bold font will be identical.  In colour mode bold fonts are
 * always the top 8 colour scheme entries, and so bold is still
 * rendered.
 *
 * Tries to calculate bold font name from the base name.  This only
 * works with fonts where the names are alike.
 */
void
zvt_term_set_font_name (ZvtTerm *term, char *name)
{
  int count;
  char c, *ptr, *rest;
  GString *newname, *outname;
  GdkFont *font, *font_bold;

  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));
  g_return_if_fail (name != NULL);

  newname = g_string_new (name);
  outname = g_string_new ("");

  rest = 0;
  ptr = newname->str;

  for (count = 0; (c = *ptr++);) {
    if (c == '-') {
      count++;
      
      d(printf("scanning (%c) (%d)\n", c, count));
      d(printf("newname = %s ptr = %s\n", newname->str, ptr));
      
      switch (count) {
      case 3:
	ptr[-1] = 0;
	break;
	
      case 5:
	rest = ptr - 1;
	break;
      }
    }
  }

  if (rest) {
    g_string_printf (outname, "%s-medium-r%s", newname->str, rest);
    font = gdk_font_load (outname->str);
    d( printf("loading normal font %s\n", outname->str) );
    
    g_string_printf (outname, "%s-bold-r%s", newname->str, rest); 
    font_bold = gdk_font_load (outname->str);
    d( printf("loading bold font %s\n", outname->str) );
    
    zvt_term_set_fonts_internal (term, font, font_bold);
  } else {
    font = gdk_font_load (name);
    zvt_term_set_fonts_internal (term, font, 0);
  }

  g_string_free (newname, TRUE);
  g_string_free (outname, TRUE);
}


/*
  Called when something has changed, size of window or scrollback.

  Fixes the adjustment and notifies the system.
*/
static void 
zvt_term_fix_scrollbar (ZvtTerm *term)
{
  GTK_ADJUSTMENT(term->adjustment)->upper = 
    term->vx->vt.scrollbacklines + term->vx->vt.height - 1;

  GTK_ADJUSTMENT(term->adjustment)->value = 
    term->vx->vt.scrollbackoffset + term->vx->vt.scrollbacklines;

  GTK_ADJUSTMENT(term->adjustment)->page_increment = 
    term->vx->vt.height - 1;

  GTK_ADJUSTMENT(term->adjustment)->page_size =
    term->vx->vt.height - 1;

  g_signal_emit_by_name (term->adjustment, "changed");
}

static void
paste_received (GtkClipboard *clipboard,
		const char   *text,
		gpointer      data)
{
  ZvtTerm *term = ZVT_TERM (data);

  /* paste selection into window! */
  if (text)
    {
      int i;
      /* FIXME: pasting utf8 doesn't really work! */
      char *buf = gdk_utf8_to_string_target (text);
      int len = strlen (buf);
      char *ctmp = buf;

      for(i = 0; i < len; i++)
	if(ctmp[i] == '\n') ctmp[i] = '\r';

      if (term->scroll_on_keystroke)
	zvt_term_scroll (term, 0);
      zvt_term_writechild(term, buf, len);
      g_free (buf);
    }

  g_object_unref (G_OBJECT (term));
}
    

static void
zvt_term_paste (ZvtTerm *term, GdkAtom selection)
{
  g_object_ref (G_OBJECT (term));
  gtk_clipboard_request_text (gtk_clipboard_get (selection),
			      paste_received, term);
}

/*
  perhaps most of the button press stuff could be shifted
  to the update file.  as for the report_button function
  shifted to the vt file ?

*/

static gint
zvt_term_button_press (GtkWidget      *widget,
		       GdkEventButton *event)
{
  gint x,y;
  GdkModifierType mask;
  struct _vtx *vx;
  ZvtTerm *term;
  struct _zvtprivate *zp;

  d(printf("button pressed\n"));

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (ZVT_IS_TERM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  term = ZVT_TERM (widget);
  vx = term->vx;
  zp = _ZVT_PRIVATE(term);

  zvt_term_show_pointer (term);


  gdk_window_get_pointer(widget->window, &x, &y, &mask);
  x /= term->charwidth;
  y = y / term->charheight + vx->vt.scrollbackoffset;

  if (zp && zp->scrollselect_id!=-1) {
    gtk_timeout_remove(zp->scrollselect_id);
    zp->scrollselect_id=-1;
  }

  /* Shift is an override key for the reporting of the buttons */
  if (!(event->state & GDK_SHIFT_MASK))
    if (vt_report_button(&vx->vt, 1, event->button, event->state, x, y)) 
      return TRUE;

  /* ignore all control-clicks' at this level */
  if (event->state & GDK_CONTROL_MASK) {
    return FALSE;
  }
    
  switch(event->button) {
  case 1:			/* left button */

    /* set selection type, and from which end we are selecting */
    switch(event->type) {
    case GDK_BUTTON_PRESS:
      vx->selectiontype = VT_SELTYPE_CHAR|VT_SELTYPE_BYSTART;
      break;
    case GDK_2BUTTON_PRESS:
      vx->selectiontype = VT_SELTYPE_WORD|VT_SELTYPE_BYSTART|VT_SELTYPE_MOVED;
      break;
    case GDK_3BUTTON_PRESS:
      vx->selectiontype = VT_SELTYPE_LINE|VT_SELTYPE_BYSTART|VT_SELTYPE_MOVED;
      break;
    default:
      break;
    }
    
    /* reset selection */
    vx->selstartx = x;
    vx->selstarty = y;
    vx->selendx = x;
    vx->selendy = y;
    
    /* reset 'drawn' screen (to avoid mis-refreshes) */
    if (!vx->selected) {
      vx->selstartxold = x;
      vx->selstartyold = y;
      vx->selendxold = x;
      vx->selendyold = y;
      vx->selected =1;
    }

    if (event->type != GDK_BUTTON_PRESS) {
      vt_fix_selection(vx);	/* handles by line/by word select update */
    }

    /* either draw (double/triple click) or undraw (single click) selection */
    vt_draw_selection(vx);

    d( printf("selection starting %d %d\n", x, y) );

    gtk_grab_add (widget);
    gdk_pointer_grab (widget->window, TRUE,
		      GDK_BUTTON_RELEASE_MASK |
		      GDK_BUTTON_MOTION_MASK |
		      GDK_POINTER_MOTION_HINT_MASK,
		      NULL, NULL, 0);

    /* 'block' input while we're selecting text */
    if (term->input_id!=-1) {
      g_source_remove(term->input_id);
      term->input_id=-1;
    }
    break;

  case 2:			/* middle button - paste */
    zvt_term_paste (ZVT_TERM (widget), GDK_SELECTION_PRIMARY);
    break;

  case 3:			/* right button - select extend? */

    if (vx->selected) {
      int midpos;
      int mypos;

      switch(event->type) {
      case GDK_BUTTON_PRESS:
	vx->selectiontype = VT_SELTYPE_CHAR|VT_SELTYPE_MOVED;
	break;
      case GDK_2BUTTON_PRESS:
	vx->selectiontype = VT_SELTYPE_WORD|VT_SELTYPE_MOVED;
	break;
      case GDK_3BUTTON_PRESS:
	vx->selectiontype = VT_SELTYPE_LINE|VT_SELTYPE_MOVED;
	break;
      default:
	break;
      }

      midpos = ((vx->selstarty+vx->selendy)/2)*vx->vt.width + (vx->selendx+vx->selstartx)/2;
      mypos = y*vx->vt.width + x;
      if (mypos < midpos) {
	vx->selstarty=y;
	vx->selstartx=x;
	vx->selectiontype |= VT_SELTYPE_BYEND;
      } else {
	vx->selendy=y;
	vx->selendx=x;
	vx->selectiontype |= VT_SELTYPE_BYSTART;
      }

      vt_fix_selection(vx);
      vt_draw_selection(vx);
      
      gtk_grab_add (widget);
      gdk_pointer_grab (widget->window, TRUE,
			GDK_BUTTON_RELEASE_MASK |
			GDK_BUTTON_MOTION_MASK |
			GDK_POINTER_MOTION_HINT_MASK,
			NULL, NULL, 0);
      /* 'block' input while we're selecting text */
      if (term->input_id!=-1) {
	g_source_remove(term->input_id);
	term->input_id=-1;
      }
    }
    break;
  }

  return TRUE;
}

static gint
zvt_term_button_release (GtkWidget      *widget,
			 GdkEventButton *event)
{
  ZvtTerm *term;
  gint x, y;
  GdkModifierType mask;
  struct _vtx *vx;
  struct _zvtprivate *zp;

  d(printf("button released\n"));

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (ZVT_IS_TERM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  term = ZVT_TERM (widget);
  vx = term->vx;
  zp = _ZVT_PRIVATE(term);

  gdk_window_get_pointer (widget->window, &x, &y, &mask);

  x = x/term->charwidth;
  y = y/term->charheight + vx->vt.scrollbackoffset;

  /* reset scrolling selection timer if it is enabled */
  if (zp && zp->scrollselect_id!=-1) {
    gtk_timeout_remove(zp->scrollselect_id);
    zp->scrollselect_id=-1;
  }

  /* ignore wheel mice buttons (4 and 5) */
  /* otherwise they affect the selection */
  if (event->button == 4 || event->button == 5)
    return FALSE;
  
  if (vx->selectiontype == VT_SELTYPE_NONE) {
    /* report mouse to terminal */
    if (!(event->state & GDK_SHIFT_MASK))
      if (vt_report_button(&vx->vt, 0, event->button, event->state, x, y))
	return FALSE;
    
    /* ignore all control-clicks' at this level */
    if (event->state & GDK_CONTROL_MASK) {
      return FALSE;
    }
  }

  if (vx->selectiontype & VT_SELTYPE_BYSTART) {
    vx->selendx = x;
    vx->selendy = y;
  } else {
    vx->selstartx = x;
    vx->selstarty = y;
  }

  switch(event->button) {
  case 1:
  case 3:
    d(printf("select from (%d,%d) to (%d,%d)\n", vx->selstartx, vx->selstarty,
	     vx->selendx, vx->selendy));

    gtk_grab_remove (widget);
    gdk_pointer_ungrab (0);

    /* re-enable input */
    if (term->input_id==-1 && term->vx->vt.childfd !=-1) {
      term->input_id =
	zvt_input_add(term->vx->vt.childfd, READ_CONDITION, zvt_term_readdata, term);
    }

    if (vx->selectiontype & VT_SELTYPE_MOVED) {
      vt_fix_selection(vx);
      vt_draw_selection(vx);
          
      /* get the selection as 32 bit utf */
      vt_get_selection(vx, 4, 0);
      
      gtk_selection_owner_set (widget,
			       GDK_SELECTION_PRIMARY,
			       event->time);
    }

    vx->selectiontype = VT_SELTYPE_NONE; /* 'turn off' selecting */

  }

  return FALSE;
}

static void
zvt_term_scroll_by_lines (ZvtTerm *term, int n)
{
  GtkAdjustment *adj = term->adjustment;
  gfloat new_value = 0;

  if (n == 0)
    return;

  new_value = CLAMP (adj->value + n, adj->lower, adj->upper - adj->page_size);
  
  gtk_adjustment_set_value (term->adjustment, new_value);
}

static gint zvt_selectscroll(gpointer data)
{
  ZvtTerm *term;
  GtkWidget *widget;
  struct _zvtprivate *zp;

  widget = data;
  term = ZVT_TERM (widget);
  zp = _ZVT_PRIVATE(term);

  if (zp) {
    zvt_term_scroll_by_lines(term, zp->scrollselect_dir);
  }
  return TRUE;
}

static gint zvt_term_scroll_event (GtkWidget *widget, GdkEventScroll *event)
{
  ZvtTerm *term;
  
  g_return_val_if_fail (ZVT_IS_TERM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);
  
  term = ZVT_TERM (widget);
  
  if (event->direction == GDK_SCROLL_UP) {
    zvt_term_scroll_by_lines (term, -12);
    return TRUE;
  }
  
  if (event->direction == GDK_SCROLL_DOWN) {
    zvt_term_scroll_by_lines (term, 12);
    return TRUE;
  }
  
  return FALSE;
}

/*
  mouse motion notify.
  only gets called for the first motion?  why?
*/

static gint
zvt_term_motion_notify (GtkWidget      *widget,
			GdkEventMotion *event)
{
  struct _vtx *vx;
  gint x, y;
  ZvtTerm *term;
  struct _zvtprivate *zp;
  struct vt_match *m;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (ZVT_IS_TERM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  term = ZVT_TERM (widget);
  vx = term->vx;
  zp = _ZVT_PRIVATE(term);

  x=(((int)event->x))/term->charwidth;
  y=(((int)event->y))/term->charheight;

  if (vx->selectiontype != VT_SELTYPE_NONE){
    
    /* move end of selection, and draw it ... */
    if (vx->selectiontype & VT_SELTYPE_BYSTART) {
      vx->selendx = x;
      vx->selendy = y + vx->vt.scrollbackoffset;
    } else {
      vx->selstartx = x;
      vx->selstarty = y + vx->vt.scrollbackoffset;
    }

    vx->selectiontype |= VT_SELTYPE_MOVED;
    
    vt_fix_selection(vx);
    vt_draw_selection(vx);

    if (zp) {
      if (zp->scrollselect_id!=-1) {
	gtk_timeout_remove(zp->scrollselect_id);
	zp->scrollselect_id = -1;
      }
      
      if (y<0 || y>vx->vt.height) {
	if (y<0)
	  zp->scrollselect_dir = -1;
	else
	  zp->scrollselect_dir = 1;
	zp->scrollselect_id = gtk_timeout_add(100, zvt_selectscroll, term);
      }
    }
  } else {
    /* check for magic matches, if we havent already for this lot of output ... */
    if (term->vx->magic_matched==0)
      vt_getmatches(term->vx);

    /* check for actual matches? */
    m = vt_match_check(vx, x, y);
    vt_match_highlight(vx, m);
    if (m) {
      zvt_term_set_pointer(term, zp->cursor_hand);
    } else {
      zvt_term_set_pointer(term, term->cursor_bar);
    }
  }
  /* otherwise, just a mouse event */
  /* make sure the pointer is visible! */
  zvt_term_show_pointer(term);

  return FALSE;
}

/*
 * zvt_term_selection_clear: [internal]
 * @term: A &ZvtTerm widget.
 * @event: Event that triggered the selection claim.
 *
 * Called when another application claims the selection.
 */
gint
zvt_term_selection_clear (GtkWidget *widget, GdkEventSelection *event)
{
  struct _vtx *vx;
  ZvtTerm *term;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (ZVT_IS_TERM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  /* Let the selection handling code know that the selection
   * has been changed, since we've overriden the default handler */
  if (!gtk_selection_clear (widget, event))
    return FALSE;

  term = ZVT_TERM (widget);
  vx = term->vx;

  vtx_unrender_selection(vx);
  return TRUE;
}

#ifdef ZVT_UTF
/* perform data-conversion on the selection */
static char *
zvt_term_convert_selection(ZvtTerm *term, int type, int *outlen)
{
  char *out;
  int i;
  uint32 c;

  switch (type) {
  default:
  case 0: {			/* this is ascii/isolatin1 */
    unsigned char *o;
    d(printf("converting selection to ISOLATIN1\n"));
    out = g_malloc(term->vx->selection_size);
    o = out;
    for(i=0;i<term->vx->selection_size;i++) {
      c = term->vx->selection_data[i];
      o[i] = c<0x100?c:'?';
    }
    *outlen = term->vx->selection_size;
    break;
  }
  case 1: {			/* this is utf-8, basically a local implementation of wcstombs() */
    unsigned char *o;
    unsigned int len=0;
    d(printf("converting selection to UTF-8\n"));
    for(i=0;i<term->vx->selection_size;i++) {
      c = term->vx->selection_data[i];
      if (c<0x80)
	len+=1;
      else if (c<0x800)
	len+=2;
      else if (c<0x10000)
	len+=3;
      else if (c<0x200000)
	len+=4;
      else if (c<0x4000000)
	len+=5;
      else
	len+=6;
    }

    out = g_malloc(len);
    o = out;
    *outlen = len;

    for(i=0;i<term->vx->selection_size;i++) {
      c = term->vx->selection_data[i];
      if (c<0x80)
	*o++=c;
      else if (c<0x800) {
	*o++=(c>>6)|0xc0;
	*o++=(c&0x3f)|0x80;
      } else if (c<0x10000) {
	*o++=(c>>12)|0xe0;
	*o++=((c>>6)&0x3f)|0x80;
	*o++=(c&0x3f)|0x80;
      } else if (c<0x200000) {
	*o++=(c>>18)|0xf0;
	*o++=((c>>12)&0x3f)|0x80;
	*o++=((c>>6)&0x3f)|0x80;
	*o++=(c&0x3f)|0x80;
      } else if (c<0x4000000) {
	*o++=(c>>24)|0xf8;
	*o++=((c>>18)&0x3f)|0x80;
	*o++=((c>>12)&0x3f)|0x80;
	*o++=((c>>6)&0x3f)|0x80;
	*o++=(c&0x3f)|0x80;
      } else
	/* these wont happen */
	;
    }
    d(printf("len = %d, but length\n", len);
    {
      unsigned char *p = out;
      printf("in utf: ");
      while(p<o) {
	printf("%02x", *p++);
      }
      printf("\n");
    });
    break;
  }
  }

  return out;
}
#endif

/* supply the current selection to the caller */
static void
zvt_term_selection_get (GtkWidget        *widget, 
			GtkSelectionData *selection_data_ptr,
			guint             info,
			guint             time)
{
  struct _vtx *vx;
  ZvtTerm *term;
  char *converted;
  int len;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));
  g_return_if_fail (selection_data_ptr != NULL);

  term = ZVT_TERM (widget);
  vx = term->vx;

  converted = zvt_term_convert_selection (term, info, &len);
  gtk_selection_data_set_text (selection_data_ptr, converted, len);
  g_free(converted);
}

/**
 * zvt_term_writechild:
 * @term: A &ZvtTerm widget.
 * @data: Data to write.
 * @len: Length of data to write.
 *
 * Writes @len bytes of data, starting from @data to the subordinate
 * child process.  If the child is unable to handle all of the data
 * at once, then it will return, and asynchronously continue to feed
 * the data to the child.
 *
 * Return Value: The number of bytes written initially.
 */
int
zvt_term_writechild(ZvtTerm *term, char *data, int len)
{
  int length;
  struct _zvtprivate *zp;

  zp = _ZVT_PRIVATE(term);

  /* if we are already pasting something, make sure we append ... */
  if (zp->paste_id == -1)
    length = vt_writechild(&term->vx->vt, data, len);
  else
    length = 0;

  d(printf("wrote %d bytes of %d bytes\n", length, len));

  if (length>=0 && length < len) {
	  
    if (zp->paste_id == -1) {
      zp->paste = g_malloc(len-length);
      zp->paste_offset = 0;
      zp->paste_len = len-length;
      memcpy(zp->paste, data+length, len-length);
      zp->paste_id =
	zvt_input_add(term->vx->vt.keyfd, WRITE_CONDITION, zvt_term_writemore, term);
    } else {
      /* could also drop off offset, but its rarely likely to happen? */
      zp->paste = g_realloc(zp->paste, zp->paste_len + len - length);
      memcpy(zp->paste + zp->paste_len, data+length, len-length);
      zp->paste_len += (len - length);
    }
  }
  return length;
}

static gboolean
zvt_term_writemore (GIOChannel *source, GIOCondition condition, gpointer data)
{
  ZvtTerm *term = (ZvtTerm *)data;
  struct _zvtprivate *zp;
  int length;

  zp = _ZVT_PRIVATE(term);
  length = vt_writechild(&term->vx->vt, zp->paste + zp->paste_offset, zp->paste_len);

  d(printf("zvt_writemore(): written %d of %d bytes\n", length, zp->paste_len));

  if (length == -1 || length==zp->paste_len) {
    if (length == -1) {
      g_warning("Write failed to child process\n");
    }
    g_free(zp->paste);
    zp->paste = 0;
    g_source_remove(zp->paste_id);
    zp->paste_id = -1;
  } else {
    zp->paste_offset += length;
    zp->paste_len -= length;
  }
  return TRUE;
}


static gint
zvt_term_cursor_blink(gpointer data)
{
  ZvtTerm *term;
  GtkWidget *widget;

  widget = data;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (ZVT_IS_TERM (widget), FALSE);

  term = ZVT_TERM (widget);

  term->cursor_blink_state ^= 1;
  vt_cursor_state(data, term->cursor_blink_state);

  return TRUE;
}

/* called by everything when the screen display
   might have updated.
   if mode=1, then the data updated
   if mode=2, then the display updated (scrollbar moved)
*/
static void
zvt_term_updated(ZvtTerm *term, int mode)
{
  /* whenever we update, clear the match table and indicator */
  if (term->vx->magic_matched)
    vt_free_match_blocks(term->vx);
}

/*
 * Callback for when the adjustment changes - i.e., the scrollbar
 * moves.
 */
static void 
zvt_term_scrollbar_moved (GtkAdjustment *adj, GtkWidget *widget)
{
  int line;
  ZvtTerm *term;
  struct _vtx *vx;
  struct _zvtprivate *zp;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));

  term = ZVT_TERM (widget);
  vx = term->vx;
  zp = _ZVT_PRIVATE(term);

  line = term->vx->vt.scrollbacklines - (int)adj->value;

  /* needed for floating point errors in slider code */
  if (line < 0)
    line = 0;

  term->vx->vt.scrollbackoffset = -line;

  d(printf("zvt_term_scrollbar_moved adj->value=%f\n", adj->value));

  /* will redraw if scrollbar moved */
  vt_update (term->vx, UPDATE_SCROLLBACK);

  /* for scrolling selection */
  if (zp && zp->scrollselect_id != -1) {
    int x,y;

    if (zp->scrollselect_dir>0) {
      x = vx->vt.width-1;
      y = vx->vt.height-1;
    } else {
      x = 0;
      y = 0;
    }

    if (vx->selectiontype & VT_SELTYPE_BYSTART) {
      vx->selendx = x;
      vx->selendy = y + vx->vt.scrollbackoffset;
    } else {
      vx->selstartx = x;
      vx->selstarty = y + vx->vt.scrollbackoffset;
    }

    vt_fix_selection(vx);
    vt_draw_selection(vx);
  }

  zvt_term_updated(term, 2);
}


/**
 * zvt_term_forkpty:
 * @term: A &ZvtTerm widget.
 * @do_uwtmp_log: If %TRUE, then log the session in wtmp(4) and utmp(4).
 *
 * Fork a child process, with a master controlling terminal.
 */
int
zvt_term_forkpty (ZvtTerm *term, int do_uwtmp_log)
{
  int pid;

  g_return_val_if_fail (term != NULL, -1);
  g_return_val_if_fail (ZVT_IS_TERM (term), -1);

  /* cannot fork twice! */
  if (term->input_id != -1)
    return -1;

  pid = vt_forkpty (&term->vx->vt, do_uwtmp_log);
  if (pid > 0) {
    term->input_id = 
      zvt_input_add(term->vx->vt.childfd, READ_CONDITION, zvt_term_readdata, term);
    term->msg_id =
      zvt_input_add(term->vx->vt.msgfd, READ_CONDITION, zvt_term_readmsg, term);
  }

  return pid;
}

/**
 * zvt_term_killchild:
 * @term: A &ZvtTerm widget.
 * @signal: A signal number.
 *
 * Send the signal @signal to the child process.  Note that a child
 * process must have first been started using zvt_term_forkpty().
 *
 * Return Value: See kill(2).
 * See Also: signal(5).
 */
int
zvt_term_killchild (ZvtTerm *term, int signal)
{
  g_return_val_if_fail (term != NULL, -1);
  g_return_val_if_fail (ZVT_IS_TERM (term), -1);

  return vt_killchild (&term->vx->vt, signal);
}

/**
 * zvt_term_closepty:
 * @term: A &ZvtTerm widget.
 *
 * Close master pty to the child process.  It is upto the child to
 * recognise its pty has been closed, and to exit appropriately.
 *
 * Note that a child process must have first been started using
 * zvt_term_forkpty().
 *
 * Return Value: See close(2).
 */
int
zvt_term_closepty (ZvtTerm *term)
{
  g_return_val_if_fail (term != NULL, -1);
  g_return_val_if_fail (ZVT_IS_TERM (term), -1);

  if (term->input_id != -1) {
    g_source_remove (term->input_id);
    term->input_id = -1;
  }

  if (term->msg_id != -1) {
    g_source_remove (term->msg_id);
    term->msg_id = -1;
  }

  return vt_closepty (&term->vx->vt);
}

static void
zvt_term_scroll (ZvtTerm *term, int n)
{
  gfloat new_value = 0;

  if (n) {
    new_value = term->adjustment->value + (n * term->adjustment->page_size);
  } else if (new_value == (term->adjustment->upper - term->adjustment->page_size)) {
    return;
  } else {
    new_value = term->adjustment->upper - term->adjustment->page_size;
  }

  gtk_adjustment_set_value (
      term->adjustment,
      n > 0 ? MIN(new_value, term->adjustment->upper- term->adjustment->page_size) :
      MAX(new_value, term->adjustment->lower));
}

/*
 * Keyboard input callback
 */

/* remapping table for function keys 5-20 */
static unsigned char f5_f20_remap[] =
   {15,17,18,19,20,21,23,24,25,26,28,29,31,32,33,34};

static void
append_erase (ZvtEraseBinding binding,
              char          **pp)
{
  switch (binding)
    {
    case ZVT_ERASE_CONTROL_H:
      g_assert (('H' - 64) == 8);
      *(*pp)++ = 8;
      break;
      
    case ZVT_ERASE_ESCAPE_SEQUENCE:
      (*pp) += sprintf ((*pp), "\033[3~");
      break;
      
    case ZVT_ERASE_ASCII_DEL:
      *(*pp)++ = '\177';
      break;
    }
}

static gint
zvt_term_key_press (GtkWidget *widget, GdkEventKey *event)
{
  char buffer[64];
  char *p=buffer;
  struct _vtx *vx;
  ZvtTerm *term;
  int handled;
  char *cursor;
  int appl_keypad;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (ZVT_IS_TERM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  term = ZVT_TERM (widget);
  vx = term->vx;

  appl_keypad = (vx->vt.mode & VTMODE_APP_KEYPAD) != 0;

  zvt_term_hide_pointer(term);
  
  d(printf("keyval = %04x state = %x\n", event->keyval, event->state));
  handled = TRUE;

  if(!gtk_bindings_activate(GTK_OBJECT(widget),
                            event->keyval,
                            event->state)){
  
  switch (event->keyval) {
  case GDK_BackSpace:
    if (term->backspace_binding == ZVT_ERASE_CONTROL_H ||
        term->backspace_binding == ZVT_ERASE_ASCII_DEL)
      {
        if (event->state & GDK_MOD1_MASK)
          *p++ = '\033';
      }
    append_erase (term->backspace_binding, &p);
    break;
  case GDK_KP_Right:
  case GDK_Right:
    cursor ="C";
    goto do_cursor;
  case GDK_KP_Left:
  case GDK_Left:
    cursor = "D";
    goto do_cursor;
  case GDK_KP_Up:
  case GDK_Up:
    cursor = "A";
    goto do_cursor;
  case GDK_KP_Down:
  case GDK_Down:
    cursor = "B";
    do_cursor:
    if (vx->vt.mode & VTMODE_APP_CURSOR)
      p+=sprintf (p, "\033O%s", cursor);
    else
      p+=sprintf (p, "\033[%s", cursor);
    break;
  case GDK_KP_Insert:
  case GDK_Insert:
    if ((event->state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK){
      zvt_term_paste (ZVT_TERM (widget), GDK_SELECTION_PRIMARY);
    } else {
      p+=sprintf (p, "\033[2~");
    }
    break;
  case GDK_Delete:
    if (term->delete_binding == ZVT_ERASE_CONTROL_H ||
        term->delete_binding == ZVT_ERASE_ASCII_DEL)
      {
        if (event->state & GDK_MOD1_MASK)
          *p++ = '\033';
      }
    append_erase (term->delete_binding, &p);
    break;
  case GDK_KP_Delete:
    p+=sprintf (p, "\033[3~");
    break;
  case GDK_KP_Home:
  case GDK_Home:
    p+=sprintf (p, "\033OH");
    break;
  case GDK_KP_End:
  case GDK_End:
    p+=sprintf (p, "\033OF");
    break;
  case GDK_KP_Page_Up:
  case GDK_Page_Up:
    if (event->state & GDK_SHIFT_MASK){
      zvt_term_scroll (term, -1);
    } else
      p+=sprintf (p, "\033[5~");
    break;
  case GDK_KP_Page_Down:
  case GDK_Page_Down:
    if (event->state & GDK_SHIFT_MASK){
      zvt_term_scroll (term, 1);
    } else
      p+=sprintf (p, "\033[6~");
    break;

  case GDK_KP_F1:
  case GDK_F1:
    p+=sprintf (p, "\033OP");
    break;
  case GDK_KP_F2:
  case GDK_F2:
    p+=sprintf (p, "\033OQ");
    break;
  case GDK_KP_F3:
  case GDK_F3:
    p+=sprintf (p, "\033OR");
    break;
  case GDK_KP_F4:
  case GDK_F4:
    p+=sprintf (p, "\033OS");
    break;
  case GDK_F5:  case GDK_F6:  case GDK_F7:  case GDK_F8:
  case GDK_F9:  case GDK_F10:  case GDK_F11:  case GDK_F12:
  case GDK_F13:  case GDK_F14:  case GDK_F15:  case GDK_F16:
  case GDK_F17:  case GDK_F18:  case GDK_F19:  case GDK_F20:
    p+=sprintf (p, "\033[%d~", f5_f20_remap[event->keyval-GDK_F5]);
    break;

  case GDK_KP_0:  case GDK_KP_1:  case GDK_KP_2:  case GDK_KP_3:
  case GDK_KP_4:  case GDK_KP_5:  case GDK_KP_6:  case GDK_KP_7:
  case GDK_KP_8:  case GDK_KP_9:
    if (appl_keypad) {
      p+=sprintf (p, "\033O%c", 'p' + (event->keyval - GDK_KP_0));
    } else {
      *p++ = '0' + (event->keyval - GDK_KP_0);
    }
    break;
  case GDK_KP_Enter:
    if (appl_keypad) {
      p+=sprintf (p, "\033OM");
    } else {
      *p++ = '\r';
    }
    break;
  case GDK_KP_Add:
    if (appl_keypad) {
      p+=sprintf (p, "\033Ok");
    } else {
      *p++ = '+';
    }
    break;
  case GDK_KP_Subtract:
    if (appl_keypad) {
      p+=sprintf (p, "\033Om");
    } else {
      *p++ = '-';
    }
    break;
  case GDK_KP_Separator:  /* aka KP Comma */
    if (appl_keypad) {
      p+=sprintf (p, "\033Ol");
    } else {
      *p++ = '-';
    }
    break;
  case GDK_KP_Decimal:
    if (appl_keypad) {
      p+=sprintf (p, "\033On");
    } else {
      *p++ = '.';
    }
    break;

  case GDK_KP_Begin:
    /* ? middle key of keypad */
  case GDK_Print:
  case GDK_Scroll_Lock:
  case GDK_Pause:
    /* control keys */
  case GDK_Shift_Lock:
  case GDK_Num_Lock:
  case GDK_Caps_Lock:
    /* ignore - for now FIXME: do something here*/
    break;
  case GDK_Control_L:
  case GDK_Control_R:
    break;
  case GDK_Shift_L:
  case GDK_Shift_R:
    break;
  case GDK_Alt_L:
  case GDK_Alt_R:
  case GDK_Meta_L:
  case GDK_Meta_R:
    break;
  case GDK_Mode_switch:
  case GDK_Multi_key:
    break;
  case GDK_ISO_Left_Tab:
    *p++ = gdk_keyval_from_name("Tab");
    break;
  case GDK_Tab:
    *p++ = event->keyval;
    break;
  case GDK_Menu:
    p+=sprintf (p, "\033[29~");
    break;
  case ' ':
    /* maps single characters to correct control and alt versions */
    if (event->state & GDK_CONTROL_MASK)
      *p++=event->keyval & 0x1f;
    else if (event->state & GDK_MOD1_MASK)
      *p++=event->keyval + 0x80; /* this works for space at least */
    else
      *p++=event->keyval;
    break;
  default:
      if (event->length > 0){
	if (event->state & (GDK_MOD1_MASK | GDK_MOD4_MASK)){
	   *p++ = '\033';
        }
	memcpy(p, event->string, event->length*sizeof(char));
	p += event->length;
      } else {
	handled = FALSE;
      }
      d(printf ("[%s,%d,%d]\n", event->string, event->length, handled));
  }
  if (handled && p>buffer) {
    vt_writechild(&vx->vt, buffer, (p-buffer));
    if (term->scroll_on_keystroke) zvt_term_scroll (term, 0);
  }
  }

  return handled;
}

/* dummy default signal handler */
static void
zvt_term_child_died (ZvtTerm *term)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));

  /* perhaps we should do something here? */
}

/* dummy default signal handler for title_changed */
static void
zvt_term_title_changed (ZvtTerm *term, VTTITLE_TYPE type, const char *str)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));

  /* perhaps we should do something here? */
}

static void
zvt_term_real_copy_clipboard (ZvtTerm *term)
{
  char *text;
  int len;

  if (term->vx->selection_size == 0)
    return;

  text = zvt_term_convert_selection (term, 1, &len);

  gtk_clipboard_set_text (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD), text, len);

  g_free (text);
}

static void
zvt_term_real_paste_clipboard (ZvtTerm *term)
{
  zvt_term_paste (term, GDK_NONE);
}

void
zvt_term_copy_clipboard (ZvtTerm *term)
{
  g_signal_emit (term, term_signals[COPY_CLIPBOARD], 0);
}

void
zvt_term_paste_clipboard (ZvtTerm *term)
{
  g_signal_emit (term, term_signals[PASTE_CLIPBOARD], 0);
}

/**
 * zvt_term_match_add:
 * @term: An initialised &ZvtTerm.
 * @regex: A regular expression to match.  It should be concise
 * enough so that it doesn't match whole lines.
 * @highlight_mask: Mask of bits used to set the attributes used
 * to highlight the match as the mouse moves over it.
 * @data: User data.
 * 
 * Add a new auto-match regular expression.  The
 * zvt_term_match_check() function can be used to check for matches
 * using screen coordinates.
 * 
 * Each regular expression @regex will be matched against each
 * line in the visible buffer.
 *
 * The @highlight_mask is taken from the VTATTR_* bits, as defined
 * in vt.h.  These include VTATTR_BOLD, VTATTR_UNDERLINE, etc.
 * Colours may also be set by including the colour index in the
 * appropriate bit position.  Colours and attributes may be combined.
 *
 * e.g. to set foreground colour 2, and background colour 5, use
 * highlight_mask = (2<<VTATTR_FORECOLOURB)|(5<<VTATTR_BACKCOLOURB).
 *
 * Return value: Returns -1 when the regular expression is invalid
 * and cannot be compiled (see regcomp(3c)).  Otherwise returns 0.
 **/
int
zvt_term_match_add(ZvtTerm *term, char *regex, uint32 highlight_mask, void *data)
{
  struct vt_magic_match *m;
  struct _vtx *vx = term->vx;

  m = g_malloc0(sizeof(*m));
  if (regcomp(&m->preg, regex, REG_EXTENDED)==0) {
    m->regex = g_strdup(regex);
    vt_list_addtail(&vx->magic_list, (struct vt_listnode *)m);
    m->user_data = data;
    m->highlight_mask = highlight_mask & VTATTR_MASK;
  } else {
    g_free(m);
    return -1;
  }
  return 0;
}


/**
 * zvt_term_match_clear:
 * @term: An initialised &ZvtTerm.
 * @regex: A regular expression to remove, or %NULL to remove
 * all match strings.
 * 
 * Remove a specific match string, or all match strings
 * from the terminal @term.
 **/
void
zvt_term_match_clear(ZvtTerm *term, char *regex)
{
  vt_match_clear(term->vx, regex);
}

/**
 * zvt_term_match_check:
 * @term: An initialised &ZvtTerm.
 * @x: X coordinate, in character coordinates.
 * @y: Y coordinate to check, in character coordinates.
 * @user_data_ptr: A pointer to a location to hold the user-data
 * associated with this match.  If NULL, then this is ignored.
 * 
 * Check for a match at a given character location.
 *
 * Return Values: Returns the string matched.  If @user_data_ptr is
 * non-NULL, then it is set to the user_data associated with this
 * match type.  The return value is only guaranteed valid until the next
 * iteration of the gtk main loop.
 **/
char *
zvt_term_match_check(ZvtTerm *term, int x, int y, void **user_data_ptr)
{
  struct vt_match *m;
  m = vt_match_check(term->vx, x, y);
  if (m) {
    if (user_data_ptr)
      *user_data_ptr = m->match->user_data;
    return m->matchstr;
  }
  return 0;
}

/* raise the title_changed signal */
static void
zvt_term_title_changed_raise (void *user_data, char *str, VTTITLE_TYPE type)
{
  ZvtTerm *term = user_data;
  
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));

  g_signal_emit(term, term_signals[TITLE_CHANGED], 0, type, str);
}

/* emit the got_output signal */
static void
zvt_term_got_output (ZvtTerm *term, const gchar *buffer, gint count)
{
    g_return_if_fail (term != NULL);
    g_return_if_fail (ZVT_IS_TERM (term));
    
    g_signal_emit(term, term_signals[GOT_OUTPUT], 0,
		    buffer, count);
}

static void
vtx_unrender_selection (struct _vtx *vx)
{
  /* need to 'un-render' selected area */
  if (vx->selected)
    {
      vx->selstartx = vx->selendx;
      vx->selstarty = vx->selendy;
      vt_draw_selection(vx);	/* un-render selection */
      vx->selected = 0;
    }
}

/*
 * this callback is called when data is ready on the child's file descriptor
 *
 * Read all data waiting on the file descriptor, updating the virtual
 * terminal buffer, until there is no more data to read, and then render it.
 *
 * NOTE: this may impact on slow machines, but it may not also ...!
 */
static gboolean
zvt_term_readdata (GIOChannel *source, GIOCondition condition, gpointer data)
{
  gboolean update;
  gchar buffer[4096];
  gint count, saveerrno;
  struct _vtx *vx;
  ZvtTerm *term;
  gint fd;

  d( printf("zvt_term_readdata\n") );

  fd = g_io_channel_unix_get_fd (source);
  term = (ZvtTerm *) data;

  if (term->input_id == -1)
    return FALSE;

  update = FALSE;
  vx = term->vx;
  vtx_unrender_selection (vx);
  saveerrno = EAGAIN;

  vt_cursor_state (term, 0);
  vt_match_highlight(term->vx, 0);
  while ( (saveerrno == EAGAIN) && (count = read (fd, buffer, 4096)) > 0)  {
    update = TRUE;
    saveerrno = errno;
    vt_parse_vt (&vx->vt, buffer, count);

    if (g_signal_has_handler_pending(G_OBJECT(term),
				   term_signals[GOT_OUTPUT], 0, TRUE))
	zvt_term_got_output(term, buffer, count);
  }

  if (update) {
    if (GTK_WIDGET_DRAWABLE (term)) {
      d( printf("zvt_term_readdata: update from read\n") );
      vt_update (vx, UPDATE_CHANGES);
    } else {
      d( printf("zvt_term_readdata: suspending update -- not drawable\n") );
    }
  } else {
    saveerrno = errno;
  }

  /* *always* turn the cursor back on */
  vt_cursor_state (term, 1);
  
  /* fix scroll bar */
  if (term->scroll_on_output) {
      zvt_term_scroll (term, 0);
  }

  /* flush all X events - this is really necessary to stop X queuing up
   * lots of screen updates and reducing interactivity on a busy terminal
   */
  gdk_flush ();

  /* read failed? oh well, that's life -- we handle dead children via
   * SIGCHLD
   */
  zvt_term_fix_scrollbar (term);

  zvt_term_updated(term, 2);
  return TRUE;
}

/**
 * zvt_term_feed:
 * @term: A &ZvtTerm widget.
 * @text: The text to feed.
 * @len:  The text length.
 *
 * This makes the terminal emulator process the stream of
 * characters in @text for @len bytes.  The text is interpreted
 * by the terminal emulator as if it were generated by a child
 * process.
 *
 * This is used by code that needs a terminal emulator, but
 * does not use a child process.
 */
void
zvt_term_feed (ZvtTerm *term, char *text, int len)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));
  g_return_if_fail (text != NULL);
  
  vt_cursor_state (term, 0);
  vt_match_highlight(term->vx, 0);
  vtx_unrender_selection (term->vx);
  vt_parse_vt (&term->vx->vt, text, len);
  vt_update (term->vx, UPDATE_CHANGES);
  
  /* *always* turn the cursor back on */
  vt_cursor_state (term, 1);
  
  /* fix scroll bar */
  if (term->scroll_on_output)
    zvt_term_scroll (term, 0);
  
  /* flush all X events - this is really necessary to stop X queuing up
   * lots of screen updates and reducing interactivity on a busy terminal
   */
  gdk_flush ();
  
  /* read failed? oh well, that's life -- we handle dead children via
   *  SIGCHLD
   */
  zvt_term_fix_scrollbar (term);

  zvt_term_updated(term, 1);
}

static gboolean
zvt_term_readmsg (GIOChannel *source, GIOCondition condition, gpointer data)
{
  ZvtTerm *term = (ZvtTerm *)data;

  /* I suppose I should bother reading the message from the fd, but
   * it doesn't seem worth the trouble <shrug>
   */
  if (term->input_id != -1) {
    g_source_remove (term->input_id);
    term->input_id=-1;
  }

  zvt_term_closepty (term);

  /* signal application FIXME: include error/non error code */
  g_signal_emit (term, term_signals[CHILD_DIED], 0);
  return TRUE;
}

/**
 * zvt_term_set_background:
 * @terminal: A &ZvtTerm widget.
 * @pixmap_file: file containing the pixmap image
 * @transparent: true if we want to run in transparent mode
 * @flags: A bitmask of background options:
 *   ZVT_BACKGROUND_SHADED, shade the transparency pixmap.
 *   ZVT_BACKGROUND_SCROLL, allow smart scrolling of the pixmap,
 *   ignored if transparency is requested.
 *
 * Sets the background of the @terminal.  If @pixmap_file and
 * @transparent are %NULL and %FALSE, then a standard filled background
 * is set.
 */
void
zvt_term_set_background (ZvtTerm *terminal, char *pixmap_file, 
			 int transparent, int flags)
{
  zvt_term_set_background_with_shading (terminal,
                                        pixmap_file,
                                        transparent,
                                        flags,
                                        0, 0, 0, 32768);
}

/**
 * zvt_term_set_background_with_shading:
 * @terminal: A &ZvtTerm widget.
 * @pixmap_file: file containing the pixmap image
 * @transparent: true if we want to run in transparent mode
 * @flags: A bitmask of background options:
 *   ZVT_BACKGROUND_SHADED, shade the transparency pixmap.
 *   ZVT_BACKGROUND_SCROLL, allow smart scrolling of the pixmap,
 *   ignored if transparency is requested.
 * @r: Red colour.
 * @g: Green colour.
 * @b: Blue colour.
 * @a: Colour intensity.
 *
 * Sets the background of the @terminal.  If @pixmap_file and
 * @transparent are %NULL and %FALSE, then a standard filled background
 * is set.
 *
 * Blends the background image of the terminal with the colour (@r,
 * @g, @b) with opacity @a.  A 0 @a results in no change, a full @a
 * results in a solid colour.
 *
 * The range of each value is 0-65535. 
 * 
 */
void
zvt_term_set_background_with_shading (ZvtTerm *terminal, char *pixmap_file, 
                                      int transparent, int flags,
                                      guint16 r,
                                      guint16 g,
                                      guint16 blue,
                                      guint16 a)
{
  struct zvt_background *b = 0;
  /* unused */
  /*
  struct _zvtprivate *zp = _ZVT_PRIVATE(terminal);
  */

#if 0
  if (!(zvt_term_get_capabilities (terminal) & ZVT_TERM_PIXMAP_SUPPORT))  
    return; 
#endif

  /* get base image */
  if (!transparent && pixmap_file) {
    b = zvt_term_background_new(terminal);  
    zvt_term_background_set_pixmap_file(b, pixmap_file);
  } else if (transparent) {
    b = zvt_term_background_new(terminal);  
    zvt_term_background_set_pixmap_atom(b, gdk_get_default_root_window(),
					gdk_atom_intern("_XROOTPMAP_ID", TRUE));
    zvt_term_background_set_translate(b, ZVT_BGTRANSLATE_ROOT, 0, 0);
  }

  /* set modifiers */
  if (b) {    
    if (flags & ZVT_BACKGROUND_SHADED)
      zvt_term_background_set_shade(b, r, g, blue, a);

    if (flags & ZVT_BACKGROUND_SCROLL)
      zvt_term_background_set_translate(b, ZVT_BGTRANSLATE_SCROLL, 0, 0);  
  }

  /* load the background */
  zvt_term_background_load(terminal, b);
  zvt_term_background_unref(b);

  /* FIXME: required to make gnome-terminal's prefs to work properly *sigh* */
  terminal->transparent = transparent;
  terminal->shaded = (flags & ZVT_BACKGROUND_SHADED) != 0;
  g_free (terminal->pixmap_filename);
  if (pixmap_file)
    terminal->pixmap_filename = g_strdup (pixmap_file);
  else
    terminal->pixmap_filename = NULL;
}

/*
 * callback rendering functions called by vt_update, etc
 */
static int
vt_cursor_state(void *user_data, int state)
{
  ZvtTerm *term;
  GtkWidget *widget;
  int old_state;

  widget = user_data;

  g_return_val_if_fail (widget != NULL, 0);
  g_return_val_if_fail (ZVT_IS_TERM (widget), 0);

  term = ZVT_TERM (widget);
  old_state = term->cursor_on;

  /* only call vt_draw_cursor if the state has changed */
  if (old_state ^ state) {
    if (GTK_WIDGET_DRAWABLE (widget))	{
      if(term->cursor_filled || !state)
        vt_draw_cursor(term->vx, state);
      else {
        vt_draw_cursor(term->vx, FALSE);
        if (term->vx->vt.scrollbackold == 0 &&
	    term->vx->vt.cursorx < term->vx->vt.width) {
	  int offx = widget->style->xthickness + PADDING;
	  int offy = widget->style->ythickness;
	  gdk_draw_rectangle (widget->window,
			      term->fore_gc, 0,
			      offx + term->vx->vt.cursorx *
			        term->charwidth + 1,
			      offy + term->vx->vt.cursory *
			        term->charheight + 1,
			      term->charwidth - 2,
			      term->charheight - 2);
	}
      }
      term->cursor_on = state;
    }
  }
  return old_state;
}

void
vt_draw_text(void *user_data, struct vt_line *line, int row, int col, int len, int attr)
{
  GdkFont *f;
  struct _vtx *vx;
  ZvtTerm *term;
  GtkWidget *widget;
  int fore, back, or;
  GdkColor pen;
  GdkGC *fgc, *bgc;
  GdkDrawable *real_drawable;
  gint real_offx, real_offy;
  int overstrike=0;
  int dofill=0;
  int offx, offy, x, y;
  int i;
  uint32 c;
  struct _zvtprivate *zp=0;

  
  widget = user_data;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));

  if (!GTK_WIDGET_DRAWABLE (widget))
    return;

  term = ZVT_TERM (widget);

  vx = term->vx;

  if (len+col>vx->vt.width)
    len = vx->vt.width-col;

  /* rendering offsetx */
  x = col * term->charwidth;
  y = row * term->charheight + term->font->ascent;
  offx = widget->style->xthickness + PADDING;
  offy = widget->style->ythickness;
  zp = _ZVT_PRIVATE(term);

  if (attr & VTATTR_BOLD)  {
    or = 8;
    f = term->font_bold;
    if (f==NULL) {
      f = term->font;
      overstrike = 1;
      if (zp && zp->bold_save) {
	gdk_draw_drawable(zp->bold_save, term->fore_gc, GTK_WIDGET(term)->window,
			x + offx + len*term->charwidth, offy + row*term->charheight,
			0, 0, 1, term->charheight);
      }
    }
  } else  {
    or = 0;
    f = term->font;
  }

  fore = (attr & VTATTR_FORECOLOURM) >> VTATTR_FORECOLOURB;
  back = (attr & VTATTR_BACKCOLOURM) >> VTATTR_BACKCOLOURB;

  if (fore < 8)
    fore |= or;

  /* set the right colour in the appropriate gc */
  fgc = term->fore_gc;
  bgc = term->back_gc;

  /* for reverse, swap colours's */
  if (attr & VTATTR_REVERSE) {
    int tmp;
    tmp = fore;fore=back;back=tmp;
  }

  if (term->back_last != back) {
    pen.pixel = term->colors [back].pixel;
    gdk_gc_set_background (fgc, &pen);
    term->back_last = back;
  }
  
  if (term->fore_last != fore) {
    pen.pixel = term->colors [fore].pixel;
    gdk_gc_set_foreground (fgc, &pen);
    term->fore_last = fore;
  }

  /* optimise: dont 'clear' background if not in 
   * expose, and background colour == window colour
   * this may look a bit weird, it really does need to 
   * be this way - so dont touch!
   *
   * if the terminal is transparent we must redraw 
   * all the time as we can't optimize in that case
   *
   * This needs a re-visit due to recent changes for pixmap
   * support.
   */
#if 0
  printf("f=%d:%d ", fore,back);
  printf("e= %d ", term->in_expose);
  printf("b= %d ", vx->back_match);
  printf("t= %d ", term->transparent);
#endif

#if 1

  if (term->in_expose || vx->back_match == 0) {
    /*    if ((term->transparent || term->pixmap_filename)*/
    if (zp->background
	&& back==17) {
      gdk_draw_rectangle (widget->window,
			  bgc, 1,
			  offx + col * term->charwidth,
			  offy + row * term->charheight,
			  len * term->charwidth,
			  term->charheight);
      d(printf("done fill "));
    } else {
      dofill=1;
      d(printf("do fill "));
    }
  }

#else

  if (term->in_expose || vx->back_match == 0) {
    if (zp->background
	&& ( term->in_expose==0 && !vx->back_match )) {
      /*	&& !term->in_expose && back >= 17) {*/
      gdk_draw_rectangle (widget->window,
			  bgc, 1,
			  offx + col * term->charwidth,
			  offy + row * term->charheight,
			  len * term->charwidth,
			  term->charheight);
      d(printf("draw pixmap\n"));
    } else if ( (term->in_expose && back < 17)
		|| ( term->in_expose==0 && !vx->back_match )) {
      dofill=1;
      /* if we can get away with it, use XDrawImageString() to do
	 the background fill! */
      d(printf("fill via image\n"));
    } else {
      d(printf("do nothing\n"));
    }
  } else {
    d(printf("not clearing background in_expose = %d, back_match=%d\n", 
	     term->in_expose, vx->back_match));
    d(printf("txt = '%.*s'\n", len, text));
  }
#endif

  /* make sure we have the space to expand the text */
  if (zp->text_expand==0 || zp->text_expandlen<len) {
    zp->text_expand = g_realloc(zp->text_expand, len*sizeof(uint32));
    zp->text_expandlen = len;
  }

  /* This funtion is provided so that we can get hokked up with the 
  real_drawable used when backing store is used, which seems to be the 
  case here, when using backing store the direct X calls needs protection
  with this. 
  */
  gdk_window_get_internal_paint_info(widget->window, &real_drawable, &real_offx,&real_offy);
  offx-=real_offx;
  offy-=real_offy;

  /* convert text in input into the format suitable for output ... */
  switch (zp->fonttype) {
  case ZVT_FONT_1BYTE: {		/* simple single-byte font */
    char *expand = zp->text_expand;
    XFontStruct *xfont;

    for(i=0;i<len;i++) {
      c=VT_ASCII(line->data[i+col]);
      if (c>=256)
	c='?';
      expand[i]=c&0xff;
    }

    /* render characters, with fill if we can */
    xfont = (XFontStruct *) GDK_FONT_XFONT(f);
    XSetFont(GDK_WINDOW_XDISPLAY(widget->window), GDK_GC_XGC(fgc), xfont->fid);
    if (dofill) {
      XDrawImageString(GDK_WINDOW_XDISPLAY(widget->window), GDK_WINDOW_XWINDOW(real_drawable),
		       GDK_GC_XGC(fgc), offx + x, offy + y, expand, len);
    } else {
      XDrawString(GDK_WINDOW_XDISPLAY(widget->window), GDK_WINDOW_XWINDOW(real_drawable),
		  GDK_GC_XGC(fgc), offx + x, offy + y, expand, len);
    }
    if (overstrike)
      XDrawString(GDK_WINDOW_XDISPLAY(widget->window), GDK_WINDOW_XWINDOW(real_drawable),
		  GDK_GC_XGC(fgc), offx + x + 1, offy + y, expand, len);
  }
  break;
  case ZVT_FONT_2BYTE: {
    XChar2b *expand16 = zp->text_expand;
    XFontStruct *xfont;

    /* this needs to check for valid chars? */
    for (i=0;i<len;i++) {
      c=VT_ASCII(line->data[i+col]);
      expand16[i].byte2=c&0xff;
      expand16[i].byte1=(c>>8)&0xff;
      /*printf("(%04x)", c);*/
    }
    /*    printf("\n");*/

    /* render 2-byte characters, with fill if we can */
    xfont = (XFontStruct *) GDK_FONT_XFONT(f);
    XSetFont(GDK_WINDOW_XDISPLAY(widget->window), GDK_GC_XGC(fgc), xfont->fid);
    if (dofill) {
      XDrawImageString16(GDK_WINDOW_XDISPLAY(widget->window), GDK_WINDOW_XWINDOW(real_drawable),
			 GDK_GC_XGC(fgc), offx + x, offy + y, expand16, len);
    } else {
      XDrawString16(GDK_WINDOW_XDISPLAY(widget->window), GDK_WINDOW_XWINDOW(real_drawable),
		    GDK_GC_XGC(fgc), offx + x, offy + y, expand16, len);
    }
    if (overstrike)
      XDrawString16(GDK_WINDOW_XDISPLAY(widget->window), GDK_WINDOW_XWINDOW(real_drawable),
		    GDK_GC_XGC(fgc), offx + x + 1, offy + y, expand16, len);
  }
  break;
  /* this is limited to 65535 characters! */
  case ZVT_FONT_FONTSET: {
    wchar_t *expandwc = zp->text_expand;
    XFontSet fontset = (XFontSet) GDK_FONT_XFONT(f);

    for (i=0;i<len;i++) {
      expandwc[i] = VT_ASCII(line->data[i+col]);
    }

    /* render wide characters, with fill if we can */
    if (dofill) {
      XwcDrawImageString(GDK_WINDOW_XDISPLAY(widget->window), GDK_WINDOW_XWINDOW(real_drawable),
			 fontset, GDK_GC_XGC(fgc), offx + x, offy + y, expandwc, len);
    } else {
      XwcDrawString(GDK_WINDOW_XDISPLAY(widget->window), GDK_WINDOW_XWINDOW(real_drawable),
		    fontset, GDK_GC_XGC(fgc), offx + x, offy + y, expandwc, len);
    }
    if (overstrike)
      XwcDrawString(GDK_WINDOW_XDISPLAY(widget->window), GDK_WINDOW_XWINDOW(real_drawable),
		    fontset, GDK_GC_XGC(fgc), offx + x + 1, offy + y, expandwc, len);
  }
  }
  offx+=real_offx;
  offy+=real_offy;

  /* check for underline */
  if (attr&VTATTR_UNDERLINE) {
    gdk_draw_line(widget->window, fgc,
		  offx + x,
		  offy + y + 1,
		  offx + (col + len) * term->charwidth - 1,
		  offy + y + 1);
  }
  
  if (overstrike && zp && zp->bold_save) {
    gdk_draw_drawable(GTK_WIDGET(term)->window,
		    term->fore_gc,
		    zp->bold_save,
		    0, 0,
		    x + offx + len*term->charwidth, offy + row*term->charheight,
		    1, term->charheight);
  }
}





void
vt_scroll_area(void *user_data, int firstrow, int count, int offset, int fill)
{
  int width, offx, offy;
  ZvtTerm *term;
  GtkWidget *widget;
  GdkEvent *event;
  GdkGC *gc;
  struct _zvtprivate *zp;
  
  widget = user_data;
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (ZVT_IS_TERM (widget));

  if (!GTK_WIDGET_DRAWABLE (widget)) {
    return;
  }

  term = ZVT_TERM (widget);
  zp = _ZVT_PRIVATE(term);

  d(printf("scrolling %d rows from %d, by %d lines, fill=%d\n",
	  count,firstrow,offset,fill));

  width = term->charwidth * term->vx->vt.width;
  offx = widget->style->xthickness + PADDING;
  offy = widget->style->ythickness;

  /* "scroll" area */
  gdk_draw_drawable(widget->window,
		  term->scroll_gc, /* must use this to generate expose events */
		  widget->window,
		  offx, offy+(firstrow+offset)*term->charheight,
		  offx, offy+firstrow*term->charheight,
		  width, count*term->charheight);

  /* clear the other part of the screen */
  /* check fill colour is right */

  /* For correcting the reverse video when scrols */
#ifdef UNDEF
  if (fill >= 17) {
    gc=term->back_gc;
  } else {
#endif /* UNDEF */
    gc=term->fore_gc;
    if (term->fore_last != fill) {
      GdkColor pen;
      
      pen.pixel = term->colors[fill].pixel;
      gdk_gc_set_foreground (term->fore_gc, &pen);
      term->fore_last = fill;
    }
#ifdef UNDEF
  }
#endif /* UNDEF */

  /* this does a 'scrolling' pixmap */
  if (zp->background) {
    /* FIXME: scrolling with pixbuf */
    zp->scroll_position = (zp->scroll_position + offset*term->charheight);/* % term->background.h;*/
    gdk_gc_set_ts_origin (term->back_gc, 0, -zp->scroll_position);
  }

  /* fill the exposed area with blank */
  if (offset > 0) {
    gdk_draw_rectangle(widget->window,
		       gc,
		       1,
		       offx, offy+(firstrow+count)*term->charheight,
		       term->vx->vt.width*term->charwidth, offset*term->charheight);
  } else {
    gdk_draw_rectangle(widget->window,
		       gc,
		       1,
 		       offx, offy+(firstrow+offset)*term->charheight,
		       term->vx->vt.width*term->charwidth, (-offset)*term->charheight);
  }

  /* fix up the screen display after a scroll - ensures all expose events handled
     before continuing.
     This only seems to be needed if we have a pixmap scrolled */
  if (zp->background) {
    while ((event = gdk_event_get_graphics_expose (widget->window)) != NULL) {
      gtk_widget_event (widget, event);
      if (event->expose.count == 0) {
	gdk_event_free (event);
	break;
      }
      gdk_event_free (event);
    }
  }
}

static void
vt_selection_changed (void *user_data)
{
  g_signal_emit_by_name (user_data, "selection_changed");
}


static void
zvt_term_real_selection_changed (ZvtTerm *term)
{
  return;
}


/**
 * zvt_term_set_del_key_swap:
 * @term:   A &ZvtTerm widget.
 * @state:  If true it swaps the del/backspace definitions
 *          from the broken defaults to another broken setting
 * 
 * Sets the mode for interpreting the Delete and Backspace keys.  If
 * @state is %TRUE, equivalent to setting the delete binding to
 * #ZVT_ERASE_CONTROL_H and backspace to #ZVT_ERASE_ASCII_DEL.  If
 * %FALSE, the keys are set to swapped values. If
 * zvt_term_set_del_is_del() has been called to set the "del_is_del"
 * setting to %FALSE, Delete will always be #ZVT_ERASE_ESCAPE_SEQUENCE
 * rather than one of the ASCII characters.
 *
 * The correct defaults are #ZVT_ERASE_ASCII_DEL for the Backspace
 * key, and #ZVT_ERASE_ESCAPE_SEQUENCE for the Delete key. Any app
 * using ZvtTerm should set these, because ZvtTerm itself has
 * the wrong defaults for historical reasons. It's best to use
 * zvt_term_set_delete_binding() and zvt_term_set_backspace_binding()
 * instead of this function.
 **/
void
zvt_term_set_del_key_swap (ZvtTerm *term, int state)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));
  
  term->swap_del_key = state != 0;
  if (term->swap_del_key)
    {
      zvt_term_set_backspace_binding (term, ZVT_ERASE_ASCII_DEL);
      if (term->del_is_del)
        zvt_term_set_delete_binding (term, ZVT_ERASE_CONTROL_H);
      else
        zvt_term_set_delete_binding (term, ZVT_ERASE_ESCAPE_SEQUENCE);
    }
  else
    {
      zvt_term_set_backspace_binding (term, ZVT_ERASE_CONTROL_H);
      if (term->del_is_del)
        zvt_term_set_delete_binding (term, ZVT_ERASE_ASCII_DEL);
      else
        zvt_term_set_delete_binding (term, ZVT_ERASE_ESCAPE_SEQUENCE);
    }
}

/**
 * zvt_term_set_del_is_del:
 * @term:   A &ZvtTerm widget.
 * @state:  %FALSE if delete key sends an escape sequence
 * 
 * If @state is %TRUE, then the Delete key sends %ZVT_ERASE_ASCII_DEL if
 * delete and backspace aren't swapped, and %ZVT_ERASE_CONTROL_H if they
 * are swapped. If @state is %FALSE then the Delete key always sends
 * %ZVT_ERASE_ESCAPE_SEQUENCE.
 *
 * The correct defaults are #ZVT_ERASE_ASCII_DEL for the Backspace
 * key, and #ZVT_ERASE_ESCAPE_SEQUENCE for the Delete key. Any app
 * using ZvtTerm should set these, because ZvtTerm itself has
 * the wrong defaults for historical reasons. It's best to use
 * zvt_term_set_delete_binding() and zvt_term_set_backspace_binding()
 * instead of this function.
 * 
 **/
void
zvt_term_set_del_is_del (ZvtTerm *term, int state)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));
  
  term->del_is_del = state != 0;

  if (term->del_is_del)
    {
      if (term->swap_del_key)
        zvt_term_set_delete_binding (term, ZVT_ERASE_CONTROL_H);
      else
        zvt_term_set_delete_binding (term, ZVT_ERASE_ASCII_DEL);
    }
  else
    {
      zvt_term_set_delete_binding (term, ZVT_ERASE_ESCAPE_SEQUENCE);
    }
}

/*
 * zvt_term_bell:
 * @term: Terminal.
 *
 * Generate a terminal bell.  Currently this is just a beep.
 **/
void
zvt_term_bell(void *user_data)
{
  gdk_beep();
}


/**
 * zvt_term_set_bell:
 * @term: A &ZvtTerm widget.
 * @state: New bell state.
 * 
 * Enable or disable the terminal bell.  If @state is %TRUE, then the
 * bell is enabled.
 **/
void
zvt_term_set_bell(ZvtTerm *term, int state)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));

  if (state)
    term->vx->vt.ring_my_bell = zvt_term_bell;
  else
    term->vx->vt.ring_my_bell = 0;
}

/**
 * zvt_term_get_bell:
 * @term: A &ZvtTerm widget.
 * 
 * get the terminal bell state.  If the bell on then %TRUE is
 * returned, otherwise %FALSE.
 **/
gboolean
zvt_term_get_bell(ZvtTerm *term)
{
  g_return_val_if_fail (term != NULL, 0);
  g_return_val_if_fail (ZVT_IS_TERM (term), 0);

  return (term->vx->vt.ring_my_bell)?TRUE:FALSE;
}

void
zvt_term_set_shadow_type(ZvtTerm  *term, GtkShadowType type)
{
  g_return_if_fail (term != NULL);
  g_return_if_fail (ZVT_IS_TERM (term));

  term->shadow_type = type;

  if (GTK_WIDGET_VISIBLE (term))
    gtk_widget_queue_resize (GTK_WIDGET (term));
}

/**
 * zvt_term_get_capabilities:
 * @term: A &ZvtTerm widget.
 * 
 * Description: Gets the compiled in capabilities of the terminal widget.
 *
 * %ZVT_TERM_PIXMAP_SUPPORT; Pixmaps can be loaded into the background
 * using the background setting function.
 *
 * %ZVT_TERM_PIXMAPSCROLL_SUPPORT; The background scrolling flag of the
 * background setting function is honoured.
 *
 * %ZVT_TERM_EMBOLDEN_SUPPORT; Bold fonts are autogenerated, and can
 * be requested by setting the bold_font of the font setting function
 * to NULL.
 *
 * %ZVT_TERM_MATCH_SUPPORT; The zvt_term_add_match() functions exist,
 * and can be used to receive the match_clicked signal when the user
 * clicks on matching text.
 *
 * %ZVT_TERM_TRANSPARENCY_SUPPORT; A transparent background can be
 * requested on the current display.
 *
 * Returns: a bitmask of the capabilities
 **/
guint32
zvt_term_get_capabilities (ZvtTerm *term)
{
  GdkAtom prop, prop2;
  guint32 out = ZVT_TERM_EMBOLDEN_SUPPORT|ZVT_TERM_PIXMAPSCROLL_SUPPORT|
    ZVT_TERM_MATCH_SUPPORT|ZVT_TERM_PIXMAP_SUPPORT;

  /* check if we really have transparency support - i think this works MPZ */
  prop = gdk_atom_intern("_XROOTPMAP_ID", TRUE);
  prop2 = gdk_atom_intern("_XROOTCOLOR_PIXEL", TRUE);
  if (prop != GDK_NONE || prop2 != GDK_NONE)
    out |= ZVT_TERM_TRANSPARENCY_SUPPORT;

  return out;
}

/**
 * zvt_term_get_buffer:
 * @term: Valid &ZvtTerm widget.
 * @len: Placeholder to store the length of text selected.  May be
 *       %NULL in which case the value is not returned.
 * @type: Type of selection.  %VT_SELTYPE_LINE, select by line,
 *       %VT_SELTYPE_WORD, select by word, or %VT_SELTYPE_CHAR, select
 *       by character.
 * @sx: Start of selection, horizontal.
 * @sy: Start of selection, vertical.  0 is the top of the visible
 *      screen, <0 is scrollback lines, >0 is visible lines (upto the
 *      height of the window).
 * @ex: End of selection, horizontal.
 * @ey: End of selection, vertical, as above.
 * 
 * Convert the buffer memory into a contiguous array which may be
 * saved or processed.  Note that this is not gauranteed to match the
 * order of characters processed by the terminal, only the order in
 * which they were displayed.  Tabs will normally be preserved in
 * the output.
 *
 * All inputs are range-checked first, so it is possible to fudge
 * a full buffer grab.
 *
 * Examples:
 *  data = zvt_term_get_buffer(term, NULL, VT_SELTYPE_LINE,
 *       -term->vx->vt.scrollbackmax, 0,
 *        term->vx->vt.height, 0);
 *  or, as a rule -
 *  data = zvt_term_get_buffer(term, NULL, VT_SELTYPE_LINE,
 *       -10000, 0, 10000, 0);
 *
 * Will return the contents of the entire scrollback and on-screen
 * buffers, remembering that all inputs are range-checked first.
 *
 *  data = zvt_term_get_buffer(term, NULL, VT_SELTYPE_CHAR,
 *       0, 0, 5, 10);
 *
 * Will return the first 5 lines of the visible screen, and the 6th
 * line upto column 10.
 * 
 * Return value: A pointer to a %NUL terminated buffer containing the
 * raw text from the buffer.  If memory could not be allocated, then
 * returns %NULL.  Note that it is upto the caller to free the memory,
 * using g_free(3c).  If @len was supplied, then the length of data is
 * stored there.
 **/
char *
zvt_term_get_buffer(ZvtTerm *term, int *len, int type, int sx, int sy, int ex, int ey)
{
  struct _vtx *vx;
  int ssx, ssy, sex, sey, stype, slen;
  uint32 *sdata;
  char *data;

  g_return_val_if_fail (term != NULL, 0);
  g_return_val_if_fail (ZVT_IS_TERM (term), 0);

  vx = term->vx;

  /* this is a bit messy - we save the current selection state,
   * override it, 'select' the new text, then restore the old
   *  selection state but return the new ...
   *  api should be more separated.  vt_get_block() is a start on this.
   */

  ssx = vx->selstartx;
  ssy = vx->selstarty;
  sex = vx->selendx;
  sey = vx->selendy;
  sdata = vx->selection_data;
  slen = vx->selection_size;
  stype = vx->selectiontype;

  vx->selstartx = sx;
  vx->selstarty = sy;
  vx->selendx = ex;
  vx->selendy = ey;
  vx->selection_data = 0;
  vx->selectiontype = type & VT_SELTYPE_MASK;

  vt_fix_selection(vx);

  /* this always currently gets the data as bytes */
  data = vt_get_selection(vx, 1, len);
  
  vx->selstartx = ssx;
  vx->selstarty = ssy;
  vx->selendx = sex;
  vx->selendy = sey;
  vx->selection_data = sdata;
  vx->selection_size = slen;
  vx->selectiontype = stype;

  return data;
}


static AtkObject *
zvt_term_get_accessible (GtkWidget *widget)
{
  static gboolean first_time = TRUE;
  AtkRegistry *registry;
  AtkObjectFactory *factory;  
  GType derived_type;
  GType derived_atk_type;

  if (first_time)
    {
      /* Determine whether accessibility is enabled by checking accessible
	 created for parent */

      registry = atk_get_default_registry ();
      derived_type = g_type_parent (ZVT_TYPE_TERM);
      factory = atk_registry_get_factory (registry,
					 derived_type);
      derived_atk_type = atk_object_factory_get_accessible_type (factory);
      if (g_type_is_a(derived_atk_type, GTK_TYPE_ACCESSIBLE))
	{
	  atk_registry_set_factory_type (registry,
					 ZVT_TYPE_TERM, ZVT_TYPE_ACCESSIBLE_FACTORY);
	}
      first_time = FALSE;
    }
  return GTK_WIDGET_CLASS(parent_class)->get_accessible (widget);
}

static void
zvt_term_hierarchy_changed (GtkWidget *widget,
                            GtkWidget *previous_toplevel)
{
  zvt_term_update_toplevel_watch (ZVT_TERM (widget), FALSE);
}
