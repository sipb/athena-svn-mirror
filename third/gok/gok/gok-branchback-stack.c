/* gok-branchback-stack.h
*
* Copyright 2002 Sun Microsystems, Inc.,
* Copyright 2001 University Of Toronto
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

#include "gok-branchback-stack.h"
#include "gok-log.h"

/* the array that contains the stack of keyboards */
static GokKeyboard* BranchBackStack [MAX_BRANCHBACKSTACK];

/* for debugging */
static gint m_PushesMinusPops;

/**
* gok_branchbackstack_initialize
*
* This must be called prior to using the stack.
*
* returns: void
**/
void gok_branchbackstack_initialize ()
{
	gint x;

	/* clear the branch back stack */
	for (x = 0; x < MAX_BRANCHBACKSTACK; x++)
	{
		BranchBackStack[x] = NULL;
	}
	
	m_PushesMinusPops = 0;
}

/**
* gok_branchbackstack_is_empty
*
* Checks if the branch back stack is empty.
*
* returns:  TRUE if the stack is empty (you can't branch back). FALSE if there is at least 
* one keyboard in the stack (you can branch back).
**/
gboolean gok_branchbackstack_is_empty ()
{
	gint x;

	for (x = 0; x < MAX_BRANCHBACKSTACK; x++)
	{
		if (BranchBackStack[x] != NULL)
		{
			return FALSE;
		}
	}

	return TRUE;
}

/**
 * gok_branchbackstack_push:
 * @pKeyboard:
 *
 * Stores a keyboard on the stack.
 */
void gok_branchbackstack_push (GokKeyboard* pKeyboard)
{
	gint x;

	gok_log_enter();
	
	/* make sure we have a valid keyboard pointer */
	if (pKeyboard == NULL)
	{
		gok_log_x ("keyboard is NULL");
		gok_log_leave();
		return;
	}
	
	gok_log ("pushing keyboard [%s] onto stack",pKeyboard->Name);
	m_PushesMinusPops++;

	/* find the first empty space in the stack */
	for (x = 0; x < MAX_BRANCHBACKSTACK; x++)
	{
		if (BranchBackStack[x] == NULL)
		{
			/* store a pointer to the keyboard */
			BranchBackStack[x] = pKeyboard;
			gok_log ("first keyboard on the stack");
			gok_log_leave();
			return;
		}
		else
		{
			/* hack bugfix */
			if ((BranchBackStack[x] == pKeyboard) && (pKeyboard->bDynamicallyCreated == TRUE))
			{
				gok_log_x ("this dynamic keyboard already on stack.");
				m_PushesMinusPops--;
				gok_log_leave();
				return;
			}
		}
	}

	/* stack is completly full */
	/* move all the items down (which discards first item in the stack) */
	for (x = 0; x < (MAX_BRANCHBACKSTACK - 1); x++)
	{
		BranchBackStack[x] = BranchBackStack[x + 1];
	}

	/* add the new keyboard as the last item in the stack */
	BranchBackStack[MAX_BRANCHBACKSTACK - 1] = pKeyboard;
	gok_log_leave();
}

/**
* gok_branchbackstack_pop
*
* Remove the last keyboard in the stack and return it.
*
* returns: A pointer to the keyboard popped off the stack, NULL if the stack is empty.
**/
GokKeyboard* gok_branchbackstack_pop ()
{
	gint x;
	GokKeyboard* pKeyboard;

	gok_log_enter();

	/* start at the end of the stack and find the first keyboard */
	for (x = (MAX_BRANCHBACKSTACK - 1); x >= 0 ; x--)
	{
		if (BranchBackStack[x] != NULL)
		{
			pKeyboard = BranchBackStack[x];
			BranchBackStack[x] = NULL;
			gok_log ("popping keyboard [%s] from stack",pKeyboard->Name);
			m_PushesMinusPops--;
			gok_log_leave();
			return pKeyboard;
		}
	}
	gok_log_leave();
	
	return NULL;
}

gint gok_branchbackstack_pushes_minus_pops (void)
{
	return m_PushesMinusPops;
}

gboolean gok_branchbackstack_contains (GokKeyboard* pKeyboard)
{
	gint x;
	gok_log_enter();

	for (x = 0; x < MAX_BRANCHBACKSTACK; x++)
	{
		if (BranchBackStack[x] == pKeyboard)
		{
			gok_log("yes");
			gok_log_leave();
			return TRUE;
		}
	}
			gok_log("no");
	gok_log_leave();
	
	return FALSE;
}
