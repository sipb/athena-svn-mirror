/*
* gok-spy.c
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

/*
 * utility for getting accessible application UI info
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <libspi/keymasks.h>
#include "main.h"
#include "gok-spy.h"
#include "gok-log.h"
#include "gok-modifier.h"
#include "gok-data.h"
#include "gok-modifier-keymasks.h"
#include "gok-gconf-keys.h"
#include "gok-word-complete.h"
#include "gok-keyboard.h"

/*
 * NOTE: we can tweak the efficiency/accuracy of the
 * gui search by adjusting using the spy/gui_search_depth and 
 * spy/gui_search_breadth gconf keys.
 */

/*
 * NOTES
 *
 * the at-spi currently requires Accessible_ref and Accessible_unref
 * calls to keep accessible pointers valid.  
 *
 * gok-spy assumes only onelistener is registered at a time.
 *
 */

/*
 * private prototypes
 */
static void gok_spy_focus_listener (const AccessibleEvent *event,
                                    void *user_data);

static void gok_spy_defunctness_listener (const AccessibleEvent *event,
                                    void *user_data);

static void gok_spy_process_focus (Accessible* accessible);

static SPIBoolean gok_spy_mouse_listener (const AccessibleDeviceEvent *event,
					  void *user_data);

SPIBoolean gok_spy_key_listener (const AccessibleKeystroke *key,
                                 void *user_data);

static void gok_spy_modifier_listener(const AccessibleEvent *event,
                                      void *user_data);

static gboolean gok_spy_worth_searching (Accessible* accessible);

static void gok_spy_window_activate_listener (const AccessibleEvent *event,
                                              void *user_data);

static void gok_spy_window_deactivate_listener (const AccessibleEvent *event,
                                                void *user_data);
												
static void gok_spy_object_state_listener (const AccessibleEvent *event,
                                                void *user_data);

static gboolean gok_spy_is_visible (Accessible *accessible);
		
static void gok_spy_free_nodes (GSList *nodelist);										
static void gok_spy_resolve_namesakes (GSList *nodelist);

static gboolean gok_spy_is_primary_container (Accessible *accessible);
									
/*
 * gok-spy variables
 */

static GQueue* eventQueue;
static void* m_ptheAppChangeListener;
static void* m_ptheWindowChangeListener;
static void* m_ptheMouseButtonListener;
static void* m_ptheStateChangeListener;

static GSList* _priv_ui_nodes = NULL;

/* pointer to the current window */
static Accessible* m_ptheWindowAccessible;

/* pointer to the current accessible with editable text */
static Accessible* m_ptheAccessibleWithText = NULL;

static Accessible* theContextMenuAccessible = NULL;

static Accessible* currently_focussed_object = NULL;

static AccessibleEventListener* focusListener;
static AccessibleEventListener* defunctnessListener;
static AccessibleDeviceListener* mouseListener;
static AccessibleEventListener* modifierListener;
static AccessibleEventListener* windowActivateListener;
static AccessibleEventListener* windowDeactivateListener;
static AccessibleEventListener* objectStateListener;
static gboolean m_gokSpyOpen = FALSE;
static gint explicitrefs;
static gint explicitunrefs;
static gint implicitrefs;
static gulong keyboardmods;
static gboolean spyshutdown = FALSE;/* this global could be removed if we have a
						way of removing our idle handler in gok_spy_stop */

#define MAX_CHILD_COUNT 100


static void
gok_spy_set_focussed_object (Accessible *accessible)
{
    if (currently_focussed_object) 
    {
	gok_spy_accessible_unref (currently_focussed_object);
    }
    currently_focussed_object = accessible;
    if (accessible) 
    {
	gok_spy_accessible_ref (accessible);
    }
}

/*
 * gok_spy_get_focussed_object:
 *
 * returns: the currently focussed object, or NULL if no object has emitted a focus event.
 */
Accessible *
gok_spy_get_focussed_object (void)
{
    return currently_focussed_object;
}

/* 
 * for debugging
 */
void gok_spy_accessible_ref(Accessible* accessible)
{
	char *s;

	gok_log_enter ();

	if (accessible != NULL)
	{
		explicitrefs++;
		Accessible_ref (accessible);
#if defined(ENABLE_LOGGING_NORMAL)
		s = Accessible_getName (accessible);
		if (s == NULL)
		{
			gok_log ("%#x", accessible);
		}
		else
		{
			gok_log ("%#x %s", accessible, s);
			SPI_freeString (s);
		}
#endif
	}
	else
	{
		gok_log ("NULL");
	}

	gok_log_leave ();
}

void gok_spy_accessible_implicit_ref(Accessible* accessible)
{
	char *s;

	gok_log_enter ();

	if (accessible != NULL)
	{
		implicitrefs++;
#if defined(ENABLE_LOGGING_NORMAL)
		s = Accessible_getName (accessible);
		if (s == NULL)
		{
			gok_log ("%#x", accessible);
		}
		else
		{
			gok_log ("%#x %s", accessible, s);
			SPI_freeString (s);
		}
#endif
	}
	else
	{
		gok_log ("NULL");
	}

	gok_log_leave ();
}

void gok_spy_accessible_unref(Accessible* accessible)
{
	char *s;

	gok_log_enter ();

	if (accessible != NULL)
	{
#if defined(ENABLE_LOGGING_NORMAL)
		s = Accessible_getName (accessible);
		if (s == NULL)
		{
			gok_log ("%#x", accessible);
		}
		else
		{
			gok_log ("%#x %s", accessible, s);
			SPI_freeString (s);
		}
#endif
		explicitunrefs++;
		Accessible_unref (accessible);
	}
	else
	{
		gok_log ("NULL");
	}

	gok_log_leave ();
}

extern SPIBoolean cspi_accessible_is_a (Accessible *accessible, const char *interface_name);

gboolean gok_spy_accessible_is_okay(Accessible* accessible)
{
	gboolean returncode = FALSE;
	gok_log_enter();
	if (accessible != NULL) 
	{
		/* FIXME: we should have a better way of doing this! */
		returncode = cspi_accessible_is_a (accessible, "IDL:Accessibility/Accessible:1.0");
		/*
		if (Accessible_isApplication(accessible) ||
		    Accessible_isComponent(accessible))
		{
			returncode = TRUE;
		}
		*/
			/*
	    	if (gok_spy_accessible_is_desktopChild(accessible) == FALSE)
	    	{
				accessible = NULL;
			}
			else
			{
				returncode = TRUE;
			}
			*/
	}
	gok_log("returning: %d",returncode);
	gok_log_leave();
	return returncode;
}

static gboolean
gok_spy_idle_handler (gpointer data)
{
	return gok_spy_check_queues ();
}


/**
 * gok_spy_open:
 *
 * Intializes gok spy.
 * Note: user must initialize the SPI prior to this call; call this only once.
 */
void gok_spy_open()
{
	gboolean success = FALSE;

	gok_log_enter ();
	if (m_gokSpyOpen != TRUE)
	{
		eventQueue = g_queue_new();
		explicitunrefs = 0;
		explicitrefs = 0;
		implicitrefs = 0;
		m_gokSpyOpen = TRUE;
		m_ptheMouseButtonListener =NULL;
		m_ptheAppChangeListener = NULL;
		m_ptheWindowChangeListener = NULL;
		m_ptheStateChangeListener = NULL;
		m_ptheWindowAccessible = NULL;
		m_ptheAccessibleWithText = NULL;

		focusListener = SPI_createAccessibleEventListener (
			gok_spy_focus_listener, NULL);
		defunctnessListener = SPI_createAccessibleEventListener (
			gok_spy_defunctness_listener, NULL);
		mouseListener = SPI_createAccessibleDeviceListener (
			gok_spy_mouse_listener, NULL);
		modifierListener = SPI_createAccessibleEventListener (
			gok_spy_modifier_listener, NULL);
		windowActivateListener = SPI_createAccessibleEventListener (
			gok_spy_window_activate_listener, NULL);
		windowDeactivateListener = SPI_createAccessibleEventListener (
			gok_spy_window_deactivate_listener, NULL);
		objectStateListener = SPI_createAccessibleEventListener (
			gok_spy_object_state_listener, NULL);

		success = SPI_registerGlobalEventListener (focusListener, "focus:"); 
		gok_log ("SPI_registerGlobalEventListener for focus events returned: %d" , success);
		success = SPI_registerGlobalEventListener (defunctnessListener, "object:state-changed:defunct");
		gok_log ("SPI_registerGlobalEventListener for defunct state events returned: %d" , success); 
		success = SPI_registerDeviceEventListener (mouseListener,
			SPI_BUTTON_PRESSED | SPI_BUTTON_RELEASED, NULL);
		gok_log ("SPI_registerDeviceEventListener for mouse events returned: %d" , success);

		success = SPI_registerGlobalEventListener (modifierListener,
			"keyboard:modifiers");
		gok_log ("SPI_registerGlobalEventListener for keyboard modifiers returned: %d", success);

		success = SPI_registerGlobalEventListener (
			windowActivateListener, "window:activate");
		gok_log ("SPI_registerGlobalEventListener for window:activate returned: %d", success);

		success = SPI_registerGlobalEventListener (
			windowDeactivateListener, "window:deactivate");
		gok_log ("SPI_registerGlobalEventListener for window:deactivate returned: %d", success);

		success = SPI_registerGlobalEventListener (
			objectStateListener, "object:state-changed");
		gok_log ("SPI_registerGlobalEventListener for object:state-changed returned: %d", success);

		success = SPI_registerGlobalEventListener (
			objectStateListener, "object:children-changed");
		gok_log ("SPI_registerGlobalEventListener for object:children-changed returned: %d", success);

	}
	gok_log_leave ();
}

/** 
 * gok_spy_stop:
 *
 * Call this to unhook all the gok_spy internal listeners. Do this as as the
 * first thing when shutting down the application.
 */
void gok_spy_stop (void)
{
	gboolean result = FALSE;	
	
	spyshutdown = TRUE;
	
	if (focusListener != NULL) 
	{
		result = SPI_deregisterGlobalEventListenerAll(focusListener);
		gok_log("deregistering focus listener returned: %d",result);
		AccessibleEventListener_unref(focusListener);
		focusListener = NULL;
	}
	if (defunctnessListener != NULL) 
	{
		result = SPI_deregisterGlobalEventListenerAll(defunctnessListener);
		gok_log("deregistering defunctness listener returned: %d",result);
		AccessibleEventListener_unref(defunctnessListener);
		defunctnessListener = NULL;
	}
	if (mouseListener != NULL)
	{
		result = SPI_deregisterDeviceEventListener (mouseListener, NULL);
		gok_log("deregistering mouse listener returned: %d",result);
		AccessibleDeviceListener_unref(mouseListener);
		mouseListener = NULL;
	}
	if (modifierListener != NULL)
	{
		result = SPI_deregisterGlobalEventListenerAll (modifierListener);
		gok_log("deregistering modifier listener returned: %d",	result);
		AccessibleEventListener_unref(modifierListener);
		modifierListener = NULL;
	}
	if (windowActivateListener != NULL)
	{
		result = SPI_deregisterGlobalEventListenerAll (windowActivateListener);
		gok_log("deregistering window activate listener returned: %d",result);
		AccessibleDeviceListener_unref(windowActivateListener);
		windowActivateListener = NULL;
	}
	if (windowDeactivateListener != NULL)
	{
		result= SPI_deregisterGlobalEventListenerAll (windowDeactivateListener);
		gok_log("deregistering window deactivate listener returned: %d",result);
		AccessibleEventListener_unref(windowDeactivateListener );
		windowDeactivateListener = NULL;
	}
	if (objectStateListener != NULL)
	{
		result= SPI_deregisterGlobalEventListenerAll (objectStateListener);
		gok_log("deregistering object state change listener returned: %d",result);
		AccessibleEventListener_unref(objectStateListener );
		objectStateListener = NULL;
	}
}
	
/** 
 * gok_spy_close:
 *
 * Frees any allocated memory.
 */
void gok_spy_close(void)
{
	EventNode* en;
	
	gok_log_enter ();

	if (m_gokSpyOpen == TRUE)
	{
		m_gokSpyOpen = FALSE;
		
		gok_spy_stop ();

		gok_spy_free_nodes (_priv_ui_nodes);
		
		if (m_ptheWindowAccessible != NULL) gok_spy_accessible_unref(m_ptheWindowAccessible);
		if (m_ptheAccessibleWithText != NULL) gok_spy_accessible_unref(m_ptheAccessibleWithText);
		if (theContextMenuAccessible != NULL) gok_spy_accessible_unref(theContextMenuAccessible);

		gok_log("accessible reference balance is  [%d] (should be zero)",explicitrefs + implicitrefs - explicitunrefs);
		gok_log("explicit accessible references   [%d]",explicitrefs);
		gok_log("explicit accessible dereferences [%d]",explicitunrefs);
		gok_log("implicit accessible references   [%d]",implicitrefs);

		gok_log("eventQueue->length = %d", eventQueue->length);

		/* iterate over event queue and dereference accessibles */
		while (g_queue_is_empty(eventQueue) == FALSE)
		{
			en = (EventNode*)g_queue_pop_head(eventQueue);
			AccessibleEvent_unref (en->event);
			g_free(en);
		}
		g_queue_free(eventQueue);
		eventQueue = NULL;
	}

	gok_log ("Exiting SPI mainloop.\n");
	SPI_exit ();

	gok_log_leave ();
}


/** 
* gok_spy_register_appchangelistener
* 
* @callback: the listener to register
**/ 
void gok_spy_register_appchangelistener(AccessibleChangeListener* callback)
{
	gok_log_x("this function deprecated, use gok_spy_register_windowchangelistener instead");
	/* m_ptheAppChangeListener = callback; */
}

/** 
* gok_spy_deregister_appchangelistener
* 
* @callback: the listener to deregister
**/ 
void gok_spy_deregister_appchangelistener(AccessibleChangeListener* callback)
{
	gok_log_x("this function deprecated, use gok_spy_deregister_windowchangelistener instead");
	/* m_ptheAppChangeListener = NULL; */
}

/** 
* gok_spy_register_windowchangelistener
* 
* @callback: the listener to register
**/ 
void gok_spy_register_windowchangelistener(AccessibleChangeListener* callback)
{
	m_ptheWindowChangeListener = callback;
}

/** 
* gok_spy_deregister_windowchangelistener
* 
* @callback: the listener to deregister
**/ 
void gok_spy_deregister_windowchangelistener(AccessibleChangeListener* callback)
{
	m_ptheWindowChangeListener = NULL;
}


/** 
* gok_spy_register_objectstatelistener
* 
* Will only notify if the state change is in the correct context. The context
* is this: the state change must be happening within the currently focused
* application, otherwise the callback is not notified.
*
* @callback: the listener to register
**/ 
void gok_spy_register_objectstatelistener(AccessibleChangeListener* callback)
{
	m_ptheStateChangeListener = callback;
}

/** 
* gok_spy_deregister_objectstatelistener
* 
* @callback: the listener to deregister
**/ 
void gok_spy_deregister_objectstatelistener(AccessibleChangeListener* callback)
{
	m_ptheStateChangeListener = NULL;
}


/** 
* gok_spy_register_mousebuttonlistener
* 
* @callback: the listener to register
**/ 
void gok_spy_register_mousebuttonlistener(MouseButtonListener* callback)
{
	m_ptheMouseButtonListener = callback;
}

/** 
* gok_spy_deregister_mousebuttonlistener
* 
* @callback: the listener to deregister
**/ 
void gok_spy_deregister_mousebuttonlistener(MouseButtonListener* callback)
{
	m_ptheMouseButtonListener = NULL;
}

/**
 * gok_spy_node_match:
 *
 * 
 * Returns: TRUE if the node matches search type @type, FALSE otherwise.
 **/
gboolean
gok_spy_node_match (AccessibleNode *node, GokSpySearchType type)
{
	switch (type)
	{
	case GOK_SPY_SEARCH_MENU:
		return node->flags.data.is_menu;
		break;
	case GOK_SPY_SEARCH_TOOLBARS:
		return node->flags.data.is_toolbar_item;
		break;
	case GOK_SPY_SEARCH_UI:
		return node->flags.data.is_ui;
		break;
	case GOK_SPY_SEARCH_EDITABLE_TEXT:
		if (Accessible_isEditableText (node->paccessible))
			return TRUE;
		break;
	case GOK_SPY_SEARCH_CHILDREN:
	case GOK_SPY_SEARCH_COMBO:
	case GOK_SPY_SEARCH_LISTITEMS:
	case GOK_SPY_SEARCH_TABLE_CELLS:
	case GOK_SPY_SEARCH_ALL:
		return TRUE;
		break;
	case GOK_SPY_SEARCH_ACTIONABLE:
		if (Accessible_isAction (node->paccessible))
		{
			return TRUE;
		}
		else 
		{
		    AccessibleStateSet *state = Accessible_getStateSet (node->paccessible);
		    if (AccessibleStateSet_contains (state, SPI_STATE_SELECTABLE))
		    {
			AccessibleStateSet_unref (state);
			return TRUE;
		    }
		    AccessibleStateSet_unref (state);
		}
		break;
	case GOK_SPY_SEARCH_APPLICATIONS:
		if (Accessible_isApplication (node->paccessible))
			return TRUE;
		break;
	default:
		break;
	}
	return FALSE;
}

/**
 *
 * gok_spy_get_editable:
 *
 * Returns the first editable-text child of @search_root.
 * Used in particular to locate the text-entry child of editable comboboxes.
 **/
Accessible *
gok_spy_get_editable (Accessible *search_root)
{
    Accessible *child = NULL;

    /* TODO: complete the logic below, to search the whole tree from the root, 
       not just the immediate children */
    gok_spy_accessible_ref (search_root);
    while (search_root) 
    {
	int i = 0;
	int child_count = Accessible_getChildCount (search_root);
	while (i < child_count) 
	{
	    AccessibleRole role = SPI_ROLE_INVALID;
	    child = Accessible_getChildAtIndex (search_root, i);
	    if (child) 
	    {
		gok_spy_accessible_implicit_ref (child);
		if (Accessible_isEditableText (child)) 
		{
		    return child;
		}
		else 
		{
		    gok_spy_accessible_unref (child);
		}
	    }
	    ++i;
	}
	gok_spy_accessible_unref (search_root);
	search_root = NULL;
    }
    return NULL;
}

/**
 *
 * gok_spy_get_list_parent:
 *
 * Returns the parent subelement of @search_root whose children are list items.
 * Used in particular to enumerate the list children of comboboxes, since the list
 * items are further down the tree than the 'combobox' ancestor.
 **/
Accessible *
gok_spy_get_list_parent (Accessible *search_root)
{
    Accessible *child = NULL;
    gok_spy_accessible_ref (search_root);
    while (search_root) 
    {
	if (Accessible_getChildCount (search_root) > 0) 
	{
	    AccessibleRole role = SPI_ROLE_INVALID;
	    child = Accessible_getChildAtIndex (search_root, 0);
	    if (child) 
	    {
		gok_spy_accessible_implicit_ref (child);
		role = Accessible_getRole (child);
		if (role == SPI_ROLE_LIST_ITEM
		    || role == SPI_ROLE_MENU_ITEM
		    || role == SPI_ROLE_RADIO_MENU_ITEM
		    || role == SPI_ROLE_CHECK_MENU_ITEM)
		{
		    gok_spy_accessible_unref (child);
		    return search_root;
		}
		else 
		{
		    gok_spy_accessible_unref (search_root);
		    search_root = child;
		}
	    }
	}
	else 
	{
	    gok_spy_accessible_unref (search_root);
	    search_root = NULL;
	}
    }
    return NULL;
}

static gchar*
gok_spy_concatenate_child_names (Accessible *parent)
{
	gint num_children = 0;
	gchar *tmp = NULL, *name = NULL;
	if (parent) 
	{
		gint i;
		num_children = Accessible_getChildCount (parent);
		for (i = 0; i < num_children; ++i) 
		{
			Accessible *child = Accessible_getChildAtIndex (parent, i);
			if (child ) 
			{
				gchar *childname = Accessible_getName (child);
				if (childname && (strlen (childname) > 0)) 
				{
					if (tmp)
					{
						name = g_strconcat (tmp, "·", childname, NULL);
						g_free (tmp);
					}
					else
					{
						name = g_strdup (childname);
					}
					SPI_freeString (childname);
					tmp = name;
				}
			}
		}
	}
	return name;
}

/**
 * 
 * gok_spy_get_table_nodes:
 * Returns a list of nodes which are children of a table, such that
 * there is exactly one child per currently visible cell 
 * plus one child per actionable table header.
 *
 **/
GSList *
gok_spy_get_table_nodes (Accessible *search_root)
{
	GSList *nodes = NULL;
	AccessibleNodeFlags flags;
	AccessibleTable *table;
	Accessible *child, *header, *first_child, *last_child;
	AccessibleComponent *component;
	long row_count, col_count;
	long i, first_row, last_row, first_index = 0, last_index = -1;
	long x, y, width, height;

	g_assert (Accessible_isTable (search_root));
	table = Accessible_getTable (search_root);
	flags.value = ~0;
	flags.data.inside_html_container = FALSE;
	col_count = AccessibleTable_getNColumns (table);
	if (col_count > 1)
	{
		for (i = 0; i < col_count; ++i)
		{
			header = AccessibleTable_getColumnHeader (table, i);
			if (header)
			{
				gok_spy_accessible_implicit_ref (header);
				nodes = gok_spy_append_node (nodes, header, flags);
				gok_spy_accessible_unref (header);
			}
		}
	}

	component = Accessible_getComponent (table);
	if (!component) return NULL;

	AccessibleComponent_getExtents (component, &x, &y, &width, &height, SPI_COORD_TYPE_SCREEN);
	first_child = AccessibleComponent_getAccessibleAtPoint (component, x, y, SPI_COORD_TYPE_SCREEN);
	last_child = AccessibleComponent_getAccessibleAtPoint (component, x + width, y + height, SPI_COORD_TYPE_SCREEN);
	AccessibleComponent_unref (component);

	if (first_child) 
		first_index = Accessible_getIndexInParent (first_child);
	if (last_child) 
		last_index = Accessible_getIndexInParent (last_child);
	Accessible_unref (first_child);
	Accessible_unref (last_child);

	gok_log ("first row index %d; last row index %d", first_index, last_index);

	row_count = AccessibleTable_getNRows (table);
	first_row = AccessibleTable_getRowAtIndex (table, first_index);
	if (last_index >= 0) 
		last_row = AccessibleTable_getRowAtIndex (table, last_index);
	else 
		last_row = row_count - 1;

	last_row = MIN (last_row, row_count - 1);
	first_row = MAX (first_row, 0);
	
	gok_log ("adding nodes for rows %d through %d (inclusive)\n", first_row, last_row);

	for (i = first_row; i <= last_row; ++i)
	{
		gint n = 0;
		/* make sure the row cell we choose is not anonymous */
		/* assignment operator on child ("=" vs "==") is intentional */
		while ((child = AccessibleTable_getAccessibleAt (table, i, n)) && (n < col_count))
		{
			char *name = Accessible_getName (child);
			char *desc = Accessible_getDescription (child);
			gboolean has_name = (name && (strlen (name) > 0)), has_desc = (desc && (strlen (desc) > 0));

			gok_spy_accessible_implicit_ref (child);
			if (name) SPI_freeString (name);
			if (desc) SPI_freeString (desc);
			/* we make exception for compound cells, which will scavenge a name from their children */
			if (has_name || has_desc || (Accessible_getChildCount (child) > 0))
			{
				nodes = gok_spy_append_node (nodes, child, flags);
				gok_spy_accessible_unref (child);
			}
			else 
			{
 			        gok_log ("ignoring node at row %d, col %d", i, n);
				gok_spy_accessible_unref (child);
			}
			++n;
		}
	}
	return nodes;
}

/**
 * 
 * gok_spy_get_children:
 *
 **/
GSList *
gok_spy_get_children (Accessible *search_root)
{
	GSList *nodes = NULL;
	AccessibleNodeFlags flags;
	Accessible *child;
	long child_count;
	long i;

	g_assert (search_root);
	/* TODO: should this recurse down, and should we check for 'interesting'-ness first ? */
	flags.value = ~0;
	flags.data.inside_html_container = FALSE;
	child_count = Accessible_getChildCount (search_root);
	for (i = 0; i < child_count; ++i)
	{
		child = Accessible_getChildAtIndex (search_root, i);
		if (child) 
		{
			gok_spy_accessible_implicit_ref (child);
			nodes = gok_spy_append_node (nodes, child, flags);
			gok_spy_accessible_unref (child);
		}
	}
	return nodes;
}

/**
 * gok_spy_get_actionable_descendants:
 * Returns a list of actionable or selectable descendants of #accessible,
 * including #accessible itself.
 **/
GSList *
gok_spy_get_actionable_descendants (Accessible *accessible, GSList *nodes)
{
    gint child_count, i, max_children = 20;
    Accessible *child;
    AccessibleNodeFlags flags;
    GSList *ret_nodes;
    gboolean is_selection = Accessible_isSelection (accessible);
    flags.value = 0;
    flags.data.inside_html_container = FALSE;
    flags.data.is_ui = TRUE;

    if (Accessible_isAction (accessible))
    {
	ret_nodes = gok_spy_append_node (nodes, accessible, flags);
    }
    else
    {
	ret_nodes = nodes;
    }
    child_count = Accessible_getChildCount (accessible);
    for (i = 0; i < child_count && i < max_children; ++i) 
    {
	AccessibleStateSet *stateset;
	child = Accessible_getChildAtIndex (accessible, i);
	stateset = Accessible_getStateSet (child);
	if (Accessible_isAction (child) && AccessibleStateSet_contains (stateset, SPI_STATE_SHOWING))
	{
	    ret_nodes = gok_spy_append_node (ret_nodes, child, flags);
	}
	else if (is_selection)
	{
	    if (AccessibleStateSet_contains (stateset, SPI_STATE_SELECTABLE))
	    {
		ret_nodes = gok_spy_append_node (ret_nodes, child, flags);
	    }
	}
	else
	{
	    ret_nodes = gok_spy_get_actionable_descendants (child, ret_nodes);
	}
	AccessibleStateSet_unref (stateset);
	Accessible_unref (child);
    }
    return ret_nodes;
}

/** 
* gok_spy_get_list 
* 
* @paccessible: The parent accessible to the list
*
* User must call gok_spy_free when finished with this list.
* 
* Returns: pointer to the list or NULL
**/ 
GSList* gok_spy_get_list( Accessible* paccessible)
{
	gok_log ("getting list of %d nodes\n.", g_slist_length (_priv_ui_nodes));
	/* TODO: should return a copy, with client-side incremented refs */
	return _priv_ui_nodes;
}

/** 
* gok_spy_refresh 
* 
* @plist: Pointer to the list to refresh
*
* not implemented.
*
* Returns: pointer to the refreshed list
**/ 
AccessibleNode* gok_spy_refresh( AccessibleNode* plist)
{
	gok_log_enter();
	gok_log_x(" this function not implemented ");
	gok_log_leave();
	return NULL;
}

/**
* gok_spy_free_nodes
*
* Frees the memory used by the given list. This must be called for every list
* that is created.
*
* @pNode: Pointer to the list that you want freed.
*
* Returns: void
**/
void gok_spy_free_nodes (GSList *nodes)
{
	GSList *nodelist = nodes;
	gok_log_enter();
	while (nodelist) 
	{
		AccessibleNode *node = nodelist->data;
		/* we gfree this since we've dup'd the SPI string */
		if (node != NULL)
		{
			g_free (node->pname);
			node->pname = NULL;
			gok_spy_accessible_unref (node->paccessible);
		}
		nodelist = nodelist->next;
	}
	g_slist_free (nodes);
	gok_log_leave();
}

/** 
* gok_spy_get_accessibleWithText
*
* accessor
**/
Accessible* gok_spy_get_accessibleWithText()
{
	return m_ptheAccessibleWithText;
}

/**
* gok_spy_check_queues
*
* this should be called during idle time. 
**/ 
gboolean gok_spy_check_queues(void)
{
	EventNode* en = NULL;
	char *s;
	GokKeyboard *current_kbd;

	gok_log_enter();
	
	if (spyshutdown) {
		return FALSE;
	}
	
	while (eventQueue && g_queue_is_empty(eventQueue) == FALSE)
	{
		en = (EventNode*)g_queue_pop_head(eventQueue);

		gok_log ("eventQueue->length = %d", eventQueue->length);

#if defined(ENABLE_LOGGING_NORMAL)
		if (en && en->event && en->event->source != NULL)
		{
			s = Accessible_getName (en->event->source);
			if (s == NULL)
			{
				gok_log ("%#x", en->event->source);
			}
			else
			{
				gok_log ("%#x %s", en->event->source, s);
				SPI_freeString (s);
			}
		}
#endif

		switch (en->type)
		{
		case GOKSPY_FOCUS_EVENT:
			/* TODO: remove obsolete focus events from queue since process
			   focus is expensive. */
			gok_log("GOKSPY_FOCUS_EVENT");
			gok_spy_process_focus (en->event->source);
			break;
		case GOKSPY_STATE_EVENT:
			gok_log("GOKSPY_STATE_EVENT");
			if (m_ptheStateChangeListener) {
				((AccessibleChangeListenerCB)(m_ptheStateChangeListener))(en->event->source);
			}
			break;
		case GOKSPY_WINDOW_ACTIVATE_EVENT:
			gok_log("GOKSPY_WINDOW_ACTIVATE_EVENT");
			/* reset word completion, if it's being used */
			gok_wordcomplete_reset (gok_wordcomplete_get_default ());
			if (en->event->source != m_ptheWindowAccessible) {
				gok_spy_accessible_unref (m_ptheWindowAccessible);
				m_ptheWindowAccessible = en->event->source;
				gok_spy_accessible_ref (m_ptheWindowAccessible);
				if (m_ptheWindowChangeListener) {
					((AccessibleChangeListenerCB)(m_ptheWindowChangeListener))(m_ptheWindowAccessible);
				}
			}
			break;
		case GOKSPY_DEFUNCT_EVENT:
			gok_log("GOKSPY_DEFUNCT_EVENT");
			/* note: currently only defunct events for the current application 
				are pushed on the queue */
			gok_spy_accessible_unref (m_ptheWindowAccessible);
			gok_spy_accessible_unref (m_ptheAccessibleWithText);
			m_ptheWindowAccessible = NULL;
			m_ptheAccessibleWithText = NULL;
			if (m_ptheWindowChangeListener) {
				((AccessibleChangeListenerCB)(m_ptheWindowChangeListener))(m_ptheWindowAccessible);
			}
			break;
		case GOKSPY_CONTAINER_EVENT:
			gok_log("GOKSPY_CONTAINER_EVENT");
			current_kbd = gok_main_get_current_keyboard ();
			gok_spy_update_component_list (gok_main_get_foreground_window_accessible (), 
			  current_kbd->flags);
			gok_main_ds (current_kbd);
			break;
		case GOKSPY_WINDOW_DEACTIVATE_EVENT:
			gok_log("GOKSPY_WINDOW_DEACTIVATE_EVENT");

			/*
			 * The test below is needed to workaround the case
			 * that a window:deactivate event may arrive
			 * after a window:activate has been emitted by
			 * another window. The test ignores window:deactivate
			 * events unless they come from what gok thinks
			 * is the current window.
			 */
			if (en->event->source == m_ptheWindowAccessible) {
				gok_spy_accessible_unref (m_ptheWindowAccessible);
				gok_spy_accessible_unref (m_ptheAccessibleWithText);
				m_ptheWindowAccessible = NULL;
				m_ptheAccessibleWithText = NULL;
				((AccessibleChangeListenerCB)(m_ptheWindowChangeListener))(m_ptheWindowAccessible);
			}
			break;
		case GOKSPY_KEYMAP_EVENT:
			gok_keyboard_notify_keys_changed ();
			break;
		default:
			gok_log_x("unknown event type in internal gok event queue!");
			break;
		}
		if (en->event) 
		{
			AccessibleEvent_unref (en->event);
		}
		g_free(en);
	}
	gok_log_leave();
	return FALSE; /* only return true if we left events in the queue! */
}

/** 
* gok_spy_check_window:
*
* @role: the role to check.
*
* This function decides if the role corresponds to something that looks like a window to the user.
*
* Returns: boolean success. 
*/ 
gboolean gok_spy_check_window(AccessibleRole role)
{
	/* TODO - improve efficiency here? Also, roles get added and we need
	   to maintain this...  maybe we need to go about this differently */
	if ((role == SPI_ROLE_WINDOW) ||
	    (role == SPI_ROLE_DIALOG) ||
	    (role == SPI_ROLE_FILE_CHOOSER) ||
	    (role == SPI_ROLE_FRAME) ||	
	    (role == SPI_ROLE_DESKTOP_FRAME) ||
		(role == SPI_ROLE_FONT_CHOOSER)	||
		(role == SPI_ROLE_COLOR_CHOOSER) ||
		(role == SPI_ROLE_APPLICATION) ||
		(role == SPI_ROLE_ALERT)
		)
	{
		return TRUE;
	}
	return FALSE;
}

/** 
 * gok_spy_worth_searching:
 *
 * @accessible: the accessible (possibly a parent) to examine for worthiness.
 *
 * This function decides if the accessible might have a subtree worth searching.
 *
 * Returns: boolean success. 
*/ 
gboolean gok_spy_worth_searching (Accessible* accessible)
{
	gboolean worthiness = TRUE;
	gboolean bmenu = FALSE;
	AccessibleStateSet* ass = NULL;

	gok_log_enter();
	
	switch (Accessible_getRole (accessible)) 
	{
	case SPI_ROLE_MENU:
	case SPI_ROLE_MENU_BAR:
	case SPI_ROLE_MENU_ITEM:
	case SPI_ROLE_CHECK_MENU_ITEM:
	case SPI_ROLE_RADIO_MENU_ITEM:
		bmenu = TRUE;
		break;
	default:
		bmenu = FALSE;
	}
	ass = Accessible_getStateSet(accessible);
	if (ass != NULL)
	{
		/* state heuristic: 
		 * - if this thing manages its own children, then it 
		 * is probably not worth traversing.
		 * - if it is a menu-like thing then it doesn't need to be "showing"
		 */
		if (!bmenu && !AccessibleStateSet_contains( ass, SPI_STATE_SHOWING ))
		{
			gchar *name = Accessible_getName (accessible);
			gchar *role_name = Accessible_getRoleName (accessible);
			gok_log ("%s %s is not worth searching", role_name, name);
			worthiness = FALSE;
		}
		/* TODO: determine whether we can safely bypass manages-descendants objects */
		/* it's not safe to exhaustviely search them */
		if (AccessibleStateSet_contains( ass, SPI_STATE_MANAGES_DESCENDANTS ))
		{
			gchar *name = Accessible_getName (accessible);
			gchar *role_name = Accessible_getRoleName (accessible);
			gok_log_x ("Manages-descendants state found on %s [%s]", name, role_name);
		}
		AccessibleStateSet_unref(ass);
	}
	gok_log_leave();
	return worthiness;
}

static gboolean
gok_spy_is_visible (Accessible *accessible)
{
	gboolean retval = FALSE;
	if (accessible) {
		AccessibleStateSet *states = Accessible_getStateSet (accessible);
		if (AccessibleStateSet_contains (states, SPI_STATE_VISIBLE) &&
		    AccessibleStateSet_contains (states, SPI_STATE_SHOWING))
			retval = TRUE;
		AccessibleStateSet_unref (states);
	}
	return retval;
}

gboolean
gok_spy_is_menu_role (AccessibleRole role)
{
	return ((role  ==  SPI_ROLE_MENU_ITEM) ||
		(role ==  SPI_ROLE_CHECK_MENU_ITEM) ||
		(role ==  SPI_ROLE_RADIO_MENU_ITEM) ||
		(role ==  SPI_ROLE_MENU));
}


static gboolean
gok_spy_is_selectable_child (Accessible *accessible, Accessible *parent)
{
    if (parent && Accessible_isSelection (parent))
    {
/* see bug #153638
	gboolean retval;
	AccessibleStateSet *stateset = Accessible_getStateSet (accessible);
	retval = AccessibleStateSet_contains (stateset, SPI_STATE_SELECTABLE);
	AccessibleStateSet_unref (stateset);
	return retval;
*/
	return TRUE;
    }
    return FALSE;
}

static gboolean
gok_spy_is_ui (Accessible *accessible, Accessible *parent, AccessibleRole role)
{
	gboolean interesting = FALSE;

	switch (role) {
	case SPI_ROLE_PUSH_BUTTON:
	case SPI_ROLE_CHECK_BOX:
	case SPI_ROLE_COMBO_BOX:
        case SPI_ROLE_SPIN_BUTTON:
	case SPI_ROLE_RADIO_BUTTON:
	case SPI_ROLE_PAGE_TAB:
	case SPI_ROLE_TOGGLE_BUTTON:
	case SPI_ROLE_SLIDER:
	case SPI_ROLE_SCROLL_BAR:
        case SPI_ROLE_LIST:
		interesting = TRUE;
		break;
	default:
		if (Accessible_isEditableText (accessible) ||
		    Accessible_isHypertext (accessible) ||
		    Accessible_isTable (accessible) ||
		    (!gok_spy_is_menu_role (role) && 
		     (Accessible_isAction (accessible) ||
		      gok_spy_is_selectable_child (accessible, parent))))
		    interesting = TRUE;
		gok_log ("checking for interesting interfaces... %s\n", interesting ? "yes" : "no");
		gok_log ("is icon: %s\n", role == SPI_ROLE_ICON ? "yes" : "no");
		break;
	}
	/* EditableText and Hypertext components are always interesting */
	/* however, invisible UI components are not interesting */
	return (interesting && gok_spy_is_visible (accessible)); 
}

/* return a node (if exists) from the list with the given name */
static GSList*
gok_spy_node_get_node_named (GSList *list, gchar *name)
{
	while (list != NULL) {
	        AccessibleNode *node = list->data;
		if (node && node->pname && strcmp (name, node->pname) == 0) {
			return list;
		}
		list = list->next;
	}
	return NULL;
}

/* modify the name for the given node so that it is distinct from namesakes */
static void
gok_spy_distinguish_node_name (AccessibleNode *node)
{
	AccessibleRelation **relations;
	Accessible *labelAccessible = NULL;
	AccessibleRelationType type = -1;
	char *s;
	gboolean distinguished;
	gint i,j, num_labels;
	i = 0;
	j = 0;
	num_labels = 0;
	distinguished = FALSE;
	
	/* find the label for the given node and prepend it to the name */
	if (node->paccessible) 
	    relations = Accessible_getRelationSet(node->paccessible);
	if (relations != NULL) {
	 while (relations[i]) {
		if (!distinguished) {
			type = AccessibleRelation_getRelationType(relations[i]);
			if (type == SPI_RELATION_LABELED_BY) {
				num_labels= AccessibleRelation_getNTargets(relations[i]);
				for (j=0; j < num_labels; j++) {
					labelAccessible = 
						AccessibleRelation_getTarget(relations[i], j);
					if (labelAccessible != NULL) {
						s = Accessible_getName (labelAccessible);
						/* don't prepend if we've already used this label as GokButton name */
						if (s != NULL && strcmp (s, node->pname)) {
							if ( strlen (s) != 0 )  {
								gchar buffer[64] = "";
								if (node->pname) 
								{
									if (strlen (node->pname)) 
									{
										strncpy (buffer, node->pname, 63);
									}
									g_free (node->pname); 
								}
								buffer[63] = '\0'; /* ensure null termination */
								node->pname = g_strconcat (s, " ", buffer, NULL);
								SPI_freeString (s);
								Accessible_unref (labelAccessible);
								distinguished = TRUE;
								break; /* from for loop*/
							}
							SPI_freeString(s);
						}
						Accessible_unref (labelAccessible);
					}
				}
				/* don't break from while loop (need to unref) */
			}
		}
		AccessibleRelation_unref(relations[i]);
		i++;
	 }
	 g_free (relations);
	}
}

static gboolean resolve_namesakes_reentry_guard = FALSE;

/**
 * gok_spy_resolve_namesakes:
 * @list the GSList of interesting nodes
 *
 * Disambiguate nodes in the given list that have the same name.
 */
static void
gok_spy_resolve_namesakes (GSList *list)
{
	gchar name[64]; /* names longer than this will not be resolved */
	GSList *namesake;
	gboolean distinguished_source = FALSE;

	name[63] = '\0';

	if (resolve_namesakes_reentry_guard) return;

	else resolve_namesakes_reentry_guard = TRUE;

	gok_log_enter ();

	/* for every node in the given list... */
	while (list && list->next) {
		AccessibleNode *node = list->data;
		if (node && node->pname) 
		{
		    strncpy (name, node->pname, 63);
		}
		/* for every node with the same name as the current node... */
		while (list && (namesake = gok_spy_node_get_node_named ( list->next, name )) 
				!= NULL) 
		{
			if (!distinguished_source) {
				distinguished_source = TRUE;
				gok_spy_distinguish_node_name ( (AccessibleNode *) list->data );
			}
			gok_spy_distinguish_node_name ( (AccessibleNode *) namesake->data );
			list = list->next;
		}
		distinguished_source = FALSE;	
		if (list) list = list->next;
	}

	resolve_namesakes_reentry_guard = FALSE;

	gok_log_leave ();
}

static GSList *
gok_spy_add_node (GSList *nodes, Accessible *accessible, AccessibleNodeFlags flags, gint link, char *name)
{
	AccessibleNode *newnode = NULL;

	newnode = (AccessibleNode*)g_malloc(sizeof(AccessibleNode));
	
	if (newnode != NULL) {
		newnode->paccessible = accessible;
		gok_spy_accessible_ref (accessible);
		newnode->flags = flags;
		newnode->link = link;
		newnode->pname = name;
		gok_log("newnode->pname = %s", newnode->pname);
		nodes = g_slist_append (nodes, newnode);
	}
	return nodes;
}

GSList *
gok_spy_remove_node (GSList *nodes, Accessible *accessible)
{
	GSList *node = nodes;

	while (node) {
		AccessibleNode *anode = nodes->data;
		if (anode && (anode->paccessible == accessible))
		{
			gok_spy_accessible_unref (anode->paccessible);
			return g_slist_delete_link (nodes, node);
		}
		node = node->next;
	}
	return nodes;
}
/**
 * gok_spy_append_node:
 * @pNode: An existing list of AccessibleNode to append to (may be NULL).
 * @pAccessible: The Accessible to append to the list.
 * @flags: Flags indicating the type of accessible in the node.
 *
 * Creates a new AccessibleNode for pAccessible and appends it to the
 * existing list of AccessibleNode at pNode.  Please note that this
 * function calls gok_spy_accessible_ref (pAccessible) when it
 * attaches it to the AccessibleNode created for it.  
 *
 * Returns: the address of the AccessibleNode created for pAccessible
 * or NULL if problems occur.
 */
GSList* gok_spy_append_node (GSList* nodes,
			     Accessible* pAccessible,
			     AccessibleNodeFlags flags)
{
	AccessibleNode* pLastNode = NULL;
	AccessibleNode* pNewNode = NULL;
	AccessibleRelation** relations = NULL;
	Accessible* targetAccessible = NULL;
	AccessibleRole role;
	AccessibleRelationType type = -1;
	AccessibleStateSet *states;

	char* pName = NULL, *s;
	int i = 0;
	int j = 0;
	int ntargets = 0;
	int maxloops = 100;  /* used to gaurd against infinite loops */

	gok_log_enter();

        s = Accessible_getName (pAccessible);
	if (s && strlen (s)) 
	  pName = g_strdup (s);
	if (s)
	  SPI_freeString (s);

	/* Hypertext objects are treated differently; one node per link */
	if (Accessible_isHypertext (pAccessible)) {
		AccessibleHypertext *hypertext = Accessible_getHypertext (pAccessible);
		AccessibleText *text = Accessible_getText (pAccessible);
		gint i, nlinks = AccessibleHypertext_getNLinks (hypertext);
		gok_log ("object contains hypertext.");
		for (i = 0; i < nlinks; ++i) {
			AccessibleHyperlink *link = AccessibleHypertext_getLink (hypertext, i);
			Accessible *link_object = AccessibleHyperlink_getObject (link, 0);
			gchar *linkname;
			long int start, end;
			/* TODO: handle multi-anchor links! */
			AccessibleHyperlink_getIndexRange (link, &start, &end);
			linkname = AccessibleText_getText (text, start, end);
			if (linkname) {
				/* create and append a node */
			  /* 
			   * FIXME: when AccessibleHyperlink objects get un-broken,
			   * we should pass the hyperlink object here instead of the 
			   * hypertext object, so we can just invoke the default action
			   * on the link!  But at the moment the link objects are not
			   * actionable, so we have to fake it.
			   */
				flags.data.is_link = TRUE;
				nodes = gok_spy_add_node (nodes, pAccessible, flags, i, g_strdup (linkname));
				SPI_freeString (linkname);
			}
			Accessible_unref (link_object);
		}
		AccessibleText_unref (text);
		AccessibleHypertext_unref (hypertext);
	}

	/* for other node types, try to create a reasonable label string */
	else {
		/* look for a label */
		if (pName == NULL)
		{
			gok_log("no name, so looking at relations...");
			relations = Accessible_getRelationSet(pAccessible);

			if (relations != NULL)
			while (relations[i]) {
				type = AccessibleRelation_getRelationType(relations[i]);
				if (type == SPI_RELATION_LABELED_BY) {
					ntargets = AccessibleRelation_getNTargets(relations[i]);
					for (j=0; j < ntargets; j++) {
						targetAccessible = 
							AccessibleRelation_getTarget(relations[i], j);
						if (targetAccessible != NULL) {
							s = Accessible_getName (targetAccessible);
							
							if (s != NULL) {
								if ( strlen (s) != 0 )  {
									pName = g_strdup (s);
									SPI_freeString (s);
									break; /* from for loop*/
								}
								else {
									SPI_freeString(s);
								}
							}
						}
					}
					break; /* from while loop */
				}
				i++;
			}
		}

		if (pName == NULL) {
			Accessible *parent = Accessible_getParent (pAccessible);
			/* if there are no siblings, we can use an ancestor's name */
			gok_log("checking ancestor names.");
			maxloops = 7; /* ignore great great great great great great grandparent */
			while (parent && Accessible_getChildCount (parent) == 1) 
			{
				char *tmp;
				Accessible *tmp_parent = parent;
				if (maxloops-- < 1) { break; }  /* guard against erroneous cyclic parent/child relations */
				tmp = Accessible_getName (tmp_parent); 
				gok_log ("getting parent name: parent %x, name %s, %d more loops allowed.\n", tmp_parent, tmp ? tmp : "", maxloops);
				if (tmp != NULL) 
				{
				    if (strlen (tmp) > 0) 
				    {
					pName = g_strdup (tmp);
					SPI_freeString (tmp);
					break;
				    }
				}
				parent = Accessible_getParent (tmp_parent);
				Accessible_unref (tmp_parent);
			}
		}
		
		if (pName == NULL) {
			gok_log("still no name, so looking at description...");
			/* one last try, we pull chars from description */
			s = Accessible_getDescription (pAccessible);
			if (s != NULL) {
				gint len = strlen (s);
				if ( len > 0 )  {
					if (len > 18) 
					{
						gchar *tmp = NULL;
						gok_log("shortening description");
						tmp = g_strndup (s, 15);
						pName = g_strconcat (tmp, "...", NULL);
						g_free (tmp);
					}
					else 
					{
						gok_log("using description");
						pName = g_strndup (s, len);
					}
				}
				SPI_freeString (s);
			}
		}

		if (pName == NULL) 
		{
		    role = Accessible_getRole (pAccessible);
		}

		/* 
		 * Role "TABLE-CELL" is used for 'generic' table children. 
		 * These children's behavior is governed more by the fact that they are
		 * children of a table than by their class-specific characteristics (unlike,
		 * for instance, ROLE_CHECKBUTTON or ROLE_PUSH_BUTTON objects which may be
		 * stored in tables).  Some table cells are composite objects, and we should
		 * attempt to create a name string from the concatenation of their children 
		 * when possible.  In the case of GtkTreeviews in particular, where row-selection
		 * is the dominant user activity, this is especially appropriate.
		 */
		if (pName == NULL && role == SPI_ROLE_TABLE_CELL) {
			pName = gok_spy_concatenate_child_names (pAccessible);
		}
		else if (pName == NULL && role == SPI_ROLE_SCROLL_BAR) {
		    states = Accessible_getStateSet (pAccessible);
		    if (AccessibleStateSet_contains (states, SPI_STATE_VERTICAL))
		    {
			/* translators: abbreviated version of "Vertical Scrollbar" */
			pName = g_strdup (_("V Scrollbar"));
		    }
		    else
		    {
			/* translators: abbreviated version of "Horizontal Scrollbar" */
			pName = g_strdup (_("H Scrollbar"));
		    }
		    AccessibleStateSet_unref (states);
		}
		else if (pName == NULL) { /* desperate attempt to reuse singleton-child's name */
			Accessible *tmp_parent = pAccessible;
			gok_log("no description... checking for singleton child with name.");
			maxloops = 3;
			Accessible_ref (tmp_parent);
			while (tmp_parent && Accessible_getChildCount (tmp_parent) == 1 && maxloops) 
			{
				Accessible *child;
				child = Accessible_getChildAtIndex (tmp_parent, 0);
				if (child) 
				{
					s = Accessible_getName (child);
					if (s)
					{
						gint len = strlen (s);
						if (len > 0) {
							pName = g_strndup (s, len);
							Accessible_unref (tmp_parent);
							tmp_parent = NULL;
						}
						SPI_freeString (s);
					}
				}
				if (tmp_parent) 
				{
					Accessible_unref (tmp_parent);
					tmp_parent = child;
				}
				--maxloops;
			}
		}
		if (pName == NULL && Accessible_isTable (pAccessible)) {
		        /* Tables are so important we must include them somehow */
			AccessibleTable *table = Accessible_getTable (pAccessible);
			Accessible *caption = AccessibleTable_getCaption (table);
			if (caption) 
			{
				char *caption_string = Accessible_getName (caption);
				if (caption_string) 
				{
					pName = g_strndup (caption_string, 20);
					SPI_freeString (caption_string);
				}
			}
			if (pName == NULL && AccessibleTable_getNColumns (table) == 1) 
			{
				char *desc = AccessibleTable_getColumnDescription (table, 0);
				if (desc) 
				{
					pName = g_strndup (desc, 20);
					SPI_freeString (desc);
				}
				else
				{
					Accessible *header;
					header = AccessibleTable_getColumnHeader (table, 0);
					if (header) 
					{
						desc = Accessible_getName (header);
						if (desc) 
						{
							pName = g_strndup (desc, 20);
							SPI_freeString (desc);
						}
						Accessible_unref (header);
					}
				}
			}
			if (pName == NULL)
			{
			        /* translators: "table" as in row/column data structure */
				pName = g_strdup (_("Table"));
			}
		}			

		if (pName != NULL) {
			gok_log("we have a name: [%s]",pName);
			/* create a new node */
			nodes = gok_spy_add_node (nodes, pAccessible, flags, 0, pName);
		}
		/* if we decide to allow access to nameless menus ...
		else if ( Accessible_getRole (pAccessible) == SPI_ROLE_MENU ) {
			pName = g_strdup (Accessible_getRoleName(pAccessible));
			gok_log ("Can't think of a good name so using role:[%s]",pName);
			nodes = gok_spy_add_node (nodes, pAccessible, -1, pName);
		}
		*/
		else if (Accessible_isText (pAccessible)) {
			/* 
			* sadly, many text objects are still nameless - yet we want 
			* to allow the user to get to them.
			* first tack is to get the starting text... if the object is
			* empty, then we can still "create" a name for it, though we 
			* can't assign it a unique meaningful name.
			*/
			AccessibleText *text = Accessible_getText (pAccessible);
			gint j = 0, len;
			gchar *word, *gs = NULL;

			gok_log("still no name, but there is text...");

			if (text) {
				long int start, end = 0;
				do {
					gchar *tmp;
					word = AccessibleText_getTextAtOffset (text, end, 
									       SPI_TEXT_BOUNDARY_WORD_END,
									       &start, &end);
					if (word) {
						len = strlen (word);
					}
					else {
						len = 0;
					}
					if (len > 0) {
						if (gs) {
							tmp = gs;
							gs = g_strconcat (tmp, word, NULL);
							g_free (tmp);
						}
						else {
							gs = g_strdup (word);
						}
					}
					if (word) { 
						SPI_freeString (word);
					}
					++j;
				} while ((j < 3) && len);
				AccessibleText_unref (text);
			}
			if (gs && strlen (gs)) {
				pName = g_strconcat (gs, "...", NULL);
				g_free (gs);
			}
			else {
				if (gs) {
					g_free (gs);
				}
				if (Accessible_isEditableText (pAccessible)) {
					pName = g_strdup ("Text Entry (empty)");
				}
			}
			if (pName != NULL) {
				gok_log ("we have a name: [%s]",pName);
				nodes = gok_spy_add_node (nodes, pAccessible, flags, 0, pName);
			}
		}
	}
	
	gok_log_leave();

	return nodes;
}

/** 
* gok_spy_focus_listener 
*
* callback for focus events in the at-spi 
**/ 
void gok_spy_focus_listener (const AccessibleEvent *event, void *user_data) 
{ 
	EventNode* en; 
	gok_log_enter();
	AccessibleEvent_ref (event);
	gok_spy_set_focussed_object (event->source);
	en = (EventNode*)g_malloc(sizeof(EventNode));
	en->event = event;
	en->type = GOKSPY_FOCUS_EVENT;
	g_queue_push_tail( eventQueue, en );

	gok_spy_add_idle_handler ();
	gok_log_leave();
}

/**
 * gok_spy_keymap_listener
 *
 **/
void 
gok_spy_keymap_listener (void)
{
	EventNode* en; 
	GList *head = NULL;
	gok_log_enter();

	/* 
	 * don't push another keymap event on the queue,
	 * if one is already pending.
	 */
	if (eventQueue) head = eventQueue->head;
	while (head)
	{
	    en = (EventNode *) head->data;
	    if (en->type == GOKSPY_KEYMAP_EVENT) 
	    {
		gok_log_leave ();
		return;
	    }
	    head = head->next;
	}

	en = (EventNode*) g_malloc(sizeof(EventNode));
	en->event = NULL;
	en->type = GOKSPY_KEYMAP_EVENT;
	g_queue_push_tail ( eventQueue, en );
	gok_spy_add_idle_handler ();
	
	gok_log_leave();
	
	return;
}


gboolean
gok_spy_is_primary_container (Accessible *accessible)
{
    AccessibleRole role;
    if (accessible) role = Accessible_getRole (accessible);
    return (role == SPI_ROLE_HTML_CONTAINER || role == SPI_ROLE_SCROLL_PANE);
}

/** 
* gok_spy_defunctness_listener 
*
* callback for when objects go defunct in the at-spi 
**/ 
void gok_spy_defunctness_listener (const AccessibleEvent *event, void *user_data) 
{
	EventNode* en; 
	gok_log_enter();

	if (event->source == m_ptheWindowAccessible) {
		en = (EventNode*) g_malloc(sizeof(EventNode));
		en->event = NULL;
		en->type = GOKSPY_DEFUNCT_EVENT;
		g_queue_push_tail ( eventQueue, en );
		gok_spy_add_idle_handler ();
		gok_log ("Our target window is now defunct!");
	}
	else if (gok_spy_is_primary_container (event->source)) 
	{
		en = (EventNode*) g_malloc(sizeof(EventNode));
		en->event = NULL;
		en->type = GOKSPY_CONTAINER_EVENT;
		g_queue_push_tail ( eventQueue, en );
		gok_spy_add_idle_handler ();
		gok_log ("A primary UI container is now defunct!");
	}
	
	gok_log_leave();
}

static void
gok_spy_window_activate_listener(const AccessibleEvent *event,
                                 void *user_data)
{
	EventNode *en;
	gok_log_enter ();

	AccessibleEvent_ref (event);
	en = (EventNode*) g_malloc (sizeof (EventNode));
	en->event = event;
	en->type = GOKSPY_WINDOW_ACTIVATE_EVENT;
	g_queue_push_tail (eventQueue, en);
	gok_spy_add_idle_handler ();

	gok_log_leave ();
}

static void
gok_spy_window_deactivate_listener(const AccessibleEvent *event,
                                   void *user_data)
{
	EventNode *en;
	gok_log_enter ();

	AccessibleEvent_ref (event);
	en = (EventNode*)g_malloc (sizeof (EventNode));
	en->event = event;
	en->type = GOKSPY_WINDOW_DEACTIVATE_EVENT;
	g_queue_push_tail (eventQueue, en);
	gok_spy_add_idle_handler ();

	gok_log_leave ();
}

static void
gok_spy_object_state_listener(const AccessibleEvent *event,
                                 void *user_data)
{
	EventNode *en;
	gboolean is_container_add = !strcmp (event->type, "object:children-changed:add");

	gok_log_enter ();

	if (is_container_add)
	{
	    if (!event->source || !gok_spy_is_primary_container (event->source))
	    {
		gok_log_leave ();
		return;
	    }
	}
	AccessibleEvent_ref (event);
	en = (EventNode*)g_malloc (sizeof (EventNode));
	en->event = event;
	en->type = is_container_add ? GOKSPY_CONTAINER_EVENT : GOKSPY_STATE_EVENT;
	g_queue_push_tail (eventQueue, en);
	gok_spy_add_idle_handler ();

	gok_log_leave ();
}

void
gok_spy_set_context_menu_accessible (Accessible *accessible, int action_index)
{
	gboolean availability_changed = FALSE;
	GokSpyUIFlags menu_change_flags, new_flags;

	if (theContextMenuAccessible != accessible) 
	{
		if (_priv_ui_nodes && theContextMenuAccessible)
			_priv_ui_nodes = gok_spy_remove_node (_priv_ui_nodes, theContextMenuAccessible);
		if (theContextMenuAccessible) 
			gok_spy_accessible_unref (theContextMenuAccessible);
		availability_changed = TRUE;
		theContextMenuAccessible = accessible;
		if (accessible) 
		{
			gok_spy_accessible_ref (theContextMenuAccessible);
		}
	}

	menu_change_flags.value = 0;
	new_flags.value = 0;
	menu_change_flags.data.context_menu = TRUE;
	new_flags.data.context_menu = (theContextMenuAccessible != NULL);
	if (theContextMenuAccessible) 
	{
		AccessibleNodeFlags context_menu_flags;
		context_menu_flags.value = 0;
		context_menu_flags.data.is_menu = 1;
		context_menu_flags.data.has_context_menu = 1;
		_priv_ui_nodes = gok_spy_add_node (_priv_ui_nodes, theContextMenuAccessible, 
						   context_menu_flags, action_index, g_strdup ("context"));
	}
	/* update the 'Menus' button on current keyboard, if appropriate */
	gok_keyboard_update_dynamic_keys (gok_main_get_current_keyboard (), menu_change_flags, new_flags);
}
	
static gboolean
gok_spy_context_menu_available (void)
{
	return (theContextMenuAccessible != NULL);
}

/** 
* gok_spy_process_focus 
* 
* @accessible: pointer to the accessible to process, assumed to have no references (see Accessible_ref in cspi)
* 
* side effects: sets boolean state variable indicating whether currently focussed object
* has a context menu or not; also may reset the global 'current accessible text object' state.
**/ 
void gok_spy_process_focus (Accessible* accessible) 
{ 
	Accessible* pnewWindowAccessible;
	Accessible* ptempAccessible;
	gboolean has_menu = FALSE;
	
	gok_log_enter();
	g_assert(accessible != NULL);
	
	pnewWindowAccessible = NULL;
	ptempAccessible = NULL;
	
	/*
	 *  check to see if focus is on a text object 
	 */
	if (Accessible_isText(accessible) == TRUE)
	{
		gok_log("this thing has a text interface");
		if (accessible != m_ptheAccessibleWithText)
		{
			gok_spy_accessible_ref(accessible); 
			gok_spy_accessible_unref(m_ptheAccessibleWithText);
			m_ptheAccessibleWithText = accessible;
			gok_log("found a (new) text interface"); 
		}
	}
	else
	{
		gok_log("no text interface on this thing");
		if (m_ptheAccessibleWithText != NULL)
		{
			gok_spy_accessible_unref(m_ptheAccessibleWithText);
		}
		m_ptheAccessibleWithText = NULL;
	}
	/* check for availability of a context menu */
	/* unfortunately we must special-case objects with selectable children,
	   since the context menu may apply to the child */

	if (Accessible_isAction (accessible))
	{
		AccessibleAction *action = Accessible_getAction (accessible);
		long i, action_count;
		action_count = AccessibleAction_getNActions (action);
		for (i = 0; i < action_count; ++i) 
		{
			char *name = AccessibleAction_getName (action, i);
			if (name && !strcmp ("menu", name))
			{
				gok_spy_set_context_menu_accessible (accessible, i);
				has_menu = TRUE;
				SPI_freeString (name);
				break; /* from for loop */
			}
			SPI_freeString (name);
		}
	}
	if (!has_menu) gok_spy_set_context_menu_accessible (NULL, 0);

	/* 
	 * if current active window is NULL, and this is a menu item, 
	 * then we're in a popup menu of some kind; branch to the menus kbd
	 */

	if (accessible && (m_ptheWindowAccessible == NULL) && 
	    gok_spy_is_menu_role (Accessible_getRole (accessible)))
	{
	    AccessibleNode *node = g_new0 (AccessibleNode, 1);
	    gchar *name = Accessible_getName (accessible);
	    node->link = -1;
	    node->flags.value = 0;
	    node->flags.data.is_menu = TRUE;
	    node->pname = g_strdup (name ? name : "");
	    gok_spy_accessible_ref (accessible);
	    node->paccessible = accessible;
	    gok_keyboard_branch_gui (node, GOK_SPY_SEARCH_CHILDREN);
	}

	gok_log_leave();
} 

/**
 * gok_spy_button_is_switch_trigger:
 *
 * @button: an int representing logical mouse button number to query.
 *  
 * Returns: TRUE if the button is used by GOK as a switch, FALSE otherwise.
 **/
static gboolean
gok_spy_button_is_switch_trigger (int button)
{
        if (!strcmp (gok_data_get_name_accessmethod (), "directselection"))
	{
		if (gok_main_window_contains_pointer () == TRUE)
			return TRUE;
		else
			return FALSE;	
	}
	else if (gok_data_get_drive_corepointer() == TRUE) {
		gok_log("gok_data_get_drive_corepointer() is TRUE");
		return FALSE;
	}
	else {
		gok_log("gok_data_get_drive_corepointer() is FALSE");
		return gok_scanner_current_state_uses_core_mouse_button(button);
	}
}

/** 
* gok_spy_mouse_listener 
* 
* callback for mouse events in the at-spi 
* 
* @event: event structure
* @user_data: not used here
*
*/ 
SPIBoolean 
gok_spy_mouse_listener (const AccessibleDeviceEvent *event, void *user_data) 
{ 
	gint button = 0;
	gint state = 0;
	gboolean is_switch_trigger;
	gok_log_enter();

	/* 
	 * must check now and save for later, since a branch action 
	 * may change this by altering the window size, etc.
	 */
	button = event->keycode;
	is_switch_trigger = gok_spy_button_is_switch_trigger (button);

	if (m_ptheMouseButtonListener != NULL)
	{
		gok_log("mouse event %ld %x %x", 
			event->keyID, (unsigned) event->type, 
			(unsigned) event->modifiers);

			if (event->type == SPI_BUTTON_PRESSED)
			{
				state = 1;
			}
			((MouseButtonListenerCB)(m_ptheMouseButtonListener))(button, 
									     state, 
									     (long) event->modifiers,
									     event->timestamp);
	}
	gok_log_leave();
        return is_switch_trigger;
}

/**
 * gok_spy_get_modmask:
 **/
gulong gok_spy_get_modmask (void)
{
	return keyboardmods;
}


/** 
 * gok_spy_modifier_listener:
 * @event: the keyboard modifier event.
 * @user_data: not used.
 *
 * Callback for keyboard modifier events from the at-spi.
 */
void gok_spy_modifier_listener(const AccessibleEvent *event, void *user_data)
{
	const struct gok_modifier_keymask *m;
       
	keyboardmods = event->detail2;

	gok_log_enter();
	gok_log ("event->detail2 = %d", event->detail2);
	gok_keyboard_update_labels ();
	gok_modifier_update_modifier_keys (gok_main_get_current_keyboard ());
	gok_log_leave();
}

/** 
 * gok_spy_add_idle_handler:
 *
 * Adds the idle handler to deal with spy events
 */
void gok_spy_add_idle_handler (void)
{
	g_idle_add (gok_spy_idle_handler, NULL);
}

/* 
* gok_spy_get_active_frame
* 
* User must call gok_spy_free when finished with this list.
* 
* Returns: pointer to the active Accessible* or NULL
*/
Accessible* 
gok_spy_get_active_frame( )
{
	gint i;
	gint j;
	gint k;
	gint n_desktops;
	gint n_apps;
	gint n_frames;
	Accessible* desktop;
	Accessible* child;
	Accessible* active_frame = NULL;
	AccessibleStateSet* ass = NULL;

	gok_log_enter();

	n_desktops = SPI_getDesktopCount();
	for (i = 0; i < n_desktops; i++)
	{
		desktop = SPI_getDesktop(i);
		n_apps= Accessible_getChildCount(desktop);
		for (j = 0; j < n_apps; j++)
		{
			child = Accessible_getChildAtIndex(desktop, j);
			
			if (!child)
				continue;
			
			/* Applications are not active
			   so we must look for a child frame */
			n_frames = Accessible_getChildCount (child);
			for (k = 0; k < n_frames; k++) {
				Accessible *frame;		
				frame = Accessible_getChildAtIndex (child, k);

				if (!frame)
					continue;

				ass = Accessible_getStateSet (frame); /* implicit ref? */
				
				if (!ass) {
					Accessible_unref (frame);
					continue;
				}

				if (AccessibleStateSet_contains (ass, SPI_STATE_ACTIVE)) {
					gok_log ("Found active frame");
					active_frame = frame;
				}
				else {
					Accessible_unref (frame);
				}
				AccessibleStateSet_unref (ass);
			}
			Accessible_unref (child);
		}
		Accessible_unref (desktop);
	}
	gok_log_leave();
	return active_frame;
}

/* recursive and exhaustive */
static gboolean
gok_spy_find_and_append_toolbar_items (Accessible *root, AccessibleNodeFlags flags)
{
	Accessible *child, *tmp;
	gint i, nchildren;
	gboolean retval = FALSE;

	if (gok_spy_is_menu_role (Accessible_getRole (root)))
	{
	        return FALSE; /* make sure we continue searching independent of toolbars here */
	}
	else if (Accessible_isAction (root)) {
		_priv_ui_nodes = gok_spy_append_node (_priv_ui_nodes, root, flags);
		retval = TRUE;
		/* uncomment if we don't care about children of actionables: */
		/* return; */
	}
	nchildren = Accessible_getChildCount (root);
	for (i = 0; i < nchildren; i++) {
		child = Accessible_getChildAtIndex (root, i);
		retval |= gok_spy_find_and_append_toolbar_items (child, flags);
		Accessible_unref (child); 
	}
	return retval;
}

static GokSpyUIFlags 
gok_spy_search_component_list (Accessible *rootAccessible, GokSpyUIFlags keyboard_ui_flags, AccessibleNodeFlags context_flags)
{
	Accessible* child;
	Accessible* parent;
	AccessibleRole role;
	AccessibleRole parent_role;
	GokSpyUIFlags ui_flags;
	long child_count;
	long i;

	gok_log_enter ();

	ui_flags.value = 0;
	parent = rootAccessible;

	if (gok_spy_worth_searching (parent) == FALSE) 
	{
		gok_log ("not worth searching.");
		gok_log_leave ();
		return ui_flags;
	}			
	parent_role = Accessible_getRole (parent);
	child_count = Accessible_getChildCount (parent);
	
	/* assign context */
	if (parent_role == SPI_ROLE_HTML_CONTAINER) {
			context_flags.data.inside_html_container = TRUE;
	}
	
	/* MAX_CHILD_COUNT is a bit of a kludge and will cause certain table items to be unreachable via UI Grab */
	if (child_count > MAX_CHILD_COUNT)
		child_count = MAX_CHILD_COUNT;
	for (i = 0; i < child_count; ++i)
	{
		AccessibleNodeFlags flags;  /* TODO? memcpy context_flags? */
		flags.value = 0;
		flags.data.inside_html_container = context_flags.data.inside_html_container;

		child = Accessible_getChildAtIndex (parent, i);
		if (child)
		{
			gok_spy_accessible_implicit_ref (child);
			role = Accessible_getRole (child);
			if (keyboard_ui_flags.data.menus && gok_spy_is_menu_role (role))
			{
				if (parent_role == SPI_ROLE_MENU_BAR &&
				    !gok_spy_is_visible (child))
				{
					gok_spy_accessible_unref (child);
					continue;
				}
				ui_flags.data.menus = flags.data.is_menu = TRUE;
			}
			else if (keyboard_ui_flags.data.toolbars && 
				 parent_role == SPI_ROLE_TOOL_BAR)
			{
				if (keyboard_ui_flags.data.gui) 
					flags.data.is_ui = TRUE; /* toolbar items are ui elements too */
				ui_flags.data.toolbars = flags.data.is_toolbar_item = TRUE;
				if (!gok_spy_find_and_append_toolbar_items (child, flags)) 
				{
				    ui_flags.value |= gok_spy_search_component_list (child, keyboard_ui_flags, context_flags).value;
				}
			}
			if ((keyboard_ui_flags.data.gui || keyboard_ui_flags.data.editable_text) && 
			    !flags.data.is_menu && !flags.data.is_toolbar_item && gok_spy_is_ui (child, parent, role))
			{
				if (keyboard_ui_flags.data.gui) 
				{
				    ui_flags.data.gui = flags.data.is_ui = TRUE;
				}
				if (keyboard_ui_flags.data.editable_text && Accessible_isEditableText (child)) 
				{
					ui_flags.data.editable_text = TRUE;
				}
			}
			if (!flags.data.is_toolbar_item) /* toolbar items already added */
			{
			        AccessibleNodeFlags non_context_flags = flags;
				non_context_flags.data.inside_html_container = FALSE;

				if (non_context_flags.value) 
				{
				    _priv_ui_nodes = gok_spy_append_node (_priv_ui_nodes, child, flags);
				    /* special case for notebook tabs, whose children should also appear */
  				    if (role == SPI_ROLE_PAGE_TAB)
  				    {
  					ui_flags.value |= gok_spy_search_component_list (child, keyboard_ui_flags, context_flags).value;
  				    }
				}
				else
				{
					ui_flags.value |= gok_spy_search_component_list (child, keyboard_ui_flags, context_flags).value;
				}
			}
			gok_spy_accessible_unref (child);
		}
	}	

	/* one last check for context menus on the currently focussed object */
	if (keyboard_ui_flags.data.menus)
	{
		ui_flags.data.context_menu = gok_spy_context_menu_available ();
	}

	gok_log_leave ();

	return ui_flags;
}


/**
 * gok_spy_update_component_list:
 *
 * Call this to update GOK's internal list of 'interesting UI components'.
 * It searches the topmost accessible of the currently-focussed window 
 * (which should be passed in as @rootAccessible), and update's GOK's list
 * based on the relevant children.  
 *
 * returns: A set of flags indicating the types of UI components available in
 *          the context specified by @rootAccessible.
 **/
GokSpyUIFlags 
gok_spy_update_component_list (Accessible *rootAccessible, GokSpyUIFlags keyboard_ui_flags)
{
	GokSpyUIFlags ui_flags;
	AccessibleNodeFlags context_flags;

	gok_log_enter ();

	gok_spy_free_nodes (_priv_ui_nodes);
	_priv_ui_nodes = NULL;

	context_flags.value = 0;
	ui_flags = gok_spy_search_component_list (rootAccessible, keyboard_ui_flags, context_flags);
	/* _priv_ui_nodes is now re-assigned, by above call. */
	gok_spy_resolve_namesakes (_priv_ui_nodes);

	gok_log_leave ();

	return ui_flags;
}

/*
gboolean gok_spy_is_desktop(Accessible* pAccessible)
{
	gint i;
	gint ndesktops;
	Accessible* desktop;

	gok_log_enter();

	ndesktops = SPI_getDesktopCount();
	for (i = 0; i < ndesktops; i++)
	{
		desktop = SPI_getDesktop(i);
		gok_spy_accessible_implicit_ref(desktop);
		if (pAccessible == desktop)
		{
			gok_spy_accessible_unref(desktop);
			gok_log_leave();
			return TRUE;
		}
	}
	gok_spy_accessible_unref(desktop);
	return FALSE;
}

gboolean gok_spy_accessible_is_desktopChild(Accessible* accessible)
{
	gint i;
	gint j;
	gint ndesktops;
	gint nchildren;
	Accessible* desktop;
	Accessible* child;
	gboolean returnCode = FALSE;
	
	gok_log_enter();

	ndesktops = SPI_getDesktopCount();
	for (i = 0; i < ndesktops; i++)
	{
		desktop = SPI_getDesktop(i);
		gok_spy_accessible_implicit_ref(desktop);
		if (desktop == NULL)
		{
			gok_log("desktop disappeared!?");
			break;
		}
		nchildren = Accessible_getChildCount(desktop);
		for (j = 0; j < nchildren; j++)
		{
			child = Accessible_getChildAtIndex(desktop, j);
			gok_spy_accessible_implicit_ref(child);
			if (child != NULL)
			{
				if (child == accessible)
				{
					gok_spy_accessible_unref(child);
					returnCode = TRUE;
					break;
				}
			}
		}
		if (returnCode == TRUE)
		{
			gok_spy_accessible_unref(desktop);
			break;
		}
		gok_spy_accessible_unref(desktop);
	}
	return returnCode;
	gok_log_leave();
}
*/
