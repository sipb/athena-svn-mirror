/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

#include <math.h>
#include <string.h>

#include "eel-editable-label.h"
#include "eel-i18n.h"
#include "eel-marshal.h"

#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkeditable.h>
#include <gtk/gtkwindow.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkclipboard.h>
#include <gtk/gtkimmulticontext.h>
#include <pango/pango.h>
#include <gtk/gtkimagemenuitem.h>
#include <gtk/gtkseparatormenuitem.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkbindings.h>
#include <gtk/gtkgc.h>

enum {
  MOVE_CURSOR,
  POPULATE_POPUP,
  DELETE_FROM_CURSOR,
  CUT_CLIPBOARD,
  COPY_CLIPBOARD,
  PASTE_CLIPBOARD,
  TOGGLE_OVERWRITE,
  LAST_SIGNAL
};

enum {
  PROP_0,
  PROP_TEXT,
  PROP_JUSTIFY,
  PROP_WRAP,
  PROP_CURSOR_POSITION,
  PROP_SELECTION_BOUND
};

static guint signals[LAST_SIGNAL] = { 0 };

static void     eel_editable_label_editable_init           (GtkEditableClass      *iface);
static void     eel_editable_label_class_init              (EelEditableLabelClass *klass);
static void     eel_editable_label_init                    (EelEditableLabel      *label);
static void     eel_editable_label_set_property            (GObject               *object,
							    guint                  prop_id,
							    const GValue          *value,
							    GParamSpec            *pspec);
static void     eel_editable_label_get_property            (GObject               *object,
							    guint                  prop_id,
							    GValue                *value,
							    GParamSpec            *pspec);
static void     eel_editable_label_finalize                (GObject               *object);
static void     eel_editable_label_size_request            (GtkWidget             *widget,
							    GtkRequisition        *requisition);
static void     eel_editable_label_size_allocate           (GtkWidget             *widget,
							    GtkAllocation         *allocation);
static void     eel_editable_label_state_changed           (GtkWidget             *widget,
							    GtkStateType           state);
static void     eel_editable_label_style_set               (GtkWidget             *widget,
							    GtkStyle              *previous_style);
static void     eel_editable_label_direction_changed       (GtkWidget             *widget,
							    GtkTextDirection       previous_dir);
static gint     eel_editable_label_expose                  (GtkWidget             *widget,
							    GdkEventExpose        *event);
static void     eel_editable_label_realize                 (GtkWidget             *widget);
static void     eel_editable_label_unrealize               (GtkWidget             *widget);
static void     eel_editable_label_map                     (GtkWidget             *widget);
static void     eel_editable_label_unmap                   (GtkWidget             *widget);
static gint     eel_editable_label_button_press            (GtkWidget             *widget,
							    GdkEventButton        *event);
static gint     eel_editable_label_button_release          (GtkWidget             *widget,
							    GdkEventButton        *event);
static gint     eel_editable_label_motion                  (GtkWidget             *widget,
							    GdkEventMotion        *event);
static gint     eel_editable_label_key_press               (GtkWidget             *widget,
							    GdkEventKey           *event);
static gint     eel_editable_label_key_release             (GtkWidget             *widget,
							    GdkEventKey           *event);
static gint     eel_editable_label_focus_in                (GtkWidget             *widget,
							    GdkEventFocus         *event);
static gint     eel_editable_label_focus_out               (GtkWidget             *widget,
							    GdkEventFocus         *event);
static void     eel_editable_label_commit_cb               (GtkIMContext          *context,
							    const gchar           *str,
							    EelEditableLabel      *label);
static void     eel_editable_label_preedit_changed_cb      (GtkIMContext          *context,
							    EelEditableLabel      *label);
static gboolean eel_editable_label_retrieve_surrounding_cb (GtkIMContext          *context,
							    EelEditableLabel      *label);
static gboolean eel_editable_label_delete_surrounding_cb   (GtkIMContext          *slave,
							    gint                   offset,
							    gint                   n_chars,
							    EelEditableLabel      *label);
static void     eel_editable_label_clear_layout            (EelEditableLabel      *label);
static void     eel_editable_label_recompute               (EelEditableLabel      *label);
static void     eel_editable_label_ensure_layout           (EelEditableLabel      *label,
							    gboolean               include_preedit);
static void     eel_editable_label_select_region_index     (EelEditableLabel      *label,
							    gint                   anchor_index,
							    gint                   end_index);
static gboolean eel_editable_label_focus                   (GtkWidget             *widget,
							    GtkDirectionType       direction);
static void     eel_editable_label_move_cursor             (EelEditableLabel      *label,
							    GtkMovementStep        step,
							    gint                   count,
							    gboolean               extend_selection);
static void     eel_editable_label_delete_from_cursor      (EelEditableLabel      *label,
							    GtkDeleteType          type,
							    gint                   count);
static void     eel_editable_label_copy_clipboard          (EelEditableLabel      *label);
static void     eel_editable_label_cut_clipboard           (EelEditableLabel      *label);
static void     eel_editable_label_paste                   (EelEditableLabel      *label,
							    GdkAtom                selection);
static void     eel_editable_label_paste_clipboard         (EelEditableLabel      *label);
static void     eel_editable_label_select_all              (EelEditableLabel      *label);
static void     eel_editable_label_do_popup                (EelEditableLabel      *label,
							    GdkEventButton        *event);
static void     eel_editable_label_toggle_overwrite        (EelEditableLabel      *label);
static gint     eel_editable_label_move_forward_word       (EelEditableLabel      *label,
							    gint                   start);
static gint     eel_editable_label_move_backward_word      (EelEditableLabel      *label,
							    gint                   start);
static void     eel_editable_label_reset_im_context        (EelEditableLabel      *label);
static void     eel_editable_label_check_cursor_blink      (EelEditableLabel      *label);
static void     eel_editable_label_pend_cursor_blink       (EelEditableLabel      *label);

/* Editable implementation: */
static void     editable_insert_text_emit     (GtkEditable *editable,
					       const gchar *new_text,
					       gint         new_text_length,
					       gint        *position);
static void     editable_delete_text_emit     (GtkEditable *editable,
					       gint         start_pos,
					       gint         end_pos);
static void     editable_insert_text          (GtkEditable *editable,
					       const gchar *new_text,
					       gint         new_text_length,
					       gint        *position);
static void     editable_delete_text          (GtkEditable *editable,
					       gint         start_pos,
					       gint         end_pos);
static gchar *  editable_get_chars            (GtkEditable *editable,
					       gint         start_pos,
					       gint         end_pos);
static void     editable_set_selection_bounds (GtkEditable *editable,
					       gint         start,
					       gint         end);
static gboolean editable_get_selection_bounds (GtkEditable *editable,
					       gint        *start,
					       gint        *end);
static void     editable_real_set_position    (GtkEditable *editable,
					       gint         position);
static gint     editable_get_position         (GtkEditable *editable);

static GdkGC *  make_cursor_gc                (GtkWidget   *widget,
					       const gchar *property_name,
					       GdkColor    *fallback);



static GtkMiscClass *parent_class = NULL;

GType
eel_editable_label_get_type (void)
{
  static GType label_type = 0;
  
  if (!label_type)
    {
      static const GTypeInfo label_info =
      {
	sizeof (EelEditableLabelClass),
	NULL,           /* base_init */
	NULL,           /* base_finalize */
	(GClassInitFunc) eel_editable_label_class_init,
	NULL,           /* class_finalize */
	NULL,           /* class_data */
	sizeof (EelEditableLabel),
	32,             /* n_preallocs */
	(GInstanceInitFunc) eel_editable_label_init,
      };

      static const GInterfaceInfo editable_info =
      {
	(GInterfaceInitFunc) eel_editable_label_editable_init,	/* interface_init */
	NULL,							/* interface_finalize */
	NULL							/* interface_data */
      };

      
      label_type = g_type_register_static (GTK_TYPE_MISC, "EelEditableLabel", &label_info, 0);
      g_type_add_interface_static (label_type,
				   GTK_TYPE_EDITABLE,
				   &editable_info);
    }
  
  return label_type;
}

static void
add_move_binding (GtkBindingSet  *binding_set,
		  guint           keyval,
		  guint           modmask,
		  GtkMovementStep step,
		  gint            count)
{
  g_return_if_fail ((modmask & GDK_SHIFT_MASK) == 0);
  
  gtk_binding_entry_add_signal (binding_set, keyval, modmask,
				"move_cursor", 3,
				G_TYPE_ENUM, step,
				G_TYPE_INT, count,
                                G_TYPE_BOOLEAN, FALSE);

  /* Selection-extending version */
  gtk_binding_entry_add_signal (binding_set, keyval, modmask | GDK_SHIFT_MASK,
				"move_cursor", 3,
				G_TYPE_ENUM, step,
				G_TYPE_INT, count,
                                G_TYPE_BOOLEAN, TRUE);
}

static void
eel_editable_label_class_init (EelEditableLabelClass *class)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);
  GtkObjectClass *object_class = GTK_OBJECT_CLASS (class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);
  GtkBindingSet *binding_set;

  parent_class = gtk_type_class (GTK_TYPE_MISC);
  
  gobject_class->set_property = eel_editable_label_set_property;
  gobject_class->get_property = eel_editable_label_get_property;
  gobject_class->finalize = eel_editable_label_finalize;

  widget_class->size_request = eel_editable_label_size_request;
  widget_class->size_allocate = eel_editable_label_size_allocate;
  widget_class->state_changed = eel_editable_label_state_changed;
  widget_class->style_set = eel_editable_label_style_set;
  widget_class->direction_changed = eel_editable_label_direction_changed;
  widget_class->expose_event = eel_editable_label_expose;
  widget_class->realize = eel_editable_label_realize;
  widget_class->unrealize = eel_editable_label_unrealize;
  widget_class->map = eel_editable_label_map;
  widget_class->unmap = eel_editable_label_unmap;
  widget_class->button_press_event = eel_editable_label_button_press;
  widget_class->button_release_event = eel_editable_label_button_release;
  widget_class->motion_notify_event = eel_editable_label_motion;
  widget_class->focus = eel_editable_label_focus;
  widget_class->key_press_event = eel_editable_label_key_press;
  widget_class->key_release_event = eel_editable_label_key_release;
  widget_class->focus_in_event = eel_editable_label_focus_in;
  widget_class->focus_out_event = eel_editable_label_focus_out;

  class->move_cursor = eel_editable_label_move_cursor;
  class->delete_from_cursor = eel_editable_label_delete_from_cursor;
  class->copy_clipboard = eel_editable_label_copy_clipboard;
  class->cut_clipboard = eel_editable_label_cut_clipboard;
  class->paste_clipboard = eel_editable_label_paste_clipboard;
  class->toggle_overwrite = eel_editable_label_toggle_overwrite;
  
  signals[MOVE_CURSOR] = 
    g_signal_new ("move_cursor",
		  G_TYPE_FROM_CLASS (object_class),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (EelEditableLabelClass, move_cursor),
		  NULL, NULL,
		  eel_marshal_VOID__ENUM_INT_BOOLEAN,
		  G_TYPE_NONE, 3, GTK_TYPE_MOVEMENT_STEP, G_TYPE_INT, G_TYPE_BOOLEAN);
  
  signals[COPY_CLIPBOARD] =
    g_signal_new ("copy_clipboard",
		  G_TYPE_FROM_CLASS (object_class),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET  (EelEditableLabelClass, copy_clipboard),
		  NULL, NULL, 
		  eel_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
  
  signals[POPULATE_POPUP] =
    g_signal_new ("populate_popup",
		  G_TYPE_FROM_CLASS (object_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (EelEditableLabelClass, populate_popup),
		  NULL, NULL, 
		  eel_marshal_VOID__OBJECT,
		  G_TYPE_NONE, 1, GTK_TYPE_MENU);

  signals[DELETE_FROM_CURSOR] = 
    g_signal_new ("delete_from_cursor",
		  G_TYPE_FROM_CLASS (object_class),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (EelEditableLabelClass, delete_from_cursor),
		  NULL, NULL, 
		  eel_marshal_VOID__ENUM_INT,
		  G_TYPE_NONE, 2, GTK_TYPE_DELETE_TYPE, G_TYPE_INT);
  
  signals[CUT_CLIPBOARD] =
    g_signal_new ("cut_clipboard",
		  G_TYPE_FROM_CLASS (object_class),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (EelEditableLabelClass, cut_clipboard),
		  NULL, NULL, 
		  eel_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  signals[PASTE_CLIPBOARD] =
    g_signal_new ("paste_clipboard",
		  G_TYPE_FROM_CLASS (object_class),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (EelEditableLabelClass, paste_clipboard),
		  NULL, NULL, 
		  eel_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  signals[TOGGLE_OVERWRITE] =
    g_signal_new ("toggle_overwrite",
		  G_TYPE_FROM_CLASS (object_class),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (EelEditableLabelClass, toggle_overwrite),
		  NULL, NULL, 
		  eel_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  
  g_object_class_install_property (G_OBJECT_CLASS(object_class),
                                   PROP_TEXT,
                                   g_param_spec_string ("text",
                                                        _("Text"),
                                                        _("The text of the label."),
                                                        NULL,
                                                        G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class,
				   PROP_JUSTIFY,
                                   g_param_spec_enum ("justify",
                                                      _("Justification"),
                                                      _("The alignment of the lines in the text of the label relative to each other. This does NOT affect the alignment of the label within its allocation. See GtkMisc::xalign for that."),
						      GTK_TYPE_JUSTIFICATION,
						      GTK_JUSTIFY_LEFT,
                                                      G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_WRAP,
                                   g_param_spec_boolean ("wrap",
                                                        _("Line wrap"),
                                                        _("If set, wrap lines if the text becomes too wide."),
                                                        FALSE,
                                                        G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_CURSOR_POSITION,
                                   g_param_spec_int ("cursor_position",
                                                     _("Cursor Position"),
                                                     _("The current position of the insertion cursor in chars."),
                                                     0,
                                                     G_MAXINT,
                                                     0,
                                                     G_PARAM_READABLE));
  
  g_object_class_install_property (gobject_class,
                                   PROP_SELECTION_BOUND,
                                   g_param_spec_int ("selection_bound",
                                                     _("Selection Bound"),
                                                     _("The position of the opposite end of the selection from the cursor in chars."),
                                                     0,
                                                     G_MAXINT,
                                                     0,
                                                     G_PARAM_READABLE));
  
  /*
   * Key bindings
   */

  binding_set = gtk_binding_set_by_class (class);

  /* Moving the insertion point */
  add_move_binding (binding_set, GDK_Right, 0,
		    GTK_MOVEMENT_VISUAL_POSITIONS, 1);
  
  add_move_binding (binding_set, GDK_Left, 0,
		    GTK_MOVEMENT_VISUAL_POSITIONS, -1);

  add_move_binding (binding_set, GDK_KP_Right, 0,
		    GTK_MOVEMENT_VISUAL_POSITIONS, 1);
  
  add_move_binding (binding_set, GDK_KP_Left, 0,
		    GTK_MOVEMENT_VISUAL_POSITIONS, -1);
  
  add_move_binding (binding_set, GDK_f, GDK_CONTROL_MASK,
		    GTK_MOVEMENT_LOGICAL_POSITIONS, 1);
  
  add_move_binding (binding_set, GDK_b, GDK_CONTROL_MASK,
		    GTK_MOVEMENT_LOGICAL_POSITIONS, -1);
  
  add_move_binding (binding_set, GDK_Right, GDK_CONTROL_MASK,
		    GTK_MOVEMENT_WORDS, 1);

  add_move_binding (binding_set, GDK_Left, GDK_CONTROL_MASK,
		    GTK_MOVEMENT_WORDS, -1);

  add_move_binding (binding_set, GDK_KP_Right, GDK_CONTROL_MASK,
		    GTK_MOVEMENT_WORDS, 1);

  add_move_binding (binding_set, GDK_KP_Left, GDK_CONTROL_MASK,
		    GTK_MOVEMENT_WORDS, -1);
  
  add_move_binding (binding_set, GDK_a, GDK_CONTROL_MASK,
		    GTK_MOVEMENT_PARAGRAPH_ENDS, -1);

  add_move_binding (binding_set, GDK_e, GDK_CONTROL_MASK,
		    GTK_MOVEMENT_PARAGRAPH_ENDS, 1);

  add_move_binding (binding_set, GDK_f, GDK_MOD1_MASK,
		    GTK_MOVEMENT_WORDS, 1);

  add_move_binding (binding_set, GDK_b, GDK_MOD1_MASK,
		    GTK_MOVEMENT_WORDS, -1);

  add_move_binding (binding_set, GDK_Home, 0,
		    GTK_MOVEMENT_DISPLAY_LINE_ENDS, -1);

  add_move_binding (binding_set, GDK_End, 0,
		    GTK_MOVEMENT_DISPLAY_LINE_ENDS, 1);

  add_move_binding (binding_set, GDK_KP_Home, 0,
		    GTK_MOVEMENT_DISPLAY_LINE_ENDS, -1);

  add_move_binding (binding_set, GDK_KP_End, 0,
		    GTK_MOVEMENT_DISPLAY_LINE_ENDS, 1);
  
  add_move_binding (binding_set, GDK_Home, GDK_CONTROL_MASK,
		    GTK_MOVEMENT_BUFFER_ENDS, -1);

  add_move_binding (binding_set, GDK_End, GDK_CONTROL_MASK,
		    GTK_MOVEMENT_BUFFER_ENDS, 1);

  add_move_binding (binding_set, GDK_KP_Home, GDK_CONTROL_MASK,
		    GTK_MOVEMENT_BUFFER_ENDS, -1);

  add_move_binding (binding_set, GDK_KP_End, GDK_CONTROL_MASK,
		    GTK_MOVEMENT_BUFFER_ENDS, 1);

  add_move_binding (binding_set, GDK_Up, 0,
                    GTK_MOVEMENT_DISPLAY_LINES, -1);

  add_move_binding (binding_set, GDK_KP_Up, 0,
                    GTK_MOVEMENT_DISPLAY_LINES, -1);
  
  add_move_binding (binding_set, GDK_Down, 0,
                    GTK_MOVEMENT_DISPLAY_LINES, 1);

  add_move_binding (binding_set, GDK_KP_Down, 0,
                    GTK_MOVEMENT_DISPLAY_LINES, 1);
  
  /* Select all
   */
  gtk_binding_entry_add_signal (binding_set, GDK_a, GDK_CONTROL_MASK,
                                "move_cursor", 3,
                                GTK_TYPE_MOVEMENT_STEP, GTK_MOVEMENT_BUFFER_ENDS,
                                G_TYPE_INT, -1,
				G_TYPE_BOOLEAN, FALSE);
  gtk_binding_entry_add_signal (binding_set, GDK_a, GDK_CONTROL_MASK,
                                "move_cursor", 3,
                                GTK_TYPE_MOVEMENT_STEP, GTK_MOVEMENT_BUFFER_ENDS,
                                G_TYPE_INT, 1,
				G_TYPE_BOOLEAN, TRUE);
  
  /* Deleting text */
  gtk_binding_entry_add_signal (binding_set, GDK_Delete, 0,
				"delete_from_cursor", 2,
				G_TYPE_ENUM, GTK_DELETE_CHARS,
				G_TYPE_INT, 1);

  gtk_binding_entry_add_signal (binding_set, GDK_KP_Delete, 0,
				"delete_from_cursor", 2,
				G_TYPE_ENUM, GTK_DELETE_CHARS,
				G_TYPE_INT, 1);
  
  gtk_binding_entry_add_signal (binding_set, GDK_BackSpace, 0,
				"delete_from_cursor", 2,
				G_TYPE_ENUM, GTK_DELETE_CHARS,
				G_TYPE_INT, -1);

  /* Make this do the same as Backspace, to help with mis-typing */
  gtk_binding_entry_add_signal (binding_set, GDK_BackSpace, GDK_SHIFT_MASK,
                                "delete_from_cursor", 2,
                                G_TYPE_ENUM, GTK_DELETE_CHARS,
                                G_TYPE_INT, -1);

  gtk_binding_entry_add_signal (binding_set, GDK_Delete, GDK_CONTROL_MASK,
				"delete_from_cursor", 2,
				G_TYPE_ENUM, GTK_DELETE_WORD_ENDS,
				G_TYPE_INT, 1);

  gtk_binding_entry_add_signal (binding_set, GDK_KP_Delete, GDK_CONTROL_MASK,
				"delete_from_cursor", 2,
				G_TYPE_ENUM, GTK_DELETE_WORD_ENDS,
				G_TYPE_INT, 1);
  
  gtk_binding_entry_add_signal (binding_set, GDK_BackSpace, GDK_CONTROL_MASK,
				"delete_from_cursor", 2,
				G_TYPE_ENUM, GTK_DELETE_WORD_ENDS,
				G_TYPE_INT, -1);

  /* Cut/copy/paste */

  gtk_binding_entry_add_signal (binding_set, GDK_x, GDK_CONTROL_MASK,
				"cut_clipboard", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_c, GDK_CONTROL_MASK,
				"copy_clipboard", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_v, GDK_CONTROL_MASK,
				"paste_clipboard", 0);

  gtk_binding_entry_add_signal (binding_set, GDK_Delete, GDK_SHIFT_MASK,
				"cut_clipboard", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_Insert, GDK_CONTROL_MASK,
				"copy_clipboard", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_Insert, GDK_SHIFT_MASK,
				"paste_clipboard", 0);

  /* Overwrite */
  gtk_binding_entry_add_signal (binding_set, GDK_Insert, 0,
				"toggle_overwrite", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KP_Insert, 0,
				"toggle_overwrite", 0);
}

static void
eel_editable_label_editable_init (GtkEditableClass *iface)
{
  iface->do_insert_text = editable_insert_text_emit;
  iface->do_delete_text = editable_delete_text_emit;
  iface->insert_text = editable_insert_text;
  iface->delete_text = editable_delete_text;
  iface->get_chars = editable_get_chars;
  iface->set_selection_bounds = editable_set_selection_bounds;
  iface->get_selection_bounds = editable_get_selection_bounds;
  iface->set_position = editable_real_set_position;
  iface->get_position = editable_get_position;
}


static void 
eel_editable_label_set_property (GObject      *object,
				 guint         prop_id,
				 const GValue *value,
				 GParamSpec   *pspec)
{
  EelEditableLabel *label;

  label = EEL_EDITABLE_LABEL (object);
  
  switch (prop_id)
    {
    case PROP_TEXT:
      eel_editable_label_set_text (label, g_value_get_string (value));
      break;
    case PROP_JUSTIFY:
      eel_editable_label_set_justify (label, g_value_get_enum (value));
      break;
    case PROP_WRAP:
      eel_editable_label_set_line_wrap (label, g_value_get_boolean (value));
      break;	  
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void 
eel_editable_label_get_property (GObject     *object,
				 guint        prop_id,
				 GValue      *value,
				 GParamSpec  *pspec)
{
  EelEditableLabel *label;
  gint offset;
  
  label = EEL_EDITABLE_LABEL (object);
  
  switch (prop_id)
    {
    case PROP_TEXT:
      g_value_set_string (value, label->text);
      break;
    case PROP_JUSTIFY:
      g_value_set_enum (value, label->jtype);
      break;
    case PROP_WRAP:
      g_value_set_boolean (value, label->wrap);
      break;
    case PROP_CURSOR_POSITION:
      offset = g_utf8_pointer_to_offset (label->text,
					 label->text + label->selection_end);
      g_value_set_int (value, offset);
      break;
    case PROP_SELECTION_BOUND:
      offset = g_utf8_pointer_to_offset (label->text,
					 label->text + label->selection_anchor);
      g_value_set_int (value, offset);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
eel_editable_label_init (EelEditableLabel *label)
{
  label->jtype = GTK_JUSTIFY_LEFT;
  label->wrap = FALSE;
  label->wrap_mode = PANGO_WRAP_WORD;

  label->layout = NULL;
  label->text = NULL;
  label->text_size = 0;
  label->n_bytes = 0;
  
  GTK_WIDGET_SET_FLAGS (label, GTK_CAN_FOCUS);

  eel_editable_label_set_text (label, "");

    /* This object is completely private. No external entity can gain a reference
   * to it; so we create it here and destroy it in finalize().
   */
  label->im_context = gtk_im_multicontext_new ();

  g_signal_connect (G_OBJECT (label->im_context), "commit",
		    G_CALLBACK (eel_editable_label_commit_cb), label);
  g_signal_connect (G_OBJECT (label->im_context), "preedit_changed",
		    G_CALLBACK (eel_editable_label_preedit_changed_cb), label);
  g_signal_connect (G_OBJECT (label->im_context), "retrieve_surrounding",
		    G_CALLBACK (eel_editable_label_retrieve_surrounding_cb), label);
  g_signal_connect (G_OBJECT (label->im_context), "delete_surrounding",
		    G_CALLBACK (eel_editable_label_delete_surrounding_cb), label);
}

/**
 * eel_editable_label_new:
 * @str: The text of the label
 *
 * Creates a new label with the given text inside it. You can
 * pass %NULL to get an empty label widget.
 *
 * Return value: the new #EelEditableLabel
 **/
GtkWidget*
eel_editable_label_new (const gchar *str)
{
  EelEditableLabel *label;
  
  label = g_object_new (EEL_TYPE_EDITABLE_LABEL, NULL);

  if (str && *str)
    eel_editable_label_set_text (label, str);
  
  return GTK_WIDGET (label);
}

/**
 * eel_editable_label_set_text:
 * @label: a #EelEditableLabel
 * @str: The text you want to set.
 *
 * Sets the text within the #EelEditableLabel widget.  It overwrites any text that
 * was there before.  
 *
 * This will also clear any previously set mnemonic accelerators.
 **/
void
eel_editable_label_set_text (EelEditableLabel *label,
			     const gchar *str)
{
  int len;
  
  g_return_if_fail (EEL_IS_EDITABLE_LABEL (label));
  
  g_object_freeze_notify (G_OBJECT (label));

  len = str ? strlen(str) + 1 : 1;

  if (label->text == NULL || label->text_size < len)
    {
      label->text = g_realloc (label->text, len);
      label->text_size = len;
    }

  if (str)
    {
      strcpy (label->text, str);
      label->n_bytes = strlen (str);
    }
  else
    {
      label->text[0] = 0;
      label->n_bytes = 0;
    }
  g_object_notify (G_OBJECT (label), "text");

  if (label->selection_anchor > label->n_bytes)
    {
      g_object_notify (G_OBJECT (label), "cursor_position");
      g_object_notify (G_OBJECT (label), "selection_bound");
      label->selection_anchor = label->n_bytes;
    }
  
  if (label->selection_end > label->n_bytes)
    {
      label->selection_end = label->n_bytes;
      g_object_notify (G_OBJECT (label), "selection_bound");
    }
  
  eel_editable_label_recompute (label);  
  gtk_widget_queue_resize (GTK_WIDGET (label));

  g_object_thaw_notify (G_OBJECT (label));
}

/**
 * eel_editable_label_get_text:
 * @label: a #EelEditableLabel
 * 
 * Fetches the text from a label widget, as displayed on the
 * screen. This does not include any embedded underlines
 * indicating mnemonics or Pango markup. (See eel_editable_label_get_label())
 * 
 * Return value: the text in the label widget. This is the internal
 *   string used by the label, and must not be modified.
 **/
G_CONST_RETURN gchar *
eel_editable_label_get_text (EelEditableLabel *label)
{
  g_return_val_if_fail (EEL_IS_EDITABLE_LABEL (label), NULL);

  return label->text;
}

/**
 * eel_editable_label_set_justify:
 * @label: a #EelEditableLabel
 * @jtype: a #GtkJustification
 *
 * Sets the alignment of the lines in the text of the label relative to
 * each other.  %GTK_JUSTIFY_LEFT is the default value when the
 * widget is first created with eel_editable_label_new(). If you instead want
 * to set the alignment of the label as a whole, use
 * gtk_misc_set_alignment() instead. eel_editable_label_set_justify() has no
 * effect on labels containing only a single line.
 **/
void
eel_editable_label_set_justify (EelEditableLabel        *label,
				GtkJustification jtype)
{
  g_return_if_fail (EEL_IS_EDITABLE_LABEL (label));
  g_return_if_fail (jtype >= GTK_JUSTIFY_LEFT && jtype <= GTK_JUSTIFY_FILL);
  
  if ((GtkJustification) label->jtype != jtype)
    {
      label->jtype = jtype;

      /* No real need to be this drastic, but easier than duplicating the code */
      eel_editable_label_recompute (label);
      
      g_object_notify (G_OBJECT (label), "justify");
      gtk_widget_queue_resize (GTK_WIDGET (label));
    }
}

/**
 * eel_editable_label_get_justify:
 * @label: a #EelEditableLabel
 *
 * Returns the justification of the label. See eel_editable_label_set_justify ().
 *
 * Return value: #GtkJustification
 **/
GtkJustification
eel_editable_label_get_justify (EelEditableLabel *label)
{
  g_return_val_if_fail (EEL_IS_EDITABLE_LABEL (label), 0);

  return label->jtype;
}

void
eel_editable_label_set_draw_outline (EelEditableLabel *label,
				     gboolean          draw_outline)
{
    draw_outline = draw_outline != FALSE;

    if (label->draw_outline != draw_outline)
    {
      label->draw_outline = draw_outline;
      
      gtk_widget_queue_draw (GTK_WIDGET (label));
    }

}


/**
 * eel_editable_label_set_line_wrap:
 * @label: a #EelEditableLabel
 * @wrap: the setting
 *
 * Toggles line wrapping within the #EelEditableLabel widget.  %TRUE makes it break
 * lines if text exceeds the widget's size.  %FALSE lets the text get cut off
 * by the edge of the widget if it exceeds the widget size.
 **/
void
eel_editable_label_set_line_wrap (EelEditableLabel *label,
				  gboolean  wrap)
{
  g_return_if_fail (EEL_IS_EDITABLE_LABEL (label));
  
  wrap = wrap != FALSE;
  
  if (label->wrap != wrap)
    {
      label->wrap = wrap;
      g_object_notify (G_OBJECT (label), "wrap");
      
      gtk_widget_queue_resize (GTK_WIDGET (label));
    }
}


void
eel_editable_label_set_line_wrap_mode (EelEditableLabel *label,
				       PangoWrapMode     mode)
{
  g_return_if_fail (EEL_IS_EDITABLE_LABEL (label));
  
  if (label->wrap_mode != mode)
    {
      label->wrap_mode = mode;
      
      gtk_widget_queue_resize (GTK_WIDGET (label));
    }
  
}


/**
 * eel_editable_label_get_line_wrap:
 * @label: a #EelEditableLabel
 *
 * Returns whether lines in the label are automatically wrapped. See eel_editable_label_set_line_wrap ().
 *
 * Return value: %TRUE if the lines of the label are automatically wrapped.
 */
gboolean
eel_editable_label_get_line_wrap (EelEditableLabel *label)
{
  g_return_val_if_fail (EEL_IS_EDITABLE_LABEL (label), FALSE);

  return label->wrap;
}

static void
eel_editable_label_finalize (GObject *object)
{
  EelEditableLabel *label;
  
  g_return_if_fail (EEL_IS_EDITABLE_LABEL (object));
  
  label = EEL_EDITABLE_LABEL (object);

  g_object_unref (G_OBJECT (label->im_context));
  label->im_context = NULL;
  
  g_free (label->text);
  label->text = NULL;

  if (label->layout)
    {
      g_object_unref (G_OBJECT (label->layout));
      label->layout = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
eel_editable_label_clear_layout (EelEditableLabel *label)
{
  if (label->layout)
    {
      g_object_unref (G_OBJECT (label->layout));
      label->layout = NULL;
    }
}

static void
eel_editable_label_recompute (EelEditableLabel *label)
{
  eel_editable_label_clear_layout (label);
  eel_editable_label_check_cursor_blink (label);
}

typedef struct _LabelWrapWidth LabelWrapWidth;
struct _LabelWrapWidth
{
  gint width;
  PangoFontDescription *font_desc;
};

static void
label_wrap_width_free (gpointer data)
{
  LabelWrapWidth *wrap_width = data;
  pango_font_description_free (wrap_width->font_desc);
  g_free (wrap_width);
}

static gint
get_label_wrap_width (EelEditableLabel *label)
{
  PangoLayout *layout;
  GtkStyle *style = GTK_WIDGET (label)->style;

  LabelWrapWidth *wrap_width = g_object_get_data (G_OBJECT (style), "gtk-label-wrap-width");
  if (!wrap_width)
    {
      wrap_width = g_new0 (LabelWrapWidth, 1);
      g_object_set_data_full (G_OBJECT (style), "gtk-label-wrap-width",
			      wrap_width, label_wrap_width_free);
    }

  if (wrap_width->font_desc && pango_font_description_equal (wrap_width->font_desc, style->font_desc))
    return wrap_width->width;

  if (wrap_width->font_desc)
    pango_font_description_free (wrap_width->font_desc);

  wrap_width->font_desc = pango_font_description_copy (style->font_desc);

  layout = gtk_widget_create_pango_layout (GTK_WIDGET (label), 
					   "This long string gives a good enough length for any line to have.");
  pango_layout_get_size (layout, &wrap_width->width, NULL);
  g_object_unref (layout);

  return wrap_width->width;
}

static void
eel_editable_label_ensure_layout (EelEditableLabel *label,
				  gboolean        include_preedit)
{
  GtkWidget *widget;
  PangoRectangle logical_rect;
  gint rwidth, rheight;

  /* Normalize for comparisons */
  include_preedit = include_preedit != 0;

  if (label->preedit_length > 0 &&
      include_preedit != label->layout_includes_preedit)
    eel_editable_label_clear_layout (label);
  
  widget = GTK_WIDGET (label);

  rwidth = label->misc.xpad * 2;
  rheight = label->misc.ypad * 2;

  if (label->layout == NULL)
    {
      gchar *preedit_string = NULL;
      gint preedit_length = 0;
      PangoAttrList *preedit_attrs = NULL;
      PangoAlignment align = PANGO_ALIGN_LEFT; /* Quiet gcc */
      PangoAttrList *tmp_attrs = pango_attr_list_new ();

      if (include_preedit)
	{
	  gtk_im_context_get_preedit_string (label->im_context,
					     &preedit_string, &preedit_attrs, NULL);
	  preedit_length = label->preedit_length;
	}

      if (preedit_length)
	{
	  GString *tmp_string = g_string_new (NULL);
	  
	  g_string_prepend_len (tmp_string, label->text, label->n_bytes);
	  g_string_insert (tmp_string, label->selection_anchor, preedit_string);
      
	  label->layout = gtk_widget_create_pango_layout (widget, tmp_string->str);
      
	  pango_attr_list_splice (tmp_attrs, preedit_attrs,
				  label->selection_anchor, preedit_length);
	  
	  g_string_free (tmp_string, TRUE);
	}
      else
	{
	  label->layout = gtk_widget_create_pango_layout (widget, label->text);
	}
      label->layout_includes_preedit = include_preedit;
      
      pango_layout_set_attributes (label->layout, tmp_attrs);
      
      if (preedit_string)
	g_free (preedit_string);
      if (preedit_attrs)
	pango_attr_list_unref (preedit_attrs);
      pango_attr_list_unref (tmp_attrs);

      switch (label->jtype)
	{
	case GTK_JUSTIFY_LEFT:
	  align = PANGO_ALIGN_LEFT;
	  break;
	case GTK_JUSTIFY_RIGHT:
	  align = PANGO_ALIGN_RIGHT;
	  break;
	case GTK_JUSTIFY_CENTER:
	  align = PANGO_ALIGN_CENTER;
	  break;
	case GTK_JUSTIFY_FILL:
	  /* FIXME: This just doesn't work to do this */
	  align = PANGO_ALIGN_LEFT;
	  pango_layout_set_justify (label->layout, TRUE);
	  break;
	default:
	  g_assert_not_reached();
	}

      pango_layout_set_alignment (label->layout, align);

      if (label->wrap)
	{
	  gint longest_paragraph;
	  gint width, height;
	  gint set_width;

	  gtk_widget_get_size_request (widget, &set_width, NULL);
	  if (set_width > 0)
	    pango_layout_set_width (label->layout, set_width * PANGO_SCALE);
	  else
	    {
	      gint wrap_width;
	      
	      pango_layout_set_width (label->layout, -1);
	      pango_layout_get_extents (label->layout, NULL, &logical_rect);

	      width = logical_rect.width;
	      
	      /* Try to guess a reasonable maximum width */
	      longest_paragraph = width;

	      wrap_width = get_label_wrap_width (label);
	      width = MIN (width, wrap_width);
	      width = MIN (width,
			   PANGO_SCALE * (gdk_screen_width () + 1) / 2);
	      
	      pango_layout_set_width (label->layout, width);
	      pango_layout_get_extents (label->layout, NULL, &logical_rect);
	      width = logical_rect.width;
	      height = logical_rect.height;
	      
	      /* Unfortunately, the above may leave us with a very unbalanced looking paragraph,
	       * so we try short search for a narrower width that leaves us with the same height
	       */
	      if (longest_paragraph > 0)
		{
		  gint nlines, perfect_width;
		  
		  nlines = pango_layout_get_line_count (label->layout);
		  perfect_width = (longest_paragraph + nlines - 1) / nlines;
		  
		  if (perfect_width < width)
		    {
		      pango_layout_set_width (label->layout, perfect_width);
		      pango_layout_get_extents (label->layout, NULL, &logical_rect);
		      
		      if (logical_rect.height <= height)
			width = logical_rect.width;
		      else
			{
			  gint mid_width = (perfect_width + width) / 2;
			  
			  if (mid_width > perfect_width)
			    {
			      pango_layout_set_width (label->layout, mid_width);
			      pango_layout_get_extents (label->layout, NULL, &logical_rect);
			      
			      if (logical_rect.height <= height)
				width = logical_rect.width;
			    }
			}
		    }
		}
	      pango_layout_set_width (label->layout, width);
	    }
	  pango_layout_set_wrap (label->layout, label->wrap_mode);
	}
      else		/* !label->wrap */
	pango_layout_set_width (label->layout, -1);
    }
}

static void
eel_editable_label_size_request (GtkWidget      *widget,
				 GtkRequisition *requisition)
{
  EelEditableLabel *label;
  gint width, height;
  PangoRectangle logical_rect;
  gint set_width;

  g_return_if_fail (EEL_IS_EDITABLE_LABEL (widget));
  g_return_if_fail (requisition != NULL);
  
  label = EEL_EDITABLE_LABEL (widget);

  /*  
   * If word wrapping is on, then the height requisition can depend
   * on:
   *
   *   - Any width set on the widget via gtk_widget_set_usize().
   *   - The padding of the widget (xpad, set by gtk_misc_set_padding)
   *
   * Instead of trying to detect changes to these quantities, if we
   * are wrapping, we just rewrap for each size request. Since
   * size requisitions are cached by the GTK+ core, this is not
   * expensive.
   */

  if (label->wrap)
    eel_editable_label_recompute (label);

  eel_editable_label_ensure_layout (label, TRUE);

  width = label->misc.xpad * 2;
  height = label->misc.ypad * 2;

  pango_layout_get_extents (label->layout, NULL, &logical_rect);
  
  gtk_widget_get_size_request (widget, &set_width, NULL);
  if (label->wrap && set_width > 0)
    width += set_width;
  else 
    width += PANGO_PIXELS (logical_rect.width);
  
  height += PANGO_PIXELS (logical_rect.height);

  requisition->width = width;
  requisition->height = height;
}

static void
eel_editable_label_size_allocate (GtkWidget     *widget,
				  GtkAllocation *allocation)
{
  EelEditableLabel *label;

  label = EEL_EDITABLE_LABEL (widget);

  (* GTK_WIDGET_CLASS (parent_class)->size_allocate) (widget, allocation);
}

static void
eel_editable_label_state_changed (GtkWidget   *widget,
				  GtkStateType prev_state)
{
  EelEditableLabel *label;
  
  label = EEL_EDITABLE_LABEL (widget);

  eel_editable_label_select_region (label, 0, 0);

  if (GTK_WIDGET_CLASS (parent_class)->state_changed)
    GTK_WIDGET_CLASS (parent_class)->state_changed (widget, prev_state);
}

static void 
eel_editable_label_style_set (GtkWidget *widget,
			      GtkStyle  *previous_style)
{
  EelEditableLabel *label;
  
  g_return_if_fail (EEL_IS_EDITABLE_LABEL (widget));
  
  label = EEL_EDITABLE_LABEL (widget);

  /* We have to clear the layout, fonts etc. may have changed */
  eel_editable_label_recompute (label);
}

static void 
eel_editable_label_direction_changed (GtkWidget        *widget,
				      GtkTextDirection previous_dir)
{
  EelEditableLabel *label = EEL_EDITABLE_LABEL (widget);

  if (label->layout)
    pango_layout_context_changed (label->layout);

  GTK_WIDGET_CLASS (parent_class)->direction_changed (widget, previous_dir);
}

static void
get_layout_location (EelEditableLabel  *label,
                     gint      *xp,
                     gint      *yp)
{
  GtkMisc *misc;
  GtkWidget *widget;
  gfloat xalign;
  GtkRequisition req;
  gint x, y;
  
  misc = GTK_MISC (label);
  widget = GTK_WIDGET (label);
  
  if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_LTR)
    xalign = misc->xalign;
  else
    xalign = 1.0 - misc->xalign;

  gtk_widget_get_child_requisition (widget, &req);
  
  x = floor ((gint)misc->xpad
             + ((widget->allocation.width - req.width) * xalign)
             + 0.5);
  
  y = floor ((gint)misc->ypad 
             + ((widget->allocation.height - req.height) * misc->yalign)
             + 0.5);

  if (xp)
    *xp = x;

  if (yp)
    *yp = y;
}

static void
eel_editable_label_get_cursor_pos (EelEditableLabel  *label,
				   PangoRectangle *strong_pos,
				   PangoRectangle *weak_pos)
{
  const gchar *text;
  const gchar *preedit_text;
  gint index;
  
  eel_editable_label_ensure_layout (label, TRUE);
  
  text = pango_layout_get_text (label->layout);
  preedit_text = text + label->selection_anchor;
  index = label->selection_anchor +
    g_utf8_offset_to_pointer (preedit_text, label->preedit_cursor) - preedit_text;
      
  pango_layout_get_cursor_pos (label->layout, index, strong_pos, weak_pos);
}

/* These functions are copies from gtk+, as they are not exported from gtk+ */

static GdkGC *
make_cursor_gc (GtkWidget   *widget,
		const gchar *property_name,
		GdkColor    *fallback)
{
  GdkGCValues gc_values;
  GdkGCValuesMask gc_values_mask;
  GdkColor *cursor_color;

  gtk_widget_style_get (widget, property_name, &cursor_color, NULL);
  
  gc_values_mask = GDK_GC_FOREGROUND;
  if (cursor_color)
    {
      gc_values.foreground = *cursor_color;
      gdk_color_free (cursor_color);
    }
  else
    gc_values.foreground = *fallback;
  
  gdk_rgb_find_color (widget->style->colormap, &gc_values.foreground);
  return gtk_gc_get (widget->style->depth, widget->style->colormap, &gc_values, gc_values_mask);
}

static void
_eel_draw_insertion_cursor (GtkWidget        *widget,
			    GdkDrawable      *drawable,
			    GdkGC            *gc,
			    GdkRectangle     *location,
                            GtkTextDirection  direction,
                            gboolean          draw_arrow)
{
  gint stem_width;
  gint arrow_width;
  gint x, y;
  gint i;
  gfloat cursor_aspect_ratio;
  gint offset;
  
  g_return_if_fail (direction != GTK_TEXT_DIR_NONE);
  
  gtk_widget_style_get (widget, "cursor-aspect-ratio", &cursor_aspect_ratio, NULL);
  
  stem_width = location->height * cursor_aspect_ratio + 1;
  arrow_width = stem_width + 1;

  /* put (stem_width % 2) on the proper side of the cursor */
  if (direction == GTK_TEXT_DIR_LTR)
    offset = stem_width / 2;
  else
    offset = stem_width - stem_width / 2;
  
  for (i = 0; i < stem_width; i++)
    gdk_draw_line (drawable, gc,
		   location->x + i - offset, location->y,
		   location->x + i - offset, location->y + location->height - 1);

  if (draw_arrow)
    {
      if (direction == GTK_TEXT_DIR_RTL)
        {
          x = location->x - offset - 1;
          y = location->y + location->height - arrow_width * 2 - arrow_width + 1;
  
          for (i = 0; i < arrow_width; i++)
            {
              gdk_draw_line (drawable, gc,
                             x, y + i + 1,
                             x, y + 2 * arrow_width - i - 1);
              x --;
            }
        }
      else if (direction == GTK_TEXT_DIR_LTR)
        {
          x = location->x + stem_width - offset;
          y = location->y + location->height - arrow_width * 2 - arrow_width + 1;
  
          for (i = 0; i < arrow_width; i++) 
            {
              gdk_draw_line (drawable, gc,
                             x, y + i + 1,
                             x, y + 2 * arrow_width - i - 1);
              x++;
            }
        }
    }
}

static void
eel_editable_label_draw_cursor (EelEditableLabel  *label, gint xoffset, gint yoffset)
{
  if (GTK_WIDGET_DRAWABLE (label))
    {
      GtkWidget *widget = GTK_WIDGET (label);

      GtkTextDirection keymap_direction;
      GtkTextDirection widget_direction;
      PangoRectangle strong_pos, weak_pos;
      gboolean split_cursor;
      PangoRectangle *cursor1 = NULL;
      PangoRectangle *cursor2 = NULL;
      GdkRectangle cursor_location;
      GtkTextDirection dir1 = GTK_TEXT_DIR_NONE;
      GtkTextDirection dir2 = GTK_TEXT_DIR_NONE;

      keymap_direction =
	(gdk_keymap_get_direction (gdk_keymap_get_default ()) == PANGO_DIRECTION_LTR) ?
	GTK_TEXT_DIR_LTR : GTK_TEXT_DIR_RTL;

      widget_direction = gtk_widget_get_direction (widget);

      eel_editable_label_get_cursor_pos (label, &strong_pos, &weak_pos);

      g_object_get (gtk_widget_get_settings (widget),
		    "gtk-split-cursor", &split_cursor,
		    NULL);

      dir1 = widget_direction;
      
      if (split_cursor)
	{
	  cursor1 = &strong_pos;

	  if (strong_pos.x != weak_pos.x ||
	      strong_pos.y != weak_pos.y)
	    {
	      dir2 = (widget_direction == GTK_TEXT_DIR_LTR) ? GTK_TEXT_DIR_RTL : GTK_TEXT_DIR_LTR;
	      cursor2 = &weak_pos;
	    }
	}
      else
	{
	  if (keymap_direction == widget_direction)
	    cursor1 = &strong_pos;
	  else
	    cursor1 = &weak_pos;
	}
      
      cursor_location.x = xoffset + PANGO_PIXELS (cursor1->x);
      cursor_location.y = yoffset + PANGO_PIXELS (cursor1->y);
      cursor_location.width = 0;
      cursor_location.height = PANGO_PIXELS (cursor1->height);

      _eel_draw_insertion_cursor (widget, widget->window,
				  label->primary_cursor_gc,
				  &cursor_location, dir1,
                                  dir2 != GTK_TEXT_DIR_NONE);
      
      if (dir2 != GTK_TEXT_DIR_NONE)
	{
	  cursor_location.x = xoffset + PANGO_PIXELS (cursor2->x);
	  cursor_location.y = yoffset + PANGO_PIXELS (cursor2->y);
	  cursor_location.width = 0;
	  cursor_location.height = PANGO_PIXELS (cursor2->height);

	  _eel_draw_insertion_cursor (widget, widget->window,
				      label->secondary_cursor_gc,
				      &cursor_location, dir2, TRUE);
	}
    }
}


static gint
eel_editable_label_expose (GtkWidget      *widget,
			   GdkEventExpose *event)
{
  EelEditableLabel *label;
  gint x, y;
  
  g_return_val_if_fail (EEL_IS_EDITABLE_LABEL (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);
  
  label = EEL_EDITABLE_LABEL (widget);

  eel_editable_label_ensure_layout (label, TRUE);
  
  if (GTK_WIDGET_VISIBLE (widget) && GTK_WIDGET_MAPPED (widget) &&
      label->text && (*label->text != '\0'))
    {
      get_layout_location (label, &x, &y);
      
      gtk_paint_layout (widget->style,
                        widget->window,
                        GTK_WIDGET_STATE (widget),
			FALSE,
                        &event->area,
                        widget,
                        "label",
                        x, y,
                        label->layout);
      
      if (label->selection_anchor != label->selection_end)
        {
          gint range[2];
	  const char *text;
          GdkRegion *clip;
	  GtkStateType state;
	  
          range[0] = label->selection_anchor;
          range[1] = label->selection_end;

	  /* Handle possible preedit string */
	  if (label->preedit_length > 0 &&
	      range[1] > label->selection_anchor)
	    {
	      text = pango_layout_get_text (label->layout) + label->selection_anchor;
	      range[1] += g_utf8_offset_to_pointer (text, label->preedit_length) - text;
	    }
	  
          if (range[0] > range[1])
            {
              gint tmp = range[0];
              range[0] = range[1];
              range[1] = tmp;
            }

          clip = gdk_pango_layout_get_clip_region (label->layout,
                                                   x, y,
                                                   range,
                                                   1);

          /* FIXME should use gtk_paint, but it can't use a clip
           * region
           */

          gdk_gc_set_clip_region (widget->style->black_gc, clip);


	  state = GTK_STATE_SELECTED;
	  if (!GTK_WIDGET_HAS_FOCUS (widget))
	    state = GTK_STATE_ACTIVE;
	      
          gdk_draw_layout_with_colors (widget->window,
                                       widget->style->black_gc,
                                       x, y,
                                       label->layout,
                                       &widget->style->text[state],
                                       &widget->style->base[state]);

          gdk_gc_set_clip_region (widget->style->black_gc, NULL);
          gdk_region_destroy (clip);
        }
      else if (GTK_WIDGET_HAS_FOCUS (widget))
	eel_editable_label_draw_cursor (label, x, y);

      if (label->draw_outline)
	gdk_draw_rectangle (widget->window,
			    widget->style->black_gc,
			    FALSE,
			    0, 0,
			    widget->allocation.width - 1,
			    widget->allocation.height - 1);
    }

  return FALSE;
}

static void
eel_editable_label_realize (GtkWidget *widget)
{
  EelEditableLabel *label;
  GdkWindowAttr attributes;
  gint attributes_mask;
  static GdkColor gray = { 0, 0x8888, 0x8888, 0x8888 };

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
  label = EEL_EDITABLE_LABEL (widget);

  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);
  attributes.cursor = gdk_cursor_new (GDK_XTERM);
  attributes.event_mask = gtk_widget_get_events (widget) |
    (GDK_EXPOSURE_MASK |
     GDK_BUTTON_PRESS_MASK |
     GDK_BUTTON_RELEASE_MASK |
     GDK_BUTTON1_MOTION_MASK |
     GDK_BUTTON3_MOTION_MASK |
     GDK_POINTER_MOTION_HINT_MASK |
     GDK_POINTER_MOTION_MASK |
     GDK_ENTER_NOTIFY_MASK |
     GDK_LEAVE_NOTIFY_MASK);

  attributes_mask = GDK_WA_X | GDK_WA_Y  | GDK_WA_VISUAL | GDK_WA_COLORMAP | GDK_WA_CURSOR;

  widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
				   &attributes, attributes_mask);
  gdk_window_set_user_data (widget->window, widget);

  gdk_cursor_unref (attributes.cursor);

  widget->style = gtk_style_attach (widget->style, widget->window);
  
  gdk_window_set_background (widget->window, &widget->style->base[GTK_WIDGET_STATE (widget)]);

  gtk_im_context_set_client_window (label->im_context, widget->window);

  label->primary_cursor_gc = make_cursor_gc (widget,
					     "cursor-color",
					     &widget->style->black);
      
  label->secondary_cursor_gc = make_cursor_gc (widget,
					       "secondary-cursor-color",
					       &gray);
}

static void
eel_editable_label_unrealize (GtkWidget *widget)
{
  EelEditableLabel *label;

  label = EEL_EDITABLE_LABEL (widget);

  gtk_gc_release (label->primary_cursor_gc);
  label->primary_cursor_gc = NULL;
  
  gtk_gc_release (label->secondary_cursor_gc);
  label->secondary_cursor_gc = NULL;

  /* Strange. Copied from GtkEntry, should be NULL? */
  gtk_im_context_set_client_window (label->im_context, NULL);
  
  (* GTK_WIDGET_CLASS (parent_class)->unrealize) (widget);
}

static void
eel_editable_label_map (GtkWidget *widget)
{
  EelEditableLabel *label;

  label = EEL_EDITABLE_LABEL (widget);
  
  (* GTK_WIDGET_CLASS (parent_class)->map) (widget);
}

static void
eel_editable_label_unmap (GtkWidget *widget)
{
  EelEditableLabel *label;

  label = EEL_EDITABLE_LABEL (widget);

  (* GTK_WIDGET_CLASS (parent_class)->unmap) (widget);
}

static void
window_to_layout_coords (EelEditableLabel *label,
                         gint     *x,
                         gint     *y)
{
  gint lx, ly;
  GtkWidget *widget;

  widget = GTK_WIDGET (label);
  
  /* get layout location in widget->window coords */
  get_layout_location (label, &lx, &ly);
  
  if (x)
    *x -= lx;                   /* go to layout */

  if (y)
    *y -= ly;                   /* go to layout */
}

static void
get_layout_index (EelEditableLabel *label,
                  gint      x,
                  gint      y,
                  gint     *index)
{
  gint trailing = 0;
  const gchar *cluster;
  const gchar *cluster_end;

  *index = 0;
  
  eel_editable_label_ensure_layout (label, TRUE);
  
  window_to_layout_coords (label, &x, &y);

  x *= PANGO_SCALE;
  y *= PANGO_SCALE;
  
  pango_layout_xy_to_index (label->layout,
                            x, y,
                            index, &trailing);

  if (*index >= label->selection_anchor && label->preedit_length)
    {
      if (*index >= label->selection_anchor + label->preedit_length)
	*index -= label->preedit_length;
      else
	{
	  *index = label->selection_anchor;
	  trailing = 0;
	}
    }
  
  cluster = label->text + *index;
  cluster_end = cluster;
  while (trailing)
    {
      cluster_end = g_utf8_next_char (cluster_end);
      --trailing;
    }

  *index += (cluster_end - cluster);
}

static void
eel_editable_label_select_word (EelEditableLabel *label)
{
  gint min, max;
  
  gint start_index = eel_editable_label_move_backward_word (label, label->selection_end);
  gint end_index = eel_editable_label_move_forward_word (label, label->selection_end);

  min = MIN (label->selection_anchor,
	     label->selection_end);
  max = MAX (label->selection_anchor,
	     label->selection_end);

  min = MIN (min, start_index);
  max = MAX (max, end_index);

  eel_editable_label_select_region_index (label, min, max);
}

static gint
eel_editable_label_button_press (GtkWidget      *widget,
				 GdkEventButton *event)
{
  EelEditableLabel *label;
  gint index = 0;
  
  label = EEL_EDITABLE_LABEL (widget);

  if (event->button == 1)
    {
      if (!GTK_WIDGET_HAS_FOCUS (widget))
	gtk_widget_grab_focus (widget);

      if (event->type == GDK_3BUTTON_PRESS)
	{
	  eel_editable_label_select_region_index (label, 0, strlen (label->text));
	  return TRUE;
	}
      
      if (event->type == GDK_2BUTTON_PRESS)
	{
	  eel_editable_label_select_word (label);
	  return TRUE;
	}
      
      get_layout_index (label, event->x, event->y, &index);
      
      if ((label->selection_anchor !=
	   label->selection_end) &&
	  (event->state & GDK_SHIFT_MASK))
	{
	  gint min, max;
	  
	  /* extend (same as motion) */
	  min = MIN (label->selection_anchor,
		     label->selection_end);
	  max = MAX (label->selection_anchor,
		     label->selection_end);
	  
	  min = MIN (min, index);
	  max = MAX (max, index);
	  
	  /* ensure the anchor is opposite index */
	  if (index == min)
	    {
	      gint tmp = min;
	      min = max;
	      max = tmp;
	    }
	  
	  eel_editable_label_select_region_index (label, min, max);
	}
      else
	{
	  if (event->type == GDK_3BUTTON_PRESS)
	      eel_editable_label_select_region_index (label, 0, strlen (label->text));
	  else if (event->type == GDK_2BUTTON_PRESS)
	      eel_editable_label_select_word (label);
	  else 
	    /* start a replacement */
	    eel_editable_label_select_region_index (label, index, index);
	}
  
      return TRUE;
    }
  else if (event->button == 2 && event->type == GDK_BUTTON_PRESS)
    {
      get_layout_index (label, event->x, event->y, &index);
      
      eel_editable_label_select_region_index (label, index, index);
      eel_editable_label_paste (label, GDK_SELECTION_PRIMARY);
      
      return TRUE;
    }
  else if (event->button == 3 && event->type == GDK_BUTTON_PRESS)
    {
      eel_editable_label_do_popup (label, event);

      return TRUE;
      
    }
  return FALSE;
}

static gint
eel_editable_label_button_release (GtkWidget      *widget,
				   GdkEventButton *event)

{
  EelEditableLabel *label;

  label = EEL_EDITABLE_LABEL (widget);
  
  if (event->button != 1)
    return FALSE;
  
  /* The goal here is to return TRUE iff we ate the
   * button press to start selecting.
   */
  
  return TRUE;
}

static gint
eel_editable_label_motion (GtkWidget      *widget,
			   GdkEventMotion *event)
{
  EelEditableLabel *label;
  gint index;
  gint x, y;
  
  label = EEL_EDITABLE_LABEL (widget);
  
  if ((event->state & GDK_BUTTON1_MASK) == 0)
    return FALSE;

  gdk_window_get_pointer (widget->window,
                          &x, &y, NULL);
  
  get_layout_index (label, x, y, &index);

  eel_editable_label_select_region_index (label,
					  label->selection_anchor,
					  index);
  
  return TRUE;
}

static void
get_text_callback (GtkClipboard     *clipboard,
                   GtkSelectionData *selection_data,
                   guint             info,
                   gpointer          user_data_or_owner)
{
  EelEditableLabel *label;
  
  label = EEL_EDITABLE_LABEL (user_data_or_owner);
  
  if ((label->selection_anchor != label->selection_end) &&
      label->text)
    {
      gint start, end;
      gint len;
      
      start = MIN (label->selection_anchor,
                   label->selection_end);
      end = MAX (label->selection_anchor,
                 label->selection_end);

      len = strlen (label->text);

      if (end > len)
        end = len;

      if (start > len)
        start = len;

      gtk_selection_data_set_text (selection_data,
				   label->text + start,
				   end - start);
    }
}

static void
clear_text_callback (GtkClipboard     *clipboard,
                     gpointer          user_data_or_owner)
{
  EelEditableLabel *label;

  label = EEL_EDITABLE_LABEL (user_data_or_owner);

  label->selection_anchor = label->selection_end;
      
  gtk_widget_queue_draw (GTK_WIDGET (label));
}

static void
eel_editable_label_select_region_index (EelEditableLabel *label,
					gint      anchor_index,
					gint      end_index)
{
  static const GtkTargetEntry targets[] = {
    { "STRING", 0, 0 },
    { "TEXT",   0, 0 }, 
    { "COMPOUND_TEXT", 0, 0 },
    { "UTF8_STRING", 0, 0 }
  };
  GtkClipboard *clipboard;

  g_return_if_fail (EEL_IS_EDITABLE_LABEL (label));
  

  if (label->selection_anchor == anchor_index &&
      label->selection_end == end_index)
    return;

  eel_editable_label_reset_im_context (label);

  label->selection_anchor = anchor_index;
  label->selection_end = end_index;

  clipboard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);      
      
  if (anchor_index != end_index)
    {
      gtk_clipboard_set_with_owner (clipboard,
				    targets,
				    G_N_ELEMENTS (targets),
				    get_text_callback,
				    clear_text_callback,
				    G_OBJECT (label));
    }
  else
    {
      if (gtk_clipboard_get_owner (clipboard) == G_OBJECT (label))
	gtk_clipboard_clear (clipboard);
    }
  
  gtk_widget_queue_draw (GTK_WIDGET (label));
  
  g_object_freeze_notify (G_OBJECT (label));
  g_object_notify (G_OBJECT (label), "cursor_position");
  g_object_notify (G_OBJECT (label), "selection_bound");
  g_object_thaw_notify (G_OBJECT (label));
}

/**
 * eel_editable_label_select_region:
 * @label: a #EelEditableLabel
 * @start_offset: start offset (in characters not bytes)
 * @end_offset: end offset (in characters not bytes)
 *
 * Selects a range of characters in the label, if the label is selectable.
 * See eel_editable_label_set_selectable(). If the label is not selectable,
 * this function has no effect. If @start_offset or
 * @end_offset are -1, then the end of the label will be substituted.
 * 
 **/
void
eel_editable_label_select_region  (EelEditableLabel *label,
				   gint      start_offset,
				   gint      end_offset)
{
  g_return_if_fail (EEL_IS_EDITABLE_LABEL (label));
  
  if (label->text)
    {
      if (start_offset < 0)
        start_offset = g_utf8_strlen (label->text, -1);
      
      if (end_offset < 0)
        end_offset = g_utf8_strlen (label->text, -1);
      
      eel_editable_label_select_region_index (label,
					      g_utf8_offset_to_pointer (label->text, start_offset) - label->text,
					      g_utf8_offset_to_pointer (label->text, end_offset) - label->text);
    }
}

/**
 * eel_editable_label_get_selection_bounds:
 * @label: a #EelEditableLabel
 * @start: return location for start of selection, as a character offset
 * @end: return location for end of selection, as a character offset
 * 
 * Gets the selected range of characters in the label, returning %TRUE
 * if there's a selection.
 * 
 * Return value: %TRUE if selection is non-empty
 **/
gboolean
eel_editable_label_get_selection_bounds (EelEditableLabel  *label,
					 gint      *start,
					 gint      *end)
{
  gint start_index, end_index;
  gint start_offset, end_offset;
  gint len;
  
  g_return_val_if_fail (EEL_IS_EDITABLE_LABEL (label), FALSE);

      
  start_index = MIN (label->selection_anchor,
		     label->selection_end);
  end_index = MAX (label->selection_anchor,
		   label->selection_end);

  len = strlen (label->text);
  
  if (end_index > len)
    end_index = len;
  
  if (start_index > len)
    start_index = len;
  
  start_offset = g_utf8_strlen (label->text, start_index);
  end_offset = g_utf8_strlen (label->text, end_index);
  
  if (start_offset > end_offset)
    {
      gint tmp = start_offset;
      start_offset = end_offset;
      end_offset = tmp;
    }
  
  if (start)
    *start = start_offset;
  
  if (end)
    *end = end_offset;
  
  return start_offset != end_offset;
}


/**
 * eel_editable_label_get_layout:
 * @label: a #EelEditableLabel
 * 
 * Gets the #PangoLayout used to display the label.
 * The layout is useful to e.g. convert text positions to
 * pixel positions, in combination with eel_editable_label_get_layout_offsets().
 * The returned layout is owned by the label so need not be
 * freed by the caller.
 * 
 * Return value: the #PangoLayout for this label
 **/
PangoLayout*
eel_editable_label_get_layout (EelEditableLabel *label)
{
  g_return_val_if_fail (EEL_IS_EDITABLE_LABEL (label), NULL);

  eel_editable_label_ensure_layout (label, TRUE);

  return label->layout;
}

/**
 * eel_editable_label_get_layout_offsets:
 * @label: a #EelEditableLabel
 * @x: location to store X offset of layout, or %NULL
 * @y: location to store Y offset of layout, or %NULL
 *
 * Obtains the coordinates where the label will draw the #PangoLayout
 * representing the text in the label; useful to convert mouse events
 * into coordinates inside the #PangoLayout, e.g. to take some action
 * if some part of the label is clicked. Of course you will need to
 * create a #GtkEventBox to receive the events, and pack the label
 * inside it, since labels are a #GTK_NO_WINDOW widget. Remember
 * when using the #PangoLayout functions you need to convert to
 * and from pixels using PANGO_PIXELS() or #PANGO_SCALE.
 * 
 **/
void
eel_editable_label_get_layout_offsets (EelEditableLabel *label,
				       gint     *x,
				       gint     *y)
{
  g_return_if_fail (EEL_IS_EDITABLE_LABEL (label));
  
  get_layout_location (label, x, y);
}

static void
eel_editable_label_pend_cursor_blink (EelEditableLabel *label)
{
  /* TODO */
}

static void
eel_editable_label_check_cursor_blink (EelEditableLabel *label)
{
  /* TODO */
}

static gint
eel_editable_label_key_press (GtkWidget   *widget,
			      GdkEventKey *event)
{
  EelEditableLabel *label = EEL_EDITABLE_LABEL (widget);

  eel_editable_label_pend_cursor_blink (label);

  if (gtk_im_context_filter_keypress (label->im_context, event))
    {
      //TODO eel_editable_label_obscure_mouse_cursor (label);
      label->need_im_reset = TRUE;
      return TRUE;
    }

  if (GTK_WIDGET_CLASS (parent_class)->key_press_event (widget, event))
    /* Activate key bindings
     */
    return TRUE;

  return FALSE;
}

static gint
eel_editable_label_key_release (GtkWidget   *widget,
				GdkEventKey *event)
{
  EelEditableLabel *label = EEL_EDITABLE_LABEL (widget);

  if (gtk_im_context_filter_keypress (label->im_context, event))
    {
      label->need_im_reset = TRUE;
      return TRUE;
    }

  return GTK_WIDGET_CLASS (parent_class)->key_release_event (widget, event);
}

static void
eel_editable_label_keymap_direction_changed (GdkKeymap *keymap,
					     EelEditableLabel  *label)
{
  gtk_widget_queue_draw (GTK_WIDGET (label));
}

static gint
eel_editable_label_focus_in (GtkWidget     *widget,
			     GdkEventFocus *event)
{
  EelEditableLabel *label = EEL_EDITABLE_LABEL (widget);
  
  gtk_widget_queue_draw (widget);
  
  label->need_im_reset = TRUE;
  gtk_im_context_focus_in (label->im_context);

  g_signal_connect (gdk_keymap_get_default (),
		    "direction_changed",
		    G_CALLBACK (eel_editable_label_keymap_direction_changed), label);

  eel_editable_label_check_cursor_blink (label);

  return FALSE;
}

static gint
eel_editable_label_focus_out (GtkWidget     *widget,
			      GdkEventFocus *event)
{
  EelEditableLabel *label = EEL_EDITABLE_LABEL (widget);
  
  gtk_widget_queue_draw (widget);

  label->need_im_reset = TRUE;
  gtk_im_context_focus_out (label->im_context);

  eel_editable_label_check_cursor_blink (label);
  
  g_signal_handlers_disconnect_by_func (gdk_keymap_get_default (),
                                        (gpointer) eel_editable_label_keymap_direction_changed,
                                        label);
  
  return FALSE;
}

static void
eel_editable_label_delete_text (EelEditableLabel *label,
				int start_pos,
				int end_pos)
{
  int anchor, end;
  
  if (start_pos < 0)
    start_pos = 0;
  if (end_pos < 0 || end_pos > label->n_bytes)
    end_pos = label->n_bytes;
  
  if (start_pos < end_pos)
    {
      g_memmove (label->text + start_pos, label->text + end_pos, label->n_bytes + 1 - end_pos);
      label->n_bytes -= (end_pos - start_pos);

      anchor = label->selection_anchor;
      if (anchor > start_pos)
	anchor -= MIN (anchor, end_pos) - start_pos;

      end = label->selection_end;
      if (end > start_pos)
	end -= MIN (end, end_pos) - start_pos;
      
      /* We might have changed the selection */
      eel_editable_label_select_region_index (label, anchor, end);
      
      eel_editable_label_recompute (label);  
      gtk_widget_queue_resize (GTK_WIDGET (label));
      
      g_object_notify (G_OBJECT (label), "text");
      g_signal_emit_by_name (GTK_EDITABLE (label), "changed");
    }
}

static void
eel_editable_label_delete_selection (EelEditableLabel *label)
{
  if (label->selection_anchor < label->selection_end)
    eel_editable_label_delete_text (label,
				    label->selection_anchor,
				    label->selection_end);
  else
    eel_editable_label_delete_text (label,
				    label->selection_end,
				    label->selection_anchor);
}

static void
eel_editable_label_insert_text (EelEditableLabel *label,
				const gchar *new_text,
				gint         new_text_length,
				gint        *index)
{
  if (new_text_length + label->n_bytes + 1 > label->text_size)
    {
      while (new_text_length + label->n_bytes + 1 > label->text_size)
	{
	  if (label->text_size == 0)
	    label->text_size = 16;
	  else
	    label->text_size *= 2;
	}

      label->text = g_realloc (label->text, label->text_size);
    }

  g_object_freeze_notify (G_OBJECT (label));

  g_memmove (label->text + *index + new_text_length, label->text + *index, label->n_bytes - *index);
  memcpy (label->text + *index, new_text, new_text_length);
  
  label->n_bytes += new_text_length;

  /* NUL terminate for safety and convenience */
  label->text[label->n_bytes] = '\0';

  g_object_notify (G_OBJECT (label), "text");

  if (label->selection_anchor > *index)
    {
      g_object_notify (G_OBJECT (label), "cursor_position");
      g_object_notify (G_OBJECT (label), "selection_bound");
      label->selection_anchor += new_text_length;
    }
  
  if (label->selection_end > *index)
    {
      label->selection_end += new_text_length;
      g_object_notify (G_OBJECT (label), "selection_bound");
    }

  *index += new_text_length;

  eel_editable_label_recompute (label);  
  gtk_widget_queue_resize (GTK_WIDGET (label));

  g_object_thaw_notify (G_OBJECT (label));
  g_signal_emit_by_name (GTK_EDITABLE (label), "changed");
}

/* Used for im_commit_cb and inserting Unicode chars */
static void
eel_editable_label_enter_text (EelEditableLabel *label,
			       const gchar    *str)
{
  gint tmp_pos;

  if (label->selection_end != label->selection_anchor)
    eel_editable_label_delete_selection (label);
  else
    {
      if (label->overwrite_mode)
        eel_editable_label_delete_from_cursor (label, GTK_DELETE_CHARS, 1);
    }
  
  tmp_pos = label->selection_anchor;
  eel_editable_label_insert_text (label, str, strlen (str), &tmp_pos);
  eel_editable_label_select_region_index (label, tmp_pos, tmp_pos);
}

/* IM Context Callbacks
 */

static void
eel_editable_label_commit_cb (GtkIMContext *context,
			      const gchar  *str,
			      EelEditableLabel  *label)
{
  eel_editable_label_enter_text (label, str);
}

static void 
eel_editable_label_preedit_changed_cb (GtkIMContext *context,
				       EelEditableLabel  *label)
{
  gchar *preedit_string;
  gint cursor_pos;
  
  gtk_im_context_get_preedit_string (label->im_context,
				     &preedit_string, NULL,
				     &cursor_pos);
  label->preedit_length = strlen (preedit_string);
  cursor_pos = CLAMP (cursor_pos, 0, g_utf8_strlen (preedit_string, -1));
  label->preedit_cursor = cursor_pos;
  g_free (preedit_string);

  eel_editable_label_recompute (label);  
  gtk_widget_queue_resize (GTK_WIDGET (label));
}

static gboolean
eel_editable_label_retrieve_surrounding_cb (GtkIMContext *context,
					    EelEditableLabel  *label)
{
  gtk_im_context_set_surrounding (context,
				  label->text,
				  strlen (label->text) + 1,
				  g_utf8_offset_to_pointer (label->text, label->selection_end) - label->text);

  return TRUE;
}

static gboolean
eel_editable_label_delete_surrounding_cb (GtkIMContext *slave,
					  gint          offset,
					  gint          n_chars,
					  EelEditableLabel  *label)
{
  gint start_index, end_index;
  char *text;

  text = label->text + label->selection_anchor;
  start_index = label->selection_anchor +
    g_utf8_offset_to_pointer (text, offset) - text;
  end_index = label->selection_anchor +
    g_utf8_offset_to_pointer (text, offset + n_chars) - text;
  
  eel_editable_label_delete_text (label, start_index, end_index);

  return TRUE;
}

static gboolean
eel_editable_label_focus (GtkWidget         *widget,
			  GtkDirectionType   direction)
{
  /* We never want to be in the tab chain */
  return FALSE;
}

/* Compute the X position for an offset that corresponds to the "more important
 * cursor position for that offset. We use this when trying to guess to which
 * end of the selection we should go to when the user hits the left or
 * right arrow key.
 */
static void
get_better_cursor (EelEditableLabel *label,
		   gint      index,
		   gint      *x,
		   gint      *y)
{
  GtkTextDirection keymap_direction =
    (gdk_keymap_get_direction (gdk_keymap_get_default ()) == PANGO_DIRECTION_LTR) ?
    GTK_TEXT_DIR_LTR : GTK_TEXT_DIR_RTL;
  GtkTextDirection widget_direction = gtk_widget_get_direction (GTK_WIDGET (label));
  gboolean split_cursor;
  PangoRectangle strong_pos, weak_pos;
  
  g_object_get (gtk_widget_get_settings (GTK_WIDGET (label)),
		"gtk-split-cursor", &split_cursor,
		NULL);

  eel_editable_label_get_cursor_pos (label, &strong_pos, &weak_pos);

  if (split_cursor)
    {
      *x = strong_pos.x / PANGO_SCALE;
      *y = strong_pos.y / PANGO_SCALE;
    }
  else
    {
      if (keymap_direction == widget_direction)
	{
	  *x = strong_pos.x / PANGO_SCALE;
	  *y = strong_pos.y / PANGO_SCALE;
	}
      else
	{
	  *x = weak_pos.x / PANGO_SCALE;
	  *y = weak_pos.y / PANGO_SCALE;
	}
    }
}


static gint
eel_editable_label_move_logically (EelEditableLabel *label,
				   gint      start,
				   gint      count)
{
  gint offset = g_utf8_pointer_to_offset (label->text,
					  label->text + start);

  if (label->text)
    {
      PangoLogAttr *log_attrs;
      gint n_attrs;
      gint length;

      eel_editable_label_ensure_layout (label, FALSE);
      
      length = g_utf8_strlen (label->text, -1);

      pango_layout_get_log_attrs (label->layout, &log_attrs, &n_attrs);

      while (count > 0 && offset < length)
	{
	  do
	    offset++;
	  while (offset < length && !log_attrs[offset].is_cursor_position);
	  
	  count--;
	}
      while (count < 0 && offset > 0)
	{
	  do
	    offset--;
	  while (offset > 0 && !log_attrs[offset].is_cursor_position);
	  
	  count++;
	}
      
      g_free (log_attrs);
    }

  return g_utf8_offset_to_pointer (label->text, offset) - label->text;
}

static gint
eel_editable_label_move_visually (EelEditableLabel *label,
				  gint      start,
				  gint      count)
{
  gint index;

  index = start;
  
  while (count != 0)
    {
      int new_index, new_trailing;
      gboolean split_cursor;
      gboolean strong;

      eel_editable_label_ensure_layout (label, FALSE);

      g_object_get (gtk_widget_get_settings (GTK_WIDGET (label)),
		    "gtk-split-cursor", &split_cursor,
		    NULL);

      if (split_cursor)
	strong = TRUE;
      else
	{
	  GtkTextDirection keymap_direction =
	    (gdk_keymap_get_direction (gdk_keymap_get_default ()) == PANGO_DIRECTION_LTR) ?
	    GTK_TEXT_DIR_LTR : GTK_TEXT_DIR_RTL;

	  strong = keymap_direction == gtk_widget_get_direction (GTK_WIDGET (label));
	}
      
      if (count > 0)
	{
	  pango_layout_move_cursor_visually (label->layout, strong, index, 0, 1, &new_index, &new_trailing);
	  count--;
	}
      else
	{
	  pango_layout_move_cursor_visually (label->layout, strong, index, 0, -1, &new_index, &new_trailing);
	  count++;
	}

      if (new_index < 0 || new_index == G_MAXINT)
	break;

      index = new_index;
      
      while (new_trailing--)
	index = g_utf8_next_char (label->text + new_index) - label->text;
    }
  
  return index;
}

static gint
eel_editable_label_move_line (EelEditableLabel *label,
			      gint      start,
			      gint      count)
{
  int n_lines, i;
  int x;
  PangoLayoutLine *line;
  int index;
  
  eel_editable_label_ensure_layout (label, FALSE);

  n_lines = pango_layout_get_line_count (label->layout);

  for (i = 0; i < n_lines; i++)
    {
      line = pango_layout_get_line (label->layout, i);
      if (start >= line->start_index &&
	  start <= line->start_index + line->length)
	{
	  pango_layout_line_index_to_x (line, start, FALSE, &x);
	  break;
	}
    }
  if (i == n_lines)
    i = n_lines - 1;
  
  i += count;
  i = CLAMP (i, 0, n_lines - 1);

  line = pango_layout_get_line (label->layout, i);
  if (pango_layout_line_x_to_index (line,
				    x,
				    &index, NULL))
    return index;
  else
    {
      if (i == n_lines - 1)
	return line->start_index + line->length;
      else
	return line->start_index + line->length - 1;
    }
      
  return start;
}

static gint
eel_editable_label_move_forward_word (EelEditableLabel *label,
				      gint      start)
{
  gint new_pos = g_utf8_pointer_to_offset (label->text,
					   label->text + start);
  gint length;

  length = g_utf8_strlen (label->text, -1);
  if (new_pos < length)
    {
      PangoLogAttr *log_attrs;
      gint n_attrs;

      eel_editable_label_ensure_layout (label, FALSE);
      
      pango_layout_get_log_attrs (label->layout, &log_attrs, &n_attrs);
      
      /* Find the next word end */
      new_pos++;
      while (new_pos < n_attrs && !log_attrs[new_pos].is_word_end)
	new_pos++;

      g_free (log_attrs);
    }

  return g_utf8_offset_to_pointer (label->text, new_pos) - label->text;
}


static gint
eel_editable_label_move_backward_word (EelEditableLabel *label,
				       gint      start)
{
  gint new_pos = g_utf8_pointer_to_offset (label->text,
					   label->text + start);
  gint length;

  length = g_utf8_strlen (label->text, -1);
  
  if (new_pos > 0)
    {
      PangoLogAttr *log_attrs;
      gint n_attrs;

      eel_editable_label_ensure_layout (label, FALSE);
      
      pango_layout_get_log_attrs (label->layout, &log_attrs, &n_attrs);
      
      new_pos -= 1;

      /* Find the previous word beginning */
      while (new_pos > 0 && !log_attrs[new_pos].is_word_start)
	new_pos--;

      g_free (log_attrs);
    }

  return g_utf8_offset_to_pointer (label->text, new_pos) - label->text;
}

static void
eel_editable_label_move_cursor (EelEditableLabel    *label,
				GtkMovementStep      step,
				gint                 count,
				gboolean             extend_selection)
{
  gint new_pos;
  
  new_pos = label->selection_end;

  if (label->selection_end != label->selection_anchor &&
      !extend_selection)
    {
      /* If we have a current selection and aren't extending it, move to the
       * start/or end of the selection as appropriate
       */
      switch (step)
	{
	case GTK_MOVEMENT_DISPLAY_LINES:
	case GTK_MOVEMENT_VISUAL_POSITIONS:
	  {
	    gint end_x, end_y;
	    gint anchor_x, anchor_y;
	    gboolean end_is_left;
	    
	    get_better_cursor (label, label->selection_end, &end_x, &end_y);
	    get_better_cursor (label, label->selection_anchor, &anchor_x, &anchor_y);

	    end_is_left = (end_y < anchor_y) || (end_y == anchor_y && end_x < anchor_x);
	    
	    if (count < 0)
	      new_pos = end_is_left ? label->selection_end : label->selection_anchor;
	    else
	      new_pos = !end_is_left ? label->selection_end : label->selection_anchor;

	    break;
	  }
	case GTK_MOVEMENT_LOGICAL_POSITIONS:
	case GTK_MOVEMENT_WORDS:
	  if (count < 0)
	    new_pos = MIN (label->selection_end, label->selection_anchor);
	  else
	    new_pos = MAX (label->selection_end, label->selection_anchor);
	  break;
	case GTK_MOVEMENT_DISPLAY_LINE_ENDS:
	case GTK_MOVEMENT_PARAGRAPH_ENDS:
	case GTK_MOVEMENT_BUFFER_ENDS:
	  /* FIXME: Can do better here */
	  new_pos = count < 0 ? 0 : strlen (label->text);
	  break;
	case GTK_MOVEMENT_PARAGRAPHS:
	case GTK_MOVEMENT_PAGES:
	  break;
	}
    }
  else
    {
      switch (step)
	{
	case GTK_MOVEMENT_LOGICAL_POSITIONS:
	  new_pos = eel_editable_label_move_logically (label, new_pos, count);
	  break;
	case GTK_MOVEMENT_VISUAL_POSITIONS:
	  new_pos = eel_editable_label_move_visually (label, new_pos, count);
	  break;
	case GTK_MOVEMENT_WORDS:
	  while (count > 0)
	    {
	      new_pos = eel_editable_label_move_forward_word (label, new_pos);
	      count--;
	    }
	  while (count < 0)
	    {
	      new_pos = eel_editable_label_move_backward_word (label, new_pos);
	      count++;
	    }
	  break;
	case GTK_MOVEMENT_DISPLAY_LINE_ENDS:
	case GTK_MOVEMENT_PARAGRAPH_ENDS:
	case GTK_MOVEMENT_BUFFER_ENDS:
	  /* FIXME: Can do better here */
	  new_pos = count < 0 ? 0 : strlen (label->text);
	  break;
	case GTK_MOVEMENT_DISPLAY_LINES:
	  new_pos = eel_editable_label_move_line (label, new_pos, count);
	  break;
	  break;
	case GTK_MOVEMENT_PARAGRAPHS:
	case GTK_MOVEMENT_PAGES:
	  break;
	}
    }

  if (extend_selection)
    eel_editable_label_select_region_index (label,
					    label->selection_anchor,
					    new_pos);
  else
    eel_editable_label_select_region_index (label, new_pos, new_pos);
}

static void
eel_editable_label_reset_im_context (EelEditableLabel  *label)
{
  if (label->need_im_reset)
    {
      label->need_im_reset = 0;
      gtk_im_context_reset (label->im_context);
    }
}


static void
eel_editable_label_delete_from_cursor (EelEditableLabel *label,
				       GtkDeleteType     type,
				       gint              count)
{
  gint start_pos = label->selection_anchor;
  gint end_pos = label->selection_anchor;
  
  eel_editable_label_reset_im_context (label);

  if (label->selection_anchor != label->selection_end)
    {
      eel_editable_label_delete_selection (label);
      return;
    }
  
  switch (type)
    {
    case GTK_DELETE_CHARS:
      end_pos = eel_editable_label_move_logically (label, start_pos, count);
      eel_editable_label_delete_text (label, MIN (start_pos, end_pos), MAX (start_pos, end_pos));
      break;
    case GTK_DELETE_WORDS:
      if (count < 0)
	{
	  /* Move to end of current word, or if not on a word, end of previous word */
	  end_pos = eel_editable_label_move_backward_word (label, end_pos);
	  end_pos = eel_editable_label_move_forward_word (label, end_pos);
	}
      else if (count > 0)
	{
	  /* Move to beginning of current word, or if not on a word, begining of next word */
	  start_pos = eel_editable_label_move_forward_word (label, start_pos);
	  start_pos = eel_editable_label_move_backward_word (label, start_pos);
	}
	
      /* Fall through */
    case GTK_DELETE_WORD_ENDS:
      while (count < 0)
	{
	  start_pos = eel_editable_label_move_backward_word (label, start_pos);
	  count++;
	}
      while (count > 0)
	{
	  end_pos = eel_editable_label_move_forward_word (label, end_pos);
	  count--;
	}
      eel_editable_label_delete_text (label, start_pos, end_pos);
      break;
    case GTK_DELETE_DISPLAY_LINE_ENDS:
    case GTK_DELETE_PARAGRAPH_ENDS:
      if (count < 0)
	eel_editable_label_delete_text (label, 0, label->selection_anchor);
      else
	eel_editable_label_delete_text (label, label->selection_anchor, -1);
      break;
    case GTK_DELETE_DISPLAY_LINES:
    case GTK_DELETE_PARAGRAPHS:
      eel_editable_label_delete_text (label, 0, -1);  
      break;
    case GTK_DELETE_WHITESPACE:
      /* TODO eel_editable_label_delete_whitespace (label); */
      break;
    }
  
  eel_editable_label_pend_cursor_blink (label);
}


static void
eel_editable_label_copy_clipboard (EelEditableLabel *label)
{
  if (label->text)
    {
      gint start, end;
      gint len;
      
      start = MIN (label->selection_anchor,
                   label->selection_end);
      end = MAX (label->selection_anchor,
                 label->selection_end);

      len = strlen (label->text);

      if (end > len)
        end = len;

      if (start > len)
        start = len;

      if (start != end)
	gtk_clipboard_set_text (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD),
				label->text + start, end - start);
    }
}

static void
eel_editable_label_cut_clipboard (EelEditableLabel *label)
{
  if (label->text)
    {
      gint start, end;
      gint len;
      
      start = MIN (label->selection_anchor,
                   label->selection_end);
      end = MAX (label->selection_anchor,
                 label->selection_end);

      len = strlen (label->text);

      if (end > len)
        end = len;

      if (start > len)
        start = len;

      if (start != end)
	{
	  gtk_clipboard_set_text (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD),
				  label->text + start, end - start);
	  eel_editable_label_delete_text (label, start, end);
	}
    }
}

static void
paste_received (GtkClipboard *clipboard,
		const gchar  *text,
		gpointer      data)
{
  EelEditableLabel *label = EEL_EDITABLE_LABEL (data);
      
  if (text)
    {
      gint tmp_pos;
      
      if (label->selection_end != label->selection_anchor)
	eel_editable_label_delete_selection (label);

      tmp_pos = label->selection_anchor;
      eel_editable_label_insert_text (label, text, strlen (text), &tmp_pos);
      eel_editable_label_select_region_index (label, tmp_pos, tmp_pos);
    }

  g_object_unref (G_OBJECT (label));
}

static void
eel_editable_label_paste (EelEditableLabel *label,
			  GdkAtom   selection)
{
  g_object_ref (G_OBJECT (label));
  gtk_clipboard_request_text (gtk_widget_get_clipboard (GTK_WIDGET (label), selection),
			      paste_received, label);
}

static void
eel_editable_label_paste_clipboard (EelEditableLabel *label)
{
  eel_editable_label_paste (label, GDK_NONE);
}

static void
eel_editable_label_select_all (EelEditableLabel *label)
{
  eel_editable_label_select_region_index (label, 0, strlen (label->text));
}

/* Quick hack of a popup menu
 */
static void
activate_cb (GtkWidget *menuitem,
	     EelEditableLabel  *label)
{
  const gchar *signal = g_object_get_data (G_OBJECT (menuitem), "gtk-signal");
  g_signal_emit_by_name (GTK_OBJECT (label), signal);
}

static void
append_action_signal (EelEditableLabel     *label,
		      GtkWidget    *menu,
		      const gchar  *stock_id,
		      const gchar  *signal,
                      gboolean      sensitive)
{
  GtkWidget *menuitem = gtk_image_menu_item_new_from_stock (stock_id, NULL);

  g_object_set_data (G_OBJECT (menuitem), "gtk-signal", (char *)signal);
  g_signal_connect (GTK_OBJECT (menuitem), "activate",
		    GTK_SIGNAL_FUNC (activate_cb), label);

  gtk_widget_set_sensitive (menuitem, sensitive);
  
  gtk_widget_show (menuitem);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
}

static void
popup_menu_detach (GtkWidget *attach_widget,
		   GtkMenu   *menu)
{
  EelEditableLabel *label;
  label = EEL_EDITABLE_LABEL (attach_widget);

  label->popup_menu = NULL;
}

static void
popup_position_func (GtkMenu   *menu,
                     gint      *x,
                     gint      *y,
                     gboolean  *push_in,
                     gpointer	user_data)
{
  EelEditableLabel *label;
  GtkWidget *widget;
  GtkRequisition req;
  
  label = EEL_EDITABLE_LABEL (user_data);  
  widget = GTK_WIDGET (label);

  g_return_if_fail (GTK_WIDGET_REALIZED (label));

  gdk_window_get_origin (widget->window, x, y);      

  //gtk_widget_size_request (label->popup_menu, &req);
  req = widget->requisition;
  
  *x += widget->allocation.width / 2;
  *y += widget->allocation.height;

  *x = CLAMP (*x, 0, MAX (0, gdk_screen_width () - req.width));
  *y = CLAMP (*y, 0, MAX (0, gdk_screen_height () - req.height));
}

static void
eel_editable_label_toggle_overwrite (EelEditableLabel *label)
{
  label->overwrite_mode = !label->overwrite_mode;
}

typedef struct
{
  EelEditableLabel *label;
  gint button;
  guint time;
} PopupInfo;

static void
popup_targets_received (GtkClipboard     *clipboard,
			GtkSelectionData *data,
			gpointer          user_data)
{
  GtkWidget *menuitem, *submenu;
  gboolean have_selection;
  gboolean clipboard_contains_text;
  PopupInfo *info;
  EelEditableLabel *label;

  info = user_data;
  label = info->label;

  if (GTK_WIDGET_REALIZED (label))
    {
      if (label->popup_menu)
	gtk_widget_destroy (label->popup_menu);
  
      label->popup_menu = gtk_menu_new ();

      gtk_menu_attach_to_widget (GTK_MENU (label->popup_menu),
				 GTK_WIDGET (label),
				 popup_menu_detach);

      have_selection =
	label->selection_anchor != label->selection_end;
  
      clipboard_contains_text = gtk_selection_data_targets_include_text (data);

      append_action_signal (label, label->popup_menu, GTK_STOCK_CUT, "cut_clipboard",
			    have_selection);
      append_action_signal (label, label->popup_menu, GTK_STOCK_COPY, "copy_clipboard",
			    have_selection);
      append_action_signal (label, label->popup_menu, GTK_STOCK_PASTE, "paste_clipboard",
			    clipboard_contains_text);
  
      menuitem = gtk_menu_item_new_with_label (_("Select All"));
      g_signal_connect_object (GTK_OBJECT (menuitem), "activate",
			       GTK_SIGNAL_FUNC (eel_editable_label_select_all), label,
			       G_CONNECT_SWAPPED);
      gtk_widget_show (menuitem);
      gtk_menu_shell_append (GTK_MENU_SHELL (label->popup_menu), menuitem);

      menuitem = gtk_separator_menu_item_new ();
      gtk_widget_show (menuitem);
      gtk_menu_shell_append (GTK_MENU_SHELL (label->popup_menu), menuitem);
      
      menuitem = gtk_menu_item_new_with_label (_("Input Methods"));
      gtk_widget_show (menuitem);
      submenu = gtk_menu_new ();
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), submenu);
      
      gtk_menu_shell_append (GTK_MENU_SHELL (label->popup_menu), menuitem);
  
      gtk_im_multicontext_append_menuitems (GTK_IM_MULTICONTEXT (label->im_context),
					    GTK_MENU_SHELL (submenu));

      g_signal_emit (GTK_OBJECT (label),
		     signals[POPULATE_POPUP], 0,
		     label->popup_menu);

      if (info->button)
	gtk_menu_popup (GTK_MENU (label->popup_menu), NULL, NULL,
			NULL, NULL,
			info->button, info->time);
      else
	{
	  gtk_menu_popup (GTK_MENU (label->popup_menu), NULL, NULL,
			  popup_position_func, label,
			  info->button, info->time);
	  gtk_menu_shell_select_first (GTK_MENU_SHELL (label->popup_menu), FALSE);
	}
    }

  g_object_unref (label);
  g_free (info);
}

static void
eel_editable_label_do_popup (EelEditableLabel *label,
			     GdkEventButton   *event)
{
  PopupInfo *info = g_new (PopupInfo, 1);

  /* In order to know what entries we should make sensitive, we
   * ask for the current targets of the clipboard, and when
   * we get them, then we actually pop up the menu.
   */
  info->label = g_object_ref (label);
  
  if (event)
    {
      info->button = event->button;
      info->time = event->time;
    }
  else
    {
      info->button = 0;
      info->time = gtk_get_current_event_time ();
    }

  gtk_clipboard_request_contents (gtk_widget_get_clipboard (GTK_WIDGET (label), GDK_SELECTION_CLIPBOARD),
				  gdk_atom_intern ("TARGETS", FALSE),
				  popup_targets_received,
				  info);
}

/************ Editable implementation ****************/

static void
editable_insert_text_emit  (GtkEditable *editable,
			    const gchar *new_text,
			    gint         new_text_length,
			    gint        *position)
{
  EelEditableLabel *label = EEL_EDITABLE_LABEL (editable);
  gchar buf[64];
  gchar *text;
  int text_length;

  text_length = g_utf8_strlen (label->text, -1);

  if (*position < 0 || *position > text_length)
    *position = text_length;
  
  g_object_ref (G_OBJECT (editable));
  
  if (new_text_length <= 63)
    text = buf;
  else
    text = g_new (gchar, new_text_length + 1);

  text[new_text_length] = '\0';
  strncpy (text, new_text, new_text_length);
  
  g_signal_emit_by_name (editable, "insert_text", text, new_text_length, position);

  if (new_text_length > 63)
    g_free (text);

  g_object_unref (G_OBJECT (editable));
}

static void
editable_delete_text_emit (GtkEditable *editable,
			   gint         start_pos,
			   gint         end_pos)
{
  EelEditableLabel *label = EEL_EDITABLE_LABEL (editable);
  int text_length;

  text_length = g_utf8_strlen (label->text, -1);

  if (end_pos < 0 || end_pos > text_length)
    end_pos = text_length;
  if (start_pos < 0)
    start_pos = 0;
  if (start_pos > end_pos)
    start_pos = end_pos;
  
  g_object_ref (G_OBJECT (editable));

  g_signal_emit_by_name (editable, "delete_text", start_pos, end_pos);

  g_object_unref (G_OBJECT (editable));
}

static void
editable_insert_text (GtkEditable *editable,
		      const gchar *new_text,
		      gint         new_text_length,
		      gint        *position)
{
  EelEditableLabel *label = EEL_EDITABLE_LABEL (editable);
  gint index;

  if (new_text_length < 0)
    new_text_length = strlen (new_text);

  index = g_utf8_offset_to_pointer (label->text, *position) - label->text;

  eel_editable_label_insert_text (label,
				  new_text,
				  new_text_length,
				  &index);
  
  *position = g_utf8_pointer_to_offset (label->text, label->text + index);
}

static void
editable_delete_text (GtkEditable *editable,
		      gint         start_pos,
		      gint         end_pos)
{
  EelEditableLabel *label = EEL_EDITABLE_LABEL (editable);
  int text_length;
  gint start_index, end_index;

  text_length = g_utf8_strlen (label->text, -1);

  if (end_pos < 0 || end_pos > text_length)
    end_pos = text_length;
  if (start_pos < 0)
    start_pos = 0;
  if (start_pos > end_pos)
    start_pos = end_pos;

  start_index = g_utf8_offset_to_pointer (label->text, start_pos) - label->text;
  end_index = g_utf8_offset_to_pointer (label->text, end_pos) - label->text;
  
  eel_editable_label_delete_text (label, start_index, end_index);
}

static gchar *    
editable_get_chars (GtkEditable   *editable,
		    gint           start_pos,
		    gint           end_pos)
{
  EelEditableLabel *label = EEL_EDITABLE_LABEL (editable);
  int text_length;
  gint start_index, end_index;
  
  text_length = g_utf8_strlen (label->text, -1);

  if (end_pos < 0 || end_pos > text_length)
    end_pos = text_length;
  if (start_pos < 0)
    start_pos = 0;
  if (start_pos > end_pos)
    start_pos = end_pos;

  start_index = g_utf8_offset_to_pointer (label->text, start_pos) - label->text;
  end_index = g_utf8_offset_to_pointer (label->text, end_pos) - label->text;

  return g_strndup (label->text + start_index, end_index - start_index);
}

static void
editable_set_selection_bounds (GtkEditable *editable,
			       gint         start,
			       gint         end)
{
  EelEditableLabel *label = EEL_EDITABLE_LABEL (editable);
  int text_length;
  gint start_index, end_index;
  
  text_length = g_utf8_strlen (label->text, -1);

  if (end < 0 || end > text_length)
    end = text_length;
  if (start < 0)
    start = text_length;
  if (start > text_length)
    start = text_length;
  
  eel_editable_label_reset_im_context (label);

  start_index = g_utf8_offset_to_pointer (label->text, start) - label->text;
  end_index = g_utf8_offset_to_pointer (label->text, end) - label->text;

  eel_editable_label_select_region_index (label, start_index, end_index);
}

static gboolean
editable_get_selection_bounds (GtkEditable *editable,
			       gint        *start,
			       gint        *end)
{
  EelEditableLabel *label = EEL_EDITABLE_LABEL (editable);

  *start = g_utf8_pointer_to_offset (label->text, label->text + label->selection_anchor);
  *end = g_utf8_pointer_to_offset (label->text, label->text + label->selection_end);

  return (label->selection_anchor != label->selection_end);
}

static void
editable_real_set_position (GtkEditable *editable,
			    gint         position)
{
  EelEditableLabel *label = EEL_EDITABLE_LABEL (editable);
  int text_length;
  int index;
  
  text_length = g_utf8_strlen (label->text, -1);
  
  if (position < 0 || position > text_length)
    position = text_length;

  index = g_utf8_offset_to_pointer (label->text, position) - label->text;
  
  if (index != label->selection_anchor ||
      index != label->selection_end)
    {
      eel_editable_label_select_region_index (label, index, index);
    }
}

static gint
editable_get_position (GtkEditable *editable)
{
  EelEditableLabel *label = EEL_EDITABLE_LABEL (editable);
  
  return g_utf8_pointer_to_offset (label->text, label->text + label->selection_anchor);
}


     
