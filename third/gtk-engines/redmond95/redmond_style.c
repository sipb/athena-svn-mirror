#include "redmond_style.h"

#include <math.h>
#include <string.h>
#include <gtk/gtk.h>

/* Default values, not normally used
 */
static GtkRequisition default_option_indicator_size = { 9, 8 };
static GtkBorder default_option_indicator_spacing = { 7, 5, 2, 2 };

static GtkStyleClass *parent_class;

typedef enum {
  CHECK_AA,
  CHECK_BASE,
  CHECK_BLACK,
  CHECK_DARK,
  CHECK_LIGHT,
  CHECK_MID,
  CHECK_TEXT,
  RADIO_BASE,
  RADIO_BLACK,
  RADIO_DARK,
  RADIO_LIGHT,
  RADIO_MID,
  RADIO_TEXT
} Part;

#define PART_SIZE 13

static char check_aa_bits[] = {
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char check_base_bits[] = {
 0x00,0x00,0x00,0x00,0xfc,0x07,0xfc,0x07,0xfc,0x07,0xfc,0x07,0xfc,0x07,0xfc,
 0x07,0xfc,0x07,0xfc,0x07,0xfc,0x07,0x00,0x00,0x00,0x00};
static char check_black_bits[] = {
 0x00,0x00,0xfe,0x0f,0x02,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x02,
 0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x00,0x00};
static char check_dark_bits[] = {
 0xff,0x1f,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
 0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00};
static char check_light_bits[] = {
 0x00,0x00,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,
 0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0xfe,0x1f};
static char check_mid_bits[] = {
 0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x08,0x00,
 0x08,0x00,0x08,0x00,0x08,0x00,0x08,0xfc,0x0f,0x00,0x00};
static char check_text_bits[] = {
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x03,0x88,0x03,0xd8,0x01,0xf8,
 0x00,0x70,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char radio_base_bits[] = {
 0x00,0x00,0x00,0x00,0xf0,0x01,0xf8,0x03,0xfc,0x07,0xfc,0x07,0xfc,0x07,0xfc,
 0x07,0xfc,0x07,0xf8,0x03,0xf0,0x01,0x00,0x00,0x00,0x00};
static char radio_black_bits[] = {
 0x00,0x00,0xf0,0x01,0x0c,0x02,0x04,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x02,
 0x00,0x02,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char radio_dark_bits[] = {
 0xf0,0x01,0x0c,0x06,0x02,0x00,0x02,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
 0x00,0x01,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x00,0x00};
static char radio_light_bits[] = {
 0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x08,0x00,0x10,0x00,0x10,0x00,0x10,0x00,
 0x10,0x00,0x10,0x00,0x08,0x00,0x08,0x0c,0x06,0xf0,0x01};
static char radio_mid_bits[] = {
 0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x04,0x00,0x08,0x00,0x08,0x00,0x08,0x00,
 0x08,0x00,0x08,0x00,0x04,0x0c,0x06,0xf0,0x01,0x00,0x00};
static char radio_text_bits[] = {
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe0,0x00,0xf0,0x01,0xf0,0x01,0xf0,
 0x01,0xe0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static struct {
  char      *bits;
  GdkBitmap *bmap;
} parts[] = {
  { check_aa_bits, NULL },
  { check_base_bits, NULL },
  { check_black_bits, NULL },
  { check_dark_bits, NULL },
  { check_light_bits, NULL },
  { check_mid_bits, NULL },
  { check_text_bits, NULL },
  { radio_base_bits, NULL },
  { radio_black_bits, NULL },
  { radio_dark_bits, NULL },
  { radio_light_bits, NULL },
  { radio_mid_bits, NULL },
  { radio_text_bits, NULL }
};

static gboolean 
sanitize_size (GdkWindow *window,
	       gint      *width,
	       gint      *height)
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
draw_part (GdkDrawable  *drawable,
	   GdkGC        *gc,
	   GdkRectangle *area,
	   gint          x,
	   gint          y,
	   Part          part)
{
  if (area)
    gdk_gc_set_clip_rectangle (gc, area);
  
  if (!parts[part].bmap)
      parts[part].bmap = gdk_bitmap_create_from_data (drawable,
						      parts[part].bits,
						      PART_SIZE, PART_SIZE);

  gdk_gc_set_ts_origin (gc, x, y);
  gdk_gc_set_stipple (gc, parts[part].bmap);
  gdk_gc_set_fill (gc, GDK_STIPPLED);

  gdk_draw_rectangle (drawable, gc, TRUE, x, y, PART_SIZE, PART_SIZE);

  gdk_gc_set_fill (gc, GDK_SOLID);

  if (area)
    gdk_gc_set_clip_rectangle (gc, NULL);
}

static void
draw_check(GtkStyle      *style,
	   GdkWindow     *window,
	   GtkStateType   state,
	   GtkShadowType  shadow,
	   GdkRectangle  *area,
	   GtkWidget     *widget,
	   const gchar   *detail,
	   gint           x,
	   gint           y,
	   gint           width,
	   gint           height)
{
  x -= (1 + PART_SIZE - width) / 2;
  y -= (1 + PART_SIZE - height) / 2;
      
  if (strcmp (detail, "check") == 0)	/* Menu item */
    {
      if (shadow == GTK_SHADOW_IN)
	{
	  draw_part (window, style->black_gc, area, x, y, CHECK_TEXT);
	  draw_part (window, style->dark_gc[state], area, x, y, CHECK_AA);
	}
    }
  else
    {
      draw_part (window, style->black_gc, area, x, y, CHECK_BLACK);
      draw_part (window, style->dark_gc[state], area, x, y, CHECK_DARK);
      draw_part (window, style->mid_gc[state], area, x, y, CHECK_MID);
      draw_part (window, style->light_gc[state], area, x, y, CHECK_LIGHT);
      draw_part (window, style->base_gc[state], area, x, y, CHECK_BASE);
      
      if (shadow == GTK_SHADOW_IN)
	{
	  draw_part (window, style->text_gc[state], area, x, y, CHECK_TEXT);
	  draw_part (window, style->text_aa_gc[state], area, x, y, CHECK_AA);
	}
    }
}

static void
draw_option(GtkStyle      *style,
	    GdkWindow     *window,
	    GtkStateType   state,
	    GtkShadowType  shadow,
	    GdkRectangle  *area,
	    GtkWidget     *widget,
	    const gchar   *detail,
	    gint           x,
	    gint           y,
	    gint           width,
	    gint           height)
{
  x -= (1 + PART_SIZE - width) / 2;
  y -= (1 + PART_SIZE - height) / 2;
      
  if (strcmp (detail, "option") == 0)	/* Menu item */
    {
      if (shadow == GTK_SHADOW_IN)
	draw_part (window, style->fg_gc[state], area, x, y, RADIO_TEXT);
    }
  else
    {
      draw_part (window, style->black_gc, area, x, y, RADIO_BLACK);
      draw_part (window, style->dark_gc[state], area, x, y, RADIO_DARK);
      draw_part (window, style->mid_gc[state], area, x, y, RADIO_MID);
      draw_part (window, style->light_gc[state], area, x, y, RADIO_LIGHT);
      draw_part (window, style->base_gc[state], area, x, y, RADIO_BASE);
      
      if (shadow == GTK_SHADOW_IN)
	draw_part (window, style->text_gc[state], area, x, y, RADIO_TEXT);
    }
}

static void
draw_varrow (GdkWindow     *window,
	     GdkGC         *gc,
	     GtkShadowType  shadow_type,
	     GdkRectangle  *area,
	     GtkArrowType   arrow_type,
	     gint           x,
	     gint           y,
	     gint           width,
	     gint           height)
{
  gint steps, extra;
  gint y_start, y_increment;
  gint i;

  if (area)
    gdk_gc_set_clip_rectangle (gc, area);
  
  width = width + width % 2 - 1;	/* Force odd */
  
  steps = 1 + width / 2;

  extra = height - steps;

  if (arrow_type == GTK_ARROW_DOWN)
    {
      y_start = y;
      y_increment = 1;
    }
  else
    {
      y_start = y + height - 1;
      y_increment = -1;
    }

#if 0
  for (i = 0; i < extra; i++)
    {
      gdk_draw_line (window, gc,
		     x,              y_start + i * y_increment,
		     x + width - 1,  y_start + i * y_increment);
    }
#endif
  for (i = extra; i < height; i++)
    {
      gdk_draw_line (window, gc,
		     x + (i - extra),              y_start + i * y_increment,
		     x + width - (i - extra) - 1,  y_start + i * y_increment);
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
	     gint           x,
	     gint           y,
	     gint           width,
	     gint           height)
{
  gint steps, extra;
  gint x_start, x_increment;
  gint i;

  if (area)
    gdk_gc_set_clip_rectangle (gc, area);
  
  height = height + height % 2 - 1;	/* Force odd */
  
  steps = 1 + height / 2;

  extra = width - steps;

  if (arrow_type == GTK_ARROW_RIGHT)
    {
      x_start = x;
      x_increment = 1;
    }
  else
    {
      x_start = x + width - 1;
      x_increment = -1;
    }

#if 0
  for (i = 0; i < extra; i++)
    {
      gdk_draw_line (window, gc,
		     x_start + i * x_increment, y,
		     x_start + i * x_increment, y + height - 1);
    }
#endif
  for (i = extra; i < width; i++)
    {
      gdk_draw_line (window, gc,
		     x_start + i * x_increment, y + (i - extra),
		     x_start + i * x_increment, y + height - (i - extra) - 1);
    }
  

  if (area)
    gdk_gc_set_clip_rectangle (gc, NULL);
}

static void
draw_arrow (GtkStyle      *style,
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
  sanitize_size (window, &width, &height);
  
  if (detail && strcmp (detail, "spinbutton") == 0)
    {
      x += (width - 7) / 2;

      if (arrow_type == GTK_ARROW_UP)
	y += (height - 4) / 2;
      else
	y += (1 + height - 4) / 2;

      draw_varrow (window, style->fg_gc[state], shadow, area, arrow_type,
		   x, y, 7, 4);
    }
  else if (detail && strcmp (detail, "vscrollbar") == 0)
    {
      parent_class->draw_box (style, window, state, shadow, area,
			      widget, detail, x, y, width, height);
      
      x += (width - 7) / 2;
      y += (height - 5) / 2;

      draw_varrow (window, style->fg_gc[state], shadow, area, arrow_type,
		   x, y, 7, 5);
      
    }
  else if (detail && strcmp (detail, "hscrollbar") == 0)
    {
      parent_class->draw_box (style, window, state, shadow, area,
			      widget, detail, x, y, width, height);
      
      y += (height - 7) / 2;
      x += (width - 5) / 2;

      draw_harrow (window, style->fg_gc[state], shadow, area, arrow_type,
		   x, y, 5, 7);
    }
  else
    {
      if (arrow_type == GTK_ARROW_UP || arrow_type == GTK_ARROW_DOWN)
	{
	  x += (width - 7) / 2;
	  y += (height - 5) / 2;
	  
	  draw_varrow (window, style->fg_gc[state], shadow, area, arrow_type,
		       x, y, 7, 5);
	}
      else
	{
	  x += (width - 5) / 2;
	  y += (height - 7) / 2;
	  
	  draw_harrow (window, style->fg_gc[state], shadow, area, arrow_type,
		       x, y, 5, 7);
	}
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
  if (detail && strcmp (detail, "menuitem") == 0) 
    shadow_type = GTK_SHADOW_NONE;
  
  if ((detail && strcmp (detail, "trough") == 0) &&
      !(widget && GTK_IS_PROGRESS_BAR (widget)))
    {
      GdkGCValues gc_values;
      GdkGC *gc;
      GdkPixmap *pixmap;

      sanitize_size (window, &width, &height);
	  
      pixmap = gdk_pixmap_new (window, 2, 2, -1);

      gdk_draw_point (pixmap, style->bg_gc[GTK_STATE_NORMAL], 0, 0);
      gdk_draw_point (pixmap, style->bg_gc[GTK_STATE_NORMAL], 1, 1);
      gdk_draw_point (pixmap, style->light_gc[GTK_STATE_NORMAL], 1, 0);
      gdk_draw_point (pixmap, style->light_gc[GTK_STATE_NORMAL], 0, 1);

      gc_values.fill = GDK_TILED;
      gc_values.tile = pixmap;
      gc_values.ts_x_origin = x;
      gc_values.ts_y_origin = y;
      gc = gdk_gc_new_with_values (window, &gc_values,
				   GDK_GC_TS_X_ORIGIN | GDK_GC_TS_Y_ORIGIN | GDK_GC_FILL | GDK_GC_TILE);

      if (area)
	gdk_gc_set_clip_rectangle (gc, area);
      
      gdk_draw_rectangle (window, gc, TRUE, x, y, width, height);

      gdk_gc_unref (gc);
      gdk_pixmap_unref (pixmap);
      
      return;
    }
      
  parent_class->draw_box (style, window, state_type, shadow_type, area,
			  widget, detail, x, y, width, height);

  if (detail && strcmp (detail, "optionmenu") == 0)
    {
      GtkRequisition indicator_size;
      GtkBorder indicator_spacing;

      option_menu_get_props (widget, &indicator_size, &indicator_spacing);

      sanitize_size (window, &width, &height);
  
      parent_class->draw_vline (style, window, state_type, area, widget,
				detail,
				y + style->ythickness + 1,
				y + height - style->ythickness - 3,
				x + width - (indicator_size.width + indicator_spacing.left + indicator_spacing.right) - style->xthickness);
    }
}

static void
draw_tab (GtkStyle      *style,
	  GdkWindow     *window,
	  GtkStateType   state,
	  GtkShadowType  shadow,
	  GdkRectangle  *area,
	  GtkWidget     *widget,
	  const gchar   *detail,
	  gint           x,
	  gint           y,
	  gint           width,
	  gint           height)
{
  GtkRequisition indicator_size;
  GtkBorder indicator_spacing;
  
  gint arrow_height;
  
  g_return_if_fail (style != NULL);
  g_return_if_fail (window != NULL);

  if (widget)
    gtk_widget_style_get (widget, "indicator_size", &indicator_size, NULL);

  option_menu_get_props (widget, &indicator_size, &indicator_spacing);

  x += (width - indicator_size.width) / 2;
  arrow_height = (indicator_size.width + 1) / 2;
  
  y += (height - arrow_height) / 2;

  draw_varrow (window, style->black_gc, shadow, area, GTK_ARROW_DOWN,
	       x, y, indicator_size.width, arrow_height);
}

static void
redmond_style_init (RedmondStyle *style)
{
}

static void
redmond_style_class_init (RedmondStyleClass *klass)
{
  GtkStyleClass *style_class = GTK_STYLE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  style_class->draw_arrow = draw_arrow;
  style_class->draw_box = draw_box;
  style_class->draw_check = draw_check;
  style_class->draw_option = draw_option;
  style_class->draw_tab = draw_tab;
}

GType redmond_type_style = 0;

void
redmond_style_register_type (GTypeModule *module)
{
  static const GTypeInfo object_info =
  {
    sizeof (RedmondStyleClass),
    (GBaseInitFunc) NULL,
    (GBaseFinalizeFunc) NULL,
    (GClassInitFunc) redmond_style_class_init,
    NULL,           /* class_finalize */
    NULL,           /* class_data */
    sizeof (RedmondStyle),
    0,              /* n_preallocs */
    (GInstanceInitFunc) redmond_style_init,
  };
  
  redmond_type_style = g_type_module_register_type (module,
						   GTK_TYPE_STYLE,
						   "Redmond95Style",
						   &object_info, 0);
}
