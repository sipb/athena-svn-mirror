/*
* gok-bounds.c
*
* Copyright 2002 Sun Microsystems, Inc.,
* Copyright 2002 University Of Toronto
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <gnome.h>

/**
 * gok_bounds_get_upper_left:
 * @bitmask:       the bitmask as returned from XParseGeometry()
 * @x:             the x location resulting from XParseGeometry()
 * @y:             the y location resulting from XParseGeometry()
 * @width:         the width resulting from XParseGeometry()
 * @height:        the height resulting from XParseGeometry()
 * @screen_width:  the screen width, for example from gdk_screen_width()
 * @screen_height: the screen height, for example from gdk_screen_height()
 * @out_x:         the address of a gint to store the processed x in
 * @out_y:         the address of a gint to store the processed y in
 *
 * Calculates the location of the upper left corner of the rectangular
 * area of the screen that gok can use given the parsed X11 geometry
 * specification and the screen width and height.
 * Precondition: x, y, width and height must all be specified
 */
void gok_bounds_get_upper_left(gint bitmask, gint x, gint y,
                               gint width, gint height,
                               gint screen_width, gint screen_height,
                               gint *out_x, gint *out_y)
{
	if (bitmask & XNegative) {
		*out_x = screen_width - width + x;
	} else {
		*out_x = x;
	}

	if (bitmask & YNegative) {
		*out_y = screen_height - height + y;
	} else {
		*out_y = y;
	}
}
