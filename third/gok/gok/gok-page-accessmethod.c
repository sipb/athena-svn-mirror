/* gok-page-accessmethod.c
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
#include "gok-page-accessmethod.h"
#include "gok-page-feedbacks.h"
#include "gok-page-actions.h"
#include "gok-data.h"
#include "gok-settings-dialog.h"
#include "gok-log.h"
#include "gok-action.h"
#include "gok-feedback.h"

typedef struct GokFilledCombo{
	gboolean bFilled;
	gint Fillwith;
	gint Qualifier;
	gchar* pNameAccessMethod;
	gchar* pNameControl;
	GtkWidget* pCombo;
	struct GokFilledCombo* pGokFilledComboNext;
} GokFilledCombo;

/* settings for the access method */
static gchar m_NameAccessMethod [MAX_ACCESS_METHOD_NAME + 1];
static GokAccessMethod* m_pAccessMethod;

/* backup of the settings (in case the "cancel" button is clicked) */
static gchar m_NameAccessMethodBackup [MAX_ACCESS_METHOD_NAME + 1];

/* this flag will be TRUE when we are changing the text in the combo entry */
static gboolean m_bUsChangingName;

/* pointer to the first in a list of combo boxes that are filled */
static GokFilledCombo* m_pGokFilledComboFirst;

/**
* gok-settings-page-accessmethod-initialize
* @pWindowSettings: Pointer to the settings dialog window.
*
* Initializes this page of the gok settings dialog. This must be called
* prior to any calls on this page.
*
* returns: TRUE if the page was properly initialized, FALSE if not.
**/
gboolean gok_page_accessmethod_initialize (GladeXML* xml)
{
	GokAccessMethod* pAccessMethod;
	GtkWidget* pComboBox;
	GtkWidget* pEntryAccessMethodName;
	GList* items = NULL;
		
	g_assert (xml != NULL);
	
	/* initialize this data */
	strcpy (m_NameAccessMethod, gok_data_get_name_accessmethod());
	m_bUsChangingName = FALSE;
	m_pAccessMethod = NULL;
	m_pGokFilledComboFirst = NULL;
	
	/* backup the initial settings */
	gok_page_accessmethod_backup();

	/* add the names of all the access methods to the combo box */
	pAccessMethod = gok_scanner_get_first_access_method();
	g_assert (pAccessMethod != NULL);
	
	pComboBox = glade_xml_get_widget (xml,  "comboAccessMethods");
	g_assert (pComboBox != NULL);
	
	while (pAccessMethod != NULL)
	{
		items = g_list_append (items, pAccessMethod->DisplayName);
		pAccessMethod = pAccessMethod->pAccessMethodNext;
	}

	/* sort the names */
	items = g_list_sort (items, gok_settingsdialog_sort);

	m_bUsChangingName = TRUE;
	gtk_combo_set_popdown_strings (GTK_COMBO (pComboBox), items);
	g_list_free (items);

	/* display the current access method */
	pEntryAccessMethodName = glade_xml_get_widget (xml, "entryAccessMethodName");
	g_assert (pEntryAccessMethodName != NULL);
	gtk_entry_set_text (GTK_ENTRY (pEntryAccessMethodName), gok_page_accessmethod_get_displayname (m_NameAccessMethod));
	m_bUsChangingName = FALSE;

	/* update the accessmethod controls */
	gok_page_accessmethod_method_changed (GTK_EDITABLE (pEntryAccessMethodName));

	return TRUE;
}

/**
* gok_page_accessmethod_close
* 
* Frees all memory allocated by this page.
**/
void gok_page_accessmethod_close()
{
	GokFilledCombo* pGokFilledCombo;
	GokFilledCombo* pGokFilledComboTemp;

	pGokFilledCombo = m_pGokFilledComboFirst;
	while (pGokFilledCombo != NULL)
	{
		pGokFilledComboTemp = pGokFilledCombo;
		pGokFilledCombo = pGokFilledCombo->pGokFilledComboNext;
		
		if (pGokFilledComboTemp->pNameAccessMethod != NULL)
		{
			g_free (pGokFilledComboTemp->pNameAccessMethod);
		}
		if (pGokFilledComboTemp->pNameControl != NULL)
		{
			g_free (pGokFilledComboTemp->pNameControl);
		}
		g_free (pGokFilledComboTemp);
	}
}

/**
* gok_page_accessmethod_get_displayname
* @NameAccessMethod: Name of the access method.
*
* returns: The display name of the given access method.
**/
gchar* gok_page_accessmethod_get_displayname (gchar* NameAccessMethod)
{
	GokAccessMethod* pAccessMethod;

	pAccessMethod = gok_scanner_get_first_access_method();
	while (pAccessMethod != NULL)
	{
		if (strcmp (pAccessMethod->Name, NameAccessMethod) == 0)
		{
			return pAccessMethod->DisplayName;
		}
		pAccessMethod = pAccessMethod->pAccessMethodNext;
	}

	gok_log_x ("Can't find display name of: %s\n", NameAccessMethod);
	return "Access Method";
}

/**
* gok_page_accessmethod_get_name
* @DisplayNameAccessMethod: Display name of the access method.
*
* returns: The name of the given access method.
**/
gchar* gok_page_accessmethod_get_name (gchar* DisplayNameAccessMethod)
{
	GokAccessMethod* pAccessMethod;

	pAccessMethod = gok_scanner_get_first_access_method();
	while (pAccessMethod != NULL)
	{
		if (strcmp (pAccessMethod->DisplayName, DisplayNameAccessMethod) == 0)
		{
			return pAccessMethod->Name;
		}
		pAccessMethod = pAccessMethod->pAccessMethodNext;
	}
	
	gok_log_x ("Can't find name of: %s\n", DisplayNameAccessMethod);
	return "Access Method";
}

/**
* gok_page_accessmethod_apply
* 
* Updates the gok data with values from the controls.
*
* returns: TRUE if any settings have changed, FALSE if no settings have changed.
**/
gboolean gok_page_accessmethod_apply ()
{
	gboolean bDataChanged;
	GokAccessMethod* pAccessMethod;

	bDataChanged = FALSE;

	/* update the gok data with any new settings */
	if (strcmp (gok_data_get_name_accessmethod(), m_NameAccessMethod) != 0)
	{
		bDataChanged = TRUE;
		gok_data_set_name_accessmethod (m_NameAccessMethod);
		
		gok_scanner_change_method (m_NameAccessMethod);
	}
	
	/* update the access methods with any new settings */
	pAccessMethod = gok_scanner_get_first_access_method();				
	while (pAccessMethod != NULL)
	{
		/* get the controls for the access method */
		if (gok_page_accessmethod_apply_controls (pAccessMethod->pControlOperation, pAccessMethod->Name) == TRUE)
		{
			bDataChanged = TRUE;
		}
		if (gok_page_accessmethod_apply_controls (pAccessMethod->pControlFeedback, pAccessMethod->Name) == TRUE)
		{
			bDataChanged = TRUE;
		}
		if (gok_page_accessmethod_apply_controls (pAccessMethod->pControlOptions, pAccessMethod->Name) == TRUE)
		{
			bDataChanged = TRUE;
		}

		pAccessMethod = pAccessMethod->pAccessMethodNext;
	}

	return bDataChanged;
}

/**
* gok_page_accessmethod_apply_controls
* @pControl: Pointer to the control that you want to get the data from.
* @NameAccessMethod: Name of the access method that the control belongs to.
* 
* Updates the gok data with values from the controls. This function is recursive
* and gets the data from the next and child controls of the one given.
*
* returns: TRUE if any conrol value has changed. Returns FALSE if all control values
* are the same.
**/
gboolean gok_page_accessmethod_apply_controls (GokControl* pControl, gchar* NameAccessMethod)
{
	const gchar* pText;
	const gchar* pType;
	GtkWidget* pEntry;
	gint valueNew;
	gboolean bTrueFalse;
	GSList* pRadioList;
	GtkRadioButton* pRadioButton;
	GokControl* pRadioControl;
	gboolean codeReturned;
	
	codeReturned = FALSE;
	
	while (pControl != NULL)
	{
		/* only consider controls that have associated widgets */
		if ((pControl->pWidget != NULL) &&
			(pControl->Name != NULL))
		{
			switch (pControl->Type)
			{
				case CONTROL_TYPE_COMBOBOX:
					pEntry = GTK_COMBO(pControl->pWidget)->entry;
					pText = gtk_entry_get_text (GTK_ENTRY(pEntry));

					/* convert the display name to the static (gconf) name */
					pType = g_object_get_data (G_OBJECT(pControl->pWidget), "type");
					if (pType != NULL)
					{
						if (strcmp (pType, "actions") == 0)
						{
							pText = gok_action_get_name ((gchar*)pText);
						}
						else if (strcmp (pType, "feedbacks") == 0)
						{
							pText = gok_feedback_get_name ((gchar*)pText);
						}
						else if (strcmp (pType, "sounds") == 0)
						{
							gok_log_x ("conversion of display name to name not implemented yet.");
						}
						else if (strcmp (pType, "options") == 0)
						{
							gok_log_x ("conversion of display name to name not implemented yet.");
						}
						else
						{
							gok_log_x ("combo has no type!");
						}

						if (gok_data_set_setting (NameAccessMethod, pControl->Name, 0, (gchar*)pText) == TRUE)
						{
							codeReturned = TRUE;
						}
					}
					break;
					
				case CONTROL_TYPE_CHECKBUTTON:
					bTrueFalse = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(pControl->pWidget));
					if (gok_data_set_setting (NameAccessMethod, pControl->Name, bTrueFalse, NULL) == TRUE)
					{
						codeReturned = TRUE;
					}
					break;
					
				case CONTROL_TYPE_RADIOBUTTON:
					pRadioList = gtk_radio_button_get_group (GTK_RADIO_BUTTON(pControl->pWidget));
					while (pRadioList != NULL)
					{
						pRadioButton = GTK_RADIO_BUTTON(pRadioList->data);
						g_assert (pRadioButton != NULL);
						if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(pRadioButton)) == TRUE)
						{
							pRadioControl = (GokControl*)gtk_object_get_data (GTK_OBJECT(pRadioButton), "control");
							g_assert (pRadioControl != NULL);
							if (gok_data_set_setting (NameAccessMethod, pControl->Name, pRadioControl->Value, NULL) == TRUE)
							{
								codeReturned = TRUE;
							}
							break;
						}
						pRadioList = pRadioList->next;
					}
					break;
					
				case CONTROL_TYPE_SPINBUTTON:
					valueNew = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(pControl->pWidget));
					if (gok_data_set_setting (NameAccessMethod, pControl->Name, valueNew, NULL) == TRUE)
					{
						codeReturned = TRUE;
					}
					break;
					
				default:
					gok_log_x ("Default hit!");
					break;
			}
		}

		if (pControl->pControlChild != NULL)
		{
			if (gok_page_accessmethod_apply_controls (pControl->pControlChild, NameAccessMethod) == TRUE)
			{
				codeReturned = TRUE;
			}
		}
		
		pControl = pControl->pControlNext;
	}

	return codeReturned;
}

/**
* gok_page_accessmethod_revert
* 
* Revert to the backup settings for this page and store them in the gok_data.
*
* returns: TRUE if any of the settings have changed, FALSE 
* if they are all still the same.
**/
gboolean gok_page_accessmethod_revert ()
{
	gboolean bDataChanged;
	GokAccessMethod* pAccessMethod;

	bDataChanged = FALSE;
	
	/* get the original settings */
	if (strcmp (m_NameAccessMethod, m_NameAccessMethodBackup) != 0)
	{
		bDataChanged = TRUE;
		strcpy (m_NameAccessMethod, m_NameAccessMethodBackup);
		gok_data_set_name_accessmethod (m_NameAccessMethod);
		gok_page_accessmethod_change_controls (m_NameAccessMethod);
		gok_scanner_change_method (m_NameAccessMethod);
	}

	pAccessMethod = gok_scanner_get_first_access_method();				
	while (pAccessMethod != NULL)
	{
		gok_page_accessmethod_update_controls (pAccessMethod->Name, pAccessMethod->pControlOperation);
		gok_page_accessmethod_update_controls (pAccessMethod->Name, pAccessMethod->pControlFeedback);
		gok_page_accessmethod_update_controls (pAccessMethod->Name, pAccessMethod->pControlOptions);

		pAccessMethod = pAccessMethod->pAccessMethodNext;
	}
	
	return bDataChanged;
}

/**
* gok_page_accessmethod_backup
* 
* Copies all the member settings to backup.
**/
void gok_page_accessmethod_backup ()
{
	strcpy (m_NameAccessMethodBackup, m_NameAccessMethod);
}

/**
* gok_page_accessmethod_method_changed
* @pEditControl: Pointer to the combo box edit control.
* 
* The user has selected a new access method so display all the controls
* associated with that access method.
**/
void gok_page_accessmethod_method_changed (GtkEditable* pEditControl)
{
	gchar* pStringMethodName;
	gchar* pAccessMethodName;
	
	/* first, get the name of the access method */
	pStringMethodName = gtk_editable_get_chars (pEditControl, 0, -1);
	if (strlen (pStringMethodName) != 0)
	{
		/* did the user change the name or did we? */
		if (m_bUsChangingName == FALSE)
		{
			/* user changed the name so update the controls */
			/* convert the display name to the access method name */
			pAccessMethodName = gok_page_accessmethod_get_name (pStringMethodName);
			gok_page_accessmethod_change_controls (pAccessMethodName);		

			/* fill the combo box controls */
			gok_page_accessmethod_fill_combos (FALSE);
		}
	}
	g_free (pStringMethodName);
}

/**
* gok_page_accessmethod_update_controls
* @NameAccessMethod: Name of the access method.
* @pControl: Pointer to the control that will be updated.
* 
* Updates all the controls on the page with values from the settings.
**/
void gok_page_accessmethod_update_controls (gchar* NameAccessMethod, GokControl* pControl)
{
	gchar* settingString;
	gint settingInt;
	GtkWidget* pEntry;

	while (pControl != NULL)
	{
		if (pControl->Name != NULL)
		{
			switch (pControl->Type)
			{
				case CONTROL_TYPE_COMBOBOX:
					if (gok_data_get_setting (NameAccessMethod, pControl->Name, &settingInt, &settingString) == TRUE)
					{
						if (pControl->Fillwith == CONTROL_FILLWITH_ACTIONS)
						{
							settingString = gok_action_get_displayname (settingString);
						}
						else if (pControl->Fillwith == CONTROL_FILLWITH_FEEDBACKS)
						{
							settingString = gok_feedback_get_displayname (settingString);
						}

						if (settingString != NULL)
						{
							if (pControl->String != NULL)
							{
								g_free (pControl->String);
							}
							pControl->String = (gchar*)g_malloc (strlen (settingString) + 1);
							strcpy (pControl->String, settingString);
							
							if (pControl->pWidget != NULL)
							{
								pEntry = GTK_COMBO(pControl->pWidget)->entry;
								g_assert (pEntry != NULL);
								gtk_entry_set_text (GTK_ENTRY (pEntry), settingString);
							}
						}
					}
					break;

				case CONTROL_TYPE_CHECKBUTTON:
					if (gok_data_get_setting (NameAccessMethod, pControl->Name, &settingInt, &settingString) == TRUE)
					{
						pControl->Value = settingInt;
						if (pControl->pWidget != NULL)
						{
							gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pControl->pWidget), (gboolean)settingInt);
						}
					}
					break;

				case CONTROL_TYPE_SPINBUTTON:
					if (gok_data_get_setting (NameAccessMethod, pControl->Name, &settingInt, &settingString) == TRUE)
					{
						pControl->Value = settingInt;
						if (pControl->pWidget != NULL)
						{
							gtk_spin_button_set_value (GTK_SPIN_BUTTON(pControl->pWidget), (gdouble)settingInt);
						}
					}
					break;

				case CONTROL_TYPE_RADIOBUTTON:
					if (gok_data_get_setting (NameAccessMethod, pControl->Name, &settingInt, &settingString) == TRUE)
					{
						if (pControl->Value == settingInt)
						{
							if (pControl->pWidget != NULL)
							{
								gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pControl->pWidget), TRUE);
							}
						}
					}
					break;

				default:
					break;
			}
		}
	
		/* update any child controls */
		if (pControl->pControlChild != NULL)
		{
			gok_page_accessmethod_update_controls (NameAccessMethod, pControl->pControlChild);
		}

		pControl = pControl->pControlNext;
	}
}

/**
* gok_page_accessmethod_draw_controls
* @NameAccessMethod: Name of the access methods that needs the controls.
* @pControlParent: Pointer to the GOK control that will contain the new controls.
* @pContainter: Pointer to the container widget for the controls.
* @pControl: Pointer to the GokControl that you want drawn.
* @bShow: If TRUE then the controls will be show, if FALSE they will be hidden.
* 
* Draws the controls for the GokControl.
**/
void gok_page_accessmethod_draw_controls (gchar* NameAccessMethod, GokControl* pControlParent, GokControl* pControl, gboolean bShow)
{
	GtkWidget* pWidget;
	GtkWidget* pEntry;
	static GSList* ListRadioButtons;
	GtkObject* spinRateAdjustment;
	gchar* pControlName;
	int settingInt;
	gchar* settingString;
	GokFilledCombo* pFilledComboNew;
	GokFilledCombo* pFilledComboTemp;
	AtkObject* atko;

	while (pControl != NULL)
	{
		if (bShow == TRUE) /* show or hide the controls */
		{
			if (pControl->pWidget != NULL)
			{
				/* control is already created, so just show it */
				gtk_widget_show (pControl->pWidget);
			}
			else /* create the control */
			{
				switch (pControl->Type)
				{
					case CONTROL_TYPE_LABEL:
						pWidget = gtk_label_new (_(pControl->String));
						gtk_widget_ref (pWidget);
						gtk_object_set_data_full (GTK_OBJECT (gok_settingsdialog_get_window()), "GOKlabel", pWidget,
						                            (GtkDestroyNotify) gtk_widget_unref);
						gtk_widget_show (pWidget);
						gtk_box_pack_start (GTK_BOX (pControlParent->pWidget), pWidget, FALSE, FALSE, 0);
						break;
						
					case CONTROL_TYPE_HBOX:
						pWidget = gtk_hbox_new (FALSE, pControl->Spacing);
						gtk_widget_ref (pWidget);
						gtk_object_set_data_full (GTK_OBJECT (gok_settingsdialog_get_window()), "GOKhbox", pWidget,
						                            (GtkDestroyNotify) gtk_widget_unref);
						gtk_widget_show (pWidget);
						
						if (pControlParent->Type == CONTROL_TYPE_FRAME)
						{
							gtk_container_add (GTK_CONTAINER (pControlParent->pWidget), pWidget);
						}
						else
						{
							gtk_box_pack_start (GTK_BOX (pControlParent->pWidget), pWidget, TRUE, TRUE, 0);
							gtk_container_set_border_width (GTK_CONTAINER (pControlParent->pWidget), pControl->Border);
						}
						break;
						
					case CONTROL_TYPE_VBOX:
						pWidget = gtk_vbox_new (FALSE, pControl->Spacing);
						gtk_widget_ref (pWidget);
						gtk_object_set_data_full (GTK_OBJECT (gok_settingsdialog_get_window()), "GOKvbox", pWidget,
						                            (GtkDestroyNotify) gtk_widget_unref);
						gtk_widget_show (pWidget);
						
						if (pControlParent->Type == CONTROL_TYPE_FRAME)
						{
							gtk_container_add (GTK_CONTAINER (pControlParent->pWidget), pWidget);
						}
						else
						{
							gtk_box_pack_start (GTK_BOX (pControlParent->pWidget), pWidget, TRUE, TRUE, 0);
							gtk_container_set_border_width (GTK_CONTAINER (pControlParent->pWidget), pControl->Border);
						}
						break;
						
					case CONTROL_TYPE_COMBOBOX:
						pControlName = (pControl->Name != NULL) ? pControl->Name : "combobox";
						pWidget = gtk_combo_new ();
						gtk_widget_ref (pWidget);
						gtk_object_set_data_full (GTK_OBJECT (gok_settingsdialog_get_window()), pControlName, pWidget,
						                            (GtkDestroyNotify) gtk_widget_unref);
						atko = gtk_widget_get_accessible (pWidget);
						atk_object_set_name (atko, pControl->String ? pControl->String : "");
						gtk_widget_show (pWidget);
						gtk_box_pack_start (GTK_BOX (pControlParent->pWidget), pWidget, FALSE, FALSE, 0);

						/* make the combo's entry field NOT editable */
						pEntry = GTK_COMBO (pWidget)->entry;
						gtk_entry_set_editable (GTK_ENTRY (pEntry), FALSE);
						
						/* create a new object to hold info about the filled combo */
						/* the combo gets filled in gok_page_accessmethod_fill_combos */
						pFilledComboNew = (GokFilledCombo*)g_malloc (sizeof (GokFilledCombo));
						pFilledComboNew->Fillwith = pControl->Fillwith;
						pFilledComboNew->Qualifier = pControl->Qualifier;
						pFilledComboNew->pCombo = pWidget;
						pFilledComboNew->pNameAccessMethod = (gchar*)g_malloc (strlen (NameAccessMethod) + 1);
						strcpy (pFilledComboNew->pNameAccessMethod, NameAccessMethod);
						pFilledComboNew->pNameControl = (gchar*)g_malloc (strlen (pControlName) + 1);
						strcpy (pFilledComboNew->pNameControl, pControlName);
						pFilledComboNew->pGokFilledComboNext = NULL;
						pFilledComboNew->bFilled = FALSE;
						
						/* add the object into our list of objects */
						if (m_pGokFilledComboFirst == NULL)
						{
							m_pGokFilledComboFirst = pFilledComboNew;
						}
						else
						{
							pFilledComboTemp = m_pGokFilledComboFirst;
							while (pFilledComboTemp->pGokFilledComboNext != NULL)
							{
								pFilledComboTemp = pFilledComboTemp->pGokFilledComboNext;
							}
							pFilledComboTemp->pGokFilledComboNext = pFilledComboNew;
						}
						break;
						
					case CONTROL_TYPE_SEPERATOR:
						pWidget = gtk_hseparator_new ();
						gtk_widget_ref (pWidget);
						gtk_object_set_data_full (GTK_OBJECT (gok_settingsdialog_get_window()), "GOKseparator", pWidget,
						                            (GtkDestroyNotify) gtk_widget_unref);
						gtk_widget_show (pWidget);
						gtk_box_pack_start (GTK_BOX (pControlParent->pWidget), pWidget, TRUE, TRUE, 0);
						break;
						
					case CONTROL_TYPE_FRAME:
						pWidget = gtk_frame_new (_(pControl->String));
						gtk_widget_ref (pWidget);
						gtk_object_set_data_full (GTK_OBJECT (gok_settingsdialog_get_window()), "GOKframe", pWidget,
						                            (GtkDestroyNotify) gtk_widget_unref);
						gtk_widget_show (pWidget);
						gtk_box_pack_start (GTK_BOX (pControlParent->pWidget), pWidget, FALSE, FALSE, 0);
						break;
						
					case CONTROL_TYPE_BUTTON:
						pControlName = (pControl->Name != NULL) ? pControl->Name : "button";
						pWidget = gtk_button_new_with_label (_(pControl->String));
						gtk_widget_ref (pWidget);
						gtk_object_set_data_full (GTK_OBJECT (gok_settingsdialog_get_window()), pControlName, pWidget,
						                            (GtkDestroyNotify) gtk_widget_unref);
						gtk_widget_show (pWidget);
						gtk_box_pack_start (GTK_BOX (pControlParent->pWidget), pWidget, FALSE, FALSE, 0);
						
						gok_control_add_handler (pWidget, pControl->Handler);
						break;
						
					case CONTROL_TYPE_CHECKBUTTON:
						pWidget = gtk_check_button_new_with_label (_(pControl->String));
						gtk_widget_ref (pWidget);
						gtk_object_set_data_full (GTK_OBJECT (gok_settingsdialog_get_window()), "GOKcheckbutton", pWidget,
						                            (GtkDestroyNotify) gtk_widget_unref);
						gtk_widget_show (pWidget);
						gtk_box_pack_start (GTK_BOX (pControlParent->pWidget), pWidget, FALSE, FALSE, 0);

						if ((pControl->Name != NULL) &&
							(gok_data_get_setting (NameAccessMethod, pControl->Name, &settingInt, &settingString) == TRUE))
						{
							if (settingInt != 0)
							{
								gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pWidget), TRUE);
							}
						}

						/* connect this signal so we get notified when the control changes state */
						/* note: This is only done for checkboxes. If you have associated controls
						   for other controls then add a handler for them. */
						gtk_signal_connect (GTK_OBJECT (pWidget), "toggled",
						GTK_SIGNAL_FUNC (gok_page_accessmethod_checkbox_changed),NULL);
						break;
						
					case CONTROL_TYPE_RADIOBUTTON:
						if (pControl->bGroupStart == TRUE)
						{
							ListRadioButtons = NULL;
						}

						pWidget = gtk_radio_button_new_with_label (ListRadioButtons, _(pControl->String));
						ListRadioButtons = gtk_radio_button_group (GTK_RADIO_BUTTON (pWidget));
						gtk_widget_ref (pWidget);
						gtk_object_set_data_full (GTK_OBJECT (gok_settingsdialog_get_window()), "GOKradiobutton", pWidget,
						                            (GtkDestroyNotify) gtk_widget_unref);
						gtk_widget_show (pWidget);
						gtk_box_pack_start (GTK_BOX (pControlParent->pWidget), pWidget, FALSE, FALSE, 0);
						
						gtk_object_set_data(GTK_OBJECT (pWidget), "control", pControl);

						if ((pControl->Name != NULL) &&
							(gok_data_get_setting (NameAccessMethod, pControl->Name, &settingInt, &settingString) == TRUE))
						{
							if (settingInt == pControl->Value)
							{
								gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(pWidget), TRUE);
							}
						}						
						break;
						
					case CONTROL_TYPE_SPINBUTTON:
						spinRateAdjustment = gtk_adjustment_new (pControl->Value, 
																pControl->Min, 
																pControl->Max, 
																pControl->StepIncrement,
																pControl->PageIncrement,
																pControl->PageSize);
		
						pWidget = gtk_spin_button_new (GTK_ADJUSTMENT (spinRateAdjustment), 1, 0);
						gtk_widget_ref (pWidget);
						gtk_object_set_data_full (GTK_OBJECT (gok_settingsdialog_get_window()), "GOKspinbutton", pWidget,
						                            (GtkDestroyNotify) gtk_widget_unref);
						gtk_widget_show (pWidget);
						gtk_box_pack_start (GTK_BOX (pControlParent->pWidget), pWidget, FALSE, FALSE, 0);

						if ((pControl->Name != NULL) &&
							(gok_data_get_setting (NameAccessMethod, pControl->Name, &settingInt, &settingString) == TRUE))
						{
							gtk_spin_button_set_value (GTK_SPIN_BUTTON(pWidget), settingInt);
						}
						break;
						
					default:
						gok_log_x ("default hit!");
						break;
				}
				pControl->pWidget = pWidget;
			}
		}
		else /* hide the control */
		{
			if (pControl->pWidget != NULL)
			{
				gtk_widget_hide (pControl->pWidget);
			}
		}
		
		/* create any child controls */
		if (pControl->pControlChild != NULL)
		{
			gok_page_accessmethod_draw_controls (NameAccessMethod, pControl, pControl->pControlChild, bShow);
		}

		pControl = pControl->pControlNext;
	}
}

/**
* gok_page_accessmethod_checkbox_changed
* @pButton: Pointer to the checkbox that has changed state
* @data: Unused.
*
* A checkbox in an access method has changed state. Check for an associated
* control for the checkbox and, if present, enable/disable the associated control.
**/
void gok_page_accessmethod_checkbox_changed (GtkButton* pButton, gpointer data)
{
	GokAccessMethod* pAccessMethod;
	GokControl* pControl;
	GokControl* pControlAssociated;
	gchar* token;
	gchar seperators[] = "+";
	gchar buffer[151];

	/* find the GokControl that corresponds to the checkbox */
	pAccessMethod = gok_scanner_get_first_access_method();
	while (pAccessMethod != NULL)
	{
		pControl = gok_control_find_by_widget (GTK_WIDGET(pButton), pAccessMethod->pControlOptions);
		if (pControl != NULL)
		{
			break;
		}
		pAccessMethod = pAccessMethod->pAccessMethodNext;
	}

	if (pAccessMethod == NULL)
	{
		gok_log_x ("Can't find associated control");
		return;
	}
	
	g_assert (pControl != NULL);
	
	/* is there an associated control for the checkbox? */
	if (pControl->NameAssociatedControl != NULL)
	{
		/* there may be several associated controls so parse the string */
		strncpy (buffer, pControl->NameAssociatedControl, 150);
		token = strtok (buffer, seperators);
		while (token != NULL)
		{
			/* find the GokControl for the associated control */
			pControlAssociated = gok_control_find_by_name (token, pAccessMethod->pControlOptions);
			if (pControlAssociated == NULL)
			{
				gok_log_x ("Can't find associated control: %s", token);
			}
			else /* make the control sensitive/insinsitive */
			{
				if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(pButton)) == TRUE)
				{
					if (pControl->bAssociatedStateActive == TRUE)
					{
						gtk_widget_set_sensitive (pControlAssociated->pWidget, TRUE);
					}
					else
					{
						gtk_widget_set_sensitive (pControlAssociated->pWidget, FALSE);
					}
				}
				else
				{
					if (pControl->bAssociatedStateActive == FALSE)
					{
						gtk_widget_set_sensitive (pControlAssociated->pWidget, TRUE);
					}
					else
					{
						gtk_widget_set_sensitive (pControlAssociated->pWidget, FALSE);
					}
				}
			}

			/* get the next associated control */
			token = strtok (NULL, seperators);
		}
	}
}

/**
* gok_page_accessmethod_change_controls
* @pNameAccessMethod: Name of the access method.
* 
* Display the controls for the given access method.
**/
void gok_page_accessmethod_change_controls (gchar* pNameAccessMethod)
{
	GokAccessMethod* pAccessMethod;
	GtkWidget* pLabelDescription;
	GtkWidget* pEntryAccessMethodName;
	GokControl ControlParent;
	GtkWidget* pFrame;

	g_assert (pNameAccessMethod != NULL);

	pFrame = glade_xml_get_widget (gok_settingsdialog_get_xml(), "AccessMethodsTab");
	g_assert (pFrame != NULL);
	ControlParent.pWidget = pFrame;
	ControlParent.Type = CONTROL_TYPE_FRAME;

	/* find the access method in our list */
	pAccessMethod = gok_scanner_get_first_access_method();				
	while (pAccessMethod != NULL)
	{
		if (strcmp (pNameAccessMethod, pAccessMethod->Name) == 0)
		{
			/* hide the controls from the previous access method */
			if (m_pAccessMethod != NULL)
			{
				gok_page_accessmethod_draw_controls (pNameAccessMethod, &ControlParent, m_pAccessMethod->pControlOperation, FALSE);
				gok_page_accessmethod_draw_controls (pNameAccessMethod, &ControlParent, m_pAccessMethod->pControlFeedback, FALSE);
				gok_page_accessmethod_draw_controls (pNameAccessMethod, &ControlParent, m_pAccessMethod->pControlOptions, FALSE);
			}

			/* store the new access method */
			strcpy (m_NameAccessMethod, pNameAccessMethod);
			m_pAccessMethod = pAccessMethod;
			
			/* display the name in the combo box */
			m_bUsChangingName = TRUE;
			pEntryAccessMethodName = glade_xml_get_widget (gok_settingsdialog_get_xml(), "entryAccessMethodName");
			g_assert (pEntryAccessMethodName != NULL);
			gtk_entry_set_text (GTK_ENTRY (pEntryAccessMethodName), gok_page_accessmethod_get_displayname (pNameAccessMethod));
			m_bUsChangingName = FALSE;
			
			/* change the access method description text */
			pLabelDescription = glade_xml_get_widget (gok_settingsdialog_get_xml(), "labelAmDescription");
			g_assert (pLabelDescription != NULL);
			gtk_label_set_text (GTK_LABEL (pLabelDescription), pAccessMethod->Description);
				
			/* display the controls for the selected access method */
			gok_page_accessmethod_draw_controls (pNameAccessMethod, &ControlParent, pAccessMethod->pControlOperation, TRUE);
			gok_page_accessmethod_draw_controls (pNameAccessMethod, &ControlParent, pAccessMethod->pControlFeedback, TRUE);
			gok_page_accessmethod_draw_controls (pNameAccessMethod, &ControlParent, pAccessMethod->pControlOptions, TRUE);

			gok_page_accessmethod_update_associated (pAccessMethod);
			return;
		}
		pAccessMethod = pAccessMethod->pAccessMethodNext;
	}
	
	gok_log_x ("Warning: Can't find name '%s' in gok_page_accessmethod_change_controls!\n", pNameAccessMethod);
}

/**
* gok_page_accessmethod_page_active
* 
* This page has just become active. Refill the 'actions' and 'feedbacks'
* combos with values.
**/
void gok_page_accessmethod_page_active()
{
	/* check if there are any changes before filling the combos */
	if ((gok_page_feedbacks_get_changed() == TRUE) ||
		(gok_page_actions_get_changed() == TRUE))
	{
		gok_page_accessmethod_fill_combos (TRUE);

		gok_page_feedbacks_set_changed (FALSE);
		gok_page_actions_set_changed (FALSE);
	}
}

/**
* gok_page_accessmethod_fill_combos
* @bRefill: If TRUE then all the combos are filled. If FALSE then only those
* combos that have not been filled will be filled.
*
* Fill the combo boxes on this page with actions and feedbacks.
* This should be done each time the feedbacks and/or actions change.
**/
void gok_page_accessmethod_fill_combos (gboolean bRefill)
{
	GokFilledCombo* pGokFilledCombo;
	GokAction* pAction;
	GokFeedback* pFeedback;
	GList* items;
	int settingInt;
	gchar* settingString;
	GtkWidget* pEntry;

	pGokFilledCombo = m_pGokFilledComboFirst;
	while (pGokFilledCombo != NULL)
	{
		if ((bRefill == TRUE) ||
		  	(pGokFilledCombo->bFilled == FALSE))
		{
			pGokFilledCombo->bFilled = TRUE;

			items = NULL;
	
			/* get the current setting string */
			if (gok_data_get_setting (pGokFilledCombo->pNameAccessMethod, pGokFilledCombo->pNameControl, &settingInt, &settingString) == FALSE)
			{
				settingString = "";
			}
	
			switch (pGokFilledCombo->Fillwith)
			{
				case CONTROL_FILLWITH_ACTIONS:
					g_object_set_data (G_OBJECT(pGokFilledCombo->pCombo), "type", "actions");					
	
					pAction = gok_action_get_first_action();
					while (pAction != NULL)
					{
						/* use only the actions specified by the 'qualifier' */
						if (pAction->Type & pGokFilledCombo->Qualifier)
						{
							items = g_list_append (items, pAction->pDisplayName);
											
							/* get the display name from the setting name */
							if (strcmp (pAction->pName, settingString) == 0)
							{
								settingString = pAction->pDisplayName;
							}
						}
						pAction = pAction->pActionNext;
					}
					break;
									
				case CONTROL_FILLWITH_FEEDBACKS:
					g_object_set_data (G_OBJECT(pGokFilledCombo->pCombo), "type", "feedbacks");					
	
					pFeedback = gok_feedback_get_first_feedback();
					while (pFeedback != NULL)
					{
						items = g_list_append (items, pFeedback->pDisplayName);
											
						/* get the display name from the setting name */
						if (strcmp (pFeedback->pName, settingString) == 0)
						{
							settingString = pFeedback->pDisplayName;
						}
						pFeedback = pFeedback->pFeedbackNext;
					}
					break;
									
				case CONTROL_FILLWITH_SOUNDS:
					g_object_set_data (G_OBJECT(pGokFilledCombo->pCombo), "type", "sounds");					
	
					items = g_list_append (items, "beep");
					items = g_list_append (items, "boop");
					items = g_list_append (items, "shreik");
					items = g_list_append (items, "bang");
					break;
									
				case CONTROL_FILLWITH_OPTIONS:
					g_object_set_data (G_OBJECT(pGokFilledCombo->pCombo), "type", "options");					
	
					items = g_list_append (items, "restart scanning");
					items = g_list_append (items, "stop scanning");
					items = g_list_append (items, "output character");
					break;
									
				default:
					break;
			}
			if (items != NULL)
			{
				/* sort the names */
				items = g_list_sort (items, gok_settingsdialog_sort);

				gtk_combo_set_popdown_strings (GTK_COMBO (pGokFilledCombo->pCombo), items);
				g_list_free (items);
			}
		
			/* display the current setting */
			pEntry = GTK_COMBO(pGokFilledCombo->pCombo)->entry;
			g_assert (pEntry != NULL);
			gtk_entry_set_text (GTK_ENTRY (pEntry), settingString);
		}

		pGokFilledCombo = pGokFilledCombo->pGokFilledComboNext;
	}
}


/**
* gok_page_accessmethod_update_associated
*
* Updates any associated controls so they are enabled/disabled.
**/
void gok_page_accessmethod_update_associated (GokAccessMethod* pAccessMethod)
{
	g_assert (pAccessMethod != NULL);

	gok_page_accessmethod_update_associated_loop (pAccessMethod->pControlOptions);
}

/**
* gok_page_accessmethod_update_associated_loop
*
* Updates any associated controls so they are enabled/disabled.
**/
void gok_page_accessmethod_update_associated_loop (GokControl* pControl)
{
	while (pControl != NULL)
	{
		if (pControl->NameAssociatedControl != NULL)
		{
			gok_page_accessmethod_checkbox_changed (GTK_BUTTON(pControl->pWidget), NULL);
		}
		
		if (pControl->pControlChild != NULL)
		{
			gok_page_accessmethod_update_associated_loop (pControl->pControlChild);
		}

		pControl = pControl->pControlNext;	
	}
}
