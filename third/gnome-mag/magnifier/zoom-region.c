/*
 * AT-SPI - Assistive Technology Service Provider Interface
 * (Gnome Accessibility Project; http://developer.gnome.org/projects/gap)
 *
 * Copyright 2001 Sun Microsystems Inc.
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

#include <stdlib.h>
#include <string.h>
#include <popt.h>
#include <sys/time.h> /* TODO: replace timeofday with g_timers */
#include <gdk/gdkwindow.h>
#include <gtk/gtk.h>
#ifdef USE_GDKPIXBUF_RENDER_TO_DRAWABLE
#include <gdk/gdkpixbuf.h>
#else
#include <gdk/gdk.h>
#endif
#include <gdk/gdkx.h>
#include <gdk/gdkrgb.h>
#include <libbonobo.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/extensions/XTest.h>
#include <math.h>

#undef ZOOM_REGION_DEBUG

#include "zoom-region.h"
#include "zoom-region-private.h"
#include "magnifier.h" /* needed to access parent data */
#include "magnifier-private.h" /* needed to access parent data */

#define DEBUG_CLIENT_CALLS

#ifdef DEBUG_CLIENT_CALLS
static gboolean client_debug = FALSE;
#define DBG(a) if (client_debug) { (a); }
#else
#define DBG(a) 
#endif

static GObjectClass *parent_class = NULL;

enum {
	ZOOM_REGION_MANAGED_PROP,
	ZOOM_REGION_SMOOTHSCROLL_PROP,
	ZOOM_REGION_INVERT_PROP,
	ZOOM_REGION_SMOOTHING_PROP,
	ZOOM_REGION_CONTRAST_PROP,
	ZOOM_REGION_XSCALE_PROP,
	ZOOM_REGION_YSCALE_PROP,
	ZOOM_REGION_BORDERSIZE_PROP,
	ZOOM_REGION_BORDERCOLOR_PROP,
	ZOOM_REGION_XALIGN_PROP,
	ZOOM_REGION_YALIGN_PROP,
	ZOOM_REGION_VIEWPORT_PROP,
	ZOOM_REGION_TESTPATTERN_PROP,
	ZOOM_REGION_TIMING_TEST_PROP,
	ZOOM_REGION_TIMING_OUTPUT_PROP,
	ZOOM_REGION_TIMING_PAN_RATE_PROP,
	ZOOM_REGION_EXIT_MAGNIFIER
} PropIdx;

#ifdef DEBUG_CLIENT_CALLS
gchar* prop_names[ZOOM_REGION_EXIT_MAGNIFIER + 1] = 
{
    "MANAGED",
    "SMOOTHSCROLL",
    "INVERT",
    "SMOOTHING",
    "CONTRAST",
    "XSCALE",
    "YSCALE",
    "BORDERSIZE",
    "BORDERCOLOR",
    "XALIGN",
    "YALIGN",
    "VIEWPORT",
    "TESTPATTERN",
    "TIMING_TEST",
    "TIMING_OUTPUT",
    "TIMING_PAN_RATE",
    "EXIT_MAGNIFIER"
};
#endif

typedef enum {
	ZOOM_REGION_ERROR_NONE,
	ZOOM_REGION_ERROR_NO_TARGET_DRAWABLE,
	ZOOM_REGION_ERROR_TOO_BIG
} ZoomRegionPixmapCreationError;

/*
 * When running the magnifier in timing test mode (--timing-pan-rate and/or --timing-iterations),
 * then the following output is displayed:
 *
 *   Pan Rate                 = 200
 *   Frames Processed         = 1225
 *   Width/Height/Depth       = 640/1024/8
 *   Zoom Factor (x/y)        = 3.000000/3.000000
 *   Update Duration          = (avg. 0.010232) (max. 0.002685) (tot. 1.084589) seconds
 *     Update Pixels          = (avg. 3046) pixels/frame
 *     Update Rate            = (avg. 97.732909) (max. 372.439485) updates/second
 *     Net Update Rate        = (avg. 0.297724) (max. 0.457741) Mpex/second
 *   Pan Latency              = (avg. 0.008229) (max. 0.562326) seconds
 *   Total Frame Duration     = (avg. 0.008363) (max. 0.562485) (tot. 10.235981) seconds
 *   Frame Rate               = (avg. 119.578189) (max. 4854.369141) frames/second
 *   Scroll Delta (x)         = (avg. 0.000000) (tot. 0) lines
 *   Scroll Delta (y)         = (avg. 5.019608) (tot. 6144) lines
 *   Scroll Rate (x)          = (avg. 0.000000) lines/second
 *   Scroll Rate (y)          = (avg. 600.235657) lines/second
 *   Net Render Rate          = (avg. 8.685322) (max. 3181.359360) Mpex/second
 *
 * Pan Rate             - Displays the pan rate specified on the command line.
 *                        Only displayed if --timing-pan-rate is used.
 * Frames Processed     - Number of frames displayed
 * Width/Height/Depth   - The width/height/depth of magnifier display.
 * Zoom Factor (x/y)    - The zoom factor used.
 * Update Duration      - Average and total time spent updating new sections of the screen.
 *   Update Pixels      - Number of pixels updated
 *   Update Rate        - Number of updates possible per second.
 *   Net Update Rate    - Number of Megapixels updated per second.
 * Pan Latency          - Time spent from the beginning of the setROI request until GTK+
 *                        has finished processing the update (in GDK's
 *                        gdk_window_update_idle idle handler.
 * Total Frame Duration - Average and total time processing frames.
 * Frame Rate           - Number of frames per second.
 * Scroll Delta         - Average and total number of lines scrolled.
 * Scroll Rate          - Number of lines scrolled per second.  Note that this value
 *                        will be equal to the --timing-pan-rate value times the
 *                        y zoom factor.
 * Net Render Rate      - Number of Megapixels processed per second.
 *
 * When running with the --timing-output argument, similar information is displayed for
 * each frame plus timing information for the frame.  Update information is not
 * displayed for frames where updates do not occur.  Pan Rate, Frames Processed,
 * and Width/Height/Depth is not displayed for each frame, only when displaying
 * summary information.
 */

typedef struct timing {
	struct timeval process_start;
	struct timeval process_end;
	struct timeval frame_start;
	struct timeval frame_end;
	struct timeval scale_start;
	struct timeval scale_end;
	struct timeval idle_start;
	struct timeval idle_end;
	float scale_val;
	float idle_val;
	float frame_val;
	int   dx;
	int   dy;
	float scale_total;
	float idle_total;
	float frame_total;
	int   dx_total;
	int   dy_total;
	long  update_pixels_total;
	long  num_scale_samples;
	long  num_idle_samples;
	long  num_frame_samples;
	long  num_line_samples;
	float last_frame_val;
	int   last_dy;
} TimingStruct;

static float timing_scale_max  = 0;
static float timing_idle_max   = 0;
static float timing_frame_max  = 0;
static float cps_max           = 0;
static float nrr_max           = 0;
static float update_nrr_max    = 0;
static gboolean	reset_timing   = FALSE;
static gboolean timing_test    = FALSE;

static guint pending_idle_handler = 0;
static TimingStruct mag_timing;
static gboolean processing_updates = FALSE;
static gboolean timing_start = FALSE;

#ifdef TEST_XTST_CURSOR
static Cursor *x_cursors;
static Window cursor_window = None;
#endif

static gboolean can_coalesce = TRUE ; /* change this when event coalescing is working */

static void zoom_region_sync (ZoomRegion *region);
static void zoom_region_finalize (GObject *object);
static void zoom_region_update (ZoomRegion *zoom_region,
				const GdkRectangle rect);
static void zoom_region_queue_update (ZoomRegion *zoom_region,
				      const GdkRectangle rect);

static int  zoom_region_process_updates (gpointer data);
static void zoom_region_paint (ZoomRegion *zoom_region, GdkRectangle *rect);
static void zoom_region_paint_pixmap (ZoomRegion *zoom_region, GdkRectangle *rect);
static int  zoom_region_update_pointer_timeout (gpointer data);
static void zoom_region_recompute_exposed_viewport (ZoomRegion *zoom_region);

static GdkRectangle zoom_region_rect_from_bounds (ZoomRegion *zoom_region,
						  const GNOME_Magnifier_RectBounds *bounds);
static ZoomRegionPixmapCreationError zoom_region_create_pixmap (ZoomRegion *zoom_region);

void
reset_timing_stats()
{
	timing_scale_max               = 0;
	timing_idle_max                = 0;
	timing_frame_max               = 0;
	cps_max                        = 0;
	nrr_max                        = 0;
	update_nrr_max                 = 0;
	mag_timing.num_scale_samples   = 0;
	mag_timing.num_idle_samples    = 0;
	mag_timing.num_frame_samples   = 0;
	mag_timing.num_line_samples    = 0;
	mag_timing.scale_total         = 0;
	mag_timing.idle_total          = 0;
	mag_timing.frame_total         = 0;
	mag_timing.update_pixels_total = 0;
	mag_timing.update_pixels_total = 0;
	mag_timing.dx_total            = 0;
	mag_timing.dy_total            = 0;
	mag_timing.last_frame_val      = 0;
	mag_timing.last_dy             = 0;
	gettimeofday(&mag_timing.process_start, NULL);
}

/** DEBUG STUFF **/

#undef DEBUG
#ifdef DEBUG
#define DEBUG_RECT(a, b) _debug_announce_rect (a, b)
#else
#define DEBUG_RECT(a, b) 
#endif
static void
_debug_announce_rect (char *msg, GdkRectangle rect)
{
	fprintf (stderr, "%s: (%d,%d - %d,%d)\n",
		 msg, rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
}

#ifdef TEST_XTST_CURSOR
static Cursor *
_get_x_cursors (Display *display)
{
	int n_x_font_cursors = XC_num_glyphs; 
	int dummy;
	if (!XTestQueryExtension (display, &dummy, &dummy,&dummy, &dummy))
		fprintf (stderr, "WARNING: XTEST extension not available!\n");
	if (x_cursors == NULL && display) {
		int i;
		x_cursors = g_new0 (Cursor, XC_num_glyphs);
		for (i = 2; i < n_x_font_cursors; i += 2)
		{
			if (i == 30)
				x_cursors[i] = XCreateFontCursor (display, XC_xterm);
			else
				x_cursors[i] = XCreateFontCursor (display, i);
		}
	}
	return x_cursors;
}
#endif

static gboolean
_diff_pixbufs (const GdkPixbuf *a, const GdkPixbuf *b)
{
	long i, j;
	int bits_per_byte = 8; /* always true? */
	guchar *pa = gdk_pixbuf_get_pixels (a);
	guchar *pb = gdk_pixbuf_get_pixels (b);
	guchar *cpa, *cpb;
	long rsa = gdk_pixbuf_get_rowstride (a);
	long rsb = gdk_pixbuf_get_rowstride (b);
	long rowbytes = gdk_pixbuf_get_width (a) *
		gdk_pixbuf_get_bits_per_sample (a) *
		gdk_pixbuf_get_n_channels (a)/ bits_per_byte;
	long n_rows = gdk_pixbuf_get_height (a);

	if (gdk_pixbuf_get_height (b) != n_rows)
		return TRUE;
	if (gdk_pixbuf_get_width (b) != gdk_pixbuf_get_width (a))
		return TRUE;
	for (j = 0; j < n_rows; ++j)
	{
		cpa = pa + j * rsa;
		cpb = pb + j * rsb;
		for (i = 0; i < rowbytes; ++i)
		{
			if (*cpa != *cpb)
			{
				return TRUE;
			}
			cpa++;
			cpb++;
		}		
	}
	return FALSE;
}

/** EVENT COALESCING **/

#ifdef BROKEN_COALESCE_STUFF_GETS_FIXED
/**
 * _combine_rects:
 * combines two GdkRectangles IFF the union of the two form
 * a rectangle.
 * a: the first GdkRectangle, which will be changed to the new bounds if
 *    coalesce operation can be performed, otherwise unchanged.
 * b: the second GdkRectangle.
 * returns: True if the two are coalesced, FALSE otherwise.
 **/
static gboolean
_combine_rects (GdkRectangle *a, GdkRectangle *b)
{
	gboolean can_combine = FALSE;
	if ((a->x == b->x) && (a->x + a->width == b->x + b->width))
	{
		can_combine = TRUE;
	}
	else if ((a->y == b->y) && (a->y + a->height == b->y + b->height))
	{
		can_combine = TRUE;
	}
	if (can_combine)
	{
		GdkRectangle c;
		/* TODO: check and fix this */
		if (gdk_rectangle_intersect (a, b, &c))
		{
			gdk_rectangle_union (a, b, &c);
			*a = c;
			can_combine = TRUE;
		}
		else
		{
			can_combine = FALSE;
		}
	}
	return can_combine;
}

/**
 * _refactor_rects:
 * Refactor GdkRectangles whose union forms an 'L' shape, swapping
 * the long and short elements. e.g. turns
 *
 *    xxx            xxx
 *    xxx            xxx
 * oooooo  into   oooxxx
 * oooooo         oooxxx
 *
 * returns: TRUE if the refactor was performed, FALSE if it could not be
 *          completed (i.e. if the rectangles did not form a suitable union).
 **/
static gboolean
_refactor_rects (GdkRectangle *p, GdkRectangle *n)
{
	gboolean refactored = FALSE;
	GdkRectangle *a, *b;
	if (p->x == n->x)
	{
		if (p->width < n->width)
		{
			a = p;
			b = n;
		}
		else
		{
			a = n;
			b = p;
		}
		if (a->y == b->y + b->height)
		{
			a->y -= b->height;
			a->height += b->height;
			b->x += a->width;
			b->width -= a->width;
			refactored = TRUE;
		}
		else if (a->y + a->height == b->y)
		{
			a->height += b->height;
			b->x += a->width;
			b->width -= a->width;
			refactored = TRUE;
		}
		if (refactored) fprintf (stderr, "REFACTOR 1\n");
	}		
	else if (p->y == n->y)
	{
		if (p->height < n->height)
		{
			a = p;
			b = n;
		}
		else
		{
			a = n;
			b = p;
		}
		if (a->x == b->x + b->width)
		{
			a->x -= b->width;
			a->width += b->width;
			b->y += a->height;
			b->height -= a->height;
			refactored = TRUE;
		}
		else if (a->x + a->width == b->x)
		{
			a->width += b->width;
			b->y += a->height;
			b->height -= a->height;
			refactored = TRUE;
		}
		if (refactored) fprintf (stderr, "REFACTOR 2\n");
	}
	else if (p->x + p->width == n->x + n->width)
	{
		if (p->width < n->width)
		{
			a = p;
			b = n;
		}
		else
		{
			a = n;
			b = p;
		}
		if (a->y == b->y + b->height)
		{
			a->y -= b->height;
			a->height += b->height;
			b->width -= a->width;
			refactored = TRUE;
		}
		else if (a->y + a->height == b->y)
		{
			a->height += b->height;
			b->width -= a->width;
			refactored = TRUE;
		}
		if (refactored) fprintf (stderr, "REFACTOR 3\n");
	}
	else if (p->y + p->height == n->y + n->height)
	{
		if (p->height < n->height)
		{
			a = p;
			b = n;
		}
		else
		{
			a = n;
			b = p;
		}
		if (a->x == b->x + b->width)
		{
			a->x -= b->width;
			a->width += b->width;
			b->height -= a->height;
			refactored = TRUE;
		}
		else if (a->x + a->width == b->x)
		{
			a->width += b->width;
			b->height -= a->height;
			refactored = TRUE;
		}
		if (refactored) fprintf (stderr, "REFACTOR 4\n");
	}
	return refactored;
}

static GList*
_combine_update_rects (GList *q, int lookahead_n)
{
	int i = 0;
	GdkRectangle *a = q->data;
	GList *p = q;
	while (i < lookahead_n && p && p->next)
	{
		if (_combine_rects (a, q->next->data))
		{
			q = g_list_delete_link (q, p->next);
		}
		else
		{
			p = p->next;
			++i;
		}
	}
	return q;
}
#endif

/*#define _is_horizontal_rect(r)   (((2 * (r)->width / 3 * (r)->height)) > 1)*/
/*#define _is_vertical_rect(r)   (((2 * (r)->height / 3 * (r)->width)) > 1)*/
#define _is_horizontal_rect(r) ((r)->width > (r)->height) 
#define _is_vertical_rect(r)   ((r)->height > (r)->width)

/**
 * _coalesce_update_rects :
 * coalesces multiple "vertical" rects and "horizontal"
 * rects into one of each.  Can result in overlapping/larger
 * update area for tiled rects, but reduces queue size dramatically.
 **/
static GList *
_coalesce_update_rects (GList *q, int min_coalesce_length)
{
	GdkRectangle *v = NULL, *h = NULL;
	GList *compact_queue = NULL;
/*	fprintf (stderr, "starting queue length = %d\n", g_list_length (q)); */
	if (g_list_length (q) < min_coalesce_length) 
		return g_list_copy (q);
	while (q)
	{
		if (_is_vertical_rect ((GdkRectangle *) (q->data)))
		{
			if (v) gdk_rectangle_union (v, q->data, v);
			else
			{
				v = g_new0 (GdkRectangle, 1);
				*v = *(GdkRectangle *)q->data;
			}
		}
		else if (_is_horizontal_rect ((GdkRectangle *) (q->data)))
		{
			if (h) gdk_rectangle_union (h, q->data, h);
			else
			{
				h = g_new0 (GdkRectangle, 1);
				*h = *(GdkRectangle *)q->data;
			}
		}
		else
			compact_queue = g_list_prepend (compact_queue, q->data);
		q = q->next;
	};
	if (v)
		compact_queue = g_list_prepend (compact_queue, v);
	if (h)
		compact_queue = g_list_prepend (compact_queue, h);
/*	fprintf (stderr, "ending queue length = %d\n", g_list_length (compact_queue));*/
	/* don't free the original queue, that's the caller's responsibility */
	return compact_queue;
}

#ifdef BROKEN_COALESCE_STUFF_GETS_FIXED
static GList *
_smartbutbroken_coalesce_update_rects (GList *q, int lookahead_n)
{
	int i = 0, len;
	fprintf (stderr, "starting queue length = %d\n", g_list_length (q));
	do {
		GdkRectangle *a;
		len = g_list_length (q);
		q = _combine_update_rects (q, lookahead_n);
		a = q->data;
		while (i < lookahead_n && q && q->next)
		{
			if (_refactor_rects (a, q->next->data))
				break;
			else
				++i;
		}
		q = _combine_update_rects (q, lookahead_n);
	} while (g_list_length (q) < len);
	fprintf (stderr, "ending queue length = %d\n", g_list_length (q));
	return q;
}
#endif

/** COORDINATE CONVERSIONS **/

/** clip an area in source coords to the exposed target area **/
static GdkRectangle
_rectangle_clip_to_rectangle (GdkRectangle area,
			      GdkRectangle clip_rect)
{
        GdkRectangle clipped;
        clipped.x = MAX (area.x, clip_rect.x);
        clipped.y = MAX (area.y, clip_rect.y);
        clipped.width = MIN ((area.x + area.width), (clip_rect.x + clip_rect.width)) - clipped.x;
        clipped.height = MIN ((area.y + area.height), (clip_rect.y + clip_rect.height)) - clipped.y;
	return clipped;
}

static GdkRectangle
_rectangle_clip_to_bounds (GdkRectangle area,
			   GNOME_Magnifier_RectBounds *clip_bounds)
{
	area.x = MAX (area.x, clip_bounds->x1);
	area.x = MIN (area.x, clip_bounds->x2);
	area.width = MIN (area.width, clip_bounds->x2 - area.x);
	area.y = MAX (area.y, clip_bounds->y1);
	area.y = MIN (area.y, clip_bounds->y2);
	area.height = MIN (area.height, clip_bounds->y2 - area.y);
	return area;
}

static GdkRectangle
zoom_region_clip_to_source (ZoomRegion *zoom_region,
			    GdkRectangle area)
{
    GNOME_Magnifier_RectBounds *source_rect_ptr;
    if (zoom_region && zoom_region->priv && zoom_region->priv->parent)
    {
	source_rect_ptr = &((Magnifier *)zoom_region->priv->parent)->source_bounds;
        DEBUG_RECT ("clipping to source bounds", zoom_region_rect_from_bounds (zoom_region, source_rect_ptr)); 
	return _rectangle_clip_to_bounds (area, source_rect_ptr);
    }
    return area;
}

static GdkRectangle
zoom_region_clip_to_exposed_target (ZoomRegion *zoom_region,
				    GdkRectangle area)
{
	GNOME_Magnifier_RectBounds onscreen_target;
	onscreen_target.x1 = MAX (zoom_region->priv->exposed_bounds.x1 / zoom_region->xscale,
				  zoom_region->priv->source_area.x1);
	onscreen_target.y1 = MAX (zoom_region->priv->exposed_bounds.y1 / zoom_region->yscale,
				  zoom_region->priv->source_area.y1);
	onscreen_target.x2 = MIN (zoom_region->priv->exposed_bounds.x2 / zoom_region->xscale,
				  zoom_region->priv->source_area.x2);
	onscreen_target.y2 = MIN (zoom_region->priv->exposed_bounds.y2 / zoom_region->yscale,
				  zoom_region->priv->source_area.y2);

	return _rectangle_clip_to_bounds (area, &onscreen_target);
}

#ifdef NEEDED
static GdkRectangle
zoom_region_clip_to_exposed_bounds (ZoomRegion *zoom_region,
				    GdkRectangle area)
{
	return _rectangle_clip_to_bounds (area, &zoom_region->priv->exposed_bounds);
}

static GdkRectangle
zoom_region_clip_to_viewport (ZoomRegion *zoom_region,
			      GdkRectangle area)
{
	return _rectangle_clip_to_bounds (area, &zoom_region->priv->exposed_viewport);
}
#endif

static GdkRectangle
zoom_region_clip_to_scaled_pixmap (ZoomRegion *zoom_region,
				   GdkRectangle area)
{
        GdkRectangle pixmap_area = {0, 0, 0, 0};
	if (zoom_region->priv && zoom_region->priv->pixmap)
	{
	    gdk_drawable_get_size (zoom_region->priv->pixmap, &pixmap_area.width, &pixmap_area.height);
	    return _rectangle_clip_to_rectangle (area, pixmap_area);
	}
	else
	    return area;
}

static GdkRectangle
zoom_region_clip_to_window (ZoomRegion *zoom_region,
			    GdkRectangle area)
{
	GdkRectangle window_rect;

	/* we can just return ATM because _rectangle_clip_to_rectangle is unimplemented now */

	return area;

	if (zoom_region->priv->w->window)
		gdk_drawable_get_size (GDK_DRAWABLE (zoom_region->priv->w->window),
				       &window_rect.x,
				       &window_rect.y);
	else 
	{
		window_rect.x = 0;
		window_rect.y = 0;
	}
	return _rectangle_clip_to_rectangle (area, window_rect);
}

static const GdkRectangle
zoom_region_source_rect_from_view_bounds (ZoomRegion *zoom_region,
					  const GNOME_Magnifier_RectBounds *view_bounds)
{
	GdkRectangle source_rect;
	source_rect.x = floor ((view_bounds->x1 + zoom_region->priv->exposed_bounds.x1)
			       / zoom_region->xscale);
	source_rect.y = floor ((view_bounds->y1 + zoom_region->priv->exposed_bounds.y1)
				/ zoom_region->yscale);
	source_rect.width = ceil ((view_bounds->x2 - view_bounds->x1) / zoom_region->xscale);
	source_rect.height = ceil ((view_bounds->y2 - view_bounds->y1) / zoom_region->yscale);
	return source_rect;
}

static GdkRectangle
zoom_region_view_rect_from_source_rect (ZoomRegion *zoom_region,
					const GdkRectangle source_rect)
{
	GdkRectangle view_rect;
	view_rect.x = source_rect.x * zoom_region->xscale - zoom_region->priv->exposed_bounds.x1;
	view_rect.y = source_rect.y * zoom_region->yscale - zoom_region->priv->exposed_bounds.y1;
	view_rect.width = source_rect.width * zoom_region->xscale;
	view_rect.height = source_rect.height * zoom_region->yscale;
	DEBUG_RECT ("source", source_rect);
	DEBUG_RECT ("converted to view-rect", view_rect);
	return view_rect;
}

static GdkRectangle
zoom_region_source_rect_from_view_rect (ZoomRegion *zoom_region,
					const GdkRectangle view_rect)
{
	GdkRectangle source_rect;
	source_rect.x = floor ((view_rect.x + zoom_region->priv->exposed_bounds.x1)
			       / zoom_region->xscale);
	source_rect.y = floor ((view_rect.y + zoom_region->priv->exposed_bounds.y1)
				/ zoom_region->yscale);
	source_rect.width = ceil (view_rect.width / zoom_region->xscale);
	source_rect.height = ceil (view_rect.height / zoom_region->yscale);
	return source_rect;
}

static GdkRectangle
zoom_region_rect_from_bounds (ZoomRegion *zoom_region,
			      const GNOME_Magnifier_RectBounds *bounds)
{
	GdkRectangle rect;
	rect.x = bounds->x1;
	rect.y = bounds->y1;
	rect.width = bounds->x2 - bounds->x1;
	rect.height = bounds->y2 - bounds->y1;
	return rect;
}

/** **************  **/

static void
zoom_region_queue_update (ZoomRegion *zoom_region,
			  const GdkRectangle update_rect)
{
	GdkRectangle *rect =
		g_new0 (GdkRectangle, 1);
	*rect = update_rect;

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	DEBUG_RECT ("queueing update", *rect);

	zoom_region->priv->q =
		g_list_prepend (zoom_region->priv->q, rect);
	if (zoom_region->priv && zoom_region->priv->update_handler_id == 0)
		zoom_region->priv->update_handler_id = 
			g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
					 zoom_region_process_updates,
					 zoom_region,
					 NULL);
}

static void
zoom_region_update_current (ZoomRegion *zoom_region)
{
#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	if (zoom_region->priv)
	{
		gboolean pixmap_valid = GDK_IS_DRAWABLE (zoom_region->priv->pixmap);
		if (!pixmap_valid)
			pixmap_valid = (zoom_region_create_pixmap (zoom_region) == ZOOM_REGION_ERROR_NONE);
		if (pixmap_valid)
			zoom_region_update (zoom_region,
					    zoom_region_source_rect_from_view_bounds (
						    zoom_region,
						    &zoom_region->priv->exposed_viewport));
	}
}

static GdkRectangle
zoom_region_cursor_rect (ZoomRegion *zoom_region)
{
	GdkRectangle rect = {0, 0, 0, 0};
	Magnifier *magnifier = zoom_region->priv->parent;
	GdkDrawable *cursor = NULL;
	if (magnifier)
		cursor = magnifier_get_cursor (magnifier);
	if (cursor)
	{
		rect.x = zoom_region->priv->last_cursor_pos.x;
		rect.y = zoom_region->priv->last_cursor_pos.y;
		rect = zoom_region_view_rect_from_source_rect (zoom_region, rect);
		rect.x -= magnifier->cursor_hotspot.x;
		rect.y -= magnifier->cursor_hotspot.y;
		gdk_drawable_get_size (cursor, &rect.width, &rect.height);
	}
	return rect;
}

static void
zoom_region_unpaint_crosswire_cursor (ZoomRegion *zoom_region,
				      GdkRectangle *clip_rect)
{
	Magnifier *magnifier = zoom_region->priv->parent;
	GdkRectangle vline_rect, hline_rect;
	GdkPoint cursor_pos;

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	if (!magnifier || magnifier->crosswire_size <= 0) return;

	cursor_pos = zoom_region->priv->last_drawn_crosswire_pos;
	vline_rect.x = cursor_pos.x - magnifier->crosswire_size/2;
	vline_rect.y = clip_rect ? clip_rect->y : 0; 
	vline_rect.width = MAX (magnifier->crosswire_size, 1);
	vline_rect.height = clip_rect ? clip_rect->height : 4096; 
	hline_rect.x = clip_rect ? clip_rect->x : 0; 
	hline_rect.y = cursor_pos.y - magnifier->crosswire_size/2;
	hline_rect.width = clip_rect ? clip_rect->width : 4096;
	hline_rect.height = MAX (magnifier->crosswire_size, 1);

	zoom_region_paint_pixmap (zoom_region, &vline_rect);
	zoom_region_paint_pixmap (zoom_region, &hline_rect);
}

static void
zoom_region_paint_crosswire_cursor (ZoomRegion *zoom_region, GdkRectangle *clip_rect)
{
	Magnifier *magnifier = zoom_region->priv->parent;
	static GdkColormap *cmap;
	static GdkColor last_color;
	static gboolean last_color_init = FALSE;
	GdkGCValues values;
	GdkRectangle rect;
	GdkDrawable *cursor;
	GdkColor color = {0, 0, 0, 0};
	GdkColor tmp_color;
	int x_left_clip = 0, x_right_clip = 0, y_top_clip = 0, y_bottom_clip = 0;
	int csize;
	
#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	if (!(magnifier &&
	      zoom_region->priv->w->window &&
	      GDK_IS_DRAWABLE (zoom_region->priv->w->window) &&
	      magnifier->crosswire_size > 0)) return;

	if (zoom_region->priv->crosswire_gc == NULL) 
	{
		zoom_region->priv->crosswire_gc = gdk_gc_new (zoom_region->priv->w->window);
		cmap = gdk_gc_get_colormap(zoom_region->priv->crosswire_gc);
		last_color_init = FALSE;
	}

	if (magnifier->crosswire_color == 0)
	{
		color.red = 0xFFFF;
		color.blue = 0xFFFF;
		color.green = 0xFFFF;
		values.function = GDK_INVERT;
	}
	else
	{
		color.red = (magnifier->crosswire_color & 0xFF0000) >> 8;
		color.green = (magnifier->crosswire_color & 0xFF00);
		color.blue = (magnifier->crosswire_color & 0xFF) << 8;
		values.function = GDK_COPY;
	}

	values.foreground = color;

	/* Only reset colors if they have changed */
    if (!last_color_init || color.red != last_color.red ||
	    color.blue != last_color.blue || color.green != last_color.green)
	{
		if (cmap)
		{
			gdk_rgb_find_color (cmap, &(values.foreground));
			gdk_gc_set_values(zoom_region->priv->crosswire_gc, &values, GDK_GC_FUNCTION | GDK_GC_FOREGROUND);
		}
		else
		{
			gdk_gc_set_values(zoom_region->priv->crosswire_gc, &values, GDK_GC_FUNCTION);
		}

		last_color.red   = color.red;
		last_color.blue  = color.blue;
		last_color.green = color.green;
		last_color_init  = TRUE;
	}

	rect.x = zoom_region->priv->last_cursor_pos.x;
	rect.y = zoom_region->priv->last_cursor_pos.y;
	rect.width = 0;
	rect.height = 0;
	rect = zoom_region_view_rect_from_source_rect (zoom_region, rect);
	if (clip_rect) gdk_gc_set_clip_rectangle (zoom_region->priv->crosswire_gc, clip_rect);
	else gdk_gc_set_clip_rectangle (zoom_region->priv->crosswire_gc, NULL);

	if (magnifier->crosswire_clip &&
	    (cursor = magnifier_get_cursor (magnifier)))
	{
		gdk_drawable_get_size (cursor, &csize, &csize);
		y_top_clip = rect.y - magnifier->cursor_hotspot.y -
			magnifier->crosswire_size;
		y_bottom_clip = rect.y +
			(csize - magnifier->cursor_hotspot.y) +
			magnifier->crosswire_size;
		x_left_clip = rect.x - magnifier->cursor_hotspot.x -
			magnifier->crosswire_size;
		x_right_clip = rect.x +
			(csize - magnifier->cursor_hotspot.x) +
			magnifier->crosswire_size;
	}
	if (magnifier->crosswire_size == 1)
	{
		if (magnifier->crosswire_clip)
		{
			gdk_draw_line (zoom_region->priv->w->window, zoom_region->priv->crosswire_gc, rect.x, 0,
				       rect.x, y_top_clip);
			gdk_draw_line (zoom_region->priv->w->window, zoom_region->priv->crosswire_gc, 0, rect.y,
				       x_left_clip, rect.y);
		}
		gdk_draw_line (zoom_region->priv->w->window, zoom_region->priv->crosswire_gc, rect.x,
			       y_bottom_clip, rect.x, 4096);
		gdk_draw_line (zoom_region->priv->w->window, zoom_region->priv->crosswire_gc, x_right_clip,
			       rect.y, 4096, rect.y);
	}
	else
	{
		if (magnifier->crosswire_clip )
		{
			gdk_draw_rectangle (zoom_region->priv->w->window, zoom_region->priv->crosswire_gc, TRUE,
					    rect.x - magnifier->crosswire_size / 2,
					    0, magnifier->crosswire_size, y_top_clip);
			gdk_draw_rectangle (zoom_region->priv->w->window, zoom_region->priv->crosswire_gc, TRUE, 0,
					    rect.y - magnifier->crosswire_size / 2,
					    x_left_clip, magnifier->crosswire_size);
		}
		gdk_draw_rectangle (zoom_region->priv->w->window, zoom_region->priv->crosswire_gc, TRUE,
				    rect.x - magnifier->crosswire_size / 2,
				    y_bottom_clip, magnifier->crosswire_size, 4096);
		gdk_draw_rectangle (zoom_region->priv->w->window, zoom_region->priv->crosswire_gc, TRUE, x_right_clip,
				    rect.y - magnifier->crosswire_size / 2,
				    4096, magnifier->crosswire_size);
	}
}

static void
zoom_region_unpaint_cursor (ZoomRegion *zoom_region, GdkRectangle *clip_rect)
{
#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	zoom_region_paint_pixmap (zoom_region, &zoom_region->priv->cursor_backing_rect);
}

static void
zoom_region_paint_cursor (ZoomRegion *zoom_region,
			  GdkRectangle *clip_rect)
{
	GdkGCValues values;
	GdkRectangle rect, intersct;
	GdkRectangle fullscreen;
	Magnifier *magnifier = zoom_region->priv->parent;
	rect = zoom_region_cursor_rect (zoom_region);
#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	if (clip_rect == NULL)
	{
		fullscreen = zoom_region_rect_from_bounds (zoom_region,
							   &zoom_region->viewport);
		clip_rect = &fullscreen;
	}
	/* save the unclipped cursor pos for 'undrawing' the crosswire, the clipped one is no good */
	zoom_region->priv->last_drawn_crosswire_pos.x = rect.x + magnifier->cursor_hotspot.x;
	zoom_region->priv->last_drawn_crosswire_pos.y = rect.y + magnifier->cursor_hotspot.y;

	if (gdk_rectangle_intersect (clip_rect, &rect, &intersct))
	{
		int width = 0, height = 0;
		
		GdkDrawable *cursor = magnifier_get_cursor (magnifier);
		if (!cursor)
			return;
		else if (!GDK_IS_DRAWABLE (cursor)) g_message ("cursor isn't DRAWABLE!");
		zoom_region->priv->cursor_backing_rect = rect;
		if (zoom_region->priv->cursor_backing_pixels) {
			gdk_drawable_get_size (zoom_region->priv->cursor_backing_pixels,
					       &width, &height);
		}
		if (rect.width != width || rect.height != height)
		{
			if (zoom_region->priv->cursor_backing_pixels) {
				g_object_unref (zoom_region->priv->cursor_backing_pixels);
			}
			zoom_region->priv->cursor_backing_pixels =
				gdk_pixmap_new (zoom_region->priv->w->window,
						rect.width,
						rect.height,
						-1);
		}
		if (zoom_region->priv->w->window != NULL)
		{
			if (zoom_region->priv->default_gc == NULL) 
				zoom_region->priv->default_gc = gdk_gc_new(zoom_region->priv->w->window);
			gdk_draw_drawable (zoom_region->priv->cursor_backing_pixels,
				     zoom_region->priv->default_gc,
				     zoom_region->priv->w->window,
				     rect.x,
				     rect.y,
				     0, 0,
				     rect.width,
				     rect.height);
		}
		DEBUG_RECT ("painting", rect);
		if (cursor && zoom_region->priv->w && GDK_IS_DRAWABLE (zoom_region->priv->w->window))
		{
		    if (zoom_region->priv->paint_cursor_gc == NULL)
				zoom_region->priv->paint_cursor_gc = gdk_gc_new (zoom_region->priv->w->window);

			gdk_gc_set_clip_rectangle (zoom_region->priv->paint_cursor_gc, clip_rect);
			values.clip_x_origin = rect.x;
			values.clip_y_origin = rect.y;
			values.clip_mask = magnifier->priv->cursor_mask;
			gdk_gc_set_values(zoom_region->priv->paint_cursor_gc, &values, GDK_GC_CLIP_X_ORIGIN |
					  GDK_GC_CLIP_Y_ORIGIN  | GDK_GC_CLIP_MASK);

			gdk_draw_rectangle (zoom_region->priv->w->window,
					   zoom_region->priv->paint_cursor_gc,
					   TRUE,
					   rect.x, rect.y, rect.width, rect.height);

			gdk_draw_drawable (zoom_region->priv->w->window,
					   zoom_region->priv->paint_cursor_gc,
					   cursor,
					   0, 0,
					   rect.x,
					   rect.y,
					   rect.width,
					   rect.height);
		}
	}
}

/**
 * zoom_region_coalesce_updates:
 * 
 **/
static void
zoom_region_coalesce_updates (ZoomRegion *zoom_region)
{
	/* TODO: lock the queue ? */
	GList *q;
	int lookahead_n = 4; /* 'distance' to look ahead in queue */
	int max_qlen = 50;

	if (zoom_region->priv && zoom_region->priv->q && g_list_length (zoom_region->priv->q) > max_qlen)
	{
		g_list_free (zoom_region->priv->q);
		zoom_region->priv->q = NULL; /* just discard and update everything */
		/* CAUTION: this can be an expensive operation! */
		zoom_region_queue_update (zoom_region, zoom_region_rect_from_bounds
			(zoom_region, &zoom_region->priv->source_area));
	}
	else 

        if (zoom_region->priv && zoom_region->priv->q && 
	    (g_list_length (zoom_region->priv->q) > 1) && can_coalesce)
	{		
		q = g_list_reverse (g_list_copy (zoom_region->priv->q));
		if (q)
		{
			GList *coalesce_copy;
			if (zoom_region->coalesce_func)
			{
				GList *new;
				coalesce_copy = (*zoom_region->coalesce_func) (q, lookahead_n);
				new = g_list_reverse (coalesce_copy);
				g_list_free (zoom_region->priv->q);
				zoom_region->priv->q = new;
			}
			g_list_free (q);
		}
	}
}


static void
zoom_region_paint_border (ZoomRegion *zoom_region,
			  GdkRectangle *area)
{
	GdkColor color;

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	if ((zoom_region->border_size > 0) && (zoom_region->priv->w->window))
	{
		if (!zoom_region->priv->border_gc)
		{
			zoom_region->priv->border_gc = gdk_gc_new (zoom_region->priv->w->window);
			color.red = (zoom_region->border_color & 0xFF0000) >> 8;
			color.green = (zoom_region->border_color & 0xFF00);
			color.blue = (zoom_region->border_color & 0xFF) << 8;
#ifdef DEBUG_BORDER
			fprintf (stderr, "border color triple RGB=%d|%d|%d",
				 color.red, color.green, color.blue);
#endif
			gdk_colormap_alloc_color (gdk_drawable_get_colormap (zoom_region->priv->w->window), 
						  &color, TRUE, TRUE);
			gdk_gc_set_foreground (zoom_region->priv->border_gc, &color);
		}
		gdk_draw_rectangle (zoom_region->priv->w->window,
				    zoom_region->priv->border_gc,
				    TRUE,
				    area->x,
				    area->y,
				    area->width,
				    area->height);
	}
}

static void
zoom_region_paint_pixmap (ZoomRegion *zoom_region,
			  GdkRectangle *area)
{
#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	g_assert (zoom_region->priv);
	g_assert (zoom_region->priv->w);

        if (!GDK_IS_DRAWABLE (zoom_region->priv->w->window)) return;
	if (zoom_region->priv->default_gc == NULL) 
		zoom_region->priv->default_gc = gdk_gc_new (zoom_region->priv->w->window);

	if (zoom_region->priv->pixmap && GDK_IS_DRAWABLE (zoom_region->priv->w->window))
	{
		gdk_draw_drawable (zoom_region->priv->w->window,
				   zoom_region->priv->default_gc,
				   zoom_region->priv->pixmap,
				   area->x + zoom_region->priv->exposed_bounds.x1,
				   area->y + zoom_region->priv->exposed_bounds.y1,
				   area->x,
				   area->y,
				   area->width,
				   area->height);
	}
}

/**
 * Note: clips to region's current GdkWindow.
 **/
static void
zoom_region_paint (ZoomRegion *zoom_region,
		   GdkRectangle *area)
{
	GdkRectangle paint_area;

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	DEBUG_RECT ("painting (clipped)", *area);
	paint_area = zoom_region_clip_to_window (zoom_region, *area);
	zoom_region_paint_border (zoom_region, area);
	zoom_region_paint_pixmap (zoom_region, &paint_area);
	zoom_region_paint_cursor (zoom_region, &paint_area);
	zoom_region_paint_crosswire_cursor (zoom_region, &paint_area);
}

static ZoomRegionPixmapCreationError
zoom_region_create_pixmap (ZoomRegion *zoom_region)
{
#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	if (zoom_region->priv->w && GDK_IS_DRAWABLE (zoom_region->priv->w->window))
	{
		long width = (zoom_region->priv->source_area.x2 -
			      zoom_region->priv->source_area.x1) * zoom_region->xscale;
		long height = (zoom_region->priv->source_area.y2 -
			       zoom_region->priv->source_area.y1) * zoom_region->yscale;
		zoom_region->priv->pixmap =
			gdk_pixmap_new (
				zoom_region->priv->w->window,
				width,
				height,
				gdk_drawable_get_depth (
					zoom_region->priv->w->window));

		if (magnifier_error_check ()) {
			zoom_region->priv->pixmap = NULL;
			return ZOOM_REGION_ERROR_TOO_BIG;
		}

		zoom_region_recompute_exposed_viewport (zoom_region);
#ifdef ZOOM_REGION_DEBUG
		g_message ("create-pixmap-update: %d,%d - %d,%d",
			   zoom_region->priv->exposed_viewport.x1,
			   zoom_region->priv->exposed_viewport.y1,
			   zoom_region->priv->exposed_viewport.x2,
			   zoom_region->priv->exposed_viewport.y2);
#endif

			    DEBUG_RECT("viewport", zoom_region_source_rect_from_view_bounds
					    (zoom_region, &zoom_region->priv->exposed_viewport));
			    DEBUG_RECT("source", zoom_region_rect_from_bounds
					    (zoom_region, &((Magnifier*)zoom_region->priv->parent)->source_bounds));

			    zoom_region_update (zoom_region,
/*				    zoom_region_source_rect_from_view_bounds (
					    zoom_region,
					    &zoom_region->priv->exposed_viewport));
*/
				    zoom_region_rect_from_bounds 
				    (zoom_region, 
				     &((Magnifier *)zoom_region->priv->parent)->source_bounds));
		return ZOOM_REGION_ERROR_NONE;
	}

	return ZOOM_REGION_ERROR_NO_TARGET_DRAWABLE;
}

static void
zoom_region_expose_handler (GtkWindow * w,
			    GdkEventExpose *event,
			    gpointer data)
{
	ZoomRegion *zoom_region = data;
	DEBUG_RECT ("expose", event->area);

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	if (zoom_region->priv->pixmap == NULL)
	{
	        ZoomRegionPixmapCreationError ret; 
		/* TODO: scale down if this fails here */
		while ((ret = zoom_region_create_pixmap (zoom_region)) ==
		    ZOOM_REGION_ERROR_TOO_BIG) {
			zoom_region->xscale -= 1.0;
			zoom_region->yscale -= 1.0;
			zoom_region->priv->pixmap = NULL;
			g_warning ("Scale factor too big to fit in memory; shrinking.");
		}
		if (ret == ZOOM_REGION_ERROR_NO_TARGET_DRAWABLE) 
		    g_warning ("create-pixmap: no target drawable");
	}
	zoom_region_paint (zoom_region, &event->area);
}

static void zoom_region_update_cursor (ZoomRegion *zoom_region, int dx, int dy,
				       GdkRectangle *clip_rect)
{
#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	zoom_region_unpaint_crosswire_cursor (zoom_region, clip_rect);
	zoom_region_unpaint_cursor (zoom_region, clip_rect);
	zoom_region->priv->cursor_backing_rect.x += dx;
	zoom_region->priv->cursor_backing_rect.y += dy;
	zoom_region->priv->last_drawn_crosswire_pos.x += dx;
	zoom_region->priv->last_drawn_crosswire_pos.y += dy;
	zoom_region_paint_cursor (zoom_region, clip_rect);
	zoom_region_paint_crosswire_cursor (zoom_region, clip_rect);
	if (GTK_IS_WIDGET (zoom_region->priv->w) && GDK_IS_WINDOW (zoom_region->priv->w->window))
	    gdk_display_sync (gdk_drawable_get_display (zoom_region->priv->w->window));
}

static gboolean
zoom_region_calculate_scroll_rects (ZoomRegion *zoom_region,
				    int dx, int dy,
				    GdkRectangle *scroll_rect,
				    GdkRectangle *expose_rect_h,
				    GdkRectangle *expose_rect_v)
{
	GdkWindow *window = NULL;
	GdkRectangle rect = {0, 0, 0, 0};
	gboolean retval = TRUE;

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	rect.x = 0;
	rect.y = 0;
	if (zoom_region && zoom_region->priv->w &&
	    zoom_region->priv->w->window)
		window = zoom_region->priv->w->window;
	else
		retval = FALSE;
	if (!window)
		retval = FALSE;

	if (window != NULL)
	  gdk_drawable_get_size (GDK_DRAWABLE (window),
				 &rect.width,
				 &rect.height);

	if ((ABS (dx) >= rect.width) || (ABS (dy) >= rect.height)) {
		*scroll_rect = rect;
		DBG(fprintf (stderr, "deltas too big to scroll\n"));
		retval = FALSE;
	}
	
	scroll_rect->x = MAX (0, dx);
	scroll_rect->y = MAX (0, dy);
	scroll_rect->width = MIN (rect.width + dx, rect.width - dx);
	scroll_rect->height = MIN (rect.height + dy, rect.height - dy);
	
	expose_rect_h->x = 0;
	expose_rect_h->y = (scroll_rect->y == 0) ? scroll_rect->height : 0;
	expose_rect_h->width = rect.width;
	expose_rect_h->height = rect.height - scroll_rect->height;

	expose_rect_v->x = (scroll_rect->x == 0) ? scroll_rect->width : 0;
	expose_rect_v->y = scroll_rect->y;
	expose_rect_v->width = rect.width - scroll_rect->width;
	expose_rect_v->height = scroll_rect->height;

	return retval;
}

static void
zoom_region_scroll_fast (ZoomRegion *zoom_region, int dx, int dy,
			 GdkRectangle *scroll_rect,
			 GdkRectangle *expose_rect_h,
			 GdkRectangle *expose_rect_v)
{
	GdkWindow *window;

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	if (zoom_region->priv->w && zoom_region->priv->w->window)
		window = zoom_region->priv->w->window;
	else {
		processing_updates = FALSE;
		return;
	}
	zoom_region_unpaint_crosswire_cursor (zoom_region, scroll_rect);
	zoom_region_unpaint_cursor (zoom_region, scroll_rect);
	gdk_window_scroll (window, dx, dy);
	zoom_region_paint_cursor (zoom_region, scroll_rect);
	zoom_region_paint_crosswire_cursor (zoom_region, scroll_rect);
	gdk_window_process_updates (window, FALSE);
	/* sync reduces cursor flicker, but slows things down */
	if (zoom_region->smooth_scroll_policy > GNOME_Magnifier_ZoomRegion_SCROLL_FASTEST)
		gdk_display_sync (gdk_drawable_get_display (window)); 
}

static void
zoom_region_scroll_smooth (ZoomRegion *zoom_region, int dx, int dy,
			   GdkRectangle *scroll_rect,
			   GdkRectangle *expose_rect_h,
			   GdkRectangle *expose_rect_v)
{
	GdkWindow *window = NULL;
	GdkRectangle window_rect;

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	if (zoom_region->priv->w && GDK_IS_DRAWABLE (zoom_region->priv->w->window))
		window = zoom_region->priv->w->window;
	else
		return;
	window_rect.x = 0;
	window_rect.y = 0;
	gdk_drawable_get_size (GDK_DRAWABLE (window),
			       &window_rect.width, &window_rect.height);
	gdk_window_begin_paint_rect (window, &window_rect);
	gdk_window_invalidate_rect (window, scroll_rect, FALSE);
	gdk_window_invalidate_rect (window, expose_rect_h, FALSE);
	gdk_window_invalidate_rect (window, expose_rect_v, FALSE);
	gdk_window_process_updates (window, FALSE); 
	gdk_window_end_paint (window);
}

static void
zoom_region_scroll (ZoomRegion *zoom_region, int dx, int dy)
{
	GdkRectangle scroll_rect, expose_rect_h, expose_rect_v;
	gboolean can_scroll;

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	if (timing_test) {
		mag_timing.num_line_samples++;
		mag_timing.dx = abs(dx);
		mag_timing.dy = abs(dy);
		mag_timing.dx_total += mag_timing.dx;
		mag_timing.dy_total += mag_timing.dy;
		if (zoom_region->timing_output) {
			fprintf(stderr, "  Panning Increment (x)    = %d (avg. %f) lines/frame\n",
				mag_timing.dx, (float)mag_timing.dx_total / (float)mag_timing.num_line_samples);
			fprintf(stderr, "  Panning Increment (y)    = %d (avg. %f) lines/frame\n",
				mag_timing.dy, (float)mag_timing.dy_total / (float)mag_timing.num_line_samples);
		}
	}

    /*
     * Currently processing a screen update.  This flag used to disallow
     * other updates to occur until this one finishes
     */
    processing_updates = TRUE;

	can_scroll = zoom_region_calculate_scroll_rects (zoom_region, dx, dy,
							 &scroll_rect,
							 &expose_rect_h,
							 &expose_rect_v);
	if (zoom_region->smooth_scroll_policy > GNOME_Magnifier_ZoomRegion_SCROLL_FAST)
	{
		zoom_region_scroll_smooth (zoom_region,
					   dx, dy, &scroll_rect,
					   &expose_rect_h, &expose_rect_v);
	} else {
		zoom_region_scroll_fast (zoom_region,
					 dx, dy, &scroll_rect,
					 &expose_rect_h, &expose_rect_v);
	}
	if (!can_scroll)	
	{
		zoom_region_queue_update (zoom_region,
					  zoom_region_source_rect_from_view_rect (zoom_region,
										  scroll_rect));
	}
	else
	{
		zoom_region_queue_update (zoom_region,
					  zoom_region_source_rect_from_view_rect (zoom_region,
										  expose_rect_h));
		zoom_region_queue_update (zoom_region,
					  zoom_region_source_rect_from_view_rect (zoom_region,
										  expose_rect_v));
	}
}

static void
zoom_region_recompute_exposed_viewport (ZoomRegion *zoom_region)
{
	zoom_region->priv->exposed_viewport.x1 = zoom_region->viewport.x1
		+ zoom_region->border_size;
	zoom_region->priv->exposed_viewport.y1 = zoom_region->viewport.y1
		+ zoom_region->border_size;;
	zoom_region->priv->exposed_viewport.x2 = zoom_region->viewport.x2
		- zoom_region->border_size;;
	zoom_region->priv->exposed_viewport.y2 = zoom_region->viewport.y2
		- zoom_region->border_size;
}

static void
zoom_region_recompute_exposed_bounds (ZoomRegion *zoom_region)
{
	zoom_region->priv->exposed_bounds.x2 = zoom_region->priv->exposed_bounds.x1
		+ (zoom_region->viewport.x2 - zoom_region->viewport.x1);
	zoom_region->priv->exposed_bounds.y2 = zoom_region->priv->exposed_bounds.y1
		+ (zoom_region->viewport.y2 - zoom_region->viewport.y1);
}

static void
zoom_region_set_cursor_pos (ZoomRegion *zoom_region, int x, int y)
{
	if (zoom_region->priv)
	{
		zoom_region->priv->last_cursor_pos.x = x;
		zoom_region->priv->last_cursor_pos.y = y;
	}
}

#ifdef TEST_XTST_CURSOR
static GdkWindow *
zoom_region_source_root (ZoomRegion *zoom_region)
{
	GdkScreen *source_screen = NULL;
	if (zoom_region->priv->source_drawable)
		source_screen =
			gdk_drawable_get_screen (zoom_region->priv->source_drawable);
	if (source_screen)
		return gdk_screen_get_root_window (source_screen);
	else return NULL;
}
#endif

static gboolean
zoom_region_update_pointer (ZoomRegion *zoom_region, gboolean draw_cursor)
{
	Magnifier *magnifier;
	gint mouse_x_return, mouse_y_return;
	guint mask_return;

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	if (!zoom_region->priv || !zoom_region->priv->parent)
	      return FALSE; 

	magnifier = zoom_region->priv->parent;

	/* TODO: there's really no reason we should be using magnifier->priv->root here */
	if (magnifier && magnifier->priv && magnifier_get_root (magnifier))
	{
		gdk_window_get_pointer (
			magnifier_get_root (magnifier),
			&mouse_x_return,
			&mouse_y_return,
			&mask_return);
		
		if (zoom_region->priv->last_cursor_pos.x != mouse_x_return
		    || zoom_region->priv->last_cursor_pos.y != mouse_y_return)
		{
			zoom_region_set_cursor_pos (zoom_region,
						    mouse_x_return, mouse_y_return);
			if (draw_cursor)
			{
				GdkRectangle paint_area, *clip = NULL;

				if (GTK_IS_WIDGET (zoom_region->priv->w) && 
				    GDK_IS_DRAWABLE (zoom_region->priv->w->window))
				{
					gdk_drawable_get_size (
						GDK_DRAWABLE (
							zoom_region->priv->w->window),
						&paint_area.width, &paint_area.height);
					paint_area.x = 0;
					paint_area.y = 0;
					clip = &paint_area;
					paint_area = zoom_region_clip_to_source (
						zoom_region, paint_area);
				}
				zoom_region_update_cursor (zoom_region, 0, 0, clip);
			}
			return TRUE;
		}
	}	
	return FALSE;
}

static int
zoom_region_update_pointer_idle (gpointer data)
{
	ZoomRegion *zoom_region = (ZoomRegion *) data;

	if (zoom_region_update_pointer (zoom_region, TRUE))
	        return TRUE;
	else {
		if (zoom_region->priv)
			zoom_region->priv->update_pointer_id =
			    g_timeout_add_full (G_PRIORITY_DEFAULT,
						100,
						zoom_region_update_pointer_timeout,
						zoom_region,
						NULL);
                return FALSE;
	}
}

static int
zoom_region_update_pointer_timeout (gpointer data)
{
	ZoomRegion *zoom_region = data;

	if (zoom_region->priv && zoom_region_update_pointer (zoom_region, TRUE)) {
	    zoom_region->priv->update_pointer_id =
	        g_idle_add_full (G_PRIORITY_HIGH_IDLE,
				 zoom_region_update_pointer_idle,
				 data,
				 NULL);
		return FALSE;
	} else 
		return TRUE;
}

static void
zoom_region_moveto (ZoomRegion *zoom_region,
		    const long x, const long y)
{
	long dx = x * zoom_region->xscale - zoom_region->priv->exposed_bounds.x1;
	long dy = y * zoom_region->yscale - zoom_region->priv->exposed_bounds.y1;
#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
/* fprintf (stderr, "moveto %ld %ld\n", x, y); */

	mag_timing.dx = 0;
	mag_timing.dy = 0;

	if ((dx != 0) || (dy != 0)) {
		zoom_region_update_pointer (zoom_region, FALSE);
		zoom_region->priv->exposed_bounds.x1 = x * zoom_region->xscale;
		zoom_region->priv->exposed_bounds.y1 = y * zoom_region->yscale;
		zoom_region_recompute_exposed_bounds (zoom_region);
		zoom_region_scroll (zoom_region,
				    -dx, -dy);
	}
}

#define invert_rgb(a) \
(~(a & 0x00FFFFFF) | (a & 0xFF000000))

#define GET_PIXEL(a,i,j,s,b) \
(*(guint32 *)(memcpy (b,(a) + ((j) * s + (i) * pixel_size_t), pixel_size_t)))

#define PUT_PIXEL(a,i,j,s,b) \
(memcpy (a + ((j) * s + (i) * pixel_size_t), &(b), pixel_size_t))

static void
_zoom_region_invert_pixbuf (GdkPixbuf *pixbuf)
{
	int rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	int i, j;
	int w = gdk_pixbuf_get_width (pixbuf);
	int h = gdk_pixbuf_get_height (pixbuf);
	guchar *pixels = gdk_pixbuf_get_pixels (pixbuf);
	guint32 pixval = 0;
	size_t pixel_size_t = 3; /* FIXME: invalid assumption ? */

	for (j = 0; j < h; ++j) {
		for (i = 0; i < w; ++i) {
			pixval = invert_rgb (GET_PIXEL (pixels, i, j, rowstride, &pixval));
			PUT_PIXEL (pixels, i, j, rowstride, pixval);
		}
	}
}

static void
zoom_region_post_process_pixbuf (ZoomRegion *zoom_region,
				 GdkPixbuf *subimage,
				 GdkPixbuf *scaled_image)
{
	/* nothing yet */
	/**
	 * ADI: This is where your image smoothing code
	 * hooks into the magnifier.  This routine can call others which
	 * post-process the GdkPixbuf before rendering to the screen.
	 *
	 * We can make this a two-stage process also, whereby the post-process code
	 * is only called in a lower-priority idle handler, and queues repaints
	 * when it is done.
	 **/
}

static GdkPixbuf *
zoom_region_get_source_subwindow (ZoomRegion *zoom_region,
				  const GdkRectangle bounds)
{
	int i, j, width, height;
	Magnifier *magnifier = zoom_region->priv->parent;
	GdkPixbuf *subimage = NULL;

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	width = gdk_screen_get_width (
		gdk_display_get_screen (magnifier->source_display,
					magnifier->source_screen_num));
	height = gdk_screen_get_height (
		gdk_display_get_screen (magnifier->source_display,
					magnifier->source_screen_num));

	if ((bounds.width <= 0) || (bounds.height <= 0))
	{
		return NULL;
	}
	
	if (!zoom_region->priv->source_drawable)
	{
		/* TESTING ONLY */
		if (zoom_region->priv->test) {
			GdkImage *test_image = NULL;

			test_image = gdk_image_new (GDK_IMAGE_FASTEST,
						    gdk_visual_get_system (),
						    width,
						    height);
			
			for (i = 0; i < width; ++i)
				for (j = 0; j < height; ++j)
					gdk_image_put_pixel (test_image, i, j, i*j);

			zoom_region->priv->source_drawable = gdk_pixmap_new (NULL, width, height, 24);

			if (zoom_region->priv->default_gc == NULL)
				zoom_region->priv->default_gc = gdk_gc_new(zoom_region->priv->w->window);

			gdk_draw_image (zoom_region->priv->source_drawable,
					zoom_region->priv->default_gc,
					test_image,
					0, 0,
					0, 0,
					width, height);
		}
		else
		{
			Magnifier *magnifier = (Magnifier *) zoom_region->priv->parent;
			zoom_region->priv->source_drawable = gdk_screen_get_root_window (
				gdk_display_get_screen (
					magnifier->source_display,
					magnifier->source_screen_num));
		}
		if (zoom_region->cache_source)
		{
			zoom_region->priv->source_pixbuf_cache =
				gdk_pixbuf_new (GDK_COLORSPACE_RGB,
						FALSE,
						8, /* FIXME: not always 8? */
						width, height);
		}
	}
	DEBUG_RECT ("getting subimage from ", bounds);

	subimage = gdk_pixbuf_get_from_drawable (NULL, zoom_region->priv->source_drawable,
						 gdk_colormap_get_system (),
						 bounds.x,
						 bounds.y,
						 0,
						 0,
						 bounds.width,
						 bounds.height);

	/* TODO: blank the region overlapped by the target display if source == target */
	
	if (!subimage)
		_debug_announce_rect ("update of invalid subregion!\n", bounds);

	/* if this zoom-region keeps a cache, do a diff to see if update is necessary */
	if (zoom_region->cache_source && subimage) {
		GdkPixbuf *cache_subpixbuf =
			gdk_pixbuf_new_subpixbuf (zoom_region->priv->source_pixbuf_cache,
						  bounds.x, bounds.y, bounds.width, bounds.height);
		if (_diff_pixbufs (subimage, cache_subpixbuf)) {
			gdk_pixbuf_copy_area (subimage, 0, 0, bounds.width, bounds.height,
					      zoom_region->priv->source_pixbuf_cache,
					      bounds.x, bounds.y);
		}
		else
		{
			if (subimage)
				g_object_unref (subimage);
			subimage = NULL;
		}
		g_object_unref (cache_subpixbuf);
	}
	return subimage;
}

/**
 * zoom_region_update:
 *
 * @rect: a RectBounds structure indicating the source area to update,
 *        in the source coordinate system.
 **/
static void
zoom_region_update (ZoomRegion *zoom_region,
		    const GdkRectangle update_rect)
{
	GdkPixbuf *subimage;
	GdkRectangle source_rect;

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	DEBUG_RECT ("unclipped update rect", update_rect);
	source_rect = zoom_region_clip_to_source (zoom_region, update_rect);
	DEBUG_RECT ("clipped to source", source_rect);
	source_rect = zoom_region_clip_to_exposed_target (zoom_region, source_rect);
	DEBUG_RECT ("update rect clipped to exposed target", source_rect); 

	subimage = zoom_region_get_source_subwindow (zoom_region, source_rect);

	if (subimage && zoom_region->priv->w && zoom_region->priv->w->window)
	{
		GdkRectangle paint_rect;
		gettimeofday(&mag_timing.scale_start, NULL);
		DEBUG_RECT ("source rect", source_rect);
		paint_rect = zoom_region_view_rect_from_source_rect (zoom_region, source_rect);
		/* paint_rect = zoom_region_clip_to_scaled_pixmap (zoom_region, paint_rect); */
		DEBUG_RECT ("paint rect", paint_rect);
		if (zoom_region->invert)
			_zoom_region_invert_pixbuf (subimage);

		/** 
		 *   XXX: We seem to be breaking with the original intention here, which was to 
		 *   keep a fullscreen scaled pixbuf in-sync.
		 **/
		gdk_pixbuf_scale (subimage,
				  zoom_region->priv->scaled_pixbuf,
				  0,
				  0,
				  paint_rect.width,
				  paint_rect.height,
				  0,
				  0,
				  zoom_region->xscale,
				  zoom_region->yscale,
				  zoom_region->priv->gdk_interp_type);

		zoom_region_post_process_pixbuf (zoom_region, subimage,
						 zoom_region->priv->scaled_pixbuf);
		if (zoom_region->priv->default_gc == NULL)
			zoom_region->priv->default_gc = gdk_gc_new(zoom_region->priv->w->window);

#ifndef USE_GDK_PIXBUF_RENDER_TO_DRAWABLE 
		if (GDK_IS_DRAWABLE (zoom_region->priv->pixmap))
		    gdk_draw_pixbuf (zoom_region->priv->pixmap,
				     zoom_region->priv->default_gc,
				     zoom_region->priv->scaled_pixbuf,
				     0,
				     0,
				     paint_rect.x + zoom_region->priv->exposed_bounds.x1,
				     paint_rect.y + zoom_region->priv->exposed_bounds.y1,
				     paint_rect.width,
				     paint_rect.height,
				     GDK_RGB_DITHER_NONE,
				     0,
				     0);
		else
		    g_warning ("updating non-drawable pixmap: region %p", zoom_region);
#else
		gdk_pixbuf_render_to_drawable (zoom_region->priv->scaled_pixbuf,
					       zoom_region->priv->pixmap,
					       zoom_region->priv->default_gc,
					       0,
					       0,
					       paint_rect.x + zoom_region->priv->exposed_bounds.x1,
					       paint_rect.y + zoom_region->priv->exposed_bounds.y1,
					       paint_rect.width,
					       paint_rect.height,
					       GDK_RGB_DITHER_NONE,
					       0,
					       0);
#endif
		if (magnifier_error_check ())
			g_warning ("Could not render scaled image to drawable; out of memory!\n");
		g_object_unref (subimage);
		gdk_window_begin_paint_rect (zoom_region->priv->w->window, &paint_rect);
		zoom_region_paint (zoom_region, &paint_rect);
		gdk_window_end_paint (zoom_region->priv->w->window);
   		gettimeofday(&mag_timing.scale_end, NULL);
		if (timing_test) {
			mag_timing.num_scale_samples++;
			mag_timing.scale_val =
				(mag_timing.scale_end.tv_sec - mag_timing.scale_start.tv_sec) +
				((float)(mag_timing.scale_end.tv_usec - mag_timing.scale_start.tv_usec) /
				1000000.0);
			mag_timing.scale_total += mag_timing.scale_val;

			if (mag_timing.scale_val != 0 && (timing_scale_max == 0 ||
			   (1.0/(float)mag_timing.scale_val) > (1.0/(float)timing_scale_max)))
				timing_scale_max = mag_timing.scale_val;
			if ((source_rect.height * source_rect.width / mag_timing.scale_val) > update_nrr_max)
				update_nrr_max = source_rect.height * source_rect.width / mag_timing.scale_val;

			mag_timing.update_pixels_total += source_rect.height * source_rect.width;

			if (zoom_region->timing_output) {
				fprintf(stderr, "  Update Duration          = %f (avg. %f) (max. %f) (tot. %f) seconds\n",
					mag_timing.scale_val, (mag_timing.scale_total / 
					mag_timing.num_scale_samples), timing_scale_max, mag_timing.scale_total);
				fprintf(stderr, "    Update Pixels          = %ld (avg. %ld) pixels/frame\n",
					(long) source_rect.height * source_rect.width,
					mag_timing.update_pixels_total / mag_timing.num_scale_samples);
				fprintf(stderr, "    Update Rate            = (avg. %f) (max. %f) updates/second\n",
					1.0/(mag_timing.scale_total / mag_timing.num_scale_samples), 1.0/(float)timing_scale_max);
				fprintf(stderr, "    Net Update Rate        = (avg. %f) (max. %f) Mpex/second\n",
					((float)mag_timing.update_pixels_total / (float)mag_timing.scale_total) / 1000000.0,
					update_nrr_max / 1000000.0);
			}
		}
	} else {
		if (subimage)
			fprintf (stderr, "update on uninitialized zoom region!\n");
	}
}

static void
zoom_region_init_window (ZoomRegion *zoom_region)
{
	GtkFixed *parent;
	GtkWidget *zoomer, *border;
	DBG(fprintf (stderr, "window not yet created...\n"));
	parent = GTK_FIXED (
		((Magnifier *)zoom_region->priv->parent)->priv->canvas);
	zoomer = gtk_drawing_area_new ();
	border = gtk_fixed_new ();
	zoom_region->priv->border = border;
	zoom_region->priv->w = zoomer;

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	gtk_widget_set_size_request (GTK_WIDGET (zoomer),
				     zoom_region->viewport.x2 -
				     zoom_region->viewport.x1 - zoom_region->border_size * 2,
				     zoom_region->viewport.y2 -
				     zoom_region->viewport.y1 - zoom_region->border_size * 2);
	gtk_fixed_put (parent, border,
		       zoom_region->viewport.x1,
		       zoom_region->viewport.y1);
	gtk_fixed_put (GTK_FIXED (border), zoomer,
		       zoom_region->border_size,
		       zoom_region->border_size);
	gtk_widget_show (GTK_WIDGET (border));
	gtk_widget_show (GTK_WIDGET (zoomer));
	gtk_widget_show (GTK_WIDGET (parent));
	zoom_region->priv->expose_handler_id =
		g_signal_connect (G_OBJECT (zoom_region->priv->w),
			    "expose_event",
			    G_CALLBACK (zoom_region_expose_handler),
			    zoom_region);
	DBG(fprintf (stderr, "New window created\n"));
	gtk_widget_show (GTK_WIDGET (zoom_region->priv->w));
}

static int
zoom_region_process_updates (gpointer data)
{
	ZoomRegion *zoom_region = (ZoomRegion *) data;

	/* TODO: lock the queue when copying it? */
	zoom_region_coalesce_updates (zoom_region);

	if (zoom_region->priv->q != NULL) {
		GList *last = g_list_last (zoom_region->priv->q);
#ifdef ZOOM_REGION_DEBUG
		fprintf (stderr, "qlen=%d\n", g_list_length (zoom_region->priv->q));
#endif
		if (last) {
			zoom_region->priv->q = g_list_remove_link (zoom_region->priv->q,
								   last);
			zoom_region_update (zoom_region,
					    * (GdkRectangle *) last->data);
			g_list_free (last);
#ifdef DEBUG
			fputs (".\n", stderr); /* debug output, means we actually did something. */
#endif
		}
		return TRUE;
	}
	else 
	{
		if (zoom_region->priv) 
			zoom_region->priv->update_handler_id = 0;
		return FALSE;
	}
}

void
timing_report(ZoomRegion *zoom_region)
{
	float frame_avg;
	float x_scroll_incr, y_scroll_incr;
	int width, height, x, y;

	if (timing_test) {
		width = (zoom_region->priv->exposed_viewport.x2 -
			zoom_region->priv->exposed_viewport.x1) / zoom_region->xscale;
		height = (zoom_region->priv->exposed_viewport.y2 -
			zoom_region->priv->exposed_viewport.y1) / zoom_region->yscale;

		frame_avg = mag_timing.frame_total / mag_timing.num_frame_samples;

		x_scroll_incr = (float)mag_timing.dx_total / (float)mag_timing.num_line_samples;
		y_scroll_incr = (float)mag_timing.dy_total / (float)mag_timing.num_line_samples;

		gdk_drawable_get_size (GDK_DRAWABLE (zoom_region->priv->w->window),
			&x, &y);

		fprintf(stderr, "  Frames Processed         = %ld\n", 
			mag_timing.num_frame_samples + 1);
		fprintf(stderr, "  Width/Height/Depth       = %d/%d/%d\n", x, y,
			gdk_drawable_get_depth (zoom_region->priv->w->window));
		fprintf(stderr, "  Zoom Factor (x/y)        = %f/%f\n", zoom_region->xscale,
			zoom_region->yscale);
		if (mag_timing.num_scale_samples != 0) {
			fprintf(stderr, "  Update Duration          = (avg. %f) (max. %f) (tot. %f) seconds\n",
				(mag_timing.scale_total / mag_timing.num_scale_samples), timing_scale_max, mag_timing.scale_total);
			fprintf(stderr, "    Update Pixels          = (avg. %ld) pixels/frame\n",
				mag_timing.update_pixels_total / mag_timing.num_scale_samples);
			fprintf(stderr, "    Update Rate            = (avg. %f) (max. %f) updates/second\n",
				1.0/((float)mag_timing.scale_total / (float)mag_timing.num_scale_samples),
				1.0/(float)timing_scale_max);
			fprintf(stderr, "    Net Update Rate        = (avg. %f) (max. %f) Mpex/second\n",
				((float)mag_timing.update_pixels_total / (float)mag_timing.scale_total) / 1000000.0,
				update_nrr_max / 1000000.0);
                }
		fprintf(stderr, "  Pan Latency              = (avg. %f) (max. %f) seconds\n",
			(mag_timing.idle_total / mag_timing.num_idle_samples), timing_idle_max);
		fprintf(stderr, "  Total Frame Duration     = (avg. %f) (max. %f) (tot. %f) seconds\n",
			frame_avg, timing_frame_max, mag_timing.frame_total);
		fprintf(stderr, "  Frame Rate               = (avg. %f) (max. %f) frames/second\n",
			1.0 / (mag_timing.frame_total / mag_timing.num_frame_samples), cps_max);
		fprintf(stderr, "  Scroll Delta (x)         = (avg. %f) (tot. %d) lines\n",
			x_scroll_incr, mag_timing.dx_total);
		fprintf(stderr, "  Scroll Delta (y)         = (avg. %f) (tot. %d) lines\n",
			y_scroll_incr, mag_timing.dy_total);
		fprintf(stderr, "  Scroll Rate (x)          = (avg. %f) lines/second\n",
			x_scroll_incr / frame_avg);
		fprintf(stderr, "  Scroll Rate (y)          = (avg. %f) lines/second\n",
			y_scroll_incr / frame_avg);

		fprintf(stderr, "  Net Render Rate          = (avg. %f) (max. %f) Mpex/second\n\n",
			(height * width *
			((float)mag_timing.num_frame_samples / (float)mag_timing.frame_total)) / 1000000.0,
			nrr_max / 1000000.0);
	}
}

static void
zoom_region_time_frame(ZoomRegion *zoom_region, Magnifier *magnifier)
{
	float frame_avg;
	float x_scroll_incr, y_scroll_incr;
	int width = magnifier->target_bounds.x2 - magnifier->target_bounds.x1;
	int height = magnifier->target_bounds.y2 - magnifier->target_bounds.y1;

	mag_timing.num_frame_samples++;
	gettimeofday(&mag_timing.frame_end, NULL);
	mag_timing.frame_val = (mag_timing.frame_end.tv_sec - mag_timing.frame_start.tv_sec) +
		((float)(mag_timing.frame_end.tv_usec - mag_timing.frame_start.tv_usec) /
		1000000.0);

	mag_timing.frame_total += mag_timing.frame_val;
	if (mag_timing.frame_val > timing_frame_max)
		timing_frame_max = mag_timing.frame_val;
	if (mag_timing.frame_val != 0 && 1.0/mag_timing.frame_val > cps_max)
		cps_max = 1.0/mag_timing.frame_val;

	frame_avg = mag_timing.frame_total / mag_timing.num_frame_samples;

	x_scroll_incr = (float)mag_timing.dx_total / (float)mag_timing.num_line_samples;
	y_scroll_incr = (float)mag_timing.dy_total / (float)mag_timing.num_line_samples;

	if ((height * width / mag_timing.frame_val) > nrr_max)
		nrr_max = height * width / mag_timing.frame_val;

	if (zoom_region->timing_output) {
		fprintf(stderr, "  Total Frame Duration     = %f (avg. %f) (max. %f) (tot. %f) seconds\n",
			mag_timing.frame_val, frame_avg, timing_frame_max, mag_timing.frame_total);
		fprintf(stderr, "  Frame Rate               = (avg. %f) (max. %f) frames/second\n",
			1.0 /frame_avg, cps_max);
		fprintf(stderr, "  Scroll Delta (x)         = (avg. %f) (tot. %d) lines\n",
			x_scroll_incr, mag_timing.dx_total);
		fprintf(stderr, "  Scroll Delta (y)         = (avg. %f) (tot. %d) lines\n",
			y_scroll_incr, mag_timing.dy_total);
		fprintf(stderr, "  Scroll Rate (x)          = (avg. %f) lines/second\n",
			x_scroll_incr / frame_avg);
		fprintf(stderr, "  Scroll Rate (y)          = (avg. %f) lines/second\n",
			y_scroll_incr / frame_avg);

		fprintf(stderr, "  Net Render Rate          = (avg. %f) (max. %f) Mpex/second\n",
			(height * width *
			((float)mag_timing.num_frame_samples / (float)mag_timing.frame_total)) / 1000000.0,
			nrr_max / 1000000.0);
	}

	mag_timing.last_frame_val = mag_timing.frame_val;
	mag_timing.last_dy        = mag_timing.dy;

	if (reset_timing) {
		fprintf(stderr, "\n### Updates summary:\n\n");
		timing_report (zoom_region);
	    	fprintf(stderr, "\n### Updates finished, starting panning test\n");
		reset_timing_stats();
		reset_timing = FALSE;
	}
}

void
exit_report(ZoomRegion *zoom_region)
{
	float frame_avg;
	float x_scroll_incr, y_scroll_incr;
	int width, height, x, y;

	if (timing_test) {
		width = (zoom_region->priv->exposed_viewport.x2 -
			zoom_region->priv->exposed_viewport.x1) / zoom_region->xscale;
		height = (zoom_region->priv->exposed_viewport.y2 -
			zoom_region->priv->exposed_viewport.y1) / zoom_region->yscale;

		frame_avg = mag_timing.frame_total / mag_timing.num_frame_samples;

		x_scroll_incr = (float)mag_timing.dx_total / (float)mag_timing.num_line_samples;
		y_scroll_incr = (float)mag_timing.dy_total / (float)mag_timing.num_line_samples;

		gdk_drawable_get_size (GDK_DRAWABLE (zoom_region->priv->w->window),
			&x, &y);

		fprintf(stderr, "\nFinal Report:\n\n");

		fprintf(stderr, "  Frames Processed         = %ld\n", 
			mag_timing.num_frame_samples + 1);
		fprintf(stderr, "  Width/Height/Depth       = %d/%d/%d\n", x, y,
			gdk_drawable_get_depth (zoom_region->priv->w->window));
		fprintf(stderr, "  Zoom Factor (x/y)        = %f/%f\n", zoom_region->xscale,
			zoom_region->yscale);
		fprintf(stderr, "  Pan Latency              = (avg. %f) (max. %f) seconds\n",
			(mag_timing.idle_total / mag_timing.num_idle_samples), timing_idle_max);
		fprintf(stderr, "  Total Frame Duration     = (avg. %f) (max. %f) (tot. %f) seconds\n",
			frame_avg, timing_frame_max, mag_timing.frame_total);
		fprintf(stderr, "  Frame Rate               = (avg. %f) (max. %f) frames/second\n",
			1.0 / (mag_timing.frame_total / mag_timing.num_frame_samples), cps_max);
		fprintf(stderr, "  Scroll Delta (x)         = (avg. %f) (tot. %d) lines\n",
			x_scroll_incr, mag_timing.dx_total);
		fprintf(stderr, "  Scroll Delta (y)         = (avg. %f) (tot. %d) lines\n",
			y_scroll_incr, mag_timing.dy_total);
		fprintf(stderr, "  Scroll Rate (x)          = (avg. %f) lines/second\n",
			x_scroll_incr / frame_avg);
		fprintf(stderr, "  Scroll Rate (y)          = (avg. %f) lines/second\n",
			y_scroll_incr / frame_avg);

		fprintf(stderr, "  Net Render Rate          = (avg. %f) (max. %f) Mpex/second\n\n",
			(height * width *
			((float)mag_timing.num_frame_samples / (float)mag_timing.frame_total)) / 1000000.0,
			nrr_max / 1000000.0);
	}
}

static void
zoom_region_sync (ZoomRegion *zoom_region)
{
	while (zoom_region->priv->q)
		zoom_region_process_updates (zoom_region);
}

static gboolean
gdk_timing_idle (gpointer data)
{
	ZoomRegion *zoom_region = data;

	/* Now update has finished, reset processing_updates */
	processing_updates = FALSE;
	gettimeofday(&mag_timing.idle_end, NULL);

	if (timing_test) {
		mag_timing.num_idle_samples++;
		mag_timing.idle_val =
			(mag_timing.idle_end.tv_sec - mag_timing.idle_start.tv_sec) +
			((float)(mag_timing.idle_end.tv_usec - mag_timing.idle_start.tv_usec) /
			1000000.0);
		mag_timing.idle_total += mag_timing.idle_val;

		if (mag_timing.idle_val > timing_idle_max)
			timing_idle_max = mag_timing.idle_val;

		if (zoom_region->timing_output) {
			fprintf(stderr, "  Pan Latency              = %f (avg. %f) (max. %f) seconds\n",
				mag_timing.idle_val, (mag_timing.idle_total /
				mag_timing.num_idle_samples), timing_idle_max);
		}
	}

	return FALSE;
}

static void
zoom_region_align (ZoomRegion *zoom_region)
{
	Magnifier *magnifier = zoom_region->priv->parent;
	long x = 0, y = 0;
	long width, height;

	if (timing_start)
		zoom_region_time_frame(zoom_region, magnifier);

	if (timing_test) {
		gettimeofday(&mag_timing.frame_start, NULL);

		if (zoom_region->timing_output) {
			gint x, y;

			gdk_drawable_get_size (GDK_DRAWABLE (zoom_region->priv->w->window),
				&x, &y);

			fprintf(stderr, "\nTiming Information - ROI   = (%d, %d) (%d, %d):\n",
				zoom_region->roi.x1, zoom_region->roi.y1, zoom_region->roi.x2,
				zoom_region->roi.y2);
			fprintf(stderr, "  Frame Number             = %ld\n", 
				mag_timing.num_frame_samples + 1);
			fprintf(stderr, "  Width/Height/Depth       = %d/%d/%d\n", x, y,
				gdk_drawable_get_depth (zoom_region->priv->w->window));
		}

		/*
		 * The timing_start flag makes sure that we don't start displaying output
		 * until we have processed an entire frame.
		 */
		if (!timing_start)
			gettimeofday(&mag_timing.process_start, NULL);

		timing_start = TRUE;
	}

	gettimeofday(&mag_timing.idle_start, NULL);

	/*
	 * zoom_region_align calls
	 *   zoom_region_moveto calls
	 *     zoom_region_scroll calls
	 *        zoom_region_scroll_fast or zoom_region_scroll_smooth calls
	 *           gdk_window_scroll or gdk_window_invalidate_rect calls
	 *              gdk_window_invalidate_region calls
	 *                 gdk_window_invalidate_maybe_recurse
	 * 
	 * The last function in the stack will set up an idle handler of
	 * priority GDK_PRIORITY_REDRAW (gdk_window_update_idle) to be called
	 * to handle the work of updateing the screen.
	 *
	 * By setting up an idle handler of priority GDK_PRIORITY_REDRAW + 1,
	 * it will be called immediately after and we can determine when GTK+
	 * is finished with the update.
	 */
	g_idle_add_full (GDK_PRIORITY_REDRAW + 1,
		gdk_timing_idle, zoom_region, NULL);

	width = (zoom_region->priv->exposed_viewport.x2 -
		zoom_region->priv->exposed_viewport.x1) / zoom_region->xscale;
	height = (zoom_region->priv->exposed_viewport.y2 -
		zoom_region->priv->exposed_viewport.y1) / zoom_region->yscale;

	switch (zoom_region->x_align_policy) {
	case GNOME_Magnifier_ZoomRegion_ALIGN_MAX:
		x = zoom_region->roi.x2 - width;
		break;
	case GNOME_Magnifier_ZoomRegion_ALIGN_MIN:
		x = zoom_region->roi.x1;
		break;
	case GNOME_Magnifier_ZoomRegion_ALIGN_CENTER:
	default:
		x = ((zoom_region->roi.x1 + zoom_region->roi.x2) - width ) / 2;
	}

	switch (zoom_region->y_align_policy) {
	case GNOME_Magnifier_ZoomRegion_ALIGN_MAX:
		y = zoom_region->roi.y2 - height;
		break;
	case GNOME_Magnifier_ZoomRegion_ALIGN_MIN:
		y = zoom_region->roi.y1;
		break;
	case GNOME_Magnifier_ZoomRegion_ALIGN_CENTER:
	default:
		y = ((zoom_region->roi.y1 + zoom_region->roi.y2) - height ) / 2;
	}

	zoom_region_moveto (zoom_region, x, y);
}

static void
zoom_region_set_viewport (ZoomRegion *zoom_region,
			  const GNOME_Magnifier_RectBounds *viewport)
{
#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	zoom_region->viewport = *viewport;
#ifdef DEBUG
	fprintf (stderr, "Setting viewport %d,%d - %d,%d\n",
		 (int) viewport->x1, (int) viewport->y1,
		 (int) viewport->x2, (int) viewport->y2);
#endif
	zoom_region_recompute_exposed_viewport (zoom_region);
	zoom_region_align (zoom_region);
	if (!zoom_region->priv->w) {
		zoom_region_init_window (zoom_region);
	} else {
		Magnifier *magnifier = (Magnifier *) zoom_region->priv->parent;
		GtkFixed *fixed = GTK_FIXED (magnifier->priv->canvas);
		gtk_fixed_move (fixed,
				zoom_region->priv->border,
				zoom_region->viewport.x1,
				zoom_region->viewport.y1);
		gtk_widget_set_size_request (GTK_WIDGET (zoom_region->priv->w),
					     zoom_region->viewport.x2 -
					     zoom_region->viewport.x1,
					     zoom_region->viewport.y2 -
					     zoom_region->viewport.y1);
	}
	zoom_region_queue_update (zoom_region,
				  zoom_region_source_rect_from_view_bounds (
					  zoom_region, &zoom_region->viewport));
}

static void
zoom_region_get_property (BonoboPropertyBag *bag,
			  BonoboArg *arg,
			  guint arg_id,
			  CORBA_Environment *ev,
			  gpointer user_data)
{
	ZoomRegion *zoom_region = user_data;

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	DBG (fprintf (stderr, "Get zoom-region property: %s\n", prop_names[arg_id]));

	switch (arg_id) {
	case ZOOM_REGION_MANAGED_PROP:
		BONOBO_ARG_SET_BOOLEAN (arg, zoom_region->is_managed);
		break;
	case ZOOM_REGION_INVERT_PROP:
		BONOBO_ARG_SET_BOOLEAN (arg, zoom_region->invert);
		break;
	case ZOOM_REGION_SMOOTHSCROLL_PROP:
		BONOBO_ARG_SET_SHORT (arg, zoom_region->smooth_scroll_policy);
		break;
	case ZOOM_REGION_TESTPATTERN_PROP:
		BONOBO_ARG_SET_BOOLEAN (arg, zoom_region->priv->test);
		break;
	case ZOOM_REGION_SMOOTHING_PROP:
		BONOBO_ARG_SET_STRING (arg, zoom_region->smoothing);
		break;
	case ZOOM_REGION_CONTRAST_PROP:
		BONOBO_ARG_SET_FLOAT (arg, zoom_region->contrast);
		break;
	case ZOOM_REGION_XSCALE_PROP:
		BONOBO_ARG_SET_FLOAT (arg, zoom_region->xscale);
		break;
	case ZOOM_REGION_YSCALE_PROP:
		BONOBO_ARG_SET_FLOAT (arg, zoom_region->yscale);
		break;
	case ZOOM_REGION_BORDERSIZE_PROP:
		BONOBO_ARG_SET_LONG (arg, zoom_region->border_size);
		break;
	case ZOOM_REGION_XALIGN_PROP:
		/* TODO: enums here */
		BONOBO_ARG_SET_INT (arg, zoom_region->x_align_policy);
		break;
	case ZOOM_REGION_YALIGN_PROP:
		BONOBO_ARG_SET_INT (arg, zoom_region->y_align_policy);
		break;
	case ZOOM_REGION_BORDERCOLOR_PROP:
		BONOBO_ARG_SET_LONG (arg,
				     zoom_region->border_color);
		break;
	case ZOOM_REGION_VIEWPORT_PROP:
		BONOBO_ARG_SET_GENERAL (arg, zoom_region->viewport,
					TC_GNOME_Magnifier_RectBounds,
					GNOME_Magnifier_RectBounds,
					NULL);
		break;
	case ZOOM_REGION_TIMING_TEST_PROP:
		BONOBO_ARG_SET_INT (arg, zoom_region->timing_iterations);
		break;
	case ZOOM_REGION_TIMING_OUTPUT_PROP:
		BONOBO_ARG_SET_BOOLEAN (arg, zoom_region->timing_output);
		break;
	case ZOOM_REGION_TIMING_PAN_RATE_PROP:
		BONOBO_ARG_SET_INT (arg, zoom_region->timing_pan_rate);
		break;
	case ZOOM_REGION_EXIT_MAGNIFIER:
		BONOBO_ARG_SET_BOOLEAN (arg, zoom_region->exit_magnifier);
		break;
	default:
		bonobo_exception_set (ev, ex_Bonobo_PropertyBag_NotFound);
	};
}

static void
zoom_region_set_property (BonoboPropertyBag *bag,
			  BonoboArg *arg,
			  guint arg_id,
			  CORBA_Environment *ev,
			  gpointer user_data)
{
	ZoomRegion *zoom_region = user_data;
	GNOME_Magnifier_RectBounds bounds;

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	DBG (fprintf (stderr, "Set zoom-region property: %s\n", prop_names[arg_id]));

	switch (arg_id) {
	case ZOOM_REGION_MANAGED_PROP:
		zoom_region->is_managed = BONOBO_ARG_GET_BOOLEAN (arg);
		break;
	case ZOOM_REGION_INVERT_PROP:
		zoom_region->invert = BONOBO_ARG_GET_BOOLEAN (arg);
		zoom_region_update_current (zoom_region);
		break;
	case ZOOM_REGION_SMOOTHSCROLL_PROP:
		zoom_region->smooth_scroll_policy = BONOBO_ARG_GET_SHORT (arg);
		break;
	case ZOOM_REGION_SMOOTHING_PROP:
		zoom_region->smoothing = BONOBO_ARG_GET_STRING (arg);
		if (!strncmp (zoom_region->smoothing, "bilinear", 8))
			zoom_region->priv->gdk_interp_type = GDK_INTERP_BILINEAR;
		else 
			zoom_region->priv->gdk_interp_type = GDK_INTERP_NEAREST;
		zoom_region_update_current (zoom_region);
		break;
	case ZOOM_REGION_TESTPATTERN_PROP:
		zoom_region->priv->test = BONOBO_ARG_GET_BOOLEAN (arg);
		if (zoom_region->priv->source_drawable)
			g_object_unref (zoom_region->priv->source_drawable);
		zoom_region_update_current (zoom_region);
		break;
	case ZOOM_REGION_CONTRAST_PROP:
		zoom_region->contrast = BONOBO_ARG_GET_FLOAT (arg);
		zoom_region_update_current (zoom_region);
		break;
	case ZOOM_REGION_XSCALE_PROP:
		zoom_region->xscale = BONOBO_ARG_GET_FLOAT (arg);
		zoom_region_update_current (zoom_region);
		break;
	case ZOOM_REGION_YSCALE_PROP:
		zoom_region->yscale = BONOBO_ARG_GET_FLOAT (arg);
		zoom_region_update_current (zoom_region);
		break;
	case ZOOM_REGION_BORDERSIZE_PROP:
		zoom_region->border_size = BONOBO_ARG_GET_LONG (arg);
		zoom_region_recompute_exposed_viewport (zoom_region);
		zoom_region_update_current (zoom_region);
		break;
	case ZOOM_REGION_BORDERCOLOR_PROP:
		zoom_region->border_color =
			BONOBO_ARG_GET_LONG (arg);
		if (zoom_region->priv->border_gc)
			g_object_unref (zoom_region->priv->border_gc);
		zoom_region->priv->border_gc = NULL;
		zoom_region_update_current (zoom_region);
		break;
	case ZOOM_REGION_XALIGN_PROP:
		zoom_region->x_align_policy = BONOBO_ARG_GET_INT (arg);
		zoom_region_update_current (zoom_region);
		break;
	case ZOOM_REGION_YALIGN_PROP:
		/* TODO: enums here */
		zoom_region->y_align_policy = BONOBO_ARG_GET_INT (arg);
		zoom_region_update_current (zoom_region);
		break;
	case ZOOM_REGION_VIEWPORT_PROP:
		bounds = BONOBO_ARG_GET_GENERAL (arg,
						 TC_GNOME_Magnifier_RectBounds,
						 GNOME_Magnifier_RectBounds,
						 NULL);
		zoom_region_set_viewport (zoom_region, &bounds);
		break;
	case ZOOM_REGION_TIMING_TEST_PROP:
		zoom_region->timing_iterations = BONOBO_ARG_GET_INT (arg);
		timing_test = TRUE;
		break;
	case ZOOM_REGION_TIMING_OUTPUT_PROP:
		zoom_region->timing_output = BONOBO_ARG_GET_BOOLEAN (arg);
		break;
	case ZOOM_REGION_TIMING_PAN_RATE_PROP:
		zoom_region->timing_pan_rate = BONOBO_ARG_GET_INT (arg);
		timing_test = TRUE;
		break;
	case ZOOM_REGION_EXIT_MAGNIFIER:
		zoom_region->exit_magnifier = BONOBO_ARG_GET_BOOLEAN (arg);
		break;
	default:
		bonobo_exception_set (ev, ex_Bonobo_PropertyBag_NotFound);
	};
}

static int
zoom_region_process_pending (gpointer data)
{
	ZoomRegion *zoom_region = (ZoomRegion *) data;

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	zoom_region_align (zoom_region);
	return FALSE;
}

static int
zoom_region_pan_test (gpointer data)
{
	ZoomRegion *zoom_region = (ZoomRegion *) data;
	Magnifier *magnifier = zoom_region->priv->parent; 
	GNOME_Magnifier_ZoomRegionList *zoom_regions;
	GNOME_Magnifier_RectBounds roi;
	CORBA_Environment ev;
	static int counter = 0;
	static gboolean finished_update = !TRUE;
        static float last_pixels_at_speed = -1;
	float pixels_at_speed;
	float total_time;
	int screen_height, height;
	int pixel_position;
	int pixel_direction;

	screen_height = gdk_screen_get_height (
		gdk_display_get_screen (magnifier->source_display,
		 magnifier->source_screen_num));

	height = (zoom_region->priv->exposed_viewport.y2 -
		zoom_region->priv->exposed_viewport.y1) / zoom_region->yscale;

	roi.x1 = zoom_region->roi.x1;
	roi.x2 = zoom_region->roi.x2;

	gettimeofday(&mag_timing.process_end, NULL);
	total_time = (mag_timing.process_end.tv_sec - mag_timing.process_start.tv_sec) +
		((float)(mag_timing.process_end.tv_usec - mag_timing.process_start.tv_usec) /
		1000000.0);

	if (mag_timing.frame_total != 0.0)
		pixels_at_speed = total_time * zoom_region->timing_pan_rate;
	else
		pixels_at_speed = 0.0;

    /* Wait until it is actually necessary to update the screen */
    if ((int)(last_pixels_at_speed) == (int)(pixels_at_speed))
        return TRUE;

	pixel_position = (int)(pixels_at_speed) % (screen_height - height);
	counter = (int)(pixels_at_speed) / (screen_height - height);
	pixel_direction = counter % 2;

	if (!finished_update) {
		if ((int)(pixels_at_speed) > (zoom_region->roi.y1 + height))
			roi.y1 = zoom_region->roi.y1 + height;
		else
			roi.y1 = (int)(pixels_at_speed);

		if (roi.y1 >= screen_height - height) {
			roi.y1 = screen_height - height;
		}
	} else {
		if (pixel_direction == 0)
			roi.y1 = screen_height - height - pixel_position;
		else
			roi.y1 = pixel_position;
	}

	roi.y2 = roi.y1 + height;
	magnifier->priv->cursor_x = (roi.x2 + roi.x1) / 2;
	magnifier->priv->cursor_y = (roi.y2 + roi.y1) / 2;

	/* Add one since in first loop we call zoom_region_process_updates */
	if (counter > zoom_region->timing_iterations - 1)
		zoom_region->exit_magnifier = TRUE;

	zoom_regions = GNOME_Magnifier_Magnifier_getZoomRegions (
		BONOBO_OBJREF (magnifier), &ev);

	if (zoom_regions && (zoom_regions->_length > 0)) {
		GNOME_Magnifier_ZoomRegion_setROI (
			zoom_regions->_buffer[0], &roi, &ev);
	}

	if (!finished_update) {
		zoom_region_process_updates(zoom_region);
		if (roi.y1 == screen_height - height) {
			finished_update = TRUE;
			reset_timing = TRUE;
		}
	}

    last_pixels_at_speed = pixels_at_speed;

	return FALSE;
}

static void
impl_zoom_region_set_roi (PortableServer_Servant servant,
			  const GNOME_Magnifier_RectBounds *bounds,
			  CORBA_Environment *ev)
{
	ZoomRegion *zoom_region =
		ZOOM_REGION (bonobo_object_from_servant (servant));

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	DBG (fprintf (stderr, "Set ROI: \t%d,%d %d,%d\n", 
		      bounds->x1, bounds->y1, bounds->x2, bounds->y2));

	/* if these bounds are clearly bogus, warn and ignore */
	if (!bounds || (bounds->x2 <= bounds->x1)
	    || (bounds->y2 < bounds->y1) || 
	    ((bounds->x1 + bounds->x2)/2 < 0) || 
	    ((bounds->y1 + bounds->y2)/2 < 0))
	{
	    g_warning ("Bad bounds request (%d,%d to %d,%d), ignoring.\n",
		       bounds->x1, bounds->y1, bounds->x2, bounds->y2);
	    return;
	}

	zoom_region->roi = *bounds;

	if (zoom_region->timing_pan_rate > 0) {
		/* Set idle handler to do panning test */
		g_idle_add_full (GDK_PRIORITY_REDRAW + 3, 
			zoom_region_pan_test, zoom_region, NULL);
	}

	if (zoom_region->exit_magnifier) {
		if (timing_test) {
			fprintf(stderr, "\n### Timing Summary:\n\n");
			if (zoom_region->timing_pan_rate)
				fprintf(stderr, "  Pan Rate                 = %d\n", zoom_region->timing_pan_rate);
			timing_report(zoom_region);
		}
		exit(0);
	}

	/*
	 * Do not bother trying to update the screen if the last
	 * screen update has not had time to complete.
	 */
	if (processing_updates) {
		/* Remove any previous idle handler */
		if (pending_idle_handler != 0) {
			g_source_remove(pending_idle_handler);
			pending_idle_handler = 0;
		}

		/* Set idle handler to process this pending update when possible */

		pending_idle_handler = g_idle_add_full (GDK_PRIORITY_REDRAW + 2,
			zoom_region_process_pending, zoom_region, NULL);

		if (zoom_region->timing_output) {
			fprintf(stderr,
				"\n  [Last update not finished, pending - ROI=(%d, %d) (%d, %d)]\n\n",
				zoom_region->roi.x1, zoom_region->roi.y1, zoom_region->roi.x2,
				zoom_region->roi.y2);
		}
	} else {
		zoom_region_align (zoom_region);
	}
}

static CORBA_boolean
impl_zoom_region_set_mag_factor (PortableServer_Servant servant,
				 const CORBA_float mag_factor_x,
				 const CORBA_float mag_factor_y,
				 CORBA_Environment *ev)
{
	ZoomRegion *zoom_region =
		ZOOM_REGION (bonobo_object_from_servant (servant));

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	CORBA_any *any;
	double xs_old = zoom_region->xscale;
	double ys_old = zoom_region->yscale;
	CORBA_boolean retval = CORBA_TRUE;

	/* TODO: assert that parent is magnifier object */
	Bonobo_PropertyBag properties =
		GNOME_Magnifier_Magnifier_getProperties(
			BONOBO_OBJREF (
				(Magnifier *) zoom_region->priv->parent), ev);
	any = Bonobo_PropertyBag_getValue (
		properties, "source-display-bounds", ev);
	if (!BONOBO_EX (ev))
		zoom_region->priv->source_area =
			*((GNOME_Magnifier_RectBounds *) any->_value);
	else
		retval = CORBA_FALSE;

	zoom_region->xscale = mag_factor_x;
	zoom_region->yscale = mag_factor_y;

	if (zoom_region->priv->scaled_pixbuf)
		g_object_unref (zoom_region->priv->scaled_pixbuf);

	zoom_region->priv->scaled_pixbuf = gdk_pixbuf_new (
		GDK_COLORSPACE_RGB, FALSE, 8,
		(zoom_region->priv->source_area.x2 -
		zoom_region->priv->source_area.x1) * zoom_region->xscale + 1,
		(zoom_region->priv->source_area.y2 -
		zoom_region->priv->source_area.y1) * zoom_region->yscale + 1);

	if (zoom_region->priv->pixmap) {
		g_object_unref (zoom_region->priv->pixmap);
		zoom_region->priv->pixmap = NULL;
	}
	if (zoom_region_create_pixmap (zoom_region) == ZOOM_REGION_ERROR_TOO_BIG) {
		zoom_region->xscale = xs_old;
		zoom_region->yscale = ys_old;
		zoom_region_create_pixmap (zoom_region);
		g_object_unref (zoom_region->priv->scaled_pixbuf);

		/* only create a scaled image big enough for the target display, for now */
		zoom_region->priv->scaled_pixbuf = gdk_pixbuf_new (
			GDK_COLORSPACE_RGB, FALSE, 8,
			(zoom_region->priv->source_area.x2 -
			zoom_region->priv->source_area.x1) * zoom_region->xscale + 1,
			(zoom_region->priv->source_area.y2 -
			zoom_region->priv->source_area.y1) * zoom_region->yscale + 1);
		retval = CORBA_FALSE;
	}
	zoom_region_update_current (zoom_region);
	zoom_region_sync (zoom_region);

	bonobo_object_release_unref (properties, NULL);
	return retval;
}

static void
impl_zoom_region_get_mag_factor (PortableServer_Servant servant,
				 CORBA_float *mag_factor_x,
				 CORBA_float *mag_factor_y,
				 CORBA_Environment *ev)
{
	ZoomRegion *zoom_region =
		ZOOM_REGION (bonobo_object_from_servant (servant));

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	*mag_factor_x = zoom_region->xscale;
	*mag_factor_y = zoom_region->yscale;
}

static Bonobo_PropertyBag
impl_zoom_region_get_properties (PortableServer_Servant servant,
				 CORBA_Environment *ev)
{
	ZoomRegion *zoom_region =
		ZOOM_REGION (bonobo_object_from_servant (servant));

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	return bonobo_object_dup_ref (
		BONOBO_OBJREF (zoom_region->properties), ev);
}

static void
impl_zoom_region_mark_dirty (PortableServer_Servant servant,
			     const GNOME_Magnifier_RectBounds *roi_dirty,
			     CORBA_Environment *ev)
{
	ZoomRegion *zoom_region =
		ZOOM_REGION (bonobo_object_from_servant (servant));

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	DEBUG_RECT ("mark dirty", zoom_region_rect_from_bounds (
			    zoom_region, roi_dirty) );

	zoom_region_update_pointer (zoom_region, TRUE);
	/* XXX ? should we clip here, or wait till process_updates? */
	zoom_region_queue_update (zoom_region, 
	  zoom_region_clip_to_source (zoom_region, 
	      zoom_region_rect_from_bounds (zoom_region, roi_dirty)));
}

static GNOME_Magnifier_RectBounds
impl_zoom_region_get_roi (PortableServer_Servant servant,
			  CORBA_Environment     *ev)
{
	ZoomRegion *zoom_region =
		ZOOM_REGION (bonobo_object_from_servant (servant));

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	return zoom_region->roi;
}

static void
impl_zoom_region_move_resize (PortableServer_Servant            servant,
			      const GNOME_Magnifier_RectBounds *viewport_bounds,
			      CORBA_Environment                *ev)
{
	ZoomRegion *zoom_region =
		ZOOM_REGION (bonobo_object_from_servant (servant));

#ifdef ZOOM_REGION_DEBUG
	g_assert (zoom_region->alive);
#endif
	zoom_region_set_viewport (zoom_region, viewport_bounds);
}

/* could be called multiple times... */
static void
zoom_region_do_dispose (ZoomRegion *zoom_region)
{
	DBG(g_message ("disposing region %p", zoom_region));
	if (zoom_region->priv && zoom_region->priv->expose_handler_id && 
	    GTK_IS_WIDGET (zoom_region->priv->w)) {
		g_signal_handler_disconnect (
			zoom_region->priv->w,
			zoom_region->priv->expose_handler_id);
		zoom_region->priv->expose_handler_id = 0;
	}
	if (zoom_region->priv && zoom_region->priv->update_pointer_id)
	    g_source_remove (zoom_region->priv->update_pointer_id);
	if (zoom_region->priv && zoom_region->priv->update_handler_id)
	    g_source_remove (zoom_region->priv->update_handler_id);
	g_idle_remove_by_data (zoom_region);
	
#ifdef ZOOM_REGION_DEBUG
	zoom_region->alive = FALSE;
#endif
}

static void
impl_zoom_region_dispose (PortableServer_Servant servant,
			  CORBA_Environment     *ev)
{
	ZoomRegion *zoom_region =
		ZOOM_REGION (bonobo_object_from_servant (servant));
	zoom_region_do_dispose (zoom_region);
}


/* could be called multiple times */
static void
zoom_region_dispose (GObject *object)
{
	ZoomRegion *zoom_region = ZOOM_REGION (object);

	zoom_region_do_dispose (zoom_region);

	BONOBO_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
zoom_region_class_init (ZoomRegionClass *klass)
{
	GObjectClass * object_class = (GObjectClass *) klass;
	POA_GNOME_Magnifier_ZoomRegion__epv *epv = &klass->epv;
	parent_class = g_type_class_peek (BONOBO_TYPE_OBJECT); /* needed by BONOBO_CALL_PARENT! */

	object_class->dispose = zoom_region_dispose;
	object_class->finalize = zoom_region_finalize;
	
	epv->setMagFactor = impl_zoom_region_set_mag_factor;
	epv->getMagFactor = impl_zoom_region_get_mag_factor;
	epv->getProperties = impl_zoom_region_get_properties;
	epv->setROI = impl_zoom_region_set_roi;
	epv->markDirty = impl_zoom_region_mark_dirty;
	epv->getROI = impl_zoom_region_get_roi;
	epv->moveResize = impl_zoom_region_move_resize;
	epv->dispose = impl_zoom_region_dispose;

	reset_timing_stats();
#ifdef DEBUG_CLIENT_CALLS
	client_debug = (g_getenv ("MAG_CLIENT_DEBUG") != NULL);
#endif
}

static void
zoom_region_properties_init (ZoomRegion *zoom_region)
{
	BonoboArg *def;
	
	zoom_region->properties =
		bonobo_property_bag_new_closure (
			g_cclosure_new_object (
				G_CALLBACK (zoom_region_get_property),
				G_OBJECT (zoom_region)),
			g_cclosure_new_object (
				G_CALLBACK (zoom_region_set_property),
				G_OBJECT (zoom_region)));

	def = bonobo_arg_new (BONOBO_ARG_BOOLEAN);
	BONOBO_ARG_SET_BOOLEAN (def, TRUE);
	
	bonobo_property_bag_add (zoom_region->properties,
				 "is-managed",
				 ZOOM_REGION_MANAGED_PROP,
				 BONOBO_ARG_BOOLEAN,
				 def,
				 "If false, zoom region does not auto-update, but is drawn into directly by the client",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);

	bonobo_arg_release (def);
	def = bonobo_arg_new (BONOBO_ARG_SHORT);
	BONOBO_ARG_SET_SHORT (def, GNOME_Magnifier_ZoomRegion_SCROLL_FASTEST);
	
	bonobo_property_bag_add (zoom_region->properties,
				 "smooth-scroll-policy",
				 ZOOM_REGION_SMOOTHSCROLL_PROP,
				 BONOBO_ARG_SHORT,
				 def,
				 "scrolling policy, slower versus faster",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);

	bonobo_arg_release (def);
	def = bonobo_arg_new (BONOBO_ARG_BOOLEAN);
	BONOBO_ARG_SET_BOOLEAN (def, FALSE);

	bonobo_property_bag_add (zoom_region->properties,
				 "use-test-pattern",
				 ZOOM_REGION_TESTPATTERN_PROP,
				 BONOBO_ARG_BOOLEAN,
				 def,
				 "use test pattern for source",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);

	bonobo_arg_release (def);
	def = bonobo_arg_new (BONOBO_ARG_BOOLEAN);
	BONOBO_ARG_SET_BOOLEAN (def, TRUE);
	
	bonobo_property_bag_add (zoom_region->properties,
				 "inverse-video",
				 ZOOM_REGION_INVERT_PROP,
				 BONOBO_ARG_BOOLEAN,
				 def,
				 "inverse video display",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);

	bonobo_arg_release (def);

	bonobo_property_bag_add (zoom_region->properties,
				 "smoothing-type",
				 ZOOM_REGION_SMOOTHING_PROP,
				 BONOBO_ARG_STRING,
				 NULL,
				 "image smoothing algorithm used",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);

	def = bonobo_arg_new (BONOBO_ARG_FLOAT);
	BONOBO_ARG_SET_FLOAT (def, 1.0);

	bonobo_property_bag_add (zoom_region->properties,
				 "contrast",
				 ZOOM_REGION_CONTRAST_PROP,
				 BONOBO_ARG_FLOAT,
				 def,
				 "image contrast ratio",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);

	bonobo_arg_release (def);
	def = bonobo_arg_new (BONOBO_ARG_FLOAT);
	BONOBO_ARG_SET_FLOAT (def, 2.0);

	bonobo_property_bag_add (zoom_region->properties,
				 "mag-factor-x",
				 ZOOM_REGION_XSCALE_PROP,
				 BONOBO_ARG_FLOAT,
				 def,
				 "x scale factor",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);

	bonobo_arg_release (def);
	def = bonobo_arg_new (BONOBO_ARG_FLOAT);
	BONOBO_ARG_SET_FLOAT (def, 2.0);

	bonobo_property_bag_add (zoom_region->properties,
				 "mag-factor-y",
				 ZOOM_REGION_YSCALE_PROP,
				 BONOBO_ARG_FLOAT,
				 def,
				 "y scale factor",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);

	bonobo_arg_release (def);
	def = bonobo_arg_new (BONOBO_ARG_LONG);
	BONOBO_ARG_SET_LONG (def, 0);
	
	bonobo_property_bag_add (zoom_region->properties,
				 "border-size",
				 ZOOM_REGION_BORDERSIZE_PROP,
				 BONOBO_ARG_LONG,
				 def,
				 "size of zoom-region borders, in pixels",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);

	bonobo_arg_release (def);
	def = bonobo_arg_new (BONOBO_ARG_LONG);
	BONOBO_ARG_SET_LONG (def, 0x00000000);
	
	bonobo_property_bag_add (zoom_region->properties,
				 "border-color",
				 ZOOM_REGION_BORDERCOLOR_PROP,
				 BONOBO_ARG_LONG,
				 def,
				 "border color, as RGBA32",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);

	bonobo_arg_release (def);
	def = bonobo_arg_new (BONOBO_ARG_INT);
	BONOBO_ARG_SET_INT (def, 0);

	bonobo_property_bag_add (zoom_region->properties,
				 "x-alignment",
				 ZOOM_REGION_XALIGN_PROP,
				 BONOBO_ARG_INT,
				 def,
				 "x-alignment policy for this region",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);

	bonobo_arg_release (def);
	def = bonobo_arg_new (BONOBO_ARG_INT);
	BONOBO_ARG_SET_INT (def, 0);

	bonobo_property_bag_add (zoom_region->properties,
				 "y-alignment",
				 ZOOM_REGION_YALIGN_PROP,
				 BONOBO_ARG_INT,
				 def,
				 "y-alignment policy for this region",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);
	bonobo_arg_release (def);

	bonobo_property_bag_add (zoom_region->properties,
				 "viewport",
				 ZOOM_REGION_VIEWPORT_PROP,
				 TC_GNOME_Magnifier_RectBounds,
				 NULL,
				 "viewport bounding box",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);

	def = bonobo_arg_new (BONOBO_ARG_INT);
	BONOBO_ARG_SET_INT (def, 0);

	bonobo_property_bag_add (zoom_region->properties,
				 "timing-iterations",
				 ZOOM_REGION_TIMING_TEST_PROP,
				 BONOBO_ARG_INT,
				 def,
				 "timing iterations",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);
	bonobo_arg_release (def);

	def = bonobo_arg_new (BONOBO_ARG_BOOLEAN);
	BONOBO_ARG_SET_BOOLEAN (def, FALSE);
	
	bonobo_property_bag_add (zoom_region->properties,
				 "timing-output",
				 ZOOM_REGION_TIMING_OUTPUT_PROP,
				 BONOBO_ARG_BOOLEAN,
				 def,
				 "timing output",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);

	bonobo_arg_release (def);

	def = bonobo_arg_new (BONOBO_ARG_INT);
	BONOBO_ARG_SET_INT (def, 0);

	bonobo_property_bag_add (zoom_region->properties,
				 "timing-pan-rate",
				 ZOOM_REGION_TIMING_PAN_RATE_PROP,
				 BONOBO_ARG_INT,
				 def,
				 "timing pan rate",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);
	bonobo_arg_release (def);

	def = bonobo_arg_new (BONOBO_ARG_BOOLEAN);
	BONOBO_ARG_SET_BOOLEAN (def, FALSE);
	
	bonobo_property_bag_add (zoom_region->properties,
				 "exit-magnifier",
				 ZOOM_REGION_EXIT_MAGNIFIER,
				 BONOBO_ARG_BOOLEAN,
				 def,
				 "timing output",
				 Bonobo_PROPERTY_READABLE |
				 Bonobo_PROPERTY_WRITEABLE);

	bonobo_arg_release (def);

}

static void
zoom_region_private_init (ZoomRegionPrivate *priv)
{
	GdkRectangle rect = {0, 0, 0, 0};
	GNOME_Magnifier_RectBounds rectbounds = {0, 0, 0, 0};
	priv->parent = NULL;
	priv->w = NULL;
	priv->default_gc = NULL;
	priv->paint_cursor_gc = NULL;
	priv->crosswire_gc = NULL;
	priv->q = NULL;
	priv->scaled_pixbuf = NULL;
	priv->source_pixbuf_cache = NULL;
	priv->source_drawable = NULL;
	priv->pixmap = NULL;
	priv->cursor_backing_rect = rect;
	priv->cursor_backing_pixels = NULL;
	priv->border_gc = NULL;
	priv->gdk_interp_type = GDK_INTERP_NEAREST;
	priv->expose_handler_id = 0;
	priv->test = FALSE;
	priv->last_cursor_pos.x = 0;
	priv->last_cursor_pos.y = 0;
	priv->last_drawn_crosswire_pos.x = 0;
	priv->last_drawn_crosswire_pos.y = 0;
	priv->exposed_bounds = rectbounds;
	priv->exposed_viewport = rectbounds;
	priv->source_area = rectbounds;
	priv->update_pointer_id = 0;
	priv->update_handler_id = 0;
}

static void
zoom_region_init (ZoomRegion *zoom_region)
{
	DBG(g_message ("initializing region %p", zoom_region));

	zoom_region_properties_init (zoom_region);
	zoom_region->smooth_scroll_policy =
		GNOME_Magnifier_ZoomRegion_SCROLL_SMOOTH;
	zoom_region->invert = FALSE;
	zoom_region->cache_source = FALSE;
	zoom_region->border_size = 0;
	zoom_region->border_color = 0;
	zoom_region->roi.x1 = 0;
	zoom_region->roi.x1 = 0;
	zoom_region->roi.x2 = 1;
	zoom_region->roi.x2 = 1;
	zoom_region->x_align_policy = GNOME_Magnifier_ZoomRegion_ALIGN_CENTER;
	zoom_region->y_align_policy = GNOME_Magnifier_ZoomRegion_ALIGN_CENTER;
	zoom_region->coalesce_func = _coalesce_update_rects;
	zoom_region->priv = g_malloc (sizeof (ZoomRegionPrivate));
	zoom_region_private_init (zoom_region->priv);
	bonobo_object_add_interface (BONOBO_OBJECT (zoom_region),
				     BONOBO_OBJECT (zoom_region->properties));
	zoom_region->timing_output = FALSE;
#ifdef ZOOM_REGION_DEBUG
	zoom_region->alive = TRUE;
#endif
	zoom_region->priv->update_pointer_id =
	    g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,
				200,
				zoom_region_update_pointer_timeout,
				zoom_region,
				NULL);
}

ZoomRegion *
zoom_region_new (void)
{
	return g_object_new (zoom_region_get_type(), NULL);
}

/* this one really shuts down the object - called once only */
static void
zoom_region_finalize (GObject *region)
{
	ZoomRegion *zoom_region = (ZoomRegion *) region;

	DBG(g_message ("finalizing region %p", zoom_region));

	if (zoom_region->priv && zoom_region->priv->q) 
	{
		g_list_free (zoom_region->priv->q);
		zoom_region->priv->q = NULL;
	}
	if (GTK_IS_WIDGET (zoom_region->priv->w))
		gtk_container_remove (GTK_CONTAINER (zoom_region->priv->border),
				      GTK_WIDGET (zoom_region->priv->w));
	if (GTK_IS_WIDGET (zoom_region->priv->border))
		gtk_container_remove (GTK_CONTAINER (((Magnifier *) 
						      zoom_region->priv->parent)->priv->canvas),
				      GTK_WIDGET (zoom_region->priv->border));
	if (zoom_region->priv->source_pixbuf_cache) 
	    g_object_unref (zoom_region->priv->source_pixbuf_cache);
	if (zoom_region->priv->scaled_pixbuf) 
	    g_object_unref (zoom_region->priv->scaled_pixbuf);
	if (zoom_region->priv->pixmap) 
	    g_object_unref (zoom_region->priv->pixmap);
	zoom_region->priv->pixmap = NULL;
	zoom_region->priv->parent = NULL;
	if (zoom_region->priv->cursor_backing_pixels) 
	    g_object_unref (zoom_region->priv->cursor_backing_pixels);
	if (zoom_region->priv->border_gc) 
	    g_object_unref (zoom_region->priv->border_gc);
	g_free (zoom_region->priv);
	zoom_region->priv = NULL;
#ifdef ZOOM_REGION_DEBUG
	zoom_region->alive = FALSE;
#endif
	BONOBO_CALL_PARENT (G_OBJECT_CLASS, finalize, (region));
}

BONOBO_TYPE_FUNC_FULL (ZoomRegion, 
		       GNOME_Magnifier_ZoomRegion,
		       BONOBO_TYPE_OBJECT,
		       zoom_region);
