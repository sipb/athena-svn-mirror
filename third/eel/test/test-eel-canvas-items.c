#include <config.h>

#include <math.h>
#include <eel/eel-canvas-rect.h>
#include <libgnomecanvas/libgnomecanvas.h>
#include <gtk/gtk.h>

static gboolean
quit_cb (GtkWidget *widget, GdkEventAny *event, gpointer dummy)
{
	gtk_main_quit ();

	return TRUE;
}

static void
zoom_changed (GtkAdjustment *adj, gpointer data)
{
	gnome_canvas_set_pixels_per_unit (data, adj->value);
}

static gint
item_event (GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
	static double x, y;
	double new_x, new_y;
	GdkCursor *fleur;
	static int dragging;
	double item_x, item_y;

	/* set item_[xy] to the event x,y position in the parent's item-relative coordinates */
	item_x = event->button.x;
	item_y = event->button.y;
	gnome_canvas_item_w2i (item->parent, &item_x, &item_y);

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			if (event->button.state & GDK_SHIFT_MASK)
				gtk_object_destroy (GTK_OBJECT (item));
			else {
				x = item_x;
				y = item_y;

				fleur = gdk_cursor_new (GDK_FLEUR);
				gnome_canvas_item_grab (item,
							GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK,
							fleur,
							event->button.time);
				g_object_unref (fleur);
				dragging = TRUE;
			}
			break;

		case 2:
			if (event->button.state & GDK_SHIFT_MASK)
				gnome_canvas_item_lower_to_bottom (item);
			else
				gnome_canvas_item_lower (item, 1);
			break;

		case 3:
			if (event->button.state & GDK_SHIFT_MASK)
				gnome_canvas_item_raise_to_top (item);
			else
				gnome_canvas_item_raise (item, 1);
			break;

		default:
			break;
		}

		break;

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			new_x = item_x;
			new_y = item_y;

			gnome_canvas_item_move (item, new_x - x, new_y - y);
			x = new_x;
			y = new_y;
		}
		break;

	case GDK_BUTTON_RELEASE:
		gnome_canvas_item_ungrab (item, event->button.time);
		dragging = FALSE;
		break;

	default:
		break;
	}

	return FALSE;
}

static void
setup_item (GnomeCanvasItem *item)
{
	g_signal_connect (item, "event",
			  G_CALLBACK (item_event), NULL);
}


static void
setup_rectangles (GnomeCanvasGroup *root)
{
	setup_item (gnome_canvas_item_new (root,
					   EEL_TYPE_CANVAS_RECT,
					   "x1", 100.0,
					   "y1", 100.0,
					   "x2", 200.0,
					   "y2", 200.0,
					   "outline_color_rgba", 0x00ff00ff,
					   "fill_color_rgba", 0x0000ff80,
					   "width_pixels", 3,
					   NULL));
	
	setup_item (gnome_canvas_item_new (root,
					   EEL_TYPE_CANVAS_RECT,
					   "x1", 40.0,
					   "y1", 60.0,
					   "x2", 120.0,
					   "y2", 270.0,
					   "outline_color_rgba", 0x4400ffff,
					   "fill_color_rgba", 0xff000080,
					   "width_pixels", 3,
					   NULL));
	
	setup_item (gnome_canvas_item_new (root,
					   EEL_TYPE_CANVAS_RECT,
					   "x1", 140.0,
					   "y1", 160.0,
					   "x2", 170.0,
					   "y2", 240.0,
					   "outline_color_rgba", 0x4400ffff,
					   "fill_color_rgba", 0x00ff0080,
					   "width_pixels", 3,
					   NULL));
}


static GtkWidget *
create_canvas_items (void)
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *table;
	GtkWidget *w;
	GtkWidget *frame;
	GtkWidget *canvas;
	GtkAdjustment *adj;
	GnomeCanvasGroup *root;

	vbox = gtk_vbox_new (FALSE, 4);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 4);
	gtk_widget_show (vbox);

	w = gtk_label_new ("Drag an item with button 1.  Click button 2 on an item to lower it,\n"
			   "or button 3 to raise it.  Shift+click with buttons 2 or 3 to send\n"
			   "an item to the bottom or top, respectively.");
	gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, FALSE, 0);
	gtk_widget_show (w);

	hbox = gtk_hbox_new (FALSE, 4);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show (hbox);

	/* Create the canvas */

	canvas = gnome_canvas_new ();

	/* Setup canvas items */

	root = gnome_canvas_root (GNOME_CANVAS (canvas));

	setup_rectangles (root);

	/* Zoom */

	w = gtk_label_new ("Zoom:");
	gtk_box_pack_start (GTK_BOX (hbox), w, FALSE, FALSE, 0);
	gtk_widget_show (w);

	adj = GTK_ADJUSTMENT (gtk_adjustment_new (1.00, 0.05, 5.00, 0.05, 0.50, 0.50));
	g_signal_connect (adj, "value_changed",
			  G_CALLBACK (zoom_changed), canvas);
	w = gtk_spin_button_new (adj, 0.0, 2);
	gtk_widget_set_size_request (w, 50, 0);
	gtk_box_pack_start (GTK_BOX (hbox), w, FALSE, FALSE, 0);
	gtk_widget_show (w);

	/* Layout the stuff */

	table = gtk_table_new (2, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), 4);
	gtk_table_set_col_spacings (GTK_TABLE (table), 4);
	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);
	gtk_widget_show (table);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_table_attach (GTK_TABLE (table), frame,
			  0, 1, 0, 1,
			  GTK_EXPAND | GTK_FILL | GTK_SHRINK,
			  GTK_EXPAND | GTK_FILL | GTK_SHRINK,
			  0, 0);
	gtk_widget_show (frame);

	gtk_widget_set_size_request (canvas, 600, 450);
	gnome_canvas_set_scroll_region (GNOME_CANVAS (canvas), 0, 0, 600, 450);
	gtk_container_add (GTK_CONTAINER (frame), canvas);
	gtk_widget_show (canvas);

#if 0
	gtk_signal_connect_after (GTK_OBJECT (canvas), "key_press_event",
				  (GtkSignalFunc) key_press,
				  NULL);
#endif

	w = gtk_hscrollbar_new (GTK_LAYOUT (canvas)->hadjustment);
	gtk_table_attach (GTK_TABLE (table), w,
			  0, 1, 1, 2,
			  GTK_EXPAND | GTK_FILL | GTK_SHRINK,
			  GTK_FILL,
			  0, 0);
	gtk_widget_show (w);

	w = gtk_vscrollbar_new (GTK_LAYOUT (canvas)->vadjustment);
	gtk_table_attach (GTK_TABLE (table), w,
			  1, 2, 0, 1,
			  GTK_FILL,
			  GTK_EXPAND | GTK_FILL | GTK_SHRINK,
			  0, 0);
	gtk_widget_show (w);

	GTK_WIDGET_SET_FLAGS (canvas, GTK_CAN_FOCUS);
	gtk_widget_grab_focus (canvas);

	return vbox;
}



static void
create_canvas (void)
{
	GtkWidget *app;

	app = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable (GTK_WINDOW (app), TRUE);

	g_signal_connect (app, "delete_event",
			  G_CALLBACK (quit_cb), NULL);

	gtk_container_add (GTK_CONTAINER (app),
			   create_canvas_items ());

	gtk_widget_show (app);
}

int
main (int argc, char *argv[])
{
	gtk_init (&argc, &argv);

	create_canvas ();

	gtk_main ();

	return 0;
}
