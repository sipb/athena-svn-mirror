
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
 *                         gconf_widgets_extensions.h  -  description 
 *                         ------------------------------------------
 *   begin                : Fri Oct 17 2003, but based on older code
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Default callbacks and notifiers to run for GConf
 *                          managed generic widgets and convenience functions.
 *
 */

#include <gconf/gconf-client.h>
#include <gtk/gtk.h>

#include <string.h>

G_BEGIN_DECLS


/* Common notice
 *
 * This file provides a few generic signal handlers and notifiers for
 * GTK widgets associated with GConf keys. If you associate the good callback
 * and the good notifier to a widget, the notifier will update the widget
 * after having blocked the signal when the GConf key is modified and
 * the GConf key will be updated when the widget changes.
 *
 * Default notifiers and default callbacks are given for GtkEntry,
 * GtkAdjustment, GtkToggleButton, GtkOptionMenu associated with a string
 * GConf key and with an int GConf key.
 */


/* DESCRIPTION  :  This function is called when an entry is activated.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 entry.  
 * PRE          :  Non-Null data corresponding to the string gconf key
 *                 to modify.
 */
void entry_activate_changed (GtkWidget *,
                             gpointer);

/* DESCRIPTION  :  This function is called when the focus of an entry changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 entry.  
 * PRE          :  Non-Null data corresponding to the string gconf key
 *                 to modify.
 */
gboolean entry_focus_changed (GtkWidget *,
                              GdkEventFocus *,
                              gpointer);


/* DESCRIPTION  :  Generic notifiers for entries.
 *                 This callback is called when a specific key of
 *                 the gconf database associated with an entry changes.
 * BEHAVIOR     :  It updates the widget.
 * PRE          :  The GConf key triggering that notifier on modification
 *                 should be of type string.
 */
void entry_changed_nt (GConfClient *,
		       guint,
		       GConfEntry *,
		       gpointer);


/* DESCRIPTION  :  This function is called when a toggle changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 toggle.  
 * PRE          :  Non-Null data corresponding to the boolean GConf key to
 *                 modify.
 */
void toggle_changed (GtkCheckButton *,
		     gpointer);


/* DESCRIPTION  :  Generic notifiers for toggles.
 *                 This callback is called when a specific key of
 *                 the gconf database associated with a toggle changes, this
 *                 only updates the toggle.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  The GConf key triggering that notifier on modification
 *"                should be of type boolean.
 */
void toggle_changed_nt (GConfClient *,
			guint,
			GConfEntry *,
			gpointer);


/* DESCRIPTION  :  This function is called when an adjustment changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 adjustment.  
 * PRE          :  Non-Null data corresponding to the int GConf key to modify.
 */
void adjustment_changed (GtkAdjustment *,
			 gpointer);


/* DESCRIPTION  :  Generic notifiers for adjustments.
 *                 This callback is called when a specific key of
 *                 the gconf database associated with an adjustment changes.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  The GConf key triggering that notifier on modification
 *                 should be of type integer.
 */
void adjustment_changed_nt (GConfClient *,
			    guint,
			    GConfEntry *,
			    gpointer);


/* DESCRIPTION  :  This function is called when an int option menu changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 int option menu.  
 * PRE          :  Non-Null data corresponding to int the gconf key to modify.
 */

void int_option_menu_changed (GtkWidget *,
			      gpointer);


/* DESCRIPTION  :  Generic notifiers for int-based option menus.
 *                 This callback is called when a specific key of
 *                 the gconf database associated with an option menu changes,
 *                 it only updates the menu.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  The GConf key triggering that notifier on modifiction
 *                 should be of type integer.
 */
void int_option_menu_changed_nt (GConfClient *,
				 guint,
				 GConfEntry *,
				 gpointer);


/* DESCRIPTION  :  This function is called when a string option menu changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 string option menu.  
 * PRE          :  Non-Null data corresponding to the string gconf key to
 *                 modify.
 */
void string_option_menu_changed (GtkWidget *,
				 gpointer);


/* DESCRIPTION  :  Generic notifiers for string-based option_menus.
 *                 This callback is called when a specific key of
 *                 the gconf database associated with an option menu changes,
 *                 this only updates the menu.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  The GConf key triggering that notifier on modifiction
 *                 should be of type string.
 */
void string_option_menu_changed_nt (GConfClient *,
				    guint,
				    GConfEntry *,
				    gpointer);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the given GConf key to the given value.
 * PRE          :  /
 */
void gconf_set_bool (gchar *,
		     gboolean);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns the value for the given GConf key.
 * PRE          :  /
 */
gboolean gconf_get_bool (gchar *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the given GConf key to the given value.
 * PRE          :  /
 */
void gconf_set_string (gchar *,
		       gchar *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns the value for the given GConf key.
 * PRE          :  /
 */
gchar *gconf_get_string (gchar *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the given GConf key to the given value.
 * PRE          :  /
 */
void gconf_set_int (gchar *,
		    int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns the value for the given GConf key.
 * PRE          :  /
 */
int gconf_get_int (gchar *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the given GConf key to the given value.
 * PRE          :  /
 */
void gconf_set_float (gchar *,
                      float);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns the value for the given GConf key.
 * PRE          :  /
 */
int gconf_get_float (gchar *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the given GConf key to the given value.
 * PRE          :  /
 */
void gconf_set_string_list (gchar *,
			    GSList *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns the value for the given GConf key.
 * PRE          :  /
 */
GSList *gconf_get_string_list (gchar *);

G_END_DECLS
