/* zvt_access-- accessibility support for libzvt
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

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <atk/atk.h>
#include <gtk/gtk.h>
#include <libzvt/libzvt.h>
#include "zvt-accessible.h"
#include "zvt-accessible-factory.h"


typedef enum {
  DIRECTION_AT,
  DIRECTION_BEFORE,
  DIRECTION_AFTER
} DIRECTION;



typedef struct _ZvtAccessiblePriv ZvtAccessiblePriv;
struct _ZvtAccessiblePriv
{
  gint caret_position;

  /* Text cache variables */

  gchar *text_cache;
  gboolean text_cache_dirty;
  gint text_cache_len;

  /* Callback pointers */

  void (*real_draw_text)(void *user_data, struct vt_line *line, int row, int col, int len, int attr);
  void (*real_scroll_area)(void *user_data, int firstrow, int count, int offset, int fill);
  int  (*real_cursor_state)(void *user_data, int state);
};

static ZvtAccessiblePriv *zvt_accessible_get_private_data (ZvtAccessible *accessible);
static void
zvt_accessible_priv_refresh_text_cache (ZvtAccessiblePriv *priv, ZvtTerm *term);
static gint
_zvt_term_offset_from_xy (ZvtTerm *term,
			       gint x, gint y);
static void
_zvt_term_xy_from_offset (ZvtTerm *term,
			       gint offset,
			       gint *x, gint *y);
static gchar *
zvt_accessible_get_text_internal (AtkText *text,
				  DIRECTION direction,
				  AtkTextBoundary boundary_type,
				  gint offset,
				  gint *start_offset, gint *end_offset);
static gboolean
_zvt_term_get_attributes_at_offset (ZvtTerm *term, gint offset, gint *attr);
static void
zvt_accessible_class_init          (ZvtAccessibleClass *klass);
static void
zvt_accessible_finalize            (GObject            *object);

static void
zvt_accessible_real_initialize (AtkObject *obj,
                                gpointer  data);
static void
atk_text_interface_init (AtkTextIface *iface);
static gchar*
zvt_accessible_get_text                           (AtkText          *text,
						   gint             start_offset,
						   gint             end_offset);
static gunichar
zvt_accessible_get_character_at_offset            (AtkText          *text,
						   gint             offset);
static gchar*
zvt_accessible_get_text_after_offset              (AtkText          *text,
						   gint             offset,
						   AtkTextBoundary  boundary_type,
						   gint             *start_offset,
						   gint	            *end_offset);
static gchar*
zvt_accessible_get_text_at_offset                 (AtkText          *text,
						   gint             offset,
						   AtkTextBoundary  boundary_type,
						   gint             *start_offset,
						   gint             *end_offset);
static gchar*
zvt_accessible_get_text_before_offset             (AtkText          *text,
						   gint             offset,
						   AtkTextBoundary  boundary_type,
						   gint             *start_offset,
						   gint	            *end_offset);
static gint
zvt_accessible_get_caret_offset                   (AtkText          *text);
static void
zvt_accessible_get_character_extents              (AtkText          *text,
						   gint             offset,
						   gint             *x,
						   gint             *y,
						   gint             *width,
						   gint             *height,
						   AtkCoordType	    coords);
static AtkAttributeSet*
zvt_accessible_get_run_attributes              (AtkText	    *text,
						gint	  	    offset,
						gint             *start_offset,
						gint	 	    *end_offset);
static AtkAttributeSet*
zvt_accessible_get_default_attributes          (AtkText	    *text);
static gint
zvt_accessible_get_character_count                (AtkText          *text);
static gint
zvt_accessible_get_offset_at_point                (AtkText          *text,
						   gint             x,
						   gint             y,
						   AtkCoordType	    coords);
static gint
zvt_accessible_get_n_selections			  (AtkText          *text);
static gchar*
zvt_accessible_get_selection			  (AtkText          *text,
						   gint		    selection_num,
						   gint             *start_offset,
						   gint             *end_offset);
static gboolean
zvt_accessible_add_selection                      (AtkText          *text,
						   gint             start_offset,
						   gint             end_offset);
static gboolean
zvt_accessible_remove_selection                   (AtkText          *text,
						   gint		    selection_num);
static gboolean
zvt_accessible_set_selection                      (AtkText          *text,
						   gint		    selection_num,
						   gint             start_offset,
						   gint             end_offset);
static gboolean
zvt_accessible_set_caret_offset                   (AtkText          *text,
						   gint             offset);
static void
zvt_accessible_draw_text (void *user_data, struct vt_line *line, int row, int col, int len, int attr);
static   int  
zvt_accessible_cursor_state (void *user_data, int state);


static gpointer parent_class = NULL;
static GQuark quark_private_data = 0;

GType
zvt_accessible_get_type (void)
{
  static GType type = 0;

  if (!type)
  {
    static GTypeInfo tinfo =
    {
      0, /* class size */
      (GBaseInitFunc) NULL, /* base init */
      (GBaseFinalizeFunc) NULL, /* base finalize */
      (GClassInitFunc) zvt_accessible_class_init, /* class init */
      (GClassFinalizeFunc) NULL, /* class finalize */
      NULL, /* class data */
      0, /* instance size */
      0, /* nb preallocs */
      (GInstanceInitFunc) NULL, /* instance init */
      NULL /* value table */
    };

    static const GInterfaceInfo atk_text_info =
    {
      (GInterfaceInitFunc) atk_text_interface_init,
      (GInterfaceFinalizeFunc) NULL,
      NULL
    };

    /*
     * Figure out the size of the class and instance
     * we are deriving from
     */
    AtkObjectFactory *factory;
    GType derived_type;
    GTypeQuery query;
    GType derived_atk_type;

    derived_type = g_type_parent (ZVT_TYPE_TERM);
    factory = atk_registry_get_factory (atk_get_default_registry (), derived_type);
    derived_atk_type = atk_object_factory_get_accessible_type (factory);
    g_type_query (derived_atk_type, &query);
    tinfo.class_size = query.class_size;
    tinfo.instance_size = query.instance_size;

    type = g_type_register_static (derived_atk_type, "ZvtAccessible", &tinfo, 0);
    g_type_add_interface_static (type, ATK_TYPE_TEXT,
                                 &atk_text_info);
  }

  return type;
}



static void
zvt_accessible_real_initialize (AtkObject *obj,
                                gpointer  data)
{
  ZvtTerm *term;
  ZvtAccessible *zvt_accessible;
  ZvtAccessiblePriv *zvt_accessible_priv;

  ATK_OBJECT_CLASS (parent_class)->initialize (obj, data);

  zvt_accessible = ZVT_ACCESSIBLE (obj);
  term = ZVT_TERM (data);

  zvt_accessible_priv = zvt_accessible_get_private_data (zvt_accessible);

  /* Initialize text cache variables */

  zvt_accessible_priv->text_cache = NULL;
  zvt_accessible_priv->text_cache_dirty = TRUE;

  zvt_accessible_priv->real_draw_text = term->vx->draw_text;
  term->vx->draw_text = zvt_accessible_draw_text;
  zvt_accessible_priv->real_cursor_state = term->vx->cursor_state;
  term->vx->cursor_state = zvt_accessible_cursor_state;
}



AtkObject *
zvt_accessible_new(GtkWidget *widget)
{
  GObject *object;
  AtkObject *accessible;
#if 0
  ZvtTerm *term;
  ZvtAccessible *zvt_accessible;
  ZvtAccessiblePriv *zvt_accessible_priv;
#endif

  object = g_object_new (ZVT_TYPE_ACCESSIBLE, NULL);

  accessible = ATK_OBJECT (object);
  atk_object_initialize (accessible, widget);
  accessible->role = ATK_ROLE_TERMINAL;

  return accessible;
}

ZvtAccessiblePriv *
zvt_accessible_get_private_data (ZvtAccessible *accessible)
{
  ZvtAccessiblePriv *private_data;

  private_data = g_object_get_qdata (G_OBJECT (accessible),
                                     quark_private_data);
  if (!private_data)
    {
      private_data = g_new (ZvtAccessiblePriv, 1);
      g_object_set_qdata (G_OBJECT (accessible),
                          quark_private_data,
                          private_data);
    }
  return private_data;
}



static gboolean
accessible_vt_in_wordclass (struct _vtx *vx, gchar ch)
{
  return (vx->wordclass[ch>>3]&(1<<(ch&7)))!=0;
}



static void
zvt_accessible_priv_refresh_text_cache (ZvtAccessiblePriv *priv, ZvtTerm *term)
{
  if (priv->text_cache_dirty)
    {
      if (priv->text_cache)
	g_free (priv->text_cache);
      priv->text_cache = zvt_term_get_buffer (term, &(priv->text_cache_len),
					VT_SELTYPE_CHAR, 0, 0,
					      10000, 10000);
      priv->text_cache_dirty = FALSE;
    }
}



static gint
_zvt_term_offset_from_xy (ZvtTerm *term,
			       gint x, gint y)
{
  struct vt_line *current;
  gint offset = 0;
  gint cur_y = 0;
  gint line_width;;
#if 0
  gint line_num = 0;
  gint cur_x = 0;
#endif
  
  current = (struct vt_line *) vt_list_index (&term->vx->vt.lines, 0);

  while (current && current->next && cur_y <= y)
    {
      line_width = current->width;

      /* FIXME: take a look at this */
      while (line_width > 0 && !(current->data[--line_width] & VTATTR_DATAMASK))
	      ;
      line_width++;
      if (cur_y < y)
	{
	  offset += line_width+1;
	  cur_y++;
	}
      else
	{
	  offset += x;
	  break;
	}
      current = current->next;
    }
  return offset;
}



static void
_zvt_term_xy_from_offset (ZvtTerm *term,
			       gint offset, gint *x, gint *y)
{
  struct vt_line *current;
  gint cur_offset = 0;
  gint cur_x = 0, cur_y = 0;
  gint line_width;
  
  current = (struct vt_line *) vt_list_index (&term->vx->vt.lines, 0);

  while (current && current->next && cur_offset < offset)
    {
      line_width = current->width;
      while (line_width > 0 && !(current->data[--line_width] & VTATTR_DATAMASK));
      line_width++;
      if (cur_offset < offset-line_width)
	{
	  if (cur_offset + line_width == offset)
	    {
	      cur_x = cur_y = -1;
	      break;
	    }
	  cur_offset += line_width+1;
	  cur_y++;
	}
      else
	{
	  cur_x += (offset-cur_offset);
	  cur_offset = offset;
	}
      current = current->next;
    }
  *x = cur_x;
  *y = cur_y;
}



static gchar *
zvt_accessible_get_text_internal (AtkText *text,
				  DIRECTION direction,
				  AtkTextBoundary boundary_type,
				  gint offset,
				  gint *start_offset, gint *end_offset)
{
  GtkWidget *widget;
  ZvtAccessible *accessible;
  ZvtAccessiblePriv *priv;
  ZvtTerm *term;
  gint x, y;
  gchar *rv;
  
  g_return_val_if_fail (ZVT_IS_ACCESSIBLE(text), NULL);
  accessible = ZVT_ACCESSIBLE(text);
  widget = GTK_ACCESSIBLE(accessible)->widget;
  g_return_val_if_fail (widget, NULL);
  term = ZVT_TERM(widget);
  priv = zvt_accessible_get_private_data (accessible);
  zvt_accessible_priv_refresh_text_cache (priv, term);

  switch (boundary_type)
    {
    case ATK_TEXT_BOUNDARY_CHAR:
      if (direction == DIRECTION_BEFORE)
	offset--;
      else if (direction == DIRECTION_AFTER)
	offset++;
      if (offset >= 0 && offset < priv->text_cache_len)
	{
	  rv = g_strndup (priv->text_cache+offset, 1);
	  *start_offset = offset;
	  *end_offset = offset+1;
	}
      else
	{
	  rv = g_strdup ("");
	  *start_offset = *end_offset = -1;
	}
      break;
    case ATK_TEXT_BOUNDARY_WORD_START:
    case ATK_TEXT_BOUNDARY_WORD_END:
      rv = priv->text_cache+offset;
      if (direction == DIRECTION_BEFORE || direction == DIRECTION_AT)
	{
	  
	  /* Find beginning of word  containing offset */

	  while (rv > priv->text_cache &&
		 accessible_vt_in_wordclass (term->vx, *rv))
	    rv--;
	  if (!accessible_vt_in_wordclass(term->vx, *rv))
	    rv++;
	}
      else
	{
  
	  /* Find beginning of word following offset */

	  while (*rv &&
		 accessible_vt_in_wordclass (term->vx, *rv))
	    rv++;
	  while (*rv &&
		 !accessible_vt_in_wordclass (term->vx, *rv))
	    rv++;
	  if (!*rv)
	    {
	      rv = g_strdup ("");
	      *start_offset = *end_offset = -1;
	      break;
	    }
	}
      if (direction == DIRECTION_BEFORE)
	{
	  rv--;
	  while (rv > priv->text_cache &&
		 !accessible_vt_in_wordclass (term->vx, *rv))
	    rv--;
	  if (rv <= priv->text_cache)
	  {
	    rv = g_strdup ("");
	    *start_offset = *end_offset = -1;
	    break;
	  }
	  while (rv > priv->text_cache &&
		 accessible_vt_in_wordclass (term->vx, *rv))
	    rv--;
	}
      *start_offset = rv-priv->text_cache;
      while (*rv &&
	     accessible_vt_in_wordclass(term->vx, *rv))
	rv++;
      *end_offset = rv-priv->text_cache;
      rv = g_strndup (priv->text_cache+*start_offset, *end_offset-*start_offset);
      break;
    case ATK_TEXT_BOUNDARY_LINE_START:
    case ATK_TEXT_BOUNDARY_LINE_END:
      _zvt_term_xy_from_offset (term, offset, &x, &y);
      if (direction == DIRECTION_BEFORE)
	y--;
      else if (direction == DIRECTION_AFTER)
	y++;
      if (y < 0)
	{
	  rv = g_strdup ("");
	  *start_offset = *end_offset = -1;
	  break;
	}
      rv = zvt_term_get_buffer (term, end_offset, VT_SELTYPE_LINE,
				0, y, term->grid_width, y);
      *start_offset = _zvt_term_offset_from_xy (term, 0, y);
      *end_offset += *start_offset;
      break;
    default:
      rv = g_strdup ("");
      *start_offset = -1;
      *end_offset = -1;
      break;
    }
  return rv;
}



static gboolean
_zvt_term_get_attributes_at_offset (ZvtTerm *term, gint offset, gint *attr)
{
  gint x, y;
  
  _zvt_term_xy_from_offset (term, offset, &x, &y);
  if (x == -1 || y == -1)
    return FALSE;
  *attr = vt_get_attr_at (term->vx, x, y);
  return TRUE;
}



static AtkAttributeSet *
zvt_accessible_get_attribute_set (gint attr)
{
  AtkAttributeSet *set = NULL;

  if (attr & VTATTR_BOLD)
    {
      AtkAttribute *at = g_new (AtkAttribute, 1);
      at->name = g_strdup ("bold");
      at->value = g_strdup ("true");
      g_slist_append (set, at);
    }
  if (attr & VTATTR_UNDERLINE)
    {
      AtkAttribute *at = g_new (AtkAttribute, 1);
      at->name = g_strdup ("underline");
      at->value = g_strdup ("true");
      g_slist_append (set, at);
    }
  if (attr & VTATTR_BLINK)
    {
      AtkAttribute *at = g_new (AtkAttribute, 1);
      at->name = g_strdup ("blink");
      at->value = g_strdup ("true");
      g_slist_append (set, at);
    }
  if (attr & VTATTR_REVERSE)
    {
      AtkAttribute *at = g_new (AtkAttribute, 1);
      at->name = g_strdup ("reverse");
      at->value = g_strdup ("true");
      g_slist_append (set, at);
    }
  if (attr & VTATTR_CONCEALED)
    {
      AtkAttribute *at = g_new (AtkAttribute, 1);
      at->name = g_strdup ("concealed");
      at->value = g_strdup ("true");
      g_slist_append (set, at);
    }
  return set;
}



static void
zvt_accessible_class_init (ZvtAccessibleClass *klass)
{
  AtkObjectClass *class = ATK_OBJECT_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  class->initialize = zvt_accessible_real_initialize;

  gobject_class->finalize = zvt_accessible_finalize;

  quark_private_data = g_quark_from_static_string ("zvt-accessible-private-data");
}

static void
zvt_accessible_finalize (GObject *object)
{
  ZvtAccessible *accessible;
  ZvtAccessiblePriv *accessible_priv;

  accessible = ZVT_ACCESSIBLE (object);
  accessible_priv = zvt_accessible_get_private_data (accessible);
  if (accessible_priv)
    {
      if (accessible_priv->text_cache)
	g_free (accessible_priv->text_cache);      
      g_free (accessible_priv);
      g_object_set_qdata (G_OBJECT (accessible),
                          quark_private_data,
                          NULL);
    }
  G_OBJECT_CLASS (parent_class)->finalize (object); 
}

static void
atk_text_interface_init (AtkTextIface *iface)
{
  g_return_if_fail (iface != NULL);
  iface->get_text = zvt_accessible_get_text;
  iface->get_character_at_offset = zvt_accessible_get_character_at_offset;
  iface->get_text_before_offset = zvt_accessible_get_text_before_offset;
  iface->get_text_at_offset = zvt_accessible_get_text_at_offset;
  iface->get_text_after_offset = zvt_accessible_get_text_after_offset;
  iface->get_character_count = zvt_accessible_get_character_count;
  iface->get_caret_offset = zvt_accessible_get_caret_offset;
  iface->set_caret_offset = zvt_accessible_set_caret_offset;
  iface->get_n_selections = zvt_accessible_get_n_selections;
  iface->get_selection = zvt_accessible_get_selection;
  iface->add_selection = zvt_accessible_add_selection;
  iface->remove_selection = zvt_accessible_remove_selection;
  iface->set_selection = zvt_accessible_set_selection;
  iface->get_character_extents = zvt_accessible_get_character_extents;
  iface->get_offset_at_point = zvt_accessible_get_offset_at_point;
  iface->get_run_attributes = zvt_accessible_get_run_attributes;
  iface->get_default_attributes = zvt_accessible_get_default_attributes;
}



static gchar*
zvt_accessible_get_text                           (AtkText          *text,
						   gint             start_offset,
						   gint             end_offset)
{
  GtkWidget *widget;
  ZvtTerm *term;
  ZvtAccessible *accessible;
  ZvtAccessiblePriv *priv;
  gchar *rv;

  g_return_val_if_fail (ZVT_IS_ACCESSIBLE(text), NULL);
  accessible = ZVT_ACCESSIBLE (text);
  widget = GTK_ACCESSIBLE(accessible)->widget;
  g_return_val_if_fail (widget, NULL);
  term = ZVT_TERM (widget);
  priv = zvt_accessible_get_private_data (accessible);

  zvt_accessible_priv_refresh_text_cache (priv, term);
  if (priv->text_cache_len == 0 ||
      priv->text_cache_len < start_offset)
    return g_strdup ("");

  if (end_offset > priv->text_cache_len)
    end_offset = priv->text_cache_len;
  
  rv = g_strndup ((priv->text_cache)+start_offset, (end_offset-start_offset));
  return rv;
}



static gunichar
zvt_accessible_get_character_at_offset            (AtkText          *text,
						   gint             offset)
{
  GtkWidget *widget;
  ZvtTerm *term;
  ZvtAccessible *accessible;
  ZvtAccessiblePriv *priv;
#if 0
  gchar *rv;
#endif

  g_return_val_if_fail (ZVT_IS_ACCESSIBLE(text), 0);
  accessible = ZVT_ACCESSIBLE (text);
  widget = GTK_ACCESSIBLE(accessible)->widget;
  g_return_val_if_fail (widget, 0);
  term = ZVT_TERM (widget);
  priv = zvt_accessible_get_private_data (accessible);

  zvt_accessible_priv_refresh_text_cache (priv, term);
  if (priv->text_cache_len == 0 ||
      priv->text_cache_len < offset)
    return 0;
  return *(priv->text_cache+offset);
}



static gchar*
zvt_accessible_get_text_after_offset              (AtkText          *text,
						   gint             offset,
						   AtkTextBoundary  boundary_type,
						   gint             *start_offset,
						   gint	            *end_offset)
{
  return zvt_accessible_get_text_internal (text,
					   DIRECTION_AFTER, boundary_type, offset,
					   start_offset, end_offset);
}



static gchar*
zvt_accessible_get_text_at_offset                 (AtkText          *text,
						   gint             offset,
						   AtkTextBoundary  boundary_type,
						   gint             *start_offset,
						   gint             *end_offset)
{
  return zvt_accessible_get_text_internal (text,
					   DIRECTION_AT, boundary_type, offset,
					   start_offset, end_offset);
}



static gchar*
zvt_accessible_get_text_before_offset             (AtkText          *text,
						   gint             offset,
						   AtkTextBoundary  boundary_type,
						   gint             *start_offset,
						   gint	            *end_offset)
{
  return zvt_accessible_get_text_internal (text,
					   DIRECTION_BEFORE, boundary_type, offset,
					   start_offset, end_offset);
}



static gint
zvt_accessible_get_caret_offset                   (AtkText          *text)
{
  ZvtAccessible *accessible;
  GtkWidget *widget;
  ZvtTerm *term;

  g_return_val_if_fail (ZVT_IS_ACCESSIBLE(text), -1);
  accessible = ZVT_ACCESSIBLE(text);
  widget = GTK_ACCESSIBLE(accessible)->widget;
  g_return_val_if_fail (widget, -1);
  term = ZVT_TERM (widget);
  return _zvt_term_offset_from_xy (term,
					term->vx->vt.cursorx, term->vx->vt.cursory);
}



static void
zvt_accessible_get_character_extents              (AtkText          *text,
						   gint             offset,
						   gint             *x,
						   gint             *y,
						   gint             *width,
						   gint             *height,
						   AtkCoordType	    coords)
{
  ZvtAccessible *accessible;
  GtkWidget *widget;
  ZvtTerm *term;
  AtkComponent *component;
  gint base_x, base_y;
  
  g_return_if_fail (ZVT_IS_ACCESSIBLE(text));
  accessible = ZVT_ACCESSIBLE(text);
  widget = GTK_ACCESSIBLE(accessible)->widget;
  term = ZVT_TERM (widget);
  component = ATK_COMPONENT(accessible);
  
  atk_component_get_position (component, &base_x, &base_y, coords);
  _zvt_term_xy_from_offset (term, offset, x, y);
  *x *= term->charwidth;
  *y *= term->charheight;
  *height = term->charheight;
  *width = term->charwidth;
  *x += base_x;
  *y += base_y;
  return;
}



static AtkAttributeSet*
zvt_accessible_get_run_attributes              (AtkText	    *text,
						gint	  	    offset,
						gint             *start_offset,
						gint	 	    *end_offset)
{
  GtkWidget *widget;
  ZvtTerm *term;
  ZvtAccessible *accessible;
  ZvtAccessiblePriv *priv;
#if 0
  gint cur_offset = offset;
#endif
  gint cur_attr = -1, prev_attr = -1;
  gboolean rv = FALSE;
  
  g_return_val_if_fail (ZVT_IS_ACCESSIBLE(text), NULL);
  accessible = ZVT_ACCESSIBLE(text);
  widget = GTK_ACCESSIBLE(accessible)->widget;
  g_return_val_if_fail (widget, NULL);
  term = ZVT_TERM (GTK_ACCESSIBLE(accessible)->widget);
  priv = zvt_accessible_get_private_data (accessible);
  
  zvt_accessible_priv_refresh_text_cache (priv, term);

  /* Find beginning of attribute run */
  *start_offset = offset;
  while (*start_offset >= 0 && (cur_attr == prev_attr || !rv))
    {
      prev_attr = cur_attr;
      rv = _zvt_term_get_attributes_at_offset (term, (*start_offset)--, &cur_attr);
      if (prev_attr == -1)
	prev_attr = cur_attr;
    }
  (*start_offset)++;
  
  /* Find end of attribute run */

  cur_attr = prev_attr;
  *end_offset = offset+1;
  rv = FALSE;
  while (*end_offset < priv->text_cache_len && (cur_attr == prev_attr || !rv))
    {
      prev_attr = cur_attr;
      rv = _zvt_term_get_attributes_at_offset (term, (*end_offset)++, &cur_attr);
    }
  (*end_offset)--;
  return zvt_accessible_get_attribute_set (prev_attr);
}



static AtkAttributeSet*
zvt_accessible_get_default_attributes          (AtkText	    *text)
{
  return NULL;
}



static gint
zvt_accessible_get_character_count                (AtkText          *text)
{
  GtkWidget *widget;
  ZvtTerm *term;
  ZvtAccessible *accessible;
  ZvtAccessiblePriv *priv;
  
  g_return_val_if_fail (ZVT_IS_ACCESSIBLE(text), -1);
  accessible = ZVT_ACCESSIBLE (text);
  priv = zvt_accessible_get_private_data (accessible);
  widget = GTK_ACCESSIBLE(accessible)->widget;
  g_return_val_if_fail (widget, -1);
  term = ZVT_TERM (widget);
  zvt_accessible_priv_refresh_text_cache (priv, term);
  return priv->text_cache_len;
}



static gint
zvt_accessible_get_offset_at_point                (AtkText          *text,
						   gint             x,
						   gint             y,
						   AtkCoordType	    coords)
{
  GtkWidget *widget;
  ZvtAccessible *accessible;
  ZvtTerm *term;
  AtkComponent *component;
  gint base_x, base_y;
  
  g_return_val_if_fail (ZVT_IS_ACCESSIBLE(text), -1);
  accessible = ZVT_ACCESSIBLE(text);
  widget = GTK_ACCESSIBLE (accessible)->widget;
  g_return_val_if_fail (widget, -1);
  term = ZVT_TERM (widget);
  component = ATK_COMPONENT(accessible);
  
  atk_component_get_position (component, &base_x, &base_y, coords);
  x -= base_x;
  y -= base_y;
  x /= term->charwidth;
  y /= term->charheight;
  return _zvt_term_offset_from_xy (term, x, y);
}




static gint
zvt_accessible_get_n_selections			  (AtkText          *text)
{
  GtkWidget *widget;
  ZvtTerm *term;

  g_return_val_if_fail (GTK_IS_ACCESSIBLE(text), -1);
  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
  {
    /* State is defunct */
    return -1;
  }

  g_return_val_if_fail (ZVT_IS_TERM(widget), -1);
  term = ZVT_TERM(widget);
  return (term->vx->selected) ? 1 : 0;
}



static gchar*
zvt_accessible_get_selection			  (AtkText          *text,
						   gint		    selection_num,
						   gint             *start_offset,
						   gint             *end_offset)
{
  GtkWidget *widget;
  ZvtTerm *term;
  ZvtAccessible *accessible;

  g_return_val_if_fail (selection_num > 0, NULL);
  g_return_val_if_fail (ZVT_IS_ACCESSIBLE(text), NULL);
  accessible = ZVT_ACCESSIBLE(text);
  widget = GTK_ACCESSIBLE(accessible)->widget;
  g_return_val_if_fail (widget, NULL);
  term = ZVT_TERM(widget);
  g_return_val_if_fail (term->vx->selected, NULL);  

  return zvt_term_get_buffer (term, NULL, term->vx->selectiontype,
			      term->vx->selstartx, term->vx->selstarty,
			      term->vx->selendx, term->vx->selendy);
}



static gboolean
zvt_accessible_add_selection                      (AtkText          *text,
						   gint             start_offset,
						   gint             end_offset)
{
  GtkWidget *widget;
  ZvtTerm *term;
  ZvtAccessible *accessible;
  gint startx, starty, endx, endy;
  VT_SELTYPE type;
  
  g_return_val_if_fail (ZVT_IS_ACCESSIBLE(text), FALSE);
  accessible = ZVT_ACCESSIBLE(text);
  widget = GTK_ACCESSIBLE(accessible)->widget;
  g_return_val_if_fail (widget, FALSE);
  term = ZVT_TERM(widget);
  g_return_val_if_fail (!term->vx->selected, FALSE);  

  _zvt_term_xy_from_offset (term, start_offset, &startx, &starty);
  _zvt_term_xy_from_offset (term, end_offset, &endx, &endy);
  if (start_offset == end_offset+1 || start_offset == end_offset-1)
    type = VT_SELTYPE_CHAR;
  else if (starty != endy)
    type = VT_SELTYPE_LINE;
  else
    type = VT_SELTYPE_WORD;

  term->vx->selectiontype = type;
  term->vx->selected = TRUE;
  term->vx->selstartx = startx;
  term->vx->selstarty = starty;
  term->vx->selendx = endx;
  term->vx->selendy = endy;
  vt_draw_selection (term->vx);
  return TRUE;
}



static gboolean
zvt_accessible_remove_selection                   (AtkText          *text,
						   gint		    selection_num)
{
  GtkWidget *widget;
  ZvtTerm *term;

  g_return_val_if_fail (GTK_IS_ACCESSIBLE(text), FALSE);
  widget = GTK_ACCESSIBLE (text)->widget;
  if (widget == NULL)
  {
    /* State is defunct */
    return FALSE;
  }

  g_return_val_if_fail (ZVT_IS_TERM(widget), FALSE);
  term = ZVT_TERM(widget);
  if (!term->vx->selected || selection_num != 0)
    return FALSE;
  term->vx->selected = 0;
  return TRUE;
}



static gboolean
zvt_accessible_set_selection                      (AtkText          *text,
						   gint		    selection_num,
						   gint             start_offset,
						   gint             end_offset)
{
  GtkWidget *widget;
  ZvtAccessible *accessible;
  ZvtTerm *term;

  g_return_val_if_fail (selection_num > 0, FALSE);
  g_return_val_if_fail (ZVT_IS_ACCESSIBLE(text), FALSE);
  accessible = ZVT_ACCESSIBLE(text);
  widget = GTK_ACCESSIBLE(accessible)->widget;
  g_return_val_if_fail (widget, FALSE);
  term = ZVT_TERM(widget);
  term->vx->selected = FALSE;
  return zvt_accessible_add_selection (text, start_offset, end_offset);
}



static gboolean
zvt_accessible_set_caret_offset                   (AtkText          *text,
						   gint             offset)
{
  return FALSE;
}



static void
zvt_accessible_draw_text (void *user_data, struct vt_line *line, int row, int col, int len, int attr)
{
  GtkWidget *widget;
  AtkObject *accessible;
  ZvtAccessible *zvt_accessible;
  ZvtAccessiblePriv *zvt_accessible_priv;
  ZvtTerm *term;
  gint offset;
  gchar *old_text, *updated_text;
  
  widget = user_data;
  g_return_if_fail (GTK_IS_WIDGET(widget));
  g_return_if_fail (ZVT_IS_TERM(widget));
  term = ZVT_TERM (widget);
  accessible = gtk_widget_get_accessible (widget);
  g_return_if_fail (ZVT_IS_ACCESSIBLE(accessible));
  zvt_accessible = ZVT_ACCESSIBLE(accessible);
  zvt_accessible_priv = zvt_accessible_get_private_data (zvt_accessible);
  if (zvt_accessible_priv->real_draw_text == NULL)
    {
      /* We're hosed!!! */
      return;
    }

  /* Note that our cache is dirty */

  zvt_accessible_priv->text_cache_dirty = TRUE;

  updated_text = zvt_term_get_buffer (term, NULL, VT_SELTYPE_CHAR,
				  col, row, col+len, row);
  
  len = strlen (updated_text);  
  offset = _zvt_term_offset_from_xy (term, col, row);

  /* Get the text currently displayed at the update position */

  if (!zvt_accessible_priv->text_cache || offset > zvt_accessible_priv->text_cache_len)
    old_text = g_strdup ("");
  else
    old_text = g_strndup ((zvt_accessible_priv->text_cache)+offset, len);
  
  if (strcmp (old_text, updated_text))
    g_signal_emit_by_name (accessible, "text_changed::insert",
			   offset, len);
  g_free (old_text);
  g_free (updated_text);

  zvt_accessible_priv->real_draw_text (user_data,
				  line, row, col, len, attr);
  
  return;
}



static   int  
zvt_accessible_cursor_state (void *user_data, int state)
{
  GtkWidget *widget;
  ZvtTerm *term;
  AtkObject *accessible;
  ZvtAccessible *zvt_accessible;
  ZvtAccessiblePriv *zvt_accessible_priv;
  gint offset;

  widget = user_data;
  g_return_val_if_fail (GTK_IS_WIDGET(widget), 0);
  g_return_val_if_fail (ZVT_IS_TERM(widget), 0);
  term = ZVT_TERM(widget);
  accessible = gtk_widget_get_accessible (widget);
  g_return_val_if_fail (ZVT_IS_ACCESSIBLE(accessible), 0);
  zvt_accessible = ZVT_ACCESSIBLE(accessible);
  zvt_accessible_priv = zvt_accessible_get_private_data (zvt_accessible);
  offset = _zvt_term_offset_from_xy (term,
					  term->vx->vt.cursorx, term->vx->vt.cursory);
  					  
  if (offset != zvt_accessible_priv->caret_position)
    {
      zvt_accessible_priv->caret_position = offset;
      g_signal_emit_by_name (accessible, "text_caret_moved",
			     offset);
    }
  if (zvt_accessible_priv->real_cursor_state)
    return zvt_accessible_priv->real_cursor_state (user_data, state);
  return 0;
}
