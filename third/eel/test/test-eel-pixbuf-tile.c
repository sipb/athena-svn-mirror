#include "test.h"

#include <eel/eel-debug-drawing.h>

static const char pixbuf_name[] = DATADIR "/pixmaps/gnome-globe.png";
static const char tile_name[] = DATADIR "/nautilus/patterns/camouflage.png";

static GdkPixbuf *global_buffer = NULL;

static void
destroy_global_buffer (void)
{
	if (global_buffer != NULL) {
		g_object_unref (global_buffer);
		global_buffer = NULL;
	}
}

static GdkPixbuf *
get_global_buffer (int minimum_width, int minimum_height)
{
	static gboolean at_exit_deallocator_installed = FALSE;

	g_return_val_if_fail (minimum_width > 0, NULL);
	g_return_val_if_fail (minimum_height > 0, NULL);

	if (global_buffer != NULL) {
		if (gdk_pixbuf_get_width (global_buffer) >= minimum_width
		    && gdk_pixbuf_get_height (global_buffer) >= minimum_height) {
			return global_buffer;
		}

		destroy_global_buffer ();
	}

	g_assert (global_buffer == NULL);

	global_buffer = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, 
					minimum_width, minimum_height);


	if (at_exit_deallocator_installed == FALSE) {
		at_exit_deallocator_installed = TRUE;
		eel_debug_call_at_shutdown (destroy_global_buffer);
	}

	return global_buffer;
}

static int
pixbuf_drawing_area_expose_event (GtkWidget *widget,
				  GdkEventExpose *event,
				  gpointer callback_data)
{
	static GdkPixbuf *tile = NULL;
	GdkPixbuf *buffer;
	ArtIRect dest;
	ArtIRect tile_area;

	buffer = get_global_buffer (widget->allocation.width,
				    widget->allocation.height);

	if (tile == NULL) {
		tile = gdk_pixbuf_new_from_file (tile_name, NULL);
		g_assert (tile != NULL);
	}

	tile_area.x0 = 0;
	tile_area.y0 = 0;
	tile_area.x1 = widget->allocation.width;
	tile_area.y1 = widget->allocation.height;

	eel_gdk_pixbuf_draw_to_pixbuf_tiled (tile,
					     buffer,
					     tile_area,
					     gdk_pixbuf_get_width (tile),
					     gdk_pixbuf_get_height (tile),
					     0,
					     0,
					     EEL_OPACITY_FULLY_OPAQUE,
					     GDK_INTERP_NEAREST);

	dest = eel_gtk_widget_get_bounds (widget);
	eel_gdk_pixbuf_draw_to_drawable (buffer,
					 widget->window,
					 widget->style->white_gc,
					 0,
					 0,
					 dest,
					 GDK_RGB_DITHER_NONE,
					 GDK_PIXBUF_ALPHA_BILEVEL,
					 EEL_STANDARD_ALPHA_THRESHHOLD);

	eel_debug_draw_rectangle_and_cross (widget->window, dest, 0xFF0000, TRUE);
	{
		ArtIRect one_tile;
		one_tile.x0 = widget->allocation.x;
		one_tile.y0 = widget->allocation.y;
		one_tile.x1 = gdk_pixbuf_get_width (tile);
		one_tile.y1 = gdk_pixbuf_get_height (tile);
		
		eel_debug_draw_rectangle_and_cross (widget->window, one_tile, 0x0000FF, TRUE);
	}

	return TRUE;
}

static int
drawable_drawing_area_expose_event (GtkWidget *widget,
				    GdkEventExpose *event,
				    gpointer callback_data)
{
	static GdkPixbuf *tile = NULL;
	ArtIRect dest;

	if (tile == NULL) {
		tile = gdk_pixbuf_new_from_file (tile_name, NULL);
		g_assert (tile != NULL);
	}

	dest = eel_gtk_widget_get_bounds (widget);
	eel_gdk_pixbuf_draw_to_drawable_tiled (tile,
					       widget->window,
					       widget->style->white_gc,
					       dest,
					       gdk_pixbuf_get_width (tile),
					       gdk_pixbuf_get_height (tile),
					       0,
					       0,
					       GDK_RGB_DITHER_NONE,
					       GDK_PIXBUF_ALPHA_BILEVEL,
					       EEL_STANDARD_ALPHA_THRESHHOLD);
	
	eel_debug_draw_rectangle_and_cross (widget->window, dest, 0xFF0000, TRUE);
	{
		ArtIRect one_tile;
		one_tile.x0 = widget->allocation.x;
		one_tile.y0 = widget->allocation.y;
		one_tile.x1 = gdk_pixbuf_get_width (tile);
		one_tile.y1 = gdk_pixbuf_get_height (tile);
		
		eel_debug_draw_rectangle_and_cross (widget->window, one_tile, 0x0000FF, TRUE);
	}

	return TRUE;
}

int 
main (int argc, char* argv[])
{
	GtkWidget *pixbuf_window;
	GtkWidget *pixbuf_drawing_area;
	GtkWidget *pixbuf_vbox;
	GtkWidget *drawable_window;
	GtkWidget *drawable_drawing_area;
	GtkWidget *drawable_vbox;
	
	test_init (&argc, &argv);

	pixbuf_window = test_window_new ("Pixbuf To Pixbuf Tile Test", 0);
	pixbuf_vbox = gtk_vbox_new (FALSE, 0);
	pixbuf_drawing_area = gtk_drawing_area_new ();
	g_signal_connect (pixbuf_drawing_area,
			    "expose_event",
			    G_CALLBACK (pixbuf_drawing_area_expose_event),
			    NULL);
	gtk_box_pack_start (GTK_BOX (pixbuf_vbox), pixbuf_drawing_area, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (pixbuf_window), pixbuf_vbox);
	gtk_widget_show_all (pixbuf_window);


	drawable_window = test_window_new ("Pixbuf To Drawable Tile Test", 0);
	drawable_vbox = gtk_vbox_new (FALSE, 0);
	drawable_drawing_area = gtk_drawing_area_new ();
	g_signal_connect (drawable_drawing_area,
			    "expose_event",
			    G_CALLBACK (drawable_drawing_area_expose_event),
			    NULL);
	gtk_box_pack_start (GTK_BOX (drawable_vbox), drawable_drawing_area, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (drawable_window), drawable_vbox);
	gtk_widget_show_all (drawable_window);

	gtk_main ();

	return 0;
}
