/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-gnome-extensions.h - interface for new functions that operate on
                                 gnome classes. Perhaps some of these should be
  			         rolled into gnome someday.

   Copyright (C) 1999, 2000, 2001 Eazel, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors: Darin Adler <darin@eazel.com>
*/

#ifndef EEL_GNOME_EXTENSIONS_H
#define EEL_GNOME_EXTENSIONS_H

#include <glade/glade.h>
#include <gtk/gtkwindow.h>
#include <libgnomecanvas/gnome-canvas.h>
#include <bonobo/bonobo-property-bag-client.h>

/* icon selection callback function. */
typedef void (* EelIconSelectionFunction) (const char *icon_path, gpointer callback_data);

/* Causes an update as needed. The GnomeCanvas code says it does this, but it doesn't. */
void          eel_gnome_canvas_set_scroll_region                      (GnomeCanvas              *canvas,
								       double                    x1,
								       double                    y1,
								       double                    x2,
								       double                    y2);

/* Make the scroll region bigger so the code in GnomeCanvas won't center it. */
void          eel_gnome_canvas_set_scroll_region_left_justify         (GnomeCanvas              *canvas,
								       double                    x1,
								       double                    y1,
								       double                    x2,
								       double                    y2);

/* Set a new scroll region without eliminating any of the currently-visible area. */
void          eel_gnome_canvas_set_scroll_region_include_visible_area (GnomeCanvas              *canvas,
								       double                    x1,
								       double                    y1,
								       double                    x2,
								       double                    y2);

/* For cases where you need to get more than one item updated. */
void          eel_gnome_canvas_request_update_all                     (GnomeCanvas              *canvas);
void          eel_gnome_canvas_item_request_update_deep               (GnomeCanvasItem          *item);

/* This is more handy than gnome_canvas_item_get_bounds because it
 * always returns the bounds * in world coordinates and it returns
 * them in a single rectangle.
 */
ArtDRect      eel_gnome_canvas_item_get_world_bounds                  (GnomeCanvasItem          *item);

/* This returns the current canvas bounds as computed by update.
 * It's not as "up to date" as get_bounds, which is accurate even
 * before an update happens.
 */
ArtIRect      eel_gnome_canvas_item_get_current_canvas_bounds         (GnomeCanvasItem          *item);

/* This returns the canvas bounds in a slower way that's always up to
 * date, even before an update.
 */
ArtIRect      eel_gnome_canvas_item_get_canvas_bounds                 (GnomeCanvasItem          *item);

/* Convenience functions for doing things with whole rectangles. */
ArtIRect      eel_gnome_canvas_world_to_canvas_rectangle              (GnomeCanvas              *canvas,
								       ArtDRect                  world_rectangle);
void          eel_gnome_canvas_request_redraw_rectangle               (GnomeCanvas              *canvas,
								       ArtIRect                  canvas_rectangle);

/* Often, it's more useful to have coordinates in widget terms, rateher than world, canvas, or bin window terms. */
void          eel_gnome_canvas_widget_to_world                        (GnomeCanvas              *canvas,
								       int                       widget_x,
								       int                       widget_y,
								       double                   *world_x,
								       double                   *world_y);
void          eel_gnome_canvas_world_to_widget                        (GnomeCanvas              *canvas,
								       double                    world_x,
								       double                    world_y,
								       int                      *widget_x,
								       int                      *widget_y);
ArtIRect      eel_gnome_canvas_world_to_widget_rectangle              (GnomeCanvas              *canvas,
								       ArtDRect                  world_rectangle);

/* Function for moving a canvas item relative to another existing item. */
void          eel_gnome_canvas_item_send_behind                       (GnomeCanvasItem          *item,
								       GnomeCanvasItem          *behind_item);

/* Requests the entire object be redrawn.
 * Normally, you use request_update when calling from outside the canvas item
 * code. This is for within canvas item code.
 */
void          eel_gnome_canvas_item_request_redraw                    (GnomeCanvasItem          *item);
void          eel_gnome_canvas_draw_pixbuf                            (GnomeCanvasBuf           *buf,
								       const GdkPixbuf          *pixbuf,
								       int                       x,
								       int                       y);
void          eel_gnome_canvas_fill_rgb                               (GnomeCanvasBuf           *buf,
								       art_u8                    r,
								       art_u8                    g,
								       art_u8                    b);

/* Functions for dealing with Pango text in a canvas. */
PangoContext *eel_gnome_canvas_get_pango_context                      (GnomeCanvas              *canvas);

/* Return a command string containing the path to a terminal on this system. */
char *        eel_gnome_make_terminal_command                         (const char               *command);

/* Open up a new terminal, optionally passing in a command to execute */
void          eel_gnome_open_terminal                                 (const char               *command);

/* Create an icon selection dialog */
GtkWidget *   eel_gnome_icon_selector_new                             (const char               *title,
								       const char               *icon_directory,
								       GtkWindow                *owner,
								       EelIconSelectionFunction  selected,
								       gpointer                  callback_data);
									 
/* Execute a command. To be used in place of the exec family commands. This function
 * protects against creating zombie processes.
 */
void          eel_gnome_shell_execute                                 (const char               *command);

char         *eel_bonobo_make_registration_id                         (const char               *iid);

void          eel_bonobo_pbclient_set_value_async                     (Bonobo_PropertyBag        bag,
								       const char               *key,
								       CORBA_any                *value,
								       CORBA_Environment        *opt_ev);

GladeXML     *eel_glade_get_file                                      (const char               *filename,
								       const char               *root,
								       const char               *domain,
								       const char               *first_required_widget,
								       ...);

#endif /* EEL_GNOME_EXTENSIONS_H */
