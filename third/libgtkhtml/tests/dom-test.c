#include <gtk/gtk.h>
#include <gtkhtml.h>
#include "dom-test-window.h"

gint
main (gint argc, gchar **argv)
{
	GtkWidget *main_window;
	
	gtk_init (&argc, &argv);

	main_window = dom_test_window_new ();
	gtk_window_set_default_size (GTK_WINDOW (main_window), 600, 400);
	
	gtk_widget_show_all (main_window);
	gtk_main ();
	
	return 0;
}
