/* gok-settings-dialog.c
*
* Copyright 2002 Sun Microsystems, Inc.,
* Copyright 2002 University Of Toronto
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <gnome.h>
#include <glade/glade.h>
#include "gok-settings-dialog.h"
#include "gok-page-keysizespace.h"
#include "gok-page-actions.h"
#include "gok-page-feedbacks.h"
#include "gok-page-accessmethod.h"
#include "gok-page-wordcomplete.h"
#include "gok-page-keyboard.h"
#include "gok-page-accessmethod.h"
#include "gok-glade-helpers.h"
#include "gok-data.h"
#include "gok-log.h"
#include "main.h"
#include "gok-gconf-keys.h"
#include "gok-gconf.h"

/* pointer to the settings dialog window */
static GtkWidget* m_pWindowSettings;

/* pointer to the settings dialog xml data type */
static GladeXML* m_pXML;

static gboolean is_locked;

/**
* gok_settingsdialog_open
* 
* Creates the GOK settings dialog.
*
* @bShow: If TRUE the settings dialog will be shown. If FALSE the dialog will be
* created but not shown.
*
* returns: TRUE if the settings dialog could be created, FALSE if not.
**/
gboolean gok_settingsdialog_open (gboolean bShow)
{
	GladeXML *xml;
	
	xml = gok_glade_xml_new("gok.glade2", "dialogSettings");
	m_pXML = xml;
	
	m_pWindowSettings = glade_xml_get_widget(xml, "dialogSettings");
	g_assert (m_pWindowSettings != NULL);
	
	is_locked = FALSE;  /* don't read gconf here */

	/* initialize all the pages */
	gok_settings_page_keysizespace_initialize (xml);
	gok_page_accessmethod_initialize (xml);
	gok_page_keyboard_initialize (xml);
	gok_page_actions_initialize (xml);
	gok_page_feedbacks_initialize (xml);
	gok_page_wordcomplete_initialize (xml);
	
	/* create a backup copy of all the settings */
	gok_data_backup_settings();
			
	if (bShow == TRUE)
	{
		gok_settingsdialog_show ();
	}

	return TRUE;
}

/**
* gok_settingsdialog_show
* 
* Displays the GOK settings dialog.
*
* returns: TRUE if the settings dialog was shown, FALSE if not.
**/
gboolean gok_settingsdialog_show()
{
	gboolean bFirstShowing;
	GtkWidget *label;
	GtkWidget *notebook;
	gboolean was_locked;

	g_assert (m_pWindowSettings != NULL);

	was_locked = is_locked;
	gok_gconf_get_bool (gconf_client_get_default(),
		GOK_GCONF_PREFS_LOCKED, &is_locked);
	
	if (was_locked != is_locked) {
		if (!is_locked) {
			gtk_widget_show(glade_xml_get_widget(m_pXML, "ActionsTab"));
			gtk_widget_show(glade_xml_get_widget(m_pXML, "AccessMethodsTab"));
		}
		else {
			gtk_widget_hide(glade_xml_get_widget(m_pXML, "ActionsTab"));
			gtk_widget_hide(glade_xml_get_widget(m_pXML, "AccessMethodsTab"));
		}
	}

	bFirstShowing = (GDK_IS_DRAWABLE(m_pWindowSettings->window) == FALSE) ? TRUE : FALSE;

	gtk_widget_show (m_pWindowSettings);

	/* if this is the first time we're showing the dialog then redisplay
	  the key size/spacing page so the example keys are centered */
	if (bFirstShowing == TRUE)
	{
		gok_settings_page_keysizespace_refresh();
	}
	
	return TRUE;
}

/**
* gok_settingsdialog_hide
* 
* Hides the GOK settings dialog.
**/
void gok_settingsdialog_hide()
{
	g_assert (m_pWindowSettings != NULL);

 	gtk_widget_hide (m_pWindowSettings);
}

/**
* gok_settingsdialog_get_window
*
* returns: A pointer to the settings dialog window.
**/
GtkWidget* gok_settingsdialog_get_window ()
{
 	return m_pWindowSettings;
}

/**
* gok_settingsdialog_get_xml
*
* returns: A pointer to the settings dialog glade xml data type.
**/
GladeXML* gok_settingsdialog_get_xml ()
{
 	return m_pXML;
}

/**
* gok_settingsdialog_close
* 
* Destroys the GOK settings dialog.
*
* returns: void
**/
void gok_settingsdialog_close()
{
	gok_log_enter();
	if (m_pWindowSettings != NULL)
	{
		gok_page_accessmethod_close();
		gtk_widget_destroy (m_pWindowSettings);
	}
	else
	{
		g_warning("The settings dialog doesn't exist in close function!");
	}
	gok_log_leave();
}

/**
* on_button_ok
* @pButton: Pointer to the button that was clicked.
* @user_data: Any user data associated with the button.
* 
* The OK button has been clicked. Apply any changes from the settings and
* hide the settings dialog.
**/
void on_button_ok (GtkButton* button, gpointer user_data)
{
	/* apply the current settings */
	on_button_try (NULL, NULL);
	
	/* copy the current settings to the backup */
	gok_settingsdialog_backup_settings();
	
	/* hide the settings dialog */
	gok_settingsdialog_hide();
}

/**
* on_button_try
* @pButton: Pointer to the button that was clicked.
* @user_data: Any user data associated with the button.
* 
* The TRY button has been clicked. Apply any changes from the settings but
* don't hide the dialog
**/
void on_button_try (GtkButton* button, gpointer user_data)
{
	gboolean bDataChanged;

	bDataChanged = FALSE;
	
	/* use the current settings */
	if (gok_settings_page_keysizespace_apply() == TRUE)
	{
		bDataChanged = TRUE;
	}
	if (gok_page_actions_apply() == TRUE)
	{
		bDataChanged = TRUE;
	}
	if (gok_page_keyboard_apply() == TRUE)
	{
		bDataChanged = TRUE;
	}
	if (gok_page_feedbacks_apply() == TRUE)
	{
		bDataChanged = TRUE;
	}
	if (gok_page_accessmethod_apply() == TRUE)
	{
		bDataChanged = TRUE;
	}
	if (gok_page_wordcomplete_apply() == TRUE)
	{
		bDataChanged = TRUE;
	}
	
	/* if any settings have changed then refresh the keyboard */
	if (bDataChanged == TRUE)
	{
		gok_main_display_scan_reset();
		/* and post a warning if the new settings use corepointer */
		gok_main_warn_if_corepointer_mode (_("You appear to be configuring GOK to use \'core pointer\' mode."), FALSE);
	}
}

/**
* on_button_revert
* @pButton: Pointer to the button that was clicked.
* @user_data: Any user data associated with the button.
* 
* The REVERT button has been clicked. Returns the settings to the way they were
* and update the keyboard.
**/
void on_button_revert (GtkButton* button, gpointer user_data)
{
	gboolean bDataChanged;

	bDataChanged = FALSE;
	
	if (gok_data_restore_settings() == TRUE)
	{
		bDataChanged = TRUE;
	}
	if (gok_settings_page_keysizespace_revert() == TRUE)
	{
		bDataChanged = TRUE;
	}
	if (gok_page_keyboard_revert() == TRUE)
	{
		bDataChanged = TRUE;
	}
	if (gok_page_accessmethod_revert() == TRUE)
	{
		bDataChanged = TRUE;
	}
	if (gok_page_wordcomplete_revert() == TRUE)
	{
		bDataChanged = TRUE;
	}
	if (gok_page_actions_revert() == TRUE)
	{
		bDataChanged = TRUE;
	}
	if (gok_page_feedbacks_revert() == TRUE)
	{
		bDataChanged = TRUE;
	}

	/* if any setting has changed then refresh the keyboard and dialog controls */
	if (bDataChanged == TRUE)
	{
		gok_main_display_scan_reset();
	}
}

/**
* on_button_cancel
* @pButton: Pointer to the button that was clicked.
* @user_data: Any user data associated with the button.
* 
* The CANCEL button has been clicked. Return settings to the way they were and
* update the keyboard and hide the settings dialog.
**/
void on_button_cancel (GtkButton* pButton, gpointer user_data)
{
	/* revert to the original settings */
	on_button_revert(NULL, NULL);

	/* hide the settings dialog */
	gok_settingsdialog_hide();
}

/**
* on_button_help
* @pButton: Pointer to the button that was clicked.
* @user_data: Any user data associated with the button.
*
* The HELP button has been clicked.
**/
void on_button_help (GtkButton* pButton, gpointer user_data)
{
	GtkWidget* pNotebook;
	const char* help_link_id = NULL;
	
	pNotebook = glade_xml_get_widget (m_pXML, "notebook1");
	if (pNotebook == NULL)
	{
		gok_log_x ("Can't find notebook!\n");
		return;
	}
	
	switch (gtk_notebook_get_current_page (GTK_NOTEBOOK (pNotebook)))
	{
		case 0:
			help_link_id = "gok-prefs-appearance";
			break;
			
		case 1:
			help_link_id = "gok-keyboards";
			break;
			
		case 2:
			help_link_id = "gok-prefs-Actions";
			break;
			
		case 3:
			help_link_id = "gok-prefs-Feedback";
			break;

		case 4:
			help_link_id = "gok-prefs-Access-Methods";
			break;
			
		case 5:
			help_link_id = "gok-prefs-Prediction";
			break;
			
		default:
			break;
	}
	if (help_link_id)
	{
		gnome_help_display_desktop (NULL, "gok", "gok.xml", help_link_id, NULL);
	}
}

/**
* gok_settingsdialog_refresh
* 
* Refreshes the dialog controls. This should be called after the user has resized the keyboard
* (which causes the key size to change) or if the gok data has changed.
**/
void gok_settingsdialog_refresh ()
{
	/* only the keysize/space page needs to get refreshed */
	gok_settings_page_keysizespace_refresh();
}

/**
* gok_settingsdialog_backup_settings
* 
* Copies all the member settings to backup.
**/
void gok_settingsdialog_backup_settings ()
{
	gok_settings_page_keysizespace_backup();
	gok_page_actions_backup();
	gok_page_keyboard_backup();
	gok_page_feedbacks_backup();
	gok_page_accessmethod_backup();
	gok_page_wordcomplete_backup();
	
	gok_data_backup_settings();
}

/**
* gok_settingsdialog_sort
* @pItem1: pointer to item #1.
* @pItem2: pointer to item #2.
*
* returns: 0 if the items are the same, -1 if pItem1 is less than pItem 2.
* Returns 1 if pItem1 is greater than pItem2.
**/
gint gok_settingsdialog_sort (gconstpointer pItem1, gconstpointer pItem2)
{
	return strcmp (pItem1, pItem2);
}
