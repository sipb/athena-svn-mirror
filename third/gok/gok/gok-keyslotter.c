/* gok-keyslotter.c
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

#include "gok-keyslotter.h"
#include "main.h"
#include "gok-data.h"
#include "gok-word-complete.h"
#include "gok-log.h"

/* programmer's note: this could do with some more generalization work */

/**
* gok_keyslotter_open
* 
* Opens and initializes the appropriate engine.
*
* returns: TRUE if it was opend OK, FALSE if not.
**/
gboolean
gok_keyslotter_on (gboolean bOnOff, int keytype)
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;

	gok_log_enter();

	/* find all the keyboards that support word completion */
	pKeyboard = gok_main_get_first_keyboard();
	g_assert (pKeyboard != NULL);
	while (pKeyboard != NULL)
	{
		if (((keytype == KEYTYPE_WORDCOMPLETE) && (gok_keyboard_get_supports_wordcomplete (pKeyboard) == TRUE)) ||
		    ((keytype == KEYTYPE_COMMANDPREDICT) && (gok_keyboard_get_supports_commandprediction (pKeyboard) == TRUE)) )
		{
			if (bOnOff)
			{
				if (((keytype == KEYTYPE_WORDCOMPLETE) && (gok_keyboard_get_wordcomplete_keys_added (pKeyboard) == FALSE)) ||
				    ((keytype == KEYTYPE_COMMANDPREDICT) && (gok_keyboard_get_commandpredict_keys_added (pKeyboard) == FALSE)) )
				{
					/* add the word completion keys to the keyboard */
					/* first, move all the keys down one row */
					pKey = pKeyboard->pKeyFirst;
					while (pKey != NULL)
					{
						pKey->Top += 1;
						pKey->Bottom += 1;
						
						pKey = pKey->pKeyNext;
					}

					/* add new slot keys */
					pKeyboard->NumberRows += 1;
					if (keytype == KEYTYPE_WORDCOMPLETE)
					{
						gok_keyslotter_add_prediction_keys (pKeyboard, keytype);
					}
					else if (keytype == KEYTYPE_COMMANDPREDICT)
					{
						gok_keyslotter_add_prediction_keys (pKeyboard, keytype);
					}
				}
			}
			else /* remove the slot keys from the keyboard */
			{
				if (((keytype == KEYTYPE_WORDCOMPLETE) && (gok_keyboard_get_wordcomplete_keys_added (pKeyboard) == TRUE)) ||
				    ((keytype == KEYTYPE_COMMANDPREDICT) && (gok_keyboard_get_commandpredict_keys_added (pKeyboard) == TRUE)) )
				{
					pKey = pKeyboard->pKeyFirst;
					while (pKey != NULL)
					{
						if (pKey->Type == keytype)
						{
							gok_key_delete(pKey, pKeyboard, TRUE);
						}
						else
						{
							/* move all the other keys up one row */
							pKey->Top -= 1;
							pKey->Bottom -= 1;
						}
						
						pKey = pKey->pKeyNext;
					}
					if (keytype == KEYTYPE_WORDCOMPLETE)
					{
						gok_keyboard_set_wordcomplete_keys_added (pKeyboard, FALSE);
					}
					else if (keytype == KEYTYPE_COMMANDPREDICT)
					{
						gok_keyboard_set_commandpredict_keys_added (pKeyboard, FALSE);
					}
					pKeyboard->NumberRows -= 1;
				}
			}
		}
		pKeyboard = pKeyboard->pKeyboardNext;
	}
	gok_log_leave();

	return TRUE;
}

/**
* gok_keyslotter_change_number_predictions
*
* @Number: Number of prediction keys.
*
* Changes the number of prediction keys displayed on the keyboard. This
* should be called after the user has selected a new number from the settings
* dialog.
*
* returns: void
**/
void gok_keyslotter_change_number_predictions (int Number, int keytype)
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	
	/* don't do anything unless the feature is turned on */
	if (gok_data_get_wordcomplete() == FALSE)
	{
		gok_log("word completion is off");
		return;
	}
	
	/* find all the keyboards that support keytype */
	pKeyboard = gok_main_get_first_keyboard();
	g_assert (pKeyboard != NULL);
	while (pKeyboard != NULL)
	{
		if ( ((keytype == KEYTYPE_WORDCOMPLETE) &&( gok_keyboard_get_supports_wordcomplete (pKeyboard) == TRUE)) ||
		     ((keytype == KEYTYPE_COMMANDPREDICT) &&( gok_keyboard_get_supports_commandprediction (pKeyboard) == TRUE) ))
		{
			/* remove all the word completion keys from the keyboard */
			pKey = pKeyboard->pKeyFirst;
			while (pKey != NULL)
			{
				if (pKey->Type == keytype)
				{
					gok_key_delete(pKey, pKeyboard, TRUE);
				}
				pKey = pKey->pKeyNext;
			}
			
			/* add new slot keys */
			gok_keyslotter_add_prediction_keys (pKeyboard, keytype);
		}
		pKeyboard = pKeyboard->pKeyboardNext;
	}
}


/**
* gok_keyslotter_add_prediction_keys
*
* @pKeyboard: Pointer to the keyboard that gets the new prediction keys.
*
* Adds a row of prediction keys to the given keyboard.
*
* returns: TRUE if the prediction keys were added, FALSE if not.
**/
gboolean gok_keyslotter_add_prediction_keys (GokKeyboard* pKeyboard, int keytype)
{
	GokKey* pKeyNew;
	GokKey* pKeyPrevious, *pKeyNext;
	gint maxcols;
	gint colperkey;
	gint colleft;
	gint colright;
	gint count;
	gint numPredictions;

	gok_log_enter();
	
	pKeyPrevious = pKeyboard->pKeyFirst;
	if (pKeyPrevious) 
	    pKeyNext = pKeyPrevious->pKeyNext;

	numPredictions = gok_data_get_num_predictions();
	maxcols = gok_keyboard_get_number_columns (pKeyboard);
	if (numPredictions > maxcols)
	{
		gok_log("reducing number of predictions to fit the keyboard.");
		numPredictions = maxcols;
	}

	/* calculate the size of each key */
	colperkey = maxcols / (keytype == KEYTYPE_WORDCOMPLETE ? (numPredictions + 1) : numPredictions);

	/* add new keys */
	colleft = 0;
	colright = colperkey;
	for (count = 0; count < numPredictions; count++)
	{
		pKeyNew = gok_key_new (pKeyPrevious, 
				       (keytype == KEYTYPE_WORDCOMPLETE || (count != numPredictions - 1)) ? NULL : pKeyNext, 
				       pKeyboard);
						
		pKeyNew->Top = 0;
		pKeyNew->Bottom = 1;
		pKeyNew->Left = colleft;
		pKeyNew->Right = colright;
		pKeyNew->FontSizeGroup = FONT_SIZE_GROUP_WORDCOMPLETE; /* TODO? */
		pKeyNew->Type = keytype;
		pKeyNew->Style = KEYSTYLE_WORDCOMPLETE; /* TODO for command prediction use original keys style.*/

		colleft = colright;
		colright += colperkey;
						
		pKeyPrevious = pKeyNew;
	}
	if (keytype == KEYTYPE_WORDCOMPLETE) 
	{
	    /* add an ADDWORD key */
	    pKeyNew = gok_key_new (pKeyPrevious, pKeyNext, pKeyboard);
	    
	    pKeyNew->Top = 0;
	    pKeyNew->Bottom = 1;
	    pKeyNew->Left = colleft;
	    pKeyNew->Right = colright;
	    pKeyNew->FontSizeGroup = FONT_SIZE_GROUP_UNIQUE;
	    pKeyNew->Type = KEYTYPE_ADDWORD;
	    pKeyNew->Style = KEYSTYLE_ADDWORD; 
	    
	    colleft = colright;
	    colright += colperkey;
	    
	    pKeyPrevious = pKeyNew;
	}
					
	/* make the prediction keys fill the row */
	gok_keyboard_fill_row (pKeyboard, 0);
	
	if (keytype == KEYTYPE_WORDCOMPLETE)
	{
		gok_keyboard_set_wordcomplete_keys_added (pKeyboard, TRUE);
	}
	else if (keytype == KEYTYPE_COMMANDPREDICT)
	{
		gok_keyboard_set_commandpredict_keys_added (pKeyboard, TRUE);
	}

	gok_log_leave();
	return TRUE;
}
