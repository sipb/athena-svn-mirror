/* gok-scanner.c
*
* Copyright 2002 Sun Microsystems, Inc.
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

#include <dirent.h>
#include <locale.h>
#include <gconf/gconf-client.h>
#include "gok-scanner.h"
#include "gok-data.h"
#include "gok-log.h"
#include "gok-action.h"
#include "gok-feedback.h"
#include "gok-keyboard.h"
#include "gok-gconf-keys.h" /* used only for uses_corepointer hack below */
#include "gok-gconf.h" 
#include "main.h"
#include "gok-repeat.h"

#define REPEAT_OFF_NAME "repeat"

/* pointer to the current access method */
static GokAccessMethod* m_pAccessMethodCurrent;

/* pointer to the first access method */
static GokAccessMethod* m_pAccessMethodFirst;

/* pointer to the current handler state */
static GokScannerState* m_pStateCurrent;

/* pointer to the key that is under the mouse pointer */
static GokKey* m_pKeyEntered;

/* pointer to the key containing the current input device motion coordinate */
static GokKey* m_pContainingKey;

/* location of the mouse pointer in our window */
static gint m_MouseX;
static gint m_MouseY;

/* integer offsets to be applied to the current motion valuator */
static gint m_OffsetX = 0;
static gint m_OffsetY = 0;

/* dwell rate */
static gint m_dwellrate;

/* flags needed to ignore double-click "press" events */
static gboolean m_bMouseLeftRelease;
static gboolean m_bMouseRightRelease;
static gboolean m_bMouseMiddleRelease;
static gboolean m_bMouse4Release;
static gboolean m_bMouse5Release;

/* timer flags */
static gboolean m_bTimer1Started;
static guint m_Timer1SourceId;
static gboolean m_bTimer2Started;
static guint m_Timer2SourceId;
static gboolean m_bTimer3Started;
static guint m_Timer3SourceId;
static gboolean m_bTimer4Started;
static guint m_Timer4SourceId;
static gboolean m_bTimer5Started;
static guint m_Timer5SourceId;
static gint m_bDwellTimerStarted;
static guint m_DwellTimerId;

/* pointers to the handlers */
/* Each of these pointers must be nulled in gok_scanner_set_handlers_null */
static GokScannerEffect* m_pEffectsLeftButtonDown;
static GokScannerEffect* m_pEffectsLeftButtonUp;
static GokScannerEffect* m_pEffectsRightButtonDown;
static GokScannerEffect* m_pEffectsRightButtonUp;
static GokScannerEffect* m_pEffectsMiddleButtonDown;
static GokScannerEffect* m_pEffectsMiddleButtonUp;
static GokScannerEffect* m_pEffectsMouseButton4Down;
static GokScannerEffect* m_pEffectsMouseButton4Up;
static GokScannerEffect* m_pEffectsMouseButton5Down;
static GokScannerEffect* m_pEffectsMouseButton5Up;
static GokScannerEffect* m_pEffectsMouseMovement;
static GokScannerEffect* m_pEffectsOnTimer1;
static GokScannerEffect* m_pEffectsOnTimer2;
static GokScannerEffect* m_pEffectsOnTimer3;
static GokScannerEffect* m_pEffectsOnTimer4;
static GokScannerEffect* m_pEffectsOnTimer5;
static GokScannerEffect* m_pEffectsOnKeyEnter;
static GokScannerEffect* m_pEffectsOnKeyLeave;
static GokScannerEffect* m_pEffectsOnSwitch1Down;
static GokScannerEffect* m_pEffectsOnSwitch1Up;
static GokScannerEffect* m_pEffectsOnSwitch2Down;
static GokScannerEffect* m_pEffectsOnSwitch2Up;
static GokScannerEffect* m_pEffectsOnSwitch3Down;
static GokScannerEffect* m_pEffectsOnSwitch3Up;
static GokScannerEffect* m_pEffectsOnSwitch4Down;
static GokScannerEffect* m_pEffectsOnSwitch4Up;
static GokScannerEffect* m_pEffectsOnSwitch5Down;
static GokScannerEffect* m_pEffectsOnSwitch5Up;
static GokScannerEffect* m_pEffectsOnDwell;

/* number of possible handlers */
#define MAX_HANDLERS 30

/* number of possible calls */
#define MAX_CALL_NAMES 62

/* names of all the possible effect calls */
/* This must be in the same order as the enum "CallIds" */
static gchar ArrayCallNames[MAX_CALL_NAMES][24] ={
"ChunkerReset", "ChunkerChunkNone", "ChunkerChunkKeys", 
"ChunkerChunkRows", "ChunkerChunkColumns", 
"ChunkerNextChunk", "ChunkerPreviousChunk", 
"ChunkerNextKey", "ChunkerPreviousKey", 
"ChunkerKeyUp", 
"ChunkerKeyDown", "ChunkerKeyLeft",
"ChunkerKeyRight", "ChunkerKeyHighlight", "ChunkerKeyUnHighlight",
"ChunkerWrapFirstChunk", "ChunkerWrapLastChunk",
"ChunkerWrapFirstKey", "ChunkerWrapLastKey",
"ChunkerWrapToBottom", "ChunkerWrapToTop",
"ChunkerWrapToLeft", "ChunkerWrapToRight",
"ChunkerMoveLeftRight", "ChunkerMoveTopBottom",
"ChunkerIfNextChunk", "ChunkerIfPreviousChunk",
"ChunkerIfNextKey", "ChunkerIfPreviousKey",
"ChunkerIfTop", "ChunkerIfBottom",
"ChunkerIfLeft", "ChunkerIfRight",
"ChunkerIfKeySelected", "HighlightCenterKey",
"HighlightFirstChunk", "HighlightFirstKey", "SelectChunk",
"ChunkerChunkHighlight", "UnhighlightAll", 
"RepeatOn", /* "RepeatOff",  use StateReset */
"Timer1Set", "Timer1Stop",
"Timer2Set", "Timer2Stop",
"Timer3Set", "Timer3Stop",
"Timer4Set", "Timer4Stop",
"Timer5Set", "Timer5Stop",
"CounterSet", "CounterIncrement",
"CounterDecrement", "CounterGet",
"StateReset", "StateNext", "StateJump",
"OutputSelectedKey", "SetSelectedKey",
"Feedback",
"GetRate"
};

static xmlChar *gok_lang = NULL;

/**
 * gok_scanner_get_lang:
 * returns: a string specifying the target language, conforming to IETF RFC 1766.
 **/
xmlChar *
gok_scanner_get_lang (void)
{
	if (!gok_lang) {
		gchar *lang;
		gchar **strings = NULL;
#if HAVE_LC_MESSAGES
		lang = setlocale (LC_MESSAGES, NULL);
#else
		lang = NULL;
#endif
		/* strip suffixes, etc. */
		if (lang) 
			strings = g_strsplit (lang, ".", 2);
		if (strings && strings[1]) 
			g_free (strings[1]);
		if (strings && strings[0]) {
			lang = strings[0];
			strings = g_strsplit (lang, "@", 2);
			if (strings[1]) 
				g_free (strings[1]);
			gok_lang = (unsigned char *)strings[0];
		}
		else
			gok_lang = (unsigned char *)"";
	}
	return gok_lang;
}

/**
 * gok_scanner_find_node:
 *
 * @pNode: Pointer to the XML node that may contain the node you're looking for.
 * @NameNode: Name of the node you're looking for.
 * @target_lang: String indicating the target language, or NULL if we don't care.
 * @node_lang: Pointer to a string which, if non-null, is filled with the xml:lang
 * tag of the node returned.
 *
 * returns: A pointer to the first node that has the given name and which provides
 *          the "best available match" to a target locale, NULL 
 *          if no node with the specified name can be found.
 *
 * Recursive method which find the first node matching the given name and providing the "best"
 * match to the specified locale.  Locale matching prefers a perfect match, otherwise matches the
 * locale ignoring suffixes, and falls back to "C".  If @target_lang lacks a suffix, 
 * returns the first node matching the target language, ignoring variants/suffixes.
 **/
xmlNode*
gok_scanner_find_node (xmlNode *pNode, const xmlChar* name, const xmlChar* target_lang, 
		       xmlChar **node_lang)
{
	xmlNode *retval = NULL;
	int      target_lang_len = strlen ((char *)target_lang);
	gboolean lang_is_matched = FALSE;	
	gboolean is_lang_match;
	gboolean is_perfect_match;

	gok_log_enter();
	is_lang_match = FALSE;
	is_perfect_match = FALSE;
	/* loop through the document looking for the name string */
	while (pNode != NULL)
	{
		/* does this node's name match ? */
		if (xmlStrcmp (pNode->name, name) == 0)
		{
			xmlChar *lang = NULL;
			/* If xml:lang matches our locale, return, else keep looking... */
			lang = xmlNodeGetLang (pNode);
			if (lang) {
				is_lang_match = !xmlStrncmp (lang, target_lang, 2);
			}
			if (is_lang_match || 
			    ((!lang || !xmlStrcmp (lang, (const xmlChar *) "C")) && (!lang_is_matched))) {
				retval = pNode;
				if (!xmlStrncmp (lang, target_lang, target_lang_len)) { /* perfect match */
					if (lang && node_lang) *node_lang = lang;
					gok_log ("found node");
					gok_log_leave();
					return retval;
				}
				else if (is_lang_match) {
					if (node_lang) {
						if (*node_lang) g_free (*node_lang);
						*node_lang = lang;
					}
					lang_is_matched = TRUE;
				}
				else {
					g_free (lang);
				}
			}
		}
		pNode = pNode->next;
	}

	gok_log_leave();
	return retval;
}

/*
 * gok_scanner_get_slop:
 * Return the number of pixels beyond the
 * current pointer clip window which the pointer is allowed.
 */
static gint
gok_scanner_get_slop (void)
{
	return 5; /* FIXME: should be gconfable, probably */
}

/* 
 * Confine a point to a window, and set 
 * last_x and last_y offsets accordingly. 
 */
static void
gok_pointer_clip_to_window (gint *xp, gint *yp, GdkWindow *window, gint slop)
{
	gint w, h, x, y;
	gdk_drawable_get_size (window, &w, &h);	
	gdk_window_get_origin (window, &x, &y);
	
	if (*xp < x - slop) {
		m_OffsetX += (*xp - x + slop);
		*xp = x - slop;
	}
	else if (*xp >= x + w + slop) {
		m_OffsetX += (*xp - (x + w + slop));
		*xp = x + w + slop;
	}
	if (*yp < y - slop) {
		m_OffsetY += (*yp - y + slop);
		*yp = y - slop;
	}
	else if (*yp >= y + h + slop) {
		m_OffsetY += (*yp - (y + h + slop));
		*yp = y + h + slop;
	}
}


/**
* gok_scanner_initialize:
* @directory: The name of the directory to read the access method files from.
* @accessmethod: If non-NULL, overrides access method name in gconf configuration data.
* @selectaction: If non-NULL, overrides action associated with 'select' for 
*                the current access method in gconf configuration data.
* @scanaction: If non-NULL, overrides access method associated with 'select' for 
*                the current access method in gconf configuration data.
*
* Reads all the access methods from the given directory
* and gets them ready to go.
*
* returns: TRUE if the access methods were initialized, FALSE if not. Don't use the 
* access methods if this fails.
**/
gboolean gok_scanner_initialize (const gchar *directory, const gchar *accessmethod, 
				 const gchar *selectaction, const gchar *scanaction)
{
	DIR* pDirectoryAccessMethods;
	struct dirent* pEntry;
	gchar* complete_path;

	/* NULL all the handler effects */
	gok_scanner_set_handlers_null();

	/* initialize member data */
	m_bTimer1Started = FALSE;
	m_Timer1SourceId = 0;
	m_bTimer2Started = FALSE;
	m_Timer1SourceId = 0;
	m_bTimer3Started = FALSE;
	m_Timer3SourceId = 0;
	m_bTimer4Started = FALSE;
	m_Timer4SourceId = 0;
	m_bTimer5Started = FALSE;
	m_Timer5SourceId = 0;
	m_bDwellTimerStarted = FALSE;
	m_DwellTimerId = 0;
	
	m_pKeyEntered = NULL;
	m_dwellrate = 0;

	m_bMouseLeftRelease = TRUE;
	m_bMouseRightRelease = TRUE;
	m_bMouseMiddleRelease = TRUE;
	
	/* read in all the access methods */
	/* first, open the access method directory */
	pDirectoryAccessMethods = opendir (directory);
	if (pDirectoryAccessMethods == NULL)
	{
		gok_log_x ("Error: Can't open access methods directory in gok_scanner_initialize!\n");
		return FALSE;
	}	

	/* look at each file in the directory */
	while ((pEntry = readdir (pDirectoryAccessMethods)) != NULL)
	{
		/* is this an access method file? */
		if (strstr (pEntry->d_name, ".xam") != NULL)
		{
			/* read the access method file */
                        complete_path = g_build_filename (directory,
							  pEntry->d_name,
							  NULL);

			gok_log ("complete_path = %s", complete_path);
			gok_scanner_read_access_method (complete_path);
			g_free (complete_path);
		}
	}

	/* get the rates for all the access methods */
	gok_scanner_update_rates ();

	if (accessmethod) /* we've specified the name in the args list */
	{
	    m_pAccessMethodCurrent = m_pAccessMethodFirst;
	    while (m_pAccessMethodCurrent)
	    { 
		if (!strcmp (m_pAccessMethodCurrent->Name, accessmethod)) 
		{
		    gok_data_set_name_accessmethod (accessmethod);
		    gok_scanner_reset_access_method ();
		    g_message ("using access method %s", accessmethod);
		    return TRUE;
		}
		m_pAccessMethodCurrent = m_pAccessMethodCurrent->pAccessMethodNext;
	    }
	}
	/* set the current access method from gconf value (note fall-through if match above failed) */
	return gok_scanner_change_method (gok_data_get_name_accessmethod());
}

/**
* gok_scanner_set_handlers_null
*
* Sets all the event handlers to NULL.
**/
void gok_scanner_set_handlers_null ()
{
	m_pEffectsLeftButtonDown = NULL;
	m_pEffectsLeftButtonUp = NULL;
	m_pEffectsRightButtonDown = NULL;
	m_pEffectsRightButtonUp = NULL;
	m_pEffectsMiddleButtonDown = NULL;
	m_pEffectsMiddleButtonUp = NULL;
	m_pEffectsMouseButton4Down = NULL;
	m_pEffectsMouseButton4Up = NULL;
	m_pEffectsMouseButton5Down = NULL;
	m_pEffectsMouseButton5Up = NULL;
	m_pEffectsMouseMovement = NULL;
	m_pEffectsOnTimer1 = NULL;
	m_pEffectsOnTimer2 = NULL;
	m_pEffectsOnTimer3 = NULL;
	m_pEffectsOnTimer4 = NULL;
	m_pEffectsOnTimer5 = NULL;
	m_pEffectsOnKeyEnter = NULL;
	m_pEffectsOnKeyLeave = NULL;
	m_pEffectsOnSwitch1Down = NULL;
	m_pEffectsOnSwitch1Up = NULL;
	m_pEffectsOnSwitch2Down = NULL;
	m_pEffectsOnSwitch2Up = NULL;
	m_pEffectsOnSwitch3Down = NULL;
	m_pEffectsOnSwitch3Up = NULL;
	m_pEffectsOnSwitch4Down = NULL;
	m_pEffectsOnSwitch4Up = NULL;
	m_pEffectsOnSwitch5Down = NULL;
	m_pEffectsOnSwitch5Up = NULL;
	m_pEffectsOnDwell = NULL;
}

/**
* gok_scanner_read_access_method
* @Filename: Name of the access method file.
*
* Read an access method file from disk and create a new access method.
* The new access method is added to the list of access methods.
*
* returns: TRUE if the acces method was created, FALSE if not.
**/
gboolean gok_scanner_read_access_method (gchar* Filename)
{
	GokAccessMethod* pAccessMethod;
	xmlDoc* pDoc;
	xmlNode* pNodeRoot;
	xmlNode* pNodeAccessmethod;
	xmlNode* pNodeInitialization;
	xmlNode* pNodeState;
	xmlNode* pNodeHandler;
	xmlNode* pNodeInit;
	xmlNs* pNamespace;
	xmlChar* pStringAttributeValue;
	xmlChar* pStringStateId;
	xmlChar* pStringHandlerName;
	xmlChar* pStringHandlerState;
	GokScannerState* pState;
	GokScannerState* pStateLast;
	GokScannerHandler* pHandler;
	GokScannerHandler* pHandlerLast;

	g_assert (Filename != NULL);

	/* read in the file and create a DOM */
	pDoc = xmlParseFile (Filename);
	if (pDoc == NULL)
	{
		gok_log_x ("Error: gok_scanner_read_access_method failed - xmlParseFile failed. Filename: '%s'\n", Filename);
		return FALSE;
	}

	/* check if the document is empty */
	pNodeRoot = xmlDocGetRootElement (pDoc);
    if (pNodeRoot == NULL)
	 {
		gok_log_x ("Error: gok_scanner_read_access_method - first node empty. Filelname: %s\n", Filename);
		xmlFreeDoc (pDoc);
		return FALSE;
	}

	/* check if the document has the correct namespace */
	pNamespace = xmlSearchNsByHref (pDoc, pNodeRoot, (const xmlChar *) "http://www.gnome.org/GOK");
	if (pNamespace == NULL)
	{
		gok_log_x ("Error: Can't create new access method '%s'- does not have GOK Namespace.\n", Filename);
		xmlFreeDoc (pDoc);
		return FALSE;
	}

	/* find the 'accessmethod' node */
	pNodeAccessmethod = gok_keyboard_find_node (pNodeRoot, "accessmethod");
	if (pNodeAccessmethod == NULL)
	{
		gok_log_x ("Error: gok_scanner_create_access_method failed: can't find 'accessmethod' node!");
		xmlFreeDoc (pDoc);
		return FALSE;	
	}

	/* get the name of the access method */
	pStringAttributeValue = xmlGetProp (pNodeAccessmethod, (const xmlChar *) "name");
	if (pStringAttributeValue != NULL)
	{
		/* create a new access method structure */
 	   pAccessMethod = gok_scanner_create_access_method ((gchar *)pStringAttributeValue);
		if (pAccessMethod == NULL)
		{
			xmlFreeDoc (pDoc);
			return FALSE;
		}
	}
	else
	{
		/* access method must have a name attribute*/
		gok_log_x ("Error: gok_scanner_create_access_method failed: can't find 'name' attribute.\n");
		xmlFreeDoc (pDoc);
		return FALSE;
	}

	/* get the display name of the access method */
	pStringAttributeValue = xmlGetProp (pNodeAccessmethod, (const xmlChar *) "displayname");
	if (pStringAttributeValue == NULL)
	{
		gok_log_x ("No display name for this access method (%s).", 
			pAccessMethod->Name);
		/* hack to fix intltool xml translation bug */
		pStringAttributeValue = xmlGetProp (pNodeAccessmethod, 
			(const xmlChar *) "_displayname");
		if (pStringAttributeValue == NULL)
		{
			gok_log_x ("using name: %s.", 
				pStringAttributeValue);
			/* use the name if no display name */
			pStringAttributeValue = (unsigned char *)pAccessMethod->Name;
		}
		else
		{
			gok_log_x ("using _display name: %s.", 
				pStringAttributeValue);
		}
	}
		
	strcpy (pAccessMethod->DisplayName, _((gchar *)pStringAttributeValue));

	/* read the UI for the access method */
	gok_scanner_read_description (pDoc, pNodeAccessmethod->xmlChildrenNode, pAccessMethod);
	gok_scanner_read_operation (pDoc, pNodeAccessmethod->xmlChildrenNode, pAccessMethod);
	gok_scanner_read_feedback (pDoc, pNodeAccessmethod->xmlChildrenNode, pAccessMethod);
	gok_scanner_read_options (pDoc, pNodeAccessmethod->xmlChildrenNode, pAccessMethod);

	/* read the rates for this effect */
	gok_scanner_read_rates (pNodeAccessmethod->xmlChildrenNode, pAccessMethod);

	/* read the initializing effects for this access method */
	pNodeInitialization = gok_keyboard_find_node (pNodeAccessmethod, "initialization");
	if (pNodeInitialization != NULL)
	{
		pAccessMethod->pInitializationEffects = gok_scanner_read_effects (pNodeInitialization, pAccessMethod);
	}

	pStateLast = NULL;

	/* get all the handler states */
	pNodeState = gok_keyboard_find_node (pNodeAccessmethod, "state");
	while (pNodeState != NULL)
	{
		/* is this a state? */
		if (xmlStrcmp (pNodeState->name, (const xmlChar*)"state") == 0)
		{
			/* yes, create the new handler state */
			pState = gok_scanner_construct_state();
			if (pState == NULL)
			{
				break;
			}

			/* add the state to the current access method */
			if (pStateLast == NULL)
			{
				pAccessMethod->pStateFirst = pState;
			}
			else
			{
				pStateLast->pStateNext = pState;
			}
			pStateLast = pState;

			/* get name of the state (name is optional) */
			pStringStateId = xmlGetProp (pNodeState, (const xmlChar *) "name");
			if (pStringStateId != NULL)
			{
				pState->NameState = (gchar*)g_malloc (strlen ((gchar *)pStringStateId) + 1);
				strcpy (pState->NameState, (gchar *)pStringStateId);
			}

			/* get the initialization for the state */
			pNodeInit = gok_keyboard_find_node (pNodeState, "stateinit");
			if (pNodeInit != NULL)
			{
				pState->pEffectInit = gok_scanner_read_effects (pNodeInit, pAccessMethod);
			}
			
			/* get all the handlers for the state */
			pHandlerLast = NULL;
			pNodeHandler = pNodeState->xmlChildrenNode;
			while (pNodeHandler != NULL)
			{
				/* is this a 'handler' node ? */
				if (xmlStrcmp (pNodeHandler->name, (const xmlChar *) "handler") != 0)
				{
					/* not a 'handler' node, so ignore it */
					pNodeHandler = pNodeHandler->next;
					continue;
				}

				pStringHandlerName = xmlGetProp (pNodeHandler, (const xmlChar *) "name");
				if (pStringHandlerName != NULL)
				{
					/* create the handler */
					pHandler = gok_scanner_construct_handler ((gchar *)pStringHandlerName);
					if (pHandler == NULL)
					{
						break;
					}

					/* get the handler state (may be undefined) */
					pStringHandlerState = xmlGetProp (pNodeHandler, (const xmlChar *) "state");
					if (pStringHandlerState != NULL)
					{
						if (xmlStrcmp (pStringHandlerState, (const xmlChar *) "press") == 0)
						{
							pHandler->EffectState = ACTION_STATE_PRESS;
						}
						else if (xmlStrcmp (pStringHandlerState, (const xmlChar *) "release") == 0)
						{
							pHandler->EffectState = ACTION_STATE_RELEASE;
						}
						else if (xmlStrcmp (pStringHandlerState, (const xmlChar *) "click") == 0)
						{
							pHandler->EffectState = ACTION_STATE_CLICK;
						}
						else if (xmlStrcmp (pStringHandlerState, (const xmlChar *) "doubleclick") == 0)
						{
							pHandler->EffectState = ACTION_STATE_DOUBLECLICK;
						}
						else if (xmlStrcmp (pStringHandlerState, (const xmlChar *) "enter") == 0)
						{
							pHandler->EffectState = ACTION_STATE_ENTER;
						}
						else if (xmlStrcmp (pStringHandlerState, (const xmlChar *) "leave") == 0)
						{
							pHandler->EffectState = ACTION_STATE_LEAVE;
						}
						else
						{
							gok_log_x ("Handler state '%s' invalid!\n", pStringHandlerState);
						}
					}
										
					/* add the handler to the state */
					if (pHandlerLast == NULL)
					{
						pState->pHandlerFirst = pHandler;
					}
					else
					{
						pHandlerLast->pHandlerNext = pHandler;
					}
					pHandlerLast = pHandler;

					/* add the effects to the handler */
					pHandler->pEffectFirst = gok_scanner_read_effects (pNodeHandler, pAccessMethod);
				}
				else
				{
					gok_log_x ("Warning: Handler has no name in gok_scanner_read_access_method!\n");
				}

				pNodeHandler = pNodeHandler->next;
			}
		}
		
		pNodeState = pNodeState->next;
	}	

	/* store the XML doc on the access method so we can get the UI stuff later */
	pAccessMethod->pXmlDoc = pDoc;

	return TRUE;
}

/**
* gok_scanner_construct_state
*
* Creates a new handler state.
*
* returns: A pointer to the new state, NULL if it was not created.
**/
GokScannerState* gok_scanner_construct_state()
{
	GokScannerState* pState;

	pState = (GokScannerState*) g_malloc(sizeof(GokScannerState));

	pState->NameState = NULL;
	pState->pStateNext = NULL;
	pState->pEffectInit = NULL;
	pState->pHandlerFirst = NULL;

	return pState;
}

/**
* gok_scanner_construct_rate
*
* Creates a new access method rate.
*
* returns: A pointer to the new rate, NULL if it was not created.
**/
GokAccessMethodRate* gok_scanner_construct_rate()
{
	GokAccessMethodRate* pRate;

	pRate = (GokAccessMethodRate*) g_malloc(sizeof(GokAccessMethodRate));

	pRate->Name = NULL;
	pRate->StringValue = NULL;
	pRate->Type = RATE_TYPE_UNDEFINED;
	pRate->ID = -1;
	pRate->Value = 0;
	pRate->pRateNext = NULL;

	return pRate;
}

/**
* gok_scanner_construct_handler
* @pHandlerName: Name of the handler.
*
* Creates a new access method handler.
*
* returns: A pointer to the new handler, NULL if it was not created.
**/
GokScannerHandler* gok_scanner_construct_handler (gchar* pHandlerName)
{
	GokScannerHandler* pHandler;

	g_assert (pHandlerName != NULL);
	g_assert (strlen (pHandlerName) != 0);
	
	pHandler = (GokScannerHandler*) g_malloc(sizeof(GokScannerHandler));
	pHandler->TypeHandler = -1;
	pHandler->bPredefined = FALSE;
	pHandler->EffectName = NULL;
	pHandler->EffectState = ACTION_STATE_UNDEFINED;
	pHandler->pEffectFirst = NULL;
	pHandler->pHandlerNext = NULL;

	if (strcmp (pHandlerName, "timer1") == 0)
	{
		pHandler->TypeHandler = ACTION_TYPE_TIMER1;
		pHandler->bPredefined = TRUE;
	}
	else if (strcmp (pHandlerName, "timer2") == 0)
	{
		pHandler->TypeHandler = ACTION_TYPE_TIMER2;
		pHandler->bPredefined = TRUE;
	}
	else if (strcmp (pHandlerName, "timer3") == 0)
	{
		pHandler->TypeHandler = ACTION_TYPE_TIMER3;
		pHandler->bPredefined = TRUE;
	}
	else if (strcmp (pHandlerName, "timer4") == 0)
	{
		pHandler->TypeHandler = ACTION_TYPE_TIMER4;
		pHandler->bPredefined = TRUE;
	}
	else if (strcmp (pHandlerName, "timer5") == 0)
	{
		pHandler->TypeHandler = ACTION_TYPE_TIMER5;
		pHandler->bPredefined = TRUE;
	}
	else
	{
		pHandler->EffectName = (gchar*)g_malloc (strlen (pHandlerName) + 1);
		strcpy (pHandler->EffectName, pHandlerName);
	}	
	
	return pHandler;
}

/**
 * gok_scanner_handler_uses_mouse_button:
 * @h: The GokScannerHandler to test.
 *
 * Returns: TRUE if the GokScannerHandler h has a state specified and
 * that state uses a mouse button, FALSE if no state is specified or a
 * state is specified and it does not use a mouse button.
 */
gboolean gok_scanner_handler_uses_button(GokScannerHandler *h)
{
	if ( (h->EffectState == ACTION_STATE_PRESS)
	     || (h->EffectState == ACTION_STATE_RELEASE)
	     || (h->EffectState == ACTION_STATE_CLICK)
	     || (h->EffectState == ACTION_STATE_DOUBLECLICK) ) {
		return TRUE;
	} else {
		return FALSE;
	}
}

/**
 * gok_scanner_handler_uses_core_mouse_button:
 * @accessname: The name of the access method that h belongs to.
 * @h: The GokScannerHandler to test.
 * @button: The button the test against.
 *
 * Returns TRUE if the GokScannerHandler h in access method accessname
 * is currently configured to use an action that uses the core mouse
 * button button and FALSE otherwise.
 */
gboolean gok_scanner_handler_uses_core_mouse_button(char *accessname,
	GokScannerHandler *h, int button)
{
	char *handlername;
	GokAction *a;
	char *actionname;

	handlername = h->EffectName;
	if (handlername != NULL) {
		if (gok_data_get_setting(accessname, handlername, NULL,
		                         &actionname)) {
			a = gok_action_find_action(actionname, FALSE);
			if ( (a != NULL)
			     && (a->Type == ACTION_TYPE_MOUSEBUTTON)
			     && (a->Number == button) ) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

/**
* gok_scanner_read_rates
* @pNode: Pointer to the XML node that contains the first rate.
* @pAccessMethod: Pointer to the access method that is associated with the rates.
*
* Reads the rates for the given access method and add them to the list of rates
* stored on the access method.
*
* returns: TRUE if the rates were read, FALSE if there were 1 or more errors  reading the rates.
**/
gboolean gok_scanner_read_rates (xmlNode* pNode, GokAccessMethod* pAccessMethod)
{
	GokAccessMethodRate* pRate;
	GokAccessMethodRate* pRateLast;
	xmlChar* pStringName = NULL;
	xmlChar* pStringType = NULL;
	xmlChar* pStringValue = NULL;
	xmlChar* pValueAsString = NULL;
	gint TypeRate;
	gint IdRate;
	gint ValueDefault;

	g_assert (pNode != NULL);
	g_assert (pAccessMethod != NULL);
	g_assert (pAccessMethod->pRateFirst == NULL);

	pRateLast = NULL;
	IdRate = 0;

	/* loop through the document looking for rates */
	while (pNode != NULL)
	{
		/* is this node a rate? */
		if (xmlStrcmp (pNode->name, (const xmlChar *) "rate") == 0)
		{
			/* yes, get the rate attributes */
			/* get the rate name (must be present) */
			pStringName = xmlGetProp (pNode, (const xmlChar *) "name");
			if (pStringName == NULL)
			{
				gok_log_x ("Warning: Rate has no name in gok_scanner_read_rates!\n");
			}
			else
			{
				/* get the attribute "stringvalue" (optional) */
				pStringValue = xmlGetProp (pNode, (const xmlChar *) "stringvalue");

				/* get the attribute "value" (optional) */
				pValueAsString = xmlGetProp (pNode,
					(const xmlChar *) "value");

				/* get the rate type */
				TypeRate = RATE_TYPE_UNDEFINED;
				pStringType = xmlGetProp (pNode, (const xmlChar *) "type");
				if (pStringType != NULL)
				{
					if (strcmp ((gchar *)pStringType, "effect") == 0)
					{
						TypeRate = RATE_TYPE_EFFECT;
					}
				}

				/* construct a new rate */
				pRate = gok_scanner_construct_rate ();
				if (pRate == NULL)
				{
					return FALSE;
				}

				/* add the new rate to the list */
				if (pAccessMethod->pRateFirst == NULL)
				{
					pAccessMethod->pRateFirst = pRate;
				}
				else
				{
					pRateLast->pRateNext = pRate;
				}
				pRateLast = pRate;

				/* populate the rate structure */
				pRate->Name = (gchar*)g_malloc (strlen ((gchar *)pStringName) + 1);
				strcpy (pRate->Name, (gchar *)pStringName);
							
				if (pStringValue != NULL)
				{
					pRate->StringValue = (gchar*)g_malloc (strlen ((gchar *)pStringValue) + 1);
					strcpy (pRate->StringValue, (gchar *)pStringValue);
				}

				if (pValueAsString != NULL)
				{
					pRate->Value = atoi ((gchar *)pValueAsString);
				}

				pRate->Type = TypeRate;
				pRate->ID = IdRate;
				IdRate++;

				gok_log ("Just populated a new rate structure");
				gok_log ("    Name = %s", pRate->Name);
				gok_log ("    Type = %d", pRate->Type);
				if (pRate->StringValue == NULL)
				{
					gok_log ("    StringValue = NULL");
				}
				else
				{
					gok_log ("    StringValue = %s",
						 pRate->StringValue);
				}
				gok_log ("    Value = %d", pRate->Value);
				gok_log ("    ID = %d", pRate->ID);
			}
		}
		pNode = pNode->next;
	}

	return TRUE;
}

/**
* gok_scanner_read_description
* @pDoc: Pointer to the XML document that contains the node.
* @pNode: Pointer to the XML root node.
* @pAccessMethod: Pointer to the access method that is associated with the description.
*
* Reads the description of the access method and stores it on the structure.
*
* returns: TRUE if the description was read, FALSE if not.
**/
gboolean gok_scanner_read_description (xmlDoc* pDoc, xmlNode* pNode, GokAccessMethod* pAccessMethod)
{
	xmlChar* pStringDescription;

	g_assert (pNode != NULL);
	g_assert (pAccessMethod != NULL);

	gok_log_enter();
	
	if (pNode = gok_scanner_find_node (pNode, (const xmlChar *) "description", gok_scanner_get_lang (),
					   NULL)) {
		pStringDescription = xmlNodeListGetString (pDoc, pNode->xmlChildrenNode, 1);
		g_strlcpy (pAccessMethod->Description, _((gchar *)pStringDescription), MAX_DESCRIPTION_TEXT);
		gok_log ("read access method description: [%s]", pStringDescription);
		xmlFree (pStringDescription);
		gok_log_leave();
		return TRUE;
	}
	else {
		g_strlcpy (pAccessMethod->Description, (gchar*)_("error reading description"), MAX_DESCRIPTION_TEXT);
		gok_log_leave();
		return FALSE;
	}
}

/**
* gok_scanner_read_operation
* @pDoc: Pointer to the XML document that contains the node.
* @pNode: Pointer to the XML root node.
* @pAccessMethod: Pointer to the access method that is associated with the operation.
*
* Reads the UI 'operation' of the access method and stores it on the structure.
*
* returns: TRUE if the operation was read, FALSE if not.
**/
gboolean gok_scanner_read_operation (xmlDoc* pDoc, xmlNode* pNode, GokAccessMethod* pAccessMethod)
{
	GokControl ControlOperation;

	g_assert (pNode != NULL);
	g_assert (pAccessMethod != NULL);

	/* loop through the document looking for the operation */
	while (pNode != NULL)
	{
		/* is this node an operation */
		if (xmlStrcmp (pNode->name, (const xmlChar *) "operation") == 0)
		{
			ControlOperation.pControlChild = NULL;
			gok_scanner_read_ui_loop (&ControlOperation, pNode->xmlChildrenNode);
			pAccessMethod->pControlOperation = ControlOperation.pControlChild;
			
			return TRUE;
		}
		pNode = pNode->next;
	}

	return FALSE;
}

/**
* gok_scanner_read_ui_loop
* @pControl: Pointer to the parent control that will contain any new controls
* found in the node.
* @pNode: Pointer to the XML root node.
*
* Reads the UI 'operation' of the access method and stores it on the structure.
**/
void gok_scanner_read_ui_loop (GokControl* pControl, xmlNode* pNode)
{
	GokControl* pControlNew;
	GokControl* pControlChildLast;
	xmlChar* pStringAttributeValue;
	
	g_assert (pControl != NULL);

	/* loop through the operation looking for controls */
	while (pNode != NULL)
	{
		/* is this node a control? */
		if (xmlStrcmp (pNode->name, (const xmlChar *) "control") == 0)
		{
			/* create a new control */
			pControlNew = gok_control_new ();
			
			/* get all the values for the new control */
			pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "type");
			if (pStringAttributeValue != NULL)
			{
				pControlNew->Type = gok_control_get_control_type ((gchar *)pStringAttributeValue);
			}
			else
			{
				gok_log_x ("no type for control!");
			}

			pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "name");
			if (pStringAttributeValue != NULL)
			{
				pControlNew->Name = (gchar*)g_malloc (strlen ((gchar *)pStringAttributeValue) + 1);
				strcpy (pControlNew->Name, (gchar *)pStringAttributeValue);
			}
			
			pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "size");
			if (pStringAttributeValue != NULL)
			{
				pControlNew->Size = atoi ((gchar *)pStringAttributeValue);
			}

			pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "border");
			if (pStringAttributeValue != NULL)
			{
				pControlNew->Border = atoi ((gchar *)pStringAttributeValue);
			}

			pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "spacing");
			if (pStringAttributeValue != NULL)
			{
				pControlNew->Spacing = atoi ((gchar *)pStringAttributeValue);
			}
			
			pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "string");
			if (pStringAttributeValue != NULL)
			{
				pControlNew->String = (gchar*)g_malloc (strlen ((gchar *)pStringAttributeValue) + 1);
				strcpy (pControlNew->String, (gchar *)pStringAttributeValue);
			}
			
			pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "fillwith");
			if (pStringAttributeValue != NULL)
			{
				if (xmlStrcmp (pStringAttributeValue, (const xmlChar *) "actions") == 0)
				{
					pControlNew->Fillwith = CONTROL_FILLWITH_ACTIONS;
				}
				else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *) "feedbacks") == 0)
				{
					pControlNew->Fillwith = CONTROL_FILLWITH_FEEDBACKS;
				}
				else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *) "sounds") == 0)
				{
					pControlNew->Fillwith = CONTROL_FILLWITH_SOUNDS;
				}
				else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *) "options") == 0)
				{
					pControlNew->Fillwith = CONTROL_FILLWITH_OPTIONS;
				}
				else
				{
					gok_log_x ("Invalid 'fillwith' value: %s!", pStringAttributeValue);
				}
			}
			
			pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "qualifier");
			if (pStringAttributeValue != NULL)
			{
				pControlNew->Qualifier = gok_scanner_make_type_from_string ((gchar *)pStringAttributeValue);
			}
			
			pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "groupstart");
			if (pStringAttributeValue != NULL)
			{
				pControlNew->bGroupStart = (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"yes") == 0) ? TRUE : FALSE;
			}
			
			pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "value");
			if (pStringAttributeValue != NULL)
			{
				pControlNew->Value = atoi ((gchar *)pStringAttributeValue);
			}

			pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "min");
			if (pStringAttributeValue != NULL)
			{
				pControlNew->Min = atoi ((gchar *)pStringAttributeValue);
			}

			pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "max");
			if (pStringAttributeValue != NULL)
			{
				pControlNew->Max = atoi ((gchar *)pStringAttributeValue);
			}

			pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "stepincrement");
			if (pStringAttributeValue != NULL)
			{
				pControlNew->StepIncrement = atoi ((gchar *)pStringAttributeValue);
			}

			pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "pageincrement");
			if (pStringAttributeValue != NULL)
			{
				pControlNew->PageIncrement = atoi ((gchar *)pStringAttributeValue);
			}

			pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "pagesize");
			if (pStringAttributeValue != NULL)
			{
				pControlNew->PageSize = atoi ((gchar *)pStringAttributeValue);
			}

			pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "associated");
			if (pStringAttributeValue != NULL)
			{
				pControlNew->NameAssociatedControl = (gchar*)g_malloc (strlen ((gchar *)pStringAttributeValue) + 1);
				strcpy (pControlNew->NameAssociatedControl, (gchar *)pStringAttributeValue);
			}

			pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "associatedstate");
			if (pStringAttributeValue != NULL)
			{
				pControlNew->bAssociatedStateActive = atoi ((gchar *)pStringAttributeValue);
			}

			pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "handler");
			if (pStringAttributeValue != NULL)
			{
				pControlNew->Handler = gok_control_get_handler_type ((gchar *)pStringAttributeValue);
			}

			/* hook the new control into the control given */
			if (pControl->pControlChild == NULL)
			{
				pControl->pControlChild = pControlNew;
			}
			else
			{
				pControlChildLast->pControlNext = pControlNew;
			}
			pControlChildLast = pControlNew;

			/* get any children for the control */
			gok_scanner_read_ui_loop (pControlNew, pNode->xmlChildrenNode);
		}
		pNode = pNode->next;
	}
}

/**
* gok_scanner_read_feedback
* @pDoc: Pointer to the XML doc that contains the feedbacks.
* @pNode: Pointer to the XML root node.
* @pAccessMethod: Pointer to the access method that is associated with the feedback.
*
* Reads the UI 'feedback' of the access method and stores it on the structure.
*
* returns: TRUE if the feedback was read, FALSE if not.
**/
gboolean gok_scanner_read_feedback (xmlDoc* pDoc, xmlNode* pNode, GokAccessMethod* pAccessMethod)
{
	GokControl ControlFeedback;

	g_assert (pNode != NULL);
	g_assert (pAccessMethod != NULL);

	/* loop through the document looking for the feedback */
	while (pNode != NULL)
	{
		/* is this node a feedback? */
		if (xmlStrcmp (pNode->name, (const xmlChar *) "feedback") == 0)
		{
			ControlFeedback.pControlChild = NULL;
			gok_scanner_read_ui_loop (&ControlFeedback, pNode->xmlChildrenNode);
			pAccessMethod->pControlFeedback = ControlFeedback.pControlChild;
			
			return TRUE;
		}
		pNode = pNode->next;
	}

	return FALSE;
}

/**
* gok_scanner_read_options
* @pDoc: Pointer to the XML document that contains the node.
* @pNode: Pointer to the XML root node.
* @pAccessMethod: Pointer to the access method that is associated with the options.
*
* Reads the UI 'options' of the access method and stores it on the structure.
*
* returns: TRUE if the options were read, FALSE if not.
**/
gboolean gok_scanner_read_options (xmlDoc* pDoc, xmlNode* pNode, GokAccessMethod* pAccessMethod)
{
	GokControl ControlOptions;

	g_assert (pNode != NULL);
	g_assert (pAccessMethod != NULL);

	/* loop through the document looking for the options */
	while (pNode != NULL)
	{
		/* is this node an option? */
		if (xmlStrcmp (pNode->name, (const xmlChar *) "options") == 0)
		{
			ControlOptions.pControlChild = NULL;
			gok_scanner_read_ui_loop (&ControlOptions, pNode->xmlChildrenNode);
			pAccessMethod->pControlOptions = ControlOptions.pControlChild;
			
			return TRUE;
		}
		pNode = pNode->next;
	}

	return FALSE;
}

/**
* gok_scanner_read_effects
* @pNodeGiven: Pointer to the node that contains the effect nodes.
* @pAccessMethod: Pointer to the access method associated with these effects.
*
* Reads in the effects for this node from the XML file.
*
* returns: A pointer to the first effect in the list of new effects.
**/
GokScannerEffect* gok_scanner_read_effects (xmlNode* pNodeGiven, GokAccessMethod* pAccessMethod)
{
	GokScannerEffect* pEffectFirst;
	GokScannerEffect* pEffect;
	GokScannerEffect* pEffectLast;
	GokScannerEffect* pEffectTrue;
	GokScannerEffect* pEffectFalse;
	xmlChar* pStringAttributeValue;
	xmlNode* pNodeEffect;
	xmlNode* pNodeCompare;
	xmlNode* pNodeTrueFalse;
	gint count;
	gchar* Param1;
	gchar* Param2;
	gint CompareType;
	gint CompareValue;
	gchar* pName;

	pEffectFirst = NULL;
	pEffectLast = NULL;
	pEffect = NULL;

	/* loop through all the child nodes (should be effects) */
	pNodeEffect = pNodeGiven->xmlChildrenNode;
	while (pNodeEffect != NULL)
	{
		/* is this node an effect? */
		if (xmlStrcmp (pNodeEffect->name, (const xmlChar *) "effect") == 0)
		{
			Param1 = NULL;
			Param2 = NULL;
			CompareType = COMPARE_NO;
			CompareValue = 0;
			pEffectTrue = NULL;
			pEffectFalse = NULL;
			pName = NULL;

			/* get the call ID for the effect */
			/* first, find the 'call' attribute */
			pStringAttributeValue = xmlGetProp (pNodeEffect, (const xmlChar *) "call");
			if (pStringAttributeValue != NULL)
			{
				/* find the 'call' value name in our array of possible call names */
				for (count = 0; count < MAX_CALL_NAMES; count++)
				{
					if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)ArrayCallNames[count]) == 0)
					{
						/* get the attributes for this effect */
						/* Paramater 1 */
						pStringAttributeValue = xmlGetProp (pNodeEffect, (const xmlChar *) "param1");
						if (pStringAttributeValue != NULL)
						{
							Param1 = (char *)pStringAttributeValue;
						}

						/* Paramater 2 */
						pStringAttributeValue = xmlGetProp (pNodeEffect, (const xmlChar *) "param2");
						if (pStringAttributeValue != NULL)
						{
							Param2 = (char *)pStringAttributeValue;
						}

						/* call name */
						pName = (char *)xmlGetProp (pNodeEffect, (const xmlChar *) "name");

#if 0
						pStringAttributeValue = xmlGetProp (pNodeEffect, (const xmlChar *) "name");
						if (pStringAttributeValue != NULL)
						{
if (gok_data_get_setting (pAccessMethod->Name, pStringAttributeValue, NULL, &pName) == FALSE)
								pName = pStringAttributeValue;
						}
#endif

						/* does this effect node have a child "compare" node? */
						pNodeCompare = pNodeEffect->xmlChildrenNode;
						while (pNodeCompare != NULL)
						{
							if (xmlStrcmp (pNodeCompare->name, (const xmlChar *) "compare") == 0)
							{
								/* get the attributes for this compare */
								/* compare type */
								pStringAttributeValue = xmlGetProp (pNodeCompare, (const xmlChar *) "type");
								if (pStringAttributeValue != NULL)
								{
									if (xmlStrcmp (pStringAttributeValue, (const xmlChar *) "equal") == 0)
									{
										CompareType = COMPARE_EQUAL;
									}
									else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *) "lessthan") == 0)
									{
										CompareType = COMPARE_LESSTHAN;
									}
									else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *) "greaterthan") == 0)
									{
										CompareType = COMPARE_GREATERTHAN;
									}
									else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *) "equalorlessthan") == 0)
									{
										CompareType = COMPARE_EQUALORLESSTHAN;
									}
									else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *) "equalorgreaterthan") == 0)
									{
										CompareType = COMPARE_EQUALORGREATERTHAN;
									}
									else
									{
										gok_log_x ("Warning: Compare type invalid (%s) in gok_scanner_read_effects!\n", pStringAttributeValue);
									}
								}

								/* compare value */
								pStringAttributeValue = xmlGetProp (pNodeCompare, (const xmlChar *) "value");
								if (pStringAttributeValue != NULL)
								{
									CompareValue = atoi ((char *)pStringAttributeValue);
								}

								/* get the true/false effects for the compare */
								pEffectTrue = NULL;
								pEffectFalse = NULL;
								pNodeTrueFalse = pNodeCompare->xmlChildrenNode;
								while (pNodeTrueFalse != NULL)
								{
									if ((xmlStrcmp (pNodeTrueFalse->name, (const xmlChar *) "true") == 0))
									{
										if (pEffectTrue != NULL)
										{
											gok_log_x ("Warning: Duplicate 'true' effect in gok_scanner_read_effects!\n");
										}
										else
										{
											pEffectTrue = gok_scanner_read_effects (pNodeTrueFalse, pAccessMethod);
										}
									}
									else if ((xmlStrcmp (pNodeTrueFalse->name, (const xmlChar *) "false") == 0))
									{
										if (pEffectFalse != NULL)
										{
											gok_log_x ("Warning: Duplicate 'false' effect in gok_scanner_read_effects!\n");
										}
										else
										{
											pEffectFalse = gok_scanner_read_effects (pNodeTrueFalse, pAccessMethod);
										}
									}
									else if ((xmlStrcmp (pNodeTrueFalse->name, (const xmlChar *) "text") != 0))
									{
										gok_log_x ("Warning: Compare child not true or false ('%s') in gok_scanner_read_effects!\n", pNodeTrueFalse->name);
									}
									pNodeTrueFalse = pNodeTrueFalse->next;
								}
								break;
							}
							else
							{
								pNodeCompare = pNodeCompare->next;
							}
						}

						/* create the effect */
						pEffect = gok_scanner_create_effect (count,
															Param1,
															Param2,
															CompareType,
															CompareValue,
															pEffectTrue,
															pEffectFalse,
															pName);
						/* add the effect to our list of effects */
						if (pEffectFirst == NULL)
						{
							pEffectFirst = pEffect;
						}
						else
						{
							pEffectLast->pEffectNext = pEffect;
						}
						pEffectLast = pEffect;

						break;
					}
				}

				/* did we find the call name? */
				if (count >= MAX_CALL_NAMES)
				{
					/* No, didn't find the call in our array! */
					gok_log_x ("Can't find call '%s' from access method: %s\n", pStringAttributeValue, pAccessMethod->Name);
				}
			}
		}

		/* get the next effect */
		pNodeEffect = pNodeEffect->next;
	}

	return pEffectFirst;
}

/**
* gok_scanner_create_effect
* @CallId: Id of the call (e.g. CALL_CHUNKER_RESET)
* @Param1: Parameter 1 of the call.
* @Param2: Parameter 2 of the call.
* @CompareType: Specifies if the return value should be compared.
* @CompareValue: Value that is compared to the return value.
* @pEffectTrue: Pointer to the effects that will be called if the comparison is TRUE.
* @pEffectFalse: Pointer to the effects that will be called if the comparison is FALSE.
* @pName: Pointer to the name of the effect.
*
* Creates a new effect call.
*
* returns: A pointer to the new call, NULL if it can't be created.
**/
GokScannerEffect* gok_scanner_create_effect (gint CallId, gchar* Param1, gchar* Param2, gint CompareType, gint CompareValue, GokScannerEffect* pEffectTrue, GokScannerEffect* pEffectFalse, gchar* pName)
{
	GokScannerEffect* pEffectNew;
	
	pEffectNew = (GokScannerEffect*) g_malloc(sizeof(GokScannerEffect));

	pEffectNew->CallId = CallId;
	pEffectNew->CallReturn = 0;
	pEffectNew->CompareType = CompareType;
	pEffectNew->CompareValue = CompareValue;
	pEffectNew->pEffectNext = NULL;
	pEffectNew->pEffectTrue = pEffectTrue;
	pEffectNew->pEffectFalse = pEffectFalse;
	if (pName == NULL)
	{
		pEffectNew->pName = NULL;
	}
	else
	{
		pEffectNew->pName = (gchar*) g_malloc (strlen (pName) + 1);
		strcpy (pEffectNew->pName, pName);
	}
	
	if (Param1 == NULL)
	{
		pEffectNew->CallParam1 = NULL;
	}
	else
	{
		pEffectNew->CallParam1 = (gchar*) g_malloc (strlen (Param1) + 1);
		strcpy (pEffectNew->CallParam1, Param1);
	}

	if (Param2 == NULL)
	{
		pEffectNew->CallParam2 = NULL;
	}
	else
	{
		pEffectNew->CallParam2 = (gchar*) g_malloc (strlen (Param2) + 1);
		strcpy (pEffectNew->CallParam2, Param2);
	}

	return pEffectNew;
}

/**
* gok_scanner_create_access_method
* @Name: Name of the access method.
*
* Creates a new access method structure and adds it to the list of access methods.
*
* returns: A pointer to the new access method, NULL if if can't be created.
**/
GokAccessMethod* gok_scanner_create_access_method (gchar* Name)
{
	GokAccessMethod* pAccessMethod;
	GokAccessMethod* pAccessMethodTemp;

	/* create the new access method structure */
	pAccessMethod = (GokAccessMethod*) g_malloc(sizeof(GokAccessMethod));

	/* initialize all the member variables */
	pAccessMethod->pStateFirst = NULL;
	pAccessMethod->pAccessMethodNext = NULL;
	pAccessMethod->pInitializationEffects = NULL;
	pAccessMethod->pRateFirst = NULL;
	pAccessMethod->pXmlDoc = NULL;
	pAccessMethod->pControlOperation = NULL;
	pAccessMethod->pControlFeedback = NULL;
	pAccessMethod->pControlOptions = NULL;

	/* set the name of the access method */
	strncpy (pAccessMethod->Name, Name, MAX_ACCESS_METHOD_NAME);

	/* add it to the list of access methods */
	/* add it at the start if the start hasn't been set */
	if (m_pAccessMethodFirst == NULL)
	{
		m_pAccessMethodFirst = pAccessMethod;
	}
	else /* add it at the end of the list */
	{
		pAccessMethodTemp = m_pAccessMethodFirst;
		while (pAccessMethodTemp->pAccessMethodNext != NULL)
		{
			pAccessMethodTemp = pAccessMethodTemp->pAccessMethodNext;
		}
		pAccessMethodTemp->pAccessMethodNext = pAccessMethod;
	}

	return pAccessMethod;
}

/**
* gok_scanner_close
*
* Stops the current access method and frees any memory allocated for the
* access methods. This must be called at the end of the program.
**/
void gok_scanner_close()
{
	GokAccessMethod* pAccessMethod;
	GokAccessMethod* pAccessMethodTemp;
	GokScannerState* pState;
	GokScannerState* pStateTemp;
	GokScannerHandler* pHandler;
	GokScannerHandler* pHandlerTemp;
	GokAccessMethodRate* pRate;
	GokAccessMethodRate* pRateTemp;

	/* delete all the access methods */
	pAccessMethod = m_pAccessMethodFirst;
	while (pAccessMethod != NULL)
	{
		/* free the XML document associated with the access method */
		xmlFreeDoc (pAccessMethod->pXmlDoc);
		
		/* delete all the initialization effects for the access method */
		gok_scanner_delete_effect (pAccessMethod->pInitializationEffects);
		
		/* delete all the controls associated with the access method */
		gok_control_delete_all (pAccessMethod->pControlOperation);
		gok_control_delete_all (pAccessMethod->pControlFeedback);
		gok_control_delete_all (pAccessMethod->pControlOptions);
		
		/* delete all the handler states for the access method */
		pState = pAccessMethod->pStateFirst;
		while (pState != NULL)
		{
			/* delete all the handlers for the state */
			pHandler = pState->pHandlerFirst;
			while (pHandler != NULL)
			{
				/* delete all the effects for the handler */
				gok_scanner_delete_effect (pHandler->pEffectFirst);

				pHandlerTemp = pHandler;
				pHandler = pHandler->pHandlerNext;
				if (pHandlerTemp->EffectName != NULL)
				{
					g_free (pHandlerTemp->EffectName);
				}
				g_free (pHandlerTemp);
			}

			pStateTemp = pState;
			pState = pState->pStateNext;
			g_free (pStateTemp);
		}

		/* delete all the access method rates */
		pRate = pAccessMethod->pRateFirst;
		while (pRate != NULL)
		{
			pRateTemp = pRate;
			pRate = pRate->pRateNext;
			
			if (pRateTemp->Name != NULL)
			{
				g_free (pRateTemp->Name);
			}

			if (pRateTemp->StringValue != NULL)
			{
				g_free (pRateTemp->StringValue);
			}

			g_free (pRateTemp);
		}

		/* delete the access method */
		pAccessMethodTemp = pAccessMethod;
		pAccessMethod = pAccessMethod->pAccessMethodNext;
		g_free (pAccessMethodTemp);
	}
}

/**
* gok_scanner_delete_effect
* @pEffect: Pointer to the effect that gets deleted.
*
* Deletes the given effect and all effects that are linked to this effect.
* This is a recursive function.
**/
void gok_scanner_delete_effect (GokScannerEffect* pEffect)
{
	GokScannerEffect* pEffectTemp;

	while (pEffect != NULL)
	{
		pEffectTemp = pEffect;
		pEffect = pEffect->pEffectNext;

		gok_scanner_delete_effect (pEffectTemp->pEffectTrue);
		gok_scanner_delete_effect (pEffectTemp->pEffectFalse);

		g_free (pEffectTemp);
	}
}

/**
* gok_scanner_change_method
* @NameAccessMethod: Name of the desired access method.
*
* Changes the type of access method.
*
* returns: TRUE if the access method was changed, FALSE if not.
**/
gboolean gok_scanner_change_method (gchar* NameAccessMethod)
{
	GokAccessMethod* pAccessMethod;

	g_assert (m_pAccessMethodFirst != NULL);

	/* Are we already using this access method? */
	if ((m_pAccessMethodCurrent != NULL) &&
		(strcmp (NameAccessMethod, m_pAccessMethodCurrent->Name) == 0))
	{
		/* yes, so nothing to do */
		return TRUE;
	}

	/* stop the current access method */
	gok_scanner_stop();

	/* find the new access method in the list */
	pAccessMethod = m_pAccessMethodFirst;
	while (pAccessMethod != NULL)
	{
		if (g_strcasecmp (pAccessMethod->Name, NameAccessMethod) == 0)
		{
			/* found the access method */
			m_pAccessMethodCurrent = pAccessMethod;

			/* set the current access method as this one */
			m_pAccessMethodCurrent = pAccessMethod;
			
			/* store the name of the access method */
			gok_data_set_name_accessmethod (pAccessMethod->Name);

			/* reset the access method */
			gok_scanner_reset_access_method();

			return TRUE;
		}
		else
		{
			/* look at the next access method in the list */
			pAccessMethod = pAccessMethod->pAccessMethodNext;
		}
	}
	
	gok_log_x ("Warning: gok_scanner_change_method failed (%s)!", NameAccessMethod);
	/* report bad access name to the user */
	fprintf(stderr,"Unkown access method name: %s, aborting.",NameAccessMethod);
	return FALSE;
}

/**
* gok_scanner_reset_access_method
*
* Resets the current access method so it's ready for use.
**/
void gok_scanner_reset_access_method ()
{
	g_assert (m_pAccessMethodCurrent != NULL);
	gok_log_enter();

	m_bMouseLeftRelease = TRUE;
	
	/* perform any initialization for the access method */
	gok_log ("call to perform effects");
	gok_scanner_perform_effects (m_pAccessMethodCurrent->pInitializationEffects);
	
	/* change handles to first state for this effect method */
	gok_log ("call to change state");
	gok_scanner_change_state (m_pAccessMethodCurrent->pStateFirst, m_pAccessMethodCurrent->Name);

	gok_log_leave();
}

/**
* gok_scanner_change_state
* @pState: Pointer to the new state.
* @NameAccessMethod: Name of the access method that contains the state.
*
* Maps event handlers to effects..  
**/
void gok_scanner_change_state (GokScannerState* pState, gchar* NameAccessMethod)
{
	GokScannerHandler* pHandler;
	gint rate;
	gchar* pNameEffect;
	gint stateEffect;
	GokAction* pAction;

	gok_log_enter();
	gok_log("STATE: %s next state? %d", pState->NameState, pState->pStateNext);	
	if (pState == NULL)
	{
		gok_log_x ("State is NULL!");
		gok_log_leave();
		return;
	}
	
	/* stop all timers - TODO: test if this causes bad behaviour */
	gok_scanner_timer_stop (1);
	gok_scanner_timer_stop (2);
	gok_scanner_timer_stop (3);
	gok_scanner_timer_stop (4);
	gok_scanner_timer_stop (5);

	/* store a pointer to the current state */
	m_pStateCurrent = pState;

	/* NULL all the handlers */
	gok_scanner_set_handlers_null();

	/* perform any initialization effects for the state */
	gok_scanner_perform_effects (pState->pEffectInit);
	
	pHandler = pState->pHandlerFirst;
	while (pHandler != NULL)
	{
		if (pHandler->bPredefined == TRUE)
		{
			switch (pHandler->TypeHandler)
			{
				case ACTION_TYPE_TIMER1:
					m_pEffectsOnTimer1 = pHandler->pEffectFirst;
					break;
					
				case ACTION_TYPE_TIMER2:
					m_pEffectsOnTimer2 = pHandler->pEffectFirst;
					break;
					
				case ACTION_TYPE_TIMER3:
					m_pEffectsOnTimer3 = pHandler->pEffectFirst;
					break;
					
				case ACTION_TYPE_TIMER4:
					m_pEffectsOnTimer4 = pHandler->pEffectFirst;
					break;
					
				case ACTION_TYPE_TIMER5:
					m_pEffectsOnTimer5 = pHandler->pEffectFirst;
					break;
					
				default:
					gok_log_x ("Invalid handler type: %d", pHandler->TypeHandler);
					break;
			}
			
			pHandler = pHandler->pHandlerNext;
			continue;
		}

		/* get the effect name from the rate */
		if (gok_data_get_setting (NameAccessMethod, pHandler->EffectName, &rate, &pNameEffect) == FALSE)
		{
			gok_log_x ("Can't get effect name for handler! Access method: %s, effect name: %s", NameAccessMethod, pHandler->EffectName);
			pHandler = pHandler->pHandlerNext;
			continue;
		}
	
		/* get the effect */
		pAction = gok_action_find_action (pNameEffect, FALSE);
		if (pAction == NULL)
		{
			gok_log_x ("Can't find effect '%s'!", pNameEffect);
			pHandler = pHandler->pHandlerNext;
			continue;
		}

		/* does the handler have a required state? */
		/* use the state from the effect if the handler state is undefined */
		stateEffect = (pHandler->EffectState == ACTION_STATE_UNDEFINED) ?
						pAction->State : pHandler->EffectState;		

		/* attach the system event to the effect's handler */
		switch (pAction->Type)
		{
			case ACTION_TYPE_SWITCH:
				switch (pAction->Number)
				{
					case 1:
						if (stateEffect == ACTION_STATE_PRESS)
						{
							m_pEffectsOnSwitch1Down = pHandler->pEffectFirst;
						}
						else /* pEffect->State == ACTION_STATE_RELEASE */
						{
							m_pEffectsOnSwitch1Up = pHandler->pEffectFirst;
						}
						break;
						
					case 2:
						if (stateEffect == ACTION_STATE_PRESS)
						{
							m_pEffectsOnSwitch2Down = pHandler->pEffectFirst;
						}
						else /* pEffect->State == ACTION_STATE_RELEASE */
						{
							m_pEffectsOnSwitch2Up = pHandler->pEffectFirst;
						}
						break;
						
					case 3:
						if (stateEffect == ACTION_STATE_PRESS)
						{
							m_pEffectsOnSwitch3Down = pHandler->pEffectFirst;
						}
						else /* pEffect->State == ACTION_STATE_RELEASE */
						{
							m_pEffectsOnSwitch3Up = pHandler->pEffectFirst;
						}
						break;
						
					case 4:
						if (stateEffect == ACTION_STATE_PRESS)
						{
							m_pEffectsOnSwitch4Down = pHandler->pEffectFirst;
						}
						else /* pEffect->State == ACTION_STATE_RELEASE */
						{
							m_pEffectsOnSwitch4Up = pHandler->pEffectFirst;
						}
						break;
						
					case 5:
						if (stateEffect == ACTION_STATE_PRESS)
						{
							m_pEffectsOnSwitch5Down = pHandler->pEffectFirst;
						}
						else /* pEffect->State == ACTION_STATE_RELEASE */
						{
							m_pEffectsOnSwitch5Up = pHandler->pEffectFirst;
						}
						break;
						
					default:
						gok_log_x ("Default hit!\n");
						break;
				}
				break;
				
			case ACTION_TYPE_MOUSEBUTTON:
				switch (pAction->Number)
				{
					case 1:
						if (stateEffect == ACTION_STATE_PRESS)
						{
							m_pEffectsLeftButtonDown = pHandler->pEffectFirst;
						}
						else if (stateEffect == ACTION_STATE_RELEASE)
						{
							m_pEffectsLeftButtonUp = pHandler->pEffectFirst;
						}
						else if (stateEffect == ACTION_STATE_CLICK)
						{
						}
						else /* DOUBLE CLICK */
						{
						}
						break;
						
					case 2:
						if (stateEffect == ACTION_STATE_PRESS)
						{
							m_pEffectsMiddleButtonDown = pHandler->pEffectFirst;
						}
						else if (stateEffect == ACTION_STATE_RELEASE)
						{
							m_pEffectsMiddleButtonUp = pHandler->pEffectFirst;
						}
						else if (stateEffect == ACTION_STATE_CLICK)
						{
						}
						else /* DOUBLE CLICK */
						{
						}
						break;
						
					case 3:
						if (stateEffect == ACTION_STATE_PRESS)
						{
							m_pEffectsRightButtonDown = pHandler->pEffectFirst;
						}
						else if (stateEffect == ACTION_STATE_RELEASE)
						{
							m_pEffectsRightButtonUp = pHandler->pEffectFirst;
						}
						else if (stateEffect == ACTION_STATE_CLICK)
						{
						}
						else /* DOUBLE CLICK */
						{
						}
						break;

					case 4:
						if (stateEffect == ACTION_STATE_PRESS)
						{
							m_pEffectsMouseButton4Down = pHandler->pEffectFirst;
						}
						else if (stateEffect == ACTION_STATE_RELEASE)
						{
							m_pEffectsMouseButton4Up = pHandler->pEffectFirst;
						}
						else if (stateEffect == ACTION_STATE_CLICK)
						{
						}
						else /* DOUBLE CLICK */
						{
						}
						break;
						
					case 5:
						if (stateEffect == ACTION_STATE_PRESS)
						{
							m_pEffectsMouseButton5Down = pHandler->pEffectFirst;
						}
						else if (stateEffect == ACTION_STATE_RELEASE)
						{
							m_pEffectsMouseButton5Up = pHandler->pEffectFirst;
						}
						else if (stateEffect == ACTION_STATE_CLICK)
						{
						}
						else /* DOUBLE CLICK */
						{
						}
						break;
						
					default:
						gok_log_x ("Default hit!\n");
						break;
				}
				break;
				
			case ACTION_TYPE_MOUSEPOINTER:
				if (stateEffect == ACTION_STATE_ENTER)
				{
					m_pEffectsOnKeyEnter = pHandler->pEffectFirst;
				}
				else if (stateEffect == ACTION_STATE_LEAVE)
				{
					m_pEffectsOnKeyLeave = pHandler->pEffectFirst;
				}
				else
				{
					gok_log_x ("Invalid state for mousepointer!");
				}
				break;
				
			case ACTION_TYPE_DWELL:
				gok_scanner_timer_set_dwell_rate (pAction->Rate);
				m_pEffectsOnDwell = pHandler->pEffectFirst;
			
				break;
				
			default:
				break;
		}

		pHandler = pHandler->pHandlerNext;
	}
	gok_log_leave();
}

/**
* gok_scanner_start
*
* Starts the current access method.
**/
void gok_scanner_start()
{
	g_assert (m_pAccessMethodCurrent != NULL);
}

/**
* gok_scanner_stop
*
* Stops the current access method.
*
* returns: void
**/
void gok_scanner_stop()
{
	gok_feedback_set_selected_key (NULL);
}

/**
* gok_scanner_left_button_down
*
* Handler for the left mouse button down event.
**/
void gok_scanner_left_button_down()
{
	/* ignore this event if there hasn't been a release yet */
	/* this is needed to ignore double-click events */
	if (m_bMouseLeftRelease == TRUE)
	{
		m_bMouseLeftRelease = FALSE;
		gok_scanner_perform_effects (m_pEffectsLeftButtonDown);
	}
}

/**
* gok_scanner_left_button_up
*
* Handler for the left mouse button up event.
**/
void gok_scanner_left_button_up()
{
	m_bMouseLeftRelease = TRUE;
	gok_scanner_perform_effects (m_pEffectsLeftButtonUp);
}

/**
* gok_scanner_right_button_down
*
* Handler for the right mouse button down event.
**/
void gok_scanner_right_button_down()
{
	/* ignore this event if there hasn't been a release yet */
	/* this is needed to ignore double-click events */
	if (m_bMouseRightRelease == TRUE)
	{
		gok_scanner_perform_effects (m_pEffectsRightButtonDown);
	}
	m_bMouseRightRelease = FALSE;
}

/**
* gok_scanner_right_button_up
*
* Handler for the right mouse button up event.
**/
void gok_scanner_right_button_up()
{
	m_bMouseRightRelease = TRUE;
	gok_scanner_perform_effects (m_pEffectsRightButtonUp);
}

/**
* gok_scanner_on_key_enter
* @pKey: Pointer to the key that the mouse pointer has entered.
*
* Handler for the key enter notify.
**/
void gok_scanner_on_key_enter (GokKey* pKey)
{
	m_pKeyEntered = pKey;

	gok_main_raise_window ();
	gok_scanner_timer_start_dwell();
		
	gok_scanner_perform_effects (m_pEffectsOnKeyEnter);
}

/**
* gok_scanner_on_key_leave
* @pKey: Pointer to the key that the mouse pointer has left.
*
* Handler for the key leave notify.
**/
void gok_scanner_on_key_leave (GokKey* pKey)
{
	gok_scanner_perform_effects (m_pEffectsOnKeyLeave);

	gok_scanner_timer_stop_dwell();

	m_pKeyEntered = NULL; 

	gok_feedback_set_selected_key (NULL);
}

/**
* gok_scanner_middle_button_down
*
* Handler for the middle mouse button down event.
**/
void gok_scanner_middle_button_down()
{
	/* ignore this event if there hasn't been a release yet */
	/* this is needed to ignore double-click events */
	if (m_bMouseMiddleRelease == TRUE)
	{
		gok_scanner_perform_effects (m_pEffectsMiddleButtonDown);
	}
	m_bMouseMiddleRelease = FALSE;
}

/**
* gok_scanner_middle_button_up
*
* Handler for the mouse middle button up event.
**/
void gok_scanner_middle_button_up()
{
	m_bMouseMiddleRelease = TRUE;
	gok_scanner_perform_effects (m_pEffectsMiddleButtonUp);
}

/**
* gok_scanner_on_button4_down
*
* Handler for mouse button #4 down event.
**/
void gok_scanner_on_button4_down()
{
	/* ignore this event if there hasn't been a release yet */
	/* this is needed to ignore double-click events */
	if (m_bMouse4Release == TRUE)
	{
		m_bMouse4Release = FALSE;
		gok_scanner_perform_effects (m_pEffectsMouseButton4Down);
	}
}

/**
* gok_scanner_on_button4_up
*
* Handler for mouse button #4 up event.
**/
void gok_scanner_on_button4_up()
{
	m_bMouse4Release = TRUE;
	gok_scanner_perform_effects (m_pEffectsMouseButton4Up);
}

/**
* gok_scanner_on_button5_down
*
* Handler for mouse button #5 down event.
**/
void gok_scanner_on_button5_down()
{
	/* ignore this event if there hasn't been a release yet */
	/* this is needed to ignore double-click events */
	if (m_bMouse5Release == TRUE)
	{
		m_bMouse5Release = FALSE;
		gok_scanner_perform_effects (m_pEffectsMouseButton5Down);
	}
}

/**
* gok_scanner_on_button5_up
*
* Handler for mouse button #5 up event.
**/
void gok_scanner_on_button5_up()
{
	m_bMouse5Release = TRUE;
	gok_scanner_perform_effects (m_pEffectsMouseButton5Up);
}

/**
* gok_scanner_get_pointer_location:
* @pX: Pointer to the integer that receives the mouse pointer X coordinate.
* @pY: Pointer to the integer that receives the mouse pointer Y coordinate.
*
* Gets the current location of the mouse pointer.
**/
void gok_scanner_get_pointer_location (gint* pX, gint* pY)
{
	*pX = m_MouseX;
	*pY = m_MouseY;
}

/**
* gok_scanner_mouse_movement
* @x: Horizontal location of the mouse pointer.
* @y: Vertical location of the mouse pointer.
*
* Handler for the mouse movement event. Stores the location of the mouse pointer.
**/
void gok_scanner_mouse_movement (gint x, gint y)
{
	m_MouseX = x;
	m_MouseY = y;

	gok_scanner_perform_effects (m_pEffectsMouseMovement);
}

/**
* gok_scanner_input_motion
* @x: Horizontal (first axis) input device motion component.
* @y: Vertical (second axis)  input device motion component.
*
* Handler for input device motion events. 
**/
void gok_scanner_input_motion (gint *motion_data, gint n_axes)
{
	GokKeyboard *pKeyboard = gok_main_get_current_keyboard ();
	GokKey *pKey;
	GtkWidget *pWindow = gok_main_get_main_window ();
	gint x=0, y=0;
	if ((n_axes >= 2) && pWindow->window) {
		gint o_x, o_y;
		gdouble multiplier = gok_data_get_valuator_sensitivity ();
		x = motion_data[0] * multiplier - m_OffsetX;
		y = motion_data[1] * multiplier - m_OffsetY;
		gdk_window_get_origin (pWindow->window, &o_x, &o_y);
		if (!gok_data_get_drive_corepointer ()) {
			gok_pointer_clip_to_window (&x, &y, pWindow->window, 
						    gok_scanner_get_slop ());
		}
		else {
			GdkWindow *root = 
				gdk_screen_get_root_window (
					gdk_drawable_get_screen (
						pWindow->window));
			gok_pointer_clip_to_window (&x, &y, root, 0);
		}
		motion_data[0] = x;
		motion_data[1] = y;
		x -= o_x;
		y -= o_y;
	}
	pKey = gok_keyboard_find_key_at (pKeyboard, x, y, m_pContainingKey);
	gok_keyboard_unpaint_pointer (pKeyboard, pWindow);
	if (m_pContainingKey != pKey) {
		gok_scanner_on_key_leave (m_pContainingKey);
		if (pKey) {
			gok_scanner_on_key_enter (pKey);
		}
		else if (m_pContainingKey) {
		        gok_scanner_on_key_leave (m_pContainingKey);
		}
		m_pContainingKey = pKey;
	}
	gok_keyboard_paint_pointer (pKeyboard, 
				    pWindow, x, y);
}

/**
* gok_scanner_on_timer1
* @data: Pointer to the user data associated with the timer.
*
* Handler for the timer1 event (timer 1 has counted down).
*
* returns: FALSE always. 
**/
gboolean gok_scanner_on_timer1 (gpointer data)
{
	m_bTimer1Started = FALSE;
	gok_scanner_perform_effects (m_pEffectsOnTimer1);

	return FALSE;
}

/**
* gok_scanner_on_timer2
* @data: Pointer to the user data associated with the timer.
*
* Handler for the timer2 event (timer 2 has counted down).
*
* returns: FALSE always. 
**/
gboolean gok_scanner_on_timer2 (gpointer data)
{
	m_bTimer2Started = FALSE;
	gok_scanner_perform_effects (m_pEffectsOnTimer2);

	return FALSE;
}

/**
* gok_scanner_on_timer3
* @data: Pointer to the user data associated with the timer.
*
* Handler for the timer3 event (timer 3 has counted down).
*
* returns: FALSE always. 
**/
gboolean gok_scanner_on_timer3 (gpointer data)
{
	m_bTimer3Started = FALSE;
	gok_scanner_perform_effects (m_pEffectsOnTimer3);

	return FALSE;
}

/**
* gok_scanner_on_timer4
* @data: Pointer to the user data associated with the timer.
*
* Handler for the timer4 event (timer 4 has counted down).
*
* returns: FALSE always. 
**/
gboolean gok_scanner_on_timer4 (gpointer data)
{
	m_bTimer4Started = FALSE;
	gok_scanner_perform_effects (m_pEffectsOnTimer4);

	return FALSE;
}

/**
* gok_scanner_on_timer5
* @data: Pointer to the user data associated with the timer.
*
* Handler for the timer5 event (timer 5 has counted down).
*
* returns: FALSE always. 
**/
gboolean gok_scanner_on_timer5 (gpointer data)
{
	m_bTimer5Started = FALSE;
	gok_scanner_perform_effects (m_pEffectsOnTimer5);

	return FALSE;
}

/**
* gok_scanner_on_switch1_down
*
* Handler for the switch 1 down event.
**/
void gok_scanner_on_switch1_down ()
{
	gok_main_raise_window ();
	gok_scanner_perform_effects (m_pEffectsOnSwitch1Down);
}

/**
* gok_scanner_on_switch1_up
*
* Handler for the switch 1 up event.
**/
void gok_scanner_on_switch1_up ()
{
	gok_scanner_perform_effects (m_pEffectsOnSwitch1Up);
}

/**
* gok_scanner_on_switch2_down
*
* Handler for the switch 2 down event.
**/
void gok_scanner_on_switch2_down ()
{
	gok_main_raise_window ();
	gok_scanner_perform_effects (m_pEffectsOnSwitch2Down);
}

/**
* gok_scanner_on_switch2_up
*
* Handler for the switch 2 up event.
**/
void gok_scanner_on_switch2_up ()
{
	gok_scanner_perform_effects (m_pEffectsOnSwitch2Up);
}

/**
* gok_scanner_on_switch3_down
*
* Handler for the switch 3 down event.
**/
void gok_scanner_on_switch3_down ()
{
	gok_main_raise_window ();
	gok_scanner_perform_effects (m_pEffectsOnSwitch3Down);
}

/**
* gok_scanner_on_switch3_up
*
* Handler for the switch 3 up event.
**/
void gok_scanner_on_switch3_up ()
{
	gok_scanner_perform_effects (m_pEffectsOnSwitch3Up);
}

/**
* gok_scanner_on_switch4_down
*
* Handler for the switch 4 down event.
**/
void gok_scanner_on_switch4_down ()
{
	gok_main_raise_window ();
	gok_scanner_perform_effects (m_pEffectsOnSwitch4Down);
}

/**
* gok_scanner_on_switch4_up
*
* Handler for the switch 4 up event.
**/
void gok_scanner_on_switch4_up ()
{
	gok_scanner_perform_effects (m_pEffectsOnSwitch4Up);
}

/**
* gok_scanner_on_switch5_down
*
* Handler for the switch 5 down event.
**/
void gok_scanner_on_switch5_down ()
{
	gok_main_raise_window ();
	gok_scanner_perform_effects (m_pEffectsOnSwitch5Down);
}

/**
* gok_scanner_on_switch5_up
*
* Handler for the switch 5 up event.
**/
void gok_scanner_on_switch5_up ()
{
	gok_scanner_perform_effects (m_pEffectsOnSwitch5Up);
}

/**
* gok_scanner_perform_effects
* @pEffect: Pointer to the first effect that will be performed.
*
* Performs one or more effects (e.g. hilight next row).
**/
void gok_scanner_perform_effects (GokScannerEffect* pEffect)
{
	gint EffectCodeReturned;
	GokAccessMethod* pAccessMethod;
	gchar* pFeedbackName;
	gint rate;
	gint param1Rate;
	gint param2Rate;
	gint counterNumber;
	GokKey* pKey;

	gok_log_enter();
	/* we need the name of the access method to get the gok_data */
	pAccessMethod = gok_scanner_get_current_access_method();
	g_assert (pAccessMethod != NULL);

	/* perform the handler effect */
	while (pEffect != NULL)
	{
		gok_log("looking at effect: [%s]",ArrayCallNames[pEffect->CallId]);
		EffectCodeReturned = 0;

		switch (pEffect->CallId)
		{
			case CALL_CHUNKER_CHUNK_NONE:
				EffectCodeReturned = gok_chunker_chunk_none ();
				break;

			case CALL_CHUNKER_CHUNK_KEYS:
				if (gok_data_get_setting (pAccessMethod->Name, pEffect->CallParam1, 
											&param1Rate, NULL) == TRUE)
				{
					if (gok_data_get_setting (pAccessMethod->Name, pEffect->CallParam2, 
												&param2Rate, NULL) == TRUE)
					{
						EffectCodeReturned = gok_chunker_chunk_keys (param1Rate, param2Rate); 
					}
					else
					{
						gok_log_x ("Can't get CHUNKER_KEYS param2 setting: %s!\n", pEffect->pName);
					}
				}
				else
				{
					gok_log_x ("Can't get CHUNKER_KEYS param1 setting: %s!\n", pEffect->pName);
				}
				break;

			case CALL_CHUNKER_CHUNK_ROWS:
				if (gok_data_get_setting (pAccessMethod->Name, pEffect->CallParam1, 
											&param1Rate, NULL) == TRUE)
				{
					if (gok_data_get_setting (pAccessMethod->Name, pEffect->CallParam2, 
												&param2Rate, NULL) == TRUE)
					{
						EffectCodeReturned = gok_chunker_chunk_rows (param1Rate, param2Rate); 
					}
					else
					{
						gok_log_x ("Can't get CHUNKER_ROWS param2 setting: %s!\n", pEffect->pName);
					}
				}
				else
				{
					gok_log_x ("Can't get CHUNKER_ROWS param1 setting: %s!\n", pEffect->pName);
				}
				break;

			case CALL_CHUNKER_CHUNK_COLUMNS:
				if (gok_data_get_setting (pAccessMethod->Name, pEffect->pName, 
											&param1Rate, NULL) == TRUE)
				{
					if (gok_data_get_setting (pAccessMethod->Name, pEffect->pName, 
												&param2Rate, NULL) == TRUE)
					{
						EffectCodeReturned = gok_chunker_chunk_columns (param1Rate, param2Rate); 
					}
					else
					{
						gok_log_x ("Can't get CHUNKER_COLUMNS param2 setting: %s!\n", pEffect->pName);
					}
				}
				else
				{
					gok_log_x ("Can't get CHUNKER_COLUMNS param1 setting: %s!\n", pEffect->pName);
				}
				break;

			case CALL_CHUNKER_RESET:
				gok_chunker_reset(); 
				break;

			case CALL_CHUNKER_NEXT_CHUNK:
				gok_chunker_next_chunk(); 
				break;

			case CALL_CHUNKER_PREVIOUS_CHUNK:
				gok_chunker_previous_chunk(); 
				break;

			case CALL_CHUNKER_NEXT_KEY:
				gok_chunker_next_key(); 
				break;

			case CALL_CHUNKER_PREVIOUS_KEY:
				gok_chunker_previous_key(); 
				break;

			case CALL_CHUNKER_KEY_UP:
				gok_chunker_keyup(); 
				break;

			case CALL_CHUNKER_KEY_DOWN:
				gok_chunker_keydown(); 
				break;

			case CALL_CHUNKER_KEY_LEFT:
				gok_chunker_keyleft();
				break;

			case CALL_CHUNKER_KEY_RIGHT:
				gok_chunker_keyright(); 
				break;

			case CALL_CHUNKER_KEY_HIGHLIGHT:
				gok_feedback_highlight (m_pKeyEntered, FALSE); 
				break;

			case CALL_CHUNKER_KEY_UNHIGHLIGHT:
				gok_feedback_unhighlight (m_pKeyEntered, FALSE); 
				break;

			case CALL_CHUNKER_WRAP_TOFIRST_CHUNK:
				gok_chunker_wraptofirstchunk (); 
				break;

			case CALL_CHUNKER_WRAP_TOLAST_CHUNK:
				gok_chunker_wraptolastchunk (); 
				break;

			case CALL_CHUNKER_WRAP_TOFIRST_KEY:
				gok_chunker_wraptofirstkey (); 
				break;

			case CALL_CHUNKER_WRAP_TOLAST_KEY:
				gok_chunker_wraptolastkey (); 
				break;

			case CALL_CHUNKER_WRAP_TOBOTTOM:
				gok_chunker_wraptobottom (rate); 
				break;

			case CALL_CHUNKER_WRAP_TOLEFT:
				gok_chunker_wraptoleft (rate); 
				break;

			case CALL_CHUNKER_WRAP_TORIGHT:
				gok_chunker_wraptoright (rate);
				break;

			case CALL_CHUNKER_WRAP_TOTOP:
				gok_chunker_wraptotop (rate); 
				break;

			case CALL_CHUNKER_MOVE_LEFTRIGHT:
gok_log_x ("chunker move leftright\n");
				if (gok_data_get_setting (pAccessMethod->Name, pEffect->pName, 
											&rate, NULL) == TRUE)
				{
					gok_chunker_move_leftright (rate); 
				}
				else
				{
					gok_log_x ("Can't get CHUNKER_MOVE_LEFTRIGHT name setting: %s!\n", pEffect->pName);
				}
				break;

			case CALL_CHUNKER_MOVE_TOPBOTTOM:
gok_log_x ("chunker move topbottom\n");
				if (gok_data_get_setting (pAccessMethod->Name, pEffect->pName, 
											&rate, NULL) == TRUE)
				{
					gok_chunker_move_topbottom (rate); 
				}
				else
				{
					gok_log_x ("Can't get CHUNKER_MOVE_TOPBOTTOM name setting: %s!\n", pEffect->pName);
				}
				break;

			case CALL_CHUNKER_IF_NEXT_CHUNK:
				EffectCodeReturned = gok_chunker_if_next_chunk(); 
				break;

			case CALL_CHUNKER_IF_PREVIOUS_CHUNK:
				EffectCodeReturned = gok_chunker_if_previous_chunk(); 
				break;

			case CALL_CHUNKER_IF_NEXT_KEY:
				EffectCodeReturned = gok_chunker_if_next_key(); 
				break;

			case CALL_CHUNKER_IF_PREVIOUS_KEY:
				EffectCodeReturned = gok_chunker_if_previous_key(); 
				break;

			case CALL_CHUNKER_IF_TOP:
				EffectCodeReturned = gok_chunker_if_top(); 
				break;

			case CALL_CHUNKER_IF_BOTTOM:
				EffectCodeReturned = gok_chunker_if_bottom(); 
				break;

			case CALL_CHUNKER_IF_LEFT:
				EffectCodeReturned = gok_chunker_if_left(); 
				break;

			case CALL_CHUNKER_IF_RIGHT:
				EffectCodeReturned = gok_chunker_if_right(); 
				break;

			case CALL_CHUNKER_IF_KEY_SELECTED:
				EffectCodeReturned = gok_chunker_if_key_selected(); 
				break;

			case CALL_CHUNKER_HIGHLIGHT_CENTER:
				gok_chunker_highlight_center_key(); 
				break;

			case CALL_CHUNKER_HIGHLIGHT_FIRST_CHUNK:
				gok_chunker_highlight_first_chunk(); 
				break;

			case CALL_CHUNKER_HIGHLIGHT_FIRST_KEY:
				gok_chunker_highlight_first_key(); 
				break;
			
			case CALL_SCANNER_REPEAT_ON:  
				gok_scanner_repeat_on();
				break;

			case CALL_CHUNKER_SELECT_CHUNK:
				EffectCodeReturned = gok_chunker_select_chunk();
				break;

			case CALL_TIMER1_SET:
				if (gok_scanner_get_multiple_rates (pAccessMethod->Name, pEffect->pName, 
											&rate) == TRUE)
				{
					EffectCodeReturned = gok_scanner_timer_set (rate, 1); 
				}
				else
				{
					gok_log_x ("Can't get timer 1 set setting: %s!\n", pEffect->pName);
				}
				break;

			case CALL_TIMER1_STOP:
				EffectCodeReturned = gok_scanner_timer_stop (1); 
				break;

			case CALL_TIMER2_SET:
				if (gok_scanner_get_multiple_rates (pAccessMethod->Name, pEffect->pName, 
											&rate) == TRUE)
				{
					EffectCodeReturned = gok_scanner_timer_set (rate, 2); 
				}
				else
				{
					gok_log_x ("Can't get timer 2 set setting: %s!\n", pEffect->pName);
				}
				break;

			case CALL_TIMER2_STOP:
				EffectCodeReturned = gok_scanner_timer_stop (2); 
				break;

			case CALL_TIMER3_SET:
				if (gok_scanner_get_multiple_rates (pAccessMethod->Name, pEffect->pName, 
											&rate) == TRUE)
				{
					EffectCodeReturned = gok_scanner_timer_set (rate, 3); 
				}
				else
				{
					gok_log_x ("Can't get timer 3 set setting: %s!\n", pEffect->pName);
				}
				break;

			case CALL_TIMER3_STOP:
				EffectCodeReturned = gok_scanner_timer_stop (3); 
				break;

			case CALL_TIMER4_SET:
				if (gok_scanner_get_multiple_rates (pAccessMethod->Name, pEffect->pName, 
											&rate) == TRUE)
				{
					EffectCodeReturned = gok_scanner_timer_set (rate, 4); 
				}
				else
				{
					gok_log_x ("Can't get timer 4 set setting: %s!\n", pEffect->pName);
				}
				break;

			case CALL_TIMER4_STOP:
				EffectCodeReturned = gok_scanner_timer_stop (4); 
				break;

			case CALL_TIMER5_SET:
				if (gok_scanner_get_multiple_rates (pAccessMethod->Name, pEffect->pName, 
											&rate) == TRUE)
				{
					EffectCodeReturned = gok_scanner_timer_set (rate, 5); 
				}
				else
				{
					gok_log_x ("Can't get timer 5 set setting: %s!\n", pEffect->pName);
				}
				break;

			case CALL_TIMER5_STOP:
				EffectCodeReturned = gok_scanner_timer_stop (5); 
				break;

			case CALL_COUNTER_SET:
				counterNumber = atoi (pEffect->CallParam1);
				if (gok_data_get_setting (pAccessMethod->Name, pEffect->CallParam2, 
												&param2Rate, NULL) == TRUE)
				{
					gok_chunker_counter_set (counterNumber, param2Rate); 
				}
				else	
				{
					gok_log_x ("Can't get CHUNKER_COUNTER_SET param2 setting: %s!\n", pEffect->pName);
				}
				break;

			case CALL_COUNTER_INCREMENT:
				gok_chunker_counter_increment (atoi (pEffect->CallParam1)); 
				break;

			case CALL_COUNTER_DECREMENT:
				gok_chunker_counter_decrement (atoi (pEffect->CallParam1)); 
				break;

			case CALL_COUNTER_GET:
				EffectCodeReturned = gok_chunker_counter_get (atoi (pEffect->CallParam1)); 
				break;

			case CALL_STATE_RESTART:
				gok_chunker_state_restart(); 
				break;

			case CALL_STATE_NEXT:
				gok_chunker_state_next(); 
				break;

			case CALL_STATE_JUMP:
				gok_chunker_state_jump (pEffect->CallParam1); 
				break;

			case CALL_OUTPUT_SELECTEDKEY:
				pKey = gok_keyboard_output_selectedkey ();
				if (gok_repeat_getArmed()){
					if (gok_repeat_key(pKey)) {
						gok_scanner_repeat_on();
						/* to fix problem with scanning methods we
						   must unfortunately cut the effect list short 
						   here. TODO revisit when repeatoff state is added
						   to xam files. */
						gok_log("leaving effect abruptly");
						gok_log_leave();
						return;
					}
				}
				EffectCodeReturned = 0;
				break;

			case CALL_SET_SELECTEDKEY:
				gok_feedback_set_selected_key (m_pKeyEntered); 
				break;

			case CALL_FEEDBACK:
				pAccessMethod = gok_scanner_get_current_access_method();
				g_assert (pAccessMethod != NULL);
				if (gok_data_get_setting (pAccessMethod->Name, pEffect->pName, 
											NULL, &pFeedbackName) == TRUE)
				{
					EffectCodeReturned = gok_feedback_perform_effect (pFeedbackName); 
				}
				else
				{
					gok_log_x ("Can't get feedback name!\n");
				}
				break;

			case CALL_CHUNKER_HIGHLIGHT_CHUNK:
				if (gok_data_get_setting (pAccessMethod->Name, pEffect->CallParam1, 
											&param1Rate, NULL) == TRUE)
				{
					gok_chunker_highlight_chunk_number (param1Rate); 
				}
				else
				{
					gok_log_x ("Can't get CHUNKER_HIGHLIGHT_CHUNK param1 setting: %s!\n", pEffect->pName);
				}
				break;

			case CALL_CHUNKER_UNHIGHLIGHT_ALL:
					gok_chunker_unhighlight_all_keys(); 
				break;

			case CALL_GET_RATE:
				if (gok_data_get_setting (pAccessMethod->Name, pEffect->pName, 
											&EffectCodeReturned, NULL) == FALSE)
				{
					gok_log_x ("Can't get rate: %s!", pEffect->pName);
					EffectCodeReturned = 0;
				}
				break;
				
			default:
				/* should not be here */
				gok_log_x ("Warning: hit default in gok_scanner_perform_effects!\n");
				break;
		}

		/* is there a condition to be checked? */
		if (pEffect->CompareType == COMPARE_EQUAL)
		{
			gok_log("compare ==");
			/* yes, compare the value returned to the compare value */
			if (EffectCodeReturned == pEffect->CompareValue)
			{
				gok_scanner_perform_effects (pEffect->pEffectTrue);
			}
			else
			{
				gok_scanner_perform_effects (pEffect->pEffectFalse);
			}
		}
		else if (pEffect->CompareType == COMPARE_LESSTHAN)
		{
			gok_log("compare <");
			if (EffectCodeReturned < pEffect->CompareValue)
			{
				gok_scanner_perform_effects (pEffect->pEffectTrue);
			}
			else
			{
				gok_scanner_perform_effects (pEffect->pEffectFalse);
			}
		}
		else if (pEffect->CompareType == COMPARE_GREATERTHAN)
		{
			gok_log("compare >");
			if (EffectCodeReturned > pEffect->CompareValue)
			{
				gok_scanner_perform_effects (pEffect->pEffectTrue);
			}
			else
			{
				gok_scanner_perform_effects (pEffect->pEffectFalse);
			}
		}
		else if (pEffect->CompareType == COMPARE_EQUALORLESSTHAN)
		{
			gok_log("compare <=");
			if (EffectCodeReturned <= pEffect->CompareValue)
			{
				gok_scanner_perform_effects (pEffect->pEffectTrue);
			}
			else
			{
				gok_scanner_perform_effects (pEffect->pEffectFalse);
			}
		}
		else if (pEffect->CompareType == COMPARE_EQUALORGREATERTHAN)
		{
			gok_log("compare >=");
			if (EffectCodeReturned >= pEffect->CompareValue)
			{
				gok_scanner_perform_effects (pEffect->pEffectTrue);
			}
			else
			{
				gok_scanner_perform_effects (pEffect->pEffectFalse);
			}
		}

		/* move on to the next effect in the handler */
		pEffect = pEffect->pEffectNext;
	}
	gok_log("leaving effect.");
	gok_log_leave();
}

/**
* gok_scanner_timer_set
* @Rate: Time in 100s of a second.
* @ID: Timer identifier
*
* Starts a timer.
*
* returns: Always 0.
**/
int gok_scanner_timer_set (gint Rate, gint ID)
{
	gint milliseconds;

	/* convert our rate (1/100 seconds) to milliseconds */
	milliseconds = Rate * 10;

	switch (ID)
	{
		case 1:
			gok_scanner_timer_stop (1);
			m_Timer1SourceId = g_timeout_add_full (G_PRIORITY_HIGH_IDLE, milliseconds, gok_scanner_on_timer1, NULL, NULL);
			m_bTimer1Started = TRUE;
			break;

		case 2:
			gok_scanner_timer_stop (2);
			m_Timer2SourceId = g_timeout_add_full (G_PRIORITY_HIGH_IDLE, milliseconds, gok_scanner_on_timer2, NULL, NULL);
			m_bTimer2Started = TRUE;
			break;

		case 3:
			gok_scanner_timer_stop (3);
			m_Timer3SourceId = g_timeout_add_full (G_PRIORITY_HIGH_IDLE, milliseconds, gok_scanner_on_timer3, NULL, NULL);
			m_bTimer3Started = TRUE;
			break;

		case 4:
			gok_scanner_timer_stop (4);
			m_Timer4SourceId = g_timeout_add_full (G_PRIORITY_HIGH_IDLE, milliseconds, gok_scanner_on_timer4, NULL, NULL);
			m_bTimer4Started = TRUE;
			break;

		case 5:
			gok_scanner_timer_stop (5);
			m_Timer5SourceId = g_timeout_add_full (G_PRIORITY_HIGH_IDLE, milliseconds, gok_scanner_on_timer5, NULL, NULL);
			m_bTimer5Started = TRUE;
			break;

		default:
			gok_log_x ("Warning: Default hit in gok_scanner_timer_set!\n");
			break;
	}

 	return 0;
}

/**
* gok_scanner_timer_set_dwell_rate
* @Rate: Dwell rate in 100s of a second.
*
* Sets the dwell rate.
**/
void gok_scanner_timer_set_dwell_rate (gint Rate)
{
	/* make sure rate is not zero */
	if (Rate == 0)
	{
		Rate = 100;
		gok_log_x ("Dwell rate is zero");
	}
	
	m_dwellrate = Rate * 10;
}

/**
* gok_scanner_timer_start_dwell
*
* Starts the dwell timer.
**/
void gok_scanner_timer_start_dwell ()
{
	if (m_dwellrate != 0)
	{
		m_DwellTimerId = g_timeout_add_full (G_PRIORITY_HIGH_IDLE, m_dwellrate, gok_scanner_timer_on_dwell, NULL, NULL);
		m_bDwellTimerStarted = TRUE;
	}	
}

/**
* gok_scanner_timer_stop_dwell
*
* Stops the dwell timer
**/
void gok_scanner_timer_stop_dwell ()
{
	if (m_bDwellTimerStarted == TRUE)
	{
		g_source_remove (m_DwellTimerId);
	}
}

/**
* gok_scanner_timer_on_dwell
* @data: Passed from the event. Ignored.
*
* This will be called when the dwell timer counts down.
*
* Returns: Always FALSE.
**/
gboolean gok_scanner_timer_on_dwell (gpointer data)
{
	gok_scanner_perform_effects (m_pEffectsOnDwell);
	return FALSE;
}

/**
* gok_scanner_timer_stop
* @TimerId: Id of the timer that will be stopped.
*
* Stops a timer.
*
* returns: Always 0.
**/
int gok_scanner_timer_stop (gint TimerId)
{
	switch (TimerId)
	{
		case 1:
			if (m_bTimer1Started == TRUE)
			{
				g_source_remove (m_Timer1SourceId);
			}
			break;

		case 2:
			if (m_bTimer2Started == TRUE)
			{
				g_source_remove (m_Timer2SourceId);
			}
			break;

		case 3:
			if (m_bTimer3Started == TRUE)
			{
				g_source_remove (m_Timer3SourceId);
			}
			break;

		case 4:
			if (m_bTimer4Started == TRUE)
			{
				g_source_remove (m_Timer4SourceId);
			}
			break;

		case 5:
			if (m_bTimer5Started == TRUE)
			{
				g_source_remove (m_Timer5SourceId);
			}
			break;

		default:
			gok_log_x ("Warning: Default hit in gok_scanner_timer_stop!\n");
			break;
	}

	return 0;
}

/**
* gok_scanner_update_rates
*
* Updates all the rates in all access methods from the GokData.
**/
void gok_scanner_update_rates (void)
{
	GokAccessMethod* pAccessMethod;
	GokAccessMethodRate* pRate;
	gint ValueNew;
	gchar* pValueStringNew;

	/* loop through all access methods */
	pAccessMethod = m_pAccessMethodFirst;
	while (pAccessMethod != NULL)
	{
		/* get the value for each rate in the access method */
		pRate = pAccessMethod->pRateFirst;
		while (pRate != NULL)
		{
			/* does the GokData have a value for this rate? */
			if (gok_data_get_setting (pAccessMethod->Name, pRate->Name, &ValueNew, &pValueStringNew) == TRUE)
			{

				/* yes, change our value to the value from the GokData */
				pRate->Value = ValueNew;

#if 0
/* writes the rates in the XML file to Gconf */
if (pRate->StringValue != NULL)
	gok_data_create_setting (pAccessMethod->Name, pRate->Name, pRate->ValueDefault, pRate->StringValue);
#endif
				if (pRate->StringValue != NULL)
				{
					g_free (pRate->StringValue);
				}
				if (pValueStringNew == NULL)
				{
					pRate->StringValue = NULL;
				}
				else
				{
					pRate->StringValue = (gchar*)g_malloc (strlen (pValueStringNew) + 1);
					strcpy (pRate->StringValue, pValueStringNew);
				}
			}
			else
			{
				/* no, tell GokData to create a new rate */
				gok_data_create_setting (pAccessMethod->Name, pRate->Name, pRate->Value, pRate->StringValue);
			}
			pRate = pRate->pRateNext;
		}
	
		/* move on to the next access method in the list */	
		pAccessMethod = pAccessMethod->pAccessMethodNext;
	}
}

/**
* gok_scanner_get_current_access_method
*
* Accessor function to get the current access method.
*
* returns: A pointer to the current access method.
**/
GokAccessMethod* gok_scanner_get_current_access_method ()
{
	return m_pAccessMethodCurrent;
}

/**
* gok_scanner_get_current_first_method
*
* Accessor function to get the first access method.
*
* returns: A pointer to the first access method.
**/
GokAccessMethod* gok_scanner_get_first_access_method ()
{
	return m_pAccessMethodFirst;
}

/**
* gok_scanner_get_current_state
*
* Accessor function to get the current handler state.
*
* returns: A pointer to the current handler state.
**/
GokScannerState* gok_scanner_get_current_state ()
{
	return m_pStateCurrent;
}


/**
 * gok_scanner_current_state_uses_corepointer:
 *
 * Returns: TRUE if the current scanner state has a handler that is
 * currently configured to use an action which is connected to the corepointer
 * (button or dwell), and FALSE otherwise.
 */
gboolean 
gok_scanner_current_state_uses_corepointer (void)
{
	gboolean retval = FALSE;
	GokAccessMethod *method;

	if  (m_pEffectsLeftButtonDown ||
	     m_pEffectsLeftButtonUp ||
	     m_pEffectsRightButtonDown ||
	     m_pEffectsRightButtonUp ||
	     m_pEffectsMiddleButtonDown ||
	     m_pEffectsMiddleButtonUp ||
	     m_pEffectsMouseButton4Down ||
	     m_pEffectsMouseButton4Up ||
	     m_pEffectsMouseButton5Down ||
	     m_pEffectsMouseMovement) {
		retval = TRUE;
	}
	/* FIXME! hack, assume we don't use corepointer if we have a valid input device */
	/* This is also inefficient since we don't cache the result */
	else {
		char *input_device_name = NULL;
		gok_gconf_get_string (gconf_client_get_default (), 
				      GOK_GCONF_INPUT_DEVICE, 
				      &input_device_name);

		if (input_device_name == NULL)
			retval = TRUE;
		else
			g_free (input_device_name);
		
		if (retval && (gok_main_get_inputdevice_name () != NULL))
			retval = FALSE;
	}

	return retval;
}


/**
 * gok_scanner_current_state_uses_core_mouse_button:
 * @button:
 *
 * Returns: TRUE if the current scanner state has a handler that is
 * currently configured to use an action that uses the core mouse
 * button button and FALSE otherwise.
 */
gboolean 
gok_scanner_current_state_uses_core_mouse_button(int button)
{
	switch (button) {
		case 1:
			if (   (m_pEffectsLeftButtonDown != NULL)
			    || (m_pEffectsLeftButtonUp   != NULL) )
				return TRUE;
			else
				return FALSE;
			break;
		case 2:
			if (   (m_pEffectsMiddleButtonDown != NULL)
			    || (m_pEffectsMiddleButtonUp   != NULL) )
				return TRUE;
			else
				return FALSE;
			break;
		case 3:
			if (   (m_pEffectsRightButtonDown != NULL)
			    || (m_pEffectsRightButtonUp   != NULL) )
				return TRUE;
			else
				return FALSE;
			break;
		case 4:
			if (   (m_pEffectsMouseButton4Down != NULL)
			    || (m_pEffectsMouseButton4Up   != NULL) )
				return TRUE;
			else
				return FALSE;
			break;
		case 5:
			if (   (m_pEffectsMouseButton5Down != NULL)
			    || (m_pEffectsMouseButton5Up   != NULL) )
				return TRUE;
			else
				return FALSE;
			break;
	}

	return FALSE;
}

/**
* gok_scanner_make_type_from_string
* @pString: Pointer to the string that describes one or more effect types.
*
* returns: An int that describes all the effect types in the string.
**/
gint gok_scanner_make_type_from_string (gchar* pString)
{
	gint codeReturned;
	gchar* pToken;
	gchar buffer[150];
	
	g_assert (pString != NULL);
	g_assert (strlen (pString) < 150 );

	codeReturned = 0;
	strcpy (buffer, pString);
	pToken = strtok (buffer, "+");
	while (pToken != NULL)
	{
		if (strcmp (pToken, "switch") == 0)
		{
			codeReturned |= ACTION_TYPE_SWITCH;
		}
		else if (strcmp (pToken, "mousebutton") == 0)
		{
			codeReturned |= ACTION_TYPE_MOUSEBUTTON;
		}
		else if (strcmp (pToken, "mousepointer") == 0)
		{
			codeReturned |= ACTION_TYPE_MOUSEPOINTER;
		}
		else if (strcmp (pToken, "dwell") == 0)
		{
			codeReturned |= ACTION_TYPE_DWELL;
		}
				
		pToken = strtok (NULL, "+");
	}
	
	return codeReturned;
}

/**
* gok_scanner_get_multiple_rates
* @NameAccessMethod: Name of the access method that has the rates.
* @NameSettings: Name of the settings. There may be multiple setting names,
* seperated by a '+'.
* @Value: Pointer to the gint that will be populated with the settings value.
*
* Gets a value that contains the combination of one or more rates.
*
* returns: TRUE if the rate was retreived, FALSE if not.
**/
gboolean gok_scanner_get_multiple_rates (gchar* NameAccessMethod, gchar* NameSettings, gint* Value)
{
	gint rate;
	gint rateTotal;
	gchar* pToken;
	gchar buffer[150];
	
	g_assert (NameAccessMethod != NULL);
	g_assert (NameSettings != NULL);
	g_assert (Value != NULL);
	
	rateTotal = 0;
	*Value = 0;

	strcpy (buffer, NameSettings);
	pToken = strtok (buffer, "+");
	while (pToken != NULL)
	{
		if (gok_data_get_setting (NameAccessMethod, pToken, &rate, NULL) == FALSE)
		{
			*Value = rateTotal;
			return FALSE;
		}
		rateTotal += rate;
				
		pToken = strtok (NULL, "+");
	}
	
	*Value = rateTotal;
	
	return TRUE;
}


/**
* gok_scanner_repeat_on
*
* Changes the state of the current access method so that the user can
* stop the repeating with an action.
*
* returns: void.
**/
void 
gok_scanner_repeat_on(void)
{
	GokAccessMethod* pAccessMethod;
	GokScannerState* pState;
	
	gok_log_enter();
	pAccessMethod = gok_scanner_get_current_access_method();
	g_assert (pAccessMethod != NULL);
	pState = pAccessMethod->pStateFirst;
	while (pState != NULL)
	{
		if ( strcmp(pState->NameState, REPEAT_OFF_NAME) == 0 )
		{
			gok_scanner_change_state (pState, pAccessMethod->Name);
			break; /* from while */
		}
		pState = pState->pStateNext;
	}
	gok_log_leave();
}

void
gok_scanner_drop_refs (GokKey *pKey)
{
  if (m_pKeyEntered == pKey) m_pKeyEntered = NULL;
  if (m_pContainingKey == pKey) m_pContainingKey = NULL;
}
