#include <gtk/gtk.h>
#include <libgnomeprint/gnome-print-config.h>
#include <libgnomeprintui/gnome-print-paper-selector.h>

int
main (int argc, char * argv[])
{
	GnomePrintConfig *config;
	GtkWidget *ps;
	GtkWidget *dialog;

	gtk_init (&argc, (char ***) &argv);

	config = gnome_print_config_default ();
	ps = gnome_paper_selector_new (config);
	dialog = gtk_dialog_new ();
	gtk_box_pack_start_defaults (GTK_BOX (GTK_DIALOG (dialog)->vbox), ps);
	gtk_widget_show (ps);
	gtk_widget_show (dialog);

	gtk_timeout_add (2000, (GSourceFunc) gtk_main_quit, NULL);
	gtk_main ();

	if (!gnome_print_config_set (config, "Printer", "PDF"))
		g_assert_not_reached ();

	gtk_timeout_add (2000, (GSourceFunc) gtk_main_quit, NULL);
	gtk_main ();
	
	return 0;
}
