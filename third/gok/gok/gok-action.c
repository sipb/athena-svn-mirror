/* gok-action.c
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

#include <sys/stat.h>
#include <sys/types.h>
#include <gconf/gconf-client.h>
#include "gok-action.h"
#include "gok-log.h"
#include "gok-gconf.h"
#include "gok-gconf-keys.h"

/* Pointer to the first action in the list of actions */
static GokAction* m_pActionFirst;

/* This flag will be set TRUE if a new action is created by the user */
static gboolean m_bActionsChanged;

/* Private functions */
void gok_action_unset_deleted_actions_in_gconf ();
gboolean gok_action_store_actions_in_gconf ();
gchar* gok_action_get_key (const gchar* action_name,
                           const gchar* attribute);

/**
 * gok_action_new:
 *
 * Allocates memory for a new GokAction and initializes its attributes
 * to default values. Please free the returned GokAction using
 * gok_action_free.
 *
 * Returns: a new GokAction
 */
GokAction* gok_action_new ()
{
    GokAction* new_action;

    new_action = (GokAction*)g_malloc (sizeof (GokAction));
    new_action->pName = NULL;
    new_action->pNameBackup = NULL;
    new_action->pDisplayName = NULL;
    new_action->pDisplayNameBackup = NULL;
    new_action->Type = ACTION_TYPE_SWITCH;
    new_action->TypeBackup = new_action->Type;
    new_action->State = ACTION_STATE_UNDEFINED;
    new_action->StateBackup = new_action->State;
    new_action->Number = 0;
    new_action->NumberBackup = new_action->Number;
    new_action->Rate = 0;
    new_action->RateBackup = new_action->Rate;
    new_action->bPermanent = FALSE;
    new_action->bNewAction = FALSE;
    new_action->bKeyAveraging = FALSE;
    new_action->bKeyAveragingBackup = FALSE;
    new_action->pActionNext = NULL;
    return new_action;
}

/**
 * gok_action_free:
 * @action: a GokAction
 *
 * Frees a GokAction and associated resources.
 */
void gok_action_free (GokAction* action)
{
    if (action->pName != NULL)
    {
        g_free (action->pName);
    }
    if (action->pDisplayName != NULL)
    {
        g_free (action->pDisplayName);
    }
    g_free (action);
}

/**
 * gok_action_open:
 *
 * Initializes the gok actions and reads them from GConf.
 *
 * Returns: TRUE if the actions were initialized without problem,
 *          FALSE if problems occured.
 */
gboolean gok_action_open()
{
    GConfClient* gconf_client = NULL;
    GSList* action_dirs;
    GSList* action_dir;
    gchar* action_name;
    gchar* key;
    gboolean successful;
    gchar* displayname;
    gchar* type_as_string;
    gint type;
    gchar* state_as_string;
    gint state;
    gint number;
    gint rate;
    gboolean permanent;
    gboolean key_averaging;
    GokAction* new_action;
    GokAction* prev_action;

    gok_log_enter ();

    m_pActionFirst = NULL;
    m_bActionsChanged = FALSE;

    gconf_client = gconf_client_get_default ();
    
    action_dirs = gconf_client_all_dirs (gconf_client,
                                         GOK_GCONF_ACTIONS,
                                         NULL);

    /* Loop through the actions */

    gok_log ("Looping through actions");

    action_dir = action_dirs;
    while (action_dir != NULL)
    {
        gok_log ("Found action_dir %s", action_dir->data);
        action_name = g_strrstr (action_dir->data, "/");
        if ( (action_name != NULL) && ( strlen (action_name) >= 2 ) )
        {
            /* Step over "/" */
            action_name++;
            gok_log ("action_name = %s", action_name);

            /*
             * We are now going to look for these action attributes in
             * GConf:
             *
             *   - displayname
             *   - type
             *   - state
             *   - number
             *   - rate
             *   - whether it is a permanent action or not
             *   - key averaging
             */

            /*
             * Displayname
             */

            key = gok_action_get_key (action_name,
                                      GOK_GCONF_ACTION_DISPLAYNAME);
            /* There is no need to g_free displayname because it is
             * freed in gok_action_close.
             */
            successful = gok_gconf_get_string (gconf_client, key,
                                               &displayname);
            g_free (key);
            if (!successful) {
                return FALSE;
            }

            /*
             * Type
             */

            key = gok_action_get_key (action_name, GOK_GCONF_ACTION_TYPE);
            successful = gok_gconf_get_string (gconf_client, key,
                                               &type_as_string);
            if (successful)
            {
                if (strcmp(type_as_string,
                           GOK_GCONF_ACTION_TYPE_SWITCH) == 0)
                {
                    type = ACTION_TYPE_SWITCH;
                }
                else if (strcmp(type_as_string,
                                GOK_GCONF_ACTION_TYPE_MOUSEBUTTON) == 0)
                {
                    type = ACTION_TYPE_MOUSEBUTTON;
                }
                else if (strcmp(type_as_string,
                                GOK_GCONF_ACTION_TYPE_MOUSEPOINTER) == 0)
                {
                    type = ACTION_TYPE_MOUSEPOINTER;
                }
                else if (strcmp(type_as_string,
                                GOK_GCONF_ACTION_TYPE_DWELL) == 0)
                {
                    type = ACTION_TYPE_DWELL;
                }
                else if (strcmp(type_as_string,
                                GOK_GCONF_ACTION_TYPE_KEY) == 0)
                {
                    type = ACTION_TYPE_KEY;
                }
                else
                {
                    gok_log_x ("Unrecognised type %s in key %s",
                               type_as_string, key);
                }

                g_free (key);
                g_free (type_as_string);
            }
            else
            {
                g_free (key);
                return FALSE;
            }

            /*
             * State
             */

            key = gok_action_get_key (action_name, GOK_GCONF_ACTION_STATE);
            successful = gok_gconf_get_string (gconf_client, key,
                                               &state_as_string);
            if (successful)
            {
                if (strcmp(state_as_string,
                           GOK_GCONF_ACTION_STATE_UNDEFINED) == 0)
                {
                    state = ACTION_STATE_UNDEFINED;
                }
                else if (strcmp(state_as_string,
                           GOK_GCONF_ACTION_STATE_PRESS) == 0)
                {
                    state = ACTION_STATE_PRESS;
                }
                else if (strcmp(state_as_string,
                                GOK_GCONF_ACTION_STATE_RELEASE) == 0)
                {
                    state = ACTION_STATE_RELEASE;
                }
                else if (strcmp(state_as_string,
                                GOK_GCONF_ACTION_STATE_CLICK) == 0)
                {
                    state = ACTION_STATE_CLICK;
                }
                else if (strcmp(state_as_string,
                                GOK_GCONF_ACTION_STATE_DOUBLECLICK) == 0)
                {
                    state = ACTION_STATE_DOUBLECLICK;
                }
                else if (strcmp(state_as_string,
                                GOK_GCONF_ACTION_STATE_ENTER) == 0)
                {
                    state = ACTION_STATE_ENTER;
                }
                else if (strcmp(state_as_string,
                                GOK_GCONF_ACTION_STATE_LEAVE) == 0)
                {
                    state = ACTION_STATE_LEAVE;
                }
                else
                {
                    gok_log_x ("Unrecognised state %s in key %s",
                               state_as_string, key);
                }

                g_free (key);
                g_free (state_as_string);
            }
            else
            {
                g_free (key);
                return FALSE;
            }

            /*
             * Number
             */

            key = gok_action_get_key (action_name, GOK_GCONF_ACTION_NUMBER);
            successful = gok_gconf_get_int (gconf_client, key, &number);
            g_free (key);
            if (!successful)
            {
                return FALSE;
            }

            /*
             * Rate
             */

            key = gok_action_get_key (action_name, GOK_GCONF_ACTION_RATE);
            successful = gok_gconf_get_int (gconf_client, key, &rate);
            g_free (key);
            if (!successful)
            {
                return FALSE;
            }

            /*
             * Permanent
             */
            
            key = gok_action_get_key (action_name, GOK_GCONF_ACTION_PERMANENT);
            successful = gok_gconf_get_bool (gconf_client, key, &permanent);
            g_free (key);
            if (!successful)
            {
                return FALSE;
            }

            /*
             * Key averaging
             */

            key = gok_action_get_key (action_name,
                                      GOK_GCONF_ACTION_KEY_AVERAGING);
            successful = gok_gconf_get_bool (gconf_client, key,
                                             &key_averaging);

            if (!successful)
            {
                return FALSE;
            }

            /*
             * We are now going to build a new action and add it to
             * the list of actions.
             */

            new_action = (GokAction*)g_malloc (sizeof (GokAction));

            new_action->pName = g_strdup (action_name);
            new_action->pNameBackup = NULL;
            new_action->pDisplayName = displayname;
            new_action->pDisplayNameBackup = NULL;
            new_action->Type = type;
            new_action->TypeBackup = type;
            new_action->State = state;
            new_action->StateBackup = state;
            new_action->Number = number;
            new_action->NumberBackup = number;
            new_action->Rate = rate;
            new_action->RateBackup = rate;
            new_action->bPermanent = permanent;
            new_action->bNewAction = FALSE;
            new_action->bKeyAveraging = key_averaging;
            new_action->pActionNext = NULL;

            if (m_pActionFirst == NULL)
            {
                m_pActionFirst = new_action;
            }
            else
            {
                prev_action->pActionNext = new_action;
            }
            prev_action = new_action;
        }
        g_free (action_dir->data);
        action_dir = action_dir->next;
    }
    g_slist_free (action_dirs);

    gok_log ("Done looping through actions");
    gok_log_leave ();

    return TRUE;
}

/**
 * gok_action_close:
 *
 * Closes the gok actions and saves them in GConf.
 */
void gok_action_close ()
{
    gboolean successful;
    GokAction* action;
    GokAction* pActionTemp;

    gok_log_enter ();

    gok_action_unset_deleted_actions_in_gconf ();

    successful = gok_action_store_actions_in_gconf ();
    if (successful)
    {
        gok_log ("Successfully stored actions in GConf");
    }
    else
    {
        gok_log_x ("Problem(s) storing actions in GConf");

        /* TODO: handle unsuccessful setting of actions in GConf
         *
         * For example abort closing down of gok and notify user
         */
    }

    /* Delete all the actions */

    gok_log ("Deleting all of the actions");

    action = m_pActionFirst;
    while (action != NULL)
    {
        pActionTemp = action;
        action = action->pActionNext;
        gok_action_free (pActionTemp);
    }

    m_pActionFirst = NULL;

    gok_log ("Done deleting all of the actions");

    gok_log_leave ();
}

/**
 * gok_action_unset_deleted_actions_in_gconf:
 *
 * Unsets any actions that are in GConf but which are not in memory.
 */
void gok_action_unset_deleted_actions_in_gconf ()
{
    GConfClient* gconf_client = NULL;
    GSList* action_dirs;
    GSList* action_dir;
    gchar* action_name;
    GSList* action_entries;
    GSList* action_entry;
    GConfEntry* entry;

    gok_log_enter ();

    gconf_client = gconf_client_get_default ();
    
    action_dirs = gconf_client_all_dirs (gconf_client,
                                         GOK_GCONF_ACTIONS,
                                         NULL);

    /* Loop through the actions */

    gok_log ("Looping through actions");

    action_dir = action_dirs;
    while (action_dir != NULL)
    {
        /* gok_log ("Found action_dir %s", action_dir->data); */
        action_name = g_strrstr (action_dir->data, "/");
        if ( (action_name != NULL) && ( strlen (action_name) >= 2 ) )
        {
            /* Step over "/" */
            action_name++;
            /* gok_log ("action_name = %s", action_name); */

            if (gok_action_find_action (action_name, FALSE) == NULL)
            {
                gok_log ("Unsetting action %s", action_name);

                action_entries = gconf_client_all_entries (gconf_client,
                                                           action_dir->data,
                                                           NULL);
                /* Loop through the entries */

                action_entry = action_entries;
                while (action_entry != NULL)
                {
                    entry = action_entry->data;
                    gok_log ("Unsetting %s", entry->key);
                    gconf_client_unset (gconf_client, entry->key, NULL);
                    g_free (entry);
                    action_entry = action_entry->next;
                }
                g_slist_free (action_entries);

                gok_log ("Unsetting %s", action_dir->data);
                gconf_client_unset (gconf_client, action_dir->data, NULL);
            }
        }

        g_free (action_dir->data);
        action_dir = action_dir->next;
    }
    g_slist_free (action_dirs);

    gok_log ("Done looping through actions");
    gok_log_leave ();
}

/**
 * gok_action_store_actions_in_gconf:
 *
 * Stores the actions in GConf.  If a problem occurs when writing
 * to GConf then storing the actions is aborted at that point and
 * the function will return with FALSE.
 *
 * Returns: TRUE if the actions were stored without problem,
 *          FALSE if problems occured.
 */
gboolean gok_action_store_actions_in_gconf ()
{
    gboolean successful;
    GConfClient* gconf_client = NULL;
    gchar* key;
    gchar* type_as_string;
    GokAction* action;
    gboolean found_value = FALSE;
    gchar* state_as_string;

    gok_log_enter ();

    gconf_client = gconf_client_get_default ();

    successful = gok_gconf_set_string (gconf_client,
                                       GOK_GCONF_ACTIONS_WORKAROUND,
                                       GOK_GCONF_WORKAROUND_TEXT);

    if (!successful)
        return FALSE;

    gok_log ("Looping through actions");

    action = m_pActionFirst;
    while (action != NULL)
    {
        /*
         * Only save actions with non-NULL names
         */
        if (action->pName != NULL)
        {
            /*
             * We are now going to write GConf (key, value) pairs for each
             * action attribute:
             *
             *   - displayname
             *   - type
             *   - state
             *   - number
             *   - rate
             *   - whether it is a permanent action or not
             *   - key averaging
             */

            /*
             * Displayname
             */

            key = gok_action_get_key (action->pName,
                                      GOK_GCONF_ACTION_DISPLAYNAME);

            if (action->pDisplayName != NULL) {
                successful = gok_gconf_set_string (gconf_client, key,
                                                   action->pDisplayName);
                if (!successful)
                    return FALSE;
            }
            else
            {
                /* If pDisplayName is NULL default to pName */
                successful = gok_gconf_set_string (gconf_client, key,
                                                   action->pName);
                if (!successful)
                    return FALSE;
            }
            g_free (key);

            /*
             * Type
             */

            key = gok_action_get_key (action->pName, GOK_GCONF_ACTION_TYPE);

            found_value = FALSE;

            switch (action->Type)
            {
            case ACTION_TYPE_SWITCH:
                type_as_string = GOK_GCONF_ACTION_TYPE_SWITCH;
                found_value = TRUE;
                break;
            case ACTION_TYPE_MOUSEBUTTON:
                type_as_string = GOK_GCONF_ACTION_TYPE_MOUSEBUTTON;
                found_value = TRUE;
                break;
            case ACTION_TYPE_MOUSEPOINTER:
                type_as_string = GOK_GCONF_ACTION_TYPE_MOUSEPOINTER;
                found_value = TRUE;
                break;
            case ACTION_TYPE_DWELL:
                type_as_string = GOK_GCONF_ACTION_TYPE_DWELL;
                found_value = TRUE;
                break;
            default:
                gok_log_x ("action->Type is set to an unrecognised value");
                found_value = FALSE;
                break;
            }
        
            if (found_value)
            {
                successful = gok_gconf_set_string (gconf_client, key,
                                                   type_as_string);
                if (!successful)
                    return FALSE;
            }

            g_free (key);

            /*
             * State
             */

            key = gok_action_get_key (action->pName, GOK_GCONF_ACTION_STATE);

            found_value = FALSE;

            switch (action->State)
            {
            case ACTION_STATE_UNDEFINED:
                state_as_string = GOK_GCONF_ACTION_STATE_UNDEFINED;
                found_value = TRUE;
                break;
            case ACTION_STATE_PRESS:
                state_as_string = GOK_GCONF_ACTION_STATE_PRESS;
                found_value = TRUE;
                break;
            case ACTION_STATE_RELEASE:
                state_as_string = GOK_GCONF_ACTION_STATE_RELEASE;
                found_value = TRUE;
                break;
            case ACTION_STATE_CLICK:
                state_as_string = GOK_GCONF_ACTION_STATE_CLICK;
                found_value = TRUE;
                break;
            case ACTION_STATE_DOUBLECLICK:
                state_as_string = GOK_GCONF_ACTION_STATE_DOUBLECLICK;
                found_value = TRUE;
                break;
            case ACTION_STATE_ENTER:
                state_as_string = GOK_GCONF_ACTION_STATE_ENTER;
                found_value = TRUE;
                break;
            case ACTION_STATE_LEAVE:
                state_as_string = GOK_GCONF_ACTION_STATE_LEAVE;
                found_value = TRUE;
                break;
            default:
                gok_log_x ("action->State is set to an unrecognised value");
                found_value = FALSE;
                break;
            }

            if (found_value)
            {
                successful = gok_gconf_set_string (gconf_client, key,
                                                   state_as_string);
                if (!successful)
                    return FALSE;
            }

            g_free (key);

            /*
             * Number
             */

            key = gok_action_get_key (action->pName, GOK_GCONF_ACTION_NUMBER);
            successful = gok_gconf_set_int (gconf_client, key, action->Number);
            g_free (key);
            if (!successful)
                return FALSE;

            /*
             * Rate
             */

            key = gok_action_get_key (action->pName, GOK_GCONF_ACTION_RATE);
            successful = gok_gconf_set_int (gconf_client, key, action->Rate);
            g_free (key);
            if (!successful)
                return FALSE;

            /*
             * Permanent
             */

            key = gok_action_get_key (action->pName,
                                      GOK_GCONF_ACTION_PERMANENT);
            successful = gok_gconf_set_bool (gconf_client, key,
                                             action->bPermanent);
            g_free (key);
            if (!successful)
                return FALSE;

            /*
             * Key averaging
             */

            key = gok_action_get_key (action->pName,
                                      GOK_GCONF_ACTION_KEY_AVERAGING);
            successful = gok_gconf_set_bool (gconf_client, key,
                                             action->bKeyAveraging);
            g_free (key);
            if (!successful)
                return FALSE;
        }

        action = action->pActionNext;
    }

    gok_log ("Done looping through actions");
    gok_log_leave ();

    return TRUE;
}

/**
 * gok_action_get_first_action:
 *
 * Returns: A pointer to the first action in the list of actions.
 */
GokAction* gok_action_get_first_action ()
{
    return m_pActionFirst;
}

/**
 * gok_action_delete_action:
 * @pActionDelete: Pointer to the action that will be deleted.
 */
void gok_action_delete_action (GokAction* pActionDelete)
{
    GokAction* pAction;
    GokAction* pActionPrevious;

    gok_log_enter ();
    
    pActionPrevious = NULL;
    pAction = m_pActionFirst;
    while (pAction != NULL)
    {
        if (pActionDelete == pAction)
        {
            /* unhook the action from our list */
            if (pActionDelete == m_pActionFirst)
            {
                m_pActionFirst = pActionDelete->pActionNext;
            }
            else
            {
                pActionPrevious->pActionNext = pAction->pActionNext;
            }

            gok_action_free (pActionDelete);

            break;
        }
        
        pActionPrevious = pAction;
        pAction = pAction->pActionNext;
    }

    gok_log_leave ();
}

/**
 * gok_action_set_modified:
 * @bTrueFalse: New state of the 'modified' flag for the gok actions.
 *
 * Sets the 'modified' flag for the gok actions. If the flag is TRUE then
 * the actions are written to a file when the program ends.
 * This should be called any time an action is added or modified.
 */
void gok_action_set_modified (gboolean bTrueFalse)
{
    m_bActionsChanged = bTrueFalse;
}

/**
 * gok_action_find_action:
 * @NameAction: Name of the action you're trying to find.
 * @bDisplayName: This should be set TRUE if you are passing in the display
 * name of the action. It should be set FALSE if passing in the actual
 * name of the action.
 *
 * Finds the action from within our list of actions.
 *
 * Returns: A pointer to the action, NULL if not found.
 */
GokAction* gok_action_find_action (gchar* NameAction, gboolean bDisplayName)
{
    GokAction* pAction;
    
    if (NameAction == NULL)
    {
        return NULL;
    }
    
    if (strlen (NameAction) == 0)
    {
        return NULL;
    }
    
    pAction = m_pActionFirst;
    while (pAction != NULL)
    {
    	if (bDisplayName == TRUE)
    	{
	        if (strcmp (pAction->pDisplayName, NameAction) == 0)
	        {
	            return pAction;
	        }
    	}
    	else
    	{
	        if (strcmp (pAction->pName, NameAction) == 0)
	        {
	            return pAction;
	        }
    	}
        
        pAction = pAction->pActionNext;
    }
    
    return NULL;
}

/**
 * gok_action_add_action:
 * @pActionNew: Pointer to an action that gets added to our list of actions.
 */
void gok_action_add_action (GokAction* pActionNew)
{
    GokAction* pAction;
    
    if (m_pActionFirst == NULL)
    {
        m_pActionFirst = pActionNew;
        return;
    }
    
    pAction = m_pActionFirst;
    while (pAction->pActionNext != NULL)
    {
        pAction = pAction->pActionNext;
    }
    
    pAction->pActionNext = pActionNew;
}

/**
 * gok_action_get_key:
 * @action_name:       The action to build the key for.
 * @attribute:         The attribute to build the key for.
 *
 * Builds a GConf key name from action_name and attribute. The
 * returned key name is of the form:
 *
 * GOK_GCONF_ACTIONS/action_name/attribute
 *
 * NOTE: The returned gchar* needs to be freed when finished with.
 *
 * Returns: a GConf key
 */
gchar* gok_action_get_key (const gchar* action_name,
                           const gchar* attribute)
{
    return g_strconcat (GOK_GCONF_ACTIONS, "/",
                        action_name, "/",
                        attribute, NULL);
}

/**
 * gok_action_revert:
 * @pAction: Pointer to the action that will be reverted.
 *
 * Moves the backup data to the current data on the action.
 *
 * Returns: TRUE if any of the reverted settings are different from
 * the current settings. FALSE if the reverted settings are the same
 * as the current settings.
 */
gboolean gok_action_revert (GokAction* pAction)
{
	gboolean bSettingsChanged = FALSE;

	/* revert the name */
	/* don't revert to a NULL name */
	if (pAction->pNameBackup != NULL)
	{
		if (pAction->pName != NULL)
		{
			/* is the backup name the same as the name? */
			if (strcmp (pAction->pName, pAction->pNameBackup) != 0)
			{
				/* no, free the name and copy the backup name to the name */
				g_free (pAction->pName);
				pAction->pName = g_malloc (strlen (pAction->pNameBackup) + 1);
				strcpy (pAction->pName, pAction->pNameBackup);

				bSettingsChanged = TRUE;
			}
		}
		else /* name is NULL */
		{
			pAction->pName = g_malloc (strlen (pAction->pNameBackup) + 1);
			strcpy (pAction->pName, pAction->pNameBackup);

			bSettingsChanged = TRUE;
		}
	}

	/* revert the display name */
	/* don't revert to a NULL name */
	if (pAction->pDisplayNameBackup != NULL)
	{
		if (pAction->pDisplayName != NULL)
		{
			/* is the backup name the same as the name? */
			if (strcmp (pAction->pDisplayNameBackup, pAction->pDisplayName) != 0)
			{
				/* no, free the backup name and copy the name to backup */
				g_free (pAction->pDisplayName);
				pAction->pDisplayName = g_malloc (strlen (pAction->pDisplayNameBackup) + 1);
				strcpy (pAction->pDisplayName, pAction->pDisplayNameBackup);

				bSettingsChanged = TRUE;
			}
		}
		else /* backup name is NULL */
		{
			pAction->pDisplayName = g_malloc (strlen (pAction->pDisplayNameBackup) + 1);
			strcpy (pAction->pDisplayName, pAction->pDisplayNameBackup);

			bSettingsChanged = TRUE;
		}
	}

	if (pAction->Type != pAction->TypeBackup)
	{
		pAction->Type = pAction->TypeBackup;
		bSettingsChanged = TRUE;
	}
	if (pAction->State != pAction->StateBackup)
	{
		pAction->State = pAction->StateBackup;
		bSettingsChanged = TRUE;
	}
	if (pAction->Number != pAction->NumberBackup)
	{
		pAction->Number = pAction->NumberBackup;
		bSettingsChanged = TRUE;
	}
	if (pAction->Rate != pAction->RateBackup)
	{
		pAction->Rate = pAction->RateBackup;
		bSettingsChanged = TRUE;
	}
	if (pAction->bKeyAveraging != pAction->bKeyAveragingBackup)
	{
		pAction->bKeyAveraging = pAction->bKeyAveragingBackup;
		bSettingsChanged = TRUE;
	}

	return bSettingsChanged;
}

/**
 * gok_action_backup:
 * @pAction: Pointer to the action that will be backed up.
 *
 * Creates a copy of all the data stored on the action.
 */
void gok_action_backup (GokAction* pAction)
{
	/* backup the name */
	if (pAction->pName == NULL)
	{
		if (pAction->pNameBackup != NULL)
		{
			g_free (pAction->pNameBackup);
			pAction->pNameBackup = NULL;
		}
	}
	else /* name is not NULL */
	{
		if (pAction->pNameBackup != NULL)
		{
			/* is the backup name the same as the name? */
			if (strcmp (pAction->pName, pAction->pNameBackup) != 0)
			{
				/* no, free the backup name and copy the name to backup */
				g_free (pAction->pNameBackup);
				pAction->pNameBackup = g_malloc (strlen (pAction->pName) + 1);
				strcpy (pAction->pNameBackup, pAction->pName);
			}
		}
		else /* backup name is NULL */
		{
			pAction->pNameBackup = g_malloc (strlen (pAction->pName) + 1);
			strcpy (pAction->pNameBackup, pAction->pName);
		}
	}
	
	/* backup the display name */
	if (pAction->pDisplayName == NULL)
	{
		if (pAction->pDisplayNameBackup != NULL)
		{
			g_free (pAction->pDisplayNameBackup);
			pAction->pDisplayNameBackup = NULL;
		}
	}
	else /* display name is not NULL */
	{
		if (pAction->pDisplayNameBackup != NULL)
		{
			/* is the backup name the same as the name? */
			if (strcmp (pAction->pDisplayName, pAction->pDisplayNameBackup) != 0)
			{
				/* no, free the backup name and copy the name to backup */
				g_free (pAction->pDisplayNameBackup);
				pAction->pDisplayNameBackup = g_malloc (strlen (pAction->pDisplayName) + 1);
				strcpy (pAction->pDisplayNameBackup, pAction->pDisplayName);
			}
		}
		else /* backup name is NULL */
		{
			pAction->pDisplayNameBackup = g_malloc (strlen (pAction->pDisplayName) + 1);
			strcpy (pAction->pDisplayNameBackup, pAction->pDisplayName);
		}
	}

	pAction->TypeBackup = pAction->Type;
	pAction->StateBackup = pAction->State;
	pAction->NumberBackup = pAction->Number;
	pAction->RateBackup = pAction->Rate;
	pAction->bKeyAveragingBackup = pAction->bKeyAveraging;
	
	/* make this action not new */
	pAction->bNewAction = FALSE;
}

/**
* gok_action_get_name
*
* @pDisplayName: Display name of the action.
*
* returns: The static (gconf) name of the action.
**/
gchar* gok_action_get_name (gchar* pDisplayName)
{
	GokAction* pAction;
	
	if ((pDisplayName == NULL) ||
		(strlen (pDisplayName) == 0))
	{
		return "";
	}

	pAction = m_pActionFirst;
	while (pAction != NULL)
	{
		if (strcmp (pDisplayName, pAction->pDisplayName) == 0)
		{
			return pAction->pName;
		}
		pAction = pAction->pActionNext;
	}
	
	gok_log_x ("Can't find name for displayname: %s", pDisplayName);
	
	return "";
}

/**
* gok_action_get_displayname
*
* @pName: Name of the action.
*
* returns: The display name of the action.
**/
gchar* gok_action_get_displayname (gchar* pName)
{
	GokAction* pAction;
	
	if ((pName == NULL) ||
		(strlen (pName) == 0))
	{
		return "";
	}

	pAction = m_pActionFirst;
	while (pAction != NULL)
	{
		if (strcmp (pName, pAction->pName) == 0)
		{
			return pAction->pDisplayName;
		}
		pAction = pAction->pActionNext;
	}
	
	gok_log_x ("Can't find display name for name: %s", pName);
	
	return "";
}

