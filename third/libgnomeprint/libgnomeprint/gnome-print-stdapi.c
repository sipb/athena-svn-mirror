/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-stdapi.c: Convenience drawing functions for drivers
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
 *
 *  Authors:
 *    Raph Levien <raph@acm.org>
 *    Miguel de Icaza <miguel@kernel.org>
 *    Lauris Kaplinski <lauris@ximian.com>
 *    Chema Celorio <chema@celorio.com>
 *
 *  Copyright (C) 1999-2003 Ximian Inc. and authors
 */

#include <config.h>
#include <math.h>
#include <string.h>

#include <libart_lgpl/art_affine.h>
#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gnome-glyphlist-private.h>
#include <libgnomeprint/gp-gc-private.h>

/**
 * gnome_print_clip:
 * @pc: A #GnomePrintContext
 *
 * Defines drawing region as inside area of currentpath. If path is
 * self-intersecting or consists of several overlapping subpaths,
 * nonzero rule is used to define the inside orea of path.
 * All open subpaths of currentpath are closed.
 * If currentpath is empty, #GNOME_PRINT_ERROR_NOCURRENTPATH is
 * returned.
 * Currentpath is emptied by this function.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_clip (GnomePrintContext *pc)
{
	gint ret;

	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (gp_gc_has_currentpath (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPATH);

	gp_gc_close_all (pc->gc);
	ret = gnome_print_clip_bpath_rule (pc, gp_gc_get_currentpath (pc->gc), ART_WIND_RULE_ODDEVEN);
	gp_gc_newpath (pc->gc);

	return ret;
}

/**
 * gnome_print_eoclip:
 * @pc: A #GnomePrintContext
 *
 * Defines drawing region as inside area of currentpath. If path is
 * self-intersecting or consists of several overlapping subpaths,
 * even-odd rule is used to define the inside area of path.
 * All open subpaths of currentpath are closed.
 * If currentpath is empty, #GNOME_PRINT_ERROR_NOCURRENTPATH is
 * returned.
 * Currentpath is emptied by this function.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_eoclip (GnomePrintContext *pc)
{
	gint ret;

	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (gp_gc_has_currentpath (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPATH);

	gp_gc_close_all (pc->gc);
	ret = gnome_print_clip_bpath_rule (pc, gp_gc_get_currentpath (pc->gc), ART_WIND_RULE_ODDEVEN);
	gp_gc_newpath (pc->gc);

	return ret;
}

/**
 * gnome_print_fill:
 * @pc: A #GnomePrintContext
 *
 * Fills the inside area of currentpath, using current graphic state.
 * If path is self-intersecting or consists of several overlapping subpaths,
 * nonzero rule is used to define the inside area of path.
 * All open subpaths of currentpath are closed.
 * If currentpath is empty, #GNOME_PRINT_ERROR_NOCURRENTPATH is
 * returned.
 * Currentpath is emptied by this function.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_fill (GnomePrintContext *pc)
{
	gint ret;

	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (gp_gc_has_currentpath (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPATH);

	gp_gc_close_all (pc->gc);
	ret = gnome_print_fill_bpath_rule (pc, gp_gc_get_currentpath (pc->gc), ART_WIND_RULE_NONZERO);
	gp_gc_newpath (pc->gc);

	return ret;
}

/**
 * gnome_print_eofill:
 * @pc: A #GnomePrintContext
 *
 * Fills the inside area of currentpath, using current graphic state.
 * If path is self-intersecting or consists of several overlapping subpaths,
 * even-odd rule is used to define the inside area of path.
 * All open subpaths of currentpath are closed.
 * If currentpath is empty, #GNOME_PRINT_ERROR_NOCURRENTPATH is
 * returned.
 * Currentpath is emptied by this function.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_eofill (GnomePrintContext *pc)
{
	gint ret;

	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (gp_gc_has_currentpath (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPATH);

	gp_gc_close_all (pc->gc);
	ret = gnome_print_fill_bpath_rule (pc, gp_gc_get_currentpath (pc->gc), ART_WIND_RULE_ODDEVEN);
	gp_gc_newpath (pc->gc);

	return ret;
}

/**
 * gnome_print_stroke:
 * @pc: A #GnomePrintContext
 *
 * Strokes currentpath, i.e. draws line along it, with style, defined
 * by current graphic state values.
 * If currentpath is empty, #GNOME_PRINT_ERROR_NOCURRENTPATH is
 * returned.
 * Currentpath is emptied by this function.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_stroke (GnomePrintContext *pc)
{
	gint ret;

	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (gp_gc_has_currentpath (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPATH);

	ret = gnome_print_stroke_bpath (pc, gp_gc_get_currentpath (pc->gc));
	gp_gc_newpath (pc->gc);

	return ret;
}

/**
 * gnome_print_grayimage:
 * @pc: A #GnomePrintContext
 * @data: Pointer to image pixel buffer
 * @width: Image buffer width
 * @height: Image buffer height
 * @rowstride: Image buffer rowstride
 *
 * Draws grayscale image into unit square (0,0 - 1,1) in current coordinate
 * system.
 * Image buffer has to be 1 byte per pixel, with value 255 marking
 * white and 0 black.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_grayimage (GnomePrintContext *pc, const guchar *data, int width, int height, int rowstride)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (data != NULL, GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail (width > 0, GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail (height > 0, GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail (rowstride >= width, GNOME_PRINT_ERROR_BADVALUE);

	return gnome_print_image_transform (pc, gp_gc_get_ctm (pc->gc), data, width, height, rowstride, 1);
}

/**
 * gnome_print_rgbimage:
 * @pc: A #GnomePrintContext
 * @data: Pointer to image pixel buffer
 * @width: Image buffer width
 * @height: Image buffer height
 * @rowstride: Image buffer rowstride
 *
 * Draws RGB color image into unit square (0,0 - 1,1) in current coordinate
 * system.
 * Image buffer has to be 3 bytes per pixel, order RGB, with value 255 marking
 * maximum and 0 minimum value.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_rgbimage (GnomePrintContext *pc, const guchar *data, int width, int height, int rowstride)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (data != NULL, GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail (width > 0, GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail (height > 0, GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail (rowstride >= 3 * width, GNOME_PRINT_ERROR_BADVALUE);

	return gnome_print_image_transform (pc, gp_gc_get_ctm (pc->gc), data, width, height, rowstride, 3);
}

/**
 * gnome_print_rgbaimage:
 * @pc: A #GnomePrintContext
 * @data: Pointer to image pixel buffer
 * @width: Image buffer width
 * @height: Image buffer height
 * @rowstride: Image buffer rowstride
 *
 * Draws RGB color image with transparency channel image into unit square
 * (0,0 - 1,1) in current coordinate system.
 * Image buffer has to be 4 bytes per pixel, order RGBA, with value 255 marking
 * maximum and 0 minimum value. Alpha value 255 means full opacity, 0 full
 * transparency.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_rgbaimage (GnomePrintContext *pc, const guchar *data, int width, int height, int rowstride)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (data != NULL, GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail (width > 0, GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail (height > 0, GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail (rowstride >= 4 * width, GNOME_PRINT_ERROR_BADVALUE);

	return gnome_print_image_transform (pc, gp_gc_get_ctm (pc->gc), data, width, height, rowstride, 4);
}

/**
 * gnome_print_concat:
 * @pc: A #GnomePrintContext
 * @matrix: 3x2 affine transformation matrix
 *
 * Appends @matrix to current transformation matrix (CTM). The resulting
 * transformation from user coordinates to page coordinates is, as
 * if coordinates would first be transformed by @matrix, and the
 * results by CTM.
 * Matrix is given in column order, i.e.
 * X' = X * m[0] + Y * m[2] + m[4]
 * Y' = X * m[1] + Y * m[3] + m[5]
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_concat (GnomePrintContext *pc, const gdouble *matrix)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (matrix != NULL, GNOME_PRINT_ERROR_BADVALUE);

	gp_gc_concat (pc->gc, matrix);

	return  GNOME_PRINT_OK;
}

/**
 * gnome_print_newpath:
 * @pc: A #GnomePrintContext
 *
 * Resets currentpath to empty path. As currentpoint is defined as
 * the last point of open path segment, is also erases currentpoint.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_newpath (GnomePrintContext *pc)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);

	gp_gc_newpath (pc->gc);

	return GNOME_PRINT_OK;
}

/**
 * gnome_print_moveto:
 * @pc: A #GnomePrintContext
 * @x: X position in user coordinates
 * @y: Y position in user coordinates
 *
 * Starts new subpath in currentpath with coordinates @x,@y.
 * Moves currentpoint to @x,@y.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_moveto (GnomePrintContext *pc, double x, double y)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);

	gp_gc_moveto (pc->gc, x, y);

	return GNOME_PRINT_OK;
}

/**
 * gnome_print_lineto:
 * @pc: A #GnomePrintContext
 * @x: X position in user coordinates
 * @y: Y position in user coordinates
 *
 * Adds new straight line segment from currentpoint to @x,@y to currentpath.
 * Moves currentpoint to @x,@y.
 * If currentpoint is not defined, returns #GNOME_PRINT_ERROR_NOCURRENTPOINT.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_lineto (GnomePrintContext *pc, double x, double y)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (gp_gc_has_currentpoint (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPOINT);

	gp_gc_lineto (pc->gc, x, y);

	return GNOME_PRINT_OK;
}

/**
 * gnome_print_curveto:
 * @pc: A #GnomePrintContext
 * @x1: X position of first control point in user coordinates
 * @y1: Y position of first control point in user coordinates
 * @x2: X position of second control point in user coordinates
 * @y2: Y position of second control point in user coordinates
 * @x3: X position of endpoint in user coordinates
 * @y3: Y position of endpoint in user coordinates
 *
 * Adds new cubig bezier segment with control points @x1,@y1 and
 * @x2,@y2 and endpoint @x3,@y3 to currentpath.
 * Moves currentpoint to @x3,@y3.
 * If currentpoint is not defined, returns #GNOME_PRINT_ERROR_NOCURRENTPOINT.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_curveto (GnomePrintContext *pc, double x1, double y1, double x2, double y2, double x3, double y3)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (gp_gc_has_currentpoint (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPOINT);

	gp_gc_curveto (pc->gc, x1, y1, x2, y2, x3, y3);

	return GNOME_PRINT_OK;
}

/**
 * gnome_print_closepath:
 * @pc: A #GnomePrintContext
 *
 * Closes the last segment of currentpath, optionally drawing straight
 * line segment from its endpoint to starting point.
 * Erases currentpoint.
 * If currentpath is empty, returns #GNOME_PRINT_ERROR_NOCURRENTPATH.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_closepath (GnomePrintContext *pc)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (gp_gc_has_currentpath (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPATH);

	gp_gc_closepath (pc->gc);

	return GNOME_PRINT_OK;
}

/**
 * gnome_print_bpath:
 * @pc: A #GnomePrintContext
 * @bpath: Array of #ArtBpath segments
 * @append: Whether to append to currentpath
 *
 * Adds all @bpath segments up to #ART_END to currentpath. If @append
 * is false, currentpath is cleared first, otherwise segments are
 * appended to existing path.
 * This is identical to adding all segments by hand, so the final state
 * of currentpoint depends on segments processed.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_bpath (GnomePrintContext *pc, const ArtBpath *bpath, gboolean append)
{
	gboolean closed;

	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (bpath != NULL, GNOME_PRINT_ERROR_BADVALUE);

	if (!append) {
		gp_gc_newpath (pc->gc);
		if (bpath->code == ART_END)
			return GNOME_PRINT_OK;
		g_return_val_if_fail ((bpath->code == ART_MOVETO) || (bpath->code == ART_MOVETO_OPEN), GNOME_PRINT_ERROR_BADVALUE);
	}

	closed = FALSE;

	while (bpath->code != ART_END) {
		switch (bpath->code) {
		case ART_MOVETO:
		case ART_MOVETO_OPEN:
			if (closed)
				gp_gc_closepath (pc->gc);
			closed = (bpath->code == ART_MOVETO);
			gp_gc_moveto (pc->gc, bpath->x3, bpath->y3);
			break;
		case ART_LINETO:
			gp_gc_lineto (pc->gc, bpath->x3, bpath->y3);
			break;
		case ART_CURVETO:
			gp_gc_curveto (pc->gc, bpath->x1, bpath->y1, bpath->x2, bpath->y2, bpath->x3, bpath->y3);
			break;
		default:
			g_warning ("file %s: line %d: Illegal pathcode %d in bpath", __FILE__, __LINE__, bpath->code);
			return GNOME_PRINT_ERROR_BADVALUE;
			break;
		}
		bpath += 1;
	}

	if (closed)
		gp_gc_closepath (pc->gc);

	return GNOME_PRINT_OK;
}

/**
 * gnome_print_vpath:
 * @pc: A #GnomePrintContext
 * @vpath: Array of #ArtVpath segments
 * @append: Whether to append to currentpath
 *
 * Adds all @vpath line segments up to #ART_END to currentpath. If @append
 * is false, currentpath is cleared first, otherwise segments are
 * appended to existing path.
 * This is identical to adding all segments by hand, so the final state
 * of currentpoint depends on segments processed.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_vpath (GnomePrintContext *pc, const ArtVpath *vpath, gboolean append)
{
	gboolean closed;

	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (vpath != NULL, GNOME_PRINT_ERROR_BADVALUE);

	if (!append) {
		gp_gc_newpath (pc->gc);
		if (vpath->code == ART_END) return GNOME_PRINT_OK;
		g_return_val_if_fail ((vpath->code == ART_MOVETO) || (vpath->code == ART_MOVETO_OPEN), GNOME_PRINT_ERROR_BADVALUE);
	}

	closed = FALSE;

	while (vpath->code != ART_END) {
		switch (vpath->code) {
		case ART_MOVETO:
		case ART_MOVETO_OPEN:
			if (closed)
				gp_gc_closepath (pc->gc);
			closed = (vpath->code == ART_MOVETO);
			gp_gc_moveto (pc->gc, vpath->x, vpath->y);
			break;
		case ART_LINETO:
			gp_gc_lineto (pc->gc, vpath->x, vpath->y);
			break;
		default:
			g_warning ("file %s: line %d: Illegal pathcode %d in vpath", __FILE__, __LINE__, vpath->code);
			return GNOME_PRINT_ERROR_BADVALUE;
			break;
		}
		vpath += 1;
	}

	if (closed)
		gp_gc_closepath (pc->gc);

	return GNOME_PRINT_OK;
}

/**
 * gnome_print_arcto:
 * @pc: A #GnomePrintContextx
 * @x: X position of control point in user coordinates
 * @y: Y position of control point in user coordinates
 * @radius: the radius of the arc
 * @angle1: start angle in degrees
 * @angle2: end angle in degrees
 * @direction: direction of movement, 0 counterclockwise 1 clockwise
 * 
 * Adds an arc with control points @x and @y with a radius @radius and from
 * @angle1 to @andgle2 in degrees. @direction 1 is clockwise 0 counterclockwise
 * 
 * Return Value: GNOME_PRINT_OK or postive value on success, a negative value otherwise
 **/
gint
gnome_print_arcto (GnomePrintContext *pc, gdouble x, gdouble y, gdouble radius, gdouble angle1, gdouble angle2, gint direction)
{
	gdouble a;

	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail ((direction == 0) || (direction == 1), GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);

	/* Find modulo 360 */
	angle1 = fmod (angle1, 360.0);
	angle2 = fmod (angle2, 360.0);

	if (!gp_gc_has_currentpoint (pc->gc)) {
		gp_gc_moveto (pc->gc, x + radius * cos (angle1 * M_PI / 180.0), y + radius * sin (angle1 * M_PI / 180.0));
	}

	/* FIXME: really use curveto's here (Lauris) */
	if (!direction) {
		/* CCW */
		if (angle1 > angle2)
			angle2 += 360.0;
		for (a = angle1; a < angle2; a += 1.0) {
			gp_gc_lineto (pc->gc, x + radius * cos (a * M_PI / 180.0), y + radius * sin (a * M_PI / 180.0));
		}
	} else {
		/* CW */
		if (angle2 > angle1)
			angle2 -= 360.0;
		for (a = angle1; a > angle2; a -= 1.0) {
			gp_gc_lineto (pc->gc, x + radius * cos (a * M_PI / 180.0), y + radius * sin (a * M_PI / 180.0));
		}
	}

	gp_gc_lineto (pc->gc, x + radius * cos (angle2 * M_PI / 180.0), y + radius * sin (angle2 * M_PI / 180.0));

	return GNOME_PRINT_OK;
}

/**
 * gnome_print_strokepath:
 * @pc: A #GnomePrintContext
 *
 * Converts currentpath to new path, that is identical to area painted
 * by #gnome_print_stroke function, using currentpath. I.e. strokepath
 * followed by fill giver result identical to stroke.
 * If currentpath is empty, returns #GNOME_PRINT_ERROR_NOCURRENTPATH.
 * Stroked path is always closed, so currentpoint is erased.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_strokepath (GnomePrintContext *pc)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (gp_gc_has_currentpath (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPATH);

	gp_gc_strokepath (pc->gc);

	return gnome_print_bpath (pc, gp_gc_get_currentpath (pc->gc), FALSE);
}

/**
 * gnome_print_setrgbcolor:
 * @pc: A #GnomePrintContext
 * @r: Red channel value
 * @g: Green channel value
 * @b: Blue channel value
 *
 * Sets color in graphic state to RGB triplet. This does not imply anything
 * about which colorspace is used internally.
 * Channel values are clamped to 0.0 - 1.0 region, 0.0 meaning minimum.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_setrgbcolor (GnomePrintContext *pc, double r, double g, double b)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);

	gp_gc_set_rgbcolor (pc->gc, r, g, b);

	return GNOME_PRINT_OK;
}

/**
 * gnome_print_setopacity:
 * @pc: A #GnomePrintContext
 * @opacity: Opacity value
 *
 * Sets painting opacity in graphic state to given value.
 * Value is clamped to 0.0 - 1.0 region, 0.0 meaning full transparency and
 * 1.0 completely opaque paint.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_setopacity (GnomePrintContext *pc, double opacity)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);

	gp_gc_set_opacity (pc->gc, opacity);

	return GNOME_PRINT_OK;
}

/**
 * gnome_print_setlinewidth:
 * @pc: A #GnomePrintContext
 * @width: Line width in user coordinates
 *
 * Sets line width in graphic state to given value.
 * Value is given in user coordinates, so effective line width depends on
 * CTM at the moment of #gnome_print_stroke or #gnome_print_strokepath.
 * Line width is always uniform in all directions, regardless of stretch
 * factor of CTM.
 * Default line width is 1.0 in user coordinates.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_setlinewidth (GnomePrintContext *pc, double width)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);

	gp_gc_set_linewidth (pc->gc, width);

	return GNOME_PRINT_OK;
}

/**
 * gnome_print_setmiterlimit:
 * @pc: A #GnomePrintContext
 * @limit: Miter limit in degrees
 *
 * Sets minimum angle between two lines, in which case miter join is
 * used. For smaller angles, join is beveled.
 * Default miter limit is 4 degrees.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_setmiterlimit (GnomePrintContext *pc, double limit)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);

	gp_gc_set_miterlimit (pc->gc, limit);

	return GNOME_PRINT_OK;
}

/**
 * gnome_print_setlinejoin:
 * @pc: A #GnomePrintContext
 * @jointype: Integer indicating join type
 *
 * Sets join type for non-colinear line segments.
 * 0 - miter
 * 1 - round
 * 2 - bevel
 * Default join type is miter.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_setlinejoin (GnomePrintContext *pc, int jointype)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);

	gp_gc_set_linejoin (pc->gc, jointype);

	return GNOME_PRINT_OK;
}

/**
 * gnome_print_setlinecap:
 * @pc: A #GnomePrintContext
 * @captype: Integer indicating cap type
 *
 * Sets cap type for line endpoints.
 * 0 - butt
 * 1 - round
 * 2 - square
 * Default cap type is butt.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_setlinecap (GnomePrintContext *pc, int captype)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);

	gp_gc_set_linecap (pc->gc, captype);

	return GNOME_PRINT_OK;
}

/**
 * gnome_print_setdash:
 * @pc: A #GnomePrintContext
 * @n_values: Number of dash segment lengths
 * @values: Array of dash segment lengths
 * @offset: Line starting offset in dash
 *
 * Sets line dashing to given pattern. If n_dash is odd, the result is,
 * as if actual number of segments is 2 times bigger, and 2 copies
 * of dash arrays concatenated.
 * If n_values is 0, line is set solid.
 * Dash segment lengths are given in user coordinates, so the actual
 * dash lengths depend on CTM at the time of #gnome_print_stroke or
 * #gnome_print_strokepath. Dashing is always uniform in all directions,
 * regardless of the stretching factor of CTM.
 * Default is solid line.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_setdash (GnomePrintContext *pc, int n_values, const double *values, double offset)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail ((n_values == 0) || (values != NULL), GNOME_PRINT_ERROR_BADVALUE);

	gp_gc_set_dash (pc->gc, n_values, values, offset);

	return GNOME_PRINT_OK;
}

/**
 * gnome_print_setfont:
 * @pc: A #GnomePrintContext
 * @font: #GnomeFont to use for text
 *
 * Sets font in graphic state. Font is referenced by gnome print,
 * so caller may discard it immediately afterwards.
 * Default font is system dependent.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_setfont (GnomePrintContext *pc, const GnomeFont *font)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (font != NULL, GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail (GNOME_IS_FONT (font), GNOME_PRINT_ERROR_BADVALUE);

	gp_gc_set_font (pc->gc, (GnomeFont *) font);

	return GNOME_PRINT_OK;
}

/**
 * gnome_print_glyphlist:
 * @pc: A #GnomePrintContext
 * @glyphlist: #GnomeGlyphList text object
 *
 * Draws text, using #GnomeGlyphList rich text format.
 * Glyphlist is rendered in user coordinates, starting from
 * currentpoint.
 * Both currentpath and currentpoint are erased.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_glyphlist (GnomePrintContext *pc, GnomeGlyphList * glyphlist)
{
	const gdouble *ctm;
	const ArtPoint *cp;
	gdouble affine[6];
	gint ret;

	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (gp_gc_has_currentpoint (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPOINT);
	g_return_val_if_fail (glyphlist != NULL, GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail (GNOME_IS_GLYPHLIST (glyphlist), GNOME_PRINT_ERROR_BADVALUE);

	ctm = gp_gc_get_ctm (pc->gc);
	cp = gp_gc_get_currentpoint (pc->gc);

	affine[0] = ctm[0];
	affine[1] = ctm[1];
	affine[2] = ctm[2];
	affine[3] = ctm[3];
	affine[4] = cp->x;
	affine[5] = cp->y;

	ret = gnome_print_glyphlist_transform (pc, affine, glyphlist);

	gp_gc_newpath (pc->gc);

	return ret;
}

/**
 * gnome_print_show:
 * @pc: A #GnomePrintContext
 * @text: Null-terminated UTF-8 string
 *
 * Draws UTF-8 text at currentpoint, using current font from graphic
 * state.
 * Input text is validated, and #GNOME_PRINT_ERROR_BADVALUE returned,
 * if it is not valid UTF-8.
 * Both currentpath and currentpoint are erased.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_show (GnomePrintContext *pc, const guchar *text)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (text != NULL, GNOME_PRINT_ERROR_BADVALUE);

	return gnome_print_show_sized (pc, text, strlen (text));
}

/**
 * gnome_print_show_sized:
 * @pc: A #GnomePrintContext
 * @text: UTF-8 text string
 * @bytes: Number of bytes to use from string
 *
 * Draws UTF-8 text at currentpoint, using current font from graphic
 * state.
 * Input text is validated, and #GNOME_PRINT_ERROR_BADVALUE returned,
 * if it is not valid UTF-8.
 * Both currentpath and currentpoint are erased.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_show_sized (GnomePrintContext *pc, const guchar *text, int bytes)
{
	const GnomeFont *font;
	const char *invalid;
	GnomeGlyphList *gl;
	gint ret;

	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);
	g_return_val_if_fail (gp_gc_has_currentpoint (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPOINT);
	g_return_val_if_fail (text != NULL, GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail (bytes >= 0, GNOME_PRINT_ERROR_BADVALUE);

	if (bytes < 1)
		return GNOME_PRINT_OK;

	g_return_val_if_fail (g_utf8_validate (text, bytes, &invalid), GNOME_PRINT_ERROR_TEXTCORRUPT);

	font = gp_gc_get_font (pc->gc);
	g_return_val_if_fail (font != NULL, GNOME_PRINT_ERROR_UNKNOWN);

	gl = gnome_glyphlist_from_text_sized_dumb ((GnomeFont *) font, gp_gc_get_rgba (pc->gc), 0.0, 0.0, text, bytes);
	ret = gnome_print_glyphlist (pc, gl);
	gnome_glyphlist_unref (gl);

	gp_gc_newpath (pc->gc);

	return ret;
}

/**
 * gnome_print_scale:
 * @pc: A #GnomePrintContext
 * @sx: X scale
 * @sy: Y scale
 *
 * Scales user coordinate system by given X and Y values.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_scale (GnomePrintContext *pc, double sx, double sy)
{
	gdouble dst[6];

	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);

	art_affine_scale (dst, sx, sy);

	return gnome_print_concat (pc, dst);
}

/**
 * gnome_print_rotate:
 * @pc: A #GnomePrintContext
 * @theta: Angle in degrees
 *
 * Rotates user coordinate system theta degrees counterclockwise.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
gint
gnome_print_rotate (GnomePrintContext *pc, gdouble theta)
{
	gdouble dst[6];

	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);

	art_affine_rotate (dst, theta);

	return gnome_print_concat (pc, dst);
}

/**
 * gnome_print_translate:
 * @pc: A #GnomePrintContext
 * @x: New starting X
 * @y: New starting Y
 *
 * Move the starting point of user coordinate system to given point.
 *
 * Returns: #GNOME_PRINT_OK or positive value on success, negative error
 * code on failure.
 */
int
gnome_print_translate (GnomePrintContext *pc, double x, double y)
{
	gdouble dst[6];

	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->haspage, GNOME_PRINT_ERROR_NOPAGE);

	art_affine_identity (dst);
	dst[4] = x;
	dst[5] = y;

	return gnome_print_concat (pc, dst);
}

/*
 * Convenience methods
 */

gint
gnome_print_line_stroked (GnomePrintContext *pc, gdouble x0, gdouble y0, gdouble x1, gdouble y1)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);

	gp_gc_newpath (pc->gc);
	gp_gc_moveto (pc->gc, x0, y0);
	gp_gc_lineto (pc->gc, x1, y1);

	return gnome_print_stroke (pc);
}

gint
gnome_print_rect_stroked (GnomePrintContext *pc, gdouble x, gdouble y, gdouble width, gdouble height)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);

	gp_gc_newpath (pc->gc);
	gp_gc_moveto (pc->gc, x, y);
	gp_gc_lineto (pc->gc, x + width, y);
	gp_gc_lineto (pc->gc, x + width, y + height);
	gp_gc_lineto (pc->gc, x, y + height);
	gp_gc_lineto (pc->gc, x, y);
	gp_gc_closepath (pc->gc);

	return gnome_print_stroke (pc);
}

gint
gnome_print_rect_filled (GnomePrintContext *pc, gdouble x, gdouble y, gdouble width, gdouble height)
{
	g_return_val_if_fail (pc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (pc->gc != NULL, GNOME_PRINT_ERROR_BADCONTEXT);

	gp_gc_newpath (pc->gc);
	gp_gc_moveto (pc->gc, x, y);
	gp_gc_lineto (pc->gc, x + width, y);
	gp_gc_lineto (pc->gc, x + width, y + height);
	gp_gc_lineto (pc->gc, x, y + height);
	gp_gc_lineto (pc->gc, x, y);
	gp_gc_closepath (pc->gc);

	return gnome_print_fill (pc);
}


