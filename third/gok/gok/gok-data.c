/* gok-data.c
*
* Copyright 2001,2002 Sun Microsystems, Inc.,
* Copyright 2001,2002 University Of Toronto
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

#include <stdio.h>
#include <stdlib.h>
#include "gok-data.h"
#include "gok-log.h"
#include "gok-gconf.h"
#include "gok-gconf-keys.h"
#include "main.h"

/* Private functions */
static void gok_data_set_name_accessmethod_internal (const char* Name);
static const GokComposeType gok_data_compose_type_from_string (const gchar *s);

/* pointer to the first GokData setting */
static GokSetting* m_pSettingFirst;

/* Will be TRUE if the settings have been changed, FALSE if not. */
/* The settings get saved if this is TRUE. */
static gboolean m_bSettingsChanged;

/*
 Settings used by the GOK.
 Settings for each access method are kept on the access method (not here).
 These settings are saved in gok_data_write_settings.
 These settings are retreived in gok_data_read_settings.
 These settings are initialized in gok_data_initialize.
 If you add a new setting then add it here and update write, read and initialize
 All these settings have accessor functions.
*/
static gint m_GokKeyWidth;
static gint m_GokKeyHeight;
static gint m_GokKeySpacing;
static gint m_GokKeyboardX;
static gint m_GokKeyboardY;
static gboolean m_bKeysizePriority;
static gboolean m_bWordCompleteOn;
static gboolean m_bCommandPredictOn;
static gint m_NumberPredictions;
static char NameAccessMethod [MAX_ACCESS_METHOD_NAME];
static gboolean m_bUseGtkPlusTheme;
static gboolean m_bDriveCorePointer = FALSE;
static gboolean m_bExpandWindow;
static gboolean m_bUseXkbKbd;
static gboolean m_bUseAuxDicts = FALSE;
static gdouble m_ValuatorSensitivity = 0.25;
static gchar *m_AuxDicts = NULL;
static GokDockType m_eDockType;
static GokComposeType compose_keyboard_type;

/* This GConfClient will be shared by all gconf_client
 * calls in this file.
 * We may need to change that if we get reentrantcy problems.
 */
static GConfClient *gconf_client;

/**
 * gok_data_initialize
 *
 * Call this before using the Data. All data is initialized here.
 */
void gok_data_initialize ()
{
	m_pSettingFirst = NULL;

	/* NOTE
	   If a setting has a corresponding GConf key
	   the default value below will be overwritten by a
	   GConf default if one exists.
	   If there is a default for a GConf key it will
	   be in gok-with-references.schemas.m4
	*/

	/* default values for all settings */
	m_GokKeyWidth = 75;
	m_GokKeyHeight = 50;
	m_GokKeySpacing = 10;
	m_GokKeyboardX = 100;
	m_GokKeyboardY = 100;
	m_bKeysizePriority = TRUE;
	gok_data_set_name_accessmethod_internal ("directselection");
	m_bWordCompleteOn = FALSE;
	m_bCommandPredictOn = FALSE;
	m_NumberPredictions = 5;
	m_bUseGtkPlusTheme = FALSE;
	m_bExpandWindow = TRUE;
	m_eDockType = GOK_DOCK_NONE;

	/* This GConfClient will be shared by all gconf_client
	 * calls in this file.
	 * We may need to change that if we get reentrantcy problems.
	 */
	gconf_client = gconf_client_get_default ();

	gok_data_read_settings();

	gok_gconf_set_string (gconf_client,
			      GOK_GCONF_ACCESS_METHOD_SETTINGS_WORKAROUND,
			      GOK_GCONF_WORKAROUND_TEXT);

	m_bSettingsChanged = FALSE;
}

GConfClient*
gok_data_get_gconf_client ()
{
	return gconf_client;
}


static GokDockType
gok_data_dock_type_from_string (gchar *string)
{
	if (!strcmp (string, "top")) {
		return GOK_DOCK_TOP;
	}
	else if (!strcmp (string, "bottom")) {
		return GOK_DOCK_BOTTOM;
	}
	else
		return GOK_DOCK_NONE;
}

/**
 * gok_data_construct_setting:
 *
 * Creates a new GokSetting structure.
 *
 * returns: A pointer to the new setting, NULL if setting was not created.
 */
GokSetting* gok_data_construct_setting (void)
{
	GokSetting* pSetting;

	pSetting = (GokSetting*) g_malloc(sizeof(GokSetting));

	pSetting->Name[0] = 0;
	pSetting->NameAccessMethod[0] = 0;
	pSetting->pValueString = NULL;
	pSetting->pValueStringBackup = NULL;
	pSetting->Value = 0;
	pSetting->ValueBackup = 0;
	pSetting->pSettingNext = NULL;
	pSetting->bIsAction = FALSE;

	return pSetting;
}

/**
 * gok_data_read_settings:
 *
 * Reads in the settings from disk. The settings are stored in a list
 * with the first item * in m_pSettingFirst.
 *
 * returns: TRUE if the settings were read in correctly, FALSE if not.
 */
gboolean gok_data_read_settings (void)
{
	GokSetting* pSetting;
	GokSetting* pSettingLast = NULL;

	gchar *a_gchar;
	GSList *access_method_dirs;
	GSList *access_method_dir;
	GSList *access_method_entries;
	GSList *access_method_entry;
	GConfEntry *a_gconfentry;
	gchar *access_method_name;
	gchar *setting_name;
	GConfValue *a_gconfvalue;

	/* Read the settings from GConf. */

	gok_gconf_get_int (gconf_client, GOK_GCONF_KEY_SPACING,
				&m_GokKeySpacing);

	gok_gconf_get_int (gconf_client, GOK_GCONF_KEY_WIDTH,
				&m_GokKeyWidth);

	gok_gconf_get_int (gconf_client, GOK_GCONF_KEY_HEIGHT,
				&m_GokKeyHeight);

	gok_gconf_get_int (gconf_client, GOK_GCONF_KEYBOARD_X,
				&m_GokKeyboardX);

	gok_gconf_get_int (gconf_client, GOK_GCONF_KEYBOARD_Y,
				&m_GokKeyboardY);

	if (gok_gconf_get_string (gconf_client, GOK_GCONF_ACCESS_METHOD,
                                  &a_gchar)) {
	    gok_data_set_name_accessmethod_internal (a_gchar);
	    g_free (a_gchar);
	}

	gok_gconf_get_bool (gconf_client, GOK_GCONF_WORD_COMPLETE,
				 &m_bWordCompleteOn);

	gok_gconf_get_int (gconf_client, GOK_GCONF_NUMBER_PREDICTIONS,
				&m_NumberPredictions);

	gok_gconf_get_bool (gconf_client, GOK_GCONF_USE_GTKPLUS_THEME,
                                 &m_bUseGtkPlusTheme);

	gok_gconf_get_bool (gconf_client, GOK_GCONF_EXPAND,
                                 &m_bExpandWindow);

	gok_gconf_get_bool (gconf_client, GOK_GCONF_USE_XKB_KBD,
                                 &m_bUseXkbKbd);

	gok_gconf_get_bool (gconf_client, GOK_GCONF_USE_AUX_DICTS, 
			         &m_bUseAuxDicts);

 	gok_gconf_get_double (gconf_client, GOK_GCONF_VALUATOR_SENSITIVITY,
                                 &m_ValuatorSensitivity);

        gok_gconf_get_string (gconf_client, GOK_GCONF_AUX_DICTS, 
			         &m_AuxDicts);

	gok_gconf_get_string (gconf_client, GOK_GCONF_DOCK_TYPE,
			      &a_gchar);
        m_eDockType = gok_data_dock_type_from_string (a_gchar);
	g_free (a_gchar);

	gok_gconf_get_string (gconf_client, GOK_GCONF_COMPOSE_KBD_TYPE,
			      &a_gchar);
        compose_keyboard_type = gok_data_compose_type_from_string (a_gchar);
	g_free (a_gchar);

	/* Read in settings for individual access methods */
	access_method_dirs
	    = gconf_client_all_dirs (gconf_client,
				     GOK_GCONF_ACCESS_METHOD_SETTINGS,
				     NULL);

	/* Loop through access methods */
	access_method_dir = access_method_dirs;
	while (access_method_dir != NULL)
	{
	    gok_log ("Found access method key %s", access_method_dir->data);
	    access_method_name = g_strrstr (access_method_dir->data, "/");
	    if ( (access_method_name != NULL)
		 && ( strlen (access_method_name) >= 2 ) )
	    {
		/* step over "/" */
		access_method_name++;
		gok_log ("access_method_name = %s", access_method_name);

		access_method_entries = gconf_client_all_entries (gconf_client,
					access_method_dir->data, NULL);

		/* Loop through settings for current access method */
		access_method_entry = access_method_entries;
		while (access_method_entry != NULL)
		{
		    a_gconfentry = access_method_entry->data;
		    gok_log ("Found key %s",
			     gconf_entry_get_key (a_gconfentry));

		    setting_name = g_strrstr(gconf_entry_get_key(a_gconfentry),
					      "/");
		    if ( (setting_name != NULL)
			 && ( strlen (setting_name) >= 2) )
		    {
			/* step over "/" */
			setting_name++;
			gok_log ("setting_name = %s", setting_name);

			/* Get value for current setting */
			a_gconfvalue = gconf_entry_get_value (a_gconfentry);

			/* Check if it is an int or a string */
			if ((a_gconfvalue->type != GCONF_VALUE_INT) 
			    && (a_gconfvalue->type != GCONF_VALUE_STRING))
			{
			    gok_log_x ("*** Error: Key %s is not an int or a string",
				       gconf_entry_get_key (a_gconfentry));
			}
			else
			{
			    /* It is an int or a string */

			    /* Create a new setting structure */
			    pSetting = gok_data_construct_setting ();
			    if (pSetting == NULL)
			    {
				break;
			    }

			    /* Populate the setting structure */
			    strncpy (pSetting->NameAccessMethod,
				     access_method_name,
				     MAX_ACCESS_METHOD_NAME);

			    strncpy (pSetting->Name,
				     setting_name,
				     MAX_RATE_NAME);

			    if (a_gconfvalue->type == GCONF_VALUE_INT)
			    {
				pSetting->Value = gconf_value_get_int (a_gconfvalue);
				pSetting->pValueString = NULL;
			    }
			    else if (a_gconfvalue->type == GCONF_VALUE_STRING)
			    {
				pSetting->pValueString = (gchar*)g_malloc (
				    strlen (gconf_value_get_string (
					a_gconfvalue)) + 1);
				strcpy (pSetting->pValueString,
					gconf_value_get_string (a_gconfvalue));
			    }

			    /* Link the new setting structure into the list */
			    if (m_pSettingFirst == NULL)
			    {
				m_pSettingFirst = pSetting;
			    }
			    else
			    {
				pSettingLast->pSettingNext = pSetting;
			    }

			    pSettingLast = pSetting;
			}
		    }
		    gconf_entry_free (access_method_entry->data);
		    access_method_entry = access_method_entry->next;
		}
		g_slist_free (access_method_entries);
	    }
	    g_free (access_method_dir->data);
	    access_method_dir = access_method_dir->next;
	}
	g_slist_free (access_method_dirs);

	return TRUE;
}

/**
 * gok_data_close:
 *
 * Frees any memory used by the data.
 */
void gok_data_close (void)
{
	GokSetting* pSetting;
	GokSetting* pSettingTemp;

	/* delete all the setting structures */
	pSetting = m_pSettingFirst;
	while (pSetting != NULL)
	{
		pSettingTemp = pSetting;
		pSetting = pSetting->pSettingNext;
		g_free (pSettingTemp);
	}
}

/**
 * gok_data_get_control_values:
 * @pControl:
 *
 * Gets the values from the settings for the given control.
 *
 * returns: TRUE if the value was updated, FALSE if not.
 */
/*
gboolean gok_data_get_control_values (GokControl* pControl)
{
	switch (pControl->Type)
	{
		case CONTROL_TYPE_COMBOBOX:
			if (gok_data_get_setting (NameAccessMethod, pControl->Name, &settingInt, &settingString) == TRUE)
			{
				if (pControl->Name != NULL)
				{
					g_free (pControl->Name);
				}
				pControl->Name = (gchar*)g_malloc (strlen (settingString) + 1);
				strcpy (pControl->Name, settingString);
			}
			break;

		case CONTROL_TYPE_CHECKBUTTON:
			if (gok_data_get_setting (NameAccessMethod, pControl->Name, &settingInt, &settingString) == TRUE)
			{
				pControl->Value = settingInt;
			}
			break;

		case CONTROL_TYPE_SPINBUTTON:
			if (gok_data_get_setting (NameAccessMethod, pControl->Name, &settingInt, &settingString) == TRUE)
			{
				pControl->Value = settingInt;
			}
			break;

		default:
			break;
	}

	return TRUE;
}
*/


static void
gok_data_fill_setting_values (gint settingValue, gchar *settingValueString, gint *Value, gchar **ValueString)
{
    /* set the given value to the new value */
    if (settingValueString != NULL)
    {
	if (ValueString != NULL)
	{
	    *ValueString = settingValueString;
	}
	if (Value != NULL)
	{
	    *Value = 0;
	}
    }
    else
    {
	if (Value != NULL)
	{
	    *Value = settingValue;
	}
	if (ValueString != NULL)
	{
	    *ValueString = NULL;
	}
    }
}


/**
 * gok_data_get_setting:
 * @NameAccessMethod: Name of the access method that contains the setting.
 * @NameSetting: Name of the setting you want.
 * This string may specify more than
 * one setting with the various names seperated by '+'. In this case, only the
 * first name will be used. 
 * @Value: Will contain the setting value if the function returns TRUE. This
 * may be NULL in which case it will not be filled in.
 * @ValueString: Will contain a pointer to the value string. This name be NULL
 * in which case it will not be filled in.
 *
 * Gets a value for an access method setting.
 *
 * returns: TRUE if the GokData has the setting, FALSE if not.
 */
gboolean gok_data_get_setting (gchar* NameAccessMethod, gchar* NameSetting, gint* Value, gchar** ValueString)
{
	gchar* pPlus;
	gchar buffer[100];
	gint count;
	GokSetting* pSetting;

	g_assert (NameAccessMethod != NULL);
	g_assert (NameSetting != NULL);
	
	/* check if there is more than one setting name */
	if ((pPlus = strrchr (NameSetting, '+')) != NULL)
	{
		/* yes, use just the first setting name */
		count = pPlus - NameSetting;
		strncpy (buffer, NameSetting, count);
		buffer[count] = 0;
		NameSetting = buffer;
	}
	
	/* first look to see if we have a commandline override for the setting */
	if (gok_main_get_scan_override () && !strcmp (gok_data_get_name_accessmethod (), NameAccessMethod) && 
	    !strcmp (NameSetting, "movehighlighter") && gok_action_find_action (gok_main_get_scan_override (), FALSE))
	{
		gok_data_fill_setting_values (0, gok_main_get_scan_override (), Value, ValueString);
		return TRUE;
	}
	else if (gok_main_get_select_override () && 
		 !strcmp (gok_data_get_name_accessmethod (), NameAccessMethod) &&
		 (!strcmp (NameSetting, "select") || !strcmp (NameSetting, "outputselected"))
		 && gok_action_find_action (gok_main_get_select_override (), FALSE))
	{
		gok_data_fill_setting_values (0, gok_main_get_select_override (), Value, ValueString);
		return TRUE;
	}
	else
	{
		/* find a value for the given access method setting in our list of settings */
		pSetting = m_pSettingFirst;
		while (pSetting != NULL)
		{
			/* check if this setting has the same access method name and setting name as given */
			if ((strcmp (pSetting->NameAccessMethod, NameAccessMethod) == 0) &&
			    (strcmp (pSetting->Name, NameSetting) == 0))
			{
				gok_data_fill_setting_values (pSetting->Value, pSetting->pValueString, Value, ValueString);
				return TRUE;
			}
			pSetting = pSetting->pSettingNext;
		}
	}
	return FALSE;
}

/**
 * gok_data_set_setting:
 * @NameAccessMethod: Name of the access method that contains the setting.
 * @NameSetting: Name of the setting you want.
 * You may specify more than one setting by seperating
 * the setting names by "+".
 * @Value: Will contain the setting value if the function returns TRUE.
 * @ValueString:
 *
 * Sets a value for an access method.
 *
 * returns: TRUE if the setting was changed, FALSE if not.
 */
gboolean gok_data_set_setting (gchar* NameAccessMethod, gchar* NameSetting, gint Value, gchar* ValueString)
{
	GokSetting* pSetting;
	gchar* pTokenSettingName;
	gchar buffer[150];
	gchar* pkey;
	gboolean codeReturned;

	codeReturned = FALSE;
	
	/* there may be more than one setting specified in NameSetting */
	/* each setting is seperated by a "+" */
	strcpy (buffer, NameSetting);
	pTokenSettingName = strtok (buffer, "+");
	while (pTokenSettingName != NULL)
	{
		/* find the setting in the list */
		pSetting = m_pSettingFirst;
		while (pSetting != NULL)
		{
			if ((strcmp (pSetting->NameAccessMethod, NameAccessMethod) == 0) &&
				(strcmp (pSetting->Name, pTokenSettingName) == 0))
			{
				/* found it */
				pkey = g_strjoin ("/", GOK_GCONF_ACCESS_METHOD_SETTINGS, NameAccessMethod, pTokenSettingName, NULL);

				/* set the string (if it's not NULL) */
				if (ValueString != NULL)
				{
					if ((pSetting->pValueString == NULL) ||
						(strcmp (pSetting->pValueString, ValueString) != 0))
					{
						/* delete the old string if there is one */
						if (pSetting->pValueString != NULL)
						{
							g_free (pSetting->pValueString);
						}
						
						pSetting->pValueString = (gchar*)g_malloc (strlen (ValueString) + 1);
						strcpy (pSetting->pValueString, ValueString);
						
						gok_log ("Writing %s = %s to GConf", pkey, ValueString);
		    			gok_gconf_set_string (gconf_client, pkey, ValueString);
	
						codeReturned = TRUE;
					}
				}
				else
				{
					/* set the int */
					if (pSetting->Value != Value)
					{
						pSetting->Value = Value;
						gok_log ("Writing %s = %d to GConf", pkey, Value);
			 		   gok_gconf_set_int (gconf_client, pkey, Value);
			 		   
			 		   codeReturned = TRUE;
					}
				}
				break;
			}
			pSetting = pSetting->pSettingNext;
		}
		
		if (pSetting == NULL)
		{
			/* couldn't find the setting in the list */
			gok_log_x ("Can't find setting! Access method = %s, setting name = %s.", NameAccessMethod, pTokenSettingName);
		}

		pTokenSettingName = strtok (NULL, "+");
	}

	return codeReturned;
}

/**
 * gok_data_create_setting:
 * @NameAccessMethod: Name of the access method that uses the setting.
 * @NameSetting: Name of the setting.
 * @Value: Setting value.
 * @pValueString:
 *
 * Creates a new setting and stores it in GConf.
 *
 * returns: TRUE if the new setting was created, FALSE if not.
 */
gboolean gok_data_create_setting (gchar* NameAccessMethod, gchar* NameSetting, gint Value, gchar* pValueString)
{
	GokSetting* pSetting;
	GokSetting* pSettingLast;
	gchar *key;

	/* create the new setting */
	pSetting = gok_data_construct_setting ();
	if (pSetting == NULL)
	{
		return FALSE;
	}

	/* populate the setting structure */
	strcpy (pSetting->NameAccessMethod, NameAccessMethod);
	strcpy (pSetting->Name, NameSetting);
	pSetting->Value = Value;
	if (pValueString != NULL)
	{
		pSetting->pValueString = (gchar*)g_malloc (strlen (pValueString) + 1);
		strcpy (pSetting->pValueString, pValueString);
	}

	/* link the new setting structure into the list */
	/* add it at the head if there is no head */
	if (m_pSettingFirst == NULL)
	{
		m_pSettingFirst = pSetting;
	}
	else
	{
		/* add it to the tail of the list of settings */
		pSettingLast = m_pSettingFirst;
		while (pSettingLast->pSettingNext != NULL)
		{
			pSettingLast = pSettingLast->pSettingNext;
		}
		pSettingLast->pSettingNext = pSetting;
	}

	/* Write new setting to GConf */

	key = g_strjoin ("/", GOK_GCONF_ACCESS_METHOD_SETTINGS,
			 NameAccessMethod, NameSetting, NULL);

	/* if pValueString is NULL the caller wanted an int setting */
	if (pValueString == NULL)
	{
		gok_log ("Writing %s = %d to GConf", key, Value);
	    gok_gconf_set_int (gconf_client, key, Value);
	}
	else
	{
	    /* if pValueString is not NULL the caller wanted
	     * a string setting
	     */
		gok_log ("Writing %s = %s to GConf", key, pValueString);
	    gok_gconf_set_string (gconf_client, key, pValueString);
	}
	g_free (key);

	/* settings are now modified */
	m_bSettingsChanged = TRUE;

	return TRUE;
}

/**
 * gok_data_get_key_width
 *
 * returns: The key width.
 */
gint gok_data_get_key_width ()
{
	return m_GokKeyWidth;
}

/**
 * gok_data_set_key_width:
 * @Width: The new key width
 */
void gok_data_set_key_width (gint Width)
{
	m_GokKeyWidth = Width;
	gok_gconf_set_int (gconf_client, GOK_GCONF_KEY_WIDTH,
				m_GokKeyWidth);
	m_bSettingsChanged = TRUE;
}

/**
 * gok_data_get_use_xkb_kbd:
 *
 * returns: A gboolean indicating whether or not to use Xkb's
 *      core-keyboard geometry information to generate 
 *      the primary compose keyboard.
 */
gboolean gok_data_get_use_xkb_kbd ()
{
	return m_bUseXkbKbd;
}

/**
 * gok_data_set_use_xkb_xkb:
 * @val: a gboolean indicating whether to use XKB's core-keyboard
 * to create GOK's main compose keyboard.
 */
void gok_data_set_use_xkb_kbd (gboolean val)
{
	m_bUseXkbKbd = val;
	gok_gconf_set_bool (gconf_client, GOK_GCONF_USE_XKB_KBD,
				m_bUseXkbKbd);
	m_bSettingsChanged = TRUE;
}

/**
 * gok_data_get_dock_type:
 *
 * returns: A GokDockType indicating whether or not GOK's main window is
 *          a "DOCK" type window, and whether it is anchored top or bottom.
 */
GokDockType gok_data_get_dock_type ()
{
	return m_eDockType;
}

/**
 * gok_data_set_dock_type:
 * @val: a GokDockType indicating whether GOK's main keyboard should be a "DOCK" window.
 */
void gok_data_set_dock_type (GokDockType val)
{
	gchar *typestring;

	m_eDockType = val;
	switch (val) {
	case GOK_DOCK_TOP:
	  typestring = "top";
	  gok_main_set_wm_dock (TRUE);
	  break;
	case GOK_DOCK_BOTTOM:
	  typestring = "bottom";
	  gok_main_set_wm_dock (TRUE);
	  break;
	default:
	  typestring = "";
	  gok_main_set_wm_dock (FALSE);
	  break;
	}
	gok_gconf_set_string (gconf_client, GOK_GCONF_DOCK_TYPE,
			      typestring);
	m_bSettingsChanged = TRUE;
}

/**
 * gok_data_get_key_height
 *
 * returns: The key height.
 */
gint gok_data_get_key_height ()
{
	return m_GokKeyHeight;
}

/**
 * gok_data_set_key_height:
 * @Height: The new key height.
 */
void gok_data_set_key_height (gint Height)
{
	m_GokKeyHeight = Height;
	gok_gconf_set_int (gconf_client, GOK_GCONF_KEY_HEIGHT,
				m_GokKeyHeight);
	m_bSettingsChanged = TRUE;
}

/**
 * gok_data_get_key_spacing
 *
 * returns: The key spacing.
 */
gint gok_data_get_key_spacing ()
{
	return m_GokKeySpacing;
}

/**
 * gok_data_set_key_spacing:
 * @Spacing: The new key spacing.
 */
void gok_data_set_key_spacing (gint Spacing)
{
	m_GokKeySpacing = Spacing;
	gok_gconf_set_int (gconf_client, GOK_GCONF_KEY_SPACING,
				m_GokKeySpacing);
	m_bSettingsChanged = TRUE;
}

/**
 * gok_data_get_keyboard_x:
 *
 * returns: The horizontal location of the keyboard.
 */
gint gok_data_get_keyboard_x ()
{
	return m_GokKeyboardX;
}

/**
 * gok_data_set_keyboard_x:
 * @X: Horizontal position of the keyboard.
 */
void gok_data_set_keyboard_x (gint X)
{
	m_GokKeyboardX = X;
	gok_gconf_set_int (gconf_client, GOK_GCONF_KEYBOARD_X,
				m_GokKeyboardX);
	m_bSettingsChanged = TRUE;
}
	
/**
 * gok_data_get_keyboard_y:
 *
 * returns: The vertical position of the keyboard.
 */
gint gok_data_get_keyboard_y ()
{
	return m_GokKeyboardY;
}

/**
 * gok_data_set_keyboard_y:
 * @Y: Vertical position of the keyboard.
 */
void gok_data_set_keyboard_y (gint Y)
{
	m_GokKeyboardY = Y;
	gok_gconf_set_int (gconf_client, GOK_GCONF_KEYBOARD_Y,
				m_GokKeyboardY);
	m_bSettingsChanged = TRUE;
}

/**
 * gok_data_get_keysize_priority:
 *
 * Returns: TRUE if the keyboard should use the keysize to determine
 * the keyboard size, FALSE if not.
 */
gboolean gok_data_get_keysize_priority ()
{
	return m_bKeysizePriority;
}

/**
 * gok_data_set_keysize_priority:
 * @bFlag: TRUE if the keysize should be used to determine keyboard size.
 */
void gok_data_set_keysize_priority (gboolean bFlag)
{
	m_bKeysizePriority = bFlag;
}

/**
 * gok_data_set_name_accessmethod_internal:
 * @Name: Name of the current access method.
 *
 * This function does not write to GConf it only copies the string
 */
static void 
gok_data_set_name_accessmethod_internal (const char* Name)
{
	if (strcmp (Name, NameAccessMethod) == 0)
	{
		return;
	}

	strncpy (NameAccessMethod, Name, MAX_ACCESS_METHOD_NAME);
	m_bSettingsChanged = TRUE;
}
	
	
/**
 * gok_data_set_name_accessmethod:
 * @Name: Name of the current access method.
 *
 * Records the new access method and stores it in GConf.
 */
void gok_data_set_name_accessmethod (const char* Name)
{
	if (gok_main_get_access_method_override() != NULL)
	{
		return; /*TODO revisit this behavior */
	}

	if (strcmp (Name, NameAccessMethod) == 0)
	{
		return;
	}

	gok_gconf_set_string (gconf_client, GOK_GCONF_ACCESS_METHOD,
				   Name);
	gok_data_set_name_accessmethod_internal (Name);
}
	
/**
 * gok_data_get_name_accessmethod:
 *
 * Returns: Pointer to the name of the current access method.
 */
char* gok_data_get_name_accessmethod ()
{
	if (gok_main_get_access_method_override() != NULL)
	{
		return gok_main_get_access_method_override();
	}
	return NameAccessMethod;
}

/**
 * gok_data_get_wordcomplete:
 *
 * Returns: TRUE if word completion is turned on, FALSE if it's turned off.
 */
gboolean gok_data_get_wordcomplete ()
{
	/* don't use during login mode */
	if (gok_main_get_login()) {
		return FALSE;
	}
	return m_bWordCompleteOn;
}

/**
 * gok_data_set_wordcomplete:
 * @bTrueFalse: The flag setting the word completion state.
 */
void gok_data_set_wordcomplete (gboolean bTrueFalse)
{
	m_bWordCompleteOn = bTrueFalse;
	gok_gconf_set_bool (gconf_client, GOK_GCONF_WORD_COMPLETE,
				 m_bWordCompleteOn);
	m_bSettingsChanged = TRUE;
}

/**
 * gok_data_get_wordcomplete:
 *
 * Returns: TRUE if word completion is turned on, FALSE if it's turned off.
 */
gboolean gok_data_get_commandprediction ()
{
	/* don't use during login mode */
	if (gok_main_get_login()) {
		return FALSE;
	}
	return TRUE;/* TODO m_bCommandPredictOn;*/
}

/**
 * gok_data_set_wordcomplete:
 * @bTrueFalse: The flag setting the word completion state.
 */
void gok_data_set_commandprediction (gboolean bTrueFalse)
{
	m_bCommandPredictOn = bTrueFalse;
/*	gok_gconf_set_bool (gconf_client, GOK_GCONF_WORD_COMPLETE,
				 m_bWordCompleteOn);
	m_bSettingsChanged = TRUE;
	*/
}

/**
 * gok_data_get_num_predictions:
 *
 * Returns: The maximum number of word predictions.
 */
gint gok_data_get_num_predictions ()
{
	return m_NumberPredictions;
}

/**
 * gok_data_set_num_predictions:
 * @Number: Maximum number of word predictions.
 */
void gok_data_set_num_predictions (gint Number)
{
	m_NumberPredictions = Number;
	gok_gconf_set_int (gconf_client, GOK_GCONF_NUMBER_PREDICTIONS,
				m_NumberPredictions);
	m_bSettingsChanged = TRUE;
}

/**
 * gok_data_get_use_gtkplus_theme:
 *
 * Returns: TRUE if we should use the Gtk+ theme, FALSE if we should not.
 */
gboolean gok_data_get_use_gtkplus_theme ()
{
    return m_bUseGtkPlusTheme;
}

/**
 * gok_data_set_use_gtkplus_theme:
 * @bTrueFalse: The flag setting whether to use the Gtk+ theme or not.
 */
void gok_data_set_use_gtkplus_theme (gboolean bTrueFalse)
{
	m_bUseGtkPlusTheme = bTrueFalse;
	gok_gconf_set_bool (gconf_client, GOK_GCONF_USE_GTKPLUS_THEME,
				 m_bUseGtkPlusTheme);
	m_bSettingsChanged = TRUE;
}

/**
 * gok_data_get_drive_corepointer:
 *
 * Returns: TRUE if the core pointer should track the GOK valuators, 
 *          FALSE if it should not.
 */
gboolean gok_data_get_drive_corepointer ()
{
    return m_bDriveCorePointer;
}

/**
 * gok_data_set_drive_corepointer:
 * @bTrueFalse: The flag setting whether the corepointer should track 
 *              any GOK input devices with valuators or not.
 */
void gok_data_set_drive_corepointer (gboolean bTrueFalse)
{
	m_bDriveCorePointer = bTrueFalse;
	/*	gok_gconf_set_bool (gconf_client, GOK_GCONF_DRIVE_COREPOINTER,
				 m_bDriveCorePointer);
				 m_bSettingsChanged = TRUE;
	*/
}

/**
 * gok_data_get_drive_corepointer:
 *
 * Returns: TRUE if the core pointer should track the GOK valuators, 
 *          FALSE if it should not.
 */
gboolean gok_data_get_expand ()
{
    return m_bExpandWindow;
}

/**
 * gok_data_set_expand:
 * @bTrueFalse: The flag setting whether GOK window should expand to fill
 *      the width of the screen.
 */
void gok_data_set_expand (gboolean bTrueFalse)
{
	GokKeyboard *kbd = gok_main_get_current_keyboard ();
	m_bExpandWindow= bTrueFalse;
	gok_gconf_set_bool (gconf_client, GOK_GCONF_EXPAND,
			    m_bExpandWindow);
	if (kbd)
	{
		gok_keyboard_display (kbd,
				      kbd,
				      gok_main_get_main_window (), TRUE);
	}
	m_bSettingsChanged = TRUE;
}

/**
 * gok_data_backup_settings:
 *
 * Backs up the values for all the settings.
 */
void gok_data_backup_settings ()
{
	GokSetting* pSetting;
	
	pSetting = m_pSettingFirst;
	while (pSetting != NULL)
	{
		gok_data_backup_setting (pSetting);
		pSetting = pSetting->pSettingNext;
	}
}

/**
 * gok_data_restore_settings:
 *
 * Restores the values for all the settings.
 *
 * Returns: TRUE if any restored setting was different from the
 * current setting.  Returns FALSE if all the restored settings are
 * the same as the current settings.
 */
gboolean gok_data_restore_settings ()
{
	gboolean codeReturned;
	GokSetting* pSetting;
	
	codeReturned = FALSE;
	
	pSetting = m_pSettingFirst;
	while (pSetting != NULL)
	{
		if (gok_data_restore_setting (pSetting) == TRUE)
		{
			codeReturned = TRUE;
		}
		
		pSetting = pSetting->pSettingNext;
	}
	
	return codeReturned;
}

/**
 * gok_data_backup_setting:
 * @pSetting: Pointer to the setting that will be backed up.
 *
 * Backs up the values for the given setting.
 */
void gok_data_backup_setting (GokSetting* pSetting)
{
	g_assert (pSetting != NULL);
	
	if (pSetting->pValueString == NULL)
	{
		if (pSetting->pValueStringBackup != NULL)
		{
			g_free (pSetting->pValueStringBackup);
			pSetting->pValueStringBackup = NULL;
		}
	}
	else
	{
		if ((pSetting->pValueStringBackup == NULL) ||
			(strcmp (pSetting->pValueString, pSetting->pValueStringBackup) != 0))
		{
			if (pSetting->pValueStringBackup != NULL)
			{
				g_free (pSetting->pValueStringBackup);
			}
			
			pSetting->pValueStringBackup = (gchar*)g_malloc (strlen (pSetting->pValueString) + 1);
			strcpy (pSetting->pValueStringBackup, pSetting->pValueString);
		}
	}

	pSetting->ValueBackup = pSetting->Value;
}

/**
 * gok_data_restore_setting:
 * @pSetting: Pointer to the setting that will be restored.
 *
 * Restores the values for the given setting.
 *
 * Returns: TRUE if the backup setting is different from the current
 * setting.  Returns FALSE if the backup setting is the same as the
 * current setting.
 */
gboolean gok_data_restore_setting (GokSetting* pSetting)
{
	gboolean codeReturned;
	g_assert (pSetting != NULL);
	
	codeReturned = FALSE;
	
	if (pSetting->pValueStringBackup == NULL)
	{
		if (pSetting->pValueString != NULL)
		{
			g_free (pSetting->pValueString);
			pSetting->pValueString = NULL;

			codeReturned = TRUE;
		}
	}
	else
	{
		if ((pSetting->pValueString == NULL) ||
			(strcmp (pSetting->pValueString, pSetting->pValueStringBackup) != 0))
		{
			if (pSetting->pValueString != NULL)
			{
				g_free (pSetting->pValueString);
			}
			pSetting->pValueString = (gchar*)g_malloc (strlen (pSetting->pValueStringBackup) + 1);
			strcpy (pSetting->pValueString, pSetting->pValueStringBackup);

			codeReturned = TRUE;
		}
	}

	if (pSetting->ValueBackup != pSetting->Value)
	{
		pSetting->Value = pSetting->ValueBackup;
		codeReturned = TRUE;
	}
	
	return codeReturned;
}

gdouble
gok_data_get_valuator_sensitivity (void)
{
	if (gok_main_get_valuatorsensitivity_override() != 0) {
		return gok_main_get_valuatorsensitivity_override();
	}
	return m_ValuatorSensitivity;
}

void 
gok_data_set_valuator_sensitivity (gdouble multiplier)
{
	if (gok_main_get_valuatorsensitivity_override() != 0) {
		return;
	}
	m_ValuatorSensitivity = multiplier;
	gok_gconf_set_double (gconf_client, GOK_GCONF_VALUATOR_SENSITIVITY,
			      m_ValuatorSensitivity);
	m_bSettingsChanged = TRUE;
}

const gchar*
gok_data_compose_keyboard_type_string (GokComposeType type)
{
    switch (type)
    {
	case GOK_COMPOSE_XKB:
	    return "xkb";
	case GOK_COMPOSE_ALPHA:
	    return "alpha";
	case GOK_COMPOSE_ALPHAFREQ:
	    return "alpha-freq";
	case GOK_COMPOSE_CUSTOM:
	    return "custom";
	case GOK_COMPOSE_DEFAULT:
	default:
	    return "default";
    }
}

static const GokComposeType
gok_data_compose_type_from_string (const gchar *s)
{
	if (!strcmp ("xkb", s)) return GOK_COMPOSE_XKB;
	else if (!strcmp ("alpha", s)) return GOK_COMPOSE_ALPHA;
	else if (!strcmp ("alpha-freq", s)) return GOK_COMPOSE_ALPHAFREQ;
	else if (!strcmp ("custom", s)) return GOK_COMPOSE_CUSTOM;
	else return GOK_COMPOSE_DEFAULT;
}

GokComposeType 
gok_data_get_compose_keyboard_type (void)
{
    return compose_keyboard_type;
}

void 
gok_data_set_compose_keyboard_type (GokComposeType type)
{
    compose_keyboard_type = type;
    gok_gconf_set_string (gconf_client, GOK_GCONF_COMPOSE_KBD_TYPE,
			  gok_data_compose_keyboard_type_string (type));
    gok_data_set_use_xkb_kbd (type == GOK_COMPOSE_XKB);    
}

gchar *
gok_data_get_custom_compose_filename (void)
{
	gchar *s;
	return (gok_gconf_get_string (gconf_client, GOK_GCONF_CUSTOM_COMPOSE_FILENAME,
				     &s) ? s : NULL);
}

void 
gok_data_set_custom_compose_filename (const gchar *filename)
{
	gok_gconf_set_string (gconf_client, GOK_GCONF_CUSTOM_COMPOSE_FILENAME, filename);
}

gchar *
gok_data_get_aux_keyboard_directory (void)
{
	gchar *s;
	return (gok_gconf_get_string (gconf_client, GOK_GCONF_AUX_KEYBOARD_DIRECTORY,
				     &s) ? s : NULL);
}

void 
gok_data_set_aux_keyboard_directory (const gchar *dirname)
{
	gok_gconf_set_string (gconf_client, GOK_GCONF_AUX_KEYBOARD_DIRECTORY, dirname);
}


gint
gok_data_get_repeat_rate (void)
{
	/* TODO: implement in gconf */
	return 50;
}

void 
gok_data_set_repeat_rate (gint rate)
{
	/* TODO: implement in gconf */
}


/* 
 * gok_data_get_aux_dictionaries:
 *
 * Returns a string which specifies a semicolon-delimited
 * list of fully-qualified paths to word lists.
 */
gchar* 
gok_data_get_aux_dictionaries (void)
{
	return m_AuxDicts;
}

/* 
 * gok_data_set_aux_dictionaries:
 *
 */
void
gok_data_set_aux_dictionaries (gchar *dictionaries)
{
	m_AuxDicts = dictionaries;
	gok_gconf_set_string (gconf_client, GOK_GCONF_AUX_DICTS,
			      m_AuxDicts);
	m_bSettingsChanged = TRUE;
}

gboolean gok_data_get_use_aux_dictionaries (void)
{	
	return m_bUseAuxDicts;
}

void gok_data_set_use_aux_dictionaries (gboolean use_aux)
{	
	m_bUseAuxDicts = use_aux;
	gok_gconf_set_bool (gconf_client, GOK_GCONF_USE_AUX_DICTS, m_bUseAuxDicts);
	m_bSettingsChanged = TRUE;
}
