#include <math.h>
#include <string.h>
#include <gtk/gtk.h>
#include "metal_style.h"


#define DETAIL(xx)   ((detail) && (!strcmp(xx, detail)))

/**************************************************************************
* GTK Metal Theme
*
* Version 0.9, Oct 2, 1998
*
* Copyright 1998: Randy Gordon, Integrand Systems
*                 http://www.integrand.com
*                 mailto://randy@integrand.com
*
* Heavily modified by Owen Taylor <otaylor@redhat.com>
*
* License: GPL (Gnu Public License)
*
*
**************************************************************************/

static void draw_box               (GtkStyle       *style,
				    GdkWindow      *window,
				    GtkStateType    state_type,
				    GtkShadowType   shadow_type,
				    GdkRectangle   *area,
				    GtkWidget      *widget,
				    const gchar    *detail,
				    gint            x,
				    gint            y,
				    gint            width,
				    gint            height);
static gboolean sanitize_size      (GdkWindow      *window,
				    gint           *width,
				    gint           *height);
static void metal_arrow            (GdkWindow      *window,
				    GtkWidget      *widget,
				    GdkGC          *gc,
				    GtkArrowType    arrow_type,
				    gint            x,
				    gint            y,
				    gint            width,
				    gint            height);
static void metal_scrollbar_trough (GtkStyle       *style,
				    GdkWindow      *window,
				    GtkStateType    state_type,
				    GtkShadowType   shadow_type,
				    GdkRectangle   *area,
				    GtkWidget      *widget,
				    const gchar    *detail,
				    gint            x,
				    gint            y,
				    gint            width,
				    gint            height);
static void metal_scrollbar_slider (GtkStyle       *style,
				    GdkWindow      *window,
				    GtkStateType    state_type,
				    GtkShadowType   shadow_type,
				    GdkRectangle   *area,
				    GtkWidget      *widget,
				    const gchar    *detail,
				    gint            x,
				    gint            y,
				    gint            width,
				    gint            height);
static void metal_scale_trough     (GtkStyle       *style,
				    GdkWindow      *window,
				    GtkStateType    state_type,
				    GtkShadowType   shadow_type,
				    GdkRectangle   *area,
				    GtkWidget      *widget,
				    const gchar    *detail,
				    gint            x,
				    gint            y,
				    gint            width,
				    gint            height);
static void metal_scale_slider     (GtkStyle       *style,
				    GdkWindow      *window,
				    GtkStateType    state_type,
				    GtkShadowType   shadow_type,
				    GdkRectangle   *area,
				    GtkWidget      *widget,
				    const gchar    *detail,
				    gint            x,
				    gint            y,
				    gint            width,
				    gint            height,
				    GtkOrientation  orientation);
static void metal_menu             (GtkStyle       *style,
				    GdkWindow      *window,
				    GtkStateType    state_type,
				    GtkShadowType   shadow_type,
				    GdkRectangle   *area,
				    GtkWidget      *widget,
				    const gchar    *detail,
				    gint            x,
				    gint            y,
				    gint            width,
				    gint            height);
static void metal_menu_item        (GtkStyle       *style,
				    GdkWindow      *window,
				    GtkStateType    state_type,
				    GtkShadowType   shadow_type,
				    GdkRectangle   *area,
				    GtkWidget      *widget,
				    const gchar    *detail,
				    gint            x,
				    gint            y,
				    gint            width,
				    gint            height);
static void metal_notebook         (GtkStyle       *style,
				    GdkWindow      *window,
				    GtkStateType    state_type,
				    GtkShadowType   shadow_type,
				    GdkRectangle   *area,
				    GtkWidget      *widget,
				    const gchar    *detail,
				    gint            x,
				    gint            y,
				    gint            width,
				    gint            height);
static void metal_tab              (GtkStyle       *style,
				    GdkWindow      *window,
				    GtkStateType    state_type,
				    GtkShadowType   shadow_type,
				    GdkRectangle   *area,
				    GtkWidget      *widget,
				    const gchar    *detail,
				    gint            x,
				    gint            y,
				    gint            width,
				    gint            height);
static gboolean is_first_tab       (GtkNotebook    *notebook,
				    gint             x,
				    gint             y);
static void metal_button           (GtkStyle       *style,
				    GdkWindow      *window,
				    GtkStateType    state_type,
				    GtkShadowType   shadow_type,
				    GdkRectangle   *area,
				    GtkWidget      *widget,
				    const gchar    *detail,
				    gint            x,
				    gint            y,
				    gint            width,
				    gint            height);

static GtkStyleClass *parent_class;

static gboolean 
sanitize_size (GdkWindow      *window,
	       gint           *width,
	       gint           *height)
{
  gboolean set_bg = FALSE;

  if ((*width == -1) && (*height == -1))
    {
      set_bg = GDK_IS_WINDOW (window);
      gdk_window_get_size (window, width, height);
    }
  else if (*width == -1)
    gdk_window_get_size (window, width, NULL);
  else if (*height == -1)
    gdk_window_get_size (window, NULL, height);

  return set_bg;
}

static void
draw_hline (GtkStyle * style,
	    GdkWindow * window,
	    GtkStateType state_type,
	    GdkRectangle * area,
	    GtkWidget * widget,
	    const gchar * detail,
	    gint x1,
	    gint x2,
	    gint y)
{
  gint thickness_light;
  gint thickness_dark;
  gint i;
  GdkGC *lightgc, *darkgc;

  g_return_if_fail (style != NULL);
  g_return_if_fail (window != NULL);

  thickness_light = style->ythickness / 2;
  thickness_dark = style->ythickness - thickness_light;

  lightgc = style->light_gc[state_type];
  darkgc = style->dark_gc[state_type];

  if (area)
    {
      gdk_gc_set_clip_rectangle (lightgc, area);
      gdk_gc_set_clip_rectangle (darkgc, area);
    }

  for (i = 0; i < thickness_dark; i++)
    {
      gdk_draw_line (window, lightgc, x2 - i - 1, y + i, x2, y + i);
      gdk_draw_line (window, darkgc, x1, y + i, x2 - i - 1, y + i);
    }

  y += thickness_dark;
  for (i = 0; i < thickness_light; i++)
    {
      gdk_draw_line (window, darkgc, x1, y + i, x1 + thickness_light - i -
		     1, y + i);
      gdk_draw_line (window, lightgc, x1 + thickness_light - i - 1, y + i,
		     x2, y + i);
    }

  if (area)
    {
      gdk_gc_set_clip_rectangle (lightgc, NULL);
      gdk_gc_set_clip_rectangle (darkgc, NULL);
    }
}
/**************************************************************************/
static void
draw_vline (GtkStyle * style,
	    GdkWindow * window,
	    GtkStateType state_type,
	    GdkRectangle * area,
	    GtkWidget * widget,
	    const gchar * detail,
	    gint y1,
	    gint y2,
	    gint x)
{
  gint thickness_light;
  gint thickness_dark;
  gint i;
  GdkGC *lightgc, *darkgc;

  g_return_if_fail (style != NULL);
  g_return_if_fail (window != NULL);

  thickness_light = style->xthickness / 2;
  thickness_dark = style->xthickness - thickness_light;

  lightgc = style->light_gc[state_type];
  darkgc = style->dark_gc[state_type];

  if (area)
    {
      gdk_gc_set_clip_rectangle (lightgc, area);
      gdk_gc_set_clip_rectangle (darkgc, area);
    }

  for (i = 0; i < thickness_dark; i++)
    {
      gdk_draw_line (window, lightgc, x + i, y2 - i - 1, x + i, y2);
      gdk_draw_line (window, darkgc, x + i, y1, x + i, y2 - i - 1);
    }

  x += thickness_dark;

  for (i = 0; i < thickness_light; i++)
    {
      gdk_draw_line (window, darkgc, x + i, y1, x + i, y1 + thickness_light
		     - i);
      gdk_draw_line (window, lightgc, x + i, y1 + thickness_light - i, x +
		     i, y2);
    }

  if (area)
    {
      gdk_gc_set_clip_rectangle (lightgc, NULL);
      gdk_gc_set_clip_rectangle (darkgc, NULL);
    }
}
/**************************************************************************/
static void
draw_shadow (GtkStyle     *style,
	     GdkWindow    *window,
	     GtkStateType  state_type,
	     GtkShadowType shadow_type,
	     GdkRectangle *area,
	     GtkWidget    *widget,
	     const gchar  *detail,
	     gint          x,
	     gint          y,
	     gint          width,
	     gint          height)
{
  GdkGC *gc1 = NULL;		/* Initialize to quiet GCC */
  GdkGC *gc2 = NULL;
  GdkGC *gc3 = NULL;
  GdkGC *gc4 = NULL;
  
  gint thickness_light;
  gint thickness_dark;
  gint i;

  g_return_if_fail (style != NULL);
  g_return_if_fail (window != NULL);

/* return; */
#if DEBUG
  printf ("draw_shadow: %p %p %s %i %i %i %i\n", widget, window, detail, x,
	  y, width, height);
#endif

  if (shadow_type == GTK_SHADOW_NONE)
    return;

  if ((width == -1) && (height == -1))
    gdk_window_get_size (window, &width, &height);
  else if (width == -1)
    gdk_window_get_size (window, &width, NULL);
  else if (height == -1)
    gdk_window_get_size (window, NULL, &height);

  /* Override shadow-type for Metal button */
  if (DETAIL ("button") || DETAIL ("buttondefault"))
    shadow_type = GTK_SHADOW_ETCHED_IN;
  if (DETAIL ("optionmenu"))
    shadow_type = GTK_SHADOW_ETCHED_IN;
  if (DETAIL ("handlebox_bin"))
    shadow_type = GTK_SHADOW_ETCHED_IN;

  /* Short-circuit some metal styles for now */
  if (DETAIL ("frame"))
    {
      gc1 = style->dark_gc[state_type];
      if (area)
	gdk_gc_set_clip_rectangle (gc1, area);
      gdk_draw_rectangle (window, gc1, FALSE, x, y, width - 1, height - 1);
      if (area)
	gdk_gc_set_clip_rectangle (gc1, NULL);
      return;			/* tbd */
    }
  if (DETAIL ("optionmenutab"))
    {
      gc1 = style->black_gc;
      if (area)
	gdk_gc_set_clip_rectangle (gc1, area);
      gdk_draw_line (window, gc1, x, y, x + 10, y);
      gdk_draw_line (window, gc1, x + 1, y + 1, x + 9, y + 1);
      gdk_draw_line (window, gc1, x + 2, y + 2, x + 8, y + 2);
      gdk_draw_line (window, gc1, x + 3, y + 3, x + 7, y + 3);
      gdk_draw_line (window, gc1, x + 4, y + 4, x + 6, y + 4);
      gdk_draw_line (window, gc1, x + 5, y + 5, x + 5, y + 4);
      if (area)
	gdk_gc_set_clip_rectangle (gc1, NULL);
      return;			/* tbd */
    }

  switch (shadow_type)
    {
    case GTK_SHADOW_NONE:
      /* Handled above */
    case GTK_SHADOW_IN:
    case GTK_SHADOW_ETCHED_IN:
      gc1 = style->light_gc[state_type];
      gc2 = style->dark_gc[state_type];
      gc3 = style->black_gc;
      gc4 = style->bg_gc[state_type];
      break;
    case GTK_SHADOW_OUT:
    case GTK_SHADOW_ETCHED_OUT:
      gc1 = style->dark_gc[state_type];
      gc2 = style->light_gc[state_type];
      gc3 = style->black_gc;
      gc4 = style->bg_gc[state_type];
      break;
    }

  if (area)
    {
      gdk_gc_set_clip_rectangle (gc1, area);
      gdk_gc_set_clip_rectangle (gc2, area);
      gdk_gc_set_clip_rectangle (gc3, area);
      gdk_gc_set_clip_rectangle (gc4, area);
    }

  switch (shadow_type)
    {
    case GTK_SHADOW_NONE:
      break;
    case GTK_SHADOW_IN:
      gdk_draw_line (window, gc1,
		     x, y + height - 1, x + width - 1, y + height - 1);
      gdk_draw_line (window, gc1,
		     x + width - 1, y, x + width - 1, y + height - 1);

      gdk_draw_line (window, gc4,
		     x + 1, y + height - 2, x + width - 2, y + height - 2);
      gdk_draw_line (window, gc4,
		     x + width - 2, y + 1, x + width - 2, y + height - 2);

      gdk_draw_line (window, gc3,
		     x + 1, y + 1, x + width - 2, y + 1);
      gdk_draw_line (window, gc3,
		     x + 1, y + 1, x + 1, y + height - 2);

      gdk_draw_line (window, gc2,
		     x, y, x + width - 1, y);
      gdk_draw_line (window, gc2,
		     x, y, x, y + height - 1);
      break;

    case GTK_SHADOW_OUT:
      gdk_draw_line (window, gc1,
		     x + 1, y + height - 2, x + width - 2, y + height - 2);
      gdk_draw_line (window, gc1,
		     x + width - 2, y + 1, x + width - 2, y + height - 2);

      gdk_draw_line (window, gc2,
		     x, y, x + width - 1, y);
      gdk_draw_line (window, gc2,
		     x, y, x, y + height - 1);

      gdk_draw_line (window, gc4,
		     x + 1, y + 1, x + width - 2, y + 1);
      gdk_draw_line (window, gc4,
		     x + 1, y + 1, x + 1, y + height - 2);

      gdk_draw_line (window, gc3,
		     x, y + height - 1, x + width - 1, y + height - 1);
      gdk_draw_line (window, gc3,
		     x + width - 1, y, x + width - 1, y + height - 1);
      break;
    case GTK_SHADOW_ETCHED_IN:
    case GTK_SHADOW_ETCHED_OUT:
      thickness_light = 1;
      thickness_dark = 1;

      for (i = 0; i < thickness_dark; i++)
	{
	  gdk_draw_line (window, gc1,
			 x + i,
			 y + height - i - 1,
			 x + width - i - 1,
			 y + height - i - 1);
	  gdk_draw_line (window, gc1,
			 x + width - i - 1,
			 y + i,
			 x + width - i - 1,
			 y + height - i - 1);

	  gdk_draw_line (window, gc2,
			 x + i,
			 y + i,
			 x + width - i - 2,
			 y + i);
	  gdk_draw_line (window, gc2,
			 x + i,
			 y + i,
			 x + i,
			 y + height - i - 2);
	}

      for (i = 0; i < thickness_light; i++)
	{
	  gdk_draw_line (window, gc1,
			 x + thickness_dark + i,
			 y + thickness_dark + i,
			 x + width - thickness_dark - i - 1,
			 y + thickness_dark + i);
	  gdk_draw_line (window, gc1,
			 x + thickness_dark + i,
			 y + thickness_dark + i,
			 x + thickness_dark + i,
			 y + height - thickness_dark - i - 1);

	  gdk_draw_line (window, gc2,
			 x + thickness_dark + i,
			 y + height - thickness_light - i - 1,
			 x + width - thickness_light - 1,
			 y + height - thickness_light - i - 1);
	  gdk_draw_line (window, gc2,
			 x + width - thickness_light - i - 1,
			 y + thickness_dark + i,
			 x + width - thickness_light - i - 1,
			 y + height - thickness_light - 1);
	}
      break;
    }

  if (area)
    {
      gdk_gc_set_clip_rectangle (gc1, NULL);
      gdk_gc_set_clip_rectangle (gc2, NULL);
      gdk_gc_set_clip_rectangle (gc3, NULL);
      gdk_gc_set_clip_rectangle (gc4, NULL);
    }
}
/**************************************************************************/
static void
draw_polygon (GtkStyle * style,
	      GdkWindow * window,
	      GtkStateType state_type,
	      GtkShadowType shadow_type,
	      GdkRectangle * area,
	      GtkWidget * widget,
	      const gchar * detail,
	      GdkPoint * points,
	      gint npoints,
	      gint fill)
{
#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif /* M_PI */
#ifndef M_PI_4
#define M_PI_4  0.78539816339744830962
#endif /* M_PI_4 */

  static const gdouble pi_over_4 = M_PI_4;
  static const gdouble pi_3_over_4 = M_PI_4 * 3;

  GdkGC *gc1;
  GdkGC *gc2;
  GdkGC *gc3;
  GdkGC *gc4;
  gdouble angle;
  gint xadjust;
  gint yadjust;
  gint i;

  g_return_if_fail (style != NULL);
  g_return_if_fail (window != NULL);
  g_return_if_fail (points != NULL);

#if DEBUG
  printf ("draw_polygon: %p %p %s\n", widget, window, detail);
#endif

  switch (shadow_type)
    {
    case GTK_SHADOW_IN:
      gc1 = style->bg_gc[state_type];
      gc2 = style->dark_gc[state_type];
      gc3 = style->light_gc[state_type];
      gc4 = style->black_gc;
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
      gc3 = style->black_gc;
      gc4 = style->bg_gc[state_type];
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

  if (area)
    {
      gdk_gc_set_clip_rectangle (gc1, area);
      gdk_gc_set_clip_rectangle (gc2, area);
      gdk_gc_set_clip_rectangle (gc3, area);
      gdk_gc_set_clip_rectangle (gc4, area);
    }

  if (fill)
    gdk_draw_polygon (window, style->bg_gc[state_type], TRUE, points, npoints);

  npoints--;

  for (i = 0; i < npoints; i++)
    {
      if ((points[i].x == points[i + 1].x) &&
	  (points[i].y == points[i + 1].y))
	{
	  angle = 0;
	}
      else
	{
	  angle = atan2 (points[i + 1].y - points[i].y,
			 points[i + 1].x - points[i].x);
	}

      if ((angle > -pi_3_over_4) && (angle < pi_over_4))
	{
	  if (angle > -pi_over_4)
	    {
	      xadjust = 0;
	      yadjust = 1;
	    }
	  else
	    {
	      xadjust = 1;
	      yadjust = 0;
	    }

	  gdk_draw_line (window, gc1,
			 points[i].x - xadjust, points[i].y - yadjust,
			 points[i + 1].x - xadjust, points[i + 1].y - yadjust);
	  gdk_draw_line (window, gc3,
			 points[i].x, points[i].y,
			 points[i + 1].x, points[i + 1].y);
	}
      else
	{
	  if ((angle < -pi_3_over_4) || (angle > pi_3_over_4))
	    {
	      xadjust = 0;
	      yadjust = 1;
	    }
	  else
	    {
	      xadjust = 1;
	      yadjust = 0;
	    }

	  gdk_draw_line (window, gc4,
			 points[i].x + xadjust, points[i].y + yadjust,
			 points[i + 1].x + xadjust, points[i + 1].y + yadjust);
	  gdk_draw_line (window, gc2,
			 points[i].x, points[i].y,
			 points[i + 1].x, points[i + 1].y);
	}
    }
  if (area)
    {
      gdk_gc_set_clip_rectangle (gc1, NULL);
      gdk_gc_set_clip_rectangle (gc2, NULL);
      gdk_gc_set_clip_rectangle (gc3, NULL);
      gdk_gc_set_clip_rectangle (gc4, NULL);
    }
}

static void
scrollbar_stepper (GtkStyle     *style,
		   GdkWindow    *window,
		   GtkStateType  state_type,
		   GdkRectangle *area,
		   GtkWidget    *widget,
		   const gchar  *detail,
		   GtkArrowType  arrow_type,
		   gint          x,
		   gint          y,
		   gint          width,
		   gint          height)
{
  GdkRectangle clip;
  
  MetalStyle *metal_style = METAL_STYLE (style);
  
  clip.x = x;
  clip.y = y;
  clip.width = width;
  clip.height = height;

  if (area)
    gdk_rectangle_intersect (&clip, area, &clip);
  
  /* We draw the last couple of pixels of the sliders on
   * the trough, since the slider should go over them at
   * the ends.
   */
  switch (arrow_type)
    {
    case GTK_ARROW_RIGHT:
      x -= 2;
      /* fall through */
    case GTK_ARROW_LEFT:
      width += 2;
      break;
    case GTK_ARROW_DOWN:
      y -= 2;
      /* fall through */
    case GTK_ARROW_UP:
      height += 2;
      break;
    }

  gdk_gc_set_clip_rectangle (metal_style->dark_gray_gc, &clip);
  gdk_gc_set_clip_rectangle (metal_style->light_gray_gc, &clip);
  gdk_gc_set_clip_rectangle (style->white_gc, &clip);

  gdk_draw_rectangle (window, style->white_gc, FALSE,
		      x + 1, y + 1, width - 2, height - 2);
  gdk_draw_rectangle (window, metal_style->dark_gray_gc, FALSE,
		      x, y, width - 2, height - 2);

  if (arrow_type != GTK_ARROW_RIGHT)
    gdk_draw_point (window, metal_style->light_gray_gc,
		    x + 1, y + height - 2);
  
  if (arrow_type != GTK_ARROW_DOWN)
    gdk_draw_point (window, metal_style->light_gray_gc,
		    x + width - 2, y + 1);
		 
  gdk_gc_set_clip_rectangle (metal_style->dark_gray_gc, NULL);
  gdk_gc_set_clip_rectangle (metal_style->light_gray_gc, NULL);
  gdk_gc_set_clip_rectangle (style->white_gc, NULL);
}

/* This function makes up for some brokeness in gtkrange.c
 * where we never get the full arrow of the stepper button
 * and the type of button in a single drawing function.
 *
 * It doesn't work correctly when the scrollbar is squished
 * to the point we don't have room for full-sized steppers.
 */
static void
reverse_engineer_stepper_box (GtkWidget    *range,
			      GtkArrowType  arrow_type,
			      gint         *x,
			      gint         *y,
			      gint         *width,
			      gint         *height)
{
  gint slider_width = 17, stepper_size = 15;
  gint box_width;
  gint box_height;
  
  if (range)
    {
      gtk_widget_style_get (range,
			    "slider_width", &slider_width,
			    "stepper_size", &stepper_size,
			    NULL);
    }
	
  if (arrow_type == GTK_ARROW_UP || arrow_type == GTK_ARROW_DOWN)
    {
      box_width = slider_width;
      box_height = stepper_size;
    }
  else
    {
      box_width = stepper_size;
      box_height = slider_width;
    }

  *x = *x - (box_width - *width) / 2;
  *y = *y - (box_height - *height) / 2;
  *width = box_width;
  *height = box_height;
}

/**************************************************************************/
static void
draw_arrow (GtkStyle * style,
	    GdkWindow * window,
	    GtkStateType state_type,
	    GtkShadowType shadow_type,
	    GdkRectangle * area,
	    GtkWidget * widget,
	    const gchar * detail,
	    GtkArrowType arrow_type,
	    gint fill,
	    gint x,
	    gint y,
	    gint width,
	    gint height)
{
  GdkGC *gc;
  gboolean set_bg;

  g_return_if_fail (style != NULL);
  g_return_if_fail (window != NULL);

  set_bg = sanitize_size (window, &width, &height);
  
  gc = style->black_gc;

  if (DETAIL ("menuitem"))
    gc = style->fg_gc[state_type];

  if (DETAIL ("hscrollbar") || DETAIL ("vscrollbar"))
    {
      /* We need to restore the full area of the entire box,
       * not just the restricted area of the stepper.
       */
      reverse_engineer_stepper_box (widget, arrow_type,
				    &x, &y, &width, &height);
      
      scrollbar_stepper (style, window, state_type, area,
			 widget, detail, arrow_type,
			 x, y, width, height);

      x += 4;
      y += 4;
      width -= 9;
      height -= 9;
    }
  else if (DETAIL ("spinbutton"))
    {
      x += 2;
      width -= 4;
    }

  if (area)
    gdk_gc_set_clip_rectangle (gc, area);

  metal_arrow (window, widget, gc, arrow_type, x, y, width, height);

  if (area)
    gdk_gc_set_clip_rectangle (gc, NULL);
}
/**************************************************************************/
static void
metal_arrow (GdkWindow * window, GtkWidget * widget, GdkGC * gc,
	     GtkArrowType arrow_type,
	     gint x, gint y, gint width, gint height)
{
  int base, span, xoffset, yoffset;
  int i;

  switch (arrow_type)
    {
    case GTK_ARROW_UP:
      base = width;
      // if (base % 2 == 0)
      //	base--;
      xoffset = (width - base) / 2;
      span = (base + 1) / 2;
      yoffset = (height + span) / 2 - 1;
      for (i = 0; i < span; i++)
	{
	  gdk_draw_line (window, gc, x + xoffset + i, y + yoffset - i,
			 x + xoffset + base - 1 - i, y + yoffset - i);
	}
      break;
    case GTK_ARROW_DOWN:
      base = width;
      // if (base % 2 == 0)
      // base--;
      xoffset = (width - base) / 2;
      span = (base + 1) / 2;
      yoffset = (height - span) / 2;
      for (i = 0; i < span; i++)
	{
	  gdk_draw_line (window, gc, x + xoffset + i, y + yoffset + i,
			 x + xoffset + base - 1 - i, y + yoffset + i);
	}
      break;
    case GTK_ARROW_RIGHT:
      if (GTK_CHECK_TYPE (widget, gtk_menu_item_get_type ()))
	{
	  base = 7;
	}
      else
	{
	  base = height;
	  // if (base % 2 == 0)
	  // 	    base--;
	}
      yoffset = (height - base) / 2;
      span = (base + 1) / 2;
      xoffset = (width - span) / 2;
      for (i = 0; i < span; i++)
	{
	  gdk_draw_line (window, gc, x + xoffset + i, y + yoffset + i,
			 x + xoffset + i, y + yoffset + base - 1 - i);
	}
      break;
    case GTK_ARROW_LEFT:
      base = height;
      //      if (base % 2 == 0)
      //	base--;
      yoffset = (height - base) / 2;
      span = (base + 1) / 2;
      xoffset = (width + span) / 2 - 1;
      for (i = 0; i < span; i++)
	{
	  gdk_draw_line (window, gc, x + xoffset - i, y + yoffset + i,
			 x + xoffset - i, y + yoffset + base - 1 - i);
	}
      break;
    }
}
/**************************************************************************/
static void
draw_diamond (GtkStyle * style,
	      GdkWindow * window,
	      GtkStateType state_type,
	      GtkShadowType shadow_type,
	      GdkRectangle * area,
	      GtkWidget * widget,
	      const gchar * detail,
	      gint x,
	      gint y,
	      gint width,
	      gint height)
{
  gint half_width;
  gint half_height;

  g_return_if_fail (style != NULL);
  g_return_if_fail (window != NULL);

  sanitize_size (window, &width, &height);

  half_width = width / 2;
  half_height = height / 2;

  if (area)
    {
      gdk_gc_set_clip_rectangle (style->light_gc[state_type], area);
      gdk_gc_set_clip_rectangle (style->bg_gc[state_type], area);
      gdk_gc_set_clip_rectangle (style->dark_gc[state_type], area);
      gdk_gc_set_clip_rectangle (style->black_gc, area);
    }

  switch (shadow_type)
    {
    case GTK_SHADOW_IN:
      gdk_draw_line (window, style->bg_gc[state_type],
		     x + 2, y + half_height,
		     x + half_width, y + height - 2);
      gdk_draw_line (window, style->bg_gc[state_type],
		     x + half_width, y + height - 2,
		     x + width - 2, y + half_height);
      gdk_draw_line (window, style->light_gc[state_type],
		     x + 1, y + half_height,
		     x + half_width, y + height - 1);
      gdk_draw_line (window, style->light_gc[state_type],
		     x + half_width, y + height - 1,
		     x + width - 1, y + half_height);
      gdk_draw_line (window, style->light_gc[state_type],
		     x, y + half_height,
		     x + half_width, y + height);
      gdk_draw_line (window, style->light_gc[state_type],
		     x + half_width, y + height,
		     x + width, y + half_height);

      gdk_draw_line (window, style->black_gc,
		     x + 2, y + half_height,
		     x + half_width, y + 2);
      gdk_draw_line (window, style->black_gc,
		     x + half_width, y + 2,
		     x + width - 2, y + half_height);
      gdk_draw_line (window, style->dark_gc[state_type],
		     x + 1, y + half_height,
		     x + half_width, y + 1);
      gdk_draw_line (window, style->dark_gc[state_type],
		     x + half_width, y + 1,
		     x + width - 1, y + half_height);
      gdk_draw_line (window, style->dark_gc[state_type],
		     x, y + half_height,
		     x + half_width, y);
      gdk_draw_line (window, style->dark_gc[state_type],
		     x + half_width, y,
		     x + width, y + half_height);
      break;
    case GTK_SHADOW_OUT:
      gdk_draw_line (window, style->dark_gc[state_type],
		     x + 2, y + half_height,
		     x + half_width, y + height - 2);
      gdk_draw_line (window, style->dark_gc[state_type],
		     x + half_width, y + height - 2,
		     x + width - 2, y + half_height);
      gdk_draw_line (window, style->dark_gc[state_type],
		     x + 1, y + half_height,
		     x + half_width, y + height - 1);
      gdk_draw_line (window, style->dark_gc[state_type],
		     x + half_width, y + height - 1,
		     x + width - 1, y + half_height);
      gdk_draw_line (window, style->black_gc,
		     x, y + half_height,
		     x + half_width, y + height);
      gdk_draw_line (window, style->black_gc,
		     x + half_width, y + height,
		     x + width, y + half_height);

      gdk_draw_line (window, style->bg_gc[state_type],
		     x + 2, y + half_height,
		     x + half_width, y + 2);
      gdk_draw_line (window, style->bg_gc[state_type],
		     x + half_width, y + 2,
		     x + width - 2, y + half_height);
      gdk_draw_line (window, style->light_gc[state_type],
		     x + 1, y + half_height,
		     x + half_width, y + 1);
      gdk_draw_line (window, style->light_gc[state_type],
		     x + half_width, y + 1,
		     x + width - 1, y + half_height);
      gdk_draw_line (window, style->light_gc[state_type],
		     x, y + half_height,
		     x + half_width, y);
      gdk_draw_line (window, style->light_gc[state_type],
		     x + half_width, y,
		     x + width, y + half_height);
      break;
    default:
      break;
    }

  if (area)
    {
      gdk_gc_set_clip_rectangle (style->light_gc[state_type], NULL);
      gdk_gc_set_clip_rectangle (style->bg_gc[state_type], NULL);
      gdk_gc_set_clip_rectangle (style->dark_gc[state_type], NULL);
      gdk_gc_set_clip_rectangle (style->black_gc, NULL);
    }
}
/**************************************************************************/
static void
draw_string (GtkStyle * style,
	     GdkWindow * window,
	     GtkStateType state_type,
	     GdkRectangle * area,
	     GtkWidget * widget,
	     const gchar * detail,
	     gint x,
	     gint y,
	     const gchar * string)
{
  GdkGC *fggc, *whitegc, *midgc;
  MetalStyle *metal_style = METAL_STYLE (style);

  g_return_if_fail (style != NULL);
  g_return_if_fail (window != NULL);

#if DEBUG
  printf ("draw_string: %p %p %s %i %i\n", widget, window, detail, x, y);
#endif

  if (DETAIL ("label"))
    {
      fggc = style->black_gc;
      whitegc = style->white_gc;
      midgc = metal_style->mid_gray_gc;
    }
  else
    {
      fggc = style->fg_gc[state_type];
      whitegc = style->white_gc;
      midgc = metal_style->mid_gray_gc;
    }

  if (area)
    {
      gdk_gc_set_clip_rectangle (fggc, area);
      gdk_gc_set_clip_rectangle (whitegc, area);
      gdk_gc_set_clip_rectangle (midgc, area);
    }

  
  if (state_type == GTK_STATE_INSENSITIVE)
    {
      gdk_draw_string (window, gtk_style_get_font (style), whitegc, x + 1, y + 1, string);
      gdk_draw_string (window, gtk_style_get_font (style), midgc, x, y, string);
    }
  else
    {
      gdk_draw_string (window, gtk_style_get_font (style), fggc, x, y, string);
    }

  if (area)
    {
      gdk_gc_set_clip_rectangle (fggc, NULL);
      gdk_gc_set_clip_rectangle (whitegc, NULL);
      gdk_gc_set_clip_rectangle (midgc, NULL);
    }
}

/**************************************************************************/
static void
draw_box (GtkStyle      *style,
	  GdkWindow     *window,
	  GtkStateType   state_type,
	  GtkShadowType  shadow_type,
	  GdkRectangle  *area,
	  GtkWidget     *widget,
	  const gchar   *detail,
	  gint           x,
	  gint           y,
	  gint           width,
	  gint           height)
{
  g_return_if_fail (style != NULL);
  g_return_if_fail (window != NULL);

  if ((width == -1) && (height == -1))
    gdk_window_get_size (window, &width, &height);
  else if (width == -1)
    gdk_window_get_size (window, &width, NULL);
  else if (height == -1)
    gdk_window_get_size (window, NULL, &height);

#if DEBUG
  printf ("draw_box: %p %p %s %i %i %i %i\n", widget, window, detail, x, y,
	  width, height);
#endif

  /* ===================================================================== */

  if (widget && DETAIL ("trough"))
    {

      if (GTK_IS_PROGRESS_BAR (widget))
	{
	  if (area)
	    gdk_gc_set_clip_rectangle (style->light_gc[GTK_STATE_NORMAL], area);
	  gdk_draw_rectangle (window, style->light_gc[GTK_STATE_NORMAL],
			      TRUE, x, y, width, height);
	  if (area)
	    gdk_gc_set_clip_rectangle (style->light_gc[GTK_STATE_NORMAL], NULL);
	  gtk_paint_shadow (style, window, state_type, shadow_type, area,
			    widget, detail,
			    x, y, width, height);
	}
      else if (GTK_IS_SCROLLBAR (widget))
	{
	  metal_scrollbar_trough (style, window, state_type, shadow_type,
				  area, widget, detail, x, y, width, height);
	}
      else if (GTK_IS_SCALE (widget))
	{
	  metal_scale_trough (style, window, state_type, shadow_type,
			      area, widget, detail, x, y, width, height);
	}
      else
	{
#if 0
	  GdkPixmap *pm;
	  gint xthik;
	  gint ythik;

	  xthik = style->xthickness;
	  ythik = style->ythickness;

	  pm = gdk_pixmap_new (window, 2, 2, -1);

	  gdk_draw_point (pm, style->bg_gc[GTK_STATE_NORMAL], 0, 0);
	  gdk_draw_point (pm, style->bg_gc[GTK_STATE_NORMAL], 1, 1);
	  gdk_draw_point (pm, style->light_gc[GTK_STATE_NORMAL], 1, 0);
	  gdk_draw_point (pm, style->light_gc[GTK_STATE_NORMAL], 0, 1);
	  gdk_window_set_back_pixmap (window, pm, FALSE);
	  gdk_window_clear (window);

	  gdk_pixmap_unref (pm);
#endif /* 0 */
	}
    }
  else if (DETAIL ("menu"))
    {
      metal_menu (style, window, state_type, shadow_type,
		  area, widget, detail, x, y, width, height);
    }
  else if (DETAIL ("menuitem"))
    {
      metal_menu_item (style, window, state_type, shadow_type,
		       area, widget, detail, x, y, width, height);
    }
  else if (DETAIL ("bar"))
    {
      if (area)
	gdk_gc_set_clip_rectangle (style->bg_gc[GTK_STATE_SELECTED], area);
      gdk_draw_rectangle (window, style->bg_gc[GTK_STATE_SELECTED],
			  TRUE, x + 1, y + 1, width - 2, height - 2);
      if (area)
	gdk_gc_set_clip_rectangle (style->bg_gc[GTK_STATE_SELECTED], NULL);
    }
  else if (DETAIL ("menubar"))
    {
      if (area)
	gdk_gc_set_clip_rectangle (style->bg_gc[state_type], area);
      gdk_draw_rectangle (window, style->bg_gc[state_type], TRUE,
			  x, y, width, height);
      if (area)
	gdk_gc_set_clip_rectangle (style->bg_gc[state_type], NULL);
    }
  else if (DETAIL ("notebook"))
    {
      metal_notebook (style, window, state_type, shadow_type,
		      area, widget, detail, x, y, width, height);
    }
  else if (DETAIL ("tab"))
    {
      metal_tab (style, window, state_type, shadow_type,
		 area, widget, detail, x, y, width, height);
    }
  else if (DETAIL ("button") || DETAIL ("togglebutton"))
    {
      metal_button (style, window, state_type, shadow_type,
		    area, widget, detail, x, y, width, height);
    }
  else if (DETAIL ("buttondefault"))
    {
    }
  else if (DETAIL ("hscrollbar") || DETAIL ("vscrollbar"))
    {
      /* We do all the drawing in draw_arrow () */
    }
  else
    {
      if ((!style->bg_pixmap[state_type]) || GDK_IS_PIXMAP (window))
	{
	  if (area)
	    gdk_gc_set_clip_rectangle (style->bg_gc[state_type], area);
	  gdk_draw_rectangle (window, style->bg_gc[state_type], TRUE,
			      x, y, width, height);
	  if (area)
	    gdk_gc_set_clip_rectangle (style->bg_gc[state_type], NULL);
	}
      else
	{
	  gtk_style_apply_default_pixmap (style, window, state_type, area,
					  x, y, width, height);
	}
      gtk_paint_shadow (style, window, state_type, shadow_type, area,
			widget, detail,
			x, y, width, height);
    }
}
/**************************************************************************/
static void
metal_scrollbar_trough (GtkStyle     *style,
			GdkWindow    *window,
			GtkStateType  state_type,
			GtkShadowType shadow_type,
			GdkRectangle *area,
			GtkWidget    *widget,
			const char   *detail,
			gint          x,
			gint          y,
			gint          width,
			gint          height)
{
  MetalStyle *metal_style = METAL_STYLE (style);
  GdkGC *lightgc, *midgc, *darkgc, *whitegc;
  gint stepper_size = 15;

  if (widget && GTK_IS_RANGE (widget))
    gtk_widget_style_get (widget, "stepper_size", &stepper_size, NULL);

  stepper_size += 2;

  /* Get colors */
  lightgc = metal_style->light_gray_gc;
  midgc = metal_style->mid_gray_gc;
  darkgc = metal_style->dark_gray_gc;
  whitegc = style->white_gc;

  /* Set Clip Region */
  if (area)
    {
      gdk_gc_set_clip_rectangle (lightgc, area);
      gdk_gc_set_clip_rectangle (midgc, area);
      gdk_gc_set_clip_rectangle (darkgc, area);
      gdk_gc_set_clip_rectangle (whitegc, area);
    }

  /* Draw backgound */
  gdk_draw_rectangle (window, lightgc, TRUE, x, y, width, height);

  /* Draw border */
  gdk_draw_rectangle (window, darkgc, FALSE, x, y, width - 2, height - 2);
  
  /* Draw inset shadow */
  if (GTK_CHECK_TYPE (widget, gtk_hscrollbar_get_type ()))
    {
      gdk_draw_line (window, whitegc, 
		     x + 1,         y + height - 1,
		     x + width - 1, y + height - 1);
      gdk_draw_line (window, darkgc, 
		     x + stepper_size - 2, y + 2, 
		     x + stepper_size - 2, y + height - 2);      
      gdk_draw_line (window, midgc, 
		     x + stepper_size - 1,         y + 1, 
		     x + width - stepper_size - 1, y + 1);
      gdk_draw_line (window, darkgc, 
		     x + width - stepper_size, y + 2, 
		     x + width - stepper_size, y + height - 2);
      gdk_draw_line (window, whitegc, 
		     x + width - stepper_size + 1, y + 1, 
		     x + width - stepper_size + 1, y + height - 2);
      gdk_draw_line (window, midgc,
		     x + stepper_size - 1, y + 1, 
		     x + stepper_size - 1, y + height - 3);
    }
  else
    {
      gdk_draw_line (window, whitegc, 
		     x + width - 1, y + 1,
		     x + width - 1, y + height - 1);
      gdk_draw_line (window, darkgc, 
		     x + 2,         y + stepper_size - 2, 
		     x + width - 2, y + stepper_size - 2);      
      gdk_draw_line (window, midgc, 
		     x + 1, y + stepper_size - 1, 
		     x + 1, y + height - stepper_size - 1);
      gdk_draw_line (window, darkgc, 
		     x + 2,         y + height - stepper_size, 
		     x + width - 2, y + height - stepper_size);
      gdk_draw_line (window, whitegc, 
		     x + 1,         y + height - stepper_size + 1, 
		     x + width - 2, y + height - stepper_size + 1);
      gdk_draw_line (window, midgc,
		     x + 1,         y + stepper_size - 1, 
		     x + width - 3, y + stepper_size - 1);
    }

  /* Reset Clip Region */
  if (area)
    {
      gdk_gc_set_clip_rectangle (lightgc, NULL);
      gdk_gc_set_clip_rectangle (midgc, NULL);
      gdk_gc_set_clip_rectangle (darkgc, NULL);
      gdk_gc_set_clip_rectangle (whitegc, NULL);
    }
}
/**************************************************************************/
static void
metal_scrollbar_slider (GtkStyle * style,
			GdkWindow * window,
			GtkStateType state_type,
			GtkShadowType shadow_type,
			GdkRectangle * area,
			GtkWidget * widget,
			const gchar * detail,
			gint x,
			gint y,
			gint width,
			gint height)
{
  MetalStyle *metal_style = METAL_STYLE (style);
  GdkPixmap *pm;
  GdkGC *fillgc;
  GdkGCValues values;
  GdkGC *lightgc, *midgc, *darkgc, *whitegc;
  int w, h;

  gint stepper_size = 15;

  if (widget && GTK_IS_RANGE (widget))
    gtk_widget_style_get (widget, "stepper_size", &stepper_size, NULL);

  stepper_size += 2;

  lightgc = style->bg_gc[GTK_STATE_PRELIGHT];
  midgc = style->bg_gc[GTK_STATE_SELECTED];
  darkgc = style->fg_gc[GTK_STATE_PRELIGHT];
  whitegc = style->white_gc;

  /* Draw textured surface */
  pm = gdk_pixmap_new (window, 4, 4, -1);

  gdk_draw_rectangle (pm, midgc, TRUE, 0, 0, 4, 4);

  gdk_draw_point (pm, darkgc, 0, 0);
  gdk_draw_point (pm, lightgc, 1, 1);
  gdk_draw_point (pm, darkgc, 2, 2);
  gdk_draw_point (pm, lightgc, 3, 3);

  values.fill = GDK_TILED;
  values.ts_x_origin = (x + 5) % 4;
  values.ts_y_origin = (y + 3) % 4;
  fillgc = gdk_gc_new_with_values (window, &values,
				   GDK_GC_FILL | GDK_GC_TS_X_ORIGIN | GDK_GC_TS_Y_ORIGIN);
  gdk_gc_set_tile (fillgc, pm);

  /* Set Clip Region */
  if (area)
    {
      gdk_gc_set_clip_rectangle (lightgc, area);
      gdk_gc_set_clip_rectangle (midgc, area);
      gdk_gc_set_clip_rectangle (darkgc, area);
      gdk_gc_set_clip_rectangle (whitegc, area);
      gdk_gc_set_clip_rectangle (fillgc, area);
    }

  /* Draw backgound */
  gdk_draw_rectangle (window, midgc, TRUE, x, y, width, height);

  /* Draw border */
  gdk_draw_rectangle (window, lightgc, FALSE, x + 1, y + 1,
		      width - 2, height - 2);
  gdk_draw_rectangle (window, darkgc, FALSE, x + 0, y + 0,
		      width - 2, height - 2);

  if (GTK_CHECK_TYPE (widget, gtk_hscrollbar_get_type ()))
    {
      gdk_draw_line (window, whitegc, 
		     x + 0,         y + height - 1, 
		     x + width - 1, y + height - 1);
      gdk_draw_line (window, metal_style->dark_gray_gc,
		     x,         y + height - 2,
		     x + width, y + height - 2);
      gdk_draw_point (window, metal_style->dark_gray_gc, x + width - 1, y);

      /* At the right end of the scrollbar, don't draw the shadow beneath
       * the scrollbar, instead draw the highlight of the button
       */
      if (widget &&
	  (x + width + stepper_size - 2 == widget->allocation.x + widget->allocation.width))
	gdk_draw_line (window, whitegc,
		       x + width - 1, y + 1, 
		       x + width - 1, y + height - 3);
      else
	gdk_draw_line (window, metal_style->mid_gray_gc,
		       x + width - 1, y + 1, 
		       x + width - 1, y + height - 3);
    }
  else
    {
      gdk_draw_line (window, whitegc, 
		     x + width - 1, y + 0, 
		     x + width - 1, y + height - 1);
      gdk_draw_line (window, metal_style->dark_gray_gc,
		     x + width - 2, y,
		     x + width - 2, y + height);
      gdk_draw_point (window, metal_style->dark_gray_gc, x, y + height - 1);

      /* At the lower end of the scrollbar, don't draw the shadow beneath
       * the scrollbar, instead draw the highlight of the button
       */
      if (widget &&
	  (y + height + stepper_size - 2 == widget->allocation.y + widget->allocation.height))
	gdk_draw_line (window, whitegc,
		       x + 1,         y + height - 1,
		       x + width - 3, y + height - 1);
      else
	gdk_draw_line (window, metal_style->mid_gray_gc,
		       x + 1,         y + height - 1,
		       x + width - 3, y + height - 1);
    }

  if (GTK_CHECK_TYPE (widget, gtk_hscrollbar_get_type ()))
    {
      w = width & 1 ? width - 11 : width - 10;
      h = height & 1 ? height - 7 : height - 8;
      gdk_draw_rectangle (window, fillgc, TRUE, x + 5, y + 3, w, h);
    }
  else
    {
      w = width & 1 ? width - 7 : width - 8;
      h = height & 1 ? height - 11 : height - 10;
      gdk_draw_rectangle (window, fillgc, TRUE, x + 3, y + 5, w, h);
    }
  gdk_gc_unref (fillgc);
  gdk_pixmap_unref (pm);

  /* Reset Clip Region */
  if (area)
    {
      gdk_gc_set_clip_rectangle (lightgc, NULL);
      gdk_gc_set_clip_rectangle (midgc, NULL);
      gdk_gc_set_clip_rectangle (darkgc, NULL);
      gdk_gc_set_clip_rectangle (whitegc, NULL);
    }
}
/**************************************************************************/
static void
metal_scale_trough (GtkStyle * style,
		    GdkWindow * window,
		    GtkStateType state_type,
		    GtkShadowType shadow_type,
		    GdkRectangle * area,
		    GtkWidget * widget,
		    const gchar * detail,
		    gint x,
		    gint y,
		    gint width,
		    gint height)
{
  MetalStyle *metal_style = METAL_STYLE (style);
  GdkGC *lightgc, *midgc, *darkgc, *whitegc;

  /* Get colors */
  lightgc = metal_style->light_gray_gc;
  midgc = style->bg_gc[GTK_STATE_SELECTED];
  darkgc = metal_style->mid_gray_gc;
  whitegc = style->white_gc;

  /* Set Clip Region */
  if (area)
    {
      gdk_gc_set_clip_rectangle (lightgc, area);
      gdk_gc_set_clip_rectangle (midgc, area);
      gdk_gc_set_clip_rectangle (darkgc, area);
      gdk_gc_set_clip_rectangle (whitegc, area);
    }

  if (GTK_CHECK_TYPE (widget, gtk_hscale_get_type ()))
    {
      /* Draw backgound */
      gdk_draw_rectangle (window, midgc, TRUE, x, y + 4, width - 2, 9);

      /* Draw border */
      gdk_draw_rectangle (window, darkgc, FALSE, x, y + 4, width - 2, 7);
      gdk_draw_rectangle (window, whitegc, FALSE, x + 1, y + 5, width - 2, 7);
    }
  else
    {
      /* Draw backgound */
      gdk_draw_rectangle (window, midgc, TRUE, x + 4, y, 9, height - 2);

      /* Draw border */
      gdk_draw_rectangle (window, darkgc, FALSE, x + 4, y, 7, height - 2);
      gdk_draw_rectangle (window, whitegc, FALSE, x + 5, y + 1, 7, height - 2);
    }

  /* Reset Clip Region */
  if (area)
    {
      gdk_gc_set_clip_rectangle (lightgc, NULL);
      gdk_gc_set_clip_rectangle (midgc, NULL);
      gdk_gc_set_clip_rectangle (darkgc, NULL);
      gdk_gc_set_clip_rectangle (whitegc, NULL);
    }
}
/**************************************************************************/
static void
metal_scale_slider (GtkStyle * style,
		    GdkWindow * window,
		    GtkStateType state_type,
		    GtkShadowType shadow_type,
		    GdkRectangle * area,
		    GtkWidget * widget,
		    const gchar * detail,
		    gint x,
		    gint y,
		    gint width,
		    gint height,
		    GtkOrientation orientation)
{
  MetalStyle *metal_style = METAL_STYLE (style);
  GdkPixmap *pm;
  GdkGC *fillgc;
  GdkGCValues values;
  GdkGC *lightgc, *midgc, *darkgc, *whitegc, *blackgc;
  int w, h;

  /* Get colors */
  lightgc = style->bg_gc[GTK_STATE_PRELIGHT];
  midgc = style->bg_gc[GTK_STATE_SELECTED];
  darkgc = style->fg_gc[GTK_STATE_PRELIGHT];
  whitegc = style->white_gc;
  blackgc = style->black_gc;

  /* Set Clip Region */
  if (area)
    {
      gdk_gc_set_clip_rectangle (lightgc, area);
      gdk_gc_set_clip_rectangle (midgc, area);
      gdk_gc_set_clip_rectangle (darkgc, area);
      gdk_gc_set_clip_rectangle (whitegc, area);
      gdk_gc_set_clip_rectangle (blackgc, area);
      gdk_gc_set_clip_rectangle (metal_style->light_gray_gc, area);
    }

#if 1
  /* Draw backgound */
  gdk_draw_rectangle (window, midgc, TRUE, x, y, width, height);

  /* Draw border */
  gdk_draw_rectangle (window, lightgc, FALSE, x + 1, y + 1, x + width - 2, y
		      + height - 2);
  gdk_draw_rectangle (window, darkgc, FALSE, x + 0, y + 0, x + width - 2, y
		      + height - 2);
  if (GTK_CHECK_TYPE (widget, gtk_hscale_get_type ()))
    {
      gdk_draw_line (window, whitegc, x + 0, y + height - 1, x + width - 1,
		     y + height - 1);
      gdk_draw_line (window, midgc, x + width - 1, y + 1, x + width - 1, y +
		     height - 2);
    }
  else
    {
      gdk_draw_line (window, whitegc, x + width - 1, y + 0, x + width - 1, y
		     + height - 1);
      gdk_draw_line (window, midgc, x + 0, y + height - 1, x + width - 2, y
		     + height - 1);
    }

  /* Draw textured surface */
  pm = gdk_pixmap_new (window, 4, 4, -1);

  gdk_draw_rectangle (pm, midgc, TRUE, 0, 0, 4, 4);
  gdk_draw_point (pm, darkgc, 0, 0);
  gdk_draw_point (pm, lightgc, 1, 1);
  gdk_draw_point (pm, darkgc, 2, 2);
  gdk_draw_point (pm, lightgc, 3, 3);

  values.fill = GDK_TILED;
  values.ts_x_origin = x + 5;
  values.ts_y_origin = y + 3;
  fillgc = gdk_gc_new_with_values (window, &values,
				   GDK_GC_FILL | GDK_GC_TS_X_ORIGIN | GDK_GC_TS_Y_ORIGIN);
  if (area)
    gdk_gc_set_clip_rectangle (fillgc, area);
  gdk_gc_set_tile (fillgc, pm);
  if (GTK_CHECK_TYPE (widget, gtk_hscale_get_type ()))
    {
      w = width & 1 ? width - 11 : width - 10;
      h = height & 1 ? height - 7 : height - 8;
      gdk_draw_rectangle (window, fillgc, TRUE, x + 5, y + 3, w, h);
    }
  else
    {
      w = width & 1 ? width - 7 : width - 8;
      h = height & 1 ? height - 11 : height - 10;
      gdk_draw_rectangle (window, fillgc, TRUE, x + 3, y + 5, w, h);
    }
  gdk_gc_unref (fillgc);
  gdk_pixmap_unref (pm);

  /* Draw middle line */
  if (GTK_CHECK_TYPE (widget, gtk_hscale_get_type ()))
    {
      if (state_type == GTK_STATE_PRELIGHT)
	{
	  gdk_draw_line (window, darkgc, x + width / 2, y + 2, x + width /
			 2, y + height - 4);
	  gdk_draw_line (window, whitegc, x + width / 2 + 1, y + 2, x +
			 width / 2 + 1, y + height - 4);
	}
      else
	{
	  gdk_draw_line (window, darkgc, x + width / 2, y + 2, x + width /
			 2, y + height - 4);
	  gdk_draw_line (window, lightgc, x + width / 2 + 1, y + 2, x +
			 width / 2 + 1, y + height - 4);
	}
    }
  else
    {
      if (state_type == GTK_STATE_PRELIGHT)
	{
	  gdk_draw_line (window, darkgc, x + 2, y + height / 2, x + width -
			 4, y + height / 2);
	  gdk_draw_line (window, whitegc, x + 2, y + height / 2 + 1, x +
			 width - 4, y + height / 2 + 1);
	}
      else
	{
	  gdk_draw_line (window, darkgc, x + 2, y + height / 2, x + width -
			 4, y + height / 2);
	  gdk_draw_line (window, lightgc, x + 2, y + height / 2 + 1, x +
			 width - 4, y + height / 2 + 1);
	}
    }

#else
  /* The following code draws the sliders more faithfully to the 
     JL&F spec, but I think it looks bad. */

  /* Requires GtkScale::slider-length of 15 */

  GdkPoint points[5];

  /* Draw backgound */
  gdk_draw_rectangle (window, lightgc, TRUE, x, y, width - 1, height - 1);

  /* Draw textured surface */
  pm = gdk_pixmap_new (window, 4, 4, -1);

  gdk_draw_rectangle (pm, midgc, TRUE, 0, 0, 4, 4);
  if (state_type == GTK_STATE_PRELIGHT)
    {
      gdk_draw_point (pm, darkgc, 0, 0);
      gdk_draw_point (pm, whitegc, 1, 1);
      gdk_draw_point (pm, darkgc, 2, 2);
      gdk_draw_point (pm, whitegc, 3, 3);
    }
  else
    {
      gdk_draw_point (pm, darkgc, 0, 0);
      gdk_draw_point (pm, lightgc, 1, 1);
      gdk_draw_point (pm, darkgc, 2, 2);
      gdk_draw_point (pm, lightgc, 3, 3);
    }

  values.fill = GDK_TILED;
  values.ts_x_origin = x + 5;
  values.ts_y_origin = y + 3;
  fillgc = gdk_gc_new_with_values (window, &values,
				   GDK_GC_FILL | GDK_GC_TS_X_ORIGIN | GDK_GC_TS_Y_ORIGIN);
  if (area)
    gdk_gc_set_clip_rectangle (fillgc, area);
  gdk_gc_set_tile (fillgc, pm);
  w = width - 4;
  h = height - 4;
  gdk_draw_rectangle (window, fillgc, TRUE, x + 2, y + 2, w, h);
  gdk_gc_unref (fillgc);
  gdk_pixmap_unref (pm);

  if (GTK_CHECK_TYPE (widget, gtk_hscale_get_type ()))
    {
      /* Draw border */
      points[0].x = x;
      points[0].y = y;
      points[1].x = x + 14;
      points[1].y = y;
      points[2].x = x + 14;
      points[2].y = y + 7;
      points[3].x = x + 7;
      points[3].y = y + 14;
      points[4].x = x;
      points[4].y = y + 7;
      gdk_draw_polygon (window, blackgc, FALSE, points, 5);
      points[0].x = x + 1;
      points[0].y = y + 1;
      points[1].x = x + 13;
      points[1].y = y + 1;
      points[2].x = x + 13;
      points[2].y = y + 7;
      points[3].x = x + 7;
      points[3].y = y + 13;
      points[4].x = x + 1;
      points[4].y = y + 7;
      gdk_draw_polygon (window, lightgc, FALSE, points, 5);

      /* Fix bottom corners */
      points[0].x = x;
      points[0].y = y + 14;
      points[1].x = x;
      points[1].y = y + 8;
      points[2].x = x + 6;
      points[2].y = y + 14;
      gdk_draw_polygon (window, metal_style->light_gray_gc, FALSE, points, 3);
      gdk_draw_polygon (window, metal_style->light_gray_gc, TRUE, points, 3);
      points[0].x = x + 14;
      points[0].y = y + 14;
      points[1].x = x + 14;
      points[1].y = y + 8;
      points[2].x = x + 8;
      points[2].y = y + 14;
      gdk_draw_polygon (window, metal_style->light_gray_gc, FALSE, points, 3);
      gdk_draw_polygon (window, metal_style->light_gray_gc, TRUE, points, 3);
      gdk_draw_rectangle (window, metal_style->light_gray_gc, TRUE, x, y +
			  15, width, height - 15);
    }
  else
    {
      /* Draw border */
      points[0].x = x;
      points[0].y = y + 7;
      points[1].x = x + 7;
      points[1].y = y;
      points[2].x = x + 14;
      points[2].y = y;
      points[3].x = x + 14;
      points[3].y = y + 14;
      points[4].x = x + 7;
      points[4].y = y + 14;
      gdk_draw_polygon (window, blackgc, FALSE, points, 5);

      points[0].x = x + 1;
      points[0].y = y + 7;
      points[1].x = x + 7;
      points[1].y = y + 1;
      points[2].x = x + 13;
      points[2].y = y + 1;
      points[3].x = x + 13;
      points[3].y = y + 13;
      points[4].x = x + 7;
      points[4].y = y + 13;
      gdk_draw_polygon (window, lightgc, FALSE, points, 5);

      /* Fix corners */
      points[0].x = x;
      points[0].y = y;
      points[1].x = x + 6;
      points[1].y = y;
      points[2].x = x;
      points[2].y = y + 6;
      gdk_draw_polygon (window, metal_style->light_gray_gc, FALSE, points, 3);
      gdk_draw_polygon (window, metal_style->light_gray_gc, TRUE, points, 3);
      points[0].x = x;
      points[0].y = y + 8;
      points[1].x = x;
      points[1].y = y + 14;
      points[2].x = x + 6;
      points[2].y = y + 14;
      gdk_draw_polygon (window, metal_style->light_gray_gc, FALSE, points, 3);
      gdk_draw_polygon (window, metal_style->light_gray_gc, TRUE, points, 3);
/*      gdk_draw_rectangle(window, metal_style->light_gray_gc, TRUE,  x, y+15, width, height-15); */
    }
#endif

  /* Reset Clip Region */
  if (area)
    {
      gdk_gc_set_clip_rectangle (lightgc, NULL);
      gdk_gc_set_clip_rectangle (midgc, NULL);
      gdk_gc_set_clip_rectangle (darkgc, NULL);
      gdk_gc_set_clip_rectangle (whitegc, NULL);
      gdk_gc_set_clip_rectangle (blackgc, NULL);
      gdk_gc_set_clip_rectangle (metal_style->light_gray_gc, NULL);
    }
}
/**************************************************************************/
static void
metal_menu (GtkStyle * style,
	    GdkWindow * window,
	    GtkStateType state_type,
	    GtkShadowType shadow_type,
	    GdkRectangle * area,
	    GtkWidget * widget,
	    const gchar * detail,
	    gint x,
	    gint y,
	    gint width,
	    gint height)
{
  GdkGC *midgc, *whitegc;

  midgc = style->bg_gc[GTK_STATE_SELECTED];
  whitegc = style->white_gc;

  /* Set Clip Region */
  if (area)
    {
      gdk_gc_set_clip_rectangle (midgc, area);
      gdk_gc_set_clip_rectangle (whitegc, area);
    }

  gdk_draw_rectangle (window, whitegc, FALSE, x + 1, y + 1, width - 2,
		      height - 2);
  gdk_draw_rectangle (window, midgc, FALSE, x, y, width - 1, height - 1);

  /* Reset Clip Region */
  if (area)
    {
      gdk_gc_set_clip_rectangle (midgc, NULL);
      gdk_gc_set_clip_rectangle (whitegc, NULL);
    }
}
/**************************************************************************/
static void
metal_menu_item (GtkStyle * style,
		 GdkWindow * window,
		 GtkStateType state_type,
		 GtkShadowType shadow_type,
		 GdkRectangle * area,
		 GtkWidget * widget,
		 const gchar * detail,
		 gint x,
		 gint y,
		 gint width,
		 gint height)
{
  /* Set Clip Region */
  if (area)
    {
      gdk_gc_set_clip_rectangle (style->bg_gc[GTK_STATE_SELECTED], area);
      gdk_gc_set_clip_rectangle (style->dark_gc[GTK_STATE_SELECTED], area);
      gdk_gc_set_clip_rectangle (style->light_gc[GTK_STATE_SELECTED], area);
    }

  gdk_draw_rectangle (window, style->bg_gc[GTK_STATE_SELECTED], TRUE, x, y,
		      width, height);
  gdk_draw_line (window, style->dark_gc[GTK_STATE_SELECTED], x, y, x +
		 width, y);
  gdk_draw_line (window, style->light_gc[GTK_STATE_SELECTED], x, y + height
		 - 1, x + width, y + height - 1);

  /* Reset Clip Region */
  if (area)
    {
      gdk_gc_set_clip_rectangle (style->bg_gc[GTK_STATE_SELECTED], NULL);
      gdk_gc_set_clip_rectangle (style->dark_gc[GTK_STATE_SELECTED], NULL);
      gdk_gc_set_clip_rectangle (style->light_gc[GTK_STATE_SELECTED], NULL);
    }
}
/**************************************************************************/
static void
metal_notebook (GtkStyle * style,
		GdkWindow * window,
		GtkStateType state_type,
		GtkShadowType shadow_type,
		GdkRectangle * area,
		GtkWidget * widget,
		const gchar * detail,
		gint x,
		gint y,
		gint width,
		gint height)
{
  MetalStyle *metal_style = METAL_STYLE (style);
  GdkGC *lightgc, *midgc, *darkgc, *whitegc;

  /* Get colors */
  if (state_type == GTK_STATE_PRELIGHT)
    {
      lightgc = style->bg_gc[GTK_STATE_PRELIGHT];
      midgc = style->bg_gc[GTK_STATE_SELECTED];
      darkgc = style->fg_gc[GTK_STATE_PRELIGHT];
      whitegc = style->white_gc;
    }
  else
    {
      lightgc = metal_style->light_gray_gc;
      midgc = metal_style->mid_gray_gc;
      darkgc = metal_style->mid_gray_gc;
      whitegc = style->white_gc;
    }

  /* Set Clip Region */
  if (area)
    {
      gdk_gc_set_clip_rectangle (lightgc, area);
      gdk_gc_set_clip_rectangle (midgc, area);
      gdk_gc_set_clip_rectangle (darkgc, area);
      gdk_gc_set_clip_rectangle (whitegc, area);
    }

  /* Draw backgound */
  gdk_draw_rectangle (window, lightgc, TRUE, x, y, width, height);

  /* Draw border */
  gdk_draw_rectangle (window, darkgc, FALSE, x, y, width - 2, height - 2);
  gdk_draw_rectangle (window, style->white_gc, FALSE, x + 1, y + 1, width -
		      2, height - 2);

  /* Reset Clip Region */
  if (area)
    {
      gdk_gc_set_clip_rectangle (lightgc, NULL);
      gdk_gc_set_clip_rectangle (midgc, NULL);
      gdk_gc_set_clip_rectangle (darkgc, NULL);
      gdk_gc_set_clip_rectangle (whitegc, NULL);
    }
}
/**************************************************************************/
static void
adjust_notebook_tab_size (GtkPositionType tab_pos,
			  gint           *width,
			  gint           *height)
{
  /* The default overlap is two pixels, but we only want a one pixel overlap
   */
  switch (tab_pos)
    {
    case GTK_POS_TOP:
    case GTK_POS_BOTTOM:
      *width -= 1;
      break;
    case GTK_POS_LEFT:
    case GTK_POS_RIGHT:
      *height -= 1;
      break;
    }
}

static void
metal_tab (GtkStyle * style,
	   GdkWindow * window,
	   GtkStateType state_type,
	   GtkShadowType shadow_type,
	   GdkRectangle * area,
	   GtkWidget * widget,
	   const gchar * detail,
	   gint x,
	   gint y,
	   gint width,
	   gint height)
{
  MetalStyle *metal_style = METAL_STYLE (style);
  GtkNotebook *notebook;
  GdkGC *lightgc, *midgc, *darkgc, *brightgc, *bggc;
  GdkPoint points[5];
  int orientation;
  gboolean is_first, selected;

  notebook = GTK_NOTEBOOK (widget);
  orientation = notebook->tab_pos;

  is_first = is_first_tab (notebook, x, y);
  selected = state_type == GTK_STATE_NORMAL;

  lightgc = metal_style->light_gray_gc;
  midgc = metal_style->mid_gray_gc;
  brightgc = style->white_gc;
  bggc = metal_style->light_gray_gc;

  if (selected)
    {
      brightgc = style->white_gc;
      darkgc = metal_style->mid_gray_gc;
    }
  else
    {
      brightgc = metal_style->light_gray_gc;
      darkgc = metal_style->dark_gray_gc;
    }

  /* Set Clip Region */
  if (area)
    {
      gdk_gc_set_clip_rectangle (lightgc, area);
      gdk_gc_set_clip_rectangle (midgc, area);
      gdk_gc_set_clip_rectangle (darkgc, area);
      gdk_gc_set_clip_rectangle (brightgc, area);
      gdk_gc_set_clip_rectangle (bggc, area);
    }

  adjust_notebook_tab_size (orientation, &width, &height);

  /* Fill area */
  gdk_draw_rectangle (window, bggc, TRUE, x + 0, y + 0, width, height);

  switch (orientation)
    {
    case GTK_POS_TOP:
      /* Draw background */
      points[0].x = x + 2;
      points[0].y = y + height;
      points[1].x = x + 2;
      points[1].y = y + 6;
      points[2].x = x + 6;
      points[2].y = y + 2;
      points[3].x = x + width - 1;
      points[3].y = y + 2;
      points[4].x = x + width - 1;
      points[4].y = y + height;
      if (selected)
	gdk_draw_polygon (window, lightgc, TRUE, points, 5);
      else
	gdk_draw_polygon (window, midgc, TRUE, points, 5);

      /* Draw border */
      if (is_first)
	gdk_draw_line (window, darkgc, x + 0, y + 6, x + 0, y + height + 1);
      else if (selected)
	gdk_draw_line (window, darkgc, x + 0, y + 6, x + 0, y + height - 1);
      gdk_draw_line (window, darkgc, x + 0, y + 6, x + 6, y + 0);
      gdk_draw_line (window, darkgc, x + 6, y + 0, x + width - 2, y + 0);
      gdk_draw_line (window, darkgc, x + width - 1, y + 1, x + width - 1, y
		     + height - 1);

      if (is_first)
	gdk_draw_line (window, brightgc, x + 1, y + 6, x + 1, y + height + 1);
      else
	gdk_draw_line (window, brightgc, x + 1, y + 6, x + 1, y + height - 1);
      gdk_draw_line (window, brightgc, x + 1, y + 6, x + 6, y + 1);
      gdk_draw_line (window, brightgc, x + 6, y + 1, x + width - 2, y + 1);
      break;
    case GTK_POS_LEFT:
      /* Draw background */
      points[0].x = x + 2;
      points[0].y = y + height;
      points[1].x = x + 2;
      points[1].y = y + 6;
      points[2].x = x + 6;
      points[2].y = y + 2;
      points[3].x = x + width - 1;
      points[3].y = y + 2;
      points[4].x = x + width - 1;
      points[4].y = y + height;
      if (selected)
	gdk_draw_polygon (window, lightgc, TRUE, points, 5);
      else
	gdk_draw_polygon (window, midgc, TRUE, points, 5);

      /* Draw border */
      gdk_draw_line (window, darkgc, x + 0, y + 6, x + 0, y + height - 1);
      gdk_draw_line (window, darkgc, x + 0, y + 6, x + 6, y + 0);
      if (is_first)
	gdk_draw_line (window, darkgc, x + 6, y + 0, x + width + 1, y + 0);
      else
	gdk_draw_line (window, darkgc, x + 6, y + 0, x + width - 1, y + 0);
      gdk_draw_line (window, darkgc, x + 0, y + height - 1, x + width - 1, y
		     + height - 1);

      gdk_draw_line (window, brightgc, x + 1, y + 6, x + 6, y + 1);
      if (is_first)
	gdk_draw_line (window, brightgc, x + 6, y + 1, x + width + 1, y + 1);
      else
	gdk_draw_line (window, brightgc, x + 6, y + 1, x + width - 1, y + 1);
      break;
    case GTK_POS_RIGHT:
      /* Draw background */
      points[0].x = x + width - 2;
      points[0].y = y + height - 1;
      points[1].x = x + width - 2;
      points[1].y = y + 6;
      points[2].x = x + width - 6;
      points[2].y = y + 2;
      points[3].x = x - 1;
      points[3].y = y + 2;
      points[4].x = x - 1;
      points[4].y = y + height - 1;
      if (selected)
	gdk_draw_polygon (window, lightgc, TRUE, points, 5);
      else
	gdk_draw_polygon (window, midgc, TRUE, points, 5);

      /* Draw border */
      gdk_draw_line (window, darkgc, x + width - 1, y + 6, x + width - 1, y
		     + height - 1);
      gdk_draw_line (window, darkgc, x + width - 1, y + 6, x + width - 7, y
		     + 0);
      if (is_first)
	gdk_draw_line (window, darkgc, x - 2, y + 0, x + width - 7, y + 0);
      else
	gdk_draw_line (window, darkgc, x - 1, y + 0, x + width - 7, y + 0);
      gdk_draw_line (window, darkgc, x - 1, y + height - 1, x + width - 1, y
		     + height - 1);

      gdk_draw_line (window, brightgc, x + width - 2, y + 6, x + width - 7, y
		     + 1);
      if (is_first)
	gdk_draw_line (window, brightgc, x + width - 7, y + 1, x - 2, y + 1);
      else
	gdk_draw_line (window, brightgc, x + width - 7, y + 1, x - 1, y + 1);
      break;
    case GTK_POS_BOTTOM:
      /* Draw background */
      points[0].x = x + 2;
      points[0].y = y + 0;
      points[1].x = x + 2;
      points[1].y = y + height - 6;
      points[2].x = x + 6;
      points[2].y = y + height - 2;
      points[3].x = x + width - 1;
      points[3].y = y + height - 2;
      points[4].x = x + width - 1;
      points[4].y = y + 0;
      if (selected)
	gdk_draw_polygon (window, lightgc, TRUE, points, 5);
      else
	gdk_draw_polygon (window, midgc, TRUE, points, 5);

      /* Draw border */
      if (is_first)
	gdk_draw_line (window, darkgc, x + 0, y + height - 6, x + 0, y - 2);
      else if (selected)
	gdk_draw_line (window, darkgc, x + 0, y + height - 6, x + 0, y - 1);
      gdk_draw_line (window, darkgc, x + 0, y + height - 6, x + 6, y + height);
      gdk_draw_line (window, darkgc, x + 5, y + height - 1, x + width - 2, y
		     + height - 1);
      gdk_draw_line (window, darkgc, x + width - 1, y + height - 1, x +
		     width - 1, y - 1);

      if (is_first)
	gdk_draw_line (window, brightgc, x + 1, y + height - 6, x + 1, y - 2);
      else
	gdk_draw_line (window, brightgc, x + 1, y + height - 6, x + 1, y - 1);
      gdk_draw_line (window, brightgc, x + 1, y + height - 6, x + 5, y +
		     height - 2);
      break;
    }

  /* Reset Clip Region */
  if (area)
    {
      gdk_gc_set_clip_rectangle (lightgc, NULL);
      gdk_gc_set_clip_rectangle (midgc, NULL);
      gdk_gc_set_clip_rectangle (darkgc, NULL);
      gdk_gc_set_clip_rectangle (brightgc, NULL);
      gdk_gc_set_clip_rectangle (bggc, NULL);
    }
}
/**************************************************************************/
static gboolean
is_first_tab (GtkNotebook *notebook,
	      int          x,
	      int          y)
{
  GtkWidget *widget = GTK_WIDGET (notebook);
  int border_width = GTK_CONTAINER (notebook)->border_width;

  switch (notebook->tab_pos)
    {
    case GTK_POS_TOP:
    case GTK_POS_BOTTOM:
      return x == widget->allocation.x + border_width;
    case GTK_POS_LEFT:
    case GTK_POS_RIGHT:
      return y == widget->allocation.y + border_width;
    }

  return FALSE;
}
/**************************************************************************/
static void
metal_button (GtkStyle * style,
	      GdkWindow * window,
	      GtkStateType state_type,
	      GtkShadowType shadow_type,
	      GdkRectangle * area,
	      GtkWidget * widget,
	      const gchar * detail,
	      gint x,
	      gint y,
	      gint width,
	      gint height)
{
  MetalStyle *metal_style = METAL_STYLE (style);
  GdkGC *dark_gc, *medium_gc, *light_gc;
  gboolean active_toggle = FALSE;

  /* Set Clip Region */
  if (area)
    {
      gdk_gc_set_clip_rectangle (style->bg_gc[GTK_STATE_PRELIGHT], area);
      gdk_gc_set_clip_rectangle (style->bg_gc[GTK_STATE_SELECTED], area);
      gdk_gc_set_clip_rectangle (style->bg_gc[GTK_STATE_NORMAL], area);
      gdk_gc_set_clip_rectangle (style->bg_gc[state_type], area);
      gdk_gc_set_clip_rectangle (style->light_gc[state_type], area);
      gdk_gc_set_clip_rectangle (style->dark_gc[state_type], area);
    }

  if (widget && GTK_IS_TOGGLE_BUTTON (widget) &&
      GTK_TOGGLE_BUTTON (widget)->active)
    {
      active_toggle = TRUE;
      gdk_draw_rectangle (window, metal_style->mid_gray_gc, TRUE, x, y,
			  width, height);
    }
  else if (state_type == GTK_STATE_ACTIVE)
    {
      gdk_draw_rectangle (window, metal_style->mid_gray_gc, TRUE, x, y,
			  width, height);
    }
  else
    {
      gdk_draw_rectangle (window, metal_style->light_gray_gc, TRUE, x, y,
			  width, height);
    }

  dark_gc = (state_type == GTK_STATE_INSENSITIVE) ? metal_style->mid_gray_gc : metal_style->dark_gray_gc;
  medium_gc = metal_style->light_gray_gc;
  light_gc = style->white_gc;

  gdk_draw_rectangle (window, dark_gc, FALSE, x, y, width
		      - 2, height - 2);

  if (state_type == GTK_STATE_INSENSITIVE)
    {
      /* Nothing */
    }
  else if (widget && GTK_WIDGET_HAS_DEFAULT (widget))
    {
      if (state_type == GTK_STATE_ACTIVE || active_toggle)
	{
	  if (active_toggle)
	    {
	      gdk_draw_line (window, medium_gc,
			     x + 2,     y + 2,
			     x + 2,     y + height);
	      gdk_draw_line (window, medium_gc,
			     x + 2,     y + 2,
			     x + width, y + 2);
	    }

	  gdk_draw_line (window, light_gc,
			 x + 2,     y + height,
			 x + width, y + height);
	  gdk_draw_line (window, light_gc,
			 x + width, y + 2,
			 x + width, y + height);
	}
      else
	{
	  gdk_draw_rectangle (window, light_gc, FALSE, x + 2, y + 2,
			      width - 2, height - 2);
	}
      
      gdk_draw_rectangle (window, dark_gc, FALSE, x + 1, y + 1, width
			  - 2, height - 2);
      
      gdk_draw_point (window, medium_gc, x + 2, y + height - 2);
      gdk_draw_point (window, medium_gc, x + width - 2, y + 2);
      gdk_draw_point (window, dark_gc, x, y + height - 1);
      gdk_draw_point (window, dark_gc, x + width - 1, y);
    }
  else
    {
      if (state_type == GTK_STATE_ACTIVE || active_toggle)
	{
	  if (active_toggle)
	    {
	      gdk_draw_line (window, medium_gc,
			     x + 1, y + 1,
			     x + 1, y + height - 1);
	      gdk_draw_line (window, medium_gc,
			     x + 1,         y + 1,
			     x + width - 1, y + 1);
	    }

	  gdk_draw_line (window, light_gc,
			 x + 1,     y + height - 1,
			 x + width, y + height - 1);
	  gdk_draw_line (window, light_gc,
			 x + width - 1, y + 1,
			 x + width - 1, y + height);
	}
      else
	gdk_draw_rectangle (window, light_gc, FALSE, x + 1, y + 1,
			    width - 2, height - 2);

      gdk_draw_point (window, medium_gc, x + 1, y + height - 2);
      gdk_draw_point (window, medium_gc, x + width - 2, y + 1);
    }

  /* Reset Clip Region */
  if (area)
    {
      gdk_gc_set_clip_rectangle (style->bg_gc[GTK_STATE_PRELIGHT], NULL);
      gdk_gc_set_clip_rectangle (style->bg_gc[GTK_STATE_SELECTED], NULL);
      gdk_gc_set_clip_rectangle (style->bg_gc[GTK_STATE_NORMAL], NULL);
      gdk_gc_set_clip_rectangle (style->bg_gc[state_type], NULL); 
      gdk_gc_set_clip_rectangle (style->light_gc[state_type], NULL);
      gdk_gc_set_clip_rectangle (style->dark_gc[state_type], NULL);
    }
}
/**************************************************************************/
static void
draw_check (GtkStyle * style,
	    GdkWindow * window,
	    GtkStateType state_type,
	    GtkShadowType shadow_type,
	    GdkRectangle * area,
	    GtkWidget * widget,
	    const gchar * detail,
	    gint x,
	    gint y,
	    gint width,
	    gint height)
{
  GdkGC *gc1, *gc2, *gc3, *gc4;

  /* Fixed size only */

#if DEBUG
  printf ("draw_check: %p %p %s %i %i %i %i\n", widget, window, detail, x,
	  y, width, height);
#endif

  gc1 = style->black_gc;
  gc2 = style->bg_gc[GTK_STATE_NORMAL];
  gc3 = style->dark_gc[state_type];
  gc4 = style->light_gc[state_type];

  if (area)
    {
      gdk_gc_set_clip_rectangle (gc1, area);
      gdk_gc_set_clip_rectangle (gc2, area);
      gdk_gc_set_clip_rectangle (gc3, area);
      gdk_gc_set_clip_rectangle (gc4, area);
    }

  /* Draw box */
  if (GTK_CHECK_TYPE (widget, gtk_menu_item_get_type ()))
    {
      gdk_draw_rectangle (window, gc3, FALSE, x - 2, y - 2, 8, 8);
      gdk_draw_rectangle (window, gc4, FALSE, x - 1, y - 1, 8, 8);

      if (shadow_type == GTK_SHADOW_IN)
	{
	  gdk_draw_line (window, gc1, x + 1, y + 0, x + 1, y + 4);
	  gdk_draw_line (window, gc1, x + 2, y + 0, x + 2, y + 4);
	  gdk_draw_line (window, gc1, x + 3, y + 3, x + 7, y - 1);
	  gdk_draw_line (window, gc1, x + 3, y + 2, x + 7, y - 2);
	}
    }
  else
    {
      gdk_draw_rectangle (window, gc2, TRUE, x, y, width, height);

      gdk_draw_rectangle (window, gc3, FALSE, x - 2, y - 2, 11, 11);
      gdk_draw_rectangle (window, gc4, FALSE, x - 1, y - 1, 11, 11);

      if (shadow_type == GTK_SHADOW_IN)
	{
	  gdk_draw_line (window, gc1, x + 1, y + 3, x + 1, y + 7);
	  gdk_draw_line (window, gc1, x + 2, y + 3, x + 2, y + 7);
	  gdk_draw_line (window, gc1, x + 3, y + 6, x + 7, y + 2);
	  gdk_draw_line (window, gc1, x + 3, y + 5, x + 7, y + 1);
	}
    }

  if (area)
    {
      gdk_gc_set_clip_rectangle (gc1, NULL);
      gdk_gc_set_clip_rectangle (gc2, NULL);
      gdk_gc_set_clip_rectangle (gc3, NULL);
      gdk_gc_set_clip_rectangle (gc4, NULL);
    }
}
/**************************************************************************/
static void
draw_option (GtkStyle * style,
	     GdkWindow * window,
	     GtkStateType state_type,
	     GtkShadowType shadow_type,
	     GdkRectangle * area,
	     GtkWidget * widget,
	     const gchar * detail,
	     gint x,
	     gint y,
	     gint width,
	     gint height)
{
  GdkGC *gc0;
  GdkGC *gc1;
  GdkGC *gc2;
  GdkGC *gc3;
  GdkGC *gc4;

  x -= 1;
  y -= 1;
  width += 2;
  height += 2;

  gc0 = style->white_gc;
  gc1 = style->light_gc[GTK_STATE_NORMAL];
  gc2 = style->bg_gc[GTK_STATE_NORMAL];
  gc3 = style->dark_gc[GTK_STATE_NORMAL];
  gc4 = style->black_gc;

  if (area)
    {
      gdk_gc_set_clip_rectangle (gc0, area);
      gdk_gc_set_clip_rectangle (gc1, area);
      gdk_gc_set_clip_rectangle (gc2, area);
      gdk_gc_set_clip_rectangle (gc3, area);
      gdk_gc_set_clip_rectangle (gc4, area);
    }

  /* Draw radio button, metal-stle
     There is probably a better way to do this
     with pixmaps. Fix later. */

  if (GTK_CHECK_TYPE (widget, gtk_menu_item_get_type ()))
    {
      /* dark */
      gdk_draw_line (window, gc3, x + 2, y, x + 6, y);
      gdk_draw_line (window, gc3, x + 1, y + 1, x + 1, y + 1);
      gdk_draw_line (window, gc3, x + 7, y + 1, x + 7, y + 1);
      gdk_draw_line (window, gc3, x + 2, y + 8, x + 2, y + 8);
      gdk_draw_line (window, gc3, x + 7, y + 7, x + 7, y + 7);
      gdk_draw_line (window, gc3, x + 2, y + 8, x + 6, y + 8);
      gdk_draw_line (window, gc3, x, y + 2, x, y + 6);
      gdk_draw_line (window, gc3, x + 8, y + 2, x + 8, y + 6);

      /* white */
      gdk_draw_line (window, gc0, x + 3, y + 1, x + 6, y + 1);
      gdk_draw_line (window, gc0, x + 8, y + 1, x + 8, y + 1);
      gdk_draw_line (window, gc0, x + 2, y + 2, x + 2, y + 2);
      gdk_draw_line (window, gc0, x + 1, y + 3, x + 1, y + 6);
      gdk_draw_line (window, gc0, x + 9, y + 2, x + 9, y + 7);
      gdk_draw_line (window, gc0, x + 1, y + 8, x + 1, y + 8);
      gdk_draw_line (window, gc0, x + 8, y + 8, x + 8, y + 8);
      gdk_draw_line (window, gc0, x + 2, y + 9, x + 7, y + 9);

      if (shadow_type == GTK_SHADOW_IN)
	{
	  gdk_draw_rectangle (window, gc4, TRUE, x + 2, y + 3, 5, 3);
	  gdk_draw_rectangle (window, gc4, TRUE, x + 3, y + 2, 3, 5);
	}
    }
  else
    {
      /* background */
      gdk_draw_rectangle (window, gc2, TRUE, x, y, width, height);

      /* dark */
      gdk_draw_line (window, gc3, x + 4, y, x + 7, y);
      gdk_draw_line (window, gc3, x + 2, y + 1, x + 3, y + 1);
      gdk_draw_line (window, gc3, x + 8, y + 1, x + 9, y + 1);
      gdk_draw_line (window, gc3, x + 2, y + 10, x + 3, y + 10);
      gdk_draw_line (window, gc3, x + 8, y + 10, x + 9, y + 10);
      gdk_draw_line (window, gc3, x + 4, y + 11, x + 7, y + 11);


      gdk_draw_line (window, gc3, x, y + 4, x, y + 7);
      gdk_draw_line (window, gc3, x + 1, y + 2, x + 1, y + 3);
      gdk_draw_line (window, gc3, x + 1, y + 8, x + 1, y + 9);
      gdk_draw_line (window, gc3, x + 10, y + 2, x + 10, y + 3);
      gdk_draw_line (window, gc3, x + 10, y + 8, x + 10, y + 9);
      gdk_draw_line (window, gc3, x + 11, y + 4, x + 11, y + 7);

      /* white */
      gdk_draw_line (window, gc0, x + 4, y + 1, x + 7, y + 1);
      gdk_draw_line (window, gc0, x + 2, y + 2, x + 3, y + 2);
      gdk_draw_line (window, gc0, x + 8, y + 2, x + 9, y + 2);
      gdk_draw_line (window, gc0, x + 2, y + 11, x + 3, y + 11);
      gdk_draw_line (window, gc0, x + 8, y + 11, x + 9, y + 11);
      gdk_draw_line (window, gc0, x + 4, y + 12, x + 7, y + 12);


      gdk_draw_line (window, gc0, x + 1, y + 4, x + 1, y + 7);
      gdk_draw_line (window, gc0, x + 2, y + 2, x + 2, y + 3);
      gdk_draw_line (window, gc0, x + 2, y + 8, x + 2, y + 9);
      gdk_draw_line (window, gc0, x + 11, y + 2, x + 11, y + 3);
      gdk_draw_line (window, gc0, x + 11, y + 8, x + 11, y + 9);
      gdk_draw_line (window, gc0, x + 12, y + 4, x + 12, y + 7);
      gdk_draw_point (window, gc0, x + 10, y + 1);
      gdk_draw_point (window, gc0, x + 10, y + 10);

      if (shadow_type == GTK_SHADOW_IN)
	{
	  gdk_draw_rectangle (window, gc4, TRUE, x + 3, y + 4, 6, 4);
	  gdk_draw_rectangle (window, gc4, TRUE, x + 4, y + 3, 4, 6);
	}
    }

  if (area)
    {
      gdk_gc_set_clip_rectangle (gc0, NULL);
      gdk_gc_set_clip_rectangle (gc1, NULL);
      gdk_gc_set_clip_rectangle (gc2, NULL);
      gdk_gc_set_clip_rectangle (gc3, NULL);
      gdk_gc_set_clip_rectangle (gc4, NULL);
    }
}
/**************************************************************************/
static void
draw_tab (GtkStyle * style,
	  GdkWindow * window,
	  GtkStateType state_type,
	  GtkShadowType shadow_type,
	  GdkRectangle * area,
	  GtkWidget * widget,
	  const gchar * detail,
	  gint x,
	  gint y,
	  gint width,
	  gint height)
{
  g_return_if_fail (style != NULL);
  g_return_if_fail (window != NULL);

#if DEBUG
  printf ("draw_tab: %p %s %i %i\n", detail, detail, width, height);
#endif

  gtk_paint_box (style, window, state_type, shadow_type, area, widget, detail,
		 x, y, width, height);
}

/**************************************************************************/
static void
draw_shadow_gap (GtkStyle * style,
		 GdkWindow * window,
		 GtkStateType state_type,
		 GtkShadowType shadow_type,
		 GdkRectangle * area,
		 GtkWidget * widget,
		 const gchar * detail,
		 gint x,
		 gint y,
		 gint width,
		 gint height,
		 GtkPositionType gap_side,
		 gint gap_x,
		 gint gap_width)
{
  GdkRectangle rect;

  g_return_if_fail (style != NULL);
  g_return_if_fail (window != NULL);

#if DEBUG
  printf ("draw_shadow_gap: %p %p %s %i %i %i %i\n", widget, window, detail,
	  x, y, width, height);
#endif

  gap_width -= 1;

  gtk_paint_shadow (style, window, state_type, shadow_type, area, widget, detail,
		    x, y, width, height);

  switch (gap_side)
    {
    case GTK_POS_TOP:
      rect.x = x + gap_x;
      rect.y = y;
      rect.width = gap_width;
      rect.height = 2;
      break;
    case GTK_POS_BOTTOM:
      rect.x = x + gap_x;
      rect.y = y + height - 2;
      rect.width = gap_width;
      rect.height = 2;
      break;
    case GTK_POS_LEFT:
      rect.x = x;
      rect.y = y + gap_x;
      rect.width = 2;
      rect.height = gap_width;
      break;
    case GTK_POS_RIGHT:
      rect.x = x + width - 2;
      rect.y = y + gap_x;
      rect.width = 2;
      rect.height = gap_width;
      break;
    }

  gtk_style_apply_default_pixmap (style, window, state_type, area,
				  rect.x, rect.y, rect.width, rect.height);
}
/**************************************************************************/
static void
draw_box_gap (GtkStyle       *style,
	      GdkWindow      *window,
	      GtkStateType    state_type,
	      GtkShadowType   shadow_type,
	      GdkRectangle   *area,
	      GtkWidget      *widget,
	      const gchar    *detail,
	      gint            x,
	      gint            y,
	      gint            width,
	      gint            height,
	      GtkPositionType gap_side,
	      gint            gap_x,
	      gint            gap_width)
{
  GdkRectangle rect;

  g_return_if_fail (style != NULL);
  g_return_if_fail (window != NULL);

#if DEBUG
  printf ("draw_box_gap: %p %p %s %i %i %i %i\n", widget, window, detail, x,
	  y, width, height);
#endif

  gtk_paint_box (style, window, state_type, shadow_type, area, widget, detail,
		 x, y, width, height);

  /* The default overlap is two pixels, but we only want a one pixel overlap
   */
  gap_width -= 1;

  switch (gap_side)
    {
    case GTK_POS_TOP:
      rect.x = x + gap_x;
      rect.y = y;
      rect.width = gap_width;
      rect.height = 2;
      break;
    case GTK_POS_BOTTOM:
      rect.x = x + gap_x;
      rect.y = y + height - 2;
      rect.width = gap_width;
      rect.height = 2;
      break;
    case GTK_POS_LEFT:
      rect.x = x;
      rect.y = y + gap_x;
      rect.width = 2;
      rect.height = gap_width;
      break;
    case GTK_POS_RIGHT:
      rect.x = x + width - 2;
      rect.y = y + gap_x;
      rect.width = 2;
      rect.height = gap_width;
      break;
    }

  gtk_style_apply_default_pixmap (style, window, state_type, area,
				  rect.x, rect.y, rect.width, rect.height);
}
/**************************************************************************/
static void
draw_extension (GtkStyle * style,
		GdkWindow * window,
		GtkStateType state_type,
		GtkShadowType shadow_type,
		GdkRectangle * area,
		GtkWidget * widget,
		const gchar * detail,
		gint x,
		gint y,
		gint width,
		gint height,
		GtkPositionType gap_side)
{
  g_return_if_fail (style != NULL);
  g_return_if_fail (window != NULL);

#if DEBUG
  printf ("draw_extension: %p %p %s %i %i %i %i\n", widget, window, detail,
	  x, y, width, height);
#endif

  gtk_paint_box (style, window, state_type, shadow_type, area, widget, detail,
		 x, y, width, height);
}
/**************************************************************************/
static void
draw_notebook_focus (GtkWidget *widget,
		     GdkWindow *window,
		     GdkGC     *gc,
		     gint       x,
		     gint       y,
		     gint       width,
		     gint       height)
{
  GtkPositionType tab_position = GTK_POS_TOP;
  GdkPoint points[6];
  gint tab_hborder = 2;
  gint tab_vborder = 2;

  if (widget && GTK_IS_NOTEBOOK (widget))
    {
      GtkNotebook *notebook = GTK_NOTEBOOK (widget);
      
      tab_hborder = notebook->tab_hborder;
      tab_vborder = notebook->tab_vborder;
      tab_position = gtk_notebook_get_tab_pos (notebook);
    }

  adjust_notebook_tab_size (tab_position, &width, &height);

  x -= tab_hborder;
  y -= tab_vborder;
  width += 2 * tab_hborder;
  height += 2 * tab_vborder;

  switch (tab_position)
    {
    default:
    case GTK_POS_TOP:
      points[0].x = x + 4;          points[0].y = y;
      points[1].x = x + width - 1;  points[1].y = y;
      points[2].x = x + width - 1;  points[2].y = y + height;
      points[3].x = x;              points[3].y = y + height;
      points[4].x = x;              points[4].y = y + 4;
      break;
    case GTK_POS_LEFT:
      points[0].x = x + 4;          points[0].y = y - 1;
      points[1].x = x + width - 1;  points[1].y = y - 1;
      points[2].x = x + width - 1;  points[2].y = y + height;
      points[3].x = x;              points[3].y = y + height;
      points[4].x = x;              points[4].y = y + 3;
      break;
    case GTK_POS_RIGHT:
      points[0].x = x;              points[0].y = y - 1;
      points[1].x = x + width - 5;  points[1].y = y - 1;
      points[2].x = x + width - 1;  points[2].y = y + 3;
      points[3].x = x + width - 1;  points[3].y = y + height;
      points[4].x = x;              points[4].y = y + height;
      break;
    case GTK_POS_BOTTOM:
      points[0].x = x;              points[0].y = y;
      points[1].x = x + width - 1;  points[1].y = y;
      points[2].x = x + width - 1;  points[2].y = y + height - 1;
      points[3].x = x + 4;          points[3].y = y + height - 1;
      points[4].x = x;              points[4].y = y + height - 5;
      break;
    }

  points[5] = points[0];
  
  gdk_draw_polygon (window, gc, FALSE, points, 6);
}

static void
draw_focus (GtkStyle * style,
	    GdkWindow * window,
	    GtkStateType state_type,
	    GdkRectangle * area,
	    GtkWidget * widget,
	    const gchar * detail,
	    gint x,
	    gint y,
	    gint width,
	    gint height)
{
  GdkGC *focusgc;
  
#if DEBUG
  printf ("draw_focus: %p %p %s %i %i %i %i\n", widget, window, detail, x,
	  y, width, height);
#endif

  if (detail && strcmp (detail, "add-mode") == 0)
    {
      parent_class->draw_focus (style, window, state_type, area, widget, detail, x, y, width, height);
      return;
    }

  if (width == -1 && height == -1)
    gdk_window_get_size (window, &width, &height);
  else if (width == -1)
    gdk_window_get_size (window, &width, NULL);
  else if (height == -1)
    gdk_window_get_size (window, NULL, &height);

  focusgc = style->bg_gc[GTK_STATE_SELECTED];

  if (area)
    gdk_gc_set_clip_rectangle (focusgc, area);

  if (DETAIL ("tab"))
    draw_notebook_focus (widget, window, focusgc, x, y, width, height);
  else
    gdk_draw_rectangle (window, focusgc, FALSE, x, y, width - 1, height - 1);

  if (area)
    gdk_gc_set_clip_rectangle (focusgc, NULL);
}
/**************************************************************************/
static void
draw_slider (GtkStyle * style,
	     GdkWindow * window,
	     GtkStateType state_type,
	     GtkShadowType shadow_type,
	     GdkRectangle * area,
	     GtkWidget * widget,
	     const gchar * detail,
	     gint x,
	     gint y,
	     gint width,
	     gint height,
	     GtkOrientation orientation)
{
  g_return_if_fail (style != NULL);
  g_return_if_fail (window != NULL);

  if ((width == -1) && (height == -1))
    gdk_window_get_size (window, &width, &height);
  else if (width == -1)
    gdk_window_get_size (window, &width, NULL);
  else if (height == -1)
    gdk_window_get_size (window, NULL, &height);

  if (DETAIL ("slider"))
    metal_scrollbar_slider (style, window, state_type, shadow_type,
			    area, widget, detail, x, y, width, height);
  else
    metal_scale_slider (style, window, state_type, shadow_type,
			area, widget, detail, x, y, width, height, orientation);
}
/**************************************************************************/
static void
draw_paned_handle (GtkStyle      *style,
		   GdkWindow     *window,
		   GtkStateType   state_type,
		   GtkShadowType  shadow_type,
		   GdkRectangle  *area,
		   GtkWidget     *widget,
		   gint           x,
		   gint           y,
		   gint           width,
		   gint           height,
		   GtkOrientation orientation)
{
  MetalStyle *metal_style = METAL_STYLE (style);
  GdkPixmap *pm;
  GdkGC *fillgc;
  GdkGCValues values;
  GdkGC *lightgc, *darkgc, *whitegc;

  /* Get colors */
  if (state_type == GTK_STATE_PRELIGHT)
    {
      lightgc = style->bg_gc[GTK_STATE_PRELIGHT];
      darkgc = style->fg_gc[GTK_STATE_PRELIGHT];
      whitegc = style->white_gc;
    }
  else
    {
      lightgc = metal_style->light_gray_gc;
      darkgc = metal_style->dark_gray_gc;
      whitegc = style->white_gc;
    }

  /* Draw textured surface */
  pm = gdk_pixmap_new (window, 4, 4, -1);

  gdk_draw_rectangle (pm, lightgc, TRUE, 0, 0, 4, 4);
  
  gdk_draw_point (pm, whitegc, 0, 0);
  gdk_draw_point (pm, darkgc, 1, 1);
  gdk_draw_point (pm, whitegc, 2, 2);
  gdk_draw_point (pm, darkgc, 3, 3);

  values.fill = GDK_TILED;
  values.ts_x_origin = x + 2;
  values.ts_y_origin = y + 2;
  fillgc = gdk_gc_new_with_values (window, &values,
				   GDK_GC_FILL | GDK_GC_TS_X_ORIGIN | GDK_GC_TS_Y_ORIGIN);

  if (area)
    gdk_gc_set_clip_rectangle (fillgc, area);
  gdk_gc_set_tile (fillgc, pm);

  gdk_draw_rectangle (window, fillgc, TRUE, x + 2, y + 2, width - 4, height - 4);

  gdk_gc_unref (fillgc);
  gdk_pixmap_unref (pm);
}
/**************************************************************************/
static void
draw_handle (GtkStyle      *style,
	     GdkWindow     *window,
	     GtkStateType   state_type,
	     GtkShadowType  shadow_type,
	     GdkRectangle  *area,
	     GtkWidget     *widget,
	     const gchar   *detail,
	     gint           x,
	     gint           y,
	     gint           width,
	     gint           height,
	     GtkOrientation orientation)
{
  MetalStyle *metal_style = METAL_STYLE (style);
  GdkPixmap *pm;
  GdkGC *fillgc;
  GdkGCValues values;
  GdkGC *lightgc, *midgc, *darkgc, *whitegc, *blackgc;

  sanitize_size (window, &width, &height);

  if (DETAIL ("paned"))
    {
      draw_paned_handle (style, window, state_type, shadow_type,
			 area, widget, x, y, width, height,
			 orientation);
      return;
    }
    
  /* Get colors */
  if (state_type == GTK_STATE_PRELIGHT)
    {
      lightgc = style->bg_gc[GTK_STATE_PRELIGHT];
      midgc = style->bg_gc[GTK_STATE_SELECTED];
      darkgc = style->fg_gc[GTK_STATE_PRELIGHT];
      whitegc = style->white_gc;
      blackgc = style->black_gc;
    }
  else
    {
      lightgc = metal_style->light_gray_gc;
      midgc = metal_style->mid_gray_gc;
      darkgc = metal_style->mid_gray_gc;
      whitegc = style->white_gc;
      blackgc = style->black_gc;
    }

  /* Draw textured surface */
  pm = gdk_pixmap_new (window, 8, 3, -1);

  gdk_draw_rectangle (pm, lightgc, TRUE, 0, 0, 8, 3);
  gdk_draw_point (pm, whitegc, 3, 0);
  gdk_draw_point (pm, whitegc, 0, 1);
  gdk_draw_point (pm, blackgc, 4, 1);
  gdk_draw_point (pm, blackgc, 1, 2);

  values.fill = GDK_TILED;
  values.ts_x_origin = x + 2;	/*5; */
  values.ts_y_origin = y + 2;	/*3; */
  fillgc = gdk_gc_new_with_values (window, &values,
				   GDK_GC_FILL | GDK_GC_TS_X_ORIGIN | GDK_GC_TS_Y_ORIGIN);

  /* Set Clip Region */
  if (area)
    {
      gdk_gc_set_clip_rectangle (lightgc, area);
      gdk_gc_set_clip_rectangle (midgc, area);
      gdk_gc_set_clip_rectangle (darkgc, area);
      gdk_gc_set_clip_rectangle (whitegc, area);
      gdk_gc_set_clip_rectangle (blackgc, area);
    }

  /* Draw backgound */
  gdk_draw_rectangle (window, lightgc, TRUE, x, y, width, height);

  /* Draw border */
  gdk_draw_rectangle (window, whitegc, FALSE, x + 1, y + 1, width - 2,
		      height - 2);
  gdk_draw_rectangle (window, darkgc, FALSE, x + 0, y + 0, width - 2, height
		      - 2);

  if (area)
    gdk_gc_set_clip_rectangle (fillgc, area);
  gdk_gc_set_tile (fillgc, pm);
  gdk_draw_rectangle (window, fillgc, TRUE, x + 2, y + 2, width - 4, height
		      - 4);

  gdk_gc_unref (fillgc);
  gdk_pixmap_unref (pm);

  /* Reset Clip Region */
  if (area)
    {
      gdk_gc_set_clip_rectangle (lightgc, NULL);
      gdk_gc_set_clip_rectangle (midgc, NULL);
      gdk_gc_set_clip_rectangle (darkgc, NULL);
      gdk_gc_set_clip_rectangle (whitegc, NULL);
      gdk_gc_set_clip_rectangle (blackgc, NULL);
    }
}

static void
shade (GdkColor * oldcolor, GdkColor * newcolor, float mult)
{
  newcolor->red = oldcolor->red * mult;
  newcolor->green = oldcolor->green * mult;
  newcolor->blue = oldcolor->blue * mult;
}

static void
metal_style_init_from_rc (GtkStyle * style,
			  GtkRcStyle * rc_style)
{
  MetalStyle *metal_style = METAL_STYLE (style);

  parent_class->init_from_rc (style, rc_style);

  /* Light Gray */
  shade (&style->white, &metal_style->light_gray, 0.8);
  shade (&style->white, &metal_style->mid_gray, 0.6);
  shade (&style->white, &metal_style->dark_gray, 0.4);
}

static GdkGC *
realize_color (GtkStyle * style,
	       GdkColor * color)
{
  GdkGCValues gc_values;

  gdk_colormap_alloc_color (style->colormap, color,
			    FALSE, TRUE);

  gc_values.foreground = *color;

  return gtk_gc_get (style->depth, style->colormap,
		     &gc_values, GDK_GC_FOREGROUND);
}

static void
metal_style_realize (GtkStyle * style)
{
  MetalStyle *metal_style = METAL_STYLE (style);

  parent_class->realize (style);

  metal_style->light_gray_gc = realize_color (style, &metal_style->light_gray);
  metal_style->mid_gray_gc = realize_color (style, &metal_style->mid_gray);
  metal_style->dark_gray_gc = realize_color (style, &metal_style->dark_gray);
}

static void
metal_style_unrealize (GtkStyle * style)
{
  MetalStyle *metal_style = METAL_STYLE (style);

  /* We don't free the colors, because we don't know if
   * gtk_gc_release() actually freed the GC. FIXME - need
   * a way of ref'ing colors explicitely so GtkGC can
   * handle things properly.
   */
  gtk_gc_release (metal_style->light_gray_gc);
  gtk_gc_release (metal_style->mid_gray_gc);
  gtk_gc_release (metal_style->dark_gray_gc);

  parent_class->unrealize (style);
}

static void
metal_style_init (MetalStyle * style)
{
}

static void
metal_style_class_init (MetalStyleClass * klass)
{
  GtkStyleClass *style_class = GTK_STYLE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  style_class->realize = metal_style_realize;
  style_class->unrealize = metal_style_unrealize;
  style_class->init_from_rc = metal_style_init_from_rc;

  style_class->draw_hline = draw_hline;
  style_class->draw_vline = draw_vline;
  style_class->draw_shadow = draw_shadow;
  style_class->draw_polygon = draw_polygon;
  style_class->draw_arrow = draw_arrow;
  style_class->draw_diamond = draw_diamond;
  style_class->draw_string = draw_string;
  style_class->draw_box = draw_box;
  style_class->draw_check = draw_check;
  style_class->draw_option = draw_option;
  style_class->draw_tab = draw_tab;
  style_class->draw_shadow_gap = draw_shadow_gap;
  style_class->draw_box_gap = draw_box_gap;
  style_class->draw_extension = draw_extension;
  style_class->draw_focus = draw_focus;
  style_class->draw_slider = draw_slider;
  style_class->draw_handle = draw_handle;
}

GType metal_type_style = 0;

void
metal_style_register_type (GTypeModule * module)
{
  static const GTypeInfo object_info =
  {
    sizeof (MetalStyleClass),
    (GBaseInitFunc) NULL,
    (GBaseFinalizeFunc) NULL,
    (GClassInitFunc) metal_style_class_init,
    NULL,			/* class_finalize */
    NULL,			/* class_data */
    sizeof (MetalStyle),
    0,				/* n_preallocs */
    (GInstanceInitFunc) metal_style_init,
  };

  metal_type_style = g_type_module_register_type (module,
						  GTK_TYPE_STYLE,
						  "MetalStyle",
						  &object_info, 0);
}
