/* gok-page-feedbacks.c
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
#include "gok-page-feedbacks.h"
#include "gok-feedback.h"
#include "gok-log.h"
#include "gok-settings-dialog.h"

/* feedback that is currently displayed */
static GokFeedback* m_pFeedbackCurrent;

/* will be TRUE when we change the name of a feedback */
static gboolean m_bIgnoreChangeName;

/* will be TRUE if any feedback has been added, deleted or had its name changed */
/* this is not changed if an existing feedback is just modified */
static gboolean m_bChanged;

/* this should be set TRUE when we change the name in the sound name entry field */
static gboolean m_bIgnoreSoundNameChange;

/**
* gok_page_feedbacks_initialize
* @pWindowSettings: Pointer to the settings dialog window.
*
* Initializes this page of the gok settings dialog. This must be called
* prior to any calls on this page.
*
* returns: TRUE if the page was properly initialized, FALSE if not.
**/
gboolean gok_page_feedbacks_initialize (GladeXML* xml)
{
	GtkWidget* pComboBox;
	GokFeedback* pFeedback;
	GList* items = NULL;
	
	g_assert (xml != NULL);
	m_pFeedbackCurrent = NULL;
	m_bIgnoreChangeName = FALSE;
	m_bChanged = TRUE;
	m_bIgnoreSoundNameChange = FALSE;

	/* add all sounds to the 'sounds' combo box */
	gok_feedbacks_update_sound_combo();
	
	/* add all the feedbacks to the combo box */
	pComboBox = glade_xml_get_widget (xml , "comboFeedback");
	g_assert (pComboBox != NULL);

	pFeedback = gok_feedback_get_first_feedback();
	while (pFeedback != NULL)
	{
		items = g_list_append (items, pFeedback->pDisplayName);
		pFeedback = pFeedback->pFeedbackNext;
	}

	if (items != NULL)
	{
		/* sort the names */
		items = g_list_sort (items, gok_settingsdialog_sort);

		m_bIgnoreChangeName = TRUE;
		gtk_combo_set_popdown_strings (GTK_COMBO (pComboBox), items);
		m_bIgnoreChangeName = FALSE;
		g_list_free (items);
	}
	
	if (m_pFeedbackCurrent == NULL)
	{
		gok_page_feedbacks_update_controls (NULL);
	}
	
	return TRUE;
}

/**
* gok_page_feedbacks_refresh
* 
* Refreshes the controls on the this page from the gok data.
**/
void gok_page_feedbacks_refresh ()
{
}

/**
* gok_page_feedbacks_apply
* 
* Updates the gok data with values from the controls.
* Note: This is not used yet. The control values are updated each time the
* control changes.
*
* returns: TRUE if any of the data members have changed, FALSE if not.
**/
gboolean gok_page_feedbacks_apply ()
{
	return FALSE;
}

/**
* gok_page_feedbacks_revert
* 
* Revert to the backup settings for this page.
*
* returns: TRUE if any of the settings have changed, FALSE 
* if they are all still the same.
**/
gboolean gok_page_feedbacks_revert ()
{
	GokFeedback* pFeedback;
	GokFeedback* pFeedbackTemp;
	gboolean bSettingsChanged;
	gboolean bFeedbackDeleted;
	gboolean bCurrentFeedbackDeleted;

	bSettingsChanged = FALSE;
	bFeedbackDeleted = FALSE;
	bCurrentFeedbackDeleted = FALSE;
	
	/* loop through all the feedbacks */
	pFeedback = gok_feedback_get_first_feedback();
	while (pFeedback != NULL)
	{
		pFeedbackTemp = pFeedback;
		pFeedback = pFeedback->pFeedbackNext;
		
		/* delete the feedback if it was just created */
		if (pFeedbackTemp->bNewFeedback == TRUE)
		{
			bFeedbackDeleted = TRUE;
			bSettingsChanged = TRUE;

			gok_feedback_delete_feedback (pFeedbackTemp);
			
			/* is this the current feedback we're deleting? */
			if (pFeedbackTemp == m_pFeedbackCurrent)
			{
				bCurrentFeedbackDeleted = TRUE;
			}
			
		}
		else /* revert the feedback to its backup values */
		{
			if (gok_feedback_revert (pFeedbackTemp) == TRUE)
			{
				bSettingsChanged = TRUE;
			}
		}
	}
	
	/* if we deleted the current feedback then use the first feedback */
	if (bCurrentFeedbackDeleted == TRUE)
	{
		m_pFeedbackCurrent = gok_feedback_get_first_feedback();
	}
	
	/* if we deleted a feedback the update the combo list */
	if (bFeedbackDeleted == TRUE)
	{
		gok_page_feedbacks_fill_combo_feedback_names();
	}
	
	/* update the controls for the current feedback */
	gok_page_feedbacks_update_controls (m_pFeedbackCurrent);
	
	if (bSettingsChanged == TRUE)
	{
		m_bChanged = TRUE;
	}
	
	return bSettingsChanged;
}

/**
* gok_page_feedbacks_backup
* 
* Copies all the member settings to backup.
**/
void gok_page_feedbacks_backup ()
{
	GokFeedback* pFeedback;
	
	pFeedback = gok_feedback_get_first_feedback();
	while (pFeedback != NULL)
	{
		gok_feedback_backup (pFeedback);
		pFeedback = pFeedback->pFeedbackNext;
	}
}

/**
* gok_page_feedbacks_feedback_changed
* @pEditControl: Pointer to the edit control that contains the names of
* the feedbacks.
*
* The feedback name in the entry control has changed. Update the
* page's controls with the new feedback data.
**/
void gok_page_feedbacks_feedback_changed (GtkEditable* pEditControl)
{
	GokFeedback* pFeedback;
	gchar* pStrComboFeedbackName;

	pFeedback = NULL;

	/* ignore this call if we are just changing the name of the feedback */
	if (m_bIgnoreChangeName == TRUE)
	{
		return;
	}
	/* get the name of the feedback from the combo box */
	pStrComboFeedbackName = gtk_editable_get_chars (pEditControl, 0, -1);

	if (strlen (pStrComboFeedbackName) == 0)
	{
		g_free (pStrComboFeedbackName);
	}
	else
	{
		/* find the feedback in our list of feedbacks */
		pFeedback = gok_feedback_find_feedback (pStrComboFeedbackName, TRUE);
		if (pFeedback == NULL)
		{
			gok_log_x ("Feedback name (%s) from combo not found in feedback list!", pStrComboFeedbackName);
		}
		g_free (pStrComboFeedbackName);
	}

	/* store the pointer */
	m_pFeedbackCurrent = pFeedback;
	
	/* update the controls to reflect this feedback */
	gok_page_feedbacks_update_controls (pFeedback);
}

/**
* gok_page_feedbacks_update_controls
* @pFeedback: Controls are changed to display this feedback.
*
* Updates the controls so they reflect the given feedback.
**/
void gok_page_feedbacks_update_controls (GokFeedback* pFeedback)
{
	GtkWidget* pButton;
	
	/* if no feedback then disable all controls */
	if (pFeedback == NULL)
	{
		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "buttonChangeFeedbackName");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, FALSE);
			
		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "buttonDeleteFeedback");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, FALSE);

		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "checkKeyFlashing");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, FALSE);

		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "spinKeyFlashing");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, FALSE);

		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "checkSoundOn");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, FALSE);

		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "comboSoundName");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, FALSE);

		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "buttonFeedbackSoundFile");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, FALSE);

		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "SpeakLabelCheckButton");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, FALSE);

		return;
	}

	/* is this a permanent feedback? */
	if (pFeedback->bPermanent == TRUE)
	{
		/* don't allow user to edit the feedback name */
		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "buttonChangeFeedbackName");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, FALSE);
		
		/* don't allow user to delete the feedback */
		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "buttonDeleteFeedback");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, FALSE);
		
		/* don't allow user to change the flashing on/off */
		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "checkKeyFlashing");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, FALSE);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pButton), pFeedback->bFlashOn);

		/* don't allow user to change the sound on/off */
		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "checkSoundOn");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, FALSE);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pButton), pFeedback->bSoundOn);
	}
	else
	{
		/* not permanent */
		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "buttonDeleteFeedback");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, TRUE);
						
		/* allow user to edit the feedback name */
		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "buttonChangeFeedbackName");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, TRUE);

		/* allow user to change the flashing on/off */
		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "checkKeyFlashing");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, TRUE);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pButton), pFeedback->bFlashOn);

		/* allow user to change the sound on/off */
		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "checkSoundOn");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, TRUE);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pButton), pFeedback->bSoundOn);

		/* allow user to turn the speech on/off */
		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "SpeakLabelCheckButton");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, TRUE);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pButton), pFeedback->bSpeechOn);
	}
	
	pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "spinKeyFlashing");
	g_assert (pButton != NULL);
	gtk_widget_set_sensitive (pButton, TRUE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(pButton), pFeedback->NumberFlashes);

	pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "comboSoundName");
	g_assert (pButton != NULL);
	gtk_widget_set_sensitive (pButton, TRUE);

	pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "entrySoundName");
	g_assert (pButton != NULL);
	gtk_widget_set_sensitive (pButton, TRUE);
	m_bIgnoreSoundNameChange = TRUE;
	
	if (pFeedback->pNameSound != NULL)
	{
		gtk_entry_set_text (GTK_ENTRY(pButton), pFeedback->pNameSound);
	}
	else
	{
		gtk_entry_set_text (GTK_ENTRY(pButton), "");
	}
	m_bIgnoreSoundNameChange = FALSE;

	pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "buttonFeedbackSoundFile");
	g_assert (pButton != NULL);
	gtk_widget_set_sensitive (pButton, TRUE);
}

/**
* gok_page_feedbacks_button_clicked_change_name
* 
* The button "change name" has been clicked so allow the user to change the
* name of the feedback.
**/
void gok_page_feedbacks_button_clicked_change_name ()
{
	GtkWidget* pDialogChangeName;
	GtkWidget* pLabel;
	GtkWidget* pEntryNewName;
	gint response;
	const gchar* pNewName;
	GtkWidget* pDialog;
	GokFeedback* pFeedback;
	gboolean bFeedbackNameExists;

	g_assert (m_pFeedbackCurrent != NULL);

	/* create the 'new feedbacks name' dialog */
	pDialogChangeName = gtk_dialog_new_with_buttons (_("GOK Feedback Name"),
												GTK_WINDOW(gok_settingsdialog_get_window()),
												GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
												GTK_STOCK_OK, GTK_RESPONSE_OK,
												GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
	/* add a text label */
	pLabel = gtk_label_new (_("Change the feedback name:"));
	gtk_widget_ref (pLabel);
	gtk_object_set_data_full (GTK_OBJECT (pDialogChangeName), "pLabel", pLabel,
                            (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (pLabel);
	gtk_box_pack_start (GTK_BOX(GTK_DIALOG (pDialogChangeName)->vbox), pLabel, FALSE, FALSE, 0);

	/* add a text entry */
	pEntryNewName = gtk_entry_new ();
	gtk_widget_ref (pEntryNewName);
	gtk_object_set_data_full (GTK_OBJECT (pDialogChangeName), "Entry", pEntryNewName,
                            (GtkDestroyNotify) gtk_widget_unref);
	gtk_entry_set_text (GTK_ENTRY(pEntryNewName), m_pFeedbackCurrent->pDisplayName);
	gtk_widget_show (pEntryNewName);
	gtk_box_pack_start (GTK_BOX(GTK_DIALOG (pDialogChangeName)->vbox), pEntryNewName, FALSE, FALSE, 0);

	/* increase the spacing among the controls */
	gtk_box_set_spacing (GTK_BOX(GTK_DIALOG (pDialogChangeName)->vbox), 10);
	
	/* display the dialog */
	while (1)
	{
		response = gtk_dialog_run (GTK_DIALOG(pDialogChangeName));

		if (response == GTK_RESPONSE_CANCEL)
		{
			break;
		}
		
		pNewName = gtk_entry_get_text (GTK_ENTRY(pEntryNewName));
		/* check if the new feedback name is empty */
		if (strlen (pNewName) == 0)
		{
			pDialog = gtk_message_dialog_new (GTK_WINDOW(pDialogChangeName),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				_("Feedback name can't be empty.\nPlease enter a new feedback name."));

			gtk_window_set_title (GTK_WINDOW (pDialog), _("GOK Feedback Name"));
			gtk_dialog_run (GTK_DIALOG (pDialog));
			gtk_widget_destroy (pDialog);
		}
		else
		{
			/* check if the feedback name already exists */
			bFeedbackNameExists = FALSE;
			pFeedback = gok_feedback_get_first_feedback();
			while (pFeedback != NULL)
			{
				if (strcmp (pFeedback->pDisplayName, pNewName) == 0)
				{
					pDialog = gtk_message_dialog_new (GTK_WINDOW(pDialogChangeName),
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_ERROR,
						GTK_BUTTONS_CLOSE,
						_("Sorry, that feedback name already exists.\nPlease enter a new feedback name"));
					
					gtk_window_set_title (GTK_WINDOW (pDialog), _("GOK Feedback Name"));
					gtk_dialog_run (GTK_DIALOG (pDialog));
					gtk_widget_destroy (pDialog);
					
					bFeedbackNameExists = TRUE;
					break;
				}
				pFeedback = pFeedback->pFeedbackNext;
			}
			
			if (bFeedbackNameExists == FALSE)
			{
				/* was the name changed? */
				if (strcmp (m_pFeedbackCurrent->pDisplayName, pNewName) != 0)
				{
					/* yes the name was changed */
					g_free (m_pFeedbackCurrent->pDisplayName);
					m_pFeedbackCurrent->pDisplayName = (gchar*)g_malloc (strlen (pNewName) + 1);
					strcpy (m_pFeedbackCurrent->pDisplayName, pNewName);
		
					/* add the new name to the combo box list */
					gok_page_feedbacks_fill_combo_feedback_names();
					
					/* set the 'changed' flag */
					m_bChanged = TRUE;
				}
				break;
			}
		}
	}

	/* destroy the dialog */
	gtk_widget_destroy (pDialogChangeName);
}

/**
* gok_page_feedbacks_button_clicked_new
* 
* The button "new feedback" has been clicked so add a new feedback.
**/
void gok_page_feedbacks_button_clicked_new()
{
	GokFeedback* pFeedbackNew;
	int count;
	gchar bufferDisplayName[200];
	gchar bufferName[200];
	
	/* create the new feedback default name */
	/* it will be something like "new feedback 1" */
	for (count = 1; count < 100; count++)
	{
		sprintf (bufferDisplayName, _("New Feedback %d"), count);
		sprintf (bufferName, "new_feedback_%d", count);
		if ((gok_feedback_find_feedback (bufferDisplayName, TRUE) == NULL) &&
			(gok_feedback_find_feedback (bufferName, FALSE) == NULL))
		{
			break;
		}
	}
	
	pFeedbackNew = gok_feedback_new();
	pFeedbackNew->pName = (gchar*)g_malloc (strlen (bufferName) + 1);
	strcpy (pFeedbackNew->pName, bufferName);
	pFeedbackNew->pDisplayName = (gchar*)g_malloc (strlen (bufferDisplayName) + 1);
	strcpy (pFeedbackNew->pDisplayName, bufferDisplayName);

	/* make the new feedback the same as the current feedback */
	if (m_pFeedbackCurrent != NULL)
	{
		pFeedbackNew->bFlashOn = m_pFeedbackCurrent->bFlashOn;
		pFeedbackNew->NumberFlashes = m_pFeedbackCurrent->NumberFlashes;
		pFeedbackNew->bSoundOn = m_pFeedbackCurrent->bSoundOn;
		if (m_pFeedbackCurrent->pNameSound != NULL)
		{
                    pFeedbackNew->pNameSound = g_strdup (m_pFeedbackCurrent->pNameSound);
		}
	}
	
	pFeedbackNew->bPermanent = FALSE;
	pFeedbackNew->bNewFeedback = TRUE;

	gok_feedback_add_feedback (pFeedbackNew);
	
	m_pFeedbackCurrent = pFeedbackNew;
	
	/* update the combo box with the new feedback name */
	gok_page_feedbacks_fill_combo_feedback_names();
	gok_page_feedbacks_update_controls (m_pFeedbackCurrent);
	
	/* mark the feedback as 'changed' */
	m_bChanged = TRUE;
}

/**
* gok_page_feedbacks_fill_combo_feedback_names
*
* Fills the combo box that displays the feedback names.
* This also displays the name of the current feedback in the entry control.
**/
void gok_page_feedbacks_fill_combo_feedback_names ()
{
	GtkWidget* pComboBox;
	GtkWidget* pEntry;
	GokFeedback* pFeedback;
	GList* items = NULL;
	gchar* pFeedbackName;

	/* ignore the 'entry text changed' message generated by this routine */
	m_bIgnoreChangeName = TRUE;
	
	/* find the combo box */
	pComboBox = glade_xml_get_widget (gok_settingsdialog_get_xml(), "comboFeedback");
	g_assert (pComboBox != NULL);

	/* create a list of feedback display names */
	pFeedback = gok_feedback_get_first_feedback();
	while (pFeedback != NULL)
	{
		items = g_list_append (items, pFeedback->pDisplayName);
		pFeedback = pFeedback->pFeedbackNext;
	}
	
	/* are there any feedback names? */
	if (items == NULL)
	{
		/* no, so empty the combo box */
		items = g_list_append (items, "");
	}
	/* sort the names */
	items = g_list_sort (items, gok_settingsdialog_sort);

	gtk_combo_set_popdown_strings (GTK_COMBO (pComboBox), items);
	g_list_free (items);

	/* display the current feedback name in the entry control of the combo */
	if (m_pFeedbackCurrent != NULL)
	{
		pFeedbackName = m_pFeedbackCurrent->pDisplayName;
	}
	else
	{
		pFeedbackName = "";
	}
	pEntry = glade_xml_get_widget (gok_settingsdialog_get_xml(), "entryFeedback");
	g_assert (pEntry != NULL);
	gtk_entry_set_text (GTK_ENTRY(pEntry), pFeedbackName);
	
	m_bIgnoreChangeName = FALSE;
}	

/**
* gok_page_feedbacks_button_clicked_delete
* 
* The button "delete feedback" has been clicked so delete the current feedback.
**/
void gok_page_feedbacks_button_clicked_delete ()
{
	GtkWidget* pDialog;
	GtkWidget* pComboEntry;
	const gchar* pNameFeedback;
	gchar buffer[200];
	gint result;
	GokFeedback* pFeedback;
	GokFeedback* pFeedbackDisplayed;
	
	pComboEntry = glade_xml_get_widget (gok_settingsdialog_get_xml(), "entryFeedback");
	g_assert (pComboEntry != NULL);
	pNameFeedback = gtk_entry_get_text (GTK_ENTRY(pComboEntry));

	sprintf (buffer, _("Do you wish to delete this feedback (%s)?"), pNameFeedback);

	pDialog = gtk_message_dialog_new ((GtkWindow*)gok_settingsdialog_get_window(),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
			buffer);
	
	gtk_window_set_title (GTK_WINDOW (pDialog), _("GOK Delete Feedback"));
	result = gtk_dialog_run (GTK_DIALOG (pDialog));
	gtk_widget_destroy (pDialog);

	if (result == GTK_RESPONSE_NO)
	{
		return;
	}
	
	pFeedbackDisplayed = NULL;
	pFeedback = gok_feedback_get_first_feedback();
	while (pFeedback != NULL)
	{
		if (strcmp (pFeedback->pDisplayName, pNameFeedback) == 0)
		{
			/* found the feedback */
			/* make sure this is not a permanent feedback */
			g_assert (pFeedback->bPermanent == FALSE);
			
			/* delete the feedback */
			if (pFeedbackDisplayed == NULL)
			{
				pFeedbackDisplayed = pFeedback->pFeedbackNext;
			}
			
			gok_feedback_delete_feedback (pFeedback);
			break;
		}
		
		pFeedbackDisplayed = pFeedback;
		pFeedback = pFeedback->pFeedbackNext;
	}

	m_pFeedbackCurrent = pFeedbackDisplayed;
	
	/* update the combo box with all the Feedback names */
	gok_page_feedbacks_fill_combo_feedback_names();
	gok_page_feedbacks_update_controls (m_pFeedbackCurrent);
	
	/* mark the feedbacks as 'changed' */
	m_bChanged = TRUE;
}

/**
* gok_page_feedbacks_check_keyflashing_clicked
*
* The checkbox has been clicked. Update the current feedback with the change.
**/
void gok_page_feedbacks_check_keyflashing_clicked ()
{
	GtkWidget* pCheckbox;
	
	if (m_pFeedbackCurrent == NULL)
	{
		return;
	}

	pCheckbox = glade_xml_get_widget (gok_settingsdialog_get_xml(), "checkKeyFlashing");
	g_assert (pCheckbox != NULL);
	m_pFeedbackCurrent->bFlashOn = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(pCheckbox));
}

/**
* gok_page_feedbacks_spin_keyflashing_changed
*
* The spin control has changed. Update the current feedback with the change.
**/
void gok_page_feedbacks_spin_keyflashing_changed ()
{
	GtkWidget* pSpinControl;
	
	if (m_pFeedbackCurrent == NULL)
	{
		return;
	}

	pSpinControl = glade_xml_get_widget (gok_settingsdialog_get_xml(), "spinKeyFlashing");
	g_assert (pSpinControl != NULL);
	m_pFeedbackCurrent->NumberFlashes = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(pSpinControl));
}

/**
* gok_page_feedbacks_check_sound_clicked
*
* The checkbox has been clicked. Update the current feedback with the change.
**/
void gok_page_feedbacks_check_sound_clicked ()
{
	GtkWidget* pCheckbox;
	
	if (m_pFeedbackCurrent == NULL)
	{
		return;
	}

	pCheckbox = glade_xml_get_widget (gok_settingsdialog_get_xml(), "checkSoundOn");
	g_assert (pCheckbox != NULL);
	m_pFeedbackCurrent->bSoundOn = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(pCheckbox));
}

/**
* gok_page_feedbacks_check_speech_toggled
*
* The checkbox has been toggled. Update the current feedback with the change.
**/
void gok_page_feedbacks_check_speech_toggled (GtkToggleButton *button)
{
	if (m_pFeedbackCurrent == NULL)
	{
		return;
	}

	m_pFeedbackCurrent->bSpeechOn = gtk_toggle_button_get_active (button);
}

/**
* gok_page_feedbacks_entry_soundname_changed
*
* The sound file name has changed. Update the current feedback with the change.
**/
void gok_page_feedbacks_entry_soundname_changed ()
{
	GtkWidget* pEntry;
	
	if (m_bIgnoreSoundNameChange == TRUE)
	{
		return;
	}
	
	if (m_pFeedbackCurrent == NULL)
	{
		return;
	}

	pEntry = glade_xml_get_widget (gok_settingsdialog_get_xml(), "entrySoundName");
	g_assert (pEntry != NULL);

	/* is there a sound file selected? */
	if (strlen (gtk_entry_get_text (GTK_ENTRY(pEntry))) > 0)
	{
		/* yes, there is a sound file selected */
		/* does the feedback already have a sound file name? */
		if (m_pFeedbackCurrent->pNameSound != NULL)
		{
			/* yes, is the feedback name the same as the name in the entry? */
			if (strcmp (m_pFeedbackCurrent->pNameSound, gtk_entry_get_text (GTK_ENTRY(pEntry))) != 0)
			{
				/* no, names are different */
				/* free the existing name and store the new name */
				g_free (m_pFeedbackCurrent->pNameSound);	
                                m_pFeedbackCurrent->pNameSound = g_strdup (gtk_entry_get_text (GTK_ENTRY(pEntry)));
			}
		}
		else /* feedback does not have a sound file name */
		{
                    m_pFeedbackCurrent->pNameSound = g_strdup (gtk_entry_get_text (GTK_ENTRY(pEntry)));
		}
	}
	else /* no sound file selected */
	{
		if (m_pFeedbackCurrent->pNameSound != NULL)
		{
			g_free (m_pFeedbackCurrent->pNameSound);	
			m_pFeedbackCurrent->pNameSound = NULL;
		}
	}
}

/**
* gok_feedbacks_update_sound_combo
*
* Adds all the sounds from the feedbacks to the 'sounds' combo box list.
**/
void gok_feedbacks_update_sound_combo()
{
	GtkWidget* pCombo;
	GList* items = NULL;
	GokFeedback* pFeedback;

	pCombo = glade_xml_get_widget (gok_settingsdialog_get_xml(), "comboSoundName");
	g_assert (pCombo != NULL);

	pFeedback = gok_feedback_get_first_feedback();
	while (pFeedback != NULL)
	{
		if (pFeedback->pNameSound != NULL)
		{
			items = g_list_append (items, pFeedback->pNameSound);
		}
		pFeedback = pFeedback->pFeedbackNext;
	}

	if (items != NULL)
	{
		/* sort the names */
		items = g_list_sort (items, gok_settingsdialog_sort);

		m_bIgnoreSoundNameChange = TRUE;
		gtk_combo_set_popdown_strings (GTK_COMBO (pCombo), items);
		g_list_free (items);
		m_bIgnoreSoundNameChange = FALSE;
	}
}

/**
* gok_page_feedbacks_get_sound_file
*
* Get a new sound file for the current feedback.
**/
void gok_page_feedbacks_get_sound_file ()
{
	GtkWidget* pDialogFilename;
	gint response;
	gchar* filename;
	GtkWidget* pEntry;

	/* make sure the current feedback is not NULL */
	if (m_pFeedbackCurrent == NULL)
	{
		return;
	}
	
	/* create the file selector dialog */
	pDialogFilename = gtk_file_chooser_dialog_new (_("Select sound file"),
						       NULL,
						       GTK_FILE_CHOOSER_ACTION_OPEN,
						       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						       GTK_STOCK_OK, GTK_RESPONSE_OK,
						       NULL);

	/* display that file selector dialog */
	gtk_window_set_default_size (GTK_WINDOW (pDialogFilename), 600, 400);
	response = gtk_dialog_run (GTK_DIALOG (pDialogFilename));

	if (response != GTK_RESPONSE_OK)
	{
		/* destroy the file selector dialog */
		gtk_widget_destroy (pDialogFilename);

		return;
	}

	/* get the file name */
	filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (pDialogFilename));

	/* add it to the feedback */
	if (m_pFeedbackCurrent->pNameSound != NULL)
	{
		g_free (m_pFeedbackCurrent->pNameSound);
	}
        
	m_pFeedbackCurrent->pNameSound = filename;

	/* update the combo box list */
	gok_feedbacks_update_sound_combo();
	
	/* display it in the entry of the combo box */
	pEntry = glade_xml_get_widget (gok_settingsdialog_get_xml(), "entrySoundName");
	g_assert (pEntry != NULL);
	m_bIgnoreSoundNameChange = TRUE;
	gtk_entry_set_text (GTK_ENTRY(pEntry), filename);
	m_bIgnoreSoundNameChange = FALSE;

	/* destroy the file selector dialog */
	gtk_widget_destroy (pDialogFilename);
}

/**
* gok_page_feedbacks_get_changed
*
* returns: TRUE if the feedbacks have been changed.
**/
gboolean gok_page_feedbacks_get_changed()
{
	return m_bChanged;
}

/**
* gok_page_feedbacks_set_changed
* @bTrueFalse: TRUE if the feedbacks should be marked as changed.
*
* Sets or clears the 'changed' flag.
**/
void gok_page_feedbacks_set_changed (gboolean bTrueFalse)
{
	m_bChanged = bTrueFalse;
}
