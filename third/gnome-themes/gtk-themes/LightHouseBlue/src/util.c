/* GtkRangeLayout is a private structure, this is ugly but ... */
struct _GtkRangeLayout
{
  GdkRectangle stepper_a;
  GdkRectangle stepper_b;
  GdkRectangle stepper_c;
  GdkRectangle stepper_d;
};

static GtkRequisition default_option_indicator_size = { 7, 13 };
static GtkBorder default_option_indicator_spacing = { 7, 5, 2, 2 };


static gboolean sanitize_size (GdkWindow *window,
							   gint *width,
							   gint *height)
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

static void theme_draw_rectangle (GdkDrawable *drawable,
								  GdkGC *gc,
								  gint filled,
								  gint x,
								  gint y,
								  gint width,
								  gint height)
{
	gdk_draw_line (drawable, gc, x + 1, y, x + width - 1, y);
	gdk_draw_line (drawable, gc, x + 1, y + height, x + width - 1, y + height);
  
	gdk_draw_line (drawable, gc, x, y + 1, x, y + height - 1);
	gdk_draw_line (drawable, gc, x + width, y + 1, x + width, y + height - 1);
	
	/*gdk_draw_point (drawable, widget->style->white_gc, x, y);
	gdk_draw_point (drawable, widget->style->white_gc, x + width, y);
	gdk_draw_point (drawable, widget->style->white_gc, x, y + height);
	gdk_draw_point (drawable, widget->style->white_gc, x + width, y + height);*/
}

static gboolean is_stepper_a (GtkWidget *widget, gint x, gint y)
{
	GtkRange *range;
	if (GTK_IS_RANGE (widget))
	{
		range = GTK_RANGE (widget);
		return range->has_stepper_a
			   && (range->layout->stepper_a.x == (x - widget->allocation.x))
			   && (range->layout->stepper_a.y == (y - widget->allocation.y));
	}
	
	return FALSE;
}

static gboolean is_stepper_b (GtkWidget *widget, gint x, gint y)
{
	GtkRange *range;
	if (GTK_IS_RANGE (widget))
	{
		range = GTK_RANGE (widget);
		return range->has_stepper_b
			   && (range->layout->stepper_b.x == (x - widget->allocation.x))
			   && (range->layout->stepper_b.y == (y - widget->allocation.y));
	}
	
	return FALSE;
}

static gboolean is_stepper_c (GtkWidget *widget, gint x, gint y)
{
	GtkRange *range;
	if (GTK_IS_RANGE (widget))
	{
		range = GTK_RANGE (widget);
		return range->has_stepper_c
			   && (range->layout->stepper_c.x == (x - widget->allocation.x))
			   && (range->layout->stepper_c.y == (y - widget->allocation.y));
	}
	
	return FALSE;
}

static gboolean is_stepper_d (GtkWidget *widget, gint x, gint y)
{
	GtkRange *range;
	if (GTK_IS_RANGE (widget))
	{
		range = GTK_RANGE (widget);
		return range->has_stepper_d
			   && (range->layout->stepper_d.x == (x - widget->allocation.x))
			   && (range->layout->stepper_d.y == (y - widget->allocation.y));
	}
	
	return FALSE;
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

static void theme_draw_arrow (GdkWindow *window, GdkGC *gc, GdkRectangle *area,
			      GtkArrowType arrow_type, gint x, gint y, gint width, gint height)
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

static void theme_generate_pixmap (GtkWidget *widget, gint width, gint height)
{
  gint i;
  GPtrArray *xpm_data;
  GString *string;
  GString *fl_line; /* first and last line of pixmap */

  GdkPixmap *pixmap;
  GdkBitmap *mask;

  xpm_data = g_ptr_array_sized_new (height + 3);
  
  string = g_string_new ("");
  g_string_printf (string, "%i %i 2 1", width, height);
  g_ptr_array_add (xpm_data, (gpointer) string->str);
  g_ptr_array_add (xpm_data, (gpointer) "       c None");
  g_ptr_array_add (xpm_data, (gpointer) ".      c #000000");

  fl_line = g_string_new (" ");
  string = g_string_new (".");
  for (i = 0; i < width - 2; i++)
    {
      fl_line = g_string_append (fl_line, ".");
      string = g_string_append (string, ".");
    }
  fl_line = g_string_append (fl_line, " ");
  string = g_string_append (string, ".");

  g_ptr_array_add (xpm_data, (gpointer) fl_line->str);

  for (i = 0; i < height - 2; i++)
    {
      g_ptr_array_add (xpm_data, (gpointer) string->str);
    }
  g_ptr_array_add (xpm_data, (gpointer) fl_line->str);

  pixmap = gdk_pixmap_create_from_xpm_d (widget->window, &mask, NULL, (gchar **) xpm_data->pdata);
  gdk_window_shape_combine_mask(widget->window, mask, 0, 0);

  g_ptr_array_free (xpm_data, TRUE);
  g_object_unref (G_OBJECT (pixmap));
  g_object_unref (G_OBJECT (mask));
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
rgb_to_hls (gdouble *r,
            gdouble *g,
            gdouble *b)
{
  gdouble min;
  gdouble max;
  gdouble red;
  gdouble green;
  gdouble blue;
  gdouble h, l, s;
  gdouble delta;
  
  red = *r;
  green = *g;
  blue = *b;
  
  if (red > green)
    {
      if (red > blue)
        max = red;
      else
        max = blue;
      
      if (green < blue)
        min = green;
      else
        min = blue;
    }
  else
    {
      if (green > blue)
        max = green;
      else
        max = blue;
      
      if (red < blue)
        min = red;
      else
        min = blue;
    }
  
  l = (max + min) / 2;
  s = 0;
  h = 0;
  
  if (max != min)
    {
      if (l <= 0.5)
        s = (max - min) / (max + min);
      else
        s = (max - min) / (2 - max - min);
      
      delta = max -min;
      if (red == max)
        h = (green - blue) / delta;
      else if (green == max)
        h = 2 + (blue - red) / delta;
      else if (blue == max)
        h = 4 + (red - green) / delta;
      
      h *= 60;
      if (h < 0.0)
        h += 360;
    }
  
  *r = h;
  *g = l;
  *b = s;
}

static void
hls_to_rgb (gdouble *h,
            gdouble *l,
            gdouble *s)
{
  gdouble hue;
  gdouble lightness;
  gdouble saturation;
  gdouble m1, m2;
  gdouble r, g, b;
  
  lightness = *l;
  saturation = *s;
  
  if (lightness <= 0.5)
    m2 = lightness * (1 + saturation);
  else
    m2 = lightness + saturation - lightness * saturation;
  m1 = 2 * lightness - m2;
  
  if (saturation == 0)
    {
      *h = lightness;
      *l = lightness;
      *s = lightness;
    }
  else
    {
      hue = *h + 120;
      while (hue > 360)
        hue -= 360;
      while (hue < 0)
        hue += 360;
      
      if (hue < 60)
        r = m1 + (m2 - m1) * hue / 60;
      else if (hue < 180)
        r = m2;
      else if (hue < 240)
        r = m1 + (m2 - m1) * (240 - hue) / 60;
      else
        r = m1;
      
      hue = *h;
      while (hue > 360)
        hue -= 360;
      while (hue < 0)
        hue += 360;
      
      if (hue < 60)
        g = m1 + (m2 - m1) * hue / 60;
      else if (hue < 180)
        g = m2;
      else if (hue < 240)
        g = m1 + (m2 - m1) * (240 - hue) / 60;
      else
        g = m1;
      
      hue = *h - 120;
      while (hue > 360)
        hue -= 360;
      while (hue < 0)
        hue += 360;
      
      if (hue < 60)
        b = m1 + (m2 - m1) * hue / 60;
      else if (hue < 180)
        b = m2;
      else if (hue < 240)
        b = m1 + (m2 - m1) * (240 - hue) / 60;
      else
        b = m1;
      
      *h = r;
      *l = g;
      *s = b;
    }
}

static void
shade (GdkColor * a, GdkColor * b, float k)
{
  gdouble red;
  gdouble green;
  gdouble blue;
  
  red = (gdouble) a->red / 65535.0;
  green = (gdouble) a->green / 65535.0;
  blue = (gdouble) a->blue / 65535.0;
  
  rgb_to_hls (&red, &green, &blue);
  
  green *= k;
  if (green > 1.0)
    green = 1.0;
  else if (green < 0.0)
    green = 0.0;
  
  blue *= k;
  if (blue > 1.0)
    blue = 1.0;
  else if (blue < 0.0)
    blue = 0.0;
  
  hls_to_rgb (&red, &green, &blue);
  
  b->red = red * 65535.0;
  b->green = green * 65535.0;
  b->blue = blue * 65535.0;
}
