/* gok-page-actions.c
*
* Copyright 2002, 2003 Sun Microsystems, Inc.,
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
#include "gok-page-actions.h"
#include "gok-action.h"
#include "gok-input.h"
#include "gok-data.h"
#include "gok-log.h"
#include "gok-settings-dialog.h"

/* action that is currently displayed */
static GokAction* m_pActionCurrent;

/* will be TRUE when we change the name of an action */
static gboolean m_bIgnoreChangeName;

/* type of controls that are active */
static gint m_ControlType;

/* will be TRUE if any action has been added, deleted or had its name changed */
/* this is not changed if an existing action is just modified */
static gboolean m_bChanged;

/**
* gok_page_actions_initialize
* @pWindowSettings: Pointer to the settings dialog window.
*
* Initializes this page of the gok settings dialog. This must be called
* prior to any calls on this page.
*
* returns: TRUE if the page was properly initialized, FALSE if not.
**/
gboolean gok_page_actions_initialize (GladeXML* xml)
{
	GtkWidget* pComboBox, *range;
	GokAction* pAction;
	GList* items = NULL;
	GSList* pDevice = NULL;
	GSList* slist;
	gboolean bReturnCode = TRUE; /* unused? */
	
	g_assert (xml != NULL);
	m_pActionCurrent = gok_action_get_first_action ();
	m_bIgnoreChangeName = FALSE;
	m_bChanged = TRUE;

	/* add all the actions to the combo box */
	pComboBox = glade_xml_get_widget (xml, "comboActionNames");
	g_assert (pComboBox != NULL);
	
	gok_page_actions_fill_combo_action_names();
	
	gok_page_actions_update_controls (gok_action_get_first_action ());

	/* initialize the XInput device combobox */
	pComboBox = glade_xml_get_widget (xml, "comboInputDevice");
	g_assert (pComboBox != NULL);
	
	pDevice = gok_input_get_device_list ();
	if (pDevice)
	{
		items = NULL;
		slist = pDevice;
		while ((slist != NULL) && (slist->data != NULL))
		{
			GokInput *input;
			input = (GokInput *) slist->data;
			/* small hack so that the gconf stored extension device will
			   be at the top of the list, and therefore not overridden 
			   TODO: store this as part of an action */
			if (strcmp(input->name, gok_input_get_extension_device_name()) == 0) {
				items = g_list_prepend (items, input->name );
			}
			else {
				items = g_list_append (items, input->name);
			}
			/* TODO: persist this list so we can query it later */
			slist = slist->next;
		}
	
		gtk_combo_set_popdown_strings (GTK_COMBO (pComboBox), items);
		g_list_free (items);

		slist = pDevice;
		while ((slist != NULL) && (slist->data != NULL))
		{
			GokInput *input;
			input = (GokInput *) slist->data;
			gok_input_free (input); 
			slist = slist->next;
		}
		g_slist_free (pDevice);
	}

	gok_page_actions_enable_radios_type (FALSE);

	/* init the (now-global) valuator sensitivity slider */
	range = glade_xml_get_widget (xml, "ValuatorSensitivityScale");
	g_assert (range != NULL);
	gtk_range_set_value (GTK_RANGE (range), 
			     gok_data_get_valuator_sensitivity ());

	return bReturnCode;
}

/**
* gok_page_actions_refresh
* 
* Refreshes the controls on the this page from the gok data.
**/
void gok_page_actions_refresh ()
{
}

/**
* gok_page_actions_apply
* 
* Updates the gok data with values from the controls.  Unfinished.
*
* Returns: TRUE if any of the data members have changed, FALSE if not.
**/
gboolean gok_page_actions_apply ()
{
	/* TODO: finish implementation */	
	return FALSE;
}

/**
* gok_page_actions_revert
* 
* Revert to the backup settings for this page.
*
* Returns: TRUE if any of the settings have changed, FALSE 
* if they are all still the same.
**/
gboolean gok_page_actions_revert ()
{
	GokAction* pAction;
	GokAction* pActionTemp;
	gboolean bSettingsChanged;
	gboolean bChangeCurrent;
	gboolean bActionDeleted;
	gboolean bCurrentActionDeleted;

	bActionDeleted = FALSE;
	bCurrentActionDeleted = FALSE;
	
	bSettingsChanged = FALSE;
	bChangeCurrent = FALSE;
	
	/* loop through all the actions */
	pAction = gok_action_get_first_action();
	while (pAction != NULL)
	{
		pActionTemp = pAction;
		pAction = pAction->pActionNext;
		
		/* delete the action if it was just created */
		if (pActionTemp->bNewAction == TRUE)
		{
			bActionDeleted = TRUE;
			bSettingsChanged = TRUE;

			gok_action_delete_action (pActionTemp);
			
			/* is this the current action we're deleting? */
			if (pActionTemp == m_pActionCurrent)
			{
				bCurrentActionDeleted = TRUE;
			}
			
		}
		else /* revert the action to its backup values */
		{
			if (gok_action_revert (pActionTemp) == TRUE)
			{
				bSettingsChanged = TRUE;
			}
		}
	}

	/* if we deleted the current action then use the first action */
	if (bCurrentActionDeleted == TRUE)
	{
		m_pActionCurrent = gok_action_get_first_action();
	}
	
	/* if we deleted a action the update the combo list */
	if (bActionDeleted == TRUE)
	{
		gok_page_actions_fill_combo_action_names();
	}
	
	/* update the controls for the current action */
	gok_page_actions_update_controls (m_pActionCurrent);
	
	if (bSettingsChanged == TRUE)
	{
		m_bChanged = TRUE;
	}
	
	return bSettingsChanged;
}

/**
* gok_page_actions_backup
* 
* Copies all the member settings to backup.
**/
void gok_page_actions_backup ()
{
	GokAction* pAction;

	pAction = gok_action_get_first_action();
	while (pAction != NULL)
	{
		gok_action_backup (pAction);		
		pAction = pAction->pActionNext;
	}
}

/**
* gok_page_actions_toggle_type_switch
* @Pressed: State of the toggle button.
*
* The radio button for the type 'switch' has been toggled.
* Hide or show the controls for the switch actions.
**/
void gok_page_actions_toggle_type_switch (gboolean Pressed)
{
	GtkToggleButton* pRadioButton;
	GtkSpinButton* pSpinButton;
	GtkNotebook *pNotebook;

	g_assert (m_pActionCurrent != NULL);

	pNotebook = (GtkNotebook*) glade_xml_get_widget (gok_settingsdialog_get_xml(), 
			"action_type_notebook");
	g_assert (pNotebook != NULL);
	gtk_notebook_set_current_page (pNotebook, (Pressed) ? 0 : 1);

	if (Pressed == TRUE)
	{
	/* update the action from the controls */
	m_pActionCurrent->Type = ACTION_TYPE_SWITCH;
	/* TODO: fix to allow MOUSEBUTTON also */

	pRadioButton = (GtkToggleButton*) glade_xml_get_widget (gok_settingsdialog_get_xml(), 
			"radiobuttonSwitch1");
	g_assert (pRadioButton != NULL);
	if (gtk_toggle_button_get_active (pRadioButton) == TRUE)
	{
		m_pActionCurrent->Number = 1;
	}
	else
	{
		pRadioButton = (GtkToggleButton*) glade_xml_get_widget (gok_settingsdialog_get_xml(), 
				"radiobuttonSwitch2");
		g_assert (pRadioButton != NULL);
		if (gtk_toggle_button_get_active (pRadioButton) == TRUE)
		{
			m_pActionCurrent->Number = 2;
		}
		else
		{
			pRadioButton = (GtkToggleButton*) glade_xml_get_widget (
					gok_settingsdialog_get_xml(), "radiobuttonSwitch3");
			g_assert (pRadioButton != NULL);
			if (gtk_toggle_button_get_active (pRadioButton) == TRUE)
			{
				m_pActionCurrent->Number = 3;
			}
			else
			{
				pRadioButton = (GtkToggleButton*) glade_xml_get_widget (
						gok_settingsdialog_get_xml(), "radiobuttonSwitch4");
				g_assert (pRadioButton != NULL);
				if (gtk_toggle_button_get_active (pRadioButton) == TRUE)
				{
					m_pActionCurrent->Number = 4;
				}
				else
				{
					m_pActionCurrent->Number = 5;
				}
			}
		}
	}

	pRadioButton = (GtkToggleButton*) glade_xml_get_widget (gok_settingsdialog_get_xml(), 
			"radiobuttonPress");
	g_assert (pRadioButton != NULL);
	if (gtk_toggle_button_get_active (pRadioButton) == TRUE)
	{
		m_pActionCurrent->State = ACTION_STATE_PRESS;
	}
	else
	{
		m_pActionCurrent->State = ACTION_STATE_RELEASE;
	}

	pSpinButton = (GtkSpinButton*) glade_xml_get_widget (gok_settingsdialog_get_xml(), 
			"spinDelay");
	g_assert (pSpinButton != NULL);
	m_pActionCurrent->Rate = gtk_spin_button_get_value_as_int (pSpinButton);
	}
	
	gok_page_actions_enable_switch_controls (Pressed);
}

/**
* gok_page_actions_toggle_type_valuator
* @Pressed: State of the toggle button.
*
* The radio button for the type 'valuator' has been toggled.
* Hide or show the controls for the valuator actions.
**/
void gok_page_actions_toggle_type_valuator (gboolean Pressed)
{
	GtkSpinButton* pSpinButton;

	g_assert (m_pActionCurrent != NULL);

	if (Pressed == TRUE)
	{
	  /* update the action from the controls */
	  m_pActionCurrent->Type = ACTION_TYPE_DWELL;
	}	
	pSpinButton = (GtkSpinButton*) glade_xml_get_widget (gok_settingsdialog_get_xml(), 
			"pointer_delay_spinbutton");
	g_assert (pSpinButton != NULL);
	m_pActionCurrent->Rate = gtk_spin_button_get_value_as_int (pSpinButton);

	gok_page_actions_enable_valuator_controls (Pressed);
}

/**
* gok_page_actions_button_clicked_change_name
* 
* The button "change name" has been clicked so allow the user to change the
* name of the action.
**/
void gok_page_actions_button_clicked_change_name ()
{
	GtkWidget* pDialogChangeName;
	GtkWidget* pLabel;
	GtkWidget* pEntryNewName;
	gint response;
	const gchar* pNewName;
	GtkWidget* pDialog;
	GokAction* pAction;
	gboolean bActionNameExists;

	g_assert (m_pActionCurrent != NULL);

	/* create the 'new action name' dialog */
	pDialogChangeName = gtk_dialog_new_with_buttons (_("GOK Action Name"),
												GTK_WINDOW(gok_settingsdialog_get_window()),
												GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
												GTK_STOCK_OK, GTK_RESPONSE_OK,
												GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
	/* add a text label */
	pLabel = gtk_label_new (_("Change the action name:"));
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
	gtk_entry_set_text (GTK_ENTRY(pEntryNewName), m_pActionCurrent->pDisplayName);
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
		/* check if the new action name is empty */
		if (strlen (pNewName) == 0)
		{
			pDialog = gtk_message_dialog_new (GTK_WINDOW(pDialogChangeName),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				_("Action name can't be empty.\nPlease enter a new action name."));

			gtk_window_set_title (GTK_WINDOW (pDialog), _("GOK Action Name"));
			gtk_dialog_run (GTK_DIALOG (pDialog));
			gtk_widget_destroy (pDialog);
		}
		else
		{
			/* check if the action name already exists */
			bActionNameExists = FALSE;
			pAction = gok_action_get_first_action();
			while (pAction != NULL)
			{
				if (strcmp (pAction->pDisplayName, pNewName) == 0)
				{
					pDialog = gtk_message_dialog_new (GTK_WINDOW(pDialogChangeName),
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_ERROR,
						GTK_BUTTONS_CLOSE,
						_("Sorry, that action name already exists.\nPlease enter a new action name"));
					
					gtk_window_set_title (GTK_WINDOW (pDialog), _("GOK Action Name"));
					gtk_dialog_run (GTK_DIALOG (pDialog));
					gtk_widget_destroy (pDialog);
					
					bActionNameExists = TRUE;
					break;
				}
				pAction = pAction->pActionNext;
			}
			
			if (bActionNameExists == FALSE)
			{
				/* was the name changed? */
				if (strcmp (m_pActionCurrent->pDisplayName, pNewName) != 0)
				{
					/* yes the name was changed */
					g_free (m_pActionCurrent->pDisplayName);
					m_pActionCurrent->pDisplayName = (gchar*)g_malloc (strlen (pNewName) + 1);
					strcpy (m_pActionCurrent->pDisplayName, pNewName);
		
					/* add the new name to the combo box list */
					gok_page_actions_fill_combo_action_names();
					
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
* gok_page_actions_button_clicked_new
* 
* The button "new action" has been clicked so add a new action.
**/
void gok_page_actions_button_clicked_new ()
{
	GokAction* pActionNew;
	int count;
	gchar bufferName[200];
	gchar bufferDisplayName[200];
	
	g_assert (m_pActionCurrent != NULL);
	
	/* create the new action default name */
	/* it will be something like "new action 1" */
	for (count = 1; count < 100; count++)
	{
		sprintf (bufferDisplayName, _("New Action %d"), count);
		sprintf (bufferName, "new_action_%d", count);
		if ((gok_action_find_action (bufferDisplayName, TRUE) == NULL) &&
			(gok_action_find_action (bufferName, FALSE) == NULL))
		{
			break;
		}
	}
	
	pActionNew = gok_action_new();
	pActionNew->pName = (gchar*)g_malloc (strlen (bufferName) + 1);
	strcpy (pActionNew->pName, bufferName);
	pActionNew->pDisplayName = (gchar*)g_malloc (strlen (bufferDisplayName) + 1);
	strcpy (pActionNew->pDisplayName, bufferDisplayName);
	
	/* make the new action the same as the current action */
	if (m_pActionCurrent != NULL)
	{
		pActionNew->Type = m_pActionCurrent->Type;
		pActionNew->TypeBackup = m_pActionCurrent->Type;
		pActionNew->State = m_pActionCurrent->State;
		pActionNew->StateBackup = m_pActionCurrent->State;
		pActionNew->Number = m_pActionCurrent->Number;
		pActionNew->NumberBackup = m_pActionCurrent->Number;
		pActionNew->Rate = m_pActionCurrent->Rate;
		pActionNew->RateBackup = m_pActionCurrent->Rate;
		pActionNew->bKeyAveraging = m_pActionCurrent->bKeyAveraging;
	}
	
	pActionNew->bPermanent = FALSE;
	pActionNew->bNewAction = TRUE;
	
	gok_action_add_action (pActionNew);

	m_pActionCurrent = pActionNew;

	/* update the combo box with the new action name */
	gok_page_actions_fill_combo_action_names();
	gok_page_actions_update_controls (m_pActionCurrent);
	
	/* mark the actions as 'changed' */
	m_bChanged = TRUE;
}

/**
* gok_page_actions_button_clicked_delete
* 
* The button "delete action" has been clicked so delete the current action.
**/
void gok_page_actions_button_clicked_delete ()
{
	GtkWidget* pDialog;
	GtkWidget* pComboEntry;
	const gchar* pNameAction;
	gchar buffer[200];
	gint result;
	GokAction* pAction;
	GokAction* pActionDisplayed;
	
	pComboEntry = glade_xml_get_widget (gok_settingsdialog_get_xml(), "entryActionName");
	g_assert (pComboEntry != NULL);
	pNameAction = gtk_entry_get_text (GTK_ENTRY(pComboEntry));

	sprintf (buffer, _("Do you wish to delete this action (%s)?"), pNameAction);

	pDialog = gtk_message_dialog_new ((GtkWindow*)gok_settingsdialog_get_window(),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
			buffer);
	
	gtk_window_set_title (GTK_WINDOW (pDialog), _("GOK Delete Action"));
	result = gtk_dialog_run (GTK_DIALOG (pDialog));
	gtk_widget_destroy (pDialog);

	if (result == GTK_RESPONSE_NO)
	{
		return;
	}
	
	pActionDisplayed = NULL;
	pAction = gok_action_get_first_action();
	while (pAction != NULL)
	{
		if (strcmp (pAction->pDisplayName, pNameAction) == 0)
		{
			/* found the action */
			/* make sure this is not a permanent action */
			g_assert (pAction->bPermanent == FALSE);
			
			/* delete the action */
			if (pActionDisplayed == NULL)
			{
				pActionDisplayed = pAction->pActionNext;
			}
			
			gok_action_delete_action (pAction);
			break;
		}
		
		pActionDisplayed = pAction;
		pAction = pAction->pActionNext;
	}

	m_pActionCurrent = pActionDisplayed;
	
	/* update the combo box with all the action names */
	gok_page_actions_fill_combo_action_names();
	gok_page_actions_update_controls (m_pActionCurrent);

	/* mark the actions as 'changed' */
	m_bChanged = TRUE;
}

/**
* gok_page_actions_fill_combo_action_names
*
* Fills the combo box that displays the action names.
**/
void gok_page_actions_fill_combo_action_names ()
{
	GtkWidget* pComboBox;
	GtkWidget* pEntry;
	GokAction* pAction;
	GList* items = NULL;
	gchar* pActionName;

	/* ignore the 'entry text changed' message generated by this routine */
	m_bIgnoreChangeName = TRUE;
	
	/* find the combo box */
	pComboBox = glade_xml_get_widget (gok_settingsdialog_get_xml(), "comboActionNames");
	g_assert (pComboBox != NULL);

	/* create a list of action display names */
	pAction = gok_action_get_first_action();
	while (pAction != NULL)
	{
		items = g_list_append (items, pAction->pDisplayName);
		pAction = pAction->pActionNext;
	}
	
	/* are there any action names? */
	if (items == NULL)
	{
		/* no, so empty the combo box */
		items = g_list_append (items, "");
	}

	/* sort the action names */
	items = g_list_sort (items, gok_settingsdialog_sort);

	gtk_combo_set_popdown_strings (GTK_COMBO (pComboBox), items);
	g_list_free (items);

	/* display the current action name in the entry control of the combo */
	if (m_pActionCurrent != NULL)
	{
		pActionName = m_pActionCurrent->pDisplayName;
	}
	else
	{
		pActionName = "";
	}
	pEntry = glade_xml_get_widget (gok_settingsdialog_get_xml(), "entryActionName");
	g_assert (pEntry != NULL);
	gtk_entry_set_text (GTK_ENTRY(pEntry), pActionName);
	
	m_bIgnoreChangeName = FALSE;
}

/**
* gok_page_actions_action_changed
* @pEditControl: Pointer to the edit control that contains the names of
* the actions.
*
* The user has selected a new action from the combo box so update the
* page's controls with the new action data.
**/
void gok_page_actions_action_changed (GtkEditable* pEditControl)
{
	GokAction* pAction;
	gchar* pStrComboActionName;

	pAction = NULL;
	
	/* ignore this call if we are just changing the name of the action */
	if (m_bIgnoreChangeName == TRUE)
	{
		return;
	}

	/* get the name of the action from the combo box */
	pStrComboActionName = gtk_editable_get_chars (pEditControl, 0, -1);
	if (strlen (pStrComboActionName) == 0)
	{
		g_free (pStrComboActionName);
	}
	else
	{
		/* find the action in our list of actions */
		pAction = gok_action_find_action (pStrComboActionName, TRUE);
		if (pAction == NULL)
		{
			gok_log_x ("Action name (%s) from combo not found in action list!", pStrComboActionName);
		}
		g_free (pStrComboActionName);
	}
	
	/* store the pointer */
	m_pActionCurrent = pAction;
	
	/* update the controls to reflect this action */

	gok_page_actions_update_controls (pAction);
}

/**
* gok_page_actions_input_device_changed
* @pEditControl: Pointer to the edit control that contains the names of
* the input devices.
*
* The user has selected a new input device from the combo box so update the
* page's controls with the new input device data.
**/
void gok_page_actions_input_device_changed (GtkEditable* pEditControl)
{
	gchar* pStrComboInputDeviceName;
	GokInput *pInput;

	/* get the name of the input device from the combo box */
	pStrComboInputDeviceName = gtk_editable_get_chars (pEditControl, 0, -1);
	if (strlen (pStrComboInputDeviceName) == 0) {
		g_free (pStrComboInputDeviceName);
	}
	else
	{
		/* find the device in our list of input devices */
	        
		pInput = gok_input_find_by_name (pStrComboInputDeviceName, TRUE);
		if (!pInput) {
			gok_log_x ("Input Device name (%s) from combo not found in device list!", pStrComboInputDeviceName);
			gtk_editable_delete_text (pEditControl, 0, -1);
		}
		else {
			gok_page_actions_set_changed ( TRUE );
			gok_input_set_extension_device_by_name (
				pStrComboInputDeviceName);
			gok_input_free (pInput);
		}
		g_free (pStrComboInputDeviceName);
	}
}

/**
* gok_page_actions_update_controls
* @pAction: Controls are changed to display this action.
*
* Updates the controls so they reflect the given action.
**/
void gok_page_actions_update_controls (GokAction* pAction)
{
	GtkWidget* pRadioButton;
	GtkWidget* pSpinControl;
	GtkWidget* pCheckbox;
	GtkWidget* pButton;
	
	/* if no action then disable all the controls */
	if (pAction == NULL)
	{
		gok_page_actions_enable_switch_controls (TRUE);
		gok_page_actions_enable_valuator_controls (FALSE);
		m_ControlType = ACTION_TYPE_SWITCH;
		return;
	}
	
	/* is this a permanent action? */
	if (pAction->bPermanent == TRUE)
	{
		/* don't allow user to edit the action name */
		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "buttonChangeName");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, FALSE);
			
		gok_page_actions_enable_radios_type (FALSE);
		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "buttonDeleteAction");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, FALSE);
	}
	else
	{
		/* not permanent, enable controls for changing action type */
		gok_page_actions_enable_radios_type (TRUE);
						
		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "buttonDeleteAction");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, TRUE);
						
		/* allow user to edit the action name */
		pButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "buttonChangeName");
		g_assert (pButton != NULL);
		gtk_widget_set_sensitive (pButton, TRUE);
	}

	/* update the values for all controls */	
	switch (pAction->Number)
	{
		case 1:
			pRadioButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "radiobuttonSwitch1");
			g_assert (pRadioButton != NULL);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pRadioButton), TRUE);

			break;
	
		case 2:
			pRadioButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "radiobuttonSwitch2");
			g_assert (pRadioButton != NULL);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pRadioButton), TRUE);

			break;
	
		case 3:
			pRadioButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "radiobuttonSwitch3");
			g_assert (pRadioButton != NULL);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pRadioButton), TRUE);

			break;
	
		case 4:
			pRadioButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "radiobuttonSwitch4");
			g_assert (pRadioButton != NULL);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pRadioButton), TRUE);

			break;
	
		case 5:
			pRadioButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "radiobuttonSwitch5");
			g_assert (pRadioButton != NULL);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pRadioButton), TRUE);

			break;
	
		default:
			break;
	}

	switch (pAction->State)
	{
		case ACTION_STATE_PRESS:
			pRadioButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "radiobuttonPress");
			g_assert (pRadioButton != NULL);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pRadioButton), TRUE);
			break;
			
		case ACTION_STATE_RELEASE:
			pRadioButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "radiobuttonRelease");
			g_assert (pRadioButton != NULL);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pRadioButton), TRUE);
			break;
			
		default:
			gok_log_x ("default hit!\n");
			break;
	}
	
	pSpinControl = glade_xml_get_widget (gok_settingsdialog_get_xml(), "spinDelay");
	g_assert (pSpinControl != NULL);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(pSpinControl), pAction->Rate);

	pSpinControl = glade_xml_get_widget (gok_settingsdialog_get_xml(), "pointer_delay_spinbutton");
	g_assert (pSpinControl != NULL);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(pSpinControl), pAction->Rate);
	
	pCheckbox = glade_xml_get_widget (gok_settingsdialog_get_xml(), "checkKeyAverage");
	g_assert (pSpinControl != NULL);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pCheckbox), pAction->bKeyAveraging);

	/* enable/disable the appropriate controls */
	switch (pAction->Type)
	{
	        case ACTION_TYPE_SWITCH:
		case ACTION_TYPE_MOUSEBUTTON:
		case ACTION_TYPE_KEY:
			/* enable/disable the controls for switch actions */
			pRadioButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "radiobuttonTypeSwitch");
			g_assert (pRadioButton != NULL);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pRadioButton), TRUE);
			gok_page_actions_enable_switch_controls (TRUE);
			gok_page_actions_enable_valuator_controls (FALSE);
			break;
							
		case ACTION_TYPE_DWELL:
			pRadioButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "radiobuttonTypeValuator");
			g_assert (pRadioButton != NULL);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pRadioButton), TRUE);
							
			pRadioButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "activate_on_dwell_button");
			g_assert (pRadioButton != NULL);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pRadioButton), TRUE);
			gok_page_actions_enable_switch_controls (FALSE);
			gok_page_actions_enable_valuator_controls (TRUE);
			break;
		case ACTION_TYPE_MOUSEPOINTER:
			pRadioButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "radiobuttonTypeValuator");
			g_assert (pRadioButton != NULL);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pRadioButton), TRUE);
			pRadioButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "activate_on_enter_button");
			g_assert (pRadioButton != NULL);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pRadioButton), TRUE);
							
			gok_page_actions_enable_switch_controls (FALSE);
			gok_page_actions_enable_valuator_controls (TRUE);
			break;
							
		default:
			gok_log_x ("default hit!\n");
			break;
	}
	pRadioButton = glade_xml_get_widget (gok_settingsdialog_get_xml(), "xinput_device_button");
	g_assert (pRadioButton != NULL);
	switch (pAction->Type)
	{
	case ACTION_TYPE_SWITCH:
	case ACTION_TYPE_KEY:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pRadioButton), TRUE);
		break;
	case ACTION_TYPE_MOUSEBUTTON:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pRadioButton), FALSE);
		break;
	default:
		break;
	}
}


/**
* gok_page_actions_enable_switch_controls
* @bTrueFalse: State that the switch controls should be set to.
*
* Enables/disables the controls for switch actions.
**/
void gok_page_actions_enable_switch_controls (gboolean bTrueFalse)
{
	GtkWidget* pFrame, *pNotebook;

	pFrame = glade_xml_get_widget (gok_settingsdialog_get_xml(), "frameSwitch");
	g_assert (pFrame != NULL);
	
	gtk_widget_set_sensitive (pFrame, bTrueFalse);
	
	if (bTrueFalse) 
	{
		pNotebook = glade_xml_get_widget (gok_settingsdialog_get_xml(), "action_type_notebook");
		g_assert (pNotebook != NULL);
		
		gtk_notebook_set_current_page (GTK_NOTEBOOK (pNotebook), 0);
	}
}

/**
* gok_page_actions_enable_valuator_controls
* @bTrueFalse: State the controls should be set to.
*
* Enables/disables the controls for mousebutton actions.
**/
void gok_page_actions_enable_valuator_controls (gboolean bTrueFalse)
{
	GtkWidget* pFrame, *pNotebook;

	pFrame = glade_xml_get_widget (gok_settingsdialog_get_xml(), "frameSwitch");
	g_assert (pFrame != NULL);
	
	gtk_widget_set_sensitive (pFrame, bTrueFalse);

	if (bTrueFalse)
	{
		pNotebook = glade_xml_get_widget (gok_settingsdialog_get_xml(), "action_type_notebook");
		g_assert (pNotebook != NULL);
		
		gtk_notebook_set_current_page (GTK_NOTEBOOK (pNotebook), 1);
	}
}

/**
* gok_page_actions_enable_radios_type
* @bTrueFalse: State the controls should be set to.
*
* Enables/disables the radio buttons for action type.
**/
void gok_page_actions_enable_radios_type (gboolean bTrueFalse)
{
	GtkWidget* pWidget;

	pWidget = glade_xml_get_widget (gok_settingsdialog_get_xml(), "labelActionName");
	g_assert (pWidget != NULL);
	gtk_widget_set_sensitive (pWidget, bTrueFalse);
	
	pWidget = glade_xml_get_widget (gok_settingsdialog_get_xml(), "radiobuttonTypeSwitch");
	g_assert (pWidget != NULL);
	gtk_widget_set_sensitive (pWidget, bTrueFalse);
	
	pWidget = glade_xml_get_widget (gok_settingsdialog_get_xml(), "radiobuttonTypeValuator");
	g_assert (pWidget != NULL);
	gtk_widget_set_sensitive (pWidget, bTrueFalse);
	
}

/**
* gok_page_actions_set_number
* @NumberSwitch: Number of the switch selected.
**/
void gok_page_actions_set_number (gint NumberSwitch)
{
	g_assert (m_pActionCurrent != NULL);

	m_pActionCurrent->Number = NumberSwitch;
	return;	
}

/**
* gok_page_actions_set_state
* @State: State selected.
**/
void gok_page_actions_set_state (gint State)
{
	g_assert (m_pActionCurrent != NULL);
	
	m_pActionCurrent->State = State;
}

/**
* gok_page_actions_set_type
* @Type: Type selected.
**/
void gok_page_actions_set_type (gint Type)
{
	g_assert (m_pActionCurrent != NULL);
	
	m_pActionCurrent->Type = Type;
}

/**
* gok_page_actions_set_is_corepointer
* @bTrueFalse: Whether or not the action uses the core pointer.
*
* Sets or clears the 'changed' flag.
**/
void gok_page_actions_set_is_corepointer (gboolean bCorePointer)
{
	g_assert (m_pActionCurrent != NULL);
	
	/* Never override dwell action type (fixes bug 136756). 
	NOTE: Currently the dwell action has a redundant free switch 1 
	press action.  This is good for dwell users who can operate a switch 
	sometimes. It might be best not to do this, and to allow aggregation 
	of actions, or allow multiple actions to be specified for a single 
	access method operation.
	TODO: make dwell action press-ability less kludgy */
	if ((m_ControlType ==  ACTION_TYPE_SWITCH) && (m_pActionCurrent->Type != ACTION_TYPE_DWELL))
	{
		if (bCorePointer)
			m_pActionCurrent->Type = ACTION_TYPE_MOUSEBUTTON;
		else
			m_pActionCurrent->Type = ACTION_TYPE_SWITCH;
	}
}

/**
* gok_page_actions_set_rate
* @Rate: Rate selected.
**/
void gok_page_actions_set_rate (gint Rate)
{
	g_assert (m_pActionCurrent != NULL);
	
	m_pActionCurrent->Rate = Rate;
}

/**
* gok_page_actions_pointer_keyaverage
* @Rate: Rate selected.
**/
void gok_page_actions_pointer_keyaverage (gboolean OnOff)
{
	g_assert (m_pActionCurrent != NULL);
	
	m_pActionCurrent->bKeyAveraging = OnOff;
}

/**
* gok_page_actions_get_changed
*
* returns: TRUE if the actions have been changed.
**/
gboolean gok_page_actions_get_changed()
{
	return m_bChanged;
}

/**
* gok_page_actions_set_changed
* @bTrueFalse: State that you want the 'changed' flag set to.
*
* Sets or clears the 'changed' flag.
**/
void gok_page_actions_set_changed (gboolean bTrueFalse)
{
	m_bChanged = bTrueFalse;
}
