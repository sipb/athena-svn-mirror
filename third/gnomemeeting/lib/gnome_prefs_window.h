
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
 *                         gnome_prefs_window.c  -  description 
 *                         ------------------------------------
 *   begin                : Mon Oct 15 2003, but based on older code
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Helpers to create GNOME compliant prefs windows.
 *
 */

#include <gconf/gconf-client.h>
#include <gtk/gtk.h>

#include <string.h>

G_BEGIN_DECLS


/* Common notice 
 *
 * The created widgets are associated to a GConf key. They have the value
 * of the GConf key as initial value and they get updated when the GConf
 * value changes.
 *
 * You have to create a prefs window with gnome_prefs_window_new. You
 * can create categories of options with gnome_prefs_window_section_new
 * and subcategories with gnome_prefs_window_subsection_new. You can fill in
 * those subcategories by blocks of options using gnome_prefs_subsection_new
 * and then add entries, toggles and such to those blocks using
 * the functions below.
 */


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a GtkEntry associated with a GConf key and returns
 *                 the result.
 *                 The first parameter is the section in which 
 *                 the GtkEntry should be attached. The other parameters are
 *                 the text label, the GConf key, the tooltip, the row where
 *                 to attach it in the section, and if the label and GtkEntry
 *                 should be packed together or aligned with others in the
 *                 section they belong to.
 * PRE          :  /
 */
GtkWidget *gnome_prefs_entry_new (GtkWidget *,
				  gchar *,
				  gchar *,
				  gchar *,
				  int,
				  gboolean);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a GtkToggleButton associated with a GConf key and
 *                 returns the result.
 *                 The first parameter is the section in which the 
 *                 GtkToggleButton should be attached. The other parameters are
 *                 the text label, the GConf key, the tooltip, the row where
 *                 to attach it in the section.
 * PRE          :  /
 */
GtkWidget *gnome_prefs_toggle_new (GtkWidget *,
				   gchar *,
				   gchar *, 
				   gchar *,
				   int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a GtkSpinButton associated with a GConf key and
 *                 returns the result.
 *                 The first parameter is the section in which 
 *                 the GtkSpinButton should be attached. The other parameters
 *                 are the text label, the GConf key, the tooltip, the
 *                 minimal and maximal values, the incrementation step,
 *                 the row where to attach it in the section, 
 *                 the rest of the label, if any, and if the label and widget
 *                 should be packed together or aligned with others in the
 *                 section they belong to. 
 * PRE          :  The gboolean must be TRUE if the rest of the label is given.
 */
GtkWidget *gnome_prefs_spin_new (GtkWidget *,
				 gchar *,
				 gchar *,
				 gchar *,
				 double,
				 double,
				 double,
				 int,
				 gchar *,
				 gboolean);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a range with 2 GtkSpinButtons associated with two
 *                 GConf keys.
 *                 The first parameter is the section in which 
 *                 the GtkSpinButton should be attached. The other parameters
 *                 are the first part of the label, a pointer that will be
 *                 updated to point to the first GtkSpinButton, the second
 *                 part of the text label, a pointer that will be updated
 *                 to point to the second GtkSpinButton, the third part
 *                 of the text label, the 2 GConf keys, the 2 tooltips, the
 *                 2 minimal, the 2 maximal values, the incrementation step,
 *                 the row where to attach it in the section.
 * PRE          :  /
 */
void gnome_prefs_range_new (GtkWidget *,
			    gchar *,
			    GtkWidget **,
			    gchar *,
			    GtkWidget **,
			    gchar *,
			    gchar *,
			    gchar *,
			    gchar *,
			    gchar *,
			    double,
			    double,
			    double,
			    double,
			    double,
			    int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a GtkOptionMenu associated with an integer GConf
 *                 key and returns the result.
 *                 The first parameter is the section in which 
 *                 the GtkEntry should be attached. The other parameters are
 *                 the text label, the possible values for the menu, the GConf
 *                 key, the tooltip, the row where to attach it in the section.
 * PRE          :  /
 */
GtkWidget *gnome_prefs_int_option_menu_new (GtkWidget *,
					    gchar *,
					    gchar **, 
					    gchar *,
					    gchar *,
					    int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a GtkOptionMenu associated with a string GConf
 *                 key and returns the result.
 *                 The first parameter is the section in which 
 *                 the GtkEntry should be attached. The other parameters are
 *                 the text label, the possible values for the menu, the GConf
 *                 key, the tooltip, the row where to attach it in the section.
 * PRE          :  The array ends with NULL. 
 */
GtkWidget *gnome_prefs_string_option_menu_new (GtkWidget *,
					       gchar *,
					       gchar **,
					       gchar *,
					       gchar *,
					       int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Updates the content of a GtkOptionMenu associated with
 *                 a string GConf key. The first parameter is the menu,
 *                 the second is the array of possible values, and the
 *                 last one is the gconf key. 
 * PRE          :  The array ends with NULL.
 */
void gnome_prefs_string_option_menu_update (GtkWidget *,
					    gchar **,
					    gchar *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a subsection inside a section of a prefs window.
 *                 The parameters are the prefs window, the section of the
 *                 prefs window in which the newly created subsection must
 *                 be added, the title of the frame, the number of rows
 *                 and of columns. Widgets can be attached to the returned
 *                 subsection.
 * PRE          :  /
 */
GtkWidget *gnome_prefs_subsection_new (GtkWidget *,
				       GtkWidget *,
				       gchar *,
				       int,
				       int);

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a new prefs window. The parameter is a filename
 *                 corresponding to the logo displayed by default. Returns
 *                 the created window which still has to be connected to the
 *                 signals.
 * PRE          :  /
 */
GtkWidget *gnome_prefs_window_new (gchar *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a new section in the given prefs window.
 *                 The parameter are the prefs window and the prefs
 *                 window section name.
 * PRE          :  /
 */
void gnome_prefs_window_section_new (GtkWidget *,
				     gchar *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a new subsection in the given prefs window and
 *                 returns it. The parameter are the prefs window and the
 *                 prefs window subsection name. General subsections can
 *                 be created in the returned gnome prefs window subsection
 *                 and widgets can be attached to them.
 * PRE          :  /
 */
GtkWidget *gnome_prefs_window_subsection_new (GtkWidget *,
					      gchar *);

G_END_DECLS
