/*
 * HighContrast GTK+ rendering engine for Gnome-Themes.
 *
 * Copyright 2003 Sun Microsystems Inc.
 *
 * Also contains code from GTK+, 
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
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

#include "hc-style.h"
#include "hc-rc-style.h"

#include <gtk/gtk.h>

#include <math.h>
#include <string.h>

static GtkStyleClass *parent_class = NULL;

static const GtkRequisition default_option_indicator_size = { 7, 13 };
static const GtkBorder default_option_indicator_spacing = { 7, 5, 2, 2 };

static void
draw_box (GdkDrawable  *drawable,
	  GdkGC        *gc,
	  GdkRectangle *area,
	  gint          x,
	  gint          y,
	  gint          width,
	  gint          height)
{
  if (area)
    gdk_gc_set_clip_rectangle (gc, area);
  
  gdk_draw_rectangle (drawable, gc, TRUE, x, y, width, height);

  if (area)
    gdk_gc_set_clip_rectangle (gc, NULL);
}

static void
draw_dot (GdkWindow    *window,
	  GdkGC        *light_gc,
	  GdkGC        *dark_gc,
	  gint          x,
	  gint          y,
	  gushort       size)
{
  
  size = CLAMP (size, 2, 3);

  if (size == 2)
    {
      gdk_draw_point (window, light_gc, x, y);
      gdk_draw_point (window, light_gc, x+1, y+1);
    }
  else if (size == 3)
    {
      gdk_draw_point (window, light_gc, x, y);
      gdk_draw_point (window, light_gc, x+1, y);
      gdk_draw_point (window, light_gc, x, y+1);
      gdk_draw_point (window, dark_gc, x+1, y+2);
      gdk_draw_point (window, dark_gc, x+2, y+1);
      gdk_draw_point (window, dark_gc, x+2, y+2);
    }
}

static void
draw_arrow (GdkWindow     *window,
	    GdkGC         *gc,
	    GdkRectangle  *area,
	    GtkArrowType   arrow_type,
	    gint           x,
	    gint           y,
	    gint           width,
	    gint           height)
{
  gint i, j;

  if (area)
    gdk_gc_set_clip_rectangle (gc, area);

  if (arrow_type == GTK_ARROW_DOWN)
    {
      for (i = 0, j = 0; i < height; i++, j++)
	gdk_draw_line (window, gc, x + j, y + i, x + width - j - 1, y + i);
    }
  else if (arrow_type == GTK_ARROW_UP)
    {
      for (i = height - 1, j = 0; i >= 0; i--, j++)
	gdk_draw_line (window, gc, x + j, y + i, x + width - j - 1, y + i);
    }
  else if (arrow_type == GTK_ARROW_LEFT)
    {
      for (i = width - 1, j = 0; i >= 0; i--, j++)
	gdk_draw_line (window, gc, x + i, y + j, x + i, y + height - j - 1);
    }
  else if (arrow_type == GTK_ARROW_RIGHT)
    {
      for (i = 0, j = 0; i < width; i++, j++)
	gdk_draw_line (window, gc, x + i, y + j, x + i, y + height - j - 1);
    }

  if (area)
    gdk_gc_set_clip_rectangle (gc, NULL);
}

static void
hc_style_realize (GtkStyle *style)
{
  GdkGCValues gc_values;
  GdkGCValuesMask gc_values_mask;
  
  gint i;

  for (i = 0; i < 5; i++)
    {
      style->light[i].red = MIN (65535, style->bg[i].red + 0.5 * 65535);
      style->light[i].green = MIN (65535, style->bg[i].green + 0.5 * 65535);
      style->light[i].blue = MIN (65535, style->bg[i].blue + 0.5 * 65535);

      style->dark[i].red = MAX (0, style->bg[i].red - 0.5 * 65535);
      style->dark[i].green = MAX (0, style->bg[i].green - 0.5 * 65535);
      style->dark[i].blue = MAX (0, style->bg[i].blue - 0.5 * 65535);

      style->mid[i].red = (style->light[i].red + style->dark[i].red) / 2;
      style->mid[i].green = (style->light[i].green + style->dark[i].green) / 2;
      style->mid[i].blue = (style->light[i].blue + style->dark[i].blue) / 2;

      style->text_aa[i].red = (style->text[i].red + style->base[i].red) / 2;
      style->text_aa[i].green = (style->text[i].green + style->base[i].green) / 2;
      style->text_aa[i].blue = (style->text[i].blue + style->base[i].blue) / 2;
    }

  style->black.red = 0x0000;
  style->black.green = 0x0000;
  style->black.blue = 0x0000;
  gdk_colormap_alloc_color (style->colormap, &style->black, FALSE, TRUE);

  style->white.red = 0xffff;
  style->white.green = 0xffff;
  style->white.blue = 0xffff;
  gdk_colormap_alloc_color (style->colormap, &style->white, FALSE, TRUE);

  gc_values_mask = GDK_GC_FOREGROUND;
  
  gc_values.foreground = style->black;
  style->black_gc = gtk_gc_get (style->depth, style->colormap, &gc_values, gc_values_mask);
  
  gc_values.foreground = style->white;
  style->white_gc = gtk_gc_get (style->depth, style->colormap, &gc_values, gc_values_mask);
  
  for (i = 0; i < 5; i++)
    {
      if (!gdk_colormap_alloc_color (style->colormap, &style->fg[i], FALSE, TRUE))
        g_warning ("unable to allocate color: ( %d %d %d )",
                   style->fg[i].red, style->fg[i].green, style->fg[i].blue);
      if (!gdk_colormap_alloc_color (style->colormap, &style->bg[i], FALSE, TRUE))
        g_warning ("unable to allocate color: ( %d %d %d )",
                   style->bg[i].red, style->bg[i].green, style->bg[i].blue);
      if (!gdk_colormap_alloc_color (style->colormap, &style->light[i], FALSE, TRUE))
        g_warning ("unable to allocate color: ( %d %d %d )",
                   style->light[i].red, style->light[i].green, style->light[i].blue);
      if (!gdk_colormap_alloc_color (style->colormap, &style->dark[i], FALSE, TRUE))
        g_warning ("unable to allocate color: ( %d %d %d )",
                   style->dark[i].red, style->dark[i].green, style->dark[i].blue);
      if (!gdk_colormap_alloc_color (style->colormap, &style->mid[i], FALSE, TRUE))
        g_warning ("unable to allocate color: ( %d %d %d )",
                   style->mid[i].red, style->mid[i].green, style->mid[i].blue);
      if (!gdk_colormap_alloc_color (style->colormap, &style->text[i], FALSE, TRUE))
        g_warning ("unable to allocate color: ( %d %d %d )",
                   style->text[i].red, style->text[i].green, style->text[i].blue);
      if (!gdk_colormap_alloc_color (style->colormap, &style->base[i], FALSE, TRUE))
        g_warning ("unable to allocate color: ( %d %d %d )",
                   style->base[i].red, style->base[i].green, style->base[i].blue);
      if (!gdk_colormap_alloc_color (style->colormap, &style->text_aa[i], FALSE, TRUE))
        g_warning ("unable to allocate color: ( %d %d %d )",
                   style->text_aa[i].red, style->text_aa[i].green, style->text_aa[i].blue);
      
      gc_values.foreground = style->fg[i];
      style->fg_gc[i] = gtk_gc_get (style->depth, style->colormap, &gc_values, gc_values_mask);
      
      gc_values.foreground = style->bg[i];
      style->bg_gc[i] = gtk_gc_get (style->depth, style->colormap, &gc_values, gc_values_mask);
      
      gc_values.foreground = style->light[i];
      style->light_gc[i] = gtk_gc_get (style->depth, style->colormap, &gc_values, gc_values_mask);
      
      gc_values.foreground = style->dark[i];
      style->dark_gc[i] = gtk_gc_get (style->depth, style->colormap, &gc_values, gc_values_mask);
      
      gc_values.foreground = style->mid[i];
      style->mid_gc[i] = gtk_gc_get (style->depth, style->colormap, &gc_values, gc_values_mask);
      
      gc_values.foreground = style->text[i];
      style->text_gc[i] = gtk_gc_get (style->depth, style->colormap, &gc_values, gc_values_mask);
      
      gc_values.foreground = style->base[i];
      style->base_gc[i] = gtk_gc_get (style->depth, style->colormap, &gc_values, gc_values_mask);

      gc_values.foreground = style->text_aa[i];
      style->text_aa_gc[i] = gtk_gc_get (style->depth, style->colormap, &gc_values, gc_values_mask);
    }
}

static void
sanitize_size (GdkWindow *window,
	       gint      *width,
	       gint      *height)
{
  if ((*width == -1) && (*height == -1))
    gdk_drawable_get_size (window, width, height);
  else if (*width == -1)
    gdk_drawable_get_size (window, width, NULL);
  else if (*height == -1)
    gdk_drawable_get_size (window, NULL, height);
}

static void
hc_draw_shadow(GtkStyle * style,
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
  GdkGC *gc1 = NULL;
  GdkGC *gc2 = NULL;
  gint thickness_light;
  gint thickness_dark;
  gint i;
  
  g_return_if_fail (GTK_IS_STYLE (style));
  g_return_if_fail (window != NULL);

  sanitize_size (window, &width, &height);

  gdk_draw_rectangle (window, style->fg_gc[state_type], FALSE,
			  x, y, width - 1, height - 1);
}


static void 
hc_draw_extension (GtkStyle       *style,
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
		   GtkPositionType gap_side)
{
  GdkGC *gc1 = NULL;
  GdkGC *gc2 = NULL;
  GdkGC *gc3 = NULL;
  GdkGC *gc4 = NULL;
  
  g_return_if_fail (GTK_IS_STYLE (style));
  g_return_if_fail (window != NULL);
  
  gtk_style_apply_default_background (style, window,
                                      widget && !GTK_WIDGET_NO_WINDOW (widget),
                                      GTK_STATE_NORMAL, area, x, y, width, height);
  
  sanitize_size (window, &width, &height);
  
  switch (shadow_type)
    {
    case GTK_SHADOW_NONE:
      return;
    case GTK_SHADOW_IN:
      gc1 = style->dark_gc[state_type];
      gc2 = style->fg_gc[state_type];
      gc3 = style->bg_gc[state_type];
      gc4 = style->light_gc[state_type];
      break;
    case GTK_SHADOW_ETCHED_IN:
      gc1 = style->dark_gc[state_type];
      gc2 = style->light_gc[state_type];
      gc3 = style->dark_gc[state_type];
      gc4 = style->light_gc[state_type];
      break;
    case GTK_SHADOW_OUT:
      gc1 = style->light_gc[state_type];
      gc2 = style->bg_gc[state_type];
      gc3 = style->dark_gc[state_type];
      gc4 = style->fg_gc[state_type];
      break;
    case GTK_SHADOW_ETCHED_OUT:
      gc1 = style->light_gc[state_type];
      gc2 = style->dark_gc[state_type];
      gc3 = style->light_gc[state_type];
      gc4 = style->dark_gc[state_type];
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
    case GTK_SHADOW_IN:
    case GTK_SHADOW_OUT:
    case GTK_SHADOW_ETCHED_IN:
    case GTK_SHADOW_ETCHED_OUT:
      switch (gap_side)
        {
        case GTK_POS_TOP:
          gtk_style_apply_default_background (style, window,
                                              widget && !GTK_WIDGET_NO_WINDOW (widget),
                                              state_type, area,
                                              x + style->xthickness, 
                                              y, 
                                              width - (2 * style->xthickness), 
                                              height - (style->ythickness));
          gdk_draw_line (window, gc1,
                         x, y, x, y + height - 2);
          gdk_draw_line (window, gc2,
                         x + 1, y, x + 1, y + height - 2);
          
          gdk_draw_line (window, gc3,
                         x + 2, y + height - 2, x + width - 2, y + height - 2);
          gdk_draw_line (window, gc3,
                         x + width - 2, y, x + width - 2, y + height - 2);
          gdk_draw_line (window, gc4,
                         x + 1, y + height - 1, x + width - 2, y + height - 1);
          gdk_draw_line (window, gc4,
                         x + width - 1, y, x + width - 1, y + height - 2);
          break;
        case GTK_POS_BOTTOM:
          gtk_style_apply_default_background (style, window,
                                              widget && !GTK_WIDGET_NO_WINDOW (widget),
                                              state_type, area,
                                              x + style->xthickness, 
                                              y + style->ythickness, 
                                              width - (2 * style->xthickness), 
                                              height - (style->ythickness));
          gdk_draw_line (window, gc1,
                         x + 1, y, x + width - 2, y);
          gdk_draw_line (window, gc1,
                         x, y + 1, x, y + height - 1);
          gdk_draw_line (window, gc2,
                         x + 1, y + 1, x + width - 2, y + 1);
          gdk_draw_line (window, gc2,
                         x + 1, y + 1, x + 1, y + height - 1);
          
          gdk_draw_line (window, gc3,
                         x + width - 2, y + 2, x + width - 2, y + height - 1);
          gdk_draw_line (window, gc4,
                         x + width - 1, y + 1, x + width - 1, y + height - 1);
          break;
        case GTK_POS_LEFT:
          gtk_style_apply_default_background (style, window,
                                              widget && !GTK_WIDGET_NO_WINDOW (widget),
                                              state_type, area,
                                              x, 
                                              y + style->ythickness, 
                                              width - (style->xthickness), 
                                              height - (2 * style->ythickness));
          gdk_draw_line (window, gc1,
                         x, y, x + width - 2, y);
          gdk_draw_line (window, gc2,
                         x + 1, y + 1, x + width - 2, y + 1);
          
          gdk_draw_line (window, gc3,
                         x, y + height - 2, x + width - 2, y + height - 2);
          gdk_draw_line (window, gc3,
                         x + width - 2, y + 2, x + width - 2, y + height - 2);
          gdk_draw_line (window, gc4,
                         x, y + height - 1, x + width - 2, y + height - 1);
          gdk_draw_line (window, gc4,
                         x + width - 1, y + 1, x + width - 1, y + height - 2);
          break;
        case GTK_POS_RIGHT:
          gtk_style_apply_default_background (style, window,
                                              widget && !GTK_WIDGET_NO_WINDOW (widget),
                                              state_type, area,
                                              x + style->xthickness, 
                                              y + style->ythickness, 
                                              width - (style->xthickness), 
                                              height - (2 * style->ythickness));
          gdk_draw_line (window, gc1,
                         x + 1, y, x + width - 1, y);
          gdk_draw_line (window, gc1,
                         x, y + 1, x, y + height - 2);
          gdk_draw_line (window, gc2,
                         x + 1, y + 1, x + width - 1, y + 1);
          gdk_draw_line (window, gc2,
                         x + 1, y + 1, x + 1, y + height - 2);
          
          gdk_draw_line (window, gc3,
                         x + 2, y + height - 2, x + width - 1, y + height - 2);
          gdk_draw_line (window, gc4,
                         x + 1, y + height - 1, x + width - 1, y + height - 1);
          break;
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
hc_draw_check (GtkStyle      *style,
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
  gint line_width = 1;
  gboolean invert_checkboxes = FALSE; /* TODO: read this from RC file in engine-specific data? */

  gtk_widget_style_get (widget,
			"focus-line-width", &line_width,
			NULL);

  gdk_draw_rectangle (window,
		      widget->style->base_gc[state_type],
		      TRUE,
		      x, y,
		      width, height);
  gdk_draw_rectangle (window,
		      widget->style->text_gc[state_type],
		      FALSE,
		      x, y,
		      width, height);  
  
  if (shadow_type == GTK_SHADOW_IN)
    {
      GdkGC *gc;
      if (invert_checkboxes) {
        draw_box (window, style->text_gc[state_type], area, x, y, width, height);
	gc = style->base_gc[state_type];
      }
      else
	gc = style->text_gc[state_type];
      gdk_gc_set_line_attributes (gc, line_width*2,
				  GDK_LINE_SOLID,
				  GDK_CAP_PROJECTING, GDK_JOIN_MITER);
      gdk_draw_line (window, gc, x+2, y+2, x+width-2, y+height-2);
      gdk_draw_line (window, gc, x+2, y+height-2, x+width-2, y+2);
      gdk_gc_set_line_attributes (gc, 0, 
				  GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
    }
  else if (shadow_type == GTK_SHADOW_ETCHED_IN) /* inconsistent */
    {
      GdkGC *gc;
      if (invert_checkboxes) {
        draw_box (window, style->text_gc[state_type], area, x, y, width, height);
	gc = style->base_gc[state_type];
      }
      else
	gc = style->text_gc[state_type];
      gdk_gc_set_line_attributes (gc, line_width*2,
				  GDK_LINE_SOLID,
				  GDK_CAP_BUTT, GDK_JOIN_MITER);
      gdk_draw_line (window, gc, x+2, y+width/2+1, x+width-1, y+width/2+1);
      gdk_gc_set_line_attributes (gc, 0, 
				  GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
    }
}

static void 
hc_draw_option (GtkStyle      *style,
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
  gdk_draw_arc (window,
		widget->style->fg_gc[state_type],
		FALSE,
		x, y,
		width,
		height,
		0, 360*64);
  
  if (shadow_type == GTK_SHADOW_IN)
    {
      gdk_draw_arc (window,
		    widget->style->fg_gc[state_type],
		    TRUE,
		    x + 1,
		    y + 1,
		    width - 2,
		    height - 2,
		    0, 360*64);
    }
  else if (shadow_type == GTK_SHADOW_ETCHED_IN) /* inconsistent */
    {
      gdk_draw_arc (window,
		    widget->style->fg_gc[state_type],
		    FALSE,
		    x + 2,
		    y + 2,
		    width - 4,
		    height - 4,
		    0, 360*64);
    }
}

static void
option_menu_get_props (GtkWidget      *widget,
		       GtkRequisition *indicator_size,
		       GtkBorder      *indicator_spacing)
{
  GtkRequisition *tmp_size = NULL;
  GtkBorder *tmp_spacing = NULL;
  
  if (widget)
    gtk_widget_style_get (widget, 
			  "indicator_size", &tmp_size,
			  "indicator_spacing", &tmp_spacing,
			  NULL);

  if (tmp_size)
    {
      *indicator_size = *tmp_size;
      g_free (tmp_size);
    }
  else
    *indicator_size = default_option_indicator_size;

  if (tmp_spacing)
    {
      *indicator_spacing = *tmp_spacing;
      g_free (tmp_spacing);
    }
  else
    *indicator_spacing = default_option_indicator_spacing;
}

static void
hc_draw_tab (GtkStyle      *style,
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
#define ARROW_SPACE 4

  GtkRequisition indicator_size;
  GtkBorder indicator_spacing;
  gint arrow_height;
  
  option_menu_get_props (widget, &indicator_size, &indicator_spacing);

  indicator_size.width += (indicator_size.width % 2) - 1;
  arrow_height = indicator_size.width / 2 + 1;

  x += (width - indicator_size.width) / 2;
  y += (height - (2 * arrow_height + ARROW_SPACE)) / 2;

  draw_arrow (window, style->fg_gc[state_type], area,
	      GTK_ARROW_UP, x, y,
	      indicator_size.width, arrow_height);
  
  
  draw_arrow (window, style->fg_gc[state_type], area,
	      GTK_ARROW_DOWN, x, y + arrow_height + ARROW_SPACE,
	      indicator_size.width, arrow_height);
}

static void 
hc_draw_shadow_gap (GtkStyle       *style,
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
  GdkGC *gc1 = NULL;
  GdkGC *gc2 = NULL;
  GdkGC *gc3 = NULL;
  GdkGC *gc4 = NULL;
  
  g_return_if_fail (GTK_IS_STYLE (style));
  g_return_if_fail (window != NULL);
  
  sanitize_size (window, &width, &height);
  
  switch (shadow_type)
    {
    case GTK_SHADOW_NONE:
      return;
    case GTK_SHADOW_IN:
      gc1 = style->dark_gc[state_type];
      gc2 = style->fg_gc[state_type];
      gc3 = style->bg_gc[state_type];
      gc4 = style->light_gc[state_type];
      break;
    case GTK_SHADOW_ETCHED_IN:
      gc1 = style->dark_gc[state_type];
      gc2 = style->light_gc[state_type];
      gc3 = style->dark_gc[state_type];
      gc4 = style->light_gc[state_type];
      break;
    case GTK_SHADOW_OUT:
      gc1 = style->light_gc[state_type];
      gc2 = style->bg_gc[state_type];
      gc3 = style->dark_gc[state_type];
      gc4 = style->fg_gc[state_type];
      break;
    case GTK_SHADOW_ETCHED_OUT:
      gc1 = style->light_gc[state_type];
      gc2 = style->dark_gc[state_type];
      gc3 = style->light_gc[state_type];
      gc4 = style->dark_gc[state_type];
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
    case GTK_SHADOW_IN:
    case GTK_SHADOW_OUT:
    case GTK_SHADOW_ETCHED_IN:
    case GTK_SHADOW_ETCHED_OUT:
      switch (gap_side)
        {
        case GTK_POS_TOP:
          gdk_draw_line (window, gc1,
                         x, y, x, y + height - 1);
          gdk_draw_line (window, gc2,
                         x + 1, y, x + 1, y + height - 2);
          
          gdk_draw_line (window, gc3,
                         x + 1, y + height - 2, x + width - 2, y + height - 2);
          gdk_draw_line (window, gc3,
                         x + width - 2, y, x + width - 2, y + height - 2);
          gdk_draw_line (window, gc4,
                         x, y + height - 1, x + width - 1, y + height - 1);
          gdk_draw_line (window, gc4,
                         x + width - 1, y, x + width - 1, y + height - 1);
          if (gap_x > 0)
            {
              gdk_draw_line (window, gc1,
                             x, y, x + gap_x - 1, y);
              gdk_draw_line (window, gc2,
                             x + 1, y + 1, x + gap_x - 1, y + 1);
              gdk_draw_line (window, gc2,
                             x + gap_x, y, x + gap_x, y);
            }
          if ((width - (gap_x + gap_width)) > 0)
            {
              gdk_draw_line (window, gc1,
                             x + gap_x + gap_width, y, x + width - 2, y);
              gdk_draw_line (window, gc2,
                             x + gap_x + gap_width, y + 1, x + width - 3, y + 1);
              gdk_draw_line (window, gc2,
                             x + gap_x + gap_width - 1, y, x + gap_x + gap_width - 1, y);
            }
          break;
        case GTK_POS_BOTTOM:
          gdk_draw_line (window, gc1,
                         x, y, x + width - 1, y);
          gdk_draw_line (window, gc1,
                         x, y, x, y + height - 1);
          gdk_draw_line (window, gc2,
                         x + 1, y + 1, x + width - 2, y + 1);
          gdk_draw_line (window, gc2,
                         x + 1, y + 1, x + 1, y + height - 1);
          
          gdk_draw_line (window, gc3,
                         x + width - 2, y + 1, x + width - 2, y + height - 1);
          gdk_draw_line (window, gc4,
                         x + width - 1, y, x + width - 1, y + height - 1);
          if (gap_x > 0)
            {
              gdk_draw_line (window, gc4,
                             x, y + height - 1, x + gap_x - 1, y + height - 1);
              gdk_draw_line (window, gc3,
                             x + 1, y + height - 2, x + gap_x - 1, y + height - 2);
              gdk_draw_line (window, gc3,
                             x + gap_x, y + height - 1, x + gap_x, y + height - 1);
            }
          if ((width - (gap_x + gap_width)) > 0)
            {
              gdk_draw_line (window, gc4,
                             x + gap_x + gap_width, y + height - 1, x + width - 2, y + height - 1);
              gdk_draw_line (window, gc3,
                             x + gap_x + gap_width, y + height - 2, x + width - 2, y + height - 2);
              gdk_draw_line (window, gc3,
                             x + gap_x + gap_width - 1, y + height - 1, x + gap_x + gap_width - 1, y + height - 1);
            }
          break;
        case GTK_POS_LEFT:
          gdk_draw_line (window, gc1,
                         x, y, x + width - 1, y);
          gdk_draw_line (window, gc2,
                         x, y + 1, x + width - 2, y + 1);
          
          gdk_draw_line (window, gc3,
                         x, y + height - 2, x + width - 2, y + height - 2);
          gdk_draw_line (window, gc3,
                         x + width - 2, y + 1, x + width - 2, y + height - 2);
          gdk_draw_line (window, gc4,
                         x, y + height - 1, x + width - 1, y + height - 1);
          gdk_draw_line (window, gc4,
                         x + width - 1, y, x + width - 1, y + height - 1);
          if (gap_x > 0)
            {
              gdk_draw_line (window, gc1,
                             x, y, x, y + gap_x - 1);
              gdk_draw_line (window, gc2,
                             x + 1, y + 1, x + 1, y + gap_x - 1);
              gdk_draw_line (window, gc2,
                             x, y + gap_x, x, y + gap_x);
            }
          if ((width - (gap_x + gap_width)) > 0)
            {
              gdk_draw_line (window, gc1,
                             x, y + gap_x + gap_width, x, y + height - 2);
              gdk_draw_line (window, gc2,
                             x + 1, y + gap_x + gap_width, x + 1, y + height - 2);
              gdk_draw_line (window, gc2,
                             x, y + gap_x + gap_width - 1, x, y + gap_x + gap_width - 1);
            }
          break;
        case GTK_POS_RIGHT:
          gdk_draw_line (window, gc1,
                         x, y, x + width - 1, y);
          gdk_draw_line (window, gc1,
                         x, y, x, y + height - 1);
          gdk_draw_line (window, gc2,
                         x + 1, y + 1, x + width - 1, y + 1);
          gdk_draw_line (window, gc2,
                         x + 1, y + 1, x + 1, y + height - 2);
          
          gdk_draw_line (window, gc3,
                         x + 1, y + height - 2, x + width - 1, y + height - 2);
          gdk_draw_line (window, gc4,
                         x, y + height - 1, x + width - 1, y + height - 1);
          if (gap_x > 0)
            {
              gdk_draw_line (window, gc4,
                             x + width - 1, y, x + width - 1, y + gap_x - 1);
              gdk_draw_line (window, gc3,
                             x + width - 2, y + 1, x + width - 2, y + gap_x - 1);
              gdk_draw_line (window, gc3,
                             x + width - 1, y + gap_x, x + width - 1, y + gap_x);
            }
          if ((width - (gap_x + gap_width)) > 0)
            {
              gdk_draw_line (window, gc4,
                             x + width - 1, y + gap_x + gap_width, x + width - 1, y + height - 2);
              gdk_draw_line (window, gc3,
                             x + width - 2, y + gap_x + gap_width, x + width - 2, y + height - 2);
              gdk_draw_line (window, gc3,
                             x + width - 1, y + gap_x + gap_width - 1, x + width - 1, y + gap_x + gap_width - 1);
            }
          break;
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
hc_draw_layout (GtkStyle        *style,
		GdkWindow       *window,
		GtkStateType     state_type,
		gboolean         use_text,
		GdkRectangle    *area,
		GtkWidget       *widget,
		const gchar     *detail,
		gint             x,
		gint             y,
		PangoLayout     *layout)
{
  GdkGC *gc;
  
  g_return_if_fail (GTK_IS_STYLE (style));
  g_return_if_fail (window != NULL);

  gc = use_text ? style->text_gc[state_type] : style->fg_gc[state_type];
  
  if (area)
    gdk_gc_set_clip_rectangle (gc, area);

  gdk_draw_layout (window, gc, x, y, layout);

  if (area)
    gdk_gc_set_clip_rectangle (gc, NULL);
}

static void
calculate_arrow_geometry (GtkArrowType  arrow_type,
			  gint         *x,
			  gint         *y,
			  gint         *width,
			  gint         *height)
{
  gint w = *width;
  gint h = *height;
  
  switch (arrow_type)
    {
    case GTK_ARROW_UP:
    case GTK_ARROW_DOWN:
      w += (w % 2) - 1;
      h = (w / 2 + 1);
      
      if (h > *height)
	{
	  h = *height;
	  w = 2 * h - 1;
	}
      
      if (arrow_type == GTK_ARROW_DOWN)
	{
	  if (*height % 2 == 1 || h % 2 == 0)
	    *height += 1;
	}
      else
	{
	  if (*height % 2 == 0 || h % 2 == 0)
	    *height -= 1;
	}
      break;

    case GTK_ARROW_RIGHT:
    case GTK_ARROW_LEFT:
      h += (h % 2) - 1;
      w = (h / 2 + 1);
      
      if (w > *width)
	{
	  w = *width;
	  h = 2 * w - 1;
	}
      
      if (arrow_type == GTK_ARROW_RIGHT)
	{
	  if (*width % 2 == 1 || w % 2 == 0)
	    *width += 1;
	}
      else
	{
	  if (*width % 2 == 0 || w % 2 == 0)
	    *width -= 1;
	}
      break;
      
    default:
      /* should not be reached */
      break;
    }

  *x += (*width - w) / 2;
  *y += (*height - h) / 2;
  *height = h;
  *width = w;
}

static void
hc_draw_arrow (GtkStyle      *style,
	       GdkWindow     *window,
	       GtkStateType   state,
	       GtkShadowType  shadow,
	       GdkRectangle  *area,
	       GtkWidget     *widget,
	       const gchar   *detail,
	       GtkArrowType   arrow_type,
	       gboolean       fill,
	       gint           x,
	       gint           y,
	       gint           width,
	       gint           height)
{
  gint original_width, original_x;
  
  sanitize_size (window, &width, &height);

  original_width = width;
  original_x = x;

  calculate_arrow_geometry (arrow_type, &x, &y, &width, &height);

  if (detail && strcmp (detail, "menuitem") == 0
      && gtk_widget_get_direction (widget) == GTK_TEXT_DIR_LTR)
    x = original_x + original_width - width;

  draw_arrow (window, style->fg_gc[state], area, arrow_type,
	      x, y, width, height);
}

static void 
hc_draw_box_gap (GtkStyle       *style,
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
  GdkGC *gc1 = NULL;
  GdkGC *gc2 = NULL;
  GdkGC *gc3 = NULL;
  GdkGC *gc4 = NULL;
  
  g_return_if_fail (GTK_IS_STYLE (style));
  g_return_if_fail (window != NULL);
  
  gtk_style_apply_default_background (style, window,
                                      widget && !GTK_WIDGET_NO_WINDOW (widget),
                                      state_type, area, x, y, width, height);
  
  sanitize_size (window, &width, &height);
  
  switch (shadow_type)
    {
    case GTK_SHADOW_NONE:
      return;
    case GTK_SHADOW_IN:
      gc1 = style->dark_gc[state_type];
      gc2 = style->fg_gc[state_type];
      gc3 = style->bg_gc[state_type];
      gc4 = style->light_gc[state_type];
      break;
    case GTK_SHADOW_ETCHED_IN:
      gc1 = style->dark_gc[state_type];
      gc2 = style->light_gc[state_type];
      gc3 = style->dark_gc[state_type];
      gc4 = style->light_gc[state_type];
      break;
    case GTK_SHADOW_OUT:
      gc1 = style->light_gc[state_type];
      gc2 = style->bg_gc[state_type];
      gc3 = style->dark_gc[state_type];
      gc4 = style->fg_gc[state_type];
      break;
    case GTK_SHADOW_ETCHED_OUT:
      gc1 = style->light_gc[state_type];
      gc2 = style->dark_gc[state_type];
      gc3 = style->light_gc[state_type];
      gc4 = style->dark_gc[state_type];
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
    case GTK_SHADOW_IN:
    case GTK_SHADOW_OUT:
    case GTK_SHADOW_ETCHED_IN:
    case GTK_SHADOW_ETCHED_OUT:
      switch (gap_side)
        {
        case GTK_POS_TOP:
          gdk_draw_line (window, gc1,
                         x, y, x, y + height - 1);
          gdk_draw_line (window, gc2,
                         x + 1, y, x + 1, y + height - 2);
          
          gdk_draw_line (window, gc3,
                         x + 1, y + height - 2, x + width - 2, y + height - 2);
          gdk_draw_line (window, gc3,
                         x + width - 2, y, x + width - 2, y + height - 2);
          gdk_draw_line (window, gc4,
                         x, y + height - 1, x + width - 1, y + height - 1);
          gdk_draw_line (window, gc4,
                         x + width - 1, y, x + width - 1, y + height - 1);
          if (gap_x > 0)
            {
              gdk_draw_line (window, gc1,
                             x, y, x + gap_x - 1, y);
              gdk_draw_line (window, gc2,
                             x + 1, y + 1, x + gap_x - 1, y + 1);
              gdk_draw_line (window, gc2,
                             x + gap_x, y, x + gap_x, y);
            }
          if ((width - (gap_x + gap_width)) > 0)
            {
              gdk_draw_line (window, gc1,
                             x + gap_x + gap_width, y, x + width - 2, y);
              gdk_draw_line (window, gc2,
                             x + gap_x + gap_width, y + 1, x + width - 2, y + 1);
              gdk_draw_line (window, gc2,
                             x + gap_x + gap_width - 1, y, x + gap_x + gap_width - 1, y);
            }
          break;
        case  GTK_POS_BOTTOM:
          gdk_draw_line (window, gc1,
                         x, y, x + width - 1, y);
          gdk_draw_line (window, gc1,
                         x, y, x, y + height - 1);
          gdk_draw_line (window, gc2,
                         x + 1, y + 1, x + width - 2, y + 1);
          gdk_draw_line (window, gc2,
                         x + 1, y + 1, x + 1, y + height - 1);
          
          gdk_draw_line (window, gc3,
                         x + width - 2, y + 1, x + width - 2, y + height - 1);
          gdk_draw_line (window, gc4,
                         x + width - 1, y, x + width - 1, y + height - 1);
          if (gap_x > 0)
            {
              gdk_draw_line (window, gc4,
                             x, y + height - 1, x + gap_x - 1, y + height - 1);
              gdk_draw_line (window, gc3,
                             x + 1, y + height - 2, x + gap_x - 1, y + height - 2);
              gdk_draw_line (window, gc3,
                             x + gap_x, y + height - 1, x + gap_x, y + height - 1);
            }
          if ((width - (gap_x + gap_width)) > 0)
            {
              gdk_draw_line (window, gc4,
                             x + gap_x + gap_width, y + height - 1, x + width - 2, y + height - 1);
              gdk_draw_line (window, gc3,
                             x + gap_x + gap_width, y + height - 2, x + width - 2, y + height - 2);
              gdk_draw_line (window, gc3,
                             x + gap_x + gap_width - 1, y + height - 1, x + gap_x + gap_width - 1, y + height - 1);
            }
          break;
        case GTK_POS_LEFT:
          gdk_draw_line (window, gc1,
                         x, y, x + width - 1, y);
          gdk_draw_line (window, gc2,
                         x, y + 1, x + width - 2, y + 1);
          
          gdk_draw_line (window, gc3,
                         x, y + height - 2, x + width - 2, y + height - 2);
          gdk_draw_line (window, gc3,
                         x + width - 2, y + 1, x + width - 2, y + height - 2);
          gdk_draw_line (window, gc4,
                         x, y + height - 1, x + width - 1, y + height - 1);
          gdk_draw_line (window, gc4,
                         x + width - 1, y, x + width - 1, y + height - 1);
          if (gap_x > 0)
            {
              gdk_draw_line (window, gc1,
                             x, y, x, y + gap_x - 1);
              gdk_draw_line (window, gc2,
                             x + 1, y + 1, x + 1, y + gap_x - 1);
              gdk_draw_line (window, gc2,
                             x, y + gap_x, x, y + gap_x);
            }
          if ((width - (gap_x + gap_width)) > 0)
            {
              gdk_draw_line (window, gc1,
                             x, y + gap_x + gap_width, x, y + height - 2);
              gdk_draw_line (window, gc2,
                             x + 1, y + gap_x + gap_width, x + 1, y + height - 2);
              gdk_draw_line (window, gc2,
                             x, y + gap_x + gap_width - 1, x, y + gap_x + gap_width - 1);
            }
          break;
        case GTK_POS_RIGHT:
          gdk_draw_line (window, gc1,
                         x, y, x + width - 1, y);
          gdk_draw_line (window, gc1,
                         x, y, x, y + height - 1);
          gdk_draw_line (window, gc2,
                         x + 1, y + 1, x + width - 1, y + 1);
          gdk_draw_line (window, gc2,
                         x + 1, y + 1, x + 1, y + height - 2);
          
          gdk_draw_line (window, gc3,
                         x + 1, y + height - 2, x + width - 1, y + height - 2);
          gdk_draw_line (window, gc4,
                         x, y + height - 1, x + width - 1, y + height - 1);
          if (gap_x > 0)
            {
              gdk_draw_line (window, gc4,
                             x + width - 1, y, x + width - 1, y + gap_x - 1);
              gdk_draw_line (window, gc3,
                             x + width - 2, y + 1, x + width - 2, y + gap_x - 1);
              gdk_draw_line (window, gc3,
                             x + width - 1, y + gap_x, x + width - 1, y + gap_x);
            }
          if ((width - (gap_x + gap_width)) > 0)
            {
              gdk_draw_line (window, gc4,
                             x + width - 1, y + gap_x + gap_width, x + width - 1, y + height - 2);
              gdk_draw_line (window, gc3,
                             x + width - 2, y + gap_x + gap_width, x + width - 2, y + height - 2);
              gdk_draw_line (window, gc3,
                             x + width - 1, y + gap_x + gap_width - 1, x + width - 1, y + gap_x + gap_width - 1);
            }
          break;
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
hc_draw_handle (GtkStyle      *style,
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
  gint xx, yy;
  gint xthick, ythick;
  GdkGC *light_gc, *dark_gc;
  GdkRectangle rect;
  GdkRectangle dest;
  gint intersect;
  
  g_return_if_fail (GTK_IS_STYLE (style));
  g_return_if_fail (window != NULL);
  
  sanitize_size (window, &width, &height);
  
  gtk_paint_box (style, window, state_type, shadow_type, area, widget, 
                 detail, x, y, width, height);
  
  
  if (!strcmp (detail, "paned"))
    {
      /* we want to ignore the shadow border in paned widgets */
      xthick = 0;
      ythick = 0;
    }
  else
    {
      xthick = style->xthickness;
      ythick = style->ythickness;
    }
  
  light_gc = style->light_gc[state_type];
  dark_gc = style->dark_gc[state_type];
  rect.x = x + xthick;
  rect.y = y + ythick;
  rect.width = width - (xthick * 2);
  rect.height = height - (ythick * 2);

  if (area)
      intersect = gdk_rectangle_intersect (area, &rect, &dest);
  else
    {
      intersect = TRUE;
      dest = rect;
    }

  if (!intersect)
    return;

  gdk_gc_set_clip_rectangle (light_gc, &dest);
  gdk_gc_set_clip_rectangle (dark_gc, &dest);

  if (!strcmp (detail, "paned"))
    {
      if (orientation == GTK_ORIENTATION_HORIZONTAL)
	for (xx = x + width/2 - 15; xx <= x + width/2 + 15; xx += 5)
	  draw_dot (window, light_gc, dark_gc, xx, y + height/2 - 1, 3);
      else
	for (yy = y + height/2 - 15; yy <= y + height/2 + 15; yy += 5)
	  draw_dot (window, light_gc, dark_gc, x + width/2 - 1, yy, 3);
    }
  else
    {
      for (yy = y + ythick; yy < (y + height - ythick); yy += 3)
	for (xx = x + xthick; xx < (x + width - xthick); xx += 6)
	  {
	    draw_dot (window, light_gc, dark_gc, xx, yy, 2);
	    draw_dot (window, light_gc, dark_gc, xx + 3, yy + 1, 2);
	  }
    }

  gdk_gc_set_clip_rectangle (light_gc, NULL);
  gdk_gc_set_clip_rectangle (dark_gc, NULL);
}

static void
hc_draw_hline (GtkStyle     *style,
	       GdkWindow    *window,
	       GtkStateType  state_type,
	       GdkRectangle  *area,
	       GtkWidget     *widget,
	       const gchar   *detail,
	       gint          x1,
	       gint          x2,
	       gint          y)
{
  gint thickness_light;
  gint thickness_dark;
  gint i;
  
  g_return_if_fail (GTK_IS_STYLE (style));
  g_return_if_fail (window != NULL);
  
  thickness_light = style->ythickness / 2;
  thickness_dark = style->ythickness - thickness_light;
  
  if (area)
    {
      gdk_gc_set_clip_rectangle (style->light_gc[state_type], area);
      gdk_gc_set_clip_rectangle (style->dark_gc[state_type], area);
    }
  
  if (detail && !strcmp (detail, "label"))
    {
      gdk_draw_line (window, style->fg_gc[state_type], x1, y, x2, y);     
    }
  else
    {
      for (i = 0; i < thickness_dark; i++)
        {
          gdk_draw_line (window, style->light_gc[state_type], x2 - i - 1, y + i, x2, y + i);
          gdk_draw_line (window, style->dark_gc[state_type], x1, y + i, x2 - i - 1, y + i);
        }
      
      y += thickness_dark;
      for (i = 0; i < thickness_light; i++)
        {
          gdk_draw_line (window, style->dark_gc[state_type], x1, y + i, x1 + thickness_light - i - 1, y + i);
          gdk_draw_line (window, style->light_gc[state_type], x1 + thickness_light - i - 1, y + i, x2, y + i);
        }
    }
  
  if (area)
    {
      gdk_gc_set_clip_rectangle (style->light_gc[state_type], NULL);
      gdk_gc_set_clip_rectangle (style->dark_gc[state_type], NULL);
    }
}

static void
hc_draw_diamond (GtkStyle      *style,
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
  gint half_width;
  gint half_height;
  GdkGC *outer_nw = NULL;
  GdkGC *outer_ne = NULL;
  GdkGC *outer_sw = NULL;
  GdkGC *outer_se = NULL;
  GdkGC *middle_nw = NULL;
  GdkGC *middle_ne = NULL;
  GdkGC *middle_sw = NULL;
  GdkGC *middle_se = NULL;
  GdkGC *inner_nw = NULL;
  GdkGC *inner_ne = NULL;
  GdkGC *inner_sw = NULL;
  GdkGC *inner_se = NULL;
  
  g_return_if_fail (GTK_IS_STYLE (style));
  g_return_if_fail (window != NULL);
  
  sanitize_size (window, &width, &height);
  
  half_width = width / 2;
  half_height = height / 2;
  
  if (area)
    {
      gdk_gc_set_clip_rectangle (style->light_gc[state_type], area);
      gdk_gc_set_clip_rectangle (style->bg_gc[state_type], area);
      gdk_gc_set_clip_rectangle (style->dark_gc[state_type], area);
      gdk_gc_set_clip_rectangle (style->fg_gc[state_type], area);
    }
  
  switch (shadow_type)
    {
    case GTK_SHADOW_IN:
      inner_sw = inner_se = style->bg_gc[state_type];
      middle_sw = middle_se = style->light_gc[state_type];
      outer_sw = outer_se = style->light_gc[state_type];
      inner_nw = inner_ne = style->fg_gc[state_type];
      middle_nw = middle_ne = style->dark_gc[state_type];
      outer_nw = outer_ne = style->dark_gc[state_type];
      break;
          
    case GTK_SHADOW_OUT:
      inner_sw = inner_se = style->dark_gc[state_type];
      middle_sw = middle_se = style->dark_gc[state_type];
      outer_sw = outer_se = style->fg_gc[state_type];
      inner_nw = inner_ne = style->bg_gc[state_type];
      middle_nw = middle_ne = style->light_gc[state_type];
      outer_nw = outer_ne = style->light_gc[state_type];
      break;

    case GTK_SHADOW_ETCHED_IN:
      inner_sw = inner_se = style->bg_gc[state_type];
      middle_sw = middle_se = style->dark_gc[state_type];
      outer_sw = outer_se = style->light_gc[state_type];
      inner_nw = inner_ne = style->bg_gc[state_type];
      middle_nw = middle_ne = style->light_gc[state_type];
      outer_nw = outer_ne = style->dark_gc[state_type];
      break;

    case GTK_SHADOW_ETCHED_OUT:
      inner_sw = inner_se = style->bg_gc[state_type];
      middle_sw = middle_se = style->light_gc[state_type];
      outer_sw = outer_se = style->dark_gc[state_type];
      inner_nw = inner_ne = style->bg_gc[state_type];
      middle_nw = middle_ne = style->dark_gc[state_type];
      outer_nw = outer_ne = style->light_gc[state_type];
      break;
      
    default:

      break;
    }

  if (inner_sw)
    {
      gdk_draw_line (window, inner_sw,
                     x + 2, y + half_height,
                     x + half_width, y + height - 2);
      gdk_draw_line (window, inner_se,
                     x + half_width, y + height - 2,
                     x + width - 2, y + half_height);
      gdk_draw_line (window, middle_sw,
                     x + 1, y + half_height,
                     x + half_width, y + height - 1);
      gdk_draw_line (window, middle_se,
                     x + half_width, y + height - 1,
                     x + width - 1, y + half_height);
      gdk_draw_line (window, outer_sw,
                     x, y + half_height,
                     x + half_width, y + height);
      gdk_draw_line (window, outer_se,
                     x + half_width, y + height,
                     x + width, y + half_height);
  
      gdk_draw_line (window, inner_nw,
                     x + 2, y + half_height,
                     x + half_width, y + 2);
      gdk_draw_line (window, inner_ne,
                     x + half_width, y + 2,
                     x + width - 2, y + half_height);
      gdk_draw_line (window, middle_nw,
                     x + 1, y + half_height,
                     x + half_width, y + 1);
      gdk_draw_line (window, middle_ne,
                     x + half_width, y + 1,
                     x + width - 1, y + half_height);
      gdk_draw_line (window, outer_nw,
                     x, y + half_height,
                     x + half_width, y);
      gdk_draw_line (window, outer_ne,
                     x + half_width, y,
                     x + width, y + half_height);
    }
  
  if (area)
    {
      gdk_gc_set_clip_rectangle (style->light_gc[state_type], NULL);
      gdk_gc_set_clip_rectangle (style->bg_gc[state_type], NULL);
      gdk_gc_set_clip_rectangle (style->dark_gc[state_type], NULL);
      gdk_gc_set_clip_rectangle (style->fg_gc[state_type], NULL);
    }
}

static void
hc_draw_polygon (GtkStyle      *style,
		 GdkWindow     *window,
		 GtkStateType   state_type,
		 GtkShadowType  shadow_type,
		 GdkRectangle  *area,
		 GtkWidget     *widget,
		 const gchar   *detail,
		 GdkPoint      *points,
		 gint           npoints,
		 gboolean       fill)
{
  static const gdouble pi_over_4 = G_PI_4;
  static const gdouble pi_3_over_4 = G_PI_4 * 3;
  GdkGC *gc1;
  GdkGC *gc2;
  GdkGC *gc3;
  GdkGC *gc4;
  gdouble angle;
  gint xadjust;
  gint yadjust;
  gint i;
  
  g_return_if_fail (GTK_IS_STYLE (style));
  g_return_if_fail (window != NULL);
  g_return_if_fail (points != NULL);
  
  switch (shadow_type)
    {
    case GTK_SHADOW_IN:
      gc1 = style->bg_gc[state_type];
      gc2 = style->dark_gc[state_type];
      gc3 = style->light_gc[state_type];
      gc4 = style->fg_gc[state_type];
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
      gc3 = style->fg_gc[state_type];
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
      if ((points[i].x == points[i+1].x) &&
          (points[i].y == points[i+1].y))
        {
          angle = 0;
        }
      else
        {
          angle = atan2 (points[i+1].y - points[i].y,
                         points[i+1].x - points[i].x);
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
                         points[i].x-xadjust, points[i].y-yadjust,
                         points[i+1].x-xadjust, points[i+1].y-yadjust);
          gdk_draw_line (window, gc3,
                         points[i].x, points[i].y,
                         points[i+1].x, points[i+1].y);
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
                         points[i].x+xadjust, points[i].y+yadjust,
                         points[i+1].x+xadjust, points[i+1].y+yadjust);
          gdk_draw_line (window, gc2,
                         points[i].x, points[i].y,
                         points[i+1].x, points[i+1].y);
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
hc_style_class_init (HcStyleClass *klass)
{
  GtkStyleClass *style_class = GTK_STYLE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  style_class->draw_shadow = hc_draw_shadow;
  style_class->realize = hc_style_realize;
  style_class->draw_check = hc_draw_check;
  style_class->draw_option = hc_draw_option;
  style_class->draw_layout = hc_draw_layout;
  style_class->draw_extension = hc_draw_extension;
  style_class->draw_tab = hc_draw_tab;
  style_class->draw_shadow_gap = hc_draw_shadow_gap;
  style_class->draw_arrow = hc_draw_arrow;
  style_class->draw_box_gap = hc_draw_box_gap;
  style_class->draw_handle = hc_draw_handle;  
  style_class->draw_hline = hc_draw_hline; 
  style_class->draw_diamond = hc_draw_diamond;
  style_class->draw_polygon = hc_draw_polygon;
}

GType hc_type_style = 0;

void
hc_style_register_type (GTypeModule *module)
{
  static const GTypeInfo object_info =
  {
    sizeof (HcStyleClass),
    (GBaseInitFunc) NULL,
    (GBaseFinalizeFunc) NULL,
    (GClassInitFunc) hc_style_class_init,
    NULL,           /* class_finalize */
    NULL,           /* class_data */
    sizeof (HcStyle),
    0,              /* n_preallocs */
    (GInstanceInitFunc) NULL,
  };
  
  hc_type_style = g_type_module_register_type (module,
							   GTK_TYPE_STYLE,
							   "HcStyle",
							   &object_info, 0);
}
