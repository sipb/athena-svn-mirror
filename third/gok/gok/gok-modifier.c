/* gok-modifier.c
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

#include "gok-modifier.h"
#include "gok-log.h"
#include "gok-keyboard.h"
#include "main.h"

static GokModifier* m_pModifierFirst;

/**
* gok_modifier_open
*
* Initializes the modifier. Call this once at the beginning of the program.
**/
void gok_modifier_open ()
{
	m_pModifierFirst = NULL;
}

/**
* gok_modifier_close
*
* Deletes any modifiers that have been created. This must be called at the
* end of the program.
**/
void gok_modifier_close ()
{
	GokModifier* pModifier;
	GokModifier* pModifierTemp;
	
	pModifier = m_pModifierFirst;
	while (pModifier != NULL)
	{
		pModifierTemp = pModifier;
		pModifier = pModifier->pModifierNext;

		g_free (pModifierTemp->Name);
		g_free (pModifierTemp);
	}
}

/**
* gok_modifier_add
* @Name: Name of the modifier.
*
* Adds a modifier to the list of modifiers.
*
* returns: TRUE if the modifier was created, FALSE if not.
**/
gboolean gok_modifier_add (gchar* Name)
{
	GokModifier* pNewModifier;
	GokModifier* pModifierTemp;

	g_assert (Name != NULL);
	
	/* check if this modifier already exists */
	pModifierTemp = m_pModifierFirst;
	while (pModifierTemp != NULL)
	{
		if (strcmp (pModifierTemp->Name, Name) == 0)
		{
			/* the modifier already exists so don't add it again */
			return TRUE;
		}
		pModifierTemp = pModifierTemp->pModifierNext;
	}
	
	/* create a new modifier */
	pNewModifier = (GokModifier*)g_malloc (sizeof (GokModifier));
	
	/* add the name to the modifier */
	pNewModifier->Name = (gchar*)g_malloc (strlen (Name) + 1);
	strcpy (pNewModifier->Name, Name);
	
	/* initialize the modifier */
	pNewModifier->State = MODIFIER_STATE_OFF;
	pNewModifier->WrapperPre = NULL;
	pNewModifier->WrapperPost = NULL;
	pNewModifier->pModifierNext = NULL;
	pNewModifier->Type = MODIFIER_TYPE_NORMAL;
	
	/* hook the new modifier into the list of modifiers */
	if (m_pModifierFirst == NULL)
	{
		m_pModifierFirst = pNewModifier;
	}
	else
	{
		pModifierTemp = m_pModifierFirst;
		while (pModifierTemp->pModifierNext != NULL)
		{
			pModifierTemp = pModifierTemp->pModifierNext;
		}
		pModifierTemp->pModifierNext = pNewModifier;
	}
		
	return TRUE;
}

/**
* gok_modifier_set_pre
* @Name: Name of the modifier.
* @pOutput: Pointer to the output that will be set as the modifier wrapper 'pre'.
*
* Sets the wrapper 'pre' output for the given modifier.
*
* returns: TRUE if the modifier 'pre' was set, FALSE if not.
**/
gboolean gok_modifier_set_pre (gchar* Name, GokOutput* pOutput)
{
	GokModifier* pModifier;

	g_assert (Name != NULL);
	g_assert (pOutput != NULL);
	
	/* find the modifier in the list of modifiers */
	pModifier = m_pModifierFirst;
	while (pModifier != NULL)
	{
		if (strcmp (pModifier->Name, Name) == 0)
		{
			break;
		}
		pModifier = pModifier->pModifierNext;
	}
	
	if (pModifier == NULL)
	{
		gok_log_x ("Can't add 'pre'! Modifier '%s' doesn't exist!\n", Name);
		return FALSE;
	}
	
	pModifier->WrapperPre = pOutput;
	
	return TRUE;
}

/**
* gok_modifier_set_type
* @Name: Name of the modifier.
* @Type: The type of the modifier.
*
* Sets the 'Type' attribute for the given modifier.
**/
void gok_modifier_set_type (gchar* Name, int Type)
{
	GokModifier* pModifier;
	
	/* find the modifier in the list of modifiers */
	pModifier = m_pModifierFirst;
	while (pModifier != NULL)
	{
		if (strcmp (pModifier->Name, Name) == 0)
		{
			pModifier->Type = Type;
			break;
		}
		pModifier = pModifier->pModifierNext;
	}	
}

/**
* gok_modifier_set_post
* @Name: Name of the modifier.
* @pOutput: Pointer to the output that will be set as the modifier wrapper 'post'.
*
* Sets the 'post' output for the given modifier.
*
* returns: TRUE if the modifier 'pre' was set, FALSE if not.
**/
gboolean gok_modifier_set_post (gchar* Name, GokOutput* pOutput)
{
	GokModifier* pModifier;

	g_assert (Name != NULL);
	g_assert (pOutput != NULL);
	
	/* find the modifier in the list of modifiers */
	pModifier = m_pModifierFirst;
	while (pModifier != NULL)
	{
		if (strcmp (pModifier->Name, Name) == 0)
		{
			break;
		}
		pModifier = pModifier->pModifierNext;
	}
	
	if (pModifier == NULL)
	{
		gok_log_x ("Can't add 'post'! Modifier '%s' doesn't exist!\n", Name);
		return FALSE;
	}
	
	pModifier->WrapperPost = pOutput;
	
	return TRUE;
}

/**
* gok_modifier_output_pre
*
* Sends all the wrapper 'pre' outputs to the system.
**/
void gok_modifier_output_pre ()
{
	GokModifier* pModifier;
	
	pModifier = m_pModifierFirst;
	while (pModifier != NULL)
	{
		if (pModifier->State != MODIFIER_STATE_OFF)
		{
			gok_output_send_to_system (pModifier->WrapperPre, FALSE);
		}
		pModifier = pModifier->pModifierNext;
	}
}

/**
* gok_modifier_output_post
*
* Sends all the wrapper 'post' outputs to the system.
**/
void gok_modifier_output_post ()
{
	GokModifier* pModifier;

	pModifier = m_pModifierFirst;
	while (pModifier != NULL)
	{
		if (pModifier->State != MODIFIER_STATE_OFF)
		{
			gok_output_send_to_system (pModifier->WrapperPost, FALSE);
		}
		pModifier = pModifier->pModifierNext;
	}
}

/**
* gok_modifier_all_off
*
* Changes the state of modifier keys to OFF, unless they are locked on.
**/
void gok_modifier_all_off ()
{
}

/**
 * gok_modifier_mask_for_name:
 * Returns the modifier mask (single bit set) corresponding to @name, for
 * a given @display.
 **/
guint
gok_modifier_mask_for_name (Display *display, char *modifier_name)
{
	/* TODO: generalize this, instead of just hacking it in */
	if (!modifier_name)
		return SPI_KEYMASK_UNMODIFIED;
	else if (!strcmp (modifier_name, "shift"))
		return SPI_KEYMASK_SHIFT;
	else if (!strcmp (modifier_name, "capslock") || !strcmp (modifier_name, "lock"))
		return SPI_KEYMASK_SHIFTLOCK;
	else if (!strcmp (modifier_name, "ctrl"))
		return SPI_KEYMASK_CONTROL;
	else if (!strcmp (modifier_name, "alt"))
		return SPI_KEYMASK_ALT;
	else if (!strcmp (modifier_name, "numlock"))
		return gok_key_get_numlock_mask (display);
	else if (!strcmp (modifier_name, "mod1"))
		return SPI_KEYMASK_MOD1;
	else if (!strcmp (modifier_name, "mod2"))
		return SPI_KEYMASK_MOD2;
	else if (!strcmp (modifier_name, "mod3"))
		return SPI_KEYMASK_MOD3;
	else if (!strcmp (modifier_name, "mod4"))
		return SPI_KEYMASK_MOD4;
	else if (!strcmp (modifier_name, "mod5"))
		return SPI_KEYMASK_MOD5;
	else
		return SPI_KEYMASK_UNMODIFIED;
}

/**
* gok_modifier_get_state
* @NameModifier: Name of the modifier you want the state for.
*
* returns: The state of the modifier
**/
int gok_modifier_get_state (gchar* NameModifier)
{
	GokModifier* pModifier;

	/* if this is one of the 'well-known' modifiers defined by Xlib, just check modmask */
	if (gok_modifier_mask_for_name (gok_main_display (), NameModifier) & gok_spy_get_modmask ()) {
		return TRUE;
	}
	else 
	{
		/* find the modifier in the list */
		pModifier = m_pModifierFirst;
		while (pModifier != NULL)
		{
			if ((pModifier->Name != NULL) &&
			    (strcmp (pModifier->Name, NameModifier) == 0))
			{
				/* return its state */
				return pModifier->State;
			}
			pModifier = pModifier->pModifierNext;
		}
		
		/* can't find the modifier in the list so return OFF */
		gok_log_x ("Can't find modifier named %s!\n", NameModifier);
	}
	return MODIFIER_STATE_OFF;
}

/**
* gok_modifier_get_type
* @NameModifier: Name of the modifier you want the type for.
*
* returns: The type of the modifier
**/
int gok_modifier_get_type (gchar* NameModifier)
{
	GokModifier* pModifier;

	/* find the modifier in the list */
	pModifier = m_pModifierFirst;
	while (pModifier != NULL)
	{
		if ((pModifier->Name != NULL) &&
			(strcmp (pModifier->Name, NameModifier) == 0))
		{
			/* return its state */
			return pModifier->Type;
		}
		pModifier = pModifier->pModifierNext;
	}

	/* can't find the modifier in the list so return MODIFIER_TYPE_NORMAL */
	gok_log_x ("Can't find modifier name!\n");
	return MODIFIER_TYPE_NORMAL;
}

/**
* gok_modifier_get_normal
*
* returns: TRUE if there are no modifiers on (or locked on). Returns
* FALSE if one or more modifiers are on (or locked on).
**/
gboolean gok_modifier_get_normal()
{
	/* FIXME: too simple if and when we add GOK-specific modifiers that don't map to keys */
	return (gok_spy_get_modmask () == 0);
}

/**
* gok_modifier_update_modifier_keys
*
* Updates the indicator on all the modifier keys for the current keyboard.
**/
void gok_modifier_update_modifier_keys (GokKeyboard* pKeyboard)
{
	GokKey* pKey;
	int state;
	gchar buffer[300];
	
	if (pKeyboard == NULL)
	{
		return;
	}

	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
	        pKey->ComponentState.latched = 0;
	        pKey->ComponentState.locked = 0;
		if (pKey->Type == KEYTYPE_MODIFIER && pKey->ModifierName)
		{
			state = gok_modifier_get_state (pKey->ModifierName);
			if (state == MODIFIER_STATE_ON)
			{
				pKey->ComponentState.latched = 1;
			}
			else if (state == MODIFIER_STATE_LOCKED)
			{
				pKey->ComponentState.locked = 1;
			}
			gok_key_update_toggle_state (pKey);
		}
		pKey = pKey->pKeyNext;
	}
}

/**
 **/
gchar*
gok_modifier_first_name_from_mask (guint modmask)
{
	/* FIXME: no relation to modifiers list, unfortunately */
	if (!modmask)
		return NULL;
	if (modmask & ShiftMask)
		return "shift";
	if (modmask & LockMask)
		return "capslock";
	if (modmask & 	
	    gok_key_get_numlock_mask (gok_keyboard_get_display ()))
		return "numlock";
	if (modmask & ControlMask)
		return "ctrl";
	if (modmask & Mod1Mask)
		return "alt";
	if (modmask & Mod2Mask)
		return "mod2";
	if (modmask & Mod3Mask)
		return "mod3";
	if (modmask & Mod4Mask)
		return "mod4";
	if (modmask & Mod5Mask)
		return "mod5";
	return NULL;
}
