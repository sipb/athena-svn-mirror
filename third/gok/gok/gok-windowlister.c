/* gok-windowlister.c
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

/* TODO
g_list_free !
use wnck_window_is_active to figure out which window in list is active (since not all apps are registered with at-spi)
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

/* we will need to keep an eye on libwnck */
#define WNCK_I_KNOW_THIS_IS_UNSTABLE


#include <libwnck/libwnck.h>
#include "gok-log.h"
#include "main.h"
#include "gok-windowlister.h"

/* private variables */
static GList* m_window_list = NULL;

/* private functions */
void gok_windowlister_refreshList (void);
gboolean gok_windowlister_isValid ( void* pWindow );

/**
* gok_windowlist_show
*
* returns: TRUE if the keyboard was displayed, FALSE if not.
*/
gboolean gok_windowlister_show()
{
	GokKeyboard* pKeyboard;
	GokKeyboard* pKeyboardNew;
	GokKey* pKey;
	GokKey* pKeyPrevious;
	int column;
	const gchar *window_name = NULL;
    GList *window_list_entry;

/*    GType window_type; */
  
	gok_log_enter();	

	/* first check if there is already such a keyboard created */
	pKeyboard = (GokKeyboard*)gok_main_get_first_keyboard();
	while (pKeyboard != NULL)
	{
		if (pKeyboard->Type == KEYBOARD_TYPE_WINDOWS)
		{
			break;
		}
		pKeyboard = pKeyboard->pKeyboardNext;
	}
	
	if (pKeyboard == NULL)
	{
		gok_log ("creating window list keyboard..");
		pKeyboardNew = gok_keyboard_new();
		if (pKeyboardNew == NULL)
		{
			gok_log_leave();
			return FALSE;
		}
		
		/* add the new keyboard to the list of keyboards (at the end)*/
		pKeyboard = (GokKeyboard*)gok_main_get_first_keyboard();
		g_assert (pKeyboard != NULL);
		while (pKeyboard->pKeyboardNext != NULL)
		{
			pKeyboard = pKeyboard->pKeyboardNext;
		}
		pKeyboard->pKeyboardNext = pKeyboardNew;
		pKeyboardNew->pKeyboardPrevious = pKeyboard;
		
		pKeyboardNew->Type = KEYBOARD_TYPE_WINDOWS;
		gok_keyboard_set_name (pKeyboardNew, _("Window List"));
		
		pKeyboard = pKeyboardNew;
	}
	else
	{
	
		gok_log ("removing old keys.");
		
		/* remove any old keys on the old keyboard */
		pKey = pKeyboard->pKeyFirst;
		while (pKey != NULL)
		{
		        GokKey *tmp = pKey->pKeyNext;
			gok_key_delete (pKey, pKeyboard, TRUE);
			pKey = tmp;
		}
	}
	
	/* set this flag so the keyboard will be laid out when it's displayed */
	pKeyboard->bLaidOut = FALSE;

	gok_windowlister_refreshList(); /* avoids potential instability bugs with libwnck */ 

	/* first, add a 'back' key */
	pKey = gok_key_new (NULL, NULL, pKeyboard);
	pKey->Style = KEYSTYLE_BRANCHBACK; 
	pKey->Type = KEYTYPE_BRANCHBACK;
	pKey->Top = 0;
	pKey->Bottom = 1;
	pKey->Left = 0;
	pKey->Right = 1;
	gok_key_add_label (pKey, _("back"), 0, 0, NULL);
	pKeyPrevious = pKey;

	/* build window keys */
    window_list_entry = m_window_list;
    while (window_list_entry != NULL)
    {
    	/* check to see if this window show be represented at all */
		/*window_type = wnck_window_get_window_type (window_list_entry->data);
    	if (1)((window_type == WNCK_WINDOW_NORMAL) ||
    	    (window_type == WNCK_WINDOW_DIALOG) ||
    	    (window_type == WNCK_WINDOW_MODAL_DIALOG))*/
    	{
			pKey = gok_key_new (pKeyPrevious, NULL, pKeyboard);
			pKeyPrevious = pKey;
			
			pKey->Style = KEYSTYLE_GENERALDYNAMIC;  /* TODO */
			pKey->Type = KEYTYPE_WINDOW;
			pKey->Top = 0;
			pKey->Bottom = 1;
			pKey->Left = column;
			pKey->Right = column + 1;
			window_name = wnck_window_get_name (window_list_entry->data);
			gok_key_add_label (pKey, window_name ? g_strdup (window_name) : g_strdup (""), 0, 0, NULL);
			
			pKey->pGeneral = window_list_entry->data;  
	
			column++;
		}
        window_list_entry = window_list_entry->next;
    }

	/* display and scan the menus keyboard */
	gok_main_display_scan (pKeyboard, NULL, KEYBOARD_TYPE_UNSPECIFIED,
			       KEYBOARD_LAYOUT_UNSPECIFIED, KEYBOARD_SHAPE_UNSPECIFIED);

	gok_log_leave();
	return TRUE;
	
}

/**
* gok_windowlister_onKey
* @pKey: Pointer to the key that was selected.
*
* This function will activate a window with the same title as the key label.
*/
void gok_windowlister_onKey (GokKey* pKey)
{
	WnckWorkspace *workspace;
	gboolean bSuccess;
	bSuccess = FALSE;
#ifdef WLISTER_USE_STRING_COMPARISON
	char* keyLabel;
    GList *window_list_entry;
	keyLabel = gok_key_get_label(pKey);
	
	gok_windowlister_refreshList();

    window_list_entry = m_window_list;
	while (window_list_entry != NULL)
	{
		gok_log("comparing key [%s] and window [%s].",keyLabel, wnck_window_get_name(window_list_entry->data));
		if (strcmp(keyLabel, wnck_window_get_name (window_list_entry->data)) == 0)
		{
			/* bingo */
		    workspace = wnck_window_get_workspace (window_list_entry->data);
		    if (workspace != NULL) 
		    {
		        wnck_workspace_activate (workspace);
		    }
			bSuccess = TRUE;
			wnck_window_activate (window_list_entry->data);
			break;
		}
        window_list_entry = window_list_entry->next;
    }
#else
	g_assert (pKey->pGeneral != NULL);
	
	gok_windowlister_refreshList();

	/* check to see if window list entry is still valid */
	if (gok_windowlister_isValid(pKey->pGeneral) == TRUE)
	{
	    workspace = wnck_window_get_workspace (pKey->pGeneral);
	    if (workspace != NULL) 
	    {
	        wnck_workspace_activate (workspace);
	    }
		bSuccess = TRUE;
		wnck_window_activate (pKey->pGeneral);
	}
	
#endif
	if ( bSuccess == FALSE )
	{
		/* TODO: error feedback sound? */
		gok_windowlister_show();  /* redraw (and refresh) the keyboard */
	}
			
}


void gok_windowlister_refreshList ()
{
    WnckScreen *screen;
    
    /* do not free the list here! this seems to cause segv -- need to look at library code 
	if (m_window_list != NULL)
	{
		g_list_free(m_window_list);
	}
	*/
	
    screen = wnck_screen_get_default ();
    wnck_screen_force_update (screen);
	
    m_window_list = wnck_screen_get_windows (screen);
	
}

gboolean gok_windowlister_isValid ( void* pWindow )
{
	GList* window_list_entry;
    window_list_entry = m_window_list;
	while (window_list_entry != NULL)
	{
		if (window_list_entry->data == pWindow)
		{
			return TRUE;
		}
        window_list_entry = window_list_entry->next;
    }
    return FALSE;
}
