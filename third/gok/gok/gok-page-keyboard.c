/* gok-page-keyboard.c
*
* Copyright 2004 Sun Microsystems, Inc.,
* Copyright 2004 University Of Toronto
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <gtk/gtk.h>
#include <glade/glade.h>
#include "gok-page-keyboard.h"
#include "gok-data.h"
#include "gok-log.h"
#include "gok-settings-dialog.h"

/* backup of the page data */
static GokComposeType save_compose_type;
static gchar *save_compose_filename = NULL;
static gchar *save_aux_kbd_dirname;

/* privates */
void gok_page_keyboard_initialize_compose_filename_entry (const char* file);
void gok_page_keyboard_initialize_aux_keyboard_dir_entry (const char* file);

/**
* gok_page_keyboard_initialize
* @pWindowSettings: Pointer to the settings dialog window.
*
* Initializes this page of the gok settings dialog. This must be called
* prior to any calls on this page.
*
* returns: TRUE if the page was properly initialized, FALSE if not.
**/
gboolean gok_page_keyboard_initialize (GladeXML* xml)
{
	GtkWidget* widget;
	
	g_assert (xml != NULL);
		
	/* store the current values */
	gok_page_keyboard_backup();

	/* update the controls */
	widget = glade_xml_get_widget (xml, "XkbComposeKeyboardRadiobutton");
	g_assert (widget != NULL);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), save_compose_type == GOK_COMPOSE_XKB);

	widget = glade_xml_get_widget (xml, "AlphaComposeKeyboardRadiobutton");
	g_assert (widget != NULL);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), save_compose_type == GOK_COMPOSE_ALPHA);

	widget = glade_xml_get_widget (xml, "AlphaFrequencyComposeKeyboardRadiobutton");
	g_assert (widget != NULL);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), save_compose_type == GOK_COMPOSE_ALPHAFREQ);

	widget = glade_xml_get_widget (xml, "XmlComposeKeyboardRadiobutton");
	g_assert (widget != NULL);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), save_compose_type == GOK_COMPOSE_CUSTOM);

	
	gok_page_keyboard_initialize_compose_filename_entry ((save_compose_type == GOK_COMPOSE_CUSTOM) ? 
							    gok_data_get_custom_compose_filename () : "");
	

 	gok_page_keyboard_initialize_aux_keyboard_dir_entry (gok_data_get_aux_keyboard_directory ());
	
	return TRUE;
}


/**
* gok_page_keyboard_apply
* 
* Updates the gok data with values from the controls.
*
* returns: TRUE if any of the gok data settings have changed, FALSE if not.
**/
gboolean gok_page_keyboard_apply ()
{
	GtkWidget* pWidget;
	gboolean bDataChanged;
	gchar* text;
	
	bDataChanged = FALSE;

	/* N.B. this page is already instant-apply; so 'Apply' really means 'sync saved values' */
	gok_page_keyboard_backup (); /* read the current gok-data values into our cache */

	return bDataChanged;
}

/**
* gok_page_keyboard_revert
* 
* Revert to the backup settings for this page.
*
* returns: TRUE if any of the settings have changed, FALSE 
* if they are all still the same.
**/
gboolean gok_page_keyboard_revert ()
{
	gok_data_set_compose_keyboard_type (save_compose_type);
	gok_data_set_custom_compose_filename (save_compose_filename);
	gok_data_set_aux_keyboard_directory (save_aux_kbd_dirname);
	gok_page_keyboard_initialize (gok_settingsdialog_get_xml ());

	return TRUE;
}

/**
* gok_page_keyboard_backup
* 
* Copies all the member settings to backup.
**/
void gok_page_keyboard_backup ()
{
	save_compose_type = gok_data_get_compose_keyboard_type ();
	if (save_compose_type == GOK_COMPOSE_CUSTOM) 
	    save_compose_filename = gok_data_get_custom_compose_filename ();
	save_aux_kbd_dirname = gok_data_get_aux_keyboard_directory ();
}

void 
gok_page_keyboard_update_custom_dir_from_control ()
{
	GtkWidget* pWidget; 
	gchar* text;
	
	gok_log_enter();
	pWidget = glade_xml_get_widget (gok_settingsdialog_get_xml(), "AuxKeyboardDirEntry");
	g_assert (pWidget != NULL);
	text = gnome_file_entry_get_full_path ( (GnomeFileEntry*)pWidget, FALSE);
	g_free (save_aux_kbd_dirname);
	save_aux_kbd_dirname = text;

	gok_log_leave();
}

void 
gok_page_keyboard_update_custom_compose_from_control ()
{
	GtkWidget* pWidget; 
	gchar* text;
	
	gok_log_enter();
	pWidget = glade_xml_get_widget (gok_settingsdialog_get_xml(), "XmlKeyboardGnomeFileEntry");
	g_assert (pWidget != NULL);
	text = gnome_file_entry_get_full_path ( (GnomeFileEntry*)pWidget, FALSE);
	g_free (save_compose_filename);
	save_compose_filename = text;

	gok_log_leave();
}

void 
gok_page_keyboard_initialize_aux_keyboard_dir_entry (const char* file)
{
	GtkWidget* pWidget; 
	
	gok_log_enter();
	pWidget = glade_xml_get_widget (gok_settingsdialog_get_xml(), "AuxKeyboardDirEntry");
	g_assert (pWidget != NULL);
	gnome_file_entry_set_modal ((GnomeFileEntry*)pWidget, FALSE);
	gnome_file_entry_set_title ((GnomeFileEntry*)pWidget, _("Enter directory to search for additional GOK keyboard files."));
	gnome_file_entry_set_directory_entry ((GnomeFileEntry*)pWidget, TRUE);
	gnome_file_entry_set_filename ( (GnomeFileEntry*)pWidget, file);
	gok_log_leave();
}
void 
gok_page_keyboard_initialize_compose_filename_entry (const char* file)
{
	GtkWidget* pWidget; 
	
	gok_log_enter();
	pWidget = glade_xml_get_widget (gok_settingsdialog_get_xml(), "XmlKeyboardGnomeFileEntry");
	g_assert (pWidget != NULL);
	gnome_file_entry_set_modal ((GnomeFileEntry*)pWidget, FALSE);
	gnome_file_entry_set_title ((GnomeFileEntry*)pWidget, _("Select the XML file defining your startup compose keyboard"));
	gnome_file_entry_set_directory_entry ((GnomeFileEntry*)pWidget, FALSE);
	gnome_file_entry_set_filename ( (GnomeFileEntry*)pWidget, file);
	gok_log_leave();
}


