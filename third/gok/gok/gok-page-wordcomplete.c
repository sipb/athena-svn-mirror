/* gok-page-wordcomplete.c
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
#include <gtk/gtk.h>
#include <glade/glade.h>
#include "gok-page-wordcomplete.h"
#include "gok-data.h"
#include "gok-log.h"
#include "gok-word-complete.h"
#include "gok-settings-dialog.h"

/* settings for the page */
static gboolean m_WordCompleteOnOff;
static int m_NumberPredictions;
static gboolean m_bUseAuxDicts = FALSE;
static gchar *m_AuxDicts = NULL;

/* backup of the page data */
static gboolean m_WordCompleteOnOffBackup;
static int m_NumberPredictionsBackup;
static gboolean m_bUseAuxDictsBackup = FALSE;
static gchar *m_AuxDictsBackup = NULL;

/* privates */
void gok_page_wordcomplete_initialize_auxwordlist_control(const char* file);

/**
* gok-page-wordcomplete-initialize
* @pWindowSettings: Pointer to the settings dialog window.
*
* Initializes this page of the gok settings dialog. This must be called
* prior to any calls on this page.
*
* returns: TRUE if the page was properly initialized, FALSE if not.
**/
gboolean gok_page_wordcomplete_initialize (GladeXML* xml)
{
	GtkWidget* pWidget;
	
	g_assert (xml != NULL);
		
	m_WordCompleteOnOff = gok_data_get_wordcomplete();
	m_NumberPredictions = gok_data_get_num_predictions();
	m_bUseAuxDicts = gok_data_get_use_aux_dictionaries();
	m_AuxDicts = g_strdup (gok_data_get_aux_dictionaries());
	
	/* store the current values */
	gok_page_wordcomplete_backup();

	/* update the controls */
	pWidget = glade_xml_get_widget (xml, "checkWordCompletion");
	g_assert (pWidget != NULL);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pWidget), gok_data_get_wordcomplete());

	pWidget = glade_xml_get_widget (xml, "spinNumberPredictions");
	g_assert (pWidget != NULL);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(pWidget), gok_data_get_num_predictions());
	
	pWidget = glade_xml_get_widget (xml, "checkExtraWordList");
	g_assert (pWidget != NULL);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pWidget), gok_data_get_use_aux_dictionaries());

	gok_page_wordcomplete_initialize_auxwordlist_control((const char*)m_AuxDicts);
	
	return TRUE;
}

/**
* gok_page_wordcomplete_refresh
* 
* Refreshes the controls on this page from the gok data.
**/
void gok_page_wordcomplete_refresh ()
{

}

/**
* gok_page_wordcomplete_apply
* 
* Updates the gok data with values from the controls.
*
* returns: TRUE if any of the gok data settings have changed, FALSE if not.
**/
gboolean gok_page_wordcomplete_apply ()
{
	GtkWidget* pWidget;
	gboolean bDataChanged;
	gchar* text;
	
	bDataChanged = FALSE;

	pWidget = glade_xml_get_widget (gok_settingsdialog_get_xml(), "checkWordCompletion");
	g_assert (pWidget != NULL);
	m_WordCompleteOnOff = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(pWidget));

	pWidget = glade_xml_get_widget (gok_settingsdialog_get_xml(), "spinNumberPredictions");
	g_assert (pWidget != NULL);
	m_NumberPredictions = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(pWidget));

	gok_page_wordcomplete_update_auxwordlist_from_control();
	
	/* update the gok data with any new settings */
	if (gok_data_get_wordcomplete() != m_WordCompleteOnOff)
	{
		bDataChanged = TRUE;
		gok_data_set_wordcomplete (m_WordCompleteOnOff);
		gok_keyslotter_on (m_WordCompleteOnOff, KEYTYPE_WORDCOMPLETE);
	}

	if (m_NumberPredictions != gok_data_get_num_predictions())
	{
		bDataChanged = TRUE;
		gok_data_set_num_predictions (m_NumberPredictions);
		gok_keyslotter_change_number_predictions (m_NumberPredictions, KEYTYPE_WORDCOMPLETE);
	}

	if (gok_data_get_use_aux_dictionaries() != m_bUseAuxDicts) {
		bDataChanged = TRUE;
		gok_data_set_use_aux_dictionaries ( m_bUseAuxDicts );
		/* TODO call rebuild internal word completion list */
	}
	
	return bDataChanged;
}

/**
* gok_page_wordcomplete_revert
* 
* Revert to the backup settings for this page.
*
* returns: TRUE if any of the settings have changed, FALSE 
* if they are all still the same.
**/
gboolean gok_page_wordcomplete_revert ()
{
	GtkWidget* pWidget;
	gboolean bDataChanged;

	bDataChanged = FALSE;

	/* get the original settings */
	if (m_WordCompleteOnOff != m_WordCompleteOnOffBackup)
	{
		bDataChanged = TRUE;
		m_WordCompleteOnOff = m_WordCompleteOnOffBackup;
		gok_data_set_wordcomplete (m_WordCompleteOnOff);
		gok_keyslotter_on (m_WordCompleteOnOff, KEYTYPE_WORDCOMPLETE);

		pWidget = glade_xml_get_widget (gok_settingsdialog_get_xml(), "checkWordCompletion");
		g_assert (pWidget != NULL);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pWidget), m_WordCompleteOnOff);

	}

	if (m_NumberPredictions != m_NumberPredictionsBackup)
	{
		bDataChanged = TRUE;
		m_NumberPredictions = m_NumberPredictionsBackup;
		gok_data_set_num_predictions (m_NumberPredictions);
		gok_keyslotter_change_number_predictions (m_NumberPredictions, KEYTYPE_WORDCOMPLETE);

		pWidget = glade_xml_get_widget (gok_settingsdialog_get_xml(), "spinNumberPredictions");
		g_assert (pWidget != NULL);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON(pWidget), m_NumberPredictions);
	}
	
	if (m_bUseAuxDicts != m_bUseAuxDictsBackup) {
		bDataChanged = TRUE;
		m_bUseAuxDicts = m_bUseAuxDictsBackup;
		gok_data_set_use_aux_dictionaries (m_bUseAuxDicts);
		/* TODO call rebuild internal dictionary */
		gok_log ("TODO: Call to rebuild word completion model");

		pWidget = glade_xml_get_widget (gok_settingsdialog_get_xml(), "checkExtraWordList");
		g_assert (pWidget != NULL);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pWidget), m_bUseAuxDicts);
	}
	
	if (m_AuxDicts != m_AuxDictsBackup) {
		bDataChanged = TRUE;
		g_free(m_AuxDicts);
		m_AuxDicts = g_strdup(m_AuxDictsBackup);
		gok_data_set_aux_dictionaries (m_AuxDicts);
		/* TODO call rebuild internal word completion list */
		gok_log ("TODO: Call to rebuild word completion model");
		
		pWidget = glade_xml_get_widget (gok_settingsdialog_get_xml(), "DictionaryPathEntry");
		g_assert (pWidget != NULL);
		if (m_AuxDicts) gtk_entry_set_text (GTK_ENTRY (pWidget),m_AuxDicts); 
		
	}

	return bDataChanged;
}

/**
* gok_page_wordcomplete_backup
* 
* Copies all the member settings to backup.
**/
void gok_page_wordcomplete_backup ()
{
	m_WordCompleteOnOffBackup = m_WordCompleteOnOff;
	m_NumberPredictionsBackup = m_NumberPredictions;
	m_bUseAuxDictsBackup = m_bUseAuxDicts;
	m_AuxDictsBackup = g_strdup(m_AuxDicts);
}


void 
gok_page_wordcomplete_toggle_wordlist (gboolean on)
{
	gok_log ("Use aux wordlist toggled");	
	m_bUseAuxDicts = on;
	/* TODO: enable/disable controls, but wait until 
	   implemented this for other cases too (for consistency)*/
}

void 
gok_page_wordcomplete_update_auxwordlist_from_control ()
{
	GtkWidget* pWidget; 
	gchar* text;
	
	gok_log_enter();
	pWidget = glade_xml_get_widget (gok_settingsdialog_get_xml(), "DictionaryPathEntry");
	g_assert (pWidget != NULL);
	text = (gchar *) gtk_entry_get_text ( GTK_ENTRY (pWidget));
	/* TODO call format check on this word list */
	/* TODO call rebuild internal word completion list */
	gok_log_x ("TODO: Call to check file for expected format on file [%s]", text);
	
	g_free (m_AuxDicts);
	m_AuxDicts = text;
	gok_log_leave();
}

void 
gok_page_wordcomplete_initialize_auxwordlist_control (const char* file)
{
	GtkWidget* pWidget; 
	
	gok_log_enter();
	pWidget = glade_xml_get_widget (gok_settingsdialog_get_xml(), "DictionaryPathEntry");
	g_assert (pWidget != NULL);
	if (file) gtk_entry_set_text ( GTK_ENTRY (pWidget), file);
	gok_log_leave();
}


