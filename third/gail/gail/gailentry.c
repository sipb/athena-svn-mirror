/* GAIL - The GNOME Accessibility Implementation Library
 * Copyright 2001 Sun Microsystems Inc.
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
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include <gtk/gtk.h>
#include "gailentry.h"
#include "gailcombo.h"
#include <libgail-util/gailmisc.h>

static void       gail_entry_class_init            (GailEntryClass       *klass);
static void       gail_entry_object_init           (GailEntry            *entry);
static void	  gail_entry_real_initialize       (AtkObject            *obj,
                                                    gpointer             data);
static void	  gail_entry_real_notify_gtk	   (GObject		 *obj,
                                                    GParamSpec		 *pspec);
static void       gail_entry_finalize              (GObject              *object);

static gint       gail_entry_get_index_in_parent   (AtkObject            *accessible);

/* atkobject.h */

static AtkStateSet* gail_entry_ref_state_set       (AtkObject            *accessible);

/* atktext.h */

static void       atk_text_interface_init          (AtkTextIface         *iface);

static gchar*     gail_entry_get_text              (AtkText              *text,
                                                    gint                 start_pos,
                                                    gint                 end_pos);
static gunichar	  gail_entry_get_character_at_offset
						   (AtkText		 *text,
						    gint		 offset);
static gchar*	  gail_entry_get_text_before_offset(AtkText		 *text,
						    gint		 offset,
						    AtkTextBoundary	 boundary_type,
						    gint		 *start_offset,
						    gint		 *end_offset);
static gchar*	  gail_entry_get_text_at_offset	   (AtkText		 *text,
						    gint		 offset,
						    AtkTextBoundary	 boundary_type,
						    gint		 *start_offset,
						    gint		 *end_offset);
static gchar*	  gail_entry_get_text_after_offset (AtkText		 *text,
						    gint		 offset,
						    AtkTextBoundary	 boundary_type,
						    gint		 *start_offset,
						    gint		 *end_offset);
static gint       gail_entry_get_caret_offset      (AtkText              *text);
static gboolean   gail_entry_set_caret_offset      (AtkText              *text,
						    gint                 offset);
static gint	  gail_entry_get_n_selections	   (AtkText		 *text);
static gchar*	  gail_entry_get_selection	   (AtkText		 *text,
						    gint		 selection_num,
						    gint		 *start_offset,
						    gint		 *end_offset);
static gboolean	  gail_entry_add_selection	   (AtkText		 *text,
						    gint		 start_offset,
						    gint		 end_offset);
static gboolean	  gail_entry_remove_selection	   (AtkText		 *text,
						    gint		 selection_num);
static gboolean	  gail_entry_set_selection	   (AtkText		 *text,
						    gint		 selection_num,
						    gint		 start_offset,
						    gint		 end_offset);
static gint	  gail_entry_get_character_count   (AtkText		 *text);
static AtkAttributeSet *  gail_entry_get_run_attributes 
                                                   (AtkText              *text,
						    gint		 offset,
        					    gint		 *start_offset,
					       	    gint 		 *end_offset);
static AtkAttributeSet *  gail_entry_get_default_attributes 
                                                   (AtkText              *text);
static void gail_entry_get_character_extents       (AtkText	         *text,
						    gint 	         offset,
		                                    gint 	         *x,
                    		   	            gint 	         *y,
                                		    gint 	         *width,
                                     		    gint 	         *height,
			        		    AtkCoordType         coords);
static gint gail_entry_get_offset_at_point         (AtkText              *text,
                                                    gint                 x,
                                                    gint                 y,
			                            AtkCoordType         coords);
/* atkeditabletext.h */

static void       atk_editable_text_interface_init (AtkEditableTextIface *iface);
static void       gail_entry_set_text_contents     (AtkEditableText      *text,
                                                    const gchar          *string);
static void       gail_entry_insert_text           (AtkEditableText      *text,
                                                    const gchar          *string,
                                                    gint                 length,
                                                    gint                 *position);
static void       gail_entry_copy_text             (AtkEditableText      *text,
                                                    gint                 start_pos,
                                                    gint                 end_pos);
static void       gail_entry_cut_text              (AtkEditableText      *text,
                                                    gint                 start_pos,
                                                    gint                 end_pos);
static void       gail_entry_delete_text           (AtkEditableText      *text,
                                                    gint                 start_pos,
                                                    gint                 end_pos);
static void       gail_entry_paste_text            (AtkEditableText      *text,
                                                    gint                 position);
static void       gail_entry_paste_received	   (GtkClipboard *clipboard,
						    const gchar  *text,
						    gpointer     data);


/* Callbacks */

static void	  _gail_entry_insert_text_cb	   (GtkEntry     	 *entry,
                                                    gchar		 *arg1,
                                                    gint		 arg2,
                                                    gpointer		 arg3);
static void	  _gail_entry_delete_text_cb	   (GtkEntry		 *entry,
                                                    gint		 arg1,
                                                    gint		 arg2);
static void	  _gail_entry_changed_cb           (GtkEntry		 *entry);
static gboolean   check_for_selection_change       (GailEntry            *entry,
                                                    GtkEntry             *gail_entry);

static void                  atk_action_interface_init   (AtkActionIface  *iface);

static gboolean              gail_entry_do_action        (AtkAction       *action,
                                                          gint            i);
static gint                  gail_entry_get_n_actions    (AtkAction       *action);
static G_CONST_RETURN gchar* gail_entry_get_description  (AtkAction       *action,
                                                          gint            i);
static G_CONST_RETURN gchar* gail_entry_action_get_name  (AtkAction       *action,
                                                          gint            i);
static gboolean              gail_entry_set_description  (AtkAction       *action,
                                                          gint            i,
                                                          const gchar     *desc);

static GailWidgetClass *parent_class = NULL;

typedef struct _GailEntryPaste			GailEntryPaste;

struct _GailEntryPaste
{
  GtkEntry* entry;
  gint position;
};

GType
gail_entry_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo tinfo =
      {
        sizeof (GailEntryClass),
        (GBaseInitFunc) NULL, /* base init */
        (GBaseFinalizeFunc) NULL, /* base finalize */
        (GClassInitFunc) gail_entry_class_init, /* class init */
        (GClassFinalizeFunc) NULL, /* class finalize */
        NULL, /* class data */
        sizeof (GailEntry), /* instance size */
        0, /* nb preallocs */
        (GInstanceInitFunc) gail_entry_object_init, /* instance init */
        NULL /* value table */
      };

      static const GInterfaceInfo atk_editable_text_info =
      {
        (GInterfaceInitFunc) atk_editable_text_interface_init,
        (GInterfaceFinalizeFunc) NULL,
        NULL
      };

      static const GInterfaceInfo atk_text_info =
      {
        (GInterfaceInitFunc) atk_text_interface_init,
        (GInterfaceFinalizeFunc) NULL,
        NULL
      };

      static const GInterfaceInfo atk_action_info =
      {
        (GInterfaceInitFunc) atk_action_interface_init,
        (GInterfaceFinalizeFunc) NULL,
        NULL
      };

      type = g_type_register_static (GAIL_TYPE_WIDGET,
                                     "GailEntry", &tinfo, 0);
      g_type_add_interface_static (type, ATK_TYPE_EDITABLE_TEXT,
                                   &atk_editable_text_info);
      g_type_add_interface_static (type, ATK_TYPE_TEXT,
                                   &atk_text_info);
      g_type_add_interface_static (type, ATK_TYPE_ACTION,
                                   &atk_action_info);
    }
  return type;
}

static void
gail_entry_class_init (GailEntryClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  AtkObjectClass  *class = ATK_OBJECT_CLASS (klass);
  GailWidgetClass *widget_class;

  widget_class = (GailWidgetClass*)klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = gail_entry_finalize;

  class->ref_state_set = gail_entry_ref_state_set;
  class->get_index_in_parent = gail_entry_get_index_in_parent;
  class->initialize = gail_entry_real_initialize;

  widget_class->notify_gtk = gail_entry_real_notify_gtk;
}

static void
gail_entry_object_init (GailEntry *entry)
{
  entry->textutil = NULL;
  entry->signal_name = NULL;
  entry->cursor_position = 0;
  entry->selection_bound = 0;
  entry->activate_description = NULL;
}

static void
gail_entry_real_initialize (AtkObject *obj, 
                            gpointer  data)
{
  GtkEntry *entry;
  GailEntry *gail_entry;
  const gchar *entry_text;

  ATK_OBJECT_CLASS (parent_class)->initialize (obj, data);

  gail_entry = GAIL_ENTRY (obj);
  gail_entry->textutil = gail_text_util_new ();
  
  g_assert (GTK_IS_ENTRY (data));

  entry = GTK_ENTRY (data);
  entry_text = gtk_entry_get_text (entry);
  gail_text_util_text_setup (gail_entry->textutil, entry_text);

  /* Set up signal callbacks */
  g_signal_connect (data, "insert-text",
	G_CALLBACK (_gail_entry_insert_text_cb), NULL);
  g_signal_connect (data, "delete-text",
	G_CALLBACK (_gail_entry_delete_text_cb), NULL);
  g_signal_connect (data, "changed",
	G_CALLBACK (_gail_entry_changed_cb), NULL);
}

static void
gail_entry_real_notify_gtk (GObject		*obj,
                            GParamSpec		*pspec)
{
  GtkWidget *widget = GTK_WIDGET (obj);
  AtkObject* atk_obj = gtk_widget_get_accessible (widget);
  GtkEntry* gtk_entry = GTK_ENTRY (widget);
  GailEntry* entry = GAIL_ENTRY (atk_obj);

  if (strcmp (pspec->name, "cursor-position") == 0)
    {
      entry = GAIL_ENTRY (atk_obj);

      if (entry->signal_name)
        {
          g_signal_emit_by_name (atk_obj, 
                                 entry->signal_name,
                                 entry->position,
                                 entry->length);
          entry->signal_name = NULL;
        }
      if (check_for_selection_change (entry, gtk_entry))
        g_signal_emit_by_name (atk_obj, "text_selection_changed");
      /*
       * The entry cursor position has moved so generate the signal.
       */
      g_signal_emit_by_name (atk_obj, "text_caret_moved", 
                             entry->cursor_position);
    }
  else if (strcmp (pspec->name, "selection-bound") == 0)
    {
      if (check_for_selection_change (entry, gtk_entry))
        g_signal_emit_by_name (atk_obj, "text_selection_changed");
    }
  else if (strcmp (pspec->name, "editable") == 0)
    {
      GValue value = { 0, };

      g_value_init (&value, G_TYPE_BOOLEAN);
      g_object_get_property (obj, "editable", &value);
      atk_object_notify_state_change (atk_obj, ATK_STATE_EDITABLE,
                                               g_value_get_boolean (&value));
    }
  else
    parent_class->notify_gtk (obj, pspec);
}

static void
gail_entry_finalize (GObject            *object)
{
  GailEntry *entry = GAIL_ENTRY (object);

  g_object_unref (entry->textutil);
  g_free (entry->activate_description);
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

AtkObject* 
gail_entry_new (GtkWidget *widget)
{
  GObject *object;
  AtkObject *accessible;

  g_return_val_if_fail (GTK_IS_ENTRY (widget), NULL);

  object = g_object_new (GAIL_TYPE_ENTRY, NULL);

  accessible = ATK_OBJECT (object);
  atk_object_initialize (accessible, widget);

  if (GTK_ENTRY (widget)->visible)
    accessible->role = ATK_ROLE_TEXT;
  else
    accessible->role = ATK_ROLE_PASSWORD_TEXT;

  return accessible;
}

static gint
gail_entry_get_index_in_parent (AtkObject *accessible)
{
  /*
   * If the parent widget is a combo box then the index is 1
   * otherwise do the normal thing.
   */
  if (accessible->accessible_parent)
    if (GAIL_IS_COMBO (accessible->accessible_parent))
      return 1;

  return ATK_OBJECT_CLASS (parent_class)->get_index_in_parent (accessible);
}

/* atkobject.h */

static AtkStateSet*
gail_entry_ref_state_set (AtkObject *accessible)
{
  AtkStateSet *state_set;
  GtkEntry *entry;
  GValue value = { 0, };
  GtkWidget *widget;

  state_set = ATK_OBJECT_CLASS (parent_class)->ref_state_set (accessible);
  widget = GTK_ACCESSIBLE (accessible)->widget;
  
  if (widget == NULL)
    return state_set;

  entry = GTK_ENTRY (widget);

  g_value_init (&value, G_TYPE_BOOLEAN);
  g_object_get_property (G_OBJECT (entry), "editable", &value);
  if (g_value_get_boolean (&value))
    atk_state_set_add_state (state_set, ATK_STATE_EDITABLE);
  atk_state_set_add_state (state_set, ATK_STATE_SINGLE_LINE);

  return state_set;
}

/* atktext.h */

static void
atk_text_interface_init (AtkTextIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->get_text = gail_entry_get_text;
  iface->get_character_at_offset = gail_entry_get_character_at_offset;
  iface->get_text_before_offset = gail_entry_get_text_before_offset;
  iface->get_text_at_offset = gail_entry_get_text_at_offset;
  iface->get_text_after_offset = gail_entry_get_text_after_offset;
  iface->get_caret_offset = gail_entry_get_caret_offset;
  iface->set_caret_offset = gail_entry_set_caret_offset;
  iface->get_character_count = gail_entry_get_character_count;
  iface->get_n_selections = gail_entry_get_n_selections;
  iface->get_selection = gail_entry_get_selection;
  iface->add_selection = gail_entry_add_selection;
  iface->remove_selection = gail_entry_remove_selection;
  iface->set_selection = gail_entry_set_selection;
  iface->get_run_attributes = gail_entry_get_run_attributes;
  iface->get_default_attributes = gail_entry_get_default_attributes;
  iface->get_character_extents = gail_entry_get_character_extents;
  iface->get_offset_at_point = gail_entry_get_offset_at_point;
}

static gchar*
gail_entry_get_text (AtkText *text,
                     gint    start_pos,
                     gint    end_pos)
{
  GtkEntry *entry;
  GtkWidget *widget;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return NULL;

  entry = GTK_ENTRY (widget);
  
  return gtk_editable_get_chars (GTK_EDITABLE (entry), start_pos, end_pos);
}

static gchar*
gail_entry_get_text_before_offset (AtkText	    *text,
				   gint		    offset,
				   AtkTextBoundary  boundary_type,
				   gint		    *start_offset,
				   gint		    *end_offset)
{
  GtkWidget *widget;
  GtkEntry *entry;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return NULL;

  /* Get Entry */
  entry = GTK_ENTRY (widget);

  return gail_text_util_get_text (GAIL_ENTRY (text)->textutil,
                          gtk_entry_get_layout (entry), GAIL_BEFORE_OFFSET, 
                          boundary_type, offset, start_offset, end_offset);
}

static gchar*
gail_entry_get_text_at_offset (AtkText          *text,
                               gint             offset,
                               AtkTextBoundary  boundary_type,
                               gint             *start_offset,
                               gint             *end_offset)
{
  GtkWidget *widget;
  GtkEntry *entry;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return NULL;

  /* Get Entry */
  entry = GTK_ENTRY (widget);

  return gail_text_util_get_text (GAIL_ENTRY (text)->textutil,
                            gtk_entry_get_layout (entry), GAIL_AT_OFFSET, 
                            boundary_type, offset, start_offset, end_offset);
}

static gchar*
gail_entry_get_text_after_offset  (AtkText	    *text,
				   gint		    offset,
				   AtkTextBoundary  boundary_type,
				   gint		    *start_offset,
				   gint		    *end_offset)
{
  GtkWidget *widget;
  GtkEntry *entry;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return NULL;

  /* Get Entry */
  entry = GTK_ENTRY (widget);

  return gail_text_util_get_text (GAIL_ENTRY (text)->textutil,
                           gtk_entry_get_layout (entry), GAIL_AFTER_OFFSET, 
                           boundary_type, offset, start_offset, end_offset);
}

static gint
gail_entry_get_character_count (AtkText *text)
{
  GtkEntry *entry;
  GtkWidget *widget;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return 0;

  entry = GTK_ENTRY (widget);
  return g_utf8_strlen (gtk_entry_get_text (entry), -1);
}

static gint
gail_entry_get_caret_offset (AtkText *text)
{
  GtkEntry *entry;
  GtkWidget *widget;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return 0;

  entry = GTK_ENTRY (widget);

  return gtk_editable_get_position (GTK_EDITABLE (entry));
}

static gboolean
gail_entry_set_caret_offset (AtkText *text, gint offset)
{
  GtkEntry *entry;
  GtkWidget *widget;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return FALSE;

  entry = GTK_ENTRY (widget);

  gtk_editable_set_position (GTK_EDITABLE (entry), offset);
  return TRUE;
}

static AtkAttributeSet*
gail_entry_get_run_attributes (AtkText *text,
			       gint    offset,
                               gint    *start_offset,
                               gint    *end_offset)
{
  GtkWidget *widget;
  GtkEntry *entry;
  AtkAttributeSet *at_set = NULL;
  GtkTextDirection dir;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return NULL;

  entry = GTK_ENTRY (widget);
 
  dir = gtk_widget_get_direction (widget);
  if (dir == GTK_TEXT_DIR_RTL)
    {
      at_set = gail_misc_add_attribute (at_set,
                                        ATK_TEXT_ATTR_DIRECTION,
       g_strdup (atk_text_attribute_get_value (ATK_TEXT_ATTR_DIRECTION, dir)));
    }

  at_set = gail_misc_layout_get_run_attributes (at_set,
                                                gtk_entry_get_layout (entry),
                                                entry->text,
                                                offset,
                                                start_offset,
                                                end_offset);
  return at_set;
}

static AtkAttributeSet*
gail_entry_get_default_attributes (AtkText *text)
{
  GtkWidget *widget;
  GtkEntry *entry;
  AtkAttributeSet *at_set = NULL;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return NULL;

  entry = GTK_ENTRY (widget);

  at_set = gail_misc_get_default_attributes (at_set,
                                             gtk_entry_get_layout (entry),
                                             widget);
  return at_set;
}
  
static void
gail_entry_get_character_extents (AtkText *text,
				  gint    offset,
		                  gint    *x,
                    		  gint 	  *y,
                                  gint 	  *width,
                                  gint 	  *height,
			          AtkCoordType coords)
{
  GtkWidget *widget;
  GtkEntry *entry;
  PangoRectangle char_rect;
  gint index, cursor_index, x_layout, y_layout;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return;

  entry = GTK_ENTRY (widget);

  gtk_entry_get_layout_offsets (entry, &x_layout, &y_layout);
  index = g_utf8_offset_to_pointer (entry->text, offset) - entry->text;
  cursor_index = g_utf8_offset_to_pointer (entry->text, entry->current_pos) - 
                          entry->text;
  if (index > cursor_index)
    index += entry->preedit_length;
  pango_layout_index_to_pos (gtk_entry_get_layout(entry), index, &char_rect);
 
  gail_misc_get_extents_from_pango_rectangle (widget, &char_rect, 
                        x_layout, y_layout, x, y, width, height, coords);
} 

static gint 
gail_entry_get_offset_at_point (AtkText *text,
                                gint x,
                                gint y,
			        AtkCoordType coords)
{ 
  GtkWidget *widget;
  GtkEntry *entry;
  gint index, cursor_index, x_layout, y_layout;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return -1;

  entry = GTK_ENTRY (widget);
  
  gtk_entry_get_layout_offsets (entry, &x_layout, &y_layout);
  
  index = gail_misc_get_index_at_point_in_layout (widget, 
               gtk_entry_get_layout(entry), x_layout, y_layout, x, y, coords);
  if (index == -1)
    {
      if (coords == ATK_XY_SCREEN || coords == ATK_XY_WINDOW)
        return g_utf8_strlen (entry->text, -1);

      return index;  
    }
  else
    {
      cursor_index = g_utf8_offset_to_pointer (entry->text, 
                                               entry->current_pos) -
                                             entry->text;
      if (index >= cursor_index && entry->preedit_length)
        {
          if (index >= cursor_index + entry->preedit_length)
            index -= entry->preedit_length;
          else
            index = cursor_index;
        }
      return g_utf8_pointer_to_offset (entry->text, entry->text + index);
    }
}

static gint
gail_entry_get_n_selections (AtkText              *text)
{
  GtkEntry *entry;
  GtkWidget *widget;
  gint select_start, select_end;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return -1;

  entry = GTK_ENTRY (widget);

  gtk_editable_get_selection_bounds (GTK_EDITABLE (entry), &select_start, 
                                     &select_end);

  if (select_start != select_end)
    return 1;
  else
    return 0;
}

static gchar*
gail_entry_get_selection (AtkText *text,
			  gint    selection_num,
                          gint    *start_pos,
                          gint    *end_pos)
{
  GtkEntry *entry;
  GtkWidget *widget;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return NULL;

 /* Only let the user get the selection if one is set, and if the
  * selection_num is 0.
  */
  if (selection_num != 0)
     return 0;

  entry = GTK_ENTRY (widget);
  gtk_editable_get_selection_bounds (GTK_EDITABLE (entry), start_pos, end_pos);

  if (*start_pos != *end_pos)
     return gtk_editable_get_chars (GTK_EDITABLE (entry), *start_pos, *end_pos);
  else
     return 0;
}

static gboolean
gail_entry_add_selection (AtkText *text,
                          gint    start_pos,
                          gint    end_pos)
{
  GtkEntry *entry;
  GtkWidget *widget;
  gint select_start, select_end;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return FALSE;

  entry = GTK_ENTRY (widget);

  gtk_editable_get_selection_bounds (GTK_EDITABLE (entry), &select_start, 
                                     &select_end);

 /* If there is already a selection, then don't allow another to be added,
  * since GtkEntry only supports one selected region.
  */
  if (select_start == select_end)
    {
       gtk_editable_select_region (GTK_EDITABLE (entry), start_pos, end_pos);
       return TRUE;
    }
  else
   return FALSE;
}

static gboolean
gail_entry_remove_selection (AtkText *text,
                             gint    selection_num)
{
  GtkEntry *entry;
  GtkWidget *widget;
  gint select_start, select_end, caret_pos;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return FALSE;

  if (selection_num != 0)
     return FALSE;

  entry = GTK_ENTRY (widget);
  gtk_editable_get_selection_bounds (GTK_EDITABLE (entry), &select_start, 
                                     &select_end);

  if (select_start != select_end)
    {
     /* Setting the start & end of the selected region to the caret position
      * turns off the selection.
      */
      caret_pos = gtk_editable_get_position (GTK_EDITABLE (entry));
      gtk_editable_select_region (GTK_EDITABLE (entry), caret_pos, caret_pos);
      return TRUE;
    }
  else
    return FALSE;
}

static gboolean
gail_entry_set_selection (AtkText *text,
			  gint	  selection_num,
                          gint    start_pos,
                          gint    end_pos)
{
  GtkEntry *entry;
  GtkWidget *widget;
  gint select_start, select_end;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return FALSE;

 /* Only let the user move the selection if one is set, and if the
  * selection_num is 0
  */
  if (selection_num != 0)
     return FALSE;

  entry = GTK_ENTRY (widget);

  gtk_editable_get_selection_bounds (GTK_EDITABLE (entry), &select_start, 
                                     &select_end);

  if (select_start != select_end)
    {
      gtk_editable_select_region (GTK_EDITABLE (entry), start_pos, end_pos);
      return TRUE;
    }
  else
    return FALSE;
}

static void
atk_editable_text_interface_init (AtkEditableTextIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->set_text_contents = gail_entry_set_text_contents;
  iface->insert_text = gail_entry_insert_text;
  iface->copy_text = gail_entry_copy_text;
  iface->cut_text = gail_entry_cut_text;
  iface->delete_text = gail_entry_delete_text;
  iface->paste_text = gail_entry_paste_text;
  iface->set_run_attributes = NULL;
}

static void
gail_entry_set_text_contents (AtkEditableText *text,
                              const gchar     *string)
{
  GtkEntry *entry;
  GtkWidget *widget;
  GtkEditable *editable;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return;

  entry = GTK_ENTRY (widget);
  editable = GTK_EDITABLE (entry);
  if (!gtk_editable_get_editable (editable))
    return;

  gtk_entry_set_text (entry, string);
}

static void
gail_entry_insert_text (AtkEditableText *text,
                        const gchar     *string,
                        gint            length,
                        gint            *position)
{
  GtkEntry *entry;
  GtkWidget *widget;
  GtkEditable *editable;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return;

  entry = GTK_ENTRY (widget);
  editable = GTK_EDITABLE (entry);
  if (!gtk_editable_get_editable (editable))
    return;

  gtk_editable_insert_text (editable, string, length, position);
}

static void
gail_entry_copy_text   (AtkEditableText *text,
                        gint            start_pos,
                        gint            end_pos)
{
  GtkEntry *entry;
  GtkWidget *widget;
  GtkEditable *editable;
  gchar *str;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return;

  entry = GTK_ENTRY (widget);
  editable = GTK_EDITABLE (entry);
  str = gtk_editable_get_chars (editable, start_pos, end_pos);
  gtk_clipboard_set_text (gtk_clipboard_get (GDK_NONE), str, -1);
}

static void
gail_entry_cut_text (AtkEditableText *text,
                     gint            start_pos,
                     gint            end_pos)
{
  GtkEntry *entry;
  GtkWidget *widget;
  GtkEditable *editable;
  gchar *str;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return;

  entry = GTK_ENTRY (widget);
  editable = GTK_EDITABLE (entry);
  if (!gtk_editable_get_editable (editable))
    return;
  str = gtk_editable_get_chars (editable, start_pos, end_pos);
  gtk_clipboard_set_text (gtk_clipboard_get (GDK_NONE), str, -1);
  gtk_editable_delete_text (editable, start_pos, end_pos);
}

static void
gail_entry_delete_text (AtkEditableText *text,
                        gint            start_pos,
                        gint            end_pos)
{
  GtkEntry *entry;
  GtkWidget *widget;
  GtkEditable *editable;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return;

  entry = GTK_ENTRY (widget);
  editable = GTK_EDITABLE (entry);
  if (!gtk_editable_get_editable (editable))
    return;

  gtk_editable_delete_text (editable, start_pos, end_pos);
}

static void
gail_entry_paste_text (AtkEditableText *text,
                       gint            position)
{
  GtkWidget *widget;
  GtkEditable *editable;
  GailEntryPaste paste_struct;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return;

  editable = GTK_EDITABLE (widget);
  if (!gtk_editable_get_editable (editable))
    return;
  paste_struct.entry = GTK_ENTRY (widget);
  paste_struct.position = position;

  g_object_ref (paste_struct.entry);
  gtk_clipboard_request_text (gtk_clipboard_get(GDK_NONE),
    gail_entry_paste_received, &paste_struct);
}

static void
gail_entry_paste_received (GtkClipboard *clipboard,
		const gchar  *text,
		gpointer     data)
{
  GailEntryPaste* paste_struct = (GailEntryPaste *)data;

  if (text)
    gtk_editable_insert_text (GTK_EDITABLE (paste_struct->entry), text, -1,
       &(paste_struct->position));

  g_object_unref (paste_struct->entry);
}

/* Callbacks */

/* Note arg1 returns the character at the start of the insert.
 * arg2 returns the number of characters inserted.
 */
static void _gail_entry_insert_text_cb (GtkEntry *entry, 
                                        gchar    *arg1, 
                                        gint     arg2,
                                        gpointer arg3)
{
  AtkObject *accessible;
  GailEntry *gail_entry;
  gint *position = (gint *) arg3;

  accessible = gtk_widget_get_accessible (GTK_WIDGET (entry));
  gail_entry = GAIL_ENTRY (accessible);
  if (!gail_entry->signal_name)
    {
      gail_entry->signal_name = "text_changed::insert";
      gail_entry->position = *position;
      gail_entry->length = arg2;
    }
  /*
   * The signal will be emitted when the cursor position is updated.
   */
}

static gunichar 
gail_entry_get_character_at_offset (AtkText	         *text,
                                    gint	         offset)
{
  GtkWidget *widget;
  GtkEntry *entry;
  gchar *string;
  gchar *index;
  gunichar unichar;

  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
    /* State is defunct */
    return '\0';

  entry = GTK_ENTRY (widget);
  string = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
  if (offset >= g_utf8_strlen (string, -1))
    {
      unichar = '\0';
    }
  else
    {
      index = g_utf8_offset_to_pointer (string, offset);

      unichar = g_utf8_get_char(index);
    }

  g_free(string);
  return unichar;
}

/* Note arg1 returns the start of the delete range, arg2 returns the
 * end of the delete range if multiple characters are deleted.	
 */
static void _gail_entry_delete_text_cb (GtkEntry *entry, 
                                        gint      arg1, 
                                        gint      arg2)
{
  AtkObject *accessible;

  accessible = gtk_widget_get_accessible (GTK_WIDGET (entry));

  g_signal_emit_by_name (accessible, "text_changed::delete", arg1, arg2 - arg1);
}

static void
_gail_entry_changed_cb (GtkEntry *entry)
{
  AtkObject *accessible;
  GailEntry *gail_entry;

  accessible = gtk_widget_get_accessible (GTK_WIDGET (entry));

  gail_entry = GAIL_ENTRY (accessible);

  /*
   * Update the text in the GtkTextBuffer
   */
  gail_text_util_text_setup (gail_entry->textutil, gtk_entry_get_text (entry));
}

static gboolean 
check_for_selection_change (GailEntry   *entry,
                            GtkEntry    *gail_entry)
{
  gint pos;
  gboolean ret_val = FALSE;
 
  pos = gtk_editable_get_position (GTK_EDITABLE (gail_entry));
  if (gail_entry->current_pos != gail_entry->selection_bound)
    {
      if (gail_entry->current_pos != entry->cursor_position ||
          gail_entry->selection_bound != entry->selection_bound)
        /*
         * This check is here as this function can be called
         * for notification of selection_bound or current_pos.
         * The values of current_pos  selection_bound may be the same 
         * for both notifications and we only want to generate one
         * text_selection_changed signal.
         */
        ret_val = TRUE;
    }
  else 
    {
      /* We had a selection */
      ret_val = (entry->cursor_position != entry->selection_bound);
    }
  entry->cursor_position = gail_entry->current_pos;
  entry->selection_bound = gail_entry->selection_bound;

  return ret_val;
}

static void
atk_action_interface_init (AtkActionIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->do_action = gail_entry_do_action;
  iface->get_n_actions = gail_entry_get_n_actions;
  iface->get_description = gail_entry_get_description;
  iface->get_name = gail_entry_action_get_name;
  iface->set_description = gail_entry_set_description;
}

static gboolean
gail_entry_do_action (AtkAction *action,
                      gint      i)
{
  GtkWidget *widget;
  gboolean return_value = TRUE;

  widget = GTK_ACCESSIBLE (action)->widget;
  if (widget == NULL)
    /*
     * State is defunct
     */
    return FALSE;

  if (!GTK_WIDGET_SENSITIVE (widget) || !GTK_WIDGET_VISIBLE (widget))
    return FALSE;

  switch (i)
    {
    case 0:
      gtk_widget_activate (widget);
      break;
    default:
      return_value = FALSE;
      break;
    }
  return return_value; 
}

static gint
gail_entry_get_n_actions (AtkAction *action)
{
  return 1;
}

static G_CONST_RETURN gchar*
gail_entry_get_description (AtkAction *action,
                            gint      i)
{
  GailEntry *entry;
  G_CONST_RETURN gchar *return_value;

  entry = GAIL_ENTRY (action);
  switch (i)
    {
    case 0:
      return_value = entry->activate_description;
      break;
    default:
      return_value = NULL;
      break;
    }
  return return_value; 
}

static G_CONST_RETURN gchar*
gail_entry_action_get_name (AtkAction *action,
                            gint      i)
{
  G_CONST_RETURN gchar *return_value;

  switch (i)
    {
    case 0:
      return_value = "activate";
      break;
    default:
      return_value = NULL;
      break;
  }
  return return_value; 
}

static gboolean
gail_entry_set_description (AtkAction      *action,
                            gint           i,
                            const gchar    *desc)
{
  GailEntry *entry;
  gchar **value;

  entry = GAIL_ENTRY (action);
  switch (i)
    {
    case 0:
      value = &entry->activate_description;
      break;
    default:
      value = NULL;
      break;
    }

  if (value)
    {
      g_free (*value);
      *value = g_strdup (desc);
      return TRUE;
    }
  else
    return FALSE;
}
