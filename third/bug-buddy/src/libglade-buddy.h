#ifndef LIBGLADE_BUDDY_H
#define LIBGLADE_BUDDY_H

#include <gtk/gtkbutton.h>

/* this file should include all of the definitions for libglade */

/* libglade callbacks */
gint delete_me (GtkWidget *w, GdkEventAny *evt, gpointer data);
void update_selected_row      (GtkWidget *widget, gpointer data);
void on_version_apply_clicked (GtkButton *button, gpointer data);
void on_file_radio_toggled    (GtkWidget *radio, gpointer data);
GtkWidget *make_anim (gchar *widget_name, gchar *string1, 
		      gchar *string2, gint int1, gint int2);
GtkWidget *make_pixmap_button (gchar *widget_name, gchar *string1, 
			       gchar *string2, gint int1, gint int2);
GtkWidget *make_image (char *widget_name, char *s1, char *s2, int i1, int i2);

void on_gdb_go_clicked (GtkWidget *w, gpointer data);
void on_gdb_stop_clicked (GtkWidget *w, gpointer data);
void on_gdb_copy_clicked (GtkWidget *w, gpointer data);
void on_gdb_save_clicked (GtkWidget *w, gpointer data);

void on_druid_help_clicked   (GtkWidget *w, gpointer);
void on_druid_about_clicked  (GtkWidget *w, gpointer);
void on_druid_prev_clicked   (GtkWidget *w, gpointer);
void on_druid_next_clicked   (GtkWidget *w, gpointer);
void on_druid_cancel_clicked (GtkWidget *w, gpointer);

GtkWidget *stock_pixmap_buddy (char *w, char *n, char *a, int b, int c);
void title_configure_size (GtkWidget *w, GtkAllocation *alloc, gpointer data);
void side_configure_size (GtkWidget *w, GtkAllocation *alloc, gpointer data);

void intro_page_changed (GtkWidget *w, gpointer data);

void on_druid_window_style_set (GtkWidget *w, GtkStyle *old_style, gpointer data);
void on_email_group_toggled (GtkWidget *w, gpointer data);
void on_email_default_radio_toggled (GtkWidget *w, gpointer data);

void on_debugging_options_button_clicked (GtkWidget *w, gpointer null);

void queue_download_restart (GtkWidget *w, gpointer data);
void on_progress_start_clicked (GtkWidget *w, gpointer data);
void on_progress_stop_clicked (GtkWidget *w, gpointer data);
void on_proxy_settings_clicked (GtkWidget *w, gpointer data);

void on_product_toggle_clicked (GtkWidget *w, gpointer data);

#endif /* LIBGLADE_BUDDY_H */
