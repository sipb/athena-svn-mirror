/* gok-feedback.c
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
#include "main.h"
#include "gok-feedback.h"
#include "gok-log.h"
#include "gok-gconf.h"
#include "gok-gconf-keys.h"
#include "gok-sound.h"
#include "gok-speech.h"
#include "gok-key.h"

/* Pointer to the first feedback in the list of feedback */
static GokFeedback* m_pFeedbackFirst;

/* key that is flashed */
static GokKey* m_pKeyflashKey;
static gint m_KeyflashTimerCount;
static guint m_KeyflashTimerId;
static gboolean m_bFlashState;

/* key that is currently selected (this may be different from key highlighted) */
static GokKey* m_pKeySelected;

static gchar*  m_keyLabel = NULL;

/* key that is currently hilighted */
static GokKey* m_pKeyHighlighted;

/* Private functions */
void gok_feedback_unset_deleted_feedbacks_in_gconf ();
gchar* gok_feedback_get_key (const gchar* feedback_name,
                             const gchar* attribute);

/**
* gok_feedback_new:
*
* Allocates memory for a new GokFeedback and initializes its attributes
* to default values. Please free the returned GokFeedback using
* gok_feedback_free.
*
* returns: A pointer to a new GokFeedback.
**/
GokFeedback* gok_feedback_new ()
{
    GokFeedback* pNewFeedback;

    pNewFeedback = (GokFeedback*)g_malloc (sizeof (GokFeedback));
    pNewFeedback->bPermanent = FALSE;
    pNewFeedback->bNewFeedback = FALSE;

    pNewFeedback->pName = NULL;
    pNewFeedback->pNameBackup = NULL;
    pNewFeedback->pDisplayName = NULL;
    pNewFeedback->pDisplayNameBackup = NULL;
    pNewFeedback->bFlashOn = TRUE;
    pNewFeedback->bFlashOnBackup = TRUE;
    pNewFeedback->NumberFlashes = 4;
    pNewFeedback->NumberFlashesBackup = 4;
    pNewFeedback->bSoundOn = FALSE;
    pNewFeedback->bSoundOnBackup = FALSE;
    pNewFeedback->pNameSound = NULL;
    pNewFeedback->pNameSoundBackup = NULL;
    pNewFeedback->pFeedbackNext = NULL;
    pNewFeedback->bSpeechOn = FALSE;

    return pNewFeedback;
}

/**
* gok_feedback_free
* @pFeedback: Pointer to the feedback that will be freed.
*
* Frees a GokFeedback and associated resources.
**/
void gok_feedback_free (GokFeedback* pFeedback)
{
    if (pFeedback->pName != NULL)
    {
        g_free (pFeedback->pName);
    }
    if (pFeedback->pNameBackup != NULL)
    {
        g_free (pFeedback->pNameBackup);
    }
    if (pFeedback->pDisplayName != NULL)
    {
        g_free (pFeedback->pDisplayName);
    }
    if (pFeedback->pDisplayNameBackup != NULL)
    {
        g_free (pFeedback->pDisplayNameBackup);
    }
    if (pFeedback->pNameSound != NULL)
    {
        g_free (pFeedback->pNameSound);
    }
    if (pFeedback->pNameSoundBackup != NULL)
    {
        g_free (pFeedback->pNameSoundBackup);
    }
    
    g_free (pFeedback);
}

/**
* gok_feedback_open
*
* Initializes the gok feedbacks and reads them from GConf.
*
* returns: TRUE if the feedbacks were initialized, FALSE if not.
**/
gboolean gok_feedback_open()
{
    GConfClient* gconf_client = NULL;
    GError* err;
    GSList* feedback_dirs;
    GSList* feedback_dir;
    gchar* feedback_name;
    gchar* key;
    gboolean successful;
    GokFeedback* new_feedback = NULL;

	/* initialize member data */
    m_pFeedbackFirst = NULL;
	m_KeyflashTimerCount = FALSE;
	m_KeyflashTimerId = 0;
	m_pKeyflashKey = NULL;
	m_bFlashState = FALSE;

    gconf_client = gconf_client_get_default ();

    feedback_dirs = gconf_client_all_dirs (gconf_client,
                                           GOK_GCONF_FEEDBACKS,
                                           NULL);

    /* Loop through the feedbacks */

    feedback_dir = feedback_dirs;
    while (feedback_dir != NULL)
    {
        feedback_name = g_strrstr (feedback_dir->data, "/");
        if ( (feedback_name != NULL) && ( strlen (feedback_name) >= 2 ) )
        {
            /* Step over "/" */
            feedback_name++;

            new_feedback = gok_feedback_new ();
            new_feedback->pName = g_strdup (feedback_name);

            /*
             * We are going to look for these feedback attributes in GConf:
             *
             *   - whether it is a permanent feedback or not
             *   - displayname
             *   - whether flash is on or not
             *   - the number of flashes
             *   - whether sound is on or not
             *   - the sound name
             */

            /*
             * Permanent
             */

            key = gok_feedback_get_key (feedback_name,
                                        GOK_GCONF_FEEDBACK_PERMANENT);
            successful = gok_gconf_get_bool (gconf_client, key,
                                             &(new_feedback->bPermanent));
            g_free (key);
            if (!successful)
                return FALSE;

            /*
             * Displayname
             */

            key = gok_feedback_get_key (feedback_name,
                                        GOK_GCONF_FEEDBACK_DISPLAYNAME);
            successful = gok_gconf_get_string (gconf_client, key,
                                               &(new_feedback->pDisplayName));
            g_free (key);
            if (!successful)
                return FALSE;

            /*
             * Flash
             */

            key = gok_feedback_get_key (feedback_name,
                                        GOK_GCONF_FEEDBACK_FLASH);
            successful = gok_gconf_get_bool (gconf_client, key,
                                             &(new_feedback->bFlashOn));
            g_free (key);
            if (!successful)
                return FALSE;

            /*
             * Number of flashes
             */

            key = gok_feedback_get_key (feedback_name,
                                        GOK_GCONF_FEEDBACK_NUMBER_FLASHES);
            successful = gok_gconf_get_int (gconf_client, key,
                                            &(new_feedback->NumberFlashes));
            g_free (key);
            if (!successful)
                return FALSE;

            /*
             * Sound
             */

            key = gok_feedback_get_key (feedback_name,
                                        GOK_GCONF_FEEDBACK_SOUND);
            successful = gok_gconf_get_bool (gconf_client, key,
                                             &(new_feedback->bSoundOn));
            g_free (key);
            if (!successful)
                return FALSE;

            /*
             * Soundname
             *
             * I am not using gok_gconf_get_string here because that
             * function treats the absence of a key as an error.  I do
             * not want that behaviour because it is ok if there is no
             * soundname in GConf. This maps to a NULL pNameSound in
             * new_feedback.
             */

            key = gok_feedback_get_key (feedback_name,
                                        GOK_GCONF_FEEDBACK_SOUNDNAME);
            err = NULL;
            new_feedback->pNameSound = gconf_client_get_string (gconf_client,
                                                                key,
                                                                &err);
            if (err != NULL)
            {
                gok_log_x ("Error getting key %s from GConf", key);
                g_error_free (err);
                new_feedback->pNameSound = NULL;
                g_free (key);
                return FALSE;
            }

            g_free (key);

            key = gok_feedback_get_key (feedback_name,
                                        GOK_GCONF_FEEDBACK_SPEECH);
            successful = gok_gconf_get_bool (gconf_client, key,
                                             &(new_feedback->bSpeechOn));
            g_free (key);
            if (!successful)
                return FALSE;

			/* backup the feedback settings */
			gok_feedback_backup (new_feedback);
			
            /*
             * We are now going to add new_feedback to the list of
             * feedbacks
             */
            gok_feedback_add_feedback (new_feedback);
        }
        g_free (feedback_dir->data);
        feedback_dir = feedback_dir->next;
    }
    g_slist_free (feedback_dirs);

    return TRUE;
}

/**
* gok_feedback_close
*
* Closes the gok feedbacks and saves them in GConf.
*
**/
void gok_feedback_close()
{
    GConfClient* gconf_client = NULL;
    gchar* key;
    gboolean successful;
    GokFeedback* pFeedback;
    GokFeedback* pFeedbackTemp;

    gok_feedback_unset_deleted_feedbacks_in_gconf ();

    gconf_client = gconf_client_get_default ();

    successful = gok_gconf_set_string (gconf_client,
                                       GOK_GCONF_FEEDBACKS_WORKAROUND,
				       GOK_GCONF_WORKAROUND_TEXT);

    /* Loop through the feedbacks */

    pFeedback = m_pFeedbackFirst;

    while (pFeedback != NULL)
    {
        /*
         * Only save feedbacks with non-NULL names
         */
        if (pFeedback->pName != NULL)
        {
            /*
             * We are now going to write GConf (key, value) pairs for each
             * feedback attribute:
             *
             *   - whether it is a permanent feedback or not
             *   - displayname
             *   - whether flash is on or not
             *   - the number of flashes
             *   - whether sound is on or not
             *   - the sound name
             */

            /*
             * Permanent
             */

            key = gok_feedback_get_key (pFeedback->pName,
                                        GOK_GCONF_FEEDBACK_PERMANENT);
            successful = gok_gconf_set_bool (gconf_client, key,
                                             pFeedback->bPermanent);
            g_free (key);

            /*
             * Displayname
             */

            key = gok_feedback_get_key (pFeedback->pName,
                                        GOK_GCONF_FEEDBACK_DISPLAYNAME);

            if (pFeedback->pDisplayName != NULL) {
                successful = gok_gconf_set_string (gconf_client, key,
                                                   pFeedback->pDisplayName);
            }
            else
            {
                /* If pDisplayName is NULL default to pName */
                successful = gok_gconf_set_string (gconf_client, key,
                                                   pFeedback->pName);
            }
            g_free (key);

            /*
             * Flash
             */

            key = gok_feedback_get_key (pFeedback->pName,
                                        GOK_GCONF_FEEDBACK_FLASH);
            successful = gok_gconf_set_bool (gconf_client, key,
                                             pFeedback->bFlashOn);
            g_free (key);

            /*
             * Number of flashes
             */
            
            key = gok_feedback_get_key (pFeedback->pName,
                                        GOK_GCONF_FEEDBACK_NUMBER_FLASHES);
            successful = gok_gconf_set_int (gconf_client, key,
                                            pFeedback->NumberFlashes);
            g_free (key);

            /*
             * Sound
             */

            key = gok_feedback_get_key (pFeedback->pName,
                                        GOK_GCONF_FEEDBACK_SOUND);
            successful = gok_gconf_set_bool (gconf_client, key,
                                             pFeedback->bSoundOn);
            g_free (key);

            /*
             * Soundname
             */

            key = gok_feedback_get_key (pFeedback->pName,
                                        GOK_GCONF_FEEDBACK_SOUNDNAME);
            successful = gok_gconf_set_string (gconf_client, key,
                                               pFeedback->pNameSound);
            g_free (key);

            /*
             * Speech
             */

            key = gok_feedback_get_key (pFeedback->pName,
                                        GOK_GCONF_FEEDBACK_SPEECH);
            successful = gok_gconf_set_bool (gconf_client, key,
                                             pFeedback->bSpeechOn);
            g_free (key);

        }
        
        pFeedback = pFeedback->pFeedbackNext;
    }

    /* delete all the feedbacks */

    gok_log ("Deleting all of the feedbacks");

    pFeedback = m_pFeedbackFirst;
    while (pFeedback != NULL)
    {
        pFeedbackTemp = pFeedback;
        pFeedback = pFeedback->pFeedbackNext;
        gok_feedback_free (pFeedbackTemp);
    }

    m_pFeedbackFirst = NULL;

    gok_log ("Done deleting all of the feedbacks");
}

/**
* gok_feedback_unset_deleted_feedbacks_in_gconf
*
* Unsets any feedbacks that are in GConf but which are not in memory.
**/
void gok_feedback_unset_deleted_feedbacks_in_gconf()
{
    GConfClient* gconf_client = NULL;
    GSList* feedback_dirs;
    GSList* feedback_dir;
    gchar* feedback_name;
    GSList* feedback_entries;
    GSList* feedback_entry;
    GConfEntry* entry;

    gok_log_enter ();

    gconf_client = gconf_client_get_default ();
    
    feedback_dirs = gconf_client_all_dirs (gconf_client,
                                           GOK_GCONF_FEEDBACKS,
                                           NULL);

    /* Loop through the feedbacks */

    gok_log ("Looping through feedbacks");

    feedback_dir = feedback_dirs;
    while (feedback_dir != NULL)
    {
        feedback_name = g_strrstr (feedback_dir->data, "/");
        if ( (feedback_name != NULL) && ( strlen (feedback_name) >= 2 ) )
        {
            /* Step over "/" */
            feedback_name++;

            if (gok_feedback_find_feedback (feedback_name, FALSE) == NULL)
            {
                gok_log ("Unsetting feedback %s", feedback_name);

                feedback_entries = gconf_client_all_entries(gconf_client,
                                                            feedback_dir->data,
                                                            NULL);
                /* Loop through the entries */

                feedback_entry = feedback_entries;
                while (feedback_entry != NULL)
                {
                    entry = feedback_entry->data;
                    gok_log ("Unsetting %s", entry->key);
                    gconf_client_unset (gconf_client, entry->key, NULL);
                    g_free (entry);
                    feedback_entry = feedback_entry->next;
                }
                g_slist_free (feedback_entries);

                gok_log ("Unsetting %s", feedback_dir->data);
                gconf_client_unset (gconf_client, feedback_dir->data, NULL);
            }
        }

        g_free (feedback_dir->data);
        feedback_dir = feedback_dir->next;
    }
    g_slist_free (feedback_dirs);

    gok_log ("Done looping through feedbacks");
    gok_log_leave ();
}

/**
* gok_feedback_get_first_feedback
*
* returns: A pointer to the first feedback in the list of feedbacks.
**/
GokFeedback* gok_feedback_get_first_feedback ()
{
	return m_pFeedbackFirst;
}

/**
* gok_feedback_find_feedback
* @NameFeedback: Name of the feedback you're trying to find.
* @bDisplayName: This should be set TRUE if you are passing in the display
* name of the feedback. It should be set FALSE if passing in the actual
* name of the feedback.
*
* Finds the feedback from within our list of feedbacks.
*
* returns: A pointer to the feedback, NULL if not found.
**/

GokFeedback* gok_feedback_find_feedback (gchar* NameFeedback, gboolean bDisplayName)
{
    GokFeedback* pFeedback;
    
    if (NameFeedback == NULL)
    {
        return NULL;
    }
    
    if (strlen (NameFeedback) == 0)
    {
        return NULL;
    }
    
    pFeedback = m_pFeedbackFirst;
    while (pFeedback != NULL)
    {
    	if (bDisplayName == TRUE)
    	{
	        if (strcmp (pFeedback->pDisplayName, NameFeedback) == 0)
	        {
	            return pFeedback;
	        }
    	}
    	else
    	{
	        if (strcmp (pFeedback->pName, NameFeedback) == 0)
	        {
	            return pFeedback;
	        }
    	}
        
        pFeedback = pFeedback->pFeedbackNext;
    }
    
    return NULL;
}

/**
* gok_feedback_add_feedback
* @pFeedbackNew: Pointer to an feedback that gets added to our list of feedbacks.
*
* Adds a new feedback to our list of feedbacks.
**/
void gok_feedback_add_feedback (GokFeedback* pFeedbackNew)
{
    GokFeedback* pFeedback;
   
    if (m_pFeedbackFirst == NULL)
    {
        m_pFeedbackFirst = pFeedbackNew;
        return;
    }
    
    pFeedback = m_pFeedbackFirst;
    while (pFeedback->pFeedbackNext != NULL)
    {
        pFeedback = pFeedback->pFeedbackNext;
    }
    
    pFeedback->pFeedbackNext = pFeedbackNew;
}

/**
* gok_feedback_delete_feedback
* @pFeedbackDelete: Pointer to the feedback that will be deleted.
*
* Deletes the given feedback and unhooks it from our list of feedbacks.
**/
void gok_feedback_delete_feedback (GokFeedback* pFeedbackDelete)
{
    GokFeedback* pFeedback;
    GokFeedback* pFeedbackPrevious;

    gok_log_enter ();
    
    pFeedbackPrevious = NULL;
    pFeedback = m_pFeedbackFirst;
    while (pFeedback != NULL)
    {
        if (pFeedbackDelete == pFeedback)
        {
            /* unhook the feedback from our list */
            if (pFeedbackDelete == m_pFeedbackFirst)
            {
                m_pFeedbackFirst = pFeedbackDelete->pFeedbackNext;
            }
            else
            {
                pFeedbackPrevious->pFeedbackNext = pFeedback->pFeedbackNext;
            }

            gok_feedback_free (pFeedbackDelete);

            break;
        }
        
        pFeedbackPrevious = pFeedback;
        pFeedback = pFeedback->pFeedbackNext;
    }

    gok_log_leave ();
}

/**
* gok_feedback_backup
* @pFeedback: Pointer to the feedback that will be backed up.
*
* Creates a copy of all the data stored on the feedback.
**/
void gok_feedback_backup (GokFeedback* pFeedback)
{
	/* backup the name */
	if (pFeedback->pName == NULL)
	{
		if (pFeedback->pNameBackup != NULL)
		{
			g_free (pFeedback->pNameBackup);
			pFeedback->pNameBackup = NULL;
		}
	}
	else /* name is not NULL */
	{
		if (pFeedback->pNameBackup != NULL)
		{
			/* is the backup name the same as the name? */
			if (strcmp (pFeedback->pName, pFeedback->pNameBackup) != 0)
			{
				/* no, free the backup name and copy the name to backup */
				g_free (pFeedback->pNameBackup);
				pFeedback->pNameBackup = g_malloc (strlen (pFeedback->pName) + 1);
				strcpy (pFeedback->pNameBackup, pFeedback->pName);
			}
		}
		else /* backup name is NULL */
		{
			pFeedback->pNameBackup = g_malloc (strlen (pFeedback->pName) + 1);
			strcpy (pFeedback->pNameBackup, pFeedback->pName);
		}
	}
	
	/* backup the display name */
	if (pFeedback->pDisplayName == NULL)
	{
		if (pFeedback->pDisplayNameBackup != NULL)
		{
			g_free (pFeedback->pDisplayNameBackup);
			pFeedback->pDisplayNameBackup = NULL;
		}
	}
	else /* display name is not NULL */
	{
		if (pFeedback->pDisplayNameBackup != NULL)
		{
			/* is the backup name the same as the name? */
			if (strcmp (pFeedback->pDisplayName, pFeedback->pDisplayNameBackup) != 0)
			{
				/* no, free the backup name and copy the name to backup */
				g_free (pFeedback->pDisplayNameBackup);
				pFeedback->pDisplayNameBackup = g_malloc (strlen (pFeedback->pDisplayName) + 1);
				strcpy (pFeedback->pDisplayNameBackup, pFeedback->pDisplayName);
			}
		}
		else /* backup name is NULL */
		{
			pFeedback->pDisplayNameBackup = g_malloc (strlen (pFeedback->pDisplayName) + 1);
			strcpy (pFeedback->pDisplayNameBackup, pFeedback->pDisplayName);
		}
	}

	/* backup the sound file name */
	if (pFeedback->pNameSound == NULL)
	{
		if (pFeedback->pNameSoundBackup != NULL)
		{
			g_free (pFeedback->pNameSoundBackup);
			pFeedback->pNameSoundBackup = NULL;
		}
	}
	else /* sound file name is not NULL */
	{
		if (pFeedback->pNameSoundBackup != NULL)
		{
			/* is the backup name the same as the name? */
			if (strcmp (pFeedback->pNameSound, pFeedback->pNameSoundBackup) != 0)
			{
				/* no, free the backup name and copy the name to backup */
				g_free (pFeedback->pNameSoundBackup);
				pFeedback->pNameSoundBackup = g_malloc (strlen (pFeedback->pNameSound) + 1);
				strcpy (pFeedback->pNameSoundBackup, pFeedback->pNameSound);
			}
		}
		else /* backup name is NULL */
		{
			pFeedback->pNameSoundBackup = g_malloc (strlen (pFeedback->pNameSound) + 1);
			strcpy (pFeedback->pNameSoundBackup, pFeedback->pNameSound);
		}
	}

	pFeedback->bFlashOnBackup = pFeedback->bFlashOn;
	pFeedback->NumberFlashesBackup = pFeedback->NumberFlashes;
	pFeedback->bSoundOnBackup = pFeedback->bSoundOn;
	pFeedback->bSpeechOnBackup = pFeedback->bSpeechOn;
	
	/* make this feedback not new */
	pFeedback->bNewFeedback = FALSE;
}

/**
* gok_feedback_revert
* @pFeedback: Pointer to the feedback that will be reverted.
*
* Moves the backup data to the current data on the feedback.
*
* returns: TRUE if any of the reverted settings are different from the
* current settings. Returns FALSE if the reverted settings are the same
* as the current settings.
**/
gboolean gok_feedback_revert (GokFeedback* pFeedback)
{
	gboolean bSettingsChanged = FALSE;
	
	/* revert the name */
	/* don't revert to a NULL name */
	if (pFeedback->pNameBackup != NULL)
	{
		if (pFeedback->pName != NULL)
		{
			/* is the backup name the same as the name? */
			if (strcmp (pFeedback->pName, pFeedback->pNameBackup) != 0)
			{
				/* no, free the name and copy the backup name to the name */
				g_free (pFeedback->pName);
				pFeedback->pName = g_malloc (strlen (pFeedback->pNameBackup) + 1);
				strcpy (pFeedback->pName, pFeedback->pNameBackup);
				
				bSettingsChanged = TRUE;
			}
		}
		else /* name is NULL */
		{
			pFeedback->pName = g_malloc (strlen (pFeedback->pNameBackup) + 1);
			strcpy (pFeedback->pName, pFeedback->pNameBackup);
			
			bSettingsChanged = TRUE;
		}
	}

	/* revert the display name */
	/* don't revert to a NULL name */
	if (pFeedback->pDisplayNameBackup != NULL)
	{
		if (pFeedback->pDisplayName != NULL)
		{
			/* is the backup name the same as the name? */
			if (strcmp (pFeedback->pDisplayNameBackup, pFeedback->pDisplayName) != 0)
			{
				/* no, free the backup name and copy the name to backup */
				g_free (pFeedback->pDisplayName);
				pFeedback->pDisplayName = g_malloc (strlen (pFeedback->pDisplayNameBackup) + 1);
				strcpy (pFeedback->pDisplayName, pFeedback->pDisplayNameBackup);
				
				bSettingsChanged = TRUE;
			}
		}
		else /* backup name is NULL */
		{
			pFeedback->pDisplayName = g_malloc (strlen (pFeedback->pDisplayNameBackup) + 1);
			strcpy (pFeedback->pDisplayName, pFeedback->pDisplayNameBackup);
			
			bSettingsChanged = TRUE;
		}
	}

	/* backup the sound file name */
	/* don't revert to a NULL name */
	if (pFeedback->pNameSoundBackup != NULL)
	{
		if (pFeedback->pNameSound != NULL)
		{
			/* is the backup name the same as the name? */
			if (strcmp (pFeedback->pNameSound, pFeedback->pNameSoundBackup) != 0)
			{
				/* no, free the backup name and copy the name to backup */
				g_free (pFeedback->pNameSound);
				pFeedback->pNameSound = g_malloc (strlen (pFeedback->pNameSoundBackup) + 1);
				strcpy (pFeedback->pNameSound, pFeedback->pNameSoundBackup);
				
				bSettingsChanged = TRUE;
			}
		}
		else /* sound file name is NULL */
		{
			pFeedback->pNameSound = g_malloc (strlen (pFeedback->pNameSoundBackup) + 1);
			strcpy (pFeedback->pNameSound, pFeedback->pNameSoundBackup);
			
			bSettingsChanged = TRUE;
		}
	}

	if (pFeedback->bFlashOn != pFeedback->bFlashOnBackup)
	{
		pFeedback->bFlashOn = pFeedback->bFlashOnBackup;
		bSettingsChanged = TRUE;
	}
	if (pFeedback->bSoundOn != pFeedback->bSoundOnBackup)
	{
		pFeedback->bSoundOn = pFeedback->bSoundOnBackup;
		bSettingsChanged = TRUE;
	}
	if (pFeedback->bSpeechOn != pFeedback->bSpeechOnBackup)
	{
		pFeedback->bSpeechOn = pFeedback->bSpeechOnBackup;
		bSettingsChanged = TRUE;
	}
	if (pFeedback->NumberFlashes != pFeedback->NumberFlashesBackup)
	{
		pFeedback->NumberFlashes = pFeedback->NumberFlashesBackup;
		bSettingsChanged = TRUE;
	}

	return bSettingsChanged;
}

/**
* gok_feedback_get_key:
* @feedback_name:         The feedback to build the key for.
* @attribute:             The attribute to build the key for.
*
* Builds a GConf key name from feedback_name and attribute.
* The returned key name is of the form:
*
* GOK_GCONF_FEEDBACKS/feedback_name/attribute
*
* NOTE: The returned gchar* needs to be freed when finished with.
**/
gchar* gok_feedback_get_key (const gchar* feedback_name,
                             const gchar* attribute)
{
    return g_strconcat (GOK_GCONF_FEEDBACKS, "/",
                        feedback_name, "/",
                        attribute, NULL);
}

/**
* gok_feedback_perform_effect
* @pNameFeedback: Name of the feedback that should be performed.
*
* Performs the effects in the given feedback.
*
* returns: Always 0.
**/
gint gok_feedback_perform_effect (gchar* pNameFeedback)
{
	GokFeedback* pFeedback;

	if ((pNameFeedback == NULL) ||
		(strlen (pNameFeedback) == 0))
	{
		return 0;
	}
	
	pFeedback = m_pFeedbackFirst;
	while (pFeedback != NULL)
	{
		if (strcmp (pNameFeedback, pFeedback->pName) == 0)
		{
			if (pFeedback->bFlashOn == TRUE)
			{
				gok_feedback_timer_start_key_flash (pFeedback->NumberFlashes);
			}
			
			if (pFeedback->bSoundOn == TRUE)
			{
				gok_sound_play (pFeedback->pNameSound);				
			}

			/* FIXME: evil use of static string here */
			if (pFeedback->bSpeechOn == TRUE && m_keyLabel 
#ifndef TEST_FEEDBACK
			    && !gok_main_safe_mode ()
#endif
			    )
			{
				gok_speech_speak (m_keyLabel);
			}
			
			break;
		}
		pFeedback = pFeedback->pFeedbackNext;
	}
		
	return 0;
}

/**
* gok_feedback_get_name
* @pDisplayName: Display name of the feedback.
*
* returns: The static (gconf) name of the feedback.
**/
gchar* gok_feedback_get_name (gchar* pDisplayName)
{
	GokFeedback* pFeedback;
	
	if ((pDisplayName == NULL) ||
		(strlen (pDisplayName) == 0))
	{
		return "";
	}

	pFeedback = m_pFeedbackFirst;
	while (pFeedback != NULL)
	{
		if (strcmp (pDisplayName, pFeedback->pDisplayName) == 0)
		{
			return pFeedback->pName;
		}
		pFeedback = pFeedback->pFeedbackNext;
	}
	
	gok_log_x ("Can't find name for display name: %s", pDisplayName);
	
	return "";
}

/**
* gok_feedback_get_displayname
* @pName: Name of the feedback.
*
* returns: The display name of the feedback.
**/
gchar* gok_feedback_get_displayname (gchar* pName)
{
	GokFeedback* pFeedback;
	
	if ((pName == NULL) ||
		(strlen (pName) == 0))
	{
		return "";
	}

	pFeedback = m_pFeedbackFirst;
	while (pFeedback != NULL)
	{
		if (strcmp (pName, pFeedback->pName) == 0)
		{
			return pFeedback->pDisplayName;
		}
		pFeedback = pFeedback->pFeedbackNext;
	}
	
	gok_log_x ("Can't find display name for name: %s", pName);
	
	return "";
}


/**
* gok_feedback_timer_start_key_flash
* @NumberFlashes: Number of times the key should be flashed.
*
* Starts the key flashing timer.
**/
void gok_feedback_timer_start_key_flash (gint NumberFlashes)
{
	/* stop the current key flashing */
	gok_feedback_timer_stop_key_flash();

	/* get the key that will be flashing */
	m_pKeyflashKey = gok_feedback_get_selected_key();
	if (m_pKeyflashKey == NULL)
	{
		return;
	}
	
	m_bFlashState = FALSE;
	m_KeyflashTimerCount = NumberFlashes * 2;
	
	/* start the timer */
	m_KeyflashTimerId = g_timeout_add_full (G_PRIORITY_HIGH_IDLE, 100, gok_feedback_timer_on_key_flash, NULL, NULL);
}

/**
* gok_feedback_timer_stop_key_flash
*
* Stop the key flashing timer.
**/
void gok_feedback_timer_stop_key_flash ()
{
	/* stop the timer */
	if (m_KeyflashTimerCount != 0)
	{
		g_source_remove (m_KeyflashTimerId);
		m_KeyflashTimerCount = 0;
	}

	if (m_pKeyflashKey != NULL)
	{
		/* return the key's highlighting to its original state */
		if (m_pKeyflashKey->StateWhenNotFlashed == GTK_STATE_NORMAL)
		{
			gok_feedback_unhighlight (m_pKeyflashKey, TRUE);
		}
		else
		{
			gok_feedback_highlight (m_pKeyflashKey, TRUE);
		}
	}
	
	m_pKeyflashKey = NULL;
}

/**
* gok_feedback_timer_on_key_flash
* @data: Data associated with the timer.
*
* The key flash timer has counted down so flash the key.
**/
gboolean gok_feedback_timer_on_key_flash (gpointer data)
{
	if (m_KeyflashTimerCount != 0)
	{
		m_KeyflashTimerCount--;
	}
	
	if (m_KeyflashTimerCount == 0)
	{
		gok_feedback_timer_stop_key_flash();
		return FALSE;
	}

	if (m_pKeyflashKey == NULL)
	{
		gok_feedback_timer_stop_key_flash();
		return FALSE;
	}
	
	if (m_bFlashState == TRUE)
	{
		gok_feedback_highlight (m_pKeyflashKey, TRUE);
		m_bFlashState = FALSE;
	}
	else
	{
		gok_feedback_unhighlight (m_pKeyflashKey, TRUE);
		m_bFlashState = TRUE;
	}

	return TRUE;
}

/**
* gok_feedback_get_key_flashing
*
* returns: A pointer to the key that is flashing, NULL if no key flashing.
**/
GokKey* gok_feedback_get_key_flashing ()
{
	return m_pKeyflashKey;
}

/**
* gok_feedback_highlight
* @pGokKey: Pointer to the key that will be highlighted.
* @bFlash: TRUE if the key is flashing.
*
* Highlightes the given key.
* Updates m_pKeyHighlighted with the key given.
**/
void gok_feedback_highlight (GokKey* pGokKey, gboolean bFlash)
{
	g_assert (pGokKey != NULL);

	pGokKey->State = GTK_STATE_PRELIGHT;

	if (GTK_IS_WIDGET (pGokKey->pButton) == TRUE)
	{
		gtk_widget_set_state (GTK_WIDGET (pGokKey->pButton), GTK_STATE_PRELIGHT);
	}

	/* if the key is flashing then don't make it the highlighted key */
	if (bFlash == FALSE)
	{
		m_pKeyHighlighted = pGokKey;

		pGokKey->StateWhenNotFlashed = GTK_STATE_PRELIGHT;
	}	
}

/**
* gok_feedback_unhighlight
* @pGokKey: Pointer to the key that will be NOT highlighted (returned to normal).
* @bFlash: TRUE if the key is flashing.
*
* Unhighlightes the given key.
* Updates m_pKeyHighlighted with the key given.
**/
void gok_feedback_unhighlight (GokKey* pGokKey, gboolean bFlash)
{
	if (pGokKey == NULL)
	{
		return;
	}
	
	pGokKey->State = GTK_STATE_NORMAL;
		
	if (GTK_IS_WIDGET (pGokKey->pButton) == TRUE)
	{
		gtk_widget_set_state (GTK_WIDGET (pGokKey->pButton), GTK_STATE_NORMAL);
	}

	/* if the key is flashing then don't make it the highlighted key */
	if (bFlash == FALSE)
	{
		m_pKeyHighlighted = NULL;

		/* the key should normally be in this state */
		pGokKey->StateWhenNotFlashed = GTK_STATE_NORMAL;
	}
}

/**
* gok_feedback_get_selected_key
*
* Accessor that returns a pointer to the selected key .
*
* returns: A pointer to the currently selected key.
**/
GokKey* gok_feedback_get_selected_key()
{
	return m_pKeySelected;
}

/**
* gok_feedback_get_highlighted_key
*
* Accessor that returns a pointer to the highlighted key .
*
* returns: A pointer to the currently highlighed key.
**/
GokKey* gok_feedback_get_highlighted_key()
{
	return m_pKeyHighlighted;
}

/**
* gok_feedback_set_selected_key
*
* Sets the currently selected key.
*
**/
void gok_feedback_set_selected_key (GokKey* key)
{
	/* 
	 * odd logic here since selected key is set to NULL before speech output 
	 * so we have to keep the old label around unless a new selected key has been set.
	 */
	if (key)
	{
		g_free (m_keyLabel);
		if (key->pLabel && key->pLabel->Text)
			m_keyLabel = g_strdup (key->pLabel->Text);
		else
			m_keyLabel = NULL;
	}
	m_pKeySelected = key;
}

/**
* gok_feedback_set_highlighted_key
*
* Accessor that returns a pointer to the highlighted key .
*
* returns: A pointer to the currently highlighed key.
**/
void gok_feedback_set_highlighted_key (GokKey* pKey)
{
	m_pKeyHighlighted = pKey;
}

/* Temporary fix which make sure we aren't holding a pointer to a dead key */
void
gok_feedback_drop_refs (GokKey *pKey)
{
  if (m_pKeyflashKey == pKey) m_pKeyflashKey = NULL;
  if (m_pKeySelected == pKey) m_pKeySelected = NULL;
  if (m_pKeyHighlighted == pKey) m_pKeyHighlighted = NULL;
}


