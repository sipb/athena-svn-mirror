
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         gconf_widgets_extensions.c  -  description 
 *                         ------------------------------------------
 *   begin                : Fri Oct 17 2003, but based on older code
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Default signals and notifiers to run for GConf
 *                          managed generic widgets and convenience functions.
 *
 */


#include "gconf_widgets_extensions.h"


/* GTK Callbacks */
/*
 * There are 2 callbacks, one modifying the gconf key, ie the GTK callback and 
 * the GConf notifier (_nt) updating the widget back when a GConf key changes.
 *
 */

void
entry_activate_changed (GtkWidget *w,
                        gpointer data)
{
  entry_focus_changed (w, NULL, data);
}


gboolean
entry_focus_changed (GtkWidget  *w,
                     GdkEventFocus *ev,
                     gpointer data)
{
  GConfClient *client = NULL; 
  gchar *key = NULL;
  gchar *current_value = NULL;
  
  client = gconf_client_get_default ();
  key = (gchar *) data;

  current_value = gconf_client_get_string (GCONF_CLIENT (client), key, NULL);

  if (!current_value 
      || strcmp (current_value, gtk_entry_get_text (GTK_ENTRY (w)))) {

    gconf_client_set_string (GCONF_CLIENT (client),
			     key,
			     gtk_entry_get_text (GTK_ENTRY (w)),
			     NULL);
  }
  g_free (current_value);

  return FALSE;
}


void
entry_changed_nt (GConfClient *client,
		  guint cid, 
		  GConfEntry *entry,
		  gpointer data)
{
  GtkWidget *e = NULL;
  gchar *current_value = NULL;
  
  if (entry->value->type == GCONF_VALUE_STRING) {

    gdk_threads_enter ();
  
    e = GTK_WIDGET (data);
    current_value = (gchar *) gconf_value_get_string (entry->value);

    if (current_value
	&& strcmp (current_value, gtk_entry_get_text (GTK_ENTRY (e)))) {

      g_signal_handlers_block_matched (G_OBJECT (e),
				       G_SIGNAL_MATCH_FUNC,
				       0, 0, NULL,
				       (gpointer) entry_focus_changed,
				       NULL);
      g_signal_handlers_block_matched (G_OBJECT (e),
				       G_SIGNAL_MATCH_FUNC,
				       0, 0, NULL,
				       (gpointer) entry_activate_changed,
				       NULL);
      gtk_entry_set_text (GTK_ENTRY (e), current_value);
      g_signal_handlers_unblock_matched (G_OBJECT (e),
					 G_SIGNAL_MATCH_FUNC,
					 0, 0, NULL,
					 (gpointer) entry_activate_changed,
					 NULL);
      g_signal_handlers_unblock_matched (G_OBJECT (e),
					 G_SIGNAL_MATCH_FUNC,
					 0, 0, NULL,
					 (gpointer) entry_focus_changed,
					 NULL);
    }

    gdk_threads_leave (); 
  }
}


void
toggle_changed (GtkCheckButton *but,
		gpointer data)
{
  GConfClient *client = NULL;
  gchar *key = NULL; 

  client = gconf_client_get_default ();
  key = (gchar *) data;

  if (gconf_client_get_bool (client, key, NULL)
      != gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (but)))
    gconf_client_set_bool (GCONF_CLIENT (client), key,
			   gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (but)),
			   NULL);
}


void
toggle_changed_nt (GConfClient *client,
		   guint cid, 
		   GConfEntry *entry,
		   gpointer data)
{
  GtkWidget *e = NULL;
  gboolean current_value = FALSE;
  
  if (entry->value->type == GCONF_VALUE_BOOL) {
   
    gdk_threads_enter ();
  
    e = GTK_WIDGET (data);

    /* We set the new value for the widget */
    current_value = gconf_value_get_bool (entry->value);

    g_signal_handlers_block_matched (G_OBJECT (e),
				     G_SIGNAL_MATCH_FUNC,
				     0, 0, NULL,
				     (gpointer) toggle_changed,
				     NULL);
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (e)) != current_value)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (e), current_value);
    g_signal_handlers_unblock_matched (G_OBJECT (e),
				       G_SIGNAL_MATCH_FUNC,
				       0, 0, NULL,
				       (gpointer) toggle_changed,
				       NULL);
    gdk_threads_leave (); 
  }
}


void
adjustment_changed (GtkAdjustment *adj,
		    gpointer data)
{
  GConfClient *client = NULL;
  gchar *key = NULL;

  client = gconf_client_get_default ();
  key = (gchar *) data;

  if (gconf_client_get_int (client, key, NULL) != (int) adj->value)
    gconf_client_set_int (GCONF_CLIENT (client), key, (int) adj->value, NULL);
}


void
adjustment_changed_nt (GConfClient *client,
		       guint cid, 
		       GConfEntry *entry,
		       gpointer data)
{
  GtkWidget *e = NULL;
  GtkAdjustment *s = NULL;
  gdouble current_value = 0.0;
  
  if (entry->value->type == GCONF_VALUE_INT) {
    
    gdk_threads_enter ();

    e = GTK_WIDGET (data);
    s = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (e));
      
    current_value = gconf_value_get_int (entry->value);

    g_signal_handlers_block_matched (G_OBJECT (e),
				     G_SIGNAL_MATCH_FUNC,
				     0, 0, NULL,
				     (gpointer) adjustment_changed,
				     NULL);
    if (gtk_adjustment_get_value (GTK_ADJUSTMENT (s)) != current_value)
      gtk_adjustment_set_value (GTK_ADJUSTMENT (s), current_value);
    g_signal_handlers_unblock_matched (G_OBJECT (e),
				       G_SIGNAL_MATCH_FUNC,
				       0, 0, NULL,
				       (gpointer) adjustment_changed,
				       NULL);
      
    gdk_threads_leave ();
  }
}


void
int_option_menu_changed (GtkWidget *menu,
			 gpointer data)
{
  GConfClient *client = NULL;
  gchar *key = NULL;
  GtkWidget *active_item = NULL;
  guint item_index = -1;
  
  client = gconf_client_get_default ();
  key = (gchar *) data;
  
  active_item = gtk_menu_get_active (GTK_MENU (menu));
  item_index = g_list_index (GTK_MENU_SHELL (GTK_MENU (menu))->children, 
			     active_item);

  if (gconf_client_get_int (client, key, NULL) != item_index)
    gconf_client_set_int (GCONF_CLIENT (client), key, item_index, NULL);
}


void
int_option_menu_changed_nt (GConfClient *client,
			    guint cid, 
			    GConfEntry *entry,
			    gpointer data)
{
  GtkWidget *e = NULL;
  gint current_value = 0;
  
  if (entry->value->type == GCONF_VALUE_INT) {
   
    gdk_threads_enter ();

    e = GTK_WIDGET (data);
    current_value = gconf_value_get_int (entry->value);

    g_signal_handlers_block_matched (G_OBJECT (e),
				     G_SIGNAL_MATCH_FUNC,
				     0, 0, NULL,
				     (gpointer) int_option_menu_changed,
				     NULL);
    if (current_value != gtk_option_menu_get_history (GTK_OPTION_MENU (e)))
	gtk_option_menu_set_history (GTK_OPTION_MENU (e), current_value);
    g_signal_handlers_block_matched (G_OBJECT (e),
				     G_SIGNAL_MATCH_FUNC,
				     0, 0, NULL,
				     (gpointer) int_option_menu_changed,
				     NULL);
    gdk_threads_leave ();
  }
}


void
string_option_menu_changed (GtkWidget *menu,
			    gpointer data)
{
  GtkWidget *active_item = NULL;
  const gchar *text = NULL;
  GConfClient *client = NULL;

  gchar *current_value = NULL;
  gchar *key = NULL;

  client = gconf_client_get_default ();
  key = (gchar *) data;


  active_item = gtk_menu_get_active (GTK_MENU (menu));
  if (!active_item)
    text = "";
  else
    text = gtk_label_get_text (GTK_LABEL (GTK_BIN (active_item)->child));

  current_value = gconf_client_get_string (client, key, NULL);

  if (text && current_value && strcmp (text, current_value))
    gconf_client_set_string (GCONF_CLIENT (client), key, text, NULL);
}


void
string_option_menu_changed_nt (GConfClient *client,
			       guint cid, 
			       GConfEntry *entry,
			       gpointer data)
{
  int cpt = 0;
  GtkWidget *e = NULL;
  GtkWidget *label = NULL;
  GList *glist = NULL;
  gpointer mydata;

  if (entry->value->type == GCONF_VALUE_STRING) {
   
    gdk_threads_enter ();

    e = GTK_WIDGET (data);
    
    /* We set the new value for the widget */
    glist = 
      g_list_first (GTK_MENU_SHELL (GTK_MENU (GTK_OPTION_MENU (e)->menu))->children);
    
    while ((mydata = g_list_nth_data (glist, cpt)) != NULL) {

      label = GTK_BIN (mydata)->child;
      if (label && !strcmp (gtk_label_get_text (GTK_LABEL (label)), 
			    gconf_value_get_string (entry->value)))
	break;
      cpt++; 
    } 

    g_signal_handlers_block_matched (G_OBJECT (e),
				     G_SIGNAL_MATCH_FUNC,
				     0, 0, NULL,
				     (gpointer) string_option_menu_changed,
				     NULL);
    if (gtk_option_menu_get_history (GTK_OPTION_MENU (data)) != cpt)
      gtk_option_menu_set_history (GTK_OPTION_MENU (data), cpt);
    g_signal_handlers_unblock_matched (G_OBJECT (e),
				       G_SIGNAL_MATCH_FUNC,
				       0, 0, NULL,
				       (gpointer) string_option_menu_changed,
				       NULL);
	 
    gdk_threads_leave ();
  }
}


void
gconf_set_bool (gchar *key,
		gboolean b)
{
  GConfClient *client = NULL;

  if (!key)
    return;

  client = gconf_client_get_default ();
  gconf_client_set_bool (client, key, b, NULL);
}


gboolean
gconf_get_bool (gchar *key)
{
  GConfClient *client = NULL;

  if (!key)
    return FALSE;

  client = gconf_client_get_default ();

  return gconf_client_get_bool (client, key, NULL);
}


void
gconf_set_string (gchar *key,
		  gchar *v)
{
  GConfClient *client = NULL;

  if (!key)
    return;

  client = gconf_client_get_default ();

  gconf_client_set_string (client, key, v, NULL);
}


gchar *
gconf_get_string (gchar *key)
{
  GConfClient *client = NULL;
 
  if (!key)
    return NULL;

  client = gconf_client_get_default ();

  return gconf_client_get_string (client, key, NULL);
}


void
gconf_set_int (gchar *key,
	       int v)
{
  GConfClient *client = NULL;
 
  if (!key)
    return;

  client = gconf_client_get_default ();

  gconf_client_set_int (client, key, v, NULL);
}


int
gconf_get_int (gchar *key)
{
  GConfClient *client = NULL;
 
  if (!key)
    return 0;

  client = gconf_client_get_default ();

  return gconf_client_get_int (client, key, NULL);
}


void
gconf_set_float (gchar *key,
                 float v)
{
  GConfClient *client = NULL;

  if (!key)
    return;

  client = gconf_client_get_default ();

  gconf_client_set_float (client, key, v, NULL);
}


int
gconf_get_float (gchar *key)
{
  GConfClient *client = NULL;

  if (!key)
    return 0;

  client = gconf_client_get_default ();

  return gconf_client_get_float (client, key, NULL);
}


void
gconf_set_string_list (gchar *key,
		       GSList *l)
{
  GConfClient *client = NULL;
 
  if (!key)
    return;

  client = gconf_client_get_default ();

  gconf_client_set_list (client, key, GCONF_VALUE_STRING, l, NULL);
}


GSList *
gconf_get_string_list (gchar *key)
{
  GConfClient *client = NULL;
 
  if (!key)
    return 0;

  client = gconf_client_get_default ();

  return gconf_client_get_list (client, key, GCONF_VALUE_STRING, NULL);
}
