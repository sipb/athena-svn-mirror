/*
* test-gok-bounds.c
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
#include <gnome.h>
#include "gok-bounds.h"

int main ()
{
	gint bitmask, x, y, width, height, screen_width, screen_height;
	gint out_x, out_y;

	screen_width = 1024;
	screen_height = 768;

	/* Test case 1 */

	bitmask = XParseGeometry("640x480+0+0", &x, &y, (unsigned int *)&width, (unsigned int *)&height);

	gok_bounds_get_upper_left(bitmask, x, y, width, height,
	                          screen_width, screen_height,
	                          &out_x, &out_y);

	g_assert(out_x == 0);
	g_assert(out_y == 0);

	/* Test case 2 */

	bitmask = XParseGeometry("640x480+0-0", &x, &y, (unsigned int *)&width, (unsigned int *)&height);

	gok_bounds_get_upper_left(bitmask, x, y, width, height,
	                          screen_width, screen_height,
	                          &out_x, &out_y);

	g_assert(out_x == 0);
	g_assert(out_y == 288);

	/* Test case 3 */

	bitmask = XParseGeometry("640x480-0+0", &x, &y, (unsigned int *)&width, (unsigned int *)&height);

	gok_bounds_get_upper_left(bitmask, x, y, width, height,
	                          screen_width, screen_height,
	                          &out_x, &out_y);

	g_assert(out_x == 384);
	g_assert(out_y == 0);

	/* Test case 4 */

	bitmask = XParseGeometry("640x480-0-0", &x, &y, (unsigned int *)&width, (unsigned int *)&height);

	gok_bounds_get_upper_left(bitmask, x, y, width, height,
	                          screen_width, screen_height,
	                          &out_x, &out_y);

	g_assert(out_x == 384);
	g_assert(out_y == 288);

	/* Test case 5 */

	bitmask = XParseGeometry("640x480+4+8", &x, &y, (unsigned int *)&width, (unsigned int *)&height);

	gok_bounds_get_upper_left(bitmask, x, y, width, height,
	                          screen_width, screen_height,
	                          &out_x, &out_y);

	g_assert(out_x == 4);
	g_assert(out_y == 8);

	/* Test case 6 */

	bitmask = XParseGeometry("640x480+4-8", &x, &y, (unsigned int *)&width, (unsigned int *)&height);

	gok_bounds_get_upper_left(bitmask, x, y, width, height,
	                          screen_width, screen_height,
	                          &out_x, &out_y);

	g_assert(out_x == 4);
	g_assert(out_y == 280);

	/* Test case 7 */

	bitmask = XParseGeometry("640x480-4+8", &x, &y, (unsigned int *)&width, (unsigned int *)&height);

	gok_bounds_get_upper_left(bitmask, x, y, width, height,
	                          screen_width, screen_height,
	                          &out_x, &out_y);

	g_assert(out_x == 380);
	g_assert(out_y == 8);

	/* Test case 8 */

	bitmask = XParseGeometry("640x480-4-8", &x, &y, (unsigned int *)&width, (unsigned int *)&height);

	gok_bounds_get_upper_left(bitmask, x, y, width, height,
	                          screen_width, screen_height,
	                          &out_x, &out_y);

	g_assert(out_x == 380);
	g_assert(out_y == 280);
	return 0;
}
