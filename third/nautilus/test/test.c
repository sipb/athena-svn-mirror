#include "test.h"

#include <libart_lgpl/art_rgb.h>
#include <libnautilus-extensions/nautilus-preferences.h>

void
test_init (int *argc,
	   char ***argv)
{
	gtk_init (argc, argv);
	gdk_rgb_init ();
	gnome_vfs_init ();
}

void
test_quit (int exit_code)
{
	gnome_vfs_shutdown ();
	gtk_main_quit ();
}

void
test_delete_event (GtkWidget *widget,
		   GdkEvent *event,
		   gpointer callback_data)
{
	test_quit (0);
}

GtkWidget *
test_window_new (const char *title, guint border_width)
{
	GtkWidget *window;

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	
	if (title != NULL) {
		gtk_window_set_title (GTK_WINDOW (window), title);
	}

	gtk_signal_connect (GTK_OBJECT (window),
			    "delete_event",
			    GTK_SIGNAL_FUNC (test_delete_event),
			    NULL);
	
	gtk_window_set_policy (GTK_WINDOW (window), TRUE, TRUE, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (window), border_width);
	
	return window;
}

void
test_gtk_widget_set_background_image (GtkWidget *widget,
				      const char *image_name)
{
	NautilusBackground *background;
	char *uri;

	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (image_name != NULL);

	background = nautilus_get_widget_background (widget);
	
	uri = g_strdup_printf ("file://%s/%s", NAUTILUS_DATADIR, image_name);

	nautilus_background_set_image_uri (background, uri);

	g_free (uri);
}

void
test_gtk_widget_set_background_color (GtkWidget *widget,
				      const char *color_spec)
{
	NautilusBackground *background;

	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (color_spec != NULL);

	background = nautilus_get_widget_background (widget);
	
	nautilus_background_set_color (background, color_spec);
}

GdkPixbuf *
test_pixbuf_new_named (const char *name, float scale)
{
	GdkPixbuf *pixbuf;
	char *path;

	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (scale >= 0.0, NULL);

	if (name[0] == '/') {
		path = g_strdup (name);
	} else {
		path = g_strdup_printf ("%s/%s", NAUTILUS_DATADIR, name);
	}

	pixbuf = gdk_pixbuf_new_from_file (path);

	g_free (path);

	g_return_val_if_fail (pixbuf != NULL, NULL);
	
	if (scale != 1.0) {
		GdkPixbuf *scaled;
		float width = gdk_pixbuf_get_width (pixbuf) * scale;
		float height = gdk_pixbuf_get_width (pixbuf) * scale;

		scaled = gdk_pixbuf_scale_simple (pixbuf, width, height, GDK_INTERP_BILINEAR);

		gdk_pixbuf_unref (pixbuf);

		g_return_val_if_fail (scaled != NULL, NULL);

		pixbuf = scaled;
	}

	return pixbuf;
}

GtkWidget *
test_image_new (const char *pixbuf_name,
		const char *tile_name,
		float scale,
		gboolean with_background)
{
	GtkWidget *image;

	if (with_background) {
		image = nautilus_image_new_with_background (NULL);
	} else {
		image = nautilus_image_new (NULL);
	}

	if (pixbuf_name != NULL) {
		GdkPixbuf *pixbuf;

		pixbuf = test_pixbuf_new_named (pixbuf_name, scale);

		if (pixbuf != NULL) {
			nautilus_image_set_pixbuf (NAUTILUS_IMAGE (image), pixbuf);
			gdk_pixbuf_unref (pixbuf);
		}
	}

	if (tile_name != NULL) {
		GdkPixbuf *tile_pixbuf;

		tile_pixbuf = test_pixbuf_new_named (tile_name, 1.0);

		if (tile_pixbuf != NULL) {
			nautilus_image_set_tile_pixbuf (NAUTILUS_IMAGE (image), tile_pixbuf);
			gdk_pixbuf_unref (tile_pixbuf);
		}
	}

	return image;
}

GtkWidget *
test_label_new (const char *text,
		const char *tile_name,
		gboolean with_background,
		int num_sizes_larger)
{
	GtkWidget *label;

	if (text == NULL) {
		text = "Foo";
	}
	
	if (with_background) {
		label = nautilus_label_new_with_background (text);
	} else {
		label = nautilus_label_new (text);
	}

	if (num_sizes_larger < 0) {
		nautilus_label_make_smaller (NAUTILUS_LABEL (label), ABS (num_sizes_larger));
	} else if (num_sizes_larger > 0) {
		nautilus_label_make_larger (NAUTILUS_LABEL (label), num_sizes_larger);
	}

	if (tile_name != NULL) {
		GdkPixbuf *tile_pixbuf;

		tile_pixbuf = test_pixbuf_new_named (tile_name, 1.0);

		if (tile_pixbuf != NULL) {
			nautilus_label_set_tile_pixbuf (NAUTILUS_LABEL (label), tile_pixbuf);
			gdk_pixbuf_unref (tile_pixbuf);
		}
	}

	return label;
}

static void
rgba_run_alpha (art_u8 *buf, art_u8 r, art_u8 g, art_u8 b, int alpha, int n)
{
  int i;
  int v;

  for (i = 0; i < n; i++)
    {
      v = *buf;
      *buf++ = v + (((r - v) * alpha + 0x80) >> 8);
      v = *buf;
      *buf++ = v + (((g - v) * alpha + 0x80) >> 8);
      v = *buf;
      *buf++ = v + (((b - v) * alpha + 0x80) >> 8);

      *buf++ = 255;
    }
}

typedef void (*FillRunCallback) (art_u8 *buf, art_u8 r, art_u8 g, art_u8 b, int alpha, int n);

/* This function is totally broken. 
 * Amongst other interesting things it will write outside
 * the pixbufs pixels.
 */
static void
pixbuf_draw_rectangle (GdkPixbuf *pixbuf,
		       const ArtIRect *rectangle,
		       guint32 color,
		       gboolean filled)
{
	guchar r;
	guchar g;
	guchar b;
	guchar opacity;

	guint width;
	guint height;
	guchar *pixels;
	guint rowstride;
 	int y;
	gboolean has_alpha;
	guint pixel_offset;
	guchar *offset;

	guint rect_width;
	guint rect_height;

	ArtIRect draw_area;

	FillRunCallback fill_run_callback;

	g_return_if_fail (pixbuf != NULL);

	width = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);
	pixels = gdk_pixbuf_get_pixels (pixbuf);
	rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);
	pixel_offset = has_alpha ? 4 : 3;

	r = NAUTILUS_RGBA_COLOR_GET_R (color);
	g = NAUTILUS_RGBA_COLOR_GET_G (color);
	b = NAUTILUS_RGBA_COLOR_GET_B (color);
	opacity = NAUTILUS_RGBA_COLOR_GET_A (color);

	fill_run_callback = has_alpha ? rgba_run_alpha : art_rgb_run_alpha;

	if (rectangle != NULL) {
		g_return_if_fail (rectangle->x1 >  rectangle->x0);
		g_return_if_fail (rectangle->y1 >  rectangle->y0);
		
		rect_width = rectangle->x1 - rectangle->x0;
		rect_height = rectangle->y1 - rectangle->y0;

		draw_area = *rectangle;
	}
	else {
		rect_width = width;
		rect_height = height;

		draw_area.x0 = 0;
		draw_area.y0 = 0;
		draw_area.x1 = width;
		draw_area.y1 = height;
	}

	if (filled) {
		offset = pixels + (draw_area.y0 * rowstride) + (draw_area.x0 * pixel_offset);

		for (y = draw_area.y0; y < draw_area.y1; y++) {
			(* fill_run_callback) (offset, r, g, b, opacity, rect_width);
			offset += rowstride;
		}
	}
	else {
		/* top */
		offset = pixels + (draw_area.y0 * rowstride) + (draw_area.x0 * pixel_offset);
		(* fill_run_callback) (offset, r, g, b, opacity, rect_width);
		
		/* bottom */
		offset += ((rect_height - 1) * rowstride);
		(* fill_run_callback) (offset, r, g, b, opacity, rect_width);
	
		for (y = draw_area.y0 + 1; y < (draw_area.y1 - 1); y++) {
			/* left */
			offset = pixels + (y * rowstride) + (draw_area.x0 * pixel_offset);
			(* fill_run_callback) (offset, r, g, b, opacity, 1);
			
			/* right */
			offset += (rect_width - 1) * pixel_offset;
			(* fill_run_callback) (offset, r, g, b, opacity, 1);
		}
	}
}

void
test_pixbuf_draw_rectangle (GdkPixbuf *pixbuf,
			    int x0,
			    int y0,
			    int x1,
			    int y1,
			    int inset,
			    gboolean filled,
			    guint32 color,
			    int opacity)
{

	g_return_if_fail (nautilus_gdk_pixbuf_is_valid (pixbuf));
 	g_return_if_fail (opacity > NAUTILUS_OPACITY_FULLY_TRANSPARENT);
 	g_return_if_fail (opacity <= NAUTILUS_OPACITY_FULLY_OPAQUE);

	color = NAUTILUS_RGBA_COLOR_PACK (NAUTILUS_RGBA_COLOR_GET_R (color),
					  NAUTILUS_RGBA_COLOR_GET_G (color),
					  NAUTILUS_RGBA_COLOR_GET_B (color),
					  opacity);
	
	if (x0 == -1 && y0 == -1 && x1 == -1 && y1 == -1) {
		pixbuf_draw_rectangle (pixbuf, NULL, color, filled);
	} else {
		ArtIRect rect;

		g_return_if_fail (x0 >= 0);
		g_return_if_fail (y0 >= 0);
		g_return_if_fail (x1 > x0);
		g_return_if_fail (y1 > y0);
	
		rect.x0 = x0;
		rect.y0 = y0;
		rect.x1 = x1;
		rect.y1 = y1;
		
		rect.x0 += inset;
		rect.y0 += inset;
		rect.x1 -= inset;
		rect.y1 -= inset;
		
		g_return_if_fail (!art_irect_empty (&rect));
		
		pixbuf_draw_rectangle (pixbuf, &rect, color, filled);
	}
}

void
test_pixbuf_draw_rectangle_tiled (GdkPixbuf *pixbuf,
				  const char *tile_name,
				  int x0,
				  int y0,
				  int x1,
				  int y1,
				  int opacity)
{
	ArtIRect area;
	GdkPixbuf *tile_pixbuf;

	g_return_if_fail (nautilus_gdk_pixbuf_is_valid (pixbuf));
	g_return_if_fail (tile_name != NULL);
 	g_return_if_fail (opacity > NAUTILUS_OPACITY_FULLY_TRANSPARENT);
 	g_return_if_fail (opacity <= NAUTILUS_OPACITY_FULLY_OPAQUE);

	tile_pixbuf = test_pixbuf_new_named (tile_name, 1.0);

 	g_return_if_fail (tile_pixbuf != NULL);

	if (x0 == -1 && y0 == -1 && x1 == -1 && y1 == -1) {
		area = nautilus_gdk_pixbuf_get_frame (pixbuf);
	} else {
		g_return_if_fail (x0 >= 0);
		g_return_if_fail (y0 >= 0);
		g_return_if_fail (x1 > x0);
		g_return_if_fail (y1 > y0);

		area.x0 = x0;
		area.y0 = y0;
		area.x1 = x1;
		area.y1 = y1;
	}
	
	nautilus_gdk_pixbuf_draw_to_pixbuf_tiled (tile_pixbuf,
						  pixbuf,
						  &area,
						  gdk_pixbuf_get_width (tile_pixbuf),
						  gdk_pixbuf_get_height (tile_pixbuf),
						  x0,
						  y0,
						  opacity,
						  GDK_INTERP_NEAREST);

	gdk_pixbuf_unref (tile_pixbuf);
}

/* Preferences hacks */
void
test_text_caption_set_text_for_int_preferences (NautilusTextCaption *text_caption,
					 const char *name)
{
	int int_value;
	char *text;

	g_return_if_fail (NAUTILUS_IS_TEXT_CAPTION (text_caption));
	g_return_if_fail (name != NULL);
	
	int_value = nautilus_preferences_get_integer (name);

	text = g_strdup_printf ("%d", int_value);

	nautilus_text_caption_set_text (NAUTILUS_TEXT_CAPTION (text_caption), text);

	g_free (text);
}

void
test_text_caption_set_text_for_string_preferences (NautilusTextCaption *text_caption,
						   const char *name)
{
	char *text;

	g_return_if_fail (NAUTILUS_IS_TEXT_CAPTION (text_caption));
	g_return_if_fail (name != NULL);
	
	text = nautilus_preferences_get (name);
	
	nautilus_text_caption_set_text (NAUTILUS_TEXT_CAPTION (text_caption), text);

	g_free (text);
}

void
test_text_caption_set_text_for_default_int_preferences (NautilusTextCaption *text_caption,
						 const char *name)
{
	int int_value;
	char *text;
	
	g_return_if_fail (NAUTILUS_IS_TEXT_CAPTION (text_caption));
	g_return_if_fail (name != NULL);
	
	int_value = nautilus_preferences_default_get_integer (name, nautilus_preferences_get_user_level ());

	text = g_strdup_printf ("%d", int_value);

	nautilus_text_caption_set_text (NAUTILUS_TEXT_CAPTION (text_caption), text);

	g_free (text);
}

void
test_text_caption_set_text_for_default_string_preferences (NautilusTextCaption *text_caption,
							   const char *name)
{
	char *text;
	
	g_return_if_fail (NAUTILUS_IS_TEXT_CAPTION (text_caption));
	g_return_if_fail (name != NULL);
	
	text = nautilus_preferences_default_get_string (name, nautilus_preferences_get_user_level ());

	nautilus_text_caption_set_text (NAUTILUS_TEXT_CAPTION (text_caption), text);

	g_free (text);
}

int
test_text_caption_get_text_as_int (const NautilusTextCaption *text_caption)
{
	int result = 0;
	char *text;

	g_return_val_if_fail (NAUTILUS_IS_TEXT_CAPTION (text_caption), 0);

	text = nautilus_text_caption_get_text (text_caption);

	nautilus_eat_str_to_int (text, &result);

	return result;
}

void 
test_window_set_title_with_pid (GtkWindow *window,
				const char *title)
{
	char *tmp;
	
	g_return_if_fail (GTK_IS_WINDOW (window));

	tmp = g_strdup_printf ("%d: %s", getpid (), title);
	gtk_window_set_title (GTK_WINDOW (window), tmp);
	g_free (tmp);
}
