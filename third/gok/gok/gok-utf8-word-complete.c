/* gok-utf8-word-complete.c
*
* Copyright 2001,2002 Sun Microsystems, Inc.,
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
*
*/

#include <string.h>
#include <gnome.h>
#include "gok-log.h"
#include "gok-utf8-word-complete.h"

#define MAX_DICTIONARY_ENTRIES 100000

#define DEFAULT_INITIAL_FREQUENCY 1

/* private functions */

/* implementations of GokWordCompleteClass virtual methods */

typedef enum 
{
	WORD_CASE_LOWER,
	WORD_CASE_INITIAL_CAPS,
	WORD_CASE_ALL_CAPS,
	WORD_CASE_TITLE,
	WORD_CASE_NONE,
	WORD_CASE_MIXED
} WordPredictionCaseType;

typedef struct
{
	gchar *string;
	gint   priority;
	gboolean in_primary;
} WordPrediction;

/* The filename of the dictionary file */
static gchar *dictionary_full_path;

/* declarations of implementation methods */
static gchar** utf8_wordcomplete_predict_string (GokWordComplete *complete, const gchar* pWord, gint num_predictions);
static gboolean utf8_wordcomplete_open (GokWordComplete *complete, const gchar *directory);
static void utf8_wordcomplete_close (GokWordComplete *complete);
static gboolean utf8_wordcomplete_add_new_word (GokWordComplete *complete, const gchar *word);
static gboolean utf8_wordcomplete_increment_word_frequency (GokWordComplete *complete, const gchar *word);
static gboolean utf8_wordcomplete_validate_word (GokWordComplete *complete, const gchar *word);

/* private internal methods */
static GList *utf8_wordcomplete_find (GokUTF8WordComplete *complete, const gchar *word);
static GList *utf8_wordcomplete_find_word (GokUTF8WordComplete *complete, const gchar *word);
static gint utf8_sort_by_priority (gconstpointer a, gconstpointer b, gpointer data);
static gint utf8_sort_by_collation (gconstpointer a, gconstpointer b);
static GList *utf8_add_from_system_dicts (GList *dictionary, const gchar *system_dict_filename);
static GList *utf8_add_to_list_from_lines (GList *list, gchar **lines, gboolean read_freqs, gboolean in_primary);
static void utf8_wordcomplete_create_unicode_hash (GokUTF8WordComplete *complete);
static void utf8_apply_case (const gchar *word,  gchar **predictions);

/* 
 * This macro initializes GokUtf8WordComplete with the GType system 
 *   and defines ..._class_init and ..._instance_init functions 
 */
GNOME_CLASS_BOILERPLATE (GokUTF8WordComplete, gok_utf8_wordcomplete,
			 GokWordComplete, GOK_TYPE_WORDCOMPLETE)

static void
gok_utf8_wordcomplete_instance_init (GokUTF8WordComplete *complete)
{
	complete->word_list = NULL;
	complete->word_list_end = NULL;
	complete->start_search = NULL;
	complete->end_search = NULL;
	complete->most_recent_word = NULL;
}

static void
gok_utf8_wordcomplete_class_init (GokUTF8WordCompleteClass *klass)
{
	GokWordCompleteClass *word_complete_class = (GokWordCompleteClass *) klass;
	word_complete_class->open = utf8_wordcomplete_open;
	word_complete_class->close = utf8_wordcomplete_close;
	word_complete_class->predict_string = utf8_wordcomplete_predict_string;
	word_complete_class->add_new_word = utf8_wordcomplete_add_new_word;
	word_complete_class->increment_word_frequency = utf8_wordcomplete_increment_word_frequency;
	word_complete_class->validate_word = utf8_wordcomplete_validate_word;
}

static gboolean
utf8_wordcomplete_open (GokWordComplete *complete, const gchar *directory)
{
	gchar *contents;
	gchar **dictionary_lines = NULL;
	gint i;
	GError *error = NULL;
	GokUTF8WordComplete *utf8_complete = GOK_UTF8WORDCOMPLETE (complete);

	gok_log ("gok_utf8_wordcomplete_open");
	
	gok_wordcomplete_reset (complete); /* reset, anyhow */

	/* open the dictionary file */
	dictionary_full_path = g_build_filename (directory, "dictionary.txt", NULL);

	if (!g_file_get_contents (dictionary_full_path, &contents, NULL, &error))
	{
	        g_warning (_("Could not read contents of dictionary file \'%s\'\n"), 
			   dictionary_full_path);
		return FALSE;
	}
	else
	{
		GOK_UTF8WORDCOMPLETE (complete)->primary_dict_filename = g_strdup (dictionary_full_path);
		g_free (dictionary_full_path);
	}

	/* split into lines */
	dictionary_lines = 
		g_strsplit (contents, "\n", MAX_DICTIONARY_ENTRIES + 1);

	g_free (contents);

	if (dictionary_lines == NULL) 
	{
		return FALSE;
	}

	/* ignore first line, it's just "WPDictFile" */
	if (!strcmp (dictionary_lines[0], "WPDictFile"))
	{
		if (dictionary_lines[1]) /* sanity check, make sure there's at least one word! */
		{
			utf8_complete->word_list = utf8_add_to_list_from_lines (utf8_complete->word_list, 
										&dictionary_lines[1], TRUE, TRUE);
		}
	}

	g_strfreev (dictionary_lines);

	if (gok_wordcomplete_get_aux_dictionaries (complete))
	{
		utf8_complete->word_list = utf8_add_from_system_dicts (utf8_complete->word_list, 
								       gok_wordcomplete_get_aux_dictionaries (complete)); 
	}

	/* sort the list, so we don't have to search exhaustively */
	utf8_complete->word_list = g_list_sort (utf8_complete->word_list, 
						 utf8_sort_by_collation);

	utf8_complete->word_list_end = g_list_last (utf8_complete->word_list);

	/* create gunichar hash for looking up search entry points */
	utf8_wordcomplete_create_unicode_hash (utf8_complete);

	fprintf (stderr, "Word prediction dictionary contains a total of %d words\n", 
		 g_list_length (utf8_complete->word_list));

	return TRUE;
}

static void
utf8_wordcomplete_write_line (gpointer data, gpointer user_data)
{
	GIOChannel *io = user_data;
	GError *error = NULL;
	gsize bytes;
	WordPrediction *word_info = data;
	gchar line[256];
	snprintf (line, 255, "%s\t%d\t%d\n", word_info->string, word_info->priority, 2);
	g_io_channel_write_chars (io, line, -1, &bytes, &error);
}

static void
utf8_wordcomplete_close (GokWordComplete *complete)
{
	GIOChannel *io;
	GError *error = NULL;
	gsize bytes;
	GokUTF8WordComplete *ucomplete = GOK_UTF8WORDCOMPLETE (complete);
	/* open for writing */
	io = g_io_channel_new_file (ucomplete->primary_dict_filename, "w", &error);
	/* write the header */
	if (!error)
		g_io_channel_write_chars (io, "WPDictFile\n", -1, &bytes, &error);
	if (!error)
	{
		/* write entries*/
		g_list_foreach (ucomplete->word_list, utf8_wordcomplete_write_line, io);
	}
	g_list_free (ucomplete->word_list);
	g_list_free (ucomplete->word_list_end);
	g_list_free (ucomplete->start_search);
	g_list_free (ucomplete->end_search);
	g_hash_table_destroy (ucomplete->unicode_start_hash);
	g_hash_table_destroy (ucomplete->unicode_end_hash);
	g_free (ucomplete->primary_dict_filename);
	g_io_channel_shutdown (io, TRUE, &error);
}

static gchar**
utf8_wordcomplete_predict_string (GokWordComplete *complete, const gchar* word, gint num_predictions)
{
	gchar **word_predict_list;
	gchar *normalized, *tmp;
	gint  i, n, m = num_predictions;
	GList *predictions;
	GokUTF8WordComplete *utf8_complete = GOK_UTF8WORDCOMPLETE (complete);
	gpointer data;
	
	/* validate the given values */
	if (word == NULL || num_predictions < 1)
	{
	  return NULL;
	}

	/* check the word against the most-recently-predicted word, and 
	   reset the search pointers if need be */
	if (!utf8_complete->most_recent_word || !g_str_has_prefix (word, utf8_complete->most_recent_word))
	{
		GList *start, *end;
		gunichar unicode_char = g_utf8_get_char (word);
		if (utf8_complete->unicode_start_hash)
			start = g_hash_table_lookup (utf8_complete->unicode_start_hash, (gpointer) unicode_char);
		if (start)
		{
			utf8_complete->start_search = start;
		}
		else
			utf8_complete->start_search = utf8_complete->word_list;
		if (utf8_complete->unicode_end_hash)
			end = g_hash_table_lookup (utf8_complete->unicode_end_hash, (gpointer) unicode_char);
		if (end)
		{
			utf8_complete->end_search = end;
		}
		else
			utf8_complete->end_search = utf8_complete->word_list_end;
;
	}

	word_predict_list = g_new0 (gchar *, num_predictions + 1); /* NULL-terminated */

	/* convert the given string to a normalized, case-insensitive form */
	tmp = g_utf8_normalize (word, -1, G_NORMALIZE_DEFAULT_COMPOSE);
	normalized = g_utf8_casefold (tmp, -1);
	g_free (tmp);

	/* find matching words */
	predictions = utf8_wordcomplete_find (utf8_complete, normalized);

	/* reorder according to priority */
	predictions = g_list_sort_with_data (predictions, utf8_sort_by_priority, complete); 

	/* convert to array of strings */
	n = -1;
	do
	{
		WordPrediction *prediction;
		++n;
		prediction = g_list_nth_data (predictions, n);
		word_predict_list [n] = prediction ? g_strdup (prediction->string) : NULL;
	} while (word_predict_list [n] && n < num_predictions);

	word_predict_list [num_predictions] = NULL; /* in case predictions-length >= num_predictions */

	/* free the extra storage */
	g_list_free (predictions);

	/* were there any predictions? */
	if (n == 0)
	{
		g_free (word_predict_list);
 	        return NULL; /* indicates no predictions made */
	}

	/* post-process the predictions to match the case of the input */
	utf8_apply_case (word, word_predict_list); 

	if (utf8_complete->most_recent_word) 
		g_free (utf8_complete->most_recent_word);
	utf8_complete->most_recent_word = g_strdup (word);

	return word_predict_list; /* predictions made! */
}

static WordPrediction *
utf8_word_prediction_new (const gchar *word, gint priority, gboolean in_primary)
{
	WordPrediction *prediction = g_new0 (WordPrediction, 1);
	prediction->string = word ? g_utf8_casefold (word, -1): NULL;
	prediction->priority = priority;
	prediction->in_primary = in_primary;
	g_assert (prediction->string);
	return prediction;
}

static gboolean 
utf8_wordcomplete_add_new_word (GokWordComplete *complete, const gchar *word)
{
	WordPrediction *prediction = utf8_word_prediction_new (word, 1, TRUE);
	GokUTF8WordComplete *ucomplete = GOK_UTF8WORDCOMPLETE (complete);
	ucomplete->word_list = g_list_sort (g_list_prepend (ucomplete->word_list, prediction),
					    utf8_sort_by_collation);
	ucomplete->word_list_end = g_list_last (ucomplete->word_list);
	utf8_wordcomplete_create_unicode_hash (ucomplete);
	return TRUE;
}

static gboolean 
utf8_wordcomplete_increment_word_frequency (GokWordComplete *complete, const gchar *word)
{
	WordPrediction *prediction;
	GokUTF8WordComplete *ucomplete = GOK_UTF8WORDCOMPLETE (complete);
	GList *node = utf8_wordcomplete_find_word (ucomplete, word);
	if (node)
	{
		WordPrediction *prediction = node->data;
		if (prediction)
		{
			prediction->priority++;
			return TRUE;
		}
	}
	return FALSE;
}

static gboolean 
utf8_wordcomplete_validate_word (GokWordComplete *complete, const gchar *word)
{
	GokUTF8WordComplete *ucomplete = GOK_UTF8WORDCOMPLETE (complete);
	GList *node = utf8_wordcomplete_find_word (ucomplete, word);
	return node ? TRUE : FALSE;
}

static WordPredictionCaseType
utf8_determine_case (const gchar *word)
{
	gunichar uc = g_utf8_get_char_validated (word, -1);
	if (g_unichar_isupper (uc)) 
	{
		gchar *upperstring = g_utf8_strup (word, -1);
		if ((g_utf8_strlen (word, -1) > 1) && !strcmp (word, upperstring))
		{
			g_free (upperstring);
			return WORD_CASE_ALL_CAPS;
		}
		g_free (upperstring);
		return WORD_CASE_INITIAL_CAPS; /* not strictly true, but our best guess */
	}
	else if (g_unichar_istitle (uc)) 
	{
		return WORD_CASE_TITLE; /* TODO: detect mixed case here */
	}
	else if (g_unichar_islower (uc))
	{
		return WORD_CASE_LOWER; /* TODO: detect mixed case here */
	}
	return WORD_CASE_NONE;
}

/* this routine may modify its first parameter; it performs an in-place conversion if possible */
static void
utf8_case_convert_char (gchar **word, gchar **utf8char, WordPredictionCaseType casetype)
{
	gunichar letter;
	gunichar newletter;
	
	g_assert (utf8char != NULL);

	letter = g_utf8_get_char_validated (*utf8char, -1);

	if (letter < 0) return; /* invalid UTF-8! */

	switch (casetype)
	{
	case WORD_CASE_ALL_CAPS:
	case WORD_CASE_INITIAL_CAPS:
		newletter = g_unichar_toupper (letter);
		break;
	case WORD_CASE_TITLE:
		newletter = g_unichar_totitle (letter);
		break;
	case WORD_CASE_LOWER:
	default:
		newletter = g_unichar_tolower (letter);
	}
	if (newletter != letter)
	{
		gchar buf[6];
		gint newlen = g_unichar_to_utf8 (newletter, buf);
		gint len = g_unichar_to_utf8 (letter, NULL);

		/* overwrite the old value if UTF8 lengths don't differ */
		if (len == newlen) 
		{
			buf[newlen]='\0';
			/* cannot use g_utf8_strncpy because it appends a null */
			memcpy (*utf8char, buf, strlen(buf));
		}
		else
		{
			/* reallocate string, dup+concat substrings */
			/* WARNING: horrible pointer math */
			gint delta = newlen - len;
			gchar *next_char = g_utf8_find_next_char (*utf8char, NULL);
			guint offset = next_char - *word;
			guint utf8_offset = (guint) (*utf8char - *word);
			gok_log ("caution: relocating input string during case conversion.");

			*word = g_realloc (*word, strlen (*word) + 1 + delta);
			next_char = *word + offset;
			strncpy (next_char, next_char + delta, strlen (next_char));
			*utf8char = *word + utf8_offset;
			g_utf8_strncpy (*utf8char, buf, 1);
		}
	}
	/* else letter didn't change, do nothing */
}

static gchar *
utf8_case_convert_string (gchar *word, WordPredictionCaseType casetype)
{
	gchar *tmp = word, *string = word;
	gint i = 0, len;
	if (!word || !g_utf8_validate (word, -1, NULL)) {
		gok_log_x ("invalid UTF-8 passed for case conversion.");
		return word;
	}
	len = g_utf8_strlen (word, -1);

	while (tmp && (i < len)) 
	{
		utf8_case_convert_char (&string, &tmp, casetype);
		++i;
		tmp = g_utf8_offset_to_pointer (string, i);
	}
	return string;
}

static void
utf8_apply_case (const gchar *word,  gchar **predictions)
{
	WordPredictionCaseType casetype;
	int i = 0;
	casetype = utf8_determine_case (word);

	while (predictions && predictions[i] && *predictions[i]) 
	{
		switch (casetype)
		{
		case WORD_CASE_INITIAL_CAPS:
			/* convert initial char */
			utf8_case_convert_char ((gchar **) &predictions[i], (gchar **) &predictions[i], casetype);
			break;
		case WORD_CASE_TITLE:
		case WORD_CASE_ALL_CAPS:	
			/* convert all chars */
			predictions[i] = utf8_case_convert_string (predictions[i], casetype);
			break;
		case WORD_CASE_NONE:
		case WORD_CASE_LOWER:
		case WORD_CASE_MIXED:
		default:
			break;
		}
		++i;
	}
}

static GList *
utf8_wordcomplete_find_word (GokUTF8WordComplete *complete, const gchar *word)
{
	GList *next = complete->start_search;
	WordPrediction *prediction;

	if (next && next->prev) 
	{
	    next = next->prev;
	}
	if (word) 
	{
		while (next && next->data) 
		{
			prediction = next->data;
			if (prediction->string && !strcmp (prediction->string, word))
			{
				return next;
			}
			else if (prediction->string && !g_str_has_prefix (prediction->string, word))
			{
				return NULL;
			}
			next = g_list_next (next);
		}
	}

	return NULL;
}

static GList *
utf8_wordcomplete_find (GokUTF8WordComplete *complete, const gchar *word)
{
	GList *found = NULL, *next = complete->start_search;
	WordPrediction *prediction;

	if (word) 
	{
	        gint word_len = g_utf8_strlen (word, -1);
		while (next && next->data) 
		{
			prediction = next->data;
			if (prediction->string && g_str_has_prefix (prediction->string, word) && 
			    g_utf8_strlen (prediction->string, -1) > word_len) 
			{
				if (!found) complete->start_search = next;
				found = g_list_append (found, prediction);
			}
			else if (found) 
			{
				complete->end_search = next;
				break;
			}
			if (complete->end_search && next == complete->end_search) 
			{
				break;
			}
			next = g_list_next (next);
		}
	}

	return found;
}

static gint
utf8_sort_by_priority (gconstpointer a, gconstpointer b, gpointer data)
{
        WordPrediction *p_a = (WordPrediction *) a;
	WordPrediction *p_b = (WordPrediction *) b;

	if (a && b)
		return p_b->priority - p_a->priority;
	else
		return (a) ? -1 : ((b) ? 1 : 0);
}

static gint
utf8_sort_by_collation (gconstpointer a, gconstpointer b)
{
        WordPrediction *p_a = (WordPrediction *) a;
	WordPrediction *p_b = (WordPrediction *) b;
	if (!a || !b)
		return (a) ? -1 : ((b) ? 1 : 0);
	else if (!p_a->string || !p_b->string)
		return (p_a->string) ? -1 : ((p_b->string) ? 1 : 0);
	else 
		return strcmp (p_a->string, p_b->string);
}

static GList *
utf8_add_from_system_dicts (GList *dictionary, const gchar *system_dict_filenames)
{
	gchar *contents;
	gchar **system_dict_lines = NULL;
	GError *error = NULL;
	gint i = 0;
	gchar **system_dict_files = NULL;

	if (system_dict_filenames)
	{
		system_dict_files = g_strsplit (system_dict_filenames, ";", 20);

		if (!system_dict_files[0])
		{
			return dictionary;
		}

		while (system_dict_files[i] && g_file_get_contents (system_dict_files[i], &contents, NULL, &error)) {
			
			fprintf (stderr, "system dict files[%d] = %s\n", i, system_dict_files[i]);

			/* split into lines */
			system_dict_lines = 
				g_strsplit (contents, "\n", MAX_DICTIONARY_ENTRIES + 1);
			
			g_free (contents);
			
			if (system_dict_lines != NULL) 
			{
				dictionary = utf8_add_to_list_from_lines (dictionary, system_dict_lines, FALSE, FALSE);
				g_strfreev (system_dict_lines);
			}
			++i;
		}
		g_strfreev (system_dict_files);
	}
	return dictionary;
}

static GList *
utf8_add_to_list_from_lines (GList *list, gchar **lines, gboolean read_freqs, gboolean in_primary)
{
	while (*lines)
	{
		gint   priority;
		gchar **tokens = NULL;
		gchar *string = NULL;
		WordPrediction *prediction;

		if (read_freqs) {
			tokens = g_strsplit (*lines, "\t", 3);
			string = tokens ? tokens[0] : NULL;
		}
		else
		{
			if (strchr (*lines, '/'))
			{
				g_strdelimit (*lines, "\t\\", '/');
				tokens = g_strsplit (*lines, "/", 2);
			}
			else
			{
				g_strdelimit (*lines, "\t\\", ' ');
				tokens = g_strsplit (*lines, " ", 2);
			}
			string = tokens ? tokens[0] : NULL;
		}

		if (string && string[0])
		{
			if (!g_utf8_validate (string, -1, NULL))
			{
				GError *error = NULL;
				gsize bytes_read, bytes_written;
				string = g_convert (string, -1, "UTF-8", "ISO-8859-1", &bytes_read, &bytes_written, &error);
				if (error)
					g_warning (error->message);
			}
			if (string)
			{
				string = g_utf8_normalize (string, -1, G_NORMALIZE_DEFAULT_COMPOSE); /* this allocs a new string */
				priority = (read_freqs && tokens && tokens[0] && tokens[1]) ? 
					g_ascii_strtod (tokens[1], NULL) : DEFAULT_INITIAL_FREQUENCY;
				prediction = utf8_word_prediction_new (string, priority, in_primary);
				/* prepend is much faster than append */
				list = g_list_prepend (list, prediction);
			}
			else
			{
				g_warning ("could not add word %s to dictionary\n", tokens[0]);
			}
		}
		if (tokens) 
			g_strfreev (tokens);
		++lines;
	}

	return list;
}

static void
utf8_wordcomplete_create_unicode_hash (GokUTF8WordComplete *complete)
{
	GList *next = complete->word_list;

	if (complete->unicode_start_hash) 
		g_hash_table_destroy (complete->unicode_start_hash);
	if (complete->unicode_end_hash) 
		g_hash_table_destroy (complete->unicode_end_hash);

	complete->unicode_start_hash = g_hash_table_new (NULL, NULL);
	complete->unicode_end_hash = g_hash_table_new (NULL, NULL);

	while (next) 
	{
		gunichar unicode_char = 0, prev_char = 0;
		WordPrediction *prediction = (WordPrediction *) next->data;
		gchar *word = (prediction) ? prediction->string : NULL;

		if (word) 
		{
			unicode_char = g_utf8_get_char (word);
		}
		if (unicode_char && (unicode_char != prev_char)) 
		{
			/* search for previous entry, just in case locale's collation 
			   sequence puts things in an 'unusual' order */
			if (!g_hash_table_lookup (complete->unicode_start_hash, (gpointer) unicode_char)) 
			{
				g_hash_table_insert (complete->unicode_start_hash, (gpointer) unicode_char, next);
			}
			if (prev_char)
			{
				g_hash_table_insert (complete->unicode_end_hash, (gpointer) prev_char, next);
			}
			prev_char = unicode_char;
		}
		next = next->next;
	}
}
