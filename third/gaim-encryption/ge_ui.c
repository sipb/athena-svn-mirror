#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gtk/gtkplug.h>

#include "ge_ui.h"

void GE_ui_error(const char* err) {
   GtkWidget *dialog, *label;
   
   /* Create the widgets */
   
   dialog = gtk_dialog_new_with_buttons ("Gaim-Encryption Error", 0, 0,
                                         GTK_STOCK_OK,
                                         GTK_RESPONSE_NONE,
                                         NULL);
   label = gtk_label_new (err);
   
   g_signal_connect_swapped (GTK_OBJECT (dialog), 
                             "response", 
                             G_CALLBACK (gtk_widget_destroy),
                             GTK_OBJECT (dialog));

   gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
                      label);
   gtk_widget_show_all (dialog);
}
