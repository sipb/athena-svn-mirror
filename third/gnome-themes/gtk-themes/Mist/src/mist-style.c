/* Mist theme for gtk, based on thinice, based on raster's Motif theme
 * and the Metal theme.

Mist Author: Dave Camp <dave@ximian.com>
Thinice Authors: Tim Gerla <timg@rrv.net>
                 Tomas Ögren <stric@ing.umu.se>
 */

#include <math.h>
#include <string.h>
#include <gtk/gtk.h>

#if (GTK_MAJOR_VERSION == 1)
#   define GTK1
#endif

#ifndef GTK1
#   include "mist-style.h"
#   include "mist-rc-style.h"
#   define DETAIL_CONST const
#   define YTHICKNESS(style) (style->ythickness)
#   define XTHICKNESS(style) (style->xthickness)
#else
#   include "mist-theme.h"
#   define DETAIL_CONST
#   define YTHICKNESS(style) (style->klass->ythickness)
#   define XTHICKNESS(style) (style->klass->xthickness)
#   define GDK_IS_PIXMAP(w) (gdk_window_get_type (w) == GDK_WINDOW_PIXMAP)
#endif

#define DETAIL(xx) ((detail) && (!strcmp(xx, detail)))

#ifndef GTK1

static GtkStyleClass *parent_class = NULL;

static void mist_style_init       (MistStyle      *style);
static void mist_style_class_init (MistStyleClass *klass);

#endif

static void
sanitize_size (GdkWindow *window, int *width, int *height)
{
	if ((*width == -1) && (*height == -1))
		gdk_window_get_size (window, width, height);
	else if (*width == -1)
		gdk_window_get_size (window, width, NULL);
	else if (*height == -1)
		gdk_window_get_size (window, NULL, height);
}

static GtkShadowType
get_shadow_type (GtkStyle *style, const char *detail, GtkShadowType requested)
{
	GtkShadowType retval = GTK_SHADOW_NONE;
	
	if (requested != GTK_SHADOW_NONE) {
		retval = GTK_SHADOW_ETCHED_IN;
	}
	
	if (DETAIL ("dockitem")
	    || DETAIL ("handlebox_bin")) {
		retval = GTK_SHADOW_NONE;
	} else if (DETAIL ("spinbutton_up")
		   || DETAIL ("spinbutton_down")) {
		retval = GTK_SHADOW_OUT;
	} else if (DETAIL ("button") 
		   || DETAIL ("togglebutton") 
		   || DETAIL ("notebook") 
		   || DETAIL ("optionmenu")) {
		retval = requested;
	} else if (DETAIL ("menu")) {
		retval = GTK_SHADOW_ETCHED_IN;
	}
	
	return retval;
}

static void
mist_dot (GdkWindow *window,
	  GdkGC *gc1,
	  GdkGC *gc2,
	  int x,
	  int y)
{
	GdkPoint points[3];
	
	points[0].x = x-1; points[0].y = y;
	points[1].x = x-1; points[1].y = y-1;
	points[2].x = x;   points[2].y = y-1;
	
	gdk_draw_points(window, gc2, points, 3);
	
	points[0].x = x+1; points[0].y = y;
	points[1].x = x+1; points[1].y = y+1;
	points[2].x = x;   points[2].y = y+1;
	
	gdk_draw_points(window, gc1, points, 3);
}

static void
mist_tab (GtkStyle *style,
	  GdkWindow *window,
	  GtkStateType state_type,
	  GtkShadowType shadow_type,
	  GdkRectangle *area,
	  GtkWidget *widget,
	  DETAIL_CONST char *detail,
	  int x,
	  int y,
	  int width,
	  int height)
{
	GtkNotebook *notebook;
	GdkGC *lightgc, *darkgc;
	int orientation;
	
	notebook = GTK_NOTEBOOK(widget);
	orientation = notebook->tab_pos;
	
	lightgc = style->light_gc[state_type];
	darkgc = style->dark_gc[state_type];
	
	if ((!style->bg_pixmap[state_type]) || GDK_IS_PIXMAP(window)) {
		if (area) {
			gdk_gc_set_clip_rectangle(style->bg_gc[state_type], 
						  area);
		}
		gdk_draw_rectangle(window, style->bg_gc[state_type], TRUE,
				   x, y, width, height);
		if (area) {
			gdk_gc_set_clip_rectangle(style->bg_gc[state_type], 
						  NULL);
		}
	} else {
		gtk_style_apply_default_background
			(style, window,
			 widget && !GTK_WIDGET_NO_WINDOW(widget),
			 state_type, area, x, y, width, height);
	}
	
	if (area) {
		gdk_gc_set_clip_rectangle(style->dark_gc[state_type], area);
		gdk_gc_set_clip_rectangle(style->light_gc[state_type], area);
	}

	switch(orientation) {
	case GTK_POS_TOP:
		gdk_draw_line(window, lightgc,
			      x, y + height - 1, x, y);
		gdk_draw_line(window, lightgc,
			      x, y, x + width - 1, y);
		gdk_draw_line(window, darkgc,
			      x + width - 1, y, x + width - 1, y + height - 1);
		break;
	case GTK_POS_BOTTOM:
		gdk_draw_line(window, lightgc,
			      x, y, x, y + height - 1);
		gdk_draw_line(window, darkgc,
			      x, y + height - 1, 
			      x + width - 1, y + height - 1);
		gdk_draw_line(window, darkgc,
			      x + width - 1, y + height - 1, x + width - 1, y);
		break;
	case GTK_POS_LEFT:
		gdk_draw_line(window, lightgc,
			      x, y + height - 1, x, y);
		gdk_draw_line(window, lightgc,
			      x, y, x + width - 1, y);
		gdk_draw_line(window, darkgc,
			      x, y + height - 1, 
			      x + width - 1, y + height - 1);
		break;
	case GTK_POS_RIGHT:
		gdk_draw_line(window, lightgc,
			      x, y, x + width - 1, y);
		gdk_draw_line(window, darkgc,
			      x + width - 1, y, 
			      x + width - 1, y + height - 1);
		gdk_draw_line(window, darkgc,
			      x, y + height - 1, 
			      x + width - 1, y + height - 1);
		break;
	}

	if (area) {
		gdk_gc_set_clip_rectangle(style->dark_gc[state_type], NULL);
		gdk_gc_set_clip_rectangle(style->light_gc[state_type], NULL);
	}
}

static void
draw_rect (GtkStyle *style, 
	   GdkWindow *window, 
	   GdkGC *gc1, 
	   GdkGC *gc2, 
	   GdkGC *fill_gc, 
	   int x, 
	   int y,
	   int width, 
	   int height)
{
	if (fill_gc) {
		gdk_draw_rectangle (window, fill_gc, TRUE, x + 1, y + 1, 
				    width - 2, height - 2);
	}
	
	gdk_draw_line(window, gc2,
		      x, y, x + width - 1, y);
	gdk_draw_line(window, gc2,
		      x, y, x, y + height - 1);
	gdk_draw_line(window, gc1,
		      x, y + height - 1, x + width - 1, y + height - 1);
	gdk_draw_line(window, gc1,
		      x + width - 1, y, x + width - 1, y + height - 1);
}

static void
draw_rect_with_shadow (GtkStyle *style,
		       GdkWindow *window,
		       GtkWidget *widget,
		       GtkStateType state_type,
		       GtkShadowType shadow_type,
		       GdkGC *fill_gc,
		       int x, 
		       int y, 
		       int width, 
		       int height)
{
	GdkGC *gc1;
	GdkGC *gc2;
	
	if (shadow_type == GTK_SHADOW_NONE) {
		return;
	}
	
	switch (shadow_type) {
	case GTK_SHADOW_ETCHED_IN :
	case GTK_SHADOW_ETCHED_OUT :
		gc1 = style->dark_gc[state_type];
		gc2 = style->dark_gc[state_type];
		break;
	case GTK_SHADOW_OUT :
		gc1 = style->dark_gc[state_type];
		gc2 = style->light_gc[state_type];
		break;
	case GTK_SHADOW_IN :
		gc1 = style->light_gc[state_type];
		gc2 = style->dark_gc[state_type];
		break;
	case GTK_SHADOW_NONE :
	default :
		gc1 = style->bg_gc[state_type];
		gc2 = style->bg_gc[state_type];
		
	}
	
	draw_rect (style, window,gc1, gc2, fill_gc, 
		   x, y, width, height);
	
}

static void
draw_hline(GtkStyle *style,
           GdkWindow *window,
           GtkStateType state_type,
           GdkRectangle *area,
           GtkWidget *widget,
           DETAIL_CONST char *detail,
           int x1,
           int x2,
           int y)
{
	int thickness_light;
	int thickness_dark;
	int i;
	
	g_return_if_fail(style != NULL);
	g_return_if_fail(window != NULL);
	
	if (area) {
		gdk_gc_set_clip_rectangle(style->light_gc[state_type], area);
		gdk_gc_set_clip_rectangle(style->dark_gc[state_type], area);
	}
	
	if (DETAIL ("hseparator") 
	    || DETAIL ("menuitem") 
	    || DETAIL ("slider")
	    || DETAIL("vscale")) {
		gdk_draw_line (window, style->dark_gc[state_type], 
			       x1, y, x2, y);
#ifdef GTK1
	} else if (DETAIL ("label")) {
		gdk_draw_line (window, style->fg_gc[state_type], 
			       x1, y, x2, y);
#endif		
	} else {
		thickness_light = YTHICKNESS(style) / 2;
		thickness_dark = YTHICKNESS(style) - thickness_light;
		
		for (i = 0; i < thickness_dark; i++) {
			gdk_draw_line(window, style->light_gc[state_type], 
				      x2 - i - 1, y + i, x2, y + i);
			gdk_draw_line(window, style->dark_gc[state_type], 
				      x1, y + i, x2 - i - 1, y + i);
		}
		
		y += thickness_dark;
		for (i = 0; i < thickness_light; i++) {
			gdk_draw_line(window, style->dark_gc[state_type], 
				      x1, y + i, 
				      x1 + thickness_light - i - 1, y + i);
			gdk_draw_line(window, style->light_gc[state_type], 
				      x1 + thickness_light - i - 1, y + i, 
				      x2, y + i);
		}
	}
	
	if (area) {
		gdk_gc_set_clip_rectangle(style->light_gc[state_type], NULL);
		gdk_gc_set_clip_rectangle(style->dark_gc[state_type], NULL);
	}
}

static void
draw_vline(GtkStyle *style,
           GdkWindow *window,
           GtkStateType state_type,
           GdkRectangle *area,
           GtkWidget *widget,
           DETAIL_CONST char *detail,
           int y1,
           int y2,
           int x)
{
	int thickness_light;
	int thickness_dark;
	int i;

	g_return_if_fail(style != NULL);
	g_return_if_fail(window != NULL);
	
	if (area) {
		gdk_gc_set_clip_rectangle(style->light_gc[state_type], area);
		gdk_gc_set_clip_rectangle(style->dark_gc[state_type], area);
	}
	
	if (DETAIL ("vseparator") 
	    || DETAIL ("toolbar") 
	    || DETAIL ("slider") 
	    || DETAIL ("hscale")) {
		gdk_draw_line (window, style->dark_gc[state_type], 
			       x, y1, x, y2);
	} else {
		thickness_light = XTHICKNESS(style) / 2;
		thickness_dark = XTHICKNESS(style) - thickness_light;
		
		for (i = 0; i < thickness_dark; i++) {
			gdk_draw_line(window, style->light_gc[state_type], 
				      x + i, y2 - i - 1, x + i, y2);
			gdk_draw_line(window, style->dark_gc[state_type], 
				      x + i, y1, x + i, y2 - i - 1);
		}
		
		x += thickness_dark;
		for (i = 0; i < thickness_light; i++) {
			gdk_draw_line(window, style->dark_gc[state_type], 
				      x + i, y1, x + i, 
				      y1 + thickness_light - i);
			gdk_draw_line(window, style->light_gc[state_type], 
				      x + i, y1 + thickness_light - i, 
				      x + i, y2);
		}
	}
	
	if (area) {
		gdk_gc_set_clip_rectangle(style->light_gc[state_type], NULL);
		gdk_gc_set_clip_rectangle(style->dark_gc[state_type], NULL);
	}
}

static void
draw_shadow(GtkStyle *style,
	    GdkWindow *window,
	    GtkStateType state_type,
	    GtkShadowType shadow_type,
	    GdkRectangle *area,
	    GtkWidget *widget,
	    DETAIL_CONST char *detail,
	    int x,
	    int y,
	    int width,
	    int height)
{
	g_return_if_fail(style != NULL);
	g_return_if_fail(window != NULL);
	
	sanitize_size (window, &width, &height);

	shadow_type = get_shadow_type (style, detail, shadow_type);

	if (state_type == GTK_STATE_INSENSITIVE && shadow_type != GTK_SHADOW_NONE) {
		shadow_type = GTK_SHADOW_ETCHED_IN;
	}
	
	if (DETAIL("frame") && widget->parent && GTK_IS_STATUSBAR (widget->parent)) {
		if (shadow_type != GTK_SHADOW_NONE) {
			gdk_draw_line (window, 
				       style->dark_gc[GTK_STATE_NORMAL], 
				       x, y, 
				       x + width - 1, y);

		}
	} else {
		draw_rect_with_shadow (style, window, widget, state_type,
				       shadow_type, NULL, x, y, width, height);
	}
}

static void
draw_polygon(GtkStyle *style,
             GdkWindow *window,
             GtkStateType state_type,
             GtkShadowType shadow_type,
             GdkRectangle *area,
             GtkWidget *widget,
             DETAIL_CONST char *detail,
             GdkPoint *points,
             int npoints,
             int fill)
{
#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif /* M_PI */
#ifndef M_PI_4
#define M_PI_4  0.78539816339744830962
#endif /* M_PI_4 */
	static const gdouble pi_over_4 = M_PI_4;
	static const gdouble pi_3_over_4 = M_PI_4 * 3;
	
	GdkGC              *gc1;
	GdkGC              *gc2;
	GdkGC              *gc3;
	GdkGC              *gc4;
	gdouble             angle;
	int                xadjust;
	int                yadjust;
	int                i;

	g_return_if_fail(style != NULL);
	g_return_if_fail(window != NULL);
	g_return_if_fail(points != NULL);
	
	switch (shadow_type) {
	case GTK_SHADOW_IN:
		gc1 = style->light_gc[state_type];
		gc2 = style->dark_gc[state_type];
		gc3 = style->light_gc[state_type];
		gc4 = style->dark_gc[state_type];
		break;
	case GTK_SHADOW_ETCHED_IN:
		gc1 = style->light_gc[state_type];
		gc2 = style->dark_gc[state_type];
		gc3 = style->dark_gc[state_type];
		gc4 = style->light_gc[state_type];
		break;
	case GTK_SHADOW_OUT:
		gc1 = style->dark_gc[state_type];
		gc2 = style->light_gc[state_type];
		gc3 = style->dark_gc[state_type];
		gc4 = style->light_gc[state_type];
		break;
	case GTK_SHADOW_ETCHED_OUT:
		gc1 = style->dark_gc[state_type];
		gc2 = style->light_gc[state_type];
		gc3 = style->light_gc[state_type];
		gc4 = style->dark_gc[state_type];
		break;
	default:
		return;
	}
	
	if (area) {
		gdk_gc_set_clip_rectangle(gc1, area);
		gdk_gc_set_clip_rectangle(gc2, area);
		gdk_gc_set_clip_rectangle(gc3, area);
		gdk_gc_set_clip_rectangle(gc4, area);
	}
	
	if (fill)
		gdk_draw_polygon(window, style->bg_gc[state_type], 
				 TRUE, points, npoints);
	
	npoints--;
	
	for (i = 0; i < npoints; i++) {
		if ((points[i].x == points[i + 1].x) &&
		    (points[i].y == points[i + 1].y)) {
			angle = 0;
		} else {
			angle = atan2(points[i + 1].y - points[i].y,
				      points[i + 1].x - points[i].x);
		}
		
		if ((angle > -pi_3_over_4) && (angle < pi_over_4)) {
			if (angle > -pi_over_4) {
				xadjust = 0;
				yadjust = 1;
			} else {
				xadjust = 1;
				yadjust = 0;
			}

			gdk_draw_line(window, gc1,
				      points[i].x - xadjust, 
				      points[i].y - yadjust,
				      points[i + 1].x - xadjust, 
				      points[i + 1].y - yadjust);
			gdk_draw_line(window, gc3,
				      points[i].x, points[i].y,
				      points[i + 1].x, points[i + 1].y);
		}
		else {
			if ((angle < -pi_3_over_4) || (angle > pi_3_over_4)) {
				xadjust = 0;
				yadjust = 1;
			} else {
				xadjust = 1;
				yadjust = 0;
			}
			
			gdk_draw_line(window, gc4,
				      points[i].x + xadjust, 
				      points[i].y + yadjust,
				      points[i + 1].x + xadjust, 
				      points[i + 1].y + yadjust);
			gdk_draw_line(window, gc2,
				      points[i].x, points[i].y,
				      points[i + 1].x, points[i + 1].y);
		}
	}
	if (area) {
		gdk_gc_set_clip_rectangle(gc1, NULL);
		gdk_gc_set_clip_rectangle(gc2, NULL);
		gdk_gc_set_clip_rectangle(gc3, NULL);
		gdk_gc_set_clip_rectangle(gc4, NULL);
	}
}

static void
draw_diamond(GtkStyle * style,
             GdkWindow * window,
             GtkStateType state_type,
             GtkShadowType shadow_type,
             GdkRectangle * area,
             GtkWidget * widget,
             DETAIL_CONST char *detail,
             int x,
             int y,
             int width,
             int height)
{
	int half_width;
	int half_height;
	
	g_return_if_fail(style != NULL);
	g_return_if_fail(window != NULL);	

	sanitize_size (window, &width, &height);
	
	half_width = width / 2;
	half_height = height / 2;
	
	if (area) {
		gdk_gc_set_clip_rectangle(style->light_gc[state_type], area);
		gdk_gc_set_clip_rectangle(style->bg_gc[state_type], area);
		gdk_gc_set_clip_rectangle(style->dark_gc[state_type], area);
		gdk_gc_set_clip_rectangle(style->black_gc, area);
	}
	switch (shadow_type) {
	case GTK_SHADOW_IN:
		gdk_draw_line(window, style->light_gc[state_type],
			      x + 2, y + half_height,
			      x + half_width, y + height - 2);
		gdk_draw_line(window, style->light_gc[state_type],
			      x + half_width, y + height - 2,
			      x + width - 2, y + half_height);
		gdk_draw_line(window, style->light_gc[state_type],
			      x + 1, y + half_height,
			      x + half_width, y + height - 1);
		gdk_draw_line(window, style->light_gc[state_type],
			      x + half_width, y + height - 1,
			      x + width - 1, y + half_height);
		gdk_draw_line(window, style->light_gc[state_type],
			      x, y + half_height,
			      x + half_width, y + height);
		gdk_draw_line(window, style->light_gc[state_type],
			      x + half_width, y + height,
			      x + width, y + half_height);
		
		gdk_draw_line(window, style->dark_gc[state_type],
			      x + 2, y + half_height,
			      x + half_width, y + 2);
		gdk_draw_line(window, style->dark_gc[state_type],
			      x + half_width, y + 2,
			      x + width - 2, y + half_height);
		gdk_draw_line(window, style->dark_gc[state_type],
			      x + 1, y + half_height,
			      x + half_width, y + 1);
		gdk_draw_line(window, style->dark_gc[state_type],
			      x + half_width, y + 1,
			      x + width - 1, y + half_height);
		gdk_draw_line(window, style->dark_gc[state_type],
			      x, y + half_height,
			      x + half_width, y);
		gdk_draw_line(window, style->dark_gc[state_type],
			      x + half_width, y,
			      x + width, y + half_height);
		break;
	case GTK_SHADOW_OUT:
		gdk_draw_line(window, style->dark_gc[state_type],
			      x + 2, y + half_height,
			      x + half_width, y + height - 2);
		gdk_draw_line(window, style->dark_gc[state_type],
			      x + half_width, y + height - 2,
			      x + width - 2, y + half_height);
		gdk_draw_line(window, style->dark_gc[state_type],
			      x + 1, y + half_height,
			      x + half_width, y + height - 1);
		gdk_draw_line(window, style->dark_gc[state_type],
			      x + half_width, y + height - 1,
			      x + width - 1, y + half_height);
		gdk_draw_line(window, style->dark_gc[state_type],
			      x, y + half_height,
			      x + half_width, y + height);
		gdk_draw_line(window, style->dark_gc[state_type],
			      x + half_width, y + height,
			      x + width, y + half_height);
		
		gdk_draw_line(window, style->light_gc[state_type],
			      x + 2, y + half_height,
			      x + half_width, y + 2);
		gdk_draw_line(window, style->light_gc[state_type],
			      x + half_width, y + 2,
			      x + width - 2, y + half_height);
		gdk_draw_line(window, style->light_gc[state_type],
			      x + 1, y + half_height,
			      x + half_width, y + 1);
		gdk_draw_line(window, style->light_gc[state_type],
			      x + half_width, y + 1,
			      x + width - 1, y + half_height);
		gdk_draw_line(window, style->light_gc[state_type],
			      x, y + half_height,
			      x + half_width, y);
		gdk_draw_line(window, style->light_gc[state_type],
			      x + half_width, y,
			      x + width, y + half_height);
		break;
	default:
		break;
	}
	if (area) {
		gdk_gc_set_clip_rectangle(style->light_gc[state_type], NULL);
		gdk_gc_set_clip_rectangle(style->bg_gc[state_type], NULL);
		gdk_gc_set_clip_rectangle(style->dark_gc[state_type], NULL);
		gdk_gc_set_clip_rectangle(style->black_gc, NULL);
	}
}

static void
draw_box(GtkStyle *style,
         GdkWindow *window,
         GtkStateType state_type,
         GtkShadowType shadow_type,
         GdkRectangle *area,
         GtkWidget *widget,
         DETAIL_CONST char *detail,
         int x,
         int y,
         int width,
         int height)
{
	GdkGC *light_gc, *dark_gc;
	GtkOrientation orientation;
	
	g_return_if_fail (style != NULL);
	g_return_if_fail (window != NULL);

	sanitize_size (window, &width, &height);

	if (DETAIL ("menuitem") && state_type == GTK_STATE_PRELIGHT) {
		state_type = GTK_STATE_SELECTED;
	}

	light_gc = style->light_gc[state_type];
	dark_gc = style->dark_gc[state_type];
	
	orientation = GTK_ORIENTATION_HORIZONTAL;
	if (height > width)
		orientation = GTK_ORIENTATION_VERTICAL;
	
	if (DETAIL("optionmenutab")) {
		gdk_draw_line (window, style->dark_gc[state_type], 
			       x - 5, y, x - 5, y + height);
		
		gtk_paint_arrow (style, window, state_type, shadow_type, area, 
				 widget, detail, GTK_ARROW_DOWN, TRUE,
				 x + 1, y + 1, width - 2, height - 2);
	} else if (DETAIL("trough")) {
#ifndef GTK1
		gdk_draw_rectangle (window, 
				    style->bg_gc[state_type],
				    TRUE,
				    x, y, width - 1, height - 1);
		gdk_draw_rectangle (window, 
				    style->dark_gc[GTK_STATE_NORMAL],
				    FALSE,
				    x, y, width - 1, height - 1);
#else
		gtk_style_apply_default_background(style, window,
                                                   widget && !GTK_WIDGET_NO_WINDOW(widget),
                                                   GTK_STATE_NORMAL, area,
                                                   x, y, width, height);
		gdk_draw_rectangle (window,
				    style->bg_gc[GTK_STATE_ACTIVE],
				    TRUE,
				    x, y, width - 1, height - 1);
		gdk_draw_rectangle (window,
				    style->dark_gc[state_type],
				    FALSE,
				    x, y, width - 1, height - 1);
#endif
	} else if (DETAIL("menubar")
		   || DETAIL ("dockitem_bin") 
		   || DETAIL ("dockitem") 
		   || DETAIL ("toolbar") 
		   || DETAIL ("handlebox")) {
		if (shadow_type != GTK_SHADOW_NONE) {
			gdk_draw_line (window, 
				       style->dark_gc[GTK_STATE_NORMAL], 
				       x, y + height - 1, 
				       x + width - 1, y + height - 1);

		}
	} else if (DETAIL("tab")) {
		mist_tab(style, window, state_type, 
			 shadow_type, area, widget,
			 detail, x, y, width, height);
	} else  if (DETAIL ("bar")) {
		draw_rect (style, window, 
			   style->dark_gc[GTK_STATE_SELECTED],
			   style->dark_gc[GTK_STATE_SELECTED],
			   style->base_gc[GTK_STATE_SELECTED],
			   x, y, width, height);
	} else if (DETAIL ("buttondefault")) {
#ifdef GTK1
		width -= 1;
		height -= 1;
#endif		
		gdk_draw_rectangle (window, style->fg_gc[GTK_STATE_NORMAL],
				    FALSE,
				    x, y, width - 1, height - 1);
	} else {
#ifndef GTK1
		/* Make sure stepper and slider outlines "overlap" - taken from
		 * bluecurve */
		if (DETAIL("slider") && widget && GTK_IS_RANGE (widget)) {
			GtkAdjustment *adj = GTK_RANGE (widget)->adjustment;
			if (adj->value <= adj->lower &&
			    (GTK_RANGE (widget)->has_stepper_a ||
			     GTK_RANGE (widget)->has_stepper_b)) {
				if (GTK_IS_VSCROLLBAR (widget)) {
					height += 1;
					y -= 1;
				} else if (GTK_IS_HSCROLLBAR (widget)) {
					width += 1;
					x -= 1;
				}
			}
			
			if (adj->value >= adj->upper - adj->page_size &&
			    (GTK_RANGE (widget)->has_stepper_c ||
			     GTK_RANGE (widget)->has_stepper_d)) {
				if (GTK_IS_VSCROLLBAR (widget)) {
					height += 1;
				} else if (GTK_IS_HSCROLLBAR (widget)) {
					width += 1;
				}
			}
		}
#endif
		
		gtk_style_apply_default_background(style, window,
						   widget && !GTK_WIDGET_NO_WINDOW(widget),
						   state_type, area,
						   x, y, width, height);

		shadow_type = get_shadow_type (style, detail, shadow_type);
		if (state_type == GTK_STATE_INSENSITIVE && shadow_type != GTK_SHADOW_NONE) {
			shadow_type = GTK_SHADOW_ETCHED_IN;
		}
		if (shadow_type != GTK_SHADOW_NONE) {
			draw_rect_with_shadow (style, window, widget, 
					       state_type, 
					       shadow_type, NULL, 
					       x, y, width, height);
		}
	}
}

static void
draw_flat_box(GtkStyle *style,
              GdkWindow *window,
              GtkStateType state_type,
              GtkShadowType shadow_type,
              GdkRectangle *area,
              GtkWidget *widget,
              DETAIL_CONST char *detail,
              int x,
              int y,
              int width,
              int height)
{
#ifndef GTK1
	GTK_STYLE_CLASS (parent_class)->draw_flat_box (style, window, 
						       state_type, shadow_type,
						       area, widget, detail, 
						       x, y, width, height);
#else
	GdkGC *gc1;
	
	g_return_if_fail(style != NULL);
	g_return_if_fail(window != NULL);

	sanitize_size (window, &width, &height);

	if (DETAIL("text") && (state_type == GTK_STATE_SELECTED))
		gc1 = style->bg_gc[state_type];
	else if (DETAIL("viewportbin"))
		gc1 = style->bg_gc[state_type];
	else if (DETAIL("entry_bg"))
		gc1 = style->base_gc[state_type];
	else 
		gc1 = style->bg_gc[state_type];
	
	if ((!style->bg_pixmap[state_type]) || (gc1 != style->bg_gc[state_type]) ||
	    (gdk_window_get_type(window) == GDK_WINDOW_PIXMAP)) {
		if (area)
			gdk_gc_set_clip_rectangle(gc1, area);
		
		gdk_draw_rectangle(window, gc1, TRUE,
				   x, y, width, height);
		if (DETAIL("tooltip"))
			gdk_draw_rectangle(window, style->black_gc, FALSE,
					   x, y, width - 1, height - 1);
		if (area)
			gdk_gc_set_clip_rectangle(gc1, NULL);
	} else {
		gtk_style_apply_default_background
			(style, window,
			 widget && !GTK_WIDGET_NO_WINDOW(widget),
			 state_type, area, x, y, width, height);
	}
#endif
}

static void
draw_check(GtkStyle *style,
           GdkWindow *window,
           GtkStateType state_type,
           GtkShadowType shadow_type,
           GdkRectangle *area,
           GtkWidget *widget,
           DETAIL_CONST char *detail,
           int x,
           int y,
           int width,
           int height)
{
	sanitize_size (window, &width, &height);

	gtk_paint_box (style, window, GTK_STATE_NORMAL, 
		       GTK_SHADOW_IN, area, widget,
		       detail, x, y, width - 1, height - 1);

	gdk_draw_rectangle (window, 
			    style->base_gc[state_type == GTK_STATE_INSENSITIVE ? GTK_STATE_INSENSITIVE : GTK_STATE_NORMAL ], 
			    TRUE,
			    x + 1,  y + 1, width - 3, height - 3);
	
	if (shadow_type == GTK_SHADOW_IN) {
		gdk_draw_rectangle(window,
				   widget->style->base_gc[GTK_STATE_SELECTED],
				   TRUE, x + 2, y + 2, width - 5, height - 5);
	} else if (shadow_type == GTK_SHADOW_ETCHED_IN) { /* inconsistent */
#define gray50_width 2
#define gray50_height 2
		GdkBitmap *stipple;
		static const char gray50_bits[] = {
			0x02, 0x01
		};
		
		stipple = gdk_bitmap_create_from_data (window,
						       gray50_bits, 
						       gray50_width,
						       gray50_height);
  
		gdk_gc_set_fill (widget->style->base_gc[GTK_STATE_SELECTED],
				 GDK_STIPPLED);
		gdk_gc_set_stipple (widget->style->base_gc[GTK_STATE_SELECTED],
				    stipple);
		gdk_draw_rectangle(window,
				   widget->style->base_gc[GTK_STATE_SELECTED],
				   TRUE, x + 2, y + 2, width - 5, height - 5);
		gdk_gc_set_fill (widget->style->base_gc[GTK_STATE_SELECTED],
				 GDK_SOLID);

#undef gray50_width
#undef gray50_height 
	}
}

static void
draw_option(GtkStyle *style,
            GdkWindow *window,
            GtkStateType state_type,
            GtkShadowType shadow_type,
            GdkRectangle *area,
            GtkWidget *widget,
            DETAIL_CONST char *detail,
            int x,
            int y,
            int width,
            int height)
{
	sanitize_size (window, &width, &height);

	gdk_draw_arc(window, 
		     style->base_gc[state_type == GTK_STATE_INSENSITIVE ? GTK_STATE_INSENSITIVE : GTK_STATE_NORMAL], 
		     TRUE,
		     x, y, width - 1, height - 1, 0, 360 * 64);
	gdk_draw_arc(window, style->dark_gc[state_type], FALSE, 
		     x, y, width - 1, height - 1, 0, 360 * 64);
	
	if (shadow_type == GTK_SHADOW_IN) {
		gdk_draw_arc(window, style->bg_gc[GTK_STATE_SELECTED], TRUE,
			     x + 2, y + 2, width - 5, height - 5, 0, 360 * 64);
	}
}

static void
draw_tab(GtkStyle *style,
         GdkWindow *window,
         GtkStateType state_type,
         GtkShadowType shadow_type,
         GdkRectangle *area,
         GtkWidget *widget,
         DETAIL_CONST char *detail,
         int x,
         int y,
         int width,
         int height)
{
	g_return_if_fail(style != NULL);
	g_return_if_fail(window != NULL);
	
	gtk_paint_box(style, window, state_type, shadow_type, 
		      area, widget, detail,
		      x, y, width, height);
}

static void
draw_shadow_gap(GtkStyle *style,
                GdkWindow *window,
                GtkStateType state_type,
                GtkShadowType shadow_type,
                GdkRectangle *area,
                GtkWidget *widget,
                DETAIL_CONST char *detail,
                int x,
                int y,
                int width,
                int height,
                GtkPositionType gap_side,
                int gap_x,
                int gap_width)
{
	GdkGC *gc1 = NULL;
	GdkGC *gc2 = NULL;
	
	g_return_if_fail (window != NULL);
	
	sanitize_size (window, &width, &height);
	shadow_type = get_shadow_type (style, detail, shadow_type);
	
	switch (shadow_type) {
	case GTK_SHADOW_NONE:
		return;
	case GTK_SHADOW_IN:
		gc1 = style->dark_gc[state_type];
		gc2 = style->light_gc[state_type];
		break;
	case GTK_SHADOW_OUT:
		gc1 = style->light_gc[state_type];
		gc2 = style->dark_gc[state_type];
		break;
	case GTK_SHADOW_ETCHED_IN:
	case GTK_SHADOW_ETCHED_OUT:
		gc1 = style->dark_gc[state_type];
		gc2 = style->dark_gc[state_type];
	}

	if (area) {
		gdk_gc_set_clip_rectangle (gc1, area);
		gdk_gc_set_clip_rectangle (gc2, area);
	}
	
	switch (gap_side) {
        case GTK_POS_TOP:
		if (gap_x > 0) {
			gdk_draw_line (window, gc1, 
				       x, y, 
				       x + gap_x, y);
		}
		if ((width - (gap_x + gap_width)) > 0) {
			gdk_draw_line (window, gc1, 
				       x + gap_x + gap_width - 1, y,
				       x + width - 1, y);
		}
		gdk_draw_line (window, gc1, 
			       x, y, 
			       x, y + height - 1);
		gdk_draw_line (window, gc2,
			       x + width - 1, y,
			       x + width - 1, y + height - 1);
		gdk_draw_line (window, gc2,
			       x, y + height - 1,
			       x + width - 1, y + height - 1);
		break;
        case GTK_POS_BOTTOM:
		gdk_draw_line (window, gc1,
			       x, y,
			       x + width - 1, y);
		gdk_draw_line (window, gc1, 
			       x, y, 
			       x, y + height - 1);
		gdk_draw_line (window, gc2,
			       x + width - 1, y,
			       x + width - 1, y + height - 1);

		if (gap_x > 0) {
			gdk_draw_line (window, gc2, 
				       x, y + height - 1, 
				       x + gap_x, y + height - 1);
		}
		if ((width - (gap_x + gap_width)) > 0) {
			gdk_draw_line (window, gc2, 
				       x + gap_x + gap_width - 1, y + height - 1,
				       x + width - 1, y + height - 1);
		}
		
		break;
        case GTK_POS_LEFT:
		gdk_draw_line (window, gc1,
			       x, y,
			       x + width - 1, y);
		if (gap_x > 0) {
			gdk_draw_line (window, gc1, 
				       x, y,
				       x, y + gap_x);
		}
		if ((height - (gap_x + gap_width)) > 0) {
			gdk_draw_line (window, gc1, 
				       x, y + gap_x + gap_width - 1,
				       x, y + height - 1);
		}
		gdk_draw_line (window, gc2,
			       x + width - 1, y,
			       x + width - 1, y + height - 1);
		gdk_draw_line (window, gc2,
			       x, y + height - 1,
			       x + width - 1, y + height - 1);
		break;
        case GTK_POS_RIGHT:
		gdk_draw_line (window, gc1,
			       x, y,
			       x + width - 1, y);
		gdk_draw_line (window, gc1, 
			       x, y, 
			       x, y + height - 1);


		if (gap_x > 0) {
			gdk_draw_line (window, gc2, 
				       x + width - 1, y,
				       x + width - 1, y + gap_x);
		}
		if ((height - (gap_x + gap_width)) > 0) {
			gdk_draw_line (window, gc2, 
				       x + width - 1, y + gap_x + gap_width - 1,
				       x + width - 1, y + height - 1);
		}
		gdk_draw_line (window, gc2,
			       x, y + height - 1,
			       x + width - 1, y + height - 1);

	}
	
	if (area) {
		gdk_gc_set_clip_rectangle (gc1, NULL);
		gdk_gc_set_clip_rectangle (gc2, NULL);
	}
}

static void
draw_box_gap(GtkStyle *style,
	     GdkWindow *window,
	     GtkStateType state_type,
	     GtkShadowType shadow_type,
	     GdkRectangle *area,
	     GtkWidget *widget,
	     DETAIL_CONST char *detail,
	     int x,
	     int y,
	     int width,
	     int height,
	     GtkPositionType gap_side,
	     int gap_x,
	     int gap_width)
{
	sanitize_size (window, &width, &height);

	gtk_style_apply_default_background(style, window,
					   widget && !GTK_WIDGET_NO_WINDOW(widget),
					   state_type, area,
					   x, y, width, height);
	draw_shadow_gap (style, window, state_type, shadow_type,
			 area, widget, detail, x, y, width, height, 
			 gap_side, gap_x, gap_width);
}

static void
draw_extension(GtkStyle *style,
               GdkWindow *window,
               GtkStateType state_type,
               GtkShadowType shadow_type,
               GdkRectangle *area,
               GtkWidget *widget,
               DETAIL_CONST char *detail,
               int x,
               int y,
               int width,
               int height,
               GtkPositionType gap_side)
{
	GdkRectangle        rect;
	
	g_return_if_fail(style != NULL);
	g_return_if_fail(window != NULL);
	
	sanitize_size (window, &width, &height);

	gtk_paint_box(style, window, state_type, shadow_type, 
		      area, widget, detail,
		      x, y, width, height);
	
	switch (gap_side) {
	case GTK_POS_TOP:
		rect.x = x + XTHICKNESS(style);
		rect.y = y;
		rect.width = width - XTHICKNESS(style) * 2;
		rect.height = YTHICKNESS(style);
		break;
	case GTK_POS_BOTTOM:
		rect.x = x + XTHICKNESS(style);
		rect.y = y + height - YTHICKNESS(style);
		rect.width = width - XTHICKNESS(style) * 2;
		rect.height = YTHICKNESS(style);
		break;
	case GTK_POS_LEFT:
		rect.x = x;
		rect.y = y + YTHICKNESS(style);
		rect.width = XTHICKNESS(style);
		rect.height = height - YTHICKNESS(style) * 2;
		break;
	case GTK_POS_RIGHT:
		rect.x = x + width - XTHICKNESS(style);
		rect.y = y + YTHICKNESS(style);
		rect.width = XTHICKNESS(style);
		rect.height = height - YTHICKNESS(style) * 2;
		break;
	}
	
	gtk_style_apply_default_background(style, window,
					   widget && !GTK_WIDGET_NO_WINDOW(widget),
					   state_type, area, rect.x, rect.y, rect.width, rect.height);
}

static void
draw_handle(GtkStyle *style,
            GdkWindow *window,
            GtkStateType state_type,
            GtkShadowType shadow_type,
            GdkRectangle *area,
            GtkWidget *widget,
            DETAIL_CONST char *detail,
            int x,
            int y,
            int width,
            int height,
            GtkOrientation orientation)
{
	GdkGC *light_gc, *dark_gc;
	GdkRectangle dest;
	int modx, mody;
	
	g_return_if_fail(style != NULL);
	g_return_if_fail(window != NULL);
	
	sanitize_size (window, &width, &height);

	gtk_paint_box(style, window, state_type, shadow_type, area, widget,
		      detail, x, y, width, height);
	
	light_gc = style->light_gc[state_type];
	dark_gc = style->dark_gc[state_type];
	
	orientation = GTK_ORIENTATION_HORIZONTAL;
	if (height > width)
		orientation = GTK_ORIENTATION_VERTICAL;
	
	dest.x = x + XTHICKNESS(style);
	dest.y = y + YTHICKNESS(style);
	dest.width = width - (XTHICKNESS(style) * 2);
	dest.height = height - (YTHICKNESS(style) * 2);
	
	if (orientation == GTK_ORIENTATION_HORIZONTAL) { 
		modx = 4; mody = 0; 
	} else { 
		modx = 0; mody = 4;
	}
	
	gdk_gc_set_clip_rectangle(light_gc, &dest);
	gdk_gc_set_clip_rectangle(dark_gc, &dest);
	mist_dot(window,
		 light_gc, dark_gc,
		 x + width / 2 - modx,
		 y + height / 2 - mody);
	mist_dot(window,
		 light_gc, dark_gc,
		 x + width / 2,
		 y + height / 2);
	mist_dot(window,
		 light_gc, dark_gc,
		 x + width / 2 + modx,
		 y + height / 2 + mody);
	
	gdk_gc_set_clip_rectangle(light_gc, NULL);
	gdk_gc_set_clip_rectangle(dark_gc, NULL);
}

static void
draw_resize_grip(GtkStyle *style,
		 GdkWindow *window,
		 GtkStateType state_type,
		 GdkRectangle *area,
		 GtkWidget *widget,
		 const char *detail,
		 GdkWindowEdge edge,
		 int x,
		 int y,
		 int width,
		 int height)
{
	GdkGC *light_gc, *dark_gc;
	GdkRectangle dest;
	int xi, yi;
	int max_x, max_y;
	int threshold;
	
	g_return_if_fail(style != NULL);
	g_return_if_fail(window != NULL);
	
	sanitize_size (window, &width, &height);
	
	if (area) {
		gdk_gc_set_clip_rectangle (style->light_gc[state_type], area);
		gdk_gc_set_clip_rectangle (style->dark_gc[state_type], area);
	}

	switch (edge) {
	case GDK_WINDOW_EDGE_NORTH_WEST:
		/* make it square */
		if (width < height) {
			height = width;
		}
		else if (height < width) {
			width = height;
		}
		break;
	case GDK_WINDOW_EDGE_NORTH:
		if (width < height) {
			height = width;
		}
		break;
	case GDK_WINDOW_EDGE_NORTH_EAST:
		/* make it square, aligning to top right */
		if (width < height) {
			height = width;
		} else if (height < width) {
			x += (width - height);
			width = height;
		}
		break;
	case GDK_WINDOW_EDGE_WEST:
		if (height < width) {
			width = height;
		}
		break;
	case GDK_WINDOW_EDGE_EAST:
		/* aligning to right */
		if (height < width) {
			x += (width - height);
			width = height;
		}
		break;
	case GDK_WINDOW_EDGE_SOUTH_WEST:
		/* make it square, aligning to bottom left */
		if (width < height) {
			y += (height - width);
			height = width;
		} else if (height < width) {
			width = height;
		}
		break;
	case GDK_WINDOW_EDGE_SOUTH:
		/* align to bottom */
		if (width < height) {
			y += (height - width);
			height = width;
		}
		break;
	case GDK_WINDOW_EDGE_SOUTH_EAST:
		/* make it square, aligning to bottom right */
		if (width < height) {
			y += (height - width);
			height = width;
		} else if (height < width) {
			x += (width - height);
			width = height;
		}
		break;
	default:
		g_assert_not_reached ();
	}
	
	gtk_style_apply_default_background (style, window, FALSE,
					    state_type, area,
					    x, y, width, height);

	light_gc = style->light_gc[state_type];
	dark_gc = style->dark_gc[state_type];

	max_x = (width - 2) / 5;
	max_y = (height - 2) / 5;
	
	threshold = max_x;
	
	for (xi = 0; xi <= max_x; xi++) {
		for (yi = 0; yi <= max_y; yi++) {
			gboolean draw_dot;
			
			switch (edge) {
			case GDK_WINDOW_EDGE_NORTH:
			case GDK_WINDOW_EDGE_WEST:
			case GDK_WINDOW_EDGE_EAST:
			case GDK_WINDOW_EDGE_SOUTH:
				draw_dot = TRUE;
				break;
			case GDK_WINDOW_EDGE_NORTH_WEST:
				draw_dot = (xi + yi <= threshold);
				break;
			case GDK_WINDOW_EDGE_NORTH_EAST:
				draw_dot = (xi >= yi);
				break;
			case GDK_WINDOW_EDGE_SOUTH_WEST:
				draw_dot = (xi <= yi);
				break;
			case GDK_WINDOW_EDGE_SOUTH_EAST:
				draw_dot = (xi + yi >= threshold);
				break;
			default:
				g_assert_not_reached ();
			}
			
			if (draw_dot) {
				mist_dot(window,
					 light_gc, dark_gc,
					 x + (xi * 5) + 2,
					 y + (yi * 5) + 2);
			}
		}
	}

	if (area)
	{
		gdk_gc_set_clip_rectangle (style->light_gc[state_type], NULL);
		gdk_gc_set_clip_rectangle (style->dark_gc[state_type], NULL);
	}
}

static void
draw_string (GtkStyle      *style,
	     GdkWindow     *window,
	     GtkStateType   state_type,
	     GdkRectangle  *area,
	     GtkWidget     *widget,
	     const char    *detail,
	     int            x,
	     int            y,
	     const char    *string)
{
	GdkDisplay *display;
	
	g_return_if_fail (GTK_IS_STYLE (style));
	g_return_if_fail (window != NULL);
	
	display = gdk_drawable_get_display (window);
	
	if (area) {
		gdk_gc_set_clip_rectangle (style->fg_gc[state_type], area);
	}	
	
	gdk_draw_string (window,
			 gtk_style_get_font (style),
			 style->fg_gc[state_type], x, y, string);
	
	if (area) {
		gdk_gc_set_clip_rectangle (style->fg_gc[state_type], NULL);
	}
}

static void
draw_layout (GtkStyle        *style,
	     GdkWindow       *window,
	     GtkStateType     state_type,
	     gboolean         use_text,
	     GdkRectangle    *area,
	     GtkWidget       *widget,
	     const char      *detail,
	     int              x,
	     int              y,
	     PangoLayout      *layout)
{
	GdkGC *gc;
	
	g_return_if_fail (GTK_IS_STYLE (style));
	g_return_if_fail (window != NULL);
	
	gc = use_text ? style->text_gc[state_type] : style->fg_gc[state_type];
	
	if (area) {
		gdk_gc_set_clip_rectangle (gc, area);
	}
	
	gdk_draw_layout (window, gc, x, y, layout);
	
	if (area) {
		gdk_gc_set_clip_rectangle (gc, NULL);
	}
}

static GdkPixbuf *
set_transparency (const GdkPixbuf *pixbuf, gdouble alpha_percent)
{
	GdkPixbuf *target;
	guchar *data, *current;
	guint x, y, rowstride, height, width;

	g_return_val_if_fail (pixbuf != NULL, NULL);
	g_return_val_if_fail (GDK_IS_PIXBUF (pixbuf), NULL);

	/* Returns a copy of pixbuf with it's non-completely-transparent pixels to
	   have an alpha level "alpha_percent" of their original value. */

	target = gdk_pixbuf_add_alpha (pixbuf, FALSE, 0, 0, 0);

	if (alpha_percent == 1.0)
		return target;
	width = gdk_pixbuf_get_width (target);
	height = gdk_pixbuf_get_height (target);
	rowstride = gdk_pixbuf_get_rowstride (target);
	data = gdk_pixbuf_get_pixels (target);

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			/* The "4" is the number of chars per pixel, in this case, RGBA,
			   the 3 means "skip to the alpha" */
			current = data + (y * rowstride) + (x * 4) + 3; 
			*(current) = (guchar) (*(current) * alpha_percent);
		}
	}

	return target;
}

static GdkPixbuf*
scale_or_ref (GdkPixbuf *src,
              int width,
              int height)
{
	if (width == gdk_pixbuf_get_width (src) &&
	    height == gdk_pixbuf_get_height (src)) {
		return g_object_ref (src);
	} else {
		return gdk_pixbuf_scale_simple (src,
						width, height,
						GDK_INTERP_BILINEAR);
	}
}

static GdkPixbuf *
render_icon (GtkStyle            *style,
	     const GtkIconSource *source,
	     GtkTextDirection     direction,
	     GtkStateType         state,
	     GtkIconSize          size,
	     GtkWidget           *widget,
	     const char          *detail)
{
	int width = 1;
	int height = 1;
	GdkPixbuf *scaled;
	GdkPixbuf *stated;
	GdkPixbuf *base_pixbuf;
	GdkScreen *screen;
	GtkSettings *settings;
	
	/* Oddly, style can be NULL in this function, because
	 * GtkIconSet can be used without a style and if so
	 * it uses this function.
	 */
	
	base_pixbuf = gtk_icon_source_get_pixbuf (source);
	
	g_return_val_if_fail (base_pixbuf != NULL, NULL);
	
	if (widget && gtk_widget_has_screen (widget)) {
		screen = gtk_widget_get_screen (widget);
		settings = gtk_settings_get_for_screen (screen);
	} else if (style->colormap) {
		screen = gdk_colormap_get_screen (style->colormap);
		settings = gtk_settings_get_for_screen (screen);
	} else {
		settings = gtk_settings_get_default ();
		GTK_NOTE (MULTIHEAD,
			  g_warning ("Using the default screen for gtk_default_render_icon()"));
	}
	
  
	if (size != (GtkIconSize) -1 && !gtk_icon_size_lookup_for_settings (settings, size, &width, &height)) {
		g_warning (G_STRLOC ": invalid icon size '%d'", size);
		return NULL;
	}

	/* If the size was wildcarded, and we're allowed to scale, then scale; otherwise,
	 * leave it alone.
	 */
	if (size != (GtkIconSize)-1 && gtk_icon_source_get_size_wildcarded (source))
		scaled = scale_or_ref (base_pixbuf, width, height);
	else
		scaled = g_object_ref (base_pixbuf);
	
	/* If the state was wildcarded, then generate a state. */
	if (gtk_icon_source_get_state_wildcarded (source)) {
		if (state == GTK_STATE_INSENSITIVE) {
			stated = set_transparency (scaled, 0.3);
#if 0
			stated =
				gdk_pixbuf_composite_color_simple (scaled,
								   gdk_pixbuf_get_width (scaled),
								   gdk_pixbuf_get_height (scaled),
								   GDK_INTERP_BILINEAR, 128,
								   gdk_pixbuf_get_width (scaled),
								   style->bg[state].pixel,
								   style->bg[state].pixel);
#endif
			gdk_pixbuf_saturate_and_pixelate (stated, stated,
							  0.1, FALSE);
			
			g_object_unref (scaled);
		} else if (state == GTK_STATE_PRELIGHT) {
			stated = gdk_pixbuf_copy (scaled);      
			
			gdk_pixbuf_saturate_and_pixelate (scaled, stated,
							  1.2, FALSE);
			
			g_object_unref (scaled);
		} else {
			stated = scaled;
		}
	}
	else
		stated = scaled;
  
  return stated;
}

#ifndef GTK1

GType mist_type_style = 0;

void
mist_style_register_type (GTypeModule *module)
{
	static const GTypeInfo object_info = {
		sizeof (MistStyleClass),
		(GBaseInitFunc) NULL,
		(GBaseFinalizeFunc) NULL,
		(GClassInitFunc) mist_style_class_init,
		NULL,           /* class_finalize */
		NULL,           /* class_data */
		sizeof (MistStyle),
		0,              /* n_preallocs */
		(GInstanceInitFunc) mist_style_init,
	};
	
	mist_type_style = g_type_module_register_type (module,
						       GTK_TYPE_STYLE,
						       "MistStyle",
						       &object_info, 0);
}

static void
mist_style_init (MistStyle *style)
{
}

static void
mist_style_class_init (MistStyleClass *klass)
{
	GtkStyleClass *style_class = GTK_STYLE_CLASS (klass);
	
	parent_class = g_type_class_peek_parent (klass);
	
	style_class->draw_hline = draw_hline;
	style_class->draw_vline = draw_vline;
	style_class->draw_shadow = draw_shadow;
	style_class->draw_polygon = draw_polygon;
	style_class->draw_diamond = draw_diamond;
	style_class->draw_box = draw_box;
	style_class->draw_flat_box = draw_flat_box;
	style_class->draw_check = draw_check;
	style_class->draw_option = draw_option;
	style_class->draw_tab = draw_tab;
	style_class->draw_shadow_gap = draw_shadow_gap;
	style_class->draw_box_gap = draw_box_gap;
	style_class->draw_extension = draw_extension;
	style_class->draw_handle = draw_handle;
	style_class->draw_resize_grip = draw_resize_grip;
	style_class->draw_string = draw_string;
	style_class->draw_layout = draw_layout;
	style_class->render_icon = render_icon;
}

#else 

static void
draw_varrow (GdkWindow     *window,
	     GdkGC         *gc,
	     GtkShadowType  shadow_type,
	     GdkRectangle  *area,
	     GtkArrowType   arrow_type,
	     int           x,
	     int           y,
	     int           width,
	     int           height)
{
	int steps, extra;
	int y_start, y_increment;
	int i;
	
	if (area)
		gdk_gc_set_clip_rectangle (gc, area);
	
	width = width + width % 2 - 1;	/* Force odd */
	
	steps = 1 + width / 2;
	
	extra = height - steps;
	
	if (arrow_type == GTK_ARROW_DOWN) {
		y_start = y;
		y_increment = 1;
	} else {
		y_start = y + height - 1;
		y_increment = -1;
	}
	
	for (i = 0; i < extra; i++) {
		gdk_draw_line (window, gc,
			       x,              y_start + i * y_increment,
			       x + width - 1,  y_start + i * y_increment);
	}
	for (; i < height; i++) {
		gdk_draw_line (window, gc,
			       x + (i - extra),              
			       y_start + i * y_increment,
			       x + width - (i - extra) - 1,  
			       y_start + i * y_increment);
	}
	
	if (area)
		gdk_gc_set_clip_rectangle (gc, NULL);
}

static void
draw_harrow (GdkWindow     *window,
	     GdkGC         *gc,
	     GtkShadowType  shadow_type,
	     GdkRectangle  *area,
	     GtkArrowType   arrow_type,
	     int           x,
	     int           y,
	     int           width,
	     int           height)
{
	int steps, extra;
	int x_start, x_increment;
	int i;
	
	if (area)
		gdk_gc_set_clip_rectangle (gc, area);
	
	height = height + height % 2 - 1;	/* Force odd */
	
	steps = 1 + height / 2;
	
	extra = width - steps;
	
	if (arrow_type == GTK_ARROW_RIGHT) {
		x_start = x;
		x_increment = 1;
	} else {
		x_start = x + width - 1;
		x_increment = -1;
	}
	
	for (i = 0; i < extra; i++) {
		gdk_draw_line (window, gc,
			       x_start + i * x_increment, y,
			       x_start + i * x_increment, y + height - 1);
	}
	for (; i < width; i++) {
		gdk_draw_line (window, gc,
			       x_start + i * x_increment, y + (i - extra),
			       x_start + i * x_increment, y + height - (i - extra) - 1);
	}
	
	if (area)
		gdk_gc_set_clip_rectangle (gc, NULL);
}

#define ARROW_LARGE 5
#define ARROW_SMALL 3 

static void
draw_arrow (GtkStyle      *style,
	    GdkWindow     *window,
	    GtkStateType   state,
	    GtkShadowType  shadow,
	    GdkRectangle  *area,
	    GtkWidget     *widget,
	    char         *detail,
	    GtkArrowType   arrow_type,
	    gboolean       fill,
	    int           x,
	    int           y,
	    int           width,
	    int           height)
{
	sanitize_size (window, &width, &height);

	if (detail && strcmp (detail, "spinbutton") == 0) {
		x += (width - 7) / 2;
		
		if (arrow_type == GTK_ARROW_UP)
			y += (height - 4) / 2;
		else
			y += (1 + height - 4) / 2;
		
		draw_varrow (window, style->black_gc, shadow, area, arrow_type,
			     x, y, 7, 4);
	} else if (detail && strcmp (detail, "vscrollbar") == 0) {
		draw_box (style, window, state, shadow, area,
			  widget, detail, x, y, width, height);
		
		x += (width - ARROW_LARGE) / 2;
		y += (height - ARROW_SMALL) / 2;
		
		draw_varrow (window, style->black_gc, shadow, area, arrow_type,
			     x, y, ARROW_LARGE, ARROW_SMALL);		
	} else if (detail && strcmp (detail, "hscrollbar") == 0) {
		draw_box (style, window, state, shadow, area,
			  widget, detail, x, y, width, height);
		
		y += (height - ARROW_LARGE) / 2;
		x += (width - ARROW_SMALL) / 2;
		
		draw_harrow (window, style->black_gc, shadow, area, arrow_type,
			     x, y, ARROW_SMALL, ARROW_LARGE);
	} else {
		if (arrow_type == GTK_ARROW_UP || arrow_type == GTK_ARROW_DOWN) {
			x += (width - ARROW_LARGE) / 2;
			y += (height - ARROW_SMALL) / 2;
			
			draw_varrow (window, style->black_gc, shadow, area, arrow_type,
				     x, y, ARROW_LARGE, ARROW_SMALL);
		} else {
			x += (width - ARROW_SMALL) / 2;
			y += (height - ARROW_LARGE) / 2;
			
			draw_harrow (window, style->black_gc, shadow, area, arrow_type,
				     x, y, ARROW_SMALL, ARROW_LARGE);
		}
	}
}

static void
draw_oval(GtkStyle * style,
          GdkWindow * window,
          GtkStateType state_type,
          GtkShadowType shadow_type,
          GdkRectangle * area,
          GtkWidget * widget,
          char * detail,
          int x,
          int y,
          int width,
          int height)
{
  g_return_if_fail(style != NULL);
  g_return_if_fail(window != NULL);
}
#if 0
static void
draw_string(GtkStyle * style,
            GdkWindow * window,
            GtkStateType state_type,
            GdkRectangle * area,
            GtkWidget * widget,
            char * detail,
            int x,
            int y,
            const char * string)
{
	g_return_if_fail(style != NULL);
	g_return_if_fail(window != NULL);
	
	if (area) {
		if (state_type == GTK_STATE_INSENSITIVE) {
			gdk_gc_set_clip_rectangle(style->base_gc[state_type], area);
		}
		gdk_gc_set_clip_rectangle(style->fg_gc[state_type], area);
	}
	if (state_type == GTK_STATE_INSENSITIVE) {
		gdk_draw_string(window, style->font, style->base_gc[state_type], x + 1, y + 1, string);
	}
	gdk_draw_string(window, style->font, style->fg_gc[state_type], x, y, string);
	if (area) {
		if (state_type == GTK_STATE_INSENSITIVE) {
			gdk_gc_set_clip_rectangle(style->base_gc[state_type], NULL);
		}
		gdk_gc_set_clip_rectangle(style->fg_gc[state_type], NULL);
	}
}
#endif
static void
draw_cross(GtkStyle * style,
           GdkWindow * window,
           GtkStateType state_type,
           GtkShadowType shadow_type,
           GdkRectangle * area,
           GtkWidget * widget,
           char * detail,
           int x,
           int y,
           int width,
           int height)
{
	g_return_if_fail(style != NULL);
	g_return_if_fail(window != NULL);
}

static void
draw_ramp(GtkStyle * style,
          GdkWindow * window,
          GtkStateType state_type,
          GtkShadowType shadow_type,
          GdkRectangle * area,
          GtkWidget * widget,
          char * detail,
          GtkArrowType arrow_type,
          int x,
          int y,
          int width,
          int height)
{
	g_return_if_fail(style != NULL);
	g_return_if_fail(window != NULL);
}

static void
draw_focus (GtkStyle * style,
	    GdkWindow * window,
	    GdkRectangle * area,
	    GtkWidget * widget,
	    char *detail,
	    int x,
	    int y,
	    int width,
	    int height)
{
  g_return_if_fail (style != NULL);
  g_return_if_fail (window != NULL);

  sanitize_size (window, &width, &height);


  if (DETAIL ("button") || DETAIL ("togglebutton") || DETAIL ("optionmenu")) {
	  x += 2;
	  y += 2;
	  width -= 4;
	  height -= 4;
  }

  if (area)
    gdk_gc_set_clip_rectangle (style->black_gc, area);

  if (!(DETAIL ("entry") || DETAIL ("text"))) {
	  gdk_gc_set_line_attributes (style->black_gc, 1, GDK_LINE_ON_OFF_DASH, 0, 0);
	  gdk_gc_set_dashes (style->black_gc, 0, "\1\1", 2);
  }
  
  gdk_draw_rectangle (window,
		      style->black_gc, FALSE,
		      x, y, width, height);
  
  gdk_gc_set_line_attributes (style->black_gc, 1, GDK_LINE_SOLID,
			      0, 0);

  if (area)
	  gdk_gc_set_clip_rectangle (style->black_gc, NULL);
}

static void
draw_slider(GtkStyle * style,
            GdkWindow * window,
            GtkStateType state_type,
            GtkShadowType shadow_type,
            GdkRectangle * area,
            GtkWidget * widget,
            char *detail,
            int x,
            int y,
            int width,
            int height,
            GtkOrientation orientation)
{
	g_return_if_fail(style != NULL);
	g_return_if_fail(window != NULL);
  
	sanitize_size (window, &width, &height);

	gtk_paint_box (style, window, state_type, shadow_type,
		       area, widget, detail, x, y, width, height);
}

GtkStyleClass mist_default_class = {
	2, 2,
	draw_hline,
	draw_vline,
	draw_shadow,
	draw_polygon,
	draw_arrow,
	draw_diamond,
	draw_oval,
	draw_string,
	draw_box,
	draw_flat_box,
	draw_check,
	draw_option,
	draw_cross,
	draw_ramp,
	draw_tab,
	draw_shadow_gap,
	draw_box_gap,
	draw_extension,
	draw_focus,
	draw_slider,
	draw_handle
};

#endif
