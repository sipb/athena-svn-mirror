/* gok-word-complete.c
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
* To use this thing:
* - Call "gok_wordcomplete_open". If it returns TRUE then you're ready to go.
* - Call "gok_wordcomplete_predict" to make the word predictions.
* - Call "gok_wordcomplete_close" when you're done. 
* - To add a word, call "gok_wordcomplete_add_new_word".
*
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <locale.h>

#include "gok-word-complete.h"
#include "gok-keyslotter.h"
#include "main.h"
#include "gok-log.h"
#include "gok-modifier.h"
#include "gok-data.h"
#include "gok-keyboard.h"

#include "word-complete.h" /* TODO: remove this when we make this class abstract */
#include "gok-utf8-word-complete.h"
#include "gok-sliding-window-word-complete.h"

static void gok_wordcomplete_finalize (GObject *obj);
static void gok_wordcomplete_real_reset (GokWordComplete *complete);
static gchar *gok_wordcomplete_real_process_unichar (GokWordComplete *complete, gunichar letter);
static gchar *gok_wordcomplete_real_unichar_push (GokWordComplete *complete, gunichar letter);
static gchar *gok_wordcomplete_real_unichar_pop (GokWordComplete *complete);
static GokWordCompletionCharacterCategory gok_wordcomplete_real_unichar_get_category (
	                                         GokWordComplete *complete, gunichar letter);
static gchar **gok_wordcomplete_real_predict (GokWordComplete *complete, gint num_predictions);
static gchar *gok_wordcomplete_real_get_word_part (GokWordComplete *complete);
static const gchar *  gok_wordcomplete_real_get_aux_dict (GokWordComplete *complete);
static gboolean gok_wordcomplete_real_set_aux_dict (GokWordComplete *complete, gchar *dict_path);
static const gchar *  gok_wordcomplete_real_get_delimiter (GokWordComplete *complete);

static GokOutput OutputSpace;

struct _GokWordCompletePrivate 
{
        /* the word, or part word, that is getting completed */
	gchar     *word_part;
	gunichar     compose;
	gchar *aux_dictionaries;
};

static GokWordComplete *default_wordcomplete_engine = NULL;

GNOME_CLASS_BOILERPLATE (GokWordComplete, gok_wordcomplete, 
			 GObject, G_TYPE_OBJECT)

static void
gok_wordcomplete_instance_init (GokWordComplete *complete)
{
	complete->priv = g_new0 (GokWordCompletePrivate, 1);
	complete->priv->word_part = NULL;
	complete->priv->compose = 0;
	complete->priv->aux_dictionaries = NULL;
}

static void
gok_wordcomplete_class_init (GokWordCompleteClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	/* public methods */
	gobject_class->finalize = gok_wordcomplete_finalize;
	klass->reset = gok_wordcomplete_real_reset;
	klass->process_unichar = gok_wordcomplete_real_process_unichar;
	klass->predict = gok_wordcomplete_real_predict;

	/* 'protected' methods, for use by subclasses/implementors only */
	klass->unichar_get_category = gok_wordcomplete_real_unichar_get_category;
	klass->unichar_push = gok_wordcomplete_real_unichar_push;
	klass->unichar_pop = gok_wordcomplete_real_unichar_pop;
	klass->get_word_part = gok_wordcomplete_real_get_word_part;
	klass->set_aux_dictionary = gok_wordcomplete_real_set_aux_dict;
	klass->get_aux_dictionary = gok_wordcomplete_real_get_aux_dict;
	klass->get_delimiter = gok_wordcomplete_real_get_delimiter;

	/* end the word with a 'space' */
	OutputSpace.Type = OUTPUT_KEYSYM;
	OutputSpace.Flag = SPI_KEY_PRESSRELEASE;
	OutputSpace.Name = "space";
	OutputSpace.pOutputNext = NULL;
}

static void
gok_wordcomplete_finalize (GObject *obj)
{
	GokWordComplete *complete = GOK_WORDCOMPLETE (obj);

	if (complete && complete->priv)
		g_free (complete->priv);
	G_OBJECT_CLASS (GOK_WORDCOMPLETE_GET_CLASS (obj))->finalize (obj);
}

/**
* gok_wordcomplete_open
* 
* Opens and initializes the word completor engine.
*
* returns: TRUE if it was opend OK, FALSE if not.
**/
gboolean gok_wordcomplete_open (GokWordComplete *complete, gchar *directory)
{
	GokWordCompleteClass *klass = GOK_WORDCOMPLETE_GET_CLASS (complete);
	
	if (klass->open)
		return (* klass->open)(complete, directory);
	else
		return FALSE;
}

/**
* gok_wordcomplete_close
* 
* Closes the word completor engine.
*
* returns: void
**/
void gok_wordcomplete_close (GokWordComplete *complete)
{
	GokWordCompleteClass *klass = GOK_WORDCOMPLETE_GET_CLASS (complete);

	if (klass->close)
		(*klass->close) (complete);
}

/**
* gok_wordcomplete_get_delimiter
*
* returns: a character string (usually one character long) that should be output after a word-completion 
*          event, for example a space.  In some locales, this string may be empty or NULL.
**/
const gchar *
gok_wordcomplete_get_delimiter (GokWordComplete *complete)
{
	GokWordCompleteClass *klass = GOK_WORDCOMPLETE_GET_CLASS (complete);

	if (klass->get_delimiter)
		return (*klass->get_delimiter) (complete);
	else
		return NULL;
}

/**
 * gok_wordcomplete_unichar_push:
 *
 **/
gchar *
gok_wordcomplete_unichar_push (GokWordComplete *complete, gunichar letter)
{
	GokWordCompleteClass *klass = GOK_WORDCOMPLETE_GET_CLASS (complete);

	if (klass->unichar_push)
		return (*klass->unichar_push) (complete, letter);
	else
		return NULL;
}

/**
 * gok_wordcomplete_unichar_pop:
 *
 **/
gchar *
gok_wordcomplete_unichar_pop (GokWordComplete *complete)
{
	GokWordCompleteClass *klass = GOK_WORDCOMPLETE_GET_CLASS (complete);

	if (klass->unichar_pop)
		return (*klass->unichar_pop) (complete);
	else
		return NULL;
}



/**
 * gok_wordcomplete_unichar_get_category
 *
 **/
GokWordCompletionCharacterCategory
gok_wordcomplete_unichar_get_category (GokWordComplete *complete, gunichar letter)
{
	GokWordCompleteClass *klass = GOK_WORDCOMPLETE_GET_CLASS (complete);
	
	if (klass->unichar_get_category)
		return (* klass->unichar_get_category)(complete, letter);
	else
		return GOK_WORDCOMPLETE_CHAR_TYPE_NORMAL;
}

/**
 * gok_wordcomplete_process_unichar:
 *
 **/
gchar *
gok_wordcomplete_process_unichar (GokWordComplete *complete, gunichar letter)
{
	GokWordCompleteClass *klass = GOK_WORDCOMPLETE_GET_CLASS (complete);
	if (klass->process_unichar)
		(* klass->process_unichar) (complete, letter);
}

/**
 *
 **/
gchar *
gok_wordcomplete_get_word_part (GokWordComplete *complete)
{
	GokWordCompleteClass *klass = GOK_WORDCOMPLETE_GET_CLASS (complete);
	if (klass->get_word_part)
		return (* klass->get_word_part) (complete);
	else
		return NULL;
}

/**
* gok_wordcomplete_process_and_predict:
* 
* Makes a prediction based on the effect of @letter on the current prediction state machine.
*
* returns: An 
array of strings representing the predicted completions.
**/
gchar **
gok_wordcomplete_process_and_predict (GokWordComplete *complete, const gunichar letter, gint num_predictions)
{
	return gok_wordcomplete_predict_string (complete, 
						gok_wordcomplete_process_unichar (complete, letter),
						num_predictions);
}

/**
* gok_wordcomplete_predict_string:
* 
* Makes a prediction. If the currently displayed keyboard is showing prediction
* keys then they are filled in with the predictions.
*
* returns: An 
array of strings representing the predicted completions.
**/
gchar **
gok_wordcomplete_predict_string (GokWordComplete *complete, const gchar *string, gint num_predictions)
{
	GokWordCompleteClass *klass = GOK_WORDCOMPLETE_GET_CLASS (complete);
	if (klass->predict_string)
		return (* klass->predict_string) (complete, string, num_predictions);
	else
		return NULL;
}

/**
* gok_wordcomplete_predict
* 
* Makes a prediction. If the currently displayed keyboard is showing prediction
* keys then they are filled in with the predictions.
*
* returns: An 
array of strings representing the predicted completions.
**/
gchar **
gok_wordcomplete_predict (GokWordComplete *complete, gint num_predictions)
{
	GokWordCompleteClass *klass = GOK_WORDCOMPLETE_GET_CLASS (complete);
	if (klass->predict)
		(* klass->predict) (complete, num_predictions);
}

/**
* gok_wordcomplete_reset
* 
* Resets the part word buffer.
*
* returns: void
**/
void 
gok_wordcomplete_reset (GokWordComplete *complete)
{
	GokWordCompleteClass *klass = GOK_WORDCOMPLETE_GET_CLASS (complete);
	if (klass->reset)
		(* klass->reset) (complete);
}

/**
* gok_wordcomplete_add_new_word
* 
* Adds a new word to the predictor dictionary.
*
* returns: TRUE if the word was added to the dictionary, FALSE if not.
**/
gboolean gok_wordcomplete_add_new_word (GokWordComplete *complete, const gchar* pWord)
{
	GokWordCompleteClass *klass = GOK_WORDCOMPLETE_GET_CLASS (complete);
	if (*klass->add_new_word)
		return (* klass->add_new_word)(complete, pWord);
	else
		return FALSE;
}

/**
* gok_wordcomplete_increment_word_frequency
* 
* Increments the frequency of a word in the dictionary.
*
* returns: TRUE if the word's frequency was incremented, FALSE if not.
**/
gboolean 
gok_wordcomplete_increment_word_frequency (GokWordComplete *complete, const gchar* pWord)
{
	GokWordCompleteClass *klass = GOK_WORDCOMPLETE_GET_CLASS (complete);
	if (*klass->increment_word_frequency && pWord && g_utf8_strlen (pWord, -1))
		return (* klass->increment_word_frequency)(complete, pWord);
	else
		return FALSE;
}

/**
* gok_wordcomplete_validate_word:
* 
* Determines whether a word is known to the completion engine.
* For dictionary-based engines this implies that the word is in the dictionary;
* some other sorts of engines may return TRUE if the word matches a heuristic,
* but may not guarantee that the word will appear in subsequent prediction
* runs.
*
* returns: TRUE if the word is know to the completion engine, FALSE if not.
**/
gboolean 
gok_wordcomplete_validate_word (GokWordComplete *complete, const gchar* pWord)
{
	GokWordCompleteClass *klass = GOK_WORDCOMPLETE_GET_CLASS (complete);
	if (*klass->validate_word)
		return (* klass->validate_word) (complete, pWord);
	else
		return FALSE;
}

/**
* gok_wordcomplete_get_aux_dictionaries:
* 
* returns: a fully-delimited file path to an auxiliary word prediction dictionary
**/
const gchar*   
gok_wordcomplete_get_aux_dictionaries (GokWordComplete *complete)
{
	GokWordCompleteClass *klass = GOK_WORDCOMPLETE_GET_CLASS (complete);
	if (*klass->get_aux_dictionary)
		return (* klass->get_aux_dictionary)(complete);
	else
		return NULL;
}

/**
* gok_wordcomplete_set_aux_dictionaries:
* 
* returns: TRUE if an aux dictionary can be used by the word-completion engine, FALSE otherwise.
**/
gboolean 
gok_wordcomplete_set_aux_dictionaries (GokWordComplete *complete, gchar *dictionary_path)
{
	GokWordCompleteClass *klass = GOK_WORDCOMPLETE_GET_CLASS (complete);
	if (*klass->set_aux_dictionary)
		return (* klass->set_aux_dictionary)(complete, dictionary_path);
	else
		return FALSE;
}

/**
 * gok_wordcomplete_get_default:
 *
 * Returns the current default word completion engine.
 **/
GokWordComplete*
gok_wordcomplete_get_default (void)
{
	if (default_wordcomplete_engine == NULL)
	{
	    gchar *locale = setlocale (LC_MESSAGES, NULL);
	    if (!strcmp (locale, "zh_CN") || 
		!strcmp (locale, "zh_TW") || 
		!strcmp (locale, "jp_JP") ||
		!strcmp (locale, "jp"))
		default_wordcomplete_engine = g_object_new (GOK_TYPE_SW_WORDCOMPLETE, NULL);  
	    else
		default_wordcomplete_engine = g_object_new (GOK_TYPE_UTF8WORDCOMPLETE, NULL);  
	}
        /* TODO: establish default subtype and create type - making the base class abstract */
	return default_wordcomplete_engine;
}

/* private methods */

static gboolean
gok_wordcomplete_unichar_is_compose (GokWordComplete *complete, gunichar letter)
{
	GUnicodeType utype = g_unichar_type (letter);
	return (utype == G_UNICODE_MODIFIER_LETTER || utype == G_UNICODE_MODIFIER_SYMBOL);
}

static gboolean
gok_wordcomplete_unichar_is_combine (GokWordComplete *complete, gunichar letter)
{
	GUnicodeType utype = g_unichar_type (letter);
	return (utype == G_UNICODE_COMBINING_MARK || utype == G_UNICODE_NON_SPACING_MARK);
}

static gboolean 
gok_wordcomplete_unichar_is_backspace (GokWordComplete *complete, gunichar letter) 
{
	return ((letter == 0x08) || (letter == 0x7f) || (letter == 0xfffd));
}

static gboolean
gok_wordcomplete_unichar_is_delimiter (GokWordComplete *complete, gunichar letter)
{		
/* 
 * Is this whitespace, empty, punctuation, or a control character? Also catches NULL/0 
 * we make exception for U+00a0 (nonbreak space) so 'words' containing 
 * space can be handled. We also make some decisions based on the unicode line-breaking type;
 * in general if a line break is allowed (except HYPHEN) after or between the adjacent 
 * characters, we take that behavior to constitute a 'delimiter' for the purposes of
 * bounding our word completion input.
 */
	GUnicodeBreakType break_type;

	if ((!g_unichar_validate (letter) ||  /* if invalid, */
	     g_unichar_iscntrl (letter)) /* or a control char, */
	    && (letter != 0x00a0))  /* but not nbspace */
		return TRUE;

	break_type = g_unichar_break_type (letter);
	switch (break_type)
	{
	case G_UNICODE_BREAK_CARRIAGE_RETURN:
	case G_UNICODE_BREAK_LINE_FEED:
	case G_UNICODE_BREAK_CONTINGENT:
	case G_UNICODE_BREAK_SPACE:
	case G_UNICODE_BREAK_INFIX_SEPARATOR:
	case G_UNICODE_BREAK_SYMBOL:
	case G_UNICODE_BREAK_OPEN_PUNCTUATION:
	case G_UNICODE_BREAK_CLOSE_PUNCTUATION:
	case G_UNICODE_BREAK_QUOTATION:
	case G_UNICODE_BREAK_EXCLAMATION:
	case G_UNICODE_BREAK_MANDATORY:
	case G_UNICODE_BREAK_AFTER:
	case G_UNICODE_BREAK_BEFORE_AND_AFTER:
		return TRUE;
	case G_UNICODE_BREAK_BEFORE:
	case G_UNICODE_BREAK_NON_STARTER:
	case G_UNICODE_BREAK_IDEOGRAPHIC:
	case G_UNICODE_BREAK_NUMERIC:
	case G_UNICODE_BREAK_ALPHABETIC:
	case G_UNICODE_BREAK_PREFIX:
	case G_UNICODE_BREAK_POSTFIX:
	case G_UNICODE_BREAK_COMPLEX_CONTEXT:
	case G_UNICODE_BREAK_HYPHEN:
	case G_UNICODE_BREAK_COMBINING_MARK:
	case G_UNICODE_BREAK_SURROGATE:
	case G_UNICODE_BREAK_ZERO_WIDTH_SPACE:
	case G_UNICODE_BREAK_INSEPARABLE:
	case G_UNICODE_BREAK_NON_BREAKING_GLUE:
	case G_UNICODE_BREAK_UNKNOWN:
	case G_UNICODE_BREAK_AMBIGUOUS:
	default:
		return FALSE;
	}
}

static gunichar
gok_wordcomplete_compose (GokWordComplete *complete, gunichar combining_letter, gunichar base_letter)
{
	gchar *tmp, *norm, utf8char[7], utf8combining[7];
	gunichar composed_letter = base_letter;

	fprintf (stderr, "combining %c [%x], %c\n", combining_letter, combining_letter, base_letter);

	/* we'll combine regardless of the order, assuming that the caller has kept track of that.
	 * certain unicode characters are officially non-combining, 
	 * but result from X compose keysym sequences. 
	 * We will convert these into their equivalent unicode combining forms, and then use
	 * utf8 normalization routines to convert them into their composed forms.
	 */
	if (combining_letter >= 0xa8 && combining_letter <= 0xb8 || combining_letter == 0x05e) 
	{
		switch (combining_letter)
		{
		case 0x5e: /* caret/circumflex */
			combining_letter = 0x0302; /* combining circumflex */
			break;
		case 0xa8: /* dieresis */
			combining_letter = 0x0308; /* combining dieresis */
			break;
		case 0xaf: /* macron */
			combining_letter = 0x0304; /* combining macron */
			break;
		case 0xb0: /* degree sign */
			combining_letter = 0x030a; /* combining ring above */
			break;
		case 0xb4: /* acute accent */
			combining_letter = 0x0301; /* combining acute accent */
			break;
		case 0xb8: /* cedilla */
			combining_letter = 0x0327; /* etc. */
			break;
		default:
			break;
		}
	}
	utf8char[g_unichar_to_utf8 (base_letter, utf8char)] = '\0';
	utf8combining[g_unichar_to_utf8 (combining_letter, utf8combining)] = '\0';
	tmp = g_strconcat (utf8char, utf8combining, NULL);
	if (tmp) 
	{
		norm = g_utf8_normalize (tmp, -1, G_NORMALIZE_DEFAULT_COMPOSE); /* maximally composed forms */
		g_free (tmp);
		if (norm) 
		{
			composed_letter = g_utf8_get_char (norm);
			g_free (norm);
		}
	}
	return composed_letter;
}

static void
gok_wordcomplete_compose_push (GokWordComplete *complete, gunichar combining_letter)
{
	complete->priv->compose = combining_letter;
}

static gunichar
gok_wordcomplete_last_combine (GokWordComplete *complete, gunichar combining_letter)
{
	gunichar last_letter = 0, letter = 0;
	gchar *word = gok_wordcomplete_get_word_part (complete);
	if (word)
	{
		gchar utf8char[7];
		gchar *tmp, *tmp2;
		utf8char[g_unichar_to_utf8 (combining_letter, utf8char)] = '\0'; /* null terminate utf8 string */
		tmp = g_strconcat (word, utf8char, NULL);
		if (tmp)
		{
			gint len;
			tmp2 = g_utf8_normalize (tmp, -1, G_NORMALIZE_DEFAULT_COMPOSE);
			g_free (tmp);
			if (tmp2) 
			{
				len = g_utf8_strlen (tmp2, -1);
				if (len)
					letter = g_utf8_get_char (g_utf8_offset_to_pointer (tmp2, len - 1));
				g_free (tmp2);
			}
		}
	}
	return letter;
}

/* implementations of GokWordCompleteClass methods */

static const gchar*
gok_wordcomplete_real_get_delimiter (GokWordComplete *complete)
{    
	return " ";
}

static void 
gok_wordcomplete_real_reset (GokWordComplete *complete)
{
	g_free (complete->priv->word_part);
	complete->priv->word_part = NULL;
}

static gchar *
gok_wordcomplete_real_unichar_push (GokWordComplete *complete, gunichar letter)
{
	gchar *tmp, utf8char[7]; /* min 6, plus NULL */
	gchar *word_part = NULL;

	if (complete && complete->priv) 
	{
		word_part = gok_wordcomplete_get_word_part (complete);
		if (complete->priv->compose)
		{
			letter = gok_wordcomplete_compose (complete, complete->priv->compose, letter);
			complete->priv->compose = 0;
		}
		utf8char [g_unichar_to_utf8 (letter, utf8char)] = '\0';
		if (word_part) 
		{
			tmp = g_strconcat (word_part, utf8char, NULL);
			g_free (word_part);
			word_part = tmp;
		}
		else
		{
			word_part = g_strdup (utf8char);
		}
		complete->priv->word_part = word_part;
	}
	return word_part;
}

static gchar *
gok_wordcomplete_real_unichar_pop (GokWordComplete *complete)
{
	gint length;
	if (complete->priv && complete->priv->word_part) 
	{
		length = strlen (complete->priv->word_part);
		if (length)
		{
			*g_utf8_offset_to_pointer (complete->priv->word_part, --length) = '\0';
		}
	}
	return (complete->priv) ? complete->priv->word_part : NULL;
}

/**
 * gok_wordcomplete_real_unichar_get_category
 *
 **/
static GokWordCompletionCharacterCategory
gok_wordcomplete_real_unichar_get_category (GokWordComplete *complete, gunichar letter)
{
	if (gok_wordcomplete_unichar_is_backspace (complete, letter))
		return GOK_WORDCOMPLETE_CHAR_TYPE_BACKSPACE;
	else if (gok_wordcomplete_unichar_is_compose (complete, letter))
		return GOK_WORDCOMPLETE_CHAR_TYPE_COMPOSE;
	else if (gok_wordcomplete_unichar_is_combine (complete, letter))
		return GOK_WORDCOMPLETE_CHAR_TYPE_COMBINE;
	else if (gok_wordcomplete_unichar_is_delimiter (complete, letter))
		return GOK_WORDCOMPLETE_CHAR_TYPE_DELIMITER;
	else
		return GOK_WORDCOMPLETE_CHAR_TYPE_NORMAL;
}

static gchar *
gok_wordcomplete_real_process_unichar (GokWordComplete *complete, gunichar letter)
{
	gchar *prefix = NULL, *tmpstring = NULL;
	gchar utf8char[7];

	switch (gok_wordcomplete_unichar_get_category (complete, letter))
	{
	case GOK_WORDCOMPLETE_CHAR_TYPE_BACKSPACE:
		prefix = gok_wordcomplete_unichar_pop (complete);
		break;
	case GOK_WORDCOMPLETE_CHAR_TYPE_DELIMITER:
		gok_wordcomplete_increment_word_frequency (complete,
							   gok_wordcomplete_get_word_part (complete));
		gok_wordcomplete_reset (complete);
		break;
	case GOK_WORDCOMPLETE_CHAR_TYPE_COMPOSE:
		gok_wordcomplete_compose_push (complete, letter);
		break;
	case GOK_WORDCOMPLETE_CHAR_TYPE_COMBINE:
		letter = gok_wordcomplete_last_combine (complete, letter);
		gok_wordcomplete_unichar_pop (complete);
		/* fall through */
	case GOK_WORDCOMPLETE_CHAR_TYPE_NORMAL:
	default:
		prefix = gok_wordcomplete_unichar_push (complete, letter);
		break;
	}

	return prefix;
}

/**
 **/
gchar *
gok_wordcomplete_real_get_word_part (GokWordComplete *complete)
{
	if (complete && complete->priv)
		return complete->priv->word_part;
	else
		return NULL;
}

/**
 **/
static const gchar *
gok_wordcomplete_real_get_aux_dict (GokWordComplete *complete)
{
	return complete->priv->aux_dictionaries;
}

/**
 **/
static gboolean
gok_wordcomplete_real_set_aux_dict (GokWordComplete *complete, gchar *dict_path)
{
	if (complete && complete->priv)
	    complete->priv->aux_dictionaries = dict_path;
	return TRUE;
}

/**
* gok_wordcomplete_real_predict
* 
* Makes a prediction. If the currently displayed keyboard is showing prediction
* keys then they are filled in with the predictions.
*
* returns: An array of strings representing the predicted completions.
**/
gchar **
gok_wordcomplete_real_predict (GokWordComplete *complete, gint num_predictions)
{
	return gok_wordcomplete_predict_string (complete, 
						gok_wordcomplete_get_word_part (complete),
						num_predictions);
}
