/* $Id: keyboard-drawing.c,v 1.1.1.2 2005-03-10 17:48:23 ghudson Exp $ */
/*
 * keyboard-drawing.c: implementation of a gtk+ widget that is a drawing of
 * the keyboard of the default display
 *
 * Copyright (c) 2004 Noah Levitt
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#include "config.h"
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <pango/pangoxft.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBgeom.h>
#include <stdlib.h>
#include <math.h>
#include "keyboard-drawing.h"
#include "keyboard-marshal.h"

enum 
{
  BAD_KEYCODE = 0,
  NUM_SIGNALS
};

static guint keyboard_drawing_signals[NUM_SIGNALS] = { 0 };

static gint
xkb_to_pixmap_coord (KeyboardDrawing *drawing, 
                     gint             n)
{
  return n * drawing->scale_numerator / drawing->scale_denominator;
}

/* angle is in tenths of a degree; coordinates can be anything as (xkb,
 * pixels, pango) as long as they are all the same */
static void
rotate_coordinate (gint  origin_x,
                   gint  origin_y,
                   gint  x,
                   gint  y, 
                   gint  angle, 
                   gint *rotated_x, 
                   gint *rotated_y)
{
  *rotated_x = origin_x + (x - origin_x) * cos (M_PI * angle / 1800.0) - (y - origin_y) * sin (M_PI * angle / 1800.0);
  *rotated_y = origin_y + (x - origin_x) * sin (M_PI * angle / 1800.0) + (y - origin_y) * cos (M_PI * angle / 1800.0);
}

static void
draw_polygon (KeyboardDrawing *drawing,
              GdkColor        *fill_color,
              gint             xkb_x,
              gint             xkb_y, 
              XkbPointRec     *xkb_points, 
              guint            num_points)
{
  GtkStateType state = GTK_WIDGET_STATE (GTK_WIDGET (drawing));
  GdkGC *gc;
  GdkPoint *points;
  gboolean filled;
  gint i;
  
  if (drawing->pixmap == NULL) 
    return;

  if (fill_color) 
    {
      gc = gdk_gc_new (GTK_WIDGET (drawing)->window);
      gdk_gc_set_rgb_fg_color (gc, fill_color);
      filled = TRUE;
    } 
  else 
    {
      gc = GTK_WIDGET (drawing)->style->dark_gc[state];
      filled = FALSE;
    }

  points = g_new (GdkPoint, num_points);

  for (i = 0; i < num_points; i++) 
    {
      points[i].x = xkb_to_pixmap_coord (drawing, xkb_x + xkb_points[i].x);
      points[i].y = xkb_to_pixmap_coord (drawing, xkb_y + xkb_points[i].y);
    }

  gdk_draw_polygon (drawing->pixmap, gc, filled, points, num_points);

  g_free (points);
  if (fill_color)
    g_object_unref (gc);
}

/* x, y, width, height are in the xkb coordinate system */
static void
draw_rectangle (KeyboardDrawing *drawing,
                GdkColor        *fill_color,
                gint             angle,
                gint             xkb_x, 
                gint             xkb_y, 
                gint             xkb_width, 
                gint             xkb_height)
{
  if (drawing->pixmap == NULL)
    return;

  if (angle == 0) 
    {
      GtkStateType state = GTK_WIDGET_STATE (GTK_WIDGET (drawing));
      gint x, y, width, height;
      gboolean filled;
      GdkGC *gc;

      if (fill_color) 
        {
          gc = gdk_gc_new (GTK_WIDGET (drawing)->window);
          gdk_gc_set_rgb_fg_color (gc, fill_color);
          filled = TRUE;
        } 
      else 
        {
          gc = GTK_WIDGET (drawing)->style->dark_gc[state];
          filled = FALSE;
        }

      x = xkb_to_pixmap_coord (drawing, xkb_x);
      y = xkb_to_pixmap_coord (drawing, xkb_y);
      width = xkb_to_pixmap_coord (drawing, xkb_x + xkb_width) - x;
      height = xkb_to_pixmap_coord (drawing, xkb_y + xkb_height) - y;

      gdk_draw_rectangle (drawing->pixmap, gc, filled, x, y, width, height);

      if (fill_color)
        g_object_unref (gc);
    } 
  else 
    {
      XkbPointRec points[4];

      points[0].x = xkb_x;
      points[0].y = xkb_y;
      rotate_coordinate (xkb_x, xkb_y, xkb_x + xkb_width, xkb_y, angle, 
                        (gint *) &points[1].x, (gint *) &points[1].y);
      rotate_coordinate (xkb_x, xkb_y, xkb_x + xkb_width, xkb_y + xkb_height, angle,
                         (gint *) &points[2].x, (gint *) &points[2].y);
      rotate_coordinate (xkb_x, xkb_y, xkb_x, xkb_y + xkb_height, angle, 
                         (gint *) &points[3].x, (gint *) &points[3].y);

      /* the points we've calculated are relative to 0,0 */
      draw_polygon (drawing, fill_color, 0, 0, points, 4);
    }
}

static void
draw_outline (KeyboardDrawing *drawing,
              XkbOutlineRec   *outline,
              GdkColor        *color, 
              gint             angle, 
              gint             origin_x, 
              gint             origin_y)
{
  if (outline->num_points == 1) 
    {
      if (color)
        draw_rectangle (drawing, color, angle, origin_x, origin_y, 
                        outline->points[0].x, outline->points[0].y);

      draw_rectangle (drawing, NULL, angle, origin_x, origin_y,
                      outline->points[0].x, outline->points[0].y);
    } 
  else if (outline->num_points == 2) 
    {
      gint rotated_x0, rotated_y0;

      rotate_coordinate (origin_x, origin_y,
                         origin_x + outline->points[0].x,
                         origin_y + outline->points[0].y,
                         angle, 
                         &rotated_x0, &rotated_y0);
      if (color)
        draw_rectangle (drawing, color, angle, rotated_x0, rotated_y0,
                        outline->points[1].x, outline->points[1].y);

      draw_rectangle (drawing, NULL, angle, rotated_x0, rotated_y0,
                      outline->points[1].x, outline->points[1].y);
    } 
  else 
    {
      if (color)
        draw_polygon (drawing, color, origin_x, origin_y, outline->points, outline->num_points);
      
      draw_polygon (drawing, NULL, origin_x, origin_y, outline->points, outline->num_points);
    }
}

/* see PSColorDef in xkbprint */
static gboolean
parse_xkb_color_spec (gchar    *colorspec, 
                      GdkColor *color)
{
  glong level;

  if (g_ascii_strcasecmp (colorspec, "black") == 0) 
    {
      color->red = 0;
      color->green = 0;
      color->blue = 0;
    } 
  else if (g_ascii_strcasecmp (colorspec, "white") == 0) 
    {
      color->red = 65535;
      color->green = 65535;
      color->blue = 65535;
    } 
  else if (g_ascii_strncasecmp (colorspec, "grey", 4) == 0 || 
           g_ascii_strncasecmp (colorspec, "gray", 4) == 0) 
    {
      level = strtol (colorspec + 4, NULL, 10);

      color->red = 65535 - 65535 * level / 100;
      color->green = 65535 - 65535 * level / 100;
      color->blue = 65535 - 65535 * level / 100;
    } 
  else if (g_ascii_strcasecmp (colorspec, "red") == 0) 
    {
      color->red = 65535;
      color->green = 0;
      color->blue = 0;
    } 
  else if (g_ascii_strcasecmp (colorspec, "green") == 0) 
    {
      color->red = 0;
      color->green = 65535;
      color->blue = 0;
    } 
  else if (g_ascii_strcasecmp (colorspec, "blue") == 0) 
    {
      color->red = 0;
      color->green = 0;
      color->blue = 65535;
    } 
  else if (g_ascii_strncasecmp (colorspec, "red", 3) == 0) 
    {
      level = strtol (colorspec + 3, NULL, 10);

      color->red = 65535 * level / 100;
      color->green = 0;
      color->blue = 0;
    } 
  else if (g_ascii_strncasecmp (colorspec, "green", 5) == 0) 
    {
      level = strtol (colorspec + 5, NULL, 10);

      color->red = 0;
      color->green = 65535 * level / 100;;
      color->blue = 0;
    } 
  else if (g_ascii_strncasecmp (colorspec, "blue", 4) == 0) 
    {
      level = strtol (colorspec + 4, NULL, 10);

      color->red = 0;
      color->green = 0;
      color->blue = 65535 * level / 100;
    } 
  else
    return FALSE;

  return TRUE;
}


static guint
find_keycode (KeyboardDrawing *drawing, 
              gchar           *key_name)
{
  guint i;

  for (i = drawing->xkb->min_key_code; i <= drawing->xkb->max_key_code; i++) 
    {
      if (drawing->xkb->names->keys[i].name[0] == key_name[0] && 
          drawing->xkb->names->keys[i].name[1] == key_name[1] && 
          drawing->xkb->names->keys[i].name[2] == key_name[2] && 
          drawing->xkb->names->keys[i].name[3] == key_name[3])
        return i;
    }

  return (guint)(-1);
}


static void
fit_width (KeyboardDrawing *drawing, 
           gint             width)
{
  PangoRectangle logical_rect;
  gint old_size;

  pango_layout_get_extents (drawing->layout, NULL, &logical_rect);

  if (logical_rect.width > 0 && logical_rect.width > width) 
    {
      old_size = pango_font_description_get_size (drawing->font_desc);
      pango_font_description_set_size (drawing->font_desc, old_size * width / logical_rect.width);
      pango_layout_set_font_description (drawing->layout, drawing->font_desc);
    }
}

static void
set_key_label_in_layout (KeyboardDrawing *drawing,
                         PangoLayout     *layout, 
                         guint            keyval)
{
  gchar buf[5];
  gunichar uc;

  switch (keyval) 
    {
    case GDK_Scroll_Lock:
      pango_layout_set_text (layout, "Scroll\nLock", -1);
      break;

    case GDK_space:
      pango_layout_set_text (layout, "", -1);
      break;

    case GDK_Sys_Req:
      pango_layout_set_text (layout, "Sys Rq", -1);
      break;

    case GDK_Page_Up:
      pango_layout_set_text (layout, "Page\nUp", -1);
      break;

    case GDK_Page_Down:
      pango_layout_set_text (layout, "Page\nDown", -1);
      break;

    case GDK_Num_Lock:
      pango_layout_set_text (layout, "Num\nLock", -1);
      break;

    case GDK_KP_Page_Up:
      pango_layout_set_text (layout, "Pg Up", -1);
      break;

    case GDK_KP_Page_Down:
      pango_layout_set_text (layout, "Pg Dn", -1);
      break;

    case GDK_KP_Home:
      pango_layout_set_text (layout, "Home", -1);
      break;

    case GDK_KP_Left:
      pango_layout_set_text (layout, "Left", -1);
      break;

    case GDK_KP_End:
      pango_layout_set_text (layout, "End", -1);
      break;

    case GDK_KP_Up:
      pango_layout_set_text (layout, "Up", -1);
      break;

    case GDK_KP_Begin:
      pango_layout_set_text (layout, "Begin", -1);
      break;

    case GDK_KP_Right:
      pango_layout_set_text (layout, "Right", -1);
      break;

    case GDK_KP_Enter:
      pango_layout_set_text (layout, "Enter", -1);
      break;

    case GDK_KP_Down:
      pango_layout_set_text (layout, "Down", -1);
      break;

    case GDK_KP_Insert:
      pango_layout_set_text (layout, "Ins", -1);
      break;

    case GDK_KP_Delete:
      pango_layout_set_text (layout, "Del", -1);
      break;

    case GDK_dead_grave:
      pango_layout_set_text (layout, "ˋ", -1);
      break;

    case GDK_dead_acute:
      pango_layout_set_text (layout, "ˊ", -1);
      break;

    case GDK_dead_circumflex:
      pango_layout_set_text (layout, "ˆ", -1);
      break;

    case GDK_dead_tilde:
      pango_layout_set_text (layout, "~", -1);
      break;

    case GDK_dead_macron:
      pango_layout_set_text (layout, "ˉ", -1);
      break;

    case GDK_dead_breve:
      pango_layout_set_text (layout, "˘", -1);
      break;

    case GDK_dead_abovedot:
      pango_layout_set_text (layout, "˙", -1);
      break;

    case GDK_dead_diaeresis:
      pango_layout_set_text (layout, "¨", -1);
      break;

    case GDK_dead_abovering:
      pango_layout_set_text (layout, "˚", -1);
      break;

    case GDK_dead_doubleacute:
      pango_layout_set_text (layout, "˝", -1);
      break;

    case GDK_dead_caron:
      pango_layout_set_text (layout, "ˇ", -1);
      break;

    case GDK_dead_cedilla:
      pango_layout_set_text (layout, "¸", -1);
      break;

    case GDK_dead_ogonek:
      pango_layout_set_text (layout, "˛", -1);
      break;

      /* case GDK_dead_iota:
       * case GDK_dead_voiced_sound:
       * case GDK_dead_semivoiced_sound: */

    case GDK_dead_belowdot:
      pango_layout_set_text (layout, " ̣", -1);
      break;

    case GDK_horizconnector:
      pango_layout_set_text (layout, "horiz\nconn", -1);
      break;

    case GDK_Mode_switch:
      pango_layout_set_text (layout, "AltGr", -1);
      break;

    case GDK_Multi_key:
      pango_layout_set_text (layout, "Compose", -1);
      break;

    default:
      uc = gdk_keyval_to_unicode (keyval);
      if (uc != 0 && g_unichar_isgraph (uc)) 
        {
          buf[g_unichar_to_utf8 (uc, buf)] = '\0';
          pango_layout_set_text (layout, buf, -1);
        } 
      else 
        {
          gchar *name = gdk_keyval_name (keyval);
          if (name)
            pango_layout_set_text (layout, name, -1);
          else
            pango_layout_set_text (layout, "", -1);
        }
    }
}


/* normally, the unshifted (level 0) value goes on the bottom, but for some
 * keys, it goes on the top */
static gboolean
is_key_flipped (KeyboardDrawing *drawing, 
                guint            keycode)
{
  gint level, group;

  for (group = 0; group < XkbKeyGroupsWidth (drawing->xkb, keycode); group++)
    for (level = 0; level < XkbKeyGroupWidth (drawing->xkb, keycode, group); level++)
      switch (XkbKeySymEntry (drawing->xkb, keycode, level, group)) 
        {
        case GDK_Escape:
        case GDK_F1:
        case GDK_F2:
        case GDK_F3:
        case GDK_F4:
        case GDK_F5:
        case GDK_F6:
        case GDK_F7:
        case GDK_F8:
        case GDK_F9:
        case GDK_F10:
        case GDK_F11:
        case GDK_F12:
        case GDK_F13:
        case GDK_F14:
        case GDK_F15:
        case GDK_F16:
        case GDK_F17:
        case GDK_F18:
        case GDK_F19:
        case GDK_F20:
        case GDK_F21:
        case GDK_F22:
        case GDK_F23:
        case GDK_F24:
        case GDK_F25:
        case GDK_F26:
        case GDK_F27:
        case GDK_F28:
        case GDK_F29:
        case GDK_F30:
        case GDK_F31:
        case GDK_F32:
        case GDK_F33:
        case GDK_F34:
        case GDK_F35:
        case GDK_Print:
        case GDK_Scroll_Lock:
        case GDK_Pause:
        case GDK_Tab:
        case GDK_Control_L:
        case GDK_Control_R:
        case GDK_Shift_L:
        case GDK_Shift_R:
        case GDK_Super_L:
        case GDK_Super_R:
        case GDK_Alt_L:
        case GDK_Alt_R:
        case GDK_Meta_L:
        case GDK_Meta_R:
        case GDK_Mode_switch:
        case GDK_Menu:
        case GDK_Return:
        case GDK_BackSpace:
        case GDK_Insert:
        case GDK_Home:
        case GDK_Page_Up:
        case GDK_Page_Down:
        case GDK_Delete:
        case GDK_End:
        case GDK_Up:
        case GDK_Down:
        case GDK_Left:
        case GDK_Right:
        case GDK_Num_Lock:
        case GDK_KP_Divide:
        case GDK_KP_Multiply:
        case GDK_KP_Subtract:
        case GDK_KP_Add:
        case GDK_KP_Enter:
          return TRUE;
        }

  return FALSE;
}

static void
substitute_func_diagonal_baseline (FcPattern *pattern, 
                                   gpointer   data)
{
  gint angle = GPOINTER_TO_INT (data);
  FcMatrix matrix;
  FcBool ret;

  FcMatrixInit (&matrix);
  FcMatrixRotate (&matrix, cos (-M_PI * angle / 1800.0), sin (-M_PI * angle / 1800.0));
  ret = FcPatternAddMatrix (pattern, FC_MATRIX, &matrix);
}

static void
draw_layout (KeyboardDrawing *drawing,
             gint             angle, 
             gint             x, 
             gint             y, 
             PangoLayout     *layout)
{
  GtkStateType state = GTK_WIDGET_STATE (GTK_WIDGET (drawing));
  PangoLayoutLine *line;
  gint x_off, y_off;
  gint i;

  if (drawing->pixmap == NULL)
    return;

  if (angle != drawing->angle) 
    {
      if (angle == 0) 
        {
          pango_xft_set_default_substitute (drawing->display, drawing->screen_num, NULL, NULL, NULL);
          pango_xft_substitute_changed (drawing->display, drawing->screen_num);
        } 
      else 
        {
          pango_xft_set_default_substitute (drawing->display, drawing->screen_num, 
                                            substitute_func_diagonal_baseline, GINT_TO_POINTER (angle), NULL);
          pango_xft_substitute_changed (drawing->display, drawing->screen_num);
        }

      pango_layout_context_changed (drawing->layout);
      drawing->angle = angle;
    }

  i = 0;
  y_off = 0;
  for (line = pango_layout_get_line (drawing->layout, i); 
       line != NULL; 
       line = pango_layout_get_line (drawing->layout, ++i)) 
    {
      GSList *runp;
      PangoRectangle line_extents;

      x_off = 0;

      for (runp = line->runs; runp != NULL; runp = runp->next) 
        {
          PangoGlyphItem *run = runp->data;
          gint j;

          for (j = 0; j < run->glyphs->num_glyphs; j++) 
            {
              PangoGlyphGeometry *geometry;
              gint xx, yy;

              geometry = &run->glyphs->glyphs[j].geometry;

              rotate_coordinate (0, 0, x_off, y_off, angle, &xx, &yy);
              geometry->x_offset -= x_off - xx;
              geometry->y_offset -= y_off - yy;

              x_off += geometry->width;
            }
        }

      pango_layout_line_get_extents (line, NULL, &line_extents);
      y_off += line_extents.height + pango_layout_get_spacing (drawing->layout);
    }

  gdk_draw_layout (drawing->pixmap, GTK_WIDGET (drawing)->style->text_gc[state], x, y, drawing->layout);
}

static void
draw_key_label (KeyboardDrawing *drawing,
                guint            keycode,
                gint             angle,
                gint             xkb_origin_x,
                gint             xkb_origin_y, 
                gint             xkb_width, 
                gint             xkb_height)
{
  gint groups[] = { drawing->leftgroup, drawing->rightgroup };
  gint levels[] = { drawing->bottomlevel, drawing->toplevel };
  gint x, y, width, height, old_size;
  gboolean flipped;
  gint padding;
  gint g, l;

  padding = 23 * drawing->scale_numerator / drawing->scale_denominator; /* 2.3mm */

  x = xkb_to_pixmap_coord (drawing, xkb_origin_x);
  y = xkb_to_pixmap_coord (drawing, xkb_origin_y);
  width = xkb_to_pixmap_coord (drawing, xkb_origin_x + xkb_width) - x;
  height = xkb_to_pixmap_coord (drawing, xkb_origin_y + xkb_height) - y;

  flipped = is_key_flipped (drawing, keycode);

  for (g = 0; g < 2; g++) 
    {
      gint label_x, label_y, label_max_width;

      if (groups[g] < 0 || groups[g] >= XkbKeyNumGroups (drawing->xkb, keycode))
        continue;

      for (l = 0; l < 2; l++) 
        {
          KeySym keysym;

          if (levels[l] < 0 || levels[l] >= XkbKeyGroupWidth (drawing->xkb, keycode, groups[g]))
            continue;

          keysym = XkbKeySymEntry (drawing->xkb, keycode, levels[l], groups[g]);

          if (keysym == 0)
            continue;

          if (groups[g] == drawing->leftgroup) 
            {
              rotate_coordinate (x, y, x + padding, 
                                 y + padding + (height - 2 * padding) * (flipped ? l : (1 - l)) * 4 / 7, 
                                 angle, &label_x, &label_y); 
              label_max_width = PANGO_SCALE * (width - 2 * padding);
            } 
          else if (groups[g] == drawing->rightgroup) 
            {
              rotate_coordinate (x, y, x + padding + (width - 2 * padding) * 4 / 7, 
                                 y + padding + (height - 2 * padding) * (flipped ? l : (1 - l)) * 4 / 7, 
                                 angle, &label_x, &label_y);
              label_max_width = PANGO_SCALE * ((width - 2 * padding) - (width - 2 * padding) * 4 / 7);
            } 
          else
            continue;

          set_key_label_in_layout (drawing, drawing->layout, keysym);

          old_size = pango_font_description_get_size (drawing->font_desc);
          fit_width (drawing, label_max_width);

          draw_layout (drawing, angle, label_x, label_y, drawing->layout);

          if (pango_font_description_get_size (drawing->font_desc) != old_size) 
            {
              pango_font_description_set_size (drawing->font_desc, old_size);
              pango_layout_set_font_description (drawing->layout, drawing->font_desc);
            }
        }
    }
}

/* groups are from 0-3 */
static void
draw_key (KeyboardDrawing    *drawing, 
          KeyboardDrawingKey *key)
{
  XkbShapeRec *shape;
  GdkColor *color;
  gint i;

  shape = &drawing->xkb->geom->shapes[key->xkbkey->shape_ndx];

  if (key->pressed)
    color = &(GTK_WIDGET (drawing)->style->base[GTK_STATE_SELECTED]);
  else
    color = &drawing->colors[key->xkbkey->color_ndx];

  for (i = 0; i < 1 /* shape->num_outlines */ ; i++)
    draw_outline (drawing, &shape->outlines[i], color, key->angle, key->origin_x, key->origin_y);

  draw_key_label (drawing, key->keycode, key->angle, key->origin_x,
                  key->origin_y, shape->bounds.x2, shape->bounds.y2);
}

static void
invalidate_region (KeyboardDrawing *drawing,
                   gdouble          angle,
                   gint             origin_x, 
                   gint             origin_y, 
                   XkbShapeRec     *shape)
{
  GdkPoint points[4];
  gint x_min, x_max, y_min, y_max;
  gint x, y, width, height;

  rotate_coordinate (0, 0, 0, 0, angle, &points[0].x, &points[0].y);
  rotate_coordinate (0, 0, shape->bounds.x2, 0, angle, &points[1].x, &points[1].y);
  rotate_coordinate (0, 0, shape->bounds.x2, shape->bounds.y2, angle, &points[2].x, &points[2].y);
  rotate_coordinate (0, 0, 0, shape->bounds.y2, angle, &points[3].x, &points[3].y);

  x_min = MIN (MIN (points[0].x, points[1].x), MIN (points[2].x, points[3].x));
  x_max = MAX (MAX (points[0].x, points[1].x), MAX (points[2].x, points[3].x));
  y_min = MIN (MIN (points[0].y, points[1].y), MIN (points[2].y, points[3].y));
  y_max = MAX (MAX (points[0].y, points[1].y), MAX (points[2].y, points[3].y));

  x = xkb_to_pixmap_coord (drawing, origin_x + x_min) - 6;
  y = xkb_to_pixmap_coord (drawing, origin_y + y_min) - 6;
  width = xkb_to_pixmap_coord (drawing, x_max - x_min) + 12;
  height = xkb_to_pixmap_coord (drawing, y_max - y_min) + 12;

  gtk_widget_queue_draw_area (GTK_WIDGET (drawing), x, y, width, height);
}

static void
invalidate_indicator_doodad_region (KeyboardDrawing       *drawing,
                                    KeyboardDrawingDoodad *doodad)
{
  invalidate_region (drawing, 
                     doodad->angle, 
                     doodad->origin_x + doodad->doodad->indicator.left,
                     doodad->origin_y + doodad->doodad->indicator.top,
                     &drawing->xkb->geom->shapes[doodad->doodad->indicator.shape_ndx]);
}

static void
invalidate_key_region (KeyboardDrawing    *drawing, 
                       KeyboardDrawingKey *key)
{
  invalidate_region (drawing, 
                     key->angle, 
                     key->origin_x, 
                     key->origin_y,
                     &drawing->xkb->geom->shapes[key->xkbkey->shape_ndx]);
}

static void
draw_text_doodad (KeyboardDrawing       *drawing,
                  KeyboardDrawingDoodad *doodad,
                  XkbTextDoodadRec      *text_doodad)
{
  gint x = xkb_to_pixmap_coord (drawing, doodad->origin_x + text_doodad->left);
  gint y = xkb_to_pixmap_coord (drawing, doodad->origin_y + text_doodad->top);

  pango_layout_set_text (drawing->layout, text_doodad->text, -1);
  draw_layout (drawing, doodad->angle, x, y, drawing->layout);
}

static void
draw_indicator_doodad (KeyboardDrawing       *drawing,
                       KeyboardDrawingDoodad *doodad,
                       XkbIndicatorDoodadRec *indicator_doodad)
{
  GdkColor *color;
  XkbShapeRec *shape;
  gint i;

  shape = &drawing->xkb->geom->shapes[indicator_doodad->shape_ndx];
  if (doodad->on)
    color = &drawing->colors[indicator_doodad->on_color_ndx];
  else
    color = &drawing->colors[indicator_doodad->off_color_ndx];

  for (i = 0; i < 1; i++)
    draw_outline (drawing, &shape->outlines[i], color, doodad->angle,
                  doodad->origin_x + indicator_doodad->left,
                  doodad->origin_y + indicator_doodad->top);
}

static void
draw_shape_doodad (KeyboardDrawing       *drawing,
                   KeyboardDrawingDoodad *doodad,
                   XkbShapeDoodadRec     *shape_doodad)
{
  XkbShapeRec *shape;
  GdkColor *color;
  gint i;

  shape = &drawing->xkb->geom->shapes[shape_doodad->shape_ndx];
  color = &drawing->colors[shape_doodad->color_ndx];

  for (i = 0; i < shape->num_outlines; i++)
    draw_outline (drawing, &shape->outlines[i], color, doodad->angle,
                  doodad->origin_x + shape_doodad->left,
                  doodad->origin_y + shape_doodad->top);
}

static void
draw_doodad (KeyboardDrawing       *drawing, 
             KeyboardDrawingDoodad *doodad)
{
  switch (doodad->doodad->any.type) 
    {
    case XkbOutlineDoodad:
    case XkbSolidDoodad:
      draw_shape_doodad (drawing, doodad, &doodad->doodad->shape);
        break;

    case XkbTextDoodad:
      draw_text_doodad (drawing, doodad, &doodad->doodad->text);
      break;

    case XkbIndicatorDoodad:
      draw_indicator_doodad (drawing, doodad, &doodad->doodad->indicator);
      break;

    case XkbLogoDoodad:
      /* g_print ("draw_doodad: logo: %s\n", doodad->doodad->logo.logo_name); */
      /* XkbLogoDoodadRec is essentially a subclass of XkbShapeDoodadRec */
      draw_shape_doodad (drawing, doodad, &doodad->doodad->shape);
      break;
    }
}

static void
draw_keyboard_item (KeyboardDrawingItem *item, 
                    KeyboardDrawing     *drawing)
{
  switch (item->type) 
    {
    case KEYBOARD_DRAWING_ITEM_TYPE_KEY:
      draw_key (drawing, (KeyboardDrawingKey *) item);
      break;

    case KEYBOARD_DRAWING_ITEM_TYPE_DOODAD:
      draw_doodad (drawing, (KeyboardDrawingDoodad *) item);
      break;
    }
}

static void
draw_keyboard (KeyboardDrawing *drawing)
{
  GtkStateType state = GTK_WIDGET_STATE (GTK_WIDGET (drawing));
  gint pixw, pixh;

  pixw = GTK_WIDGET (drawing)->allocation.width;
  pixh = GTK_WIDGET (drawing)->allocation.height;

  drawing->pixmap = gdk_pixmap_new (GTK_WIDGET (drawing)->window, pixw, pixh, -1);

  /* blank background */
  gdk_draw_rectangle (drawing->pixmap, GTK_WIDGET (drawing)->style->base_gc[state], TRUE, 0, 0, pixw, pixh);

  if (drawing->xkb == NULL)
    return;

  g_list_foreach (drawing->keyboard_items, (GFunc) draw_keyboard_item, drawing);
}

static gboolean
expose_event (GtkWidget       *widget,
              GdkEventExpose  *event, 
              KeyboardDrawing *drawing)
{
  GtkStateType state = GTK_WIDGET_STATE (GTK_WIDGET (drawing));

  if (drawing->pixmap == NULL)
    draw_keyboard (drawing);

  gdk_draw_drawable (widget->window,
                     widget->style->fg_gc[state],
                     drawing->pixmap,
                     event->area.x, event->area.y,
                     event->area.x, event->area.y,
                     event->area.width, event->area.height);

  if (GTK_WIDGET_HAS_FOCUS (widget))
    gtk_paint_focus (widget->style, widget->window,
                     GTK_WIDGET_STATE (widget), &event->area,
                     widget, "keyboard-drawing",
                     0, 0,
                     widget->allocation.width,
                     widget->allocation.height);

  return FALSE;
}

static void
size_allocate (GtkWidget       *widget,
               GtkAllocation   *allocation, 
               KeyboardDrawing *drawing)
{
  if (drawing->pixmap) 
    {
      g_object_unref (drawing->pixmap);
      drawing->pixmap = NULL;
    }

  if (drawing->xkb->geom->width_mm <= 0 || drawing->xkb->geom->height_mm <= 0) 
    {
      g_critical ("keyboard geometry reports width or height as zero!");
      return;
    }

  if (allocation->width * drawing->xkb->geom->height_mm < allocation->height * drawing->xkb->geom->width_mm) {
      drawing->scale_numerator = allocation->width;
      drawing->scale_denominator = drawing->xkb->geom->width_mm;
  } 
  else 
    {
      drawing->scale_numerator = allocation->height;
      drawing->scale_denominator = drawing->xkb->geom->height_mm;
    }

  pango_font_description_set_size (drawing->font_desc, 36000 * drawing->scale_numerator / drawing->scale_denominator);
  pango_layout_set_spacing (drawing->layout, -8000 * drawing->scale_numerator / drawing->scale_denominator);
  pango_layout_set_font_description (drawing->layout, drawing->font_desc);
}

static gint
key_event (GtkWidget       *widget, 
           GdkEventKey     *event,
           KeyboardDrawing *drawing)
{
  KeyboardDrawingKey *key = &drawing->keys[event->hardware_keycode];

  if (event->hardware_keycode > drawing->xkb->max_key_code || 
      event->hardware_keycode < drawing->xkb->min_key_code || 
      key->xkbkey == NULL) 
    {
      g_signal_emit (drawing, keyboard_drawing_signals[BAD_KEYCODE], 0, event->hardware_keycode);
      return FALSE;
    }

  if ((event->type == GDK_KEY_PRESS && key->pressed) || 
      (event->type == GDK_KEY_RELEASE && !key->pressed))
    return FALSE;
    /* otherwise this event changes the state we believed we had before */

  key->pressed = (event->type == GDK_KEY_PRESS);

  draw_key (drawing, key);
  invalidate_key_region (drawing, key);

  return FALSE;
}

static gint
button_press_event (GtkWidget       *widget,
                    GdkEventButton  *event, 
                    KeyboardDrawing *drawing)
{
  gtk_widget_grab_focus (widget);
  return FALSE;
}

static gboolean
unpress_keys (KeyboardDrawing *drawing)
{
  gint i;

  for (i = drawing->xkb->min_key_code; i <= drawing->xkb->max_key_code; i++)
    if (drawing->keys[i].pressed) 
      {
        drawing->keys[i].pressed = FALSE;
        draw_key (drawing, &drawing->keys[i]);
        invalidate_key_region (drawing, &drawing->keys[i]);
      }

  return FALSE;
}

static gint
focus_event (GtkWidget       *widget,
             GdkEventFocus   *event, 
             KeyboardDrawing *drawing)
{
  if (event->in && drawing->timeout > 0) 
    {
      g_source_remove (drawing->timeout);
      drawing->timeout = 0;
    }
  else
    drawing->timeout = g_timeout_add (120, (GSourceFunc) unpress_keys, drawing);

  return FALSE;
}

static gint
compare_keyboard_item_priorities (KeyboardDrawingItem *a,
                                  KeyboardDrawingItem *b)
{
  if (a->priority > b->priority)
    return 1;
  else if (a->priority < b->priority)
    return -1;
  else
    return 0;
}

static void
init_keys_and_doodads (KeyboardDrawing *drawing)
{
  gint i, j, k;
  gint x, y;

  for (i = 0; i < drawing->xkb->geom->num_doodads; i++) 
    {
      XkbDoodadRec *xkbdoodad = &drawing->xkb->geom->doodads[i];
      KeyboardDrawingDoodad *doodad = g_new (KeyboardDrawingDoodad, 1);

      doodad->type = KEYBOARD_DRAWING_ITEM_TYPE_DOODAD;
      doodad->origin_x = 0;
      doodad->origin_y = 0;
      doodad->angle = 0;
      doodad->priority = xkbdoodad->any.priority * 256 * 256;
      doodad->doodad = xkbdoodad;

      if (xkbdoodad->any.type == XkbIndicatorDoodad) 
        {
          gint index;
          gboolean real;

          XkbGetNamedIndicator (drawing->display, xkbdoodad->indicator.name, &index, &doodad->on, NULL, &real);

          if (real) 
            {
              if (index < 0 || index >= drawing->physical_indicators_size)
                g_warning ("XkbGetNamedIndicator returned junk data, ignoring");
              else 
                drawing->physical_indicators[index] = doodad;
            }
        }

      drawing->keyboard_items = g_list_append (drawing->keyboard_items, doodad);
    }

  for (i = 0; i < drawing->xkb->geom->num_sections; i++) 
    {
      XkbSectionRec *section = &drawing->xkb->geom->sections[i];
      guint priority;

      x = section->left;
      y = section->top;
      priority = section->priority * 256 * 256;

      for (j = 0; j < section->num_rows; j++) 
        {
          XkbRowRec *row = &section->rows[j];

          x = section->left + row->left;
          y = section->top + row->top;

          for (k = 0; k < row->num_keys; k++) 
            {
              XkbKeyRec *xkbkey = &row->keys[k];
              KeyboardDrawingKey *key;
              XkbShapeRec *shape = &drawing->xkb->geom->shapes[xkbkey->shape_ndx];
              guint keycode = find_keycode (drawing, xkbkey->name.  name);

              if (row->vertical)
                y += xkbkey->gap;
              else
                x += xkbkey->gap;

              if (keycode >= drawing->xkb->min_key_code && keycode <= drawing->xkb->max_key_code) 
                key = &drawing->keys[keycode];
              else 
                {
                  g_warning ("key %4.4s: keycode = %u; not in range %d..%d\n",
                             xkbkey->name.name, keycode,
                             drawing->xkb->min_key_code,
                             drawing->xkb->max_key_code);

                  key = g_new0 (KeyboardDrawingKey, 1);
                }

              key->type = KEYBOARD_DRAWING_ITEM_TYPE_KEY;
              key->xkbkey = xkbkey;
              key->angle = section->angle;
              rotate_coordinate (section->left, section->top, x, y, section->angle, 
                                 &key->origin_x, &key->origin_y);
              key->priority = priority;
              key->keycode = keycode;

              drawing->keyboard_items = g_list_append (drawing->keyboard_items, key);

              if (row->vertical)
                y += shape->bounds.y2;
              else
                x += shape->bounds.x2;

              priority++;
            }
        }

      for (j = 0; j < section->num_doodads; j++) 
        {
          XkbDoodadRec *xkbdoodad = &section->doodads[j];
          KeyboardDrawingDoodad *doodad = g_new (KeyboardDrawingDoodad, 1);

          doodad->type = KEYBOARD_DRAWING_ITEM_TYPE_DOODAD;
          doodad->origin_x = x;
          doodad->origin_y = y;
          doodad->angle = section->angle;
          doodad->priority = priority + xkbdoodad->any.priority;
          doodad->doodad = xkbdoodad;

          drawing->keyboard_items = g_list_append (drawing->keyboard_items, doodad);

          if (xkbdoodad->any.type == XkbIndicatorDoodad) 
            {
              gint index;
              gboolean real;

              XkbGetNamedIndicator (drawing->display, xkbdoodad->indicator.name, 
                                    &index, &doodad->on, NULL, &real);

              if (real)
                drawing->physical_indicators[index] = doodad;
            }
        }
    }

  g_list_sort (drawing->keyboard_items, (GCompareFunc) compare_keyboard_item_priorities);
}

static void
init_colors (KeyboardDrawing *drawing)
{
  gboolean result;
  gint i;

  drawing->colors = g_new (GdkColor, drawing->xkb->geom->num_colors);

  for (i = 0; i < drawing->xkb->geom->num_colors; i++) 
    {
      result = parse_xkb_color_spec (drawing->xkb->geom->colors[i].spec, &drawing->colors[i]);

      if (!result)
        g_warning ("init_colors: unable to parse color %s\n", drawing->xkb->geom->colors[i].spec);
    }
}

static void
free_keys_and_doodads_and_colors (KeyboardDrawing *drawing)
{
  GList *itemp;

  for (itemp = drawing->keyboard_items; itemp; itemp = itemp->next) 
    {
      KeyboardDrawingItem *item = itemp->data;
      KeyboardDrawingKey *key;

      switch (item->type) 
        {
        case KEYBOARD_DRAWING_ITEM_TYPE_DOODAD:
          g_free (item);
          break;

        case KEYBOARD_DRAWING_ITEM_TYPE_KEY:
          key = (KeyboardDrawingKey *) item;
          if (key->keycode < drawing->xkb->min_key_code || key->keycode > drawing->xkb->max_key_code)
            g_free (key);
          /* otherwise it's part of the array */
          break;
        }
    }

  g_list_free (drawing->keyboard_items);
  drawing->keyboard_items = NULL;

  g_free (drawing->keys);
  g_free (drawing->colors);
}

static GdkFilterReturn
xkb_state_notify_event_filter (GdkXEvent       *gdkxev,
                               GdkEvent        *event, 
                               KeyboardDrawing *drawing)
{
  gint group_change_mask = XkbGroupStateMask | XkbGroupBaseMask | XkbGroupLatchMask | XkbGroupLockMask;

  if (((XEvent *) gdkxev)->type == drawing->xkb_event_type) 
    {
      XkbEvent *kev = (XkbEvent *) gdkxev;
      switch (kev->any.xkb_type) 
        {
        case XkbStateNotify:
          if ((kev->state.changed & group_change_mask) && drawing->track_group) 
            {
              free_keys_and_doodads_and_colors (drawing);
              keyboard_drawing_set_groups (drawing, kev->state.locked_group, -1);
              drawing->keys = g_new0 (KeyboardDrawingKey, drawing->xkb->max_key_code + 1);
              size_allocate (GTK_WIDGET (drawing), &(GTK_WIDGET (drawing)->allocation), drawing);

              init_keys_and_doodads (drawing);
              init_colors (drawing);
            }
          break;

        case XkbIndicatorStateNotify:
            {
              XkbIndicatorNotifyEvent *iev = &((XkbEvent *) gdkxev)->indicators;
              gint i;

              for (i = 0; i <= drawing->xkb->indicators->phys_indicators; i++)
                if (drawing->physical_indicators[i] != NULL && (iev->changed & 1 << i)) 
                  {
                    gint state = (iev->state & 1 << i) != FALSE;

                    if ((state && !drawing->physical_indicators[i]->on) || 
                        (!state && drawing->physical_indicators[i]->on)) 
                      {
                        drawing->physical_indicators[i]->on = state;
                        draw_doodad (drawing, drawing->physical_indicators[i]);
                        invalidate_indicator_doodad_region (drawing, drawing->physical_indicators[i]);
                      }
                  }
            }
          break;

        case XkbIndicatorMapNotify:
        case XkbControlsNotify:
        case XkbNamesNotify:
        case XkbNewKeyboardNotify:
          if (drawing->track_group) 
              keyboard_drawing_set_groups (drawing, 0, -1);
          if (drawing->track_config)
            keyboard_drawing_set_keyboard (drawing, NULL);
          break;
        }
    }

  return GDK_FILTER_CONTINUE;
}

static void
destroy(KeyboardDrawing *drawing)
{
  printf ("kbdrawing.destroy\n");
  gdk_window_remove_filter (NULL, (GdkFilterFunc) xkb_state_notify_event_filter, drawing);
  if (drawing->timeout > 0) 
    {
      g_source_remove (drawing->timeout);
      drawing->timeout = 0;
    }
}

static void
keyboard_drawing_init (KeyboardDrawing *drawing)
{
  gint opcode = 0, error = 0, major = 1, minor = 0;
  PangoContext *context;
  gint mask;

  drawing->display = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());

  if (!XkbQueryExtension (drawing->display, &opcode, &drawing->xkb_event_type, &error, &major, &minor))
    g_critical ("XkbQueryExtension failed! Stuff probably won't work.");

  /* XXX: this stuff probably doesn't matter.. also, gdk_screen_get_default can fail */
  if (gtk_widget_has_screen (GTK_WIDGET (drawing)))
    drawing->screen_num = gdk_screen_get_number (gtk_widget_get_screen (GTK_WIDGET (drawing)));
  else 
    drawing->screen_num = gdk_screen_get_number (gdk_screen_get_default ());

  drawing->pixmap = NULL;
  context = pango_xft_get_context (drawing->display, drawing->screen_num);
  drawing->layout = pango_layout_new (context);
  g_object_unref (context);
  drawing->font_desc = pango_font_description_copy (GTK_WIDGET (drawing)->style->font_desc);
  drawing->keyboard_items = NULL;
  drawing->colors = NULL;
  drawing->leftgroup = 0;
  drawing->rightgroup = -1;
  drawing->angle = 0;
  drawing->scale_numerator = 1;
  drawing->scale_denominator = 1;

  drawing->track_group = 0;
  drawing->track_config = 0;

  gtk_widget_set_double_buffered (GTK_WIDGET (drawing), FALSE);

  /* XXX: XkbClientMapMask | XkbIndicatorMapMask | XkbNamesMask | XkbGeometryMask */
  drawing->xkb = XkbGetKeyboard (drawing->display, 
                                 XkbGBN_GeometryMask | XkbGBN_KeyNamesMask | XkbGBN_OtherNamesMask | XkbGBN_ClientSymbolsMask | XkbGBN_IndicatorMapMask, 
                                 XkbUseCoreKbd);
  if (drawing->xkb == NULL)
    g_critical ("XkbGetKeyboard failed to get keyboard from the server!");

  drawing->physical_indicators_size = drawing->xkb->indicators->phys_indicators + 1;
  drawing->physical_indicators = g_new0 (KeyboardDrawingDoodad *, drawing->physical_indicators_size);

  XkbSelectEventDetails (drawing->display, XkbUseCoreKbd, XkbIndicatorStateNotify, 
                         drawing->xkb->indicators->phys_indicators, 
                         drawing->xkb->indicators->phys_indicators);

  mask = (XkbStateNotifyMask|XkbNamesNotifyMask|XkbControlsNotifyMask|XkbIndicatorMapNotifyMask|XkbNewKeyboardNotifyMask);
  XkbSelectEvents (drawing->display, XkbUseCoreKbd, mask, mask);

  mask = XkbGroupStateMask;
  XkbSelectEventDetails (drawing->display, XkbUseCoreKbd, XkbStateNotify, mask, mask);

  mask = (XkbGroupNamesMask|XkbIndicatorNamesMask);
  XkbSelectEventDetails (drawing->display, XkbUseCoreKbd, XkbNamesNotify, mask, mask);
  drawing->keys = g_new0 (KeyboardDrawingKey, drawing->xkb->max_key_code + 1);
  init_keys_and_doodads (drawing);
  init_colors (drawing);

  /* required to get key events */
  GTK_WIDGET_SET_FLAGS (GTK_WIDGET (drawing), GTK_CAN_FOCUS);

  gtk_widget_set_events (GTK_WIDGET (drawing), 
                         GDK_EXPOSURE_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | 
                         GDK_BUTTON_PRESS_MASK | GDK_FOCUS_CHANGE_MASK);
  g_signal_connect (G_OBJECT (drawing), "expose-event", G_CALLBACK (expose_event), drawing);
  g_signal_connect (G_OBJECT (drawing), "key-press-event", G_CALLBACK (key_event), drawing);
  g_signal_connect (G_OBJECT (drawing), "key-release-event", G_CALLBACK (key_event), drawing);
  g_signal_connect (G_OBJECT (drawing), "button-press-event", G_CALLBACK (button_press_event), drawing);
  g_signal_connect (G_OBJECT (drawing), "focus-out-event", G_CALLBACK (focus_event), drawing);
  g_signal_connect (G_OBJECT (drawing), "focus-in-event", G_CALLBACK (focus_event), drawing);
  g_signal_connect (G_OBJECT (drawing), "size-allocate", G_CALLBACK (size_allocate), drawing);
  g_signal_connect (G_OBJECT (drawing), "destroy", G_CALLBACK (destroy), drawing);

  gdk_window_add_filter (NULL, (GdkFilterFunc) xkb_state_notify_event_filter, drawing);
}

GtkWidget *
keyboard_drawing_new ()
{
  return GTK_WIDGET (g_object_new (keyboard_drawing_get_type (), NULL));
}

static void
keyboard_drawing_class_init (KeyboardDrawingClass *klass)
{
  klass->bad_keycode = NULL;

  keyboard_drawing_signals[BAD_KEYCODE] =
            g_signal_new ("bad-keycode", keyboard_drawing_get_type (), G_SIGNAL_RUN_FIRST, 
                          G_STRUCT_OFFSET (KeyboardDrawingClass, bad_keycode), NULL, NULL, 
                          keyboard_marshal_VOID__UINT, G_TYPE_NONE, 1, G_TYPE_UINT);
}

GType
keyboard_drawing_get_type ()
{
  static GType keyboard_drawing_type = 0;

  if (!keyboard_drawing_type) 
    {
      static const GTypeInfo keyboard_drawing_info = 
        {
          sizeof (KeyboardDrawingClass),
          NULL, /* base_init */
          NULL, /* base_finalize */
          (GClassInitFunc) keyboard_drawing_class_init,
          NULL, /* class_finalize */
          NULL, /* class_data */
          sizeof (KeyboardDrawing),
          0,    /* n_preallocs */
          (GInstanceInitFunc) keyboard_drawing_init,
        };

      keyboard_drawing_type = g_type_register_static (GTK_TYPE_DRAWING_AREA, "KeyboardDrawing", 
                                                      &keyboard_drawing_info, 0);
    }

  return keyboard_drawing_type;
}

/* KeyboardDrawing can only display two groups at a time, one on the left
 * side of the key and one on the right. */
void
keyboard_drawing_set_groups (KeyboardDrawing *kbdrawing,
                             gint             leftgroup, 
                             gint             rightgroup)
{
  if (leftgroup != kbdrawing->leftgroup || 
      rightgroup != kbdrawing->rightgroup) 
    {
      kbdrawing->leftgroup = leftgroup;
      kbdrawing->rightgroup = rightgroup;
      gtk_widget_queue_draw (GTK_WIDGET (kbdrawing));
    }
}

/* KeyboardDrawing can only display two shift levels at a time, one on the
 * bottom and one on the top (flipped for certain keys) */
void
keyboard_drawing_set_levels (KeyboardDrawing *kbdrawing,
                             gint             bottomlevel, 
                             gint             toplevel)
{
  if (bottomlevel != kbdrawing->bottomlevel || toplevel != kbdrawing->toplevel) 
    {
      kbdrawing->bottomlevel = bottomlevel;
      kbdrawing->toplevel = toplevel;
      gtk_widget_queue_draw (GTK_WIDGET (kbdrawing));
    }
}

/* returns a pixbuf with the keyboard drawing at the current pixel size
 * (which can then be saved to disk, etc) */
GdkPixbuf *
keyboard_drawing_get_pixbuf (KeyboardDrawing *kbdrawing)
{
  if (kbdrawing->pixmap == NULL)
    draw_keyboard (kbdrawing);

  return gdk_pixbuf_get_from_drawable (NULL, kbdrawing->pixmap, NULL, 0, 0, 0, 0, 
                                       xkb_to_pixmap_coord (kbdrawing, kbdrawing->xkb->geom->width_mm), 
                                       xkb_to_pixmap_coord (kbdrawing, kbdrawing->xkb->geom->height_mm));
}

gboolean
keyboard_drawing_set_keyboard (KeyboardDrawing      *kbdrawing,
                               XkbComponentNamesRec *names)
{
  free_keys_and_doodads_and_colors (kbdrawing);
  XkbFreeKeyboard (kbdrawing->xkb, 0, TRUE);    /* free_all = TRUE */
  kbdrawing->xkb = NULL;  

  if (names)
    kbdrawing->xkb = XkbGetKeyboardByName (kbdrawing->display, XkbUseCoreKbd, names, 0, 
                                           XkbGBN_GeometryMask | XkbGBN_KeyNamesMask | XkbGBN_OtherNamesMask | XkbGBN_ClientSymbolsMask | XkbGBN_IndicatorMapMask, 
                                           FALSE);
  else
    kbdrawing->xkb = XkbGetKeyboard (kbdrawing->display,
                                     XkbGBN_GeometryMask | XkbGBN_KeyNamesMask | XkbGBN_OtherNamesMask | XkbGBN_ClientSymbolsMask | XkbGBN_IndicatorMapMask, 
	                              XkbUseCoreKbd);

  if (kbdrawing->xkb == NULL)
    return FALSE;

  kbdrawing->keys = g_new0 (KeyboardDrawingKey, kbdrawing->xkb->max_key_code + 1);
  init_keys_and_doodads (kbdrawing);
  init_colors (kbdrawing);

  size_allocate (GTK_WIDGET (kbdrawing), &(GTK_WIDGET (kbdrawing)->allocation), kbdrawing);
  gtk_widget_queue_draw (GTK_WIDGET (kbdrawing));

  return TRUE;
}

G_CONST_RETURN gchar *
keyboard_drawing_get_keycodes (KeyboardDrawing *kbdrawing)
{
  if (kbdrawing->xkb->names->keycodes <= 0)
    return NULL;
  else
    return XGetAtomName (kbdrawing->display, kbdrawing->xkb->names->keycodes);
}

G_CONST_RETURN gchar *
keyboard_drawing_get_geometry (KeyboardDrawing *kbdrawing)
{
  if (kbdrawing->xkb->names->geometry <= 0)
    return NULL;
  else
    return XGetAtomName (kbdrawing->display, kbdrawing->xkb->names->geometry);
}

G_CONST_RETURN gchar *
keyboard_drawing_get_symbols (KeyboardDrawing * kbdrawing)
{
  if (kbdrawing->xkb->names->symbols <= 0)
    return NULL;
  else
    return XGetAtomName (kbdrawing->display, kbdrawing->xkb->names->symbols);
}

G_CONST_RETURN gchar *
keyboard_drawing_get_types (KeyboardDrawing * kbdrawing)
{
  if (kbdrawing->xkb->names->types <= 0)
    return NULL;
  else
    return XGetAtomName (kbdrawing->display, kbdrawing->xkb->names->types);
}

G_CONST_RETURN gchar *
keyboard_drawing_get_compat (KeyboardDrawing * kbdrawing)
{
  if (kbdrawing->xkb->names->compat <= 0)
    return NULL;
  else
    return XGetAtomName (kbdrawing->display, kbdrawing->xkb->names->compat);
}

void
keyboard_drawing_set_track_group  (KeyboardDrawing      *kbdrawing, 
                                   gboolean              enable)
{
  if (enable)
    kbdrawing->track_group = 1;
  else
    kbdrawing->track_group = 0;
}

void
keyboard_drawing_set_track_config (KeyboardDrawing      *kbdrawing, 
                                   gboolean              enable)
{
  if (enable)
    kbdrawing->track_config = 1;
  else
    kbdrawing->track_config = 0;
}
