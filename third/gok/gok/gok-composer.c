/* gok-composer.c
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

/* 
   This file provides implementations for the static "Text Manipulation"  keyboard that 
   provides gok access to accessible text interface methods
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "gok-composer.h"
#include "gok-log.h"
#include "main.h"
#include "gok-spy.h"

typedef enum {
	TEXT_COMMAND_NAVIGATE,
	TEXT_COMMAND_SELECT,
	TEXT_COMMAND_CUT,
	TEXT_COMMAND_COPY,
	TEXT_COMMAND_PASTE
} GokTextCommandType;

typedef enum {
	TEXT_CHUNK_CHAR,
	TEXT_CHUNK_WORD,
	TEXT_CHUNK_LINE,
	TEXT_CHUNK_SENTENCE,
	TEXT_CHUNK_SELECTION,
	TEXT_CHUNK_ALL
} GokTextChunkType;

typedef struct {
	GokTextCommandType type;
	GokTextChunkType range_type;
	gboolean backward;
} GokTextCommand;


static gboolean select_mode = FALSE;

/**
 *
 *
 *
 **/
void
gok_composer_validate (gchar *keyboard_name, Accessible *accessible)
{
	GokKeyboard *keyboard = gok_main_keyboard_find_byname (keyboard_name);

	/* TODO: find the edit keys and make them insensitive if the accessible isn't editable */
	gboolean is_editable = Accessible_isEditableText (accessible);

	if (!Accessible_isEditableText (accessible)) {
		GokKey *key = keyboard->pKeyFirst;

		while (key != NULL)
		{
			if (key->Type == KEYTYPE_EDIT)
			{
				key->Style == KEYSTYLE_INSENSITIVE;
			}
			key = key->pKeyNext;
		}
	}
	/* FIXME: global again, should be in GokKeyboard struct */
	select_mode = FALSE;
}

/**
 *
 *
 *
 **/
void
gok_compose_key_init (GokKey *key, xmlNode *node)
{
	xmlChar *attribute;
	GokTextCommand *command;

	attribute = xmlGetProp (node, (const xmlChar *) "detail");
	command = g_new0 (GokTextCommand, 1);

	if (attribute != NULL) {
		switch (key->Type) {
		case KEYTYPE_TEXTNAV:
			command->type = TEXT_COMMAND_NAVIGATE;
			if (xmlStrcmp (attribute, (const xmlChar *) "char-") == 0) {
				command->range_type = TEXT_CHUNK_CHAR;
				command->backward = TRUE;
			}
			else if (xmlStrcmp (attribute, (const xmlChar *) "char+") == 0) {
				command->range_type = TEXT_CHUNK_CHAR;
				command->backward = FALSE;
			}
			else if (xmlStrcmp (attribute, (const xmlChar *) "word-") == 0) {
				command->range_type = TEXT_CHUNK_WORD;
				command->backward = TRUE;
			}
			else if (xmlStrcmp (attribute, (const xmlChar *) "word+") == 0) {
				command->range_type = TEXT_CHUNK_WORD;
				command->backward = FALSE;
			}
			else if (xmlStrcmp (attribute, (const xmlChar *) "line-") == 0) {
				command->range_type = TEXT_CHUNK_LINE;
				command->backward = TRUE;
			}
			else if (xmlStrcmp (attribute, (const xmlChar *) "line+") == 0) {
				command->range_type = TEXT_CHUNK_LINE;
				command->backward = FALSE;
			}
			else if (xmlStrcmp (attribute, (const xmlChar *) "sent-") == 0) {
				command->range_type = TEXT_CHUNK_SENTENCE;
				command->backward = TRUE;
			}
			else if (xmlStrcmp (attribute, (const xmlChar *) "sent+") == 0) {
				command->range_type = TEXT_CHUNK_SENTENCE;
				command->backward = FALSE;
			}
			else
				key->Style = KEYSTYLE_INSENSITIVE;
			break;
		case KEYTYPE_EDIT:
			command->range_type = TEXT_CHUNK_SELECTION;
			if (xmlStrcmp (attribute, (const xmlChar *) "cut") == 0) {
				command->type = TEXT_COMMAND_CUT;
			}
			else if (xmlStrcmp (attribute, (const xmlChar *) "copy") == 0) {
				command->type = TEXT_COMMAND_COPY;
			}
			else if (xmlStrcmp (attribute, (const xmlChar *) "paste") == 0) {
				command->type = TEXT_COMMAND_PASTE;
			}
			else {
				key->Style = KEYSTYLE_INSENSITIVE;
			}
			break;
		case KEYTYPE_SELECT:
			command->type = TEXT_COMMAND_SELECT;
			command->backward = FALSE;
			if (xmlStrcmp (attribute, (const xmlChar *) "word") == 0)
				command->range_type = TEXT_CHUNK_WORD;
			else if (xmlStrcmp (attribute, (const xmlChar *) "sent") == 0)
				command->range_type = TEXT_CHUNK_SENTENCE;
			else if (xmlStrcmp (attribute, (const xmlChar *) "all") == 0)
				command->range_type = TEXT_CHUNK_ALL;
			else
				key->Style = KEYSTYLE_INSENSITIVE;
			break;
		case KEYTYPE_TOGGLESELECT:
			break;
		default:
			break;
		}
	}
	key->pGeneral = command;
}

/**
* gok_composer_branch_textAction
* @pKeyboard: pointer to the keyboard.
* @pKey: pointer to the key
*
* Performs a text manipulation/navigation action.
*
* returns: TRUE if the action call was made.
**/
gboolean gok_composer_branch_textAction (GokKeyboard* pKeyboard, GokKey* pKey)
{
	/* TODO: Refactor, this method has gotten too long! */
	Accessible* paccessible;
	AccessibleEditableText* peditableText;
	AccessibleText* paccessibleText = NULL;
	long currentCaretPos;
	long selectionStart;
	long selectionEnd;
	long rangeStart;
	long rangeEnd;
	gboolean returncode = FALSE;
	gboolean is_editable = TRUE;
	AccessibleTextBoundaryType boundary_type = SPI_TEXT_BOUNDARY_CHAR;
	GokTextCommand *command;
	
	currentCaretPos = 0;

	gok_log_enter();
	
	/* do we have an accessible object? */
	paccessible = gok_keyboard_get_accessible ( pKeyboard );
	if (paccessible == NULL)
	{
		gok_log ("no Accessible object -- giving up\n");
		gok_log_leave ();
		return FALSE;
	}
	/* do we have an accessible text interface? */
	if (paccessible) {
		paccessibleText = Accessible_getText ( paccessible );
	}

	if ( paccessibleText == NULL)
	{
		fprintf (stderr, "no AccessibleText object - aborting");
		gok_log_x("no AccessibleText object - aborting");
		gok_log_leave();
		return FALSE;
	}
	gok_spy_accessible_implicit_ref(paccessible);

	/* do we have an editabletext interface? */
	peditableText = Accessible_getEditableText ( paccessible );
	if (peditableText == NULL)
	{
		is_editable = FALSE;
	}

	currentCaretPos = AccessibleText_getCaretOffset( paccessibleText );
	fprintf (stderr, "starting caret position is %d\n", currentCaretPos);
	if (currentCaretPos < 0)
	{
		currentCaretPos = 0;
	}
	
	if (AccessibleText_getNSelections (paccessibleText) > 0) {
		AccessibleText_getSelection( paccessibleText, 0, &selectionStart, &selectionEnd );
		fprintf (stderr, "text from %ld to %ld is already selected.\n", selectionStart, selectionEnd);
	} 
	else {
		selectionStart = selectionEnd = currentCaretPos;
	}

	command = (GokTextCommand *) pKey->pGeneral;

	if (!command) {
		gok_log_x ("no command data in pGeneral storage");	
		gok_log_leave();
		return FALSE;
	}

	switch (pKey->Type)
	{
	case KEYTYPE_TEXTNAV:
		switch (command->range_type) {
		case TEXT_CHUNK_WORD:
			boundary_type = ((command->backward) ? 
					 SPI_TEXT_BOUNDARY_WORD_START : 
					 SPI_TEXT_BOUNDARY_WORD_END);
			break;
		case TEXT_CHUNK_LINE:
			boundary_type = ((command->backward) ? 
					 SPI_TEXT_BOUNDARY_LINE_START : 
					 SPI_TEXT_BOUNDARY_LINE_END);
			break;
		case TEXT_CHUNK_SENTENCE:
			boundary_type = ((command->backward) ? 
					 SPI_TEXT_BOUNDARY_SENTENCE_START : 
					 SPI_TEXT_BOUNDARY_SENTENCE_END);
			break;
		case TEXT_CHUNK_CHAR:
		default:
			boundary_type = SPI_TEXT_BOUNDARY_CHAR;
			break;
		}
		break;
	case KEYTYPE_SELECT:
		switch (command->range_type) {
		case TEXT_CHUNK_WORD:
			boundary_type = SPI_TEXT_BOUNDARY_WORD_END;
			break;
		case TEXT_CHUNK_SENTENCE:
			boundary_type = SPI_TEXT_BOUNDARY_SENTENCE_END;
			break;
		case TEXT_CHUNK_LINE:
			boundary_type = SPI_TEXT_BOUNDARY_LINE_END;
			break;
		case TEXT_CHUNK_CHAR:
		default:
			boundary_type = SPI_TEXT_BOUNDARY_CHAR;
			break;
		}
		command->backward = FALSE;
		break;
	case KEYTYPE_EDIT:
		switch (command->type) {
		case TEXT_COMMAND_CUT:
			gok_log ("cut");	
			returncode = (is_editable &&
				      AccessibleEditableText_cutText ( peditableText, 
								       selectionStart, selectionEnd ));
			break;
		case TEXT_COMMAND_COPY:
			gok_log ("copy");
			/* TODO: use getText and send to system buffer if is_editable==FALSE */
			returncode = (is_editable &&
				      AccessibleEditableText_copyText ( peditableText, 
									selectionStart, selectionEnd ));
			break;
		case TEXT_COMMAND_PASTE:
			gok_log ("paste");	
			returncode = (is_editable && 
				      AccessibleEditableText_pasteText ( peditableText, 
									 currentCaretPos ));
			break;
		}
		break;
	case KEYTYPE_TOGGLESELECT:
		select_mode = !select_mode;
		pKey->ComponentState.active = select_mode;
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pKey->pButton), 
					      select_mode);
		return TRUE;
		break;
	default:
		gok_log_leave ();
		return FALSE;
		break;
	}

	if ((command->type == TEXT_COMMAND_NAVIGATE) || (command->type == TEXT_COMMAND_SELECT)) {
		if (command->backward && (currentCaretPos > 0))
		{
			--currentCaretPos;
		}
		else if (command->range_type == TEXT_CHUNK_LINE)
		{
			++currentCaretPos; /* we were already at end-of-line, must increment */
		}
		AccessibleText_getTextAtOffset (paccessibleText,
						currentCaretPos,
						boundary_type,
						&rangeStart,
						&rangeEnd);
	}
	if (command->type == TEXT_COMMAND_NAVIGATE) {
		if (command->backward) 
			currentCaretPos = rangeStart;
		else 
			currentCaretPos = rangeEnd;
		returncode = AccessibleText_setCaretOffset (paccessibleText,
							    currentCaretPos);
		fprintf (stderr, "setting caret position to %d\n", currentCaretPos);
	}
	if ((command->type == TEXT_COMMAND_SELECT) || (select_mode == TRUE)) {
		if (command->type == TEXT_COMMAND_SELECT) {
			if (command->range_type == TEXT_CHUNK_ALL) {
				selectionStart = 0;
				selectionEnd = AccessibleText_getCharacterCount (paccessibleText);
			}
			else { 
				/* we want to clip the whitespace, so a second range call to 'START' of range is needed */
				switch (command->range_type) {
				case TEXT_CHUNK_WORD:
					AccessibleText_getTextAtOffset (paccessibleText,
									currentCaretPos, 
									SPI_TEXT_BOUNDARY_WORD_START,
									&selectionStart,
									&selectionEnd);
					break;
				case TEXT_CHUNK_SENTENCE:
					AccessibleText_getTextAtOffset (paccessibleText,
									currentCaretPos, 
									SPI_TEXT_BOUNDARY_SENTENCE_START,
									&selectionStart,
									&selectionEnd);
					break;
				default:
					selectionStart = rangeStart;
					selectionEnd = rangeEnd;
					break;
				}
				selectionStart = MAX (selectionStart, rangeStart);
				selectionEnd = MIN (selectionEnd, rangeEnd);
			}
		}
		else { /* just select as we go */
			long tmp;
			if ((command->type == TEXT_COMMAND_NAVIGATE) && (command->backward)) {
				tmp = MIN (selectionStart, currentCaretPos);
				selectionStart = MAX (selectionEnd, currentCaretPos);
				selectionEnd = tmp;
			}
			else {
				selectionStart = MIN (selectionStart, currentCaretPos);
				selectionEnd = MAX (selectionEnd, currentCaretPos);
			}
		}
		fprintf (stderr, "selecting text from %ld %ld\n", selectionStart, selectionEnd);
		if (AccessibleText_getNSelections(paccessibleText) != 0) {
			returncode = AccessibleText_setSelection(paccessibleText, 
								 0, 
								 selectionStart,
								 selectionEnd);
		}
		else {
			returncode = AccessibleText_addSelection(paccessibleText, 
								 selectionStart, 
								 selectionEnd);
		}
	}
	AccessibleEditableText_unref (peditableText);
	AccessibleText_unref(paccessibleText);
	gok_log_leave();
	return returncode;
}
