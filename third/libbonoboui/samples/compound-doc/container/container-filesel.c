#include "config.h"
#include <gtk/gtkfilesel.h>
#include "container-filesel.h"
#include "container.h"

static void
cancel_cb (GtkWidget *caller, GtkWidget *fs)
{
	gtk_widget_destroy (fs);
}

void
container_request_file (SampleApp    *app,
			gboolean      save,
			GtkSignalFunc cb,
			gpointer      user_data)
{
	GtkWidget *fs;

	app->fileselection = fs =
	    gtk_file_selection_new ("Select file");

	if (save)
		gtk_file_selection_show_fileop_buttons (GTK_FILE_SELECTION (fs));
	else
		gtk_file_selection_hide_fileop_buttons (GTK_FILE_SELECTION (fs));

	g_signal_connect_data (G_OBJECT (GTK_FILE_SELECTION (fs)->ok_button),
			       "clicked", G_CALLBACK (cb), user_data,
			       NULL, 0);

	g_signal_connect_data (G_OBJECT (GTK_FILE_SELECTION (fs)->cancel_button),
			       "clicked", G_CALLBACK (cancel_cb), fs,
			       NULL, 0);

	gtk_window_set_modal (GTK_WINDOW (fs), TRUE);

	gtk_widget_show (fs);
}
