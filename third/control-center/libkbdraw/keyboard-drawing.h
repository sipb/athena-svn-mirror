/* $Id: keyboard-drawing.h,v 1.1.1.1 2004-10-04 03:55:57 ghudson Exp $ */
/*
 * keyboard-drawing.h: header file for a gtk+ widget that is a drawing of
 * the keyboard of the default display
 *
 * Copyright (c) 2003 Noah Levitt
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

#ifndef KEYBOARD_DRAWING_H
#define KEYBOARD_DRAWING_H 1

#include <gtk/gtk.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBgeom.h>

G_BEGIN_DECLS

#define KEYBOARD_DRAWING(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), keyboard_drawing_get_type (), \
                               KeyboardDrawing))

#define KEYBOARD_DRAWING_CLASS(clazz) (G_TYPE_CHECK_CLASS_CAST ((clazz), keyboard_drawing_get_type () \
                                       KeyboardDrawingClass))

#define IS_KEYBOARD_DRAWING(obj) G_TYPE_CHECK_INSTANCE_TYPE ((obj), keyboard_drawing_get_type ())

typedef struct _KeyboardDrawing       KeyboardDrawing;
typedef struct _KeyboardDrawingClass  KeyboardDrawingClass;

typedef struct _KeyboardDrawingItem   KeyboardDrawingItem;
typedef struct _KeyboardDrawingKey    KeyboardDrawingKey;
typedef struct _KeyboardDrawingDoodad KeyboardDrawingDoodad;

typedef enum 
{ 
  KEYBOARD_DRAWING_ITEM_TYPE_KEY,
  KEYBOARD_DRAWING_ITEM_TYPE_DOODAD
} 
KeyboardDrawingItemType;

/* units are in xkb form */
struct _KeyboardDrawingItem
{
  /*< private >*/

  KeyboardDrawingItemType type;
  gint                    origin_x;
  gint                    origin_y;
  gint                    angle;
  guint                   priority;
};

/* units are in xkb form */
struct _KeyboardDrawingKey
{
  /*< private >*/

  KeyboardDrawingItemType  type;
  gint                     origin_x;
  gint                     origin_y;
  gint                     angle;
  guint                    priority;

  XkbKeyRec               *xkbkey;
  gboolean                 pressed;
  guint                    keycode;
};

/* units are in xkb form */
struct _KeyboardDrawingDoodad
{
  /*< private >*/

  KeyboardDrawingItemType  type;
  gint                     origin_x;
  gint                     origin_y;
  gint                     angle;
  guint                    priority;

  XkbDoodadRec            *doodad;
  gboolean                 on; /* for indicator doodads */
};


struct _KeyboardDrawing
{
  /*< private >*/

  GtkDrawingArea          parent;

  GdkPixmap              *pixmap;
  XkbDescRec             *xkb;

  gint                    angle;  /* current angle pango is set to draw at, in tenths of a degree */
  PangoLayout            *layout;
  PangoFontDescription   *font_desc;

  gint                    scale_numerator;
  gint                    scale_denominator;

  KeyboardDrawingKey     *keys;

  /* list of stuff to draw in priority order */
  GList                  *keyboard_items;

  GdkColor               *colors;

  guint                   timeout;

  gint                    leftgroup;
  gint                    rightgroup;

  gint                    bottomlevel;
  gint                    toplevel;

  Display                *display;
  gint                    screen_num;

  gint                    xkb_event_type;

  KeyboardDrawingDoodad **physical_indicators;
  gint                    physical_indicators_size;

  guint                   track_config : 1;
  guint                   track_group : 1;
};

struct _KeyboardDrawingClass
{
  GtkDrawingAreaClass parent_class;

  /* we send this signal when the user presses a key that "doesn't exist"
   * according to the keyboard geometry; it probably means their xkb
   * configuration is incorrect */
  void (* bad_keycode) (KeyboardDrawing *drawing, 
                        guint            keycode);
};

GType                 keyboard_drawing_get_type         ();
GtkWidget *           keyboard_drawing_new              ();
void                  keyboard_drawing_set_groups       (KeyboardDrawing      *kbdrawing, 
                                                         gint                  leftgroup, 
                                                         gint                  rightgroup);
void                  keyboard_drawing_set_levels       (KeyboardDrawing      *kbdrawing, 
                                                         gint                  bottomlevel, 
                                                         gint                  toplevel);
GdkPixbuf *           keyboard_drawing_get_pixbuf       (KeyboardDrawing      *kbdrawing);
gboolean              keyboard_drawing_set_keyboard     (KeyboardDrawing      *kbdrawing, 
                                                         XkbComponentNamesRec *names);

G_CONST_RETURN gchar *keyboard_drawing_get_keycodes     (KeyboardDrawing      *kbdrawing);
G_CONST_RETURN gchar *keyboard_drawing_get_geometry     (KeyboardDrawing      *kbdrawing);
G_CONST_RETURN gchar *keyboard_drawing_get_symbols      (KeyboardDrawing      *kbdrawing);
G_CONST_RETURN gchar *keyboard_drawing_get_types        (KeyboardDrawing      *kbdrawing);
G_CONST_RETURN gchar *keyboard_drawing_get_compat       (KeyboardDrawing      *kbdrawing);

void                  keyboard_drawing_set_track_group  (KeyboardDrawing      *kbdrawing, 
                                                         gboolean              enable);
void                  keyboard_drawing_set_track_config (KeyboardDrawing      *kbdrawing, 
                                                         gboolean              enable);

G_END_DECLS

#endif /* #ifndef KEYBOARD_DRAWING_H */
