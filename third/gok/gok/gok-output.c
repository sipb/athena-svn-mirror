/* gok-output.c
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
 
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#include "gok-output.h"
#include "gok-log.h"
#include "gok-word-complete.h"
#include "main.h"
#include "gok-gconf-keys.h"

static gboolean
gok_output_can_predict (void)
{
	GokKeyboard *kbd = gok_main_get_current_keyboard ();
	Accessible *acc_with_text = gok_spy_get_accessibleWithText ();
	gboolean is_password = (acc_with_text && 
	    Accessible_getRole (acc_with_text) == SPI_ROLE_PASSWORD_TEXT);
	return (kbd && !gok_main_get_login() && !is_password && gok_data_get_wordcomplete () &&
		gok_keyboard_get_supports_wordcomplete (kbd));
}

/**
 * gok_output_update_predictions:
 * Send new data to the predictor based on output type, and update the keyboard's prediction keys. 
 **/
static void
gok_output_update_predictions (GokOutput    *output,
			       gpointer      data)
{
	gunichar letter;
	gchar *string;
	gchar **prediction_list = NULL;
	gchar *part = NULL;
	GokWordComplete *complete = gok_wordcomplete_get_default ();

	if (output) {
		switch (output->Type) 
		{
		case OUTPUT_KEYCODE:
		case OUTPUT_KEYSYM:
			prediction_list = gok_wordcomplete_process_and_predict (complete,
										(gunichar) data,
										gok_data_get_num_predictions ());
			part = gok_wordcomplete_get_word_part (complete);
			if (gok_wordcomplete_validate_word (complete, part)) { 
			    /* don't offer to add a word that's already in the dictionary */
			    part = NULL;
			}
			gok_keyboard_set_predictions (gok_main_get_current_keyboard (),
						      prediction_list, 
						      part);
			break;
		case OUTPUT_KEYSTRING:
			string = (gchar *) data;
			if (string && g_utf8_validate (string, -1, NULL)) 
			{
			        letter = g_utf8_get_char (string);
				while (letter && string) 
				{
					if (prediction_list)
					{
						int i;
						for (i = 0; prediction_list && prediction_list[i]; ++i)
							g_free (prediction_list[i]);
						g_free (prediction_list);
					}

					prediction_list = gok_wordcomplete_process_and_predict (
						complete,
						letter,
						gok_data_get_num_predictions ());
					string = g_utf8_next_char (string);
					letter = g_utf8_get_char (string);
				}
				part = gok_wordcomplete_get_word_part (complete);
				if (gok_wordcomplete_validate_word (complete, part)) { 
				    /* don't offer to add a word that's already in the dictionary */
				    part = NULL;
				}
				gok_keyboard_set_predictions (gok_main_get_current_keyboard (), 
							      prediction_list,
							      part);				
			}
			break;
		default:
			break;
		}
	}
}

static KeySym last_dead_key = 0;

gunichar gok_output_unichar_from_keysym (KeySym keysym, guint mods)
{
	char *charstring;
	if ((keysym >= 0xFF08) && (keysym < 0xFF20)) { /* X11 control codes, etc. */
		last_dead_key = 0;
		return (gunichar) keysym & 0x00FF; /* mask out the high bits */
	}
	else if ((keysym >= 0xFE50) && (keysym <= 0xFE62)) { /* deadkey: save to apply later */
		last_dead_key = keysym;
		return 0;
	}
	else {
		long ucs;
		if ((ucs = keysym2ucs (keysym)) > 0) 
		{
			return (gunichar) ucs;
		}
		return 0;
	}
}

/**
* gok_output_delete_all
* @pOutput: Pointer to the GokOutput that you want deleted.
*
* Deletes the given GokOutput and other GokOutputs that are in its list.
**/
void gok_output_delete_all (GokOutput* pOutput)
{
	GokOutput* pOutputTemp;
	
	while (pOutput != NULL)
	{
		pOutputTemp = pOutput;
		pOutput = pOutput->pOutputNext;
		g_free (pOutputTemp->Name);
		g_free (pOutputTemp);
	}
}

/**
* gok_output_new
* @Type: Type of output (e.g. keysym or keycode).
* @Name: Keycode, Keysym, or UTF-8 string.
* @Flag:
*
* Creates a new GokOutput and initializes it to the given values.
*
* returns: A pointer to the new GokOutput, NULL if not created. 
**/
GokOutput* gok_output_new (gint Type, gchar* Name, AccessibleKeySynthType Flag)
{
	GokOutput* pNewOutput;
	
	if (Name == NULL)
	{
		return NULL;
	}

	pNewOutput = (GokOutput*)g_malloc (sizeof (GokOutput));
	pNewOutput->pOutputNext = NULL;
	pNewOutput->Flag = Flag;
	pNewOutput->Type = Type;
	pNewOutput->Name = (gchar*)g_malloc (strlen (Name) + 1);
	strcpy (pNewOutput->Name, Name);

	return pNewOutput;
}

/**
* gok_output_new_from_xml
* @pNode: Pointer to the XML node.
*
* Creates a new GokOutput and initializes it from the given XML node.
*
* returns: A pointer to the new GokOutput, NULL if not created. 
**/
GokOutput* gok_output_new_from_xml (xmlNode* pNode)
{
	xmlChar* pStringAttributeValue;
	xmlChar* pStringFlag;
	gint typeOutput;
	AccessibleKeySynthType flagOutput;

	g_assert (pNode != NULL);
		
	pStringAttributeValue = xmlGetProp (pNode, (const xmlChar *) "type");
	if (pStringAttributeValue == NULL)
	{
		gok_log_x ("Output '%s' has no type!\n", xmlNodeGetContent (pNode));
		return NULL;
	}
	
	if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"keycode") == 0)
	{
		typeOutput = OUTPUT_KEYCODE;
		
		pStringFlag = xmlGetProp (pNode, (const xmlChar *) "flag");
		if (pStringFlag == NULL)
		{
			gok_log_x ("Output '%s'is keycode but has no flag!\n", xmlNodeGetContent (pNode));
		}
		else
		{
			if (xmlStrcmp (pStringFlag, (const xmlChar *)"press") == 0)
			{
				flagOutput = SPI_KEY_PRESS;
			}
			else if (xmlStrcmp (pStringFlag, (const xmlChar *)"release") == 0)
			{
				flagOutput = SPI_KEY_RELEASE;
			}
			else if (xmlStrcmp (pStringFlag, (const xmlChar *)"pressrelease") == 0)
			{
				flagOutput = SPI_KEY_PRESSRELEASE;
			}
			else
			{
				gok_log_x ("Output flag '%s' is invalid!\n", pStringFlag);
			}
		}
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"keysym") == 0)
	{
		typeOutput = OUTPUT_KEYSYM;
		flagOutput = SPI_KEY_PRESSRELEASE;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"keystring") == 0)
	{
		typeOutput = OUTPUT_KEYSTRING;
		flagOutput = SPI_KEY_PRESSRELEASE;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"exec") == 0)
	{
		typeOutput = OUTPUT_EXEC;
		flagOutput = SPI_KEY_PRESSRELEASE;
	}
	else if (xmlStrcmp (pStringAttributeValue, (const xmlChar *)"internal") == 0)
	{
		typeOutput = OUTPUT_INTERNAL;
		flagOutput = SPI_KEY_PRESSRELEASE;  /* not necessary? */
	}
	else
	{
		gok_log_x ("Output type '%s' not correct!\n", pStringAttributeValue);
		return NULL;
	}

	return gok_output_new (typeOutput, g_strstrip (xmlNodeGetContent (pNode)), flagOutput);
}

static gunichar
gok_output_unichar_postfix_from_deadkey (KeySym deadkey)
{
	switch (deadkey) 
	{
	case XK_dead_grave:
		return 0x0300;
	case XK_dead_acute:
		return 0x0301;
	case XK_dead_circumflex:
		return 0x0302;
	case XK_dead_tilde:
		return 0x0303;
	case XK_dead_macron:
		return 0x0304;
	case XK_dead_breve:
		return 0x0306;
	case XK_dead_abovedot:
		return 0x0307;
	case XK_dead_diaeresis:
		return 0x0308;
	case XK_dead_abovering:
		return 0x030a;
	case XK_dead_doubleacute:
		return 0x030b;
	case XK_dead_caron:
		return 0x030c;
	case XK_dead_cedilla:
		return 0x0327;
	case XK_dead_ogonek:
		return 0x0328;
	case XK_dead_iota:
		return 0x0345;
	case XK_dead_voiced_sound:
		return 0x032c;
	case XK_dead_semivoiced_sound:
		return 0x0324;
	case XK_dead_belowdot:
		return 0x0323;
/* ifdefs needed because XSun doesn't include these keysyms */
#ifdef XK_dead_hook
	case XK_dead_hook:
		return 0x0309;
#endif
#ifdef XK_dead_horn
	case XK_dead_horn:
		return 0x031b;
#endif
	default:
		return 0;
	}
}


/**
* gok_output_send_to_system
* @pOutput: Pointer to a GokOutput that will be sent to the system.
*
* Sends the given GokOutput to the system. All other GokOutputs that are
* linked to this output are also sent.
**/
void gok_output_send_to_system (GokOutput* pOutput, gboolean bWordCompletion)
{
	long keysym;
	long keycode;
	gunichar unichar;

	gok_log_enter();
	while (pOutput != NULL)
	{
		if (pOutput->Type == OUTPUT_KEYSYM)
		{
		        if (g_str_has_prefix (pOutput->Name, "U+"))
		        {
			        keysym = 0x01000000 |
				    g_ascii_strtoull ((gchar *) (pOutput->Name + 2), NULL, 16);
		        } 
		        else
		        {
			        keysym = XStringToKeysym (pOutput->Name);
		        }
			
			gok_log ("Sending a keysym");
			gok_log ("pOutput->Name = %s", pOutput->Name);
			gok_log ("SPI_KEY_SYM = %s [%d]", pOutput->Name, (int) keysym);
			SPI_generateKeyboardEvent ((long) keysym, NULL, SPI_KEY_SYM);

			/* update the word prediction keys with the new output */
			if (bWordCompletion && gok_output_can_predict ()) {
				unichar = gok_output_unichar_from_keysym (keysym, 
									  gok_spy_get_modmask ());
				if (last_dead_key && unichar) {
					gchar *s;
					s = gok_wordcomplete_process_unichar (gok_wordcomplete_get_default (), unichar);
					gok_output_update_predictions (pOutput, 
								       (gpointer) gok_output_unichar_postfix_from_deadkey (last_dead_key));
					last_dead_key = 0;
				}
				else if (unichar) {
					gok_output_update_predictions (pOutput, 
								       (gpointer) unichar);
				}
			}
		}
		else if (pOutput->Type == OUTPUT_KEYCODE)
		{
			XkbDescRec *xkb = gok_keyboard_get_xkb_desc ();
			keycode = strtol (pOutput->Name, NULL, 0);

			gok_log ("Sending a keycode");
			gok_log ("pOutput->Name = %s", pOutput->Name);
			gok_log ("pOutput->Flag = %d", (int) pOutput->Flag);

			SPI_generateKeyboardEvent ((long) keycode, NULL, pOutput->Flag);

			/* update the word prediction keys with the new output */
			if (bWordCompletion && gok_output_can_predict () &&
			    !gok_key_modifier_for_keycode (
				    gok_main_display (),
				    xkb,
				    keycode)) 
			{
				guint mask = gok_spy_get_modmask ();
				gint group = gok_key_get_effective_group ();
				int type = gok_key_get_xkb_type_index (xkb, keycode, group);
			        int level = gok_key_level_for_type (gok_keyboard_get_display (),
								    xkb, 
								    type, &mask);

			        keysym = XkbKeycodeToKeysym (gok_main_display (), 
							   keycode, group, level);
				
				unichar = gok_output_unichar_from_keysym (keysym, 
									  gok_spy_get_modmask ());
				if (last_dead_key && unichar) {
					gchar *s;
					s = gok_wordcomplete_process_unichar (gok_wordcomplete_get_default (), unichar);
					gok_output_update_predictions (pOutput, 
								       (gpointer) gok_output_unichar_postfix_from_deadkey (last_dead_key));
					last_dead_key = 0;
				}
				else if (unichar) {
					gok_output_update_predictions (pOutput, 
								       (gpointer) unichar);
				}
			}
		}
		else if (pOutput->Type == OUTPUT_KEYSTRING)
		{
			Accessible *focussed_object = gok_spy_get_focussed_object ();
			AccessibleStateSet *state = NULL;
			gok_log ("Sending a string");
			gok_log ("pOutput->Name = SPI_KEY_STRING = %s", pOutput->Name);
			
			if (focussed_object && Accessible_isEditableText (focussed_object) &&
				(state = Accessible_getStateSet (focussed_object)) && 
				AccessibleStateSet_contains (state, SPI_STATE_FOCUSED)) 
			{
				AccessibleText *text = Accessible_getText (focussed_object);
				AccessibleEditableText *editable = Accessible_getEditableText (focussed_object);
				AccessibleEditableText_insertText (editable, AccessibleText_getCaretOffset (text),
								   pOutput->Name, 
								   strlen (pOutput->Name));
				AccessibleText_unref (text);
				AccessibleEditableText_unref (editable);
			}
			else
			{
			    SPI_generateKeyboardEvent (0, pOutput->Name, SPI_KEY_STRING);
			}
			if (state) AccessibleStateSet_unref (state);
			/* update the word prediction keys with the new output */
			if (bWordCompletion == TRUE && gok_output_can_predict ())
			{
				gok_output_update_predictions (pOutput,
							       pOutput->Name);
			}
		}
		else if (pOutput->Type == OUTPUT_EXEC)
		{
			gok_log ("g_spawn_command_line_async(\"%s\")",
				pOutput->Name);

			/* check special cases */
			if (strcmp("default:browser",pOutput->Name) == 0) {
				/* special: default browser */
				gchar* browser = NULL;
				gok_gconf_get_string (gconf_client_get_default (),
						GOK_GCONF_DEFAULT_BROWSER, &browser);
				g_spawn_command_line_async (browser, NULL);
			}
			else {
				/* not special */
				g_spawn_command_line_async (pOutput->Name, NULL);
			}
		}
		else if (pOutput->Type == OUTPUT_INTERNAL)
		{
			gok_log ("Internal command: %s", pOutput->Name);
			gok_output_internal( pOutput->Name );
		}
		else
		{
			gok_log_x ("Output has invalid type '%d'!\n", pOutput->Type);
		}
		pOutput = pOutput->pOutputNext;
	}
	gok_log_leave();
}

/**
 *
 * Returns keycode for OUTPUT_KEYCODE and OUTPUT_KEYSYM keys, or -1.
 **/
int 
gok_output_get_keycode (GokOutput *output)
{
    int keycode = -1;
    if (output && output->Type == OUTPUT_KEYCODE)
	keycode = atoi (output->Name);
    else if (output && output->Type == OUTPUT_KEYSYM)
    {
	keycode = XKeysymToKeycode (gok_main_display (), 
				    XStringToKeysym (output->Name));
    }
    return keycode;
}

void
gok_output_internal (gchar* action)
{
	if (strcmp (action, "quit") == 0)
	{
		/* quit gok */
		gok_main_close();
	}
	else
	{
		gok_log_x("Unknown gok internal action: %s", action);
	}
}
