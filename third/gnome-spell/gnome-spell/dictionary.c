/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of gnome-spell bonobo component
    copied from echo.c written by Miguel de Icaza and updated for Spell.idl needs

    Copyright (C) 1999, 2000 Helix Code, Inc.
    Authors:                 Radek Doulik <rodo@helixcode.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <config.h>
#include <glib.h>
#include <libgnome/gnome-i18n.h>
#include <libgnome/gnome-config.h>
#include <gconf/gconf-client.h>
#include <bonobo.h>

#include "Spell.h"
#include "dictionary.h"

static BonoboObjectClass                  *dictionary_parent_class;

#define DICT_DEBUG(x)
#define GNOME_SPELL_GCONF_DIR "/GNOME/Spell"

static void release_engines (GNOMESpellDictionary *dict);

static gchar *
engine_to_language (GNOMESpellDictionary *dict, SpellEngine *se)
{
	return (gchar *) se ? g_hash_table_lookup (dict->engines_ht, se) : NULL;
}

static SpellEngine *
language_to_engine (GNOMESpellDictionary *dict, gchar *language)
{
	return (SpellEngine *) language ? g_hash_table_lookup (dict->engines_ht, language) : NULL;
}

static void
raise_error (CORBA_Environment * ev, const gchar *s)
{
	GNOME_Spell_Dictionary_Error *exception;
	exception = GNOME_Spell_Dictionary_Error__alloc ();
		
	exception->error = CORBA_string_dup (s);
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			     ex_GNOME_Spell_Dictionary_Error,
			     exception);
}

static void
gnome_spell_dictionary_init (GObject *object)
{
	GNOMESpellDictionary *dict = GNOME_SPELL_DICTIONARY (object);

	dict->changed = TRUE;
	dict->engines = NULL;
	dict->languages = g_hash_table_new (g_str_hash, g_str_equal);
	dict->engines_ht = g_hash_table_new (NULL, NULL);
}

static void
dictionary_finalize (GObject *object)
{
	GNOMESpellDictionary *dictionary = GNOME_SPELL_DICTIONARY (object);

	release_engines (dictionary);
	g_hash_table_destroy (dictionary->languages);
	dictionary->languages = NULL;
	g_hash_table_destroy (dictionary->engines_ht);
	dictionary->engines_ht = NULL;

	G_OBJECT_CLASS (dictionary_parent_class)->finalize (object);
}

static SpellEngine *
new_engine (const gchar *language)
{
	SpellEngine *se;

	se = g_new0 (SpellEngine, 1);
	se->config = new_aspell_config ();
	aspell_config_replace (se->config, "language-tag", language);
	aspell_config_replace (se->config, "encoding", "utf-8");
	se->changed = TRUE;

	return se;
}

static gboolean
remove_language (gpointer key, gpointer value, gpointer user_data)
{
	g_free (key);

	return TRUE;
}

static gboolean
remove_engine_ht (gpointer key, gpointer value, gpointer user_data)
{
	g_free (value);

	return TRUE;
}

static void
release_engines (GNOMESpellDictionary *dict)
{
	for (; dict->engines; ) {
		SpellEngine *se = dict->engines->data;

		if (se->speller)
			delete_aspell_speller (se->speller);
		if (se->config)
			delete_aspell_config (se->config);
		g_free (se);
		dict->engines = g_slist_remove (dict->engines, se);
	}

	g_hash_table_foreach_remove (dict->languages, remove_language, NULL);
	g_hash_table_foreach_remove (dict->languages, remove_engine_ht, NULL);

	dict->engines = NULL;
	dict->changed = TRUE;
}

#define KNOWN_LANGUAGES 25
static gchar *known_languages [KNOWN_LANGUAGES*2 + 1] = {
	"br", N_("Breton"),
	"ca", N_("Catalan"),
	"cs", N_("Czech"),
	"da", N_("Danish"),
	"de-DE", N_("German (Germany)"),
	"de-CH", N_("German (Swiss)"),
	"el", N_("Greek"),
	"en-US", N_("English (American)"),
	"en-GB", N_("English (British)"),
	"en-CA", N_("English (Canadian)"),
	"eo", N_("Esperanto"),
	"es", N_("Spanish"),
	"fo", N_("Faroese"),
	"fr-FR", N_("French (France)"),
	"fr-CH", N_("French (Swiss)"),
	"it", N_("Italian"),
	"nl", N_("Dutch"),
	"no", N_("Norwegian"),
	"pl", N_("Polish"),
	"pt-PT", N_("Portuguese (Portugal)"),
	"pt-BR", N_("Portuguese (Brazilian)"),
	"ru", N_("Russian"),
	"sk", N_("Slovak"),
	"sv", N_("Swedish"),
	"uk", N_("Ukrainian"),
	NULL
};

static GSList *
get_languages_real (gint *ln)
{
	GSList *langs;
	AspellCanHaveError *err;
	AspellConfig  *config;
	AspellSpeller *speller;
	gint i;

	DICT_DEBUG (printf ("get_languages_real\n"));

	langs = NULL;
	*ln = 0;
	for (i=0; known_languages [i]; i++) {
		config = new_aspell_config ();
		aspell_config_replace (config, "language-tag", known_languages [i]);
		i++;
		err = new_aspell_speller (config);
		if (aspell_error_number (err) == 0) {
			speller = to_aspell_speller (err);
			DICT_DEBUG (printf ("Language: %s\n", known_languages [i]));
			delete_aspell_speller (speller);
			langs = g_slist_prepend (langs, GINT_TO_POINTER (i - 1));
			(*ln) ++;
		}
	}

	return langs;
}

static GSList *
get_languages_load (GConfClient *gc, gint *ln)
{
	GString *str;
	GSList *langs = NULL;
	gint i, lang_num;

	/* printf ("get_languages_load\n"); */

	str = g_string_new (NULL);
	*ln = gconf_client_get_int (gc, GNOME_SPELL_GCONF_DIR "/languages", NULL);
	for (i = 0; i < *ln; i++) {
		g_string_sprintf (str, GNOME_SPELL_GCONF_DIR "/language%d", i);
		lang_num = gconf_client_get_int (gc, str->str, NULL);
		langs = g_slist_prepend (langs, GINT_TO_POINTER (lang_num));
	}

	return langs;
}

static GSList *
get_languages (gint *ln)
{
	GSList *langs, *l;
	GConfClient *gc;
	time_t mtime;
	struct stat buf;
	gint i, kl;

	gc = gconf_client_get_default ();

	mtime = gconf_client_get_int (gc, GNOME_SPELL_GCONF_DIR "/mtime", NULL);
	kl = gconf_client_get_int (gc, GNOME_SPELL_GCONF_DIR "/known_languages", NULL);

 	if (stat (ASPELL_DICT, &buf) || buf.st_mtime != mtime || kl != KNOWN_LANGUAGES) {
		GString *str;
		langs = get_languages_real (ln);

		str = g_string_new (NULL);
		gconf_client_set_int (gc, GNOME_SPELL_GCONF_DIR "/languages", *ln, NULL);
		for (l = langs, i = 0; i < *ln; i ++) {
			g_string_sprintf (str, GNOME_SPELL_GCONF_DIR "/language%d", *ln - i - 1);
			gconf_client_set_int (gc, str->str, GPOINTER_TO_INT (l->data), NULL);
			l = l->next;
		}
		gconf_client_set_int (gc, GNOME_SPELL_GCONF_DIR "/mtime", buf.st_mtime, NULL);
		gconf_client_set_int (gc, GNOME_SPELL_GCONF_DIR "/known_languages", KNOWN_LANGUAGES, NULL);
		g_string_free (str, TRUE);
		gnome_config_sync ();
	} else
		langs = get_languages_load (gc, ln);

	gconf_client_suggest_sync (gc, NULL);
	g_object_unref (gc);

	return langs;
}

static GNOME_Spell_LanguageSeq *
impl_gnome_spell_dictionary_get_languages (PortableServer_Servant servant, CORBA_Environment *ev)
{
	GNOME_Spell_LanguageSeq *seq;
	GSList *l, *langs;
	gint i, ln, pos;

	langs = get_languages (&ln);

	seq = GNOME_Spell_LanguageSeq__alloc ();
	seq->_length = ln;

	if (seq->_length == 0)
		return seq;

	seq->_buffer = CORBA_sequence_GNOME_Spell_Language_allocbuf (seq->_length);

	for (i = ln - 1, l = langs; l; l = l->next, i--) {

		pos = GPOINTER_TO_INT (l->data);
		seq->_buffer [i].name = CORBA_string_dup (_(known_languages [pos + 1]));
		seq->_buffer [i].abbreviation = CORBA_string_dup (known_languages [pos]);
	}
	CORBA_sequence_set_release (seq, CORBA_TRUE);
	g_slist_free (langs);

	return seq;
}

static void
impl_gnome_spell_dictionary_set_language (PortableServer_Servant  servant,
					  const CORBA_char       *language,
					  CORBA_Environment      *ev)
{
	GNOMESpellDictionary *dict = GNOME_SPELL_DICTIONARY (bonobo_object_from_servant (servant));
	const gchar *s, *begin, *end;
	gchar *one_language;
	gint len;

	g_assert (dict);
	if (!language)
		language = "";

	DICT_DEBUG (printf ("setLanguage: %s\n", language));

	release_engines (dict);
	for (s = language; *s; s = end) {
		begin = s;
		while (*begin && *begin == ' ')
			begin++;
		end = begin;
		len = 0;
		while (*end && *end != ' ') {
			end ++;
			len ++;
		}

		if (len) {
			SpellEngine *se;
			
			one_language = g_strndup (begin, len);
			se = new_engine (one_language);
			dict->engines = g_slist_prepend (dict->engines, se);
			g_hash_table_insert (dict->languages, one_language, se);
			g_hash_table_insert (dict->engines_ht, se, g_strdup (one_language));

			dict->changed = TRUE;
		}
	}
}

static void
update_engine (SpellEngine *se, CORBA_Environment * ev)
{
	AspellCanHaveError *err;

	DICT_DEBUG (printf ("Dictionary: creating new aspell speller\n"));

	if (se->changed) {
		if (se->speller)
			delete_aspell_speller (se->speller);
		err = new_aspell_speller (se->config);
		if (aspell_error_number (err) != 0) {
			g_warning ("aspell error: %s\n", aspell_error_message (err));
			se->speller = NULL;
			raise_error (ev, aspell_error_message (err));
		} else {
			se->speller = to_aspell_speller (err);
			se->changed = FALSE;
		}
	}
}

static void
update_engines (GNOMESpellDictionary *dict, CORBA_Environment * ev)
{
	g_assert (IS_GNOME_SPELL_DICTIONARY (dict));

	if (dict->changed) {
		GSList *l;

		for (l = dict->engines; l; l = l->next) {
			update_engine ((SpellEngine *) l->data, ev);
		}

		dict->changed = FALSE;
	}
}

static CORBA_boolean
engine_check_word (SpellEngine *se, const gchar *word, CORBA_Environment *ev)
{
	CORBA_boolean result = CORBA_TRUE;
	gint aspell_result;

	g_return_val_if_fail (se->speller, CORBA_TRUE);

	aspell_result = aspell_speller_check (se->speller, word, strlen (word));
	if (aspell_result == 0)
		result = CORBA_FALSE;
	if (aspell_result == -1) {
		g_warning ("aspell error: %s\n", aspell_speller_error_message (se->speller));
		raise_error (ev, aspell_speller_error_message (se->speller));
	}

	return result;
}

static CORBA_boolean
impl_gnome_spell_dictionary_check_word (PortableServer_Servant servant, const CORBA_char *word, CORBA_Environment *ev)
{
	GNOMESpellDictionary *dict = GNOME_SPELL_DICTIONARY (bonobo_object_from_servant (servant));
	CORBA_boolean result = CORBA_FALSE;
	GSList *l;
	gboolean valid_speller = FALSE;

	g_return_val_if_fail (word, result);

	if (!strcmp (word, "Ximian"))
		return CORBA_TRUE;

	update_engines (dict, ev);
	for (l = dict->engines; l; l = l->next) {
		if (((SpellEngine *) l->data)->speller) {
			valid_speller = TRUE;
			if (engine_check_word ((SpellEngine *) l->data, word, ev))
				result = CORBA_TRUE;
		}
	}

	if (!valid_speller) {
		DICT_DEBUG (printf ("Dictionary check_word: %s --> 1 (not valid speller)\n", word));
		return CORBA_TRUE;
	}
	DICT_DEBUG (printf ("Dictionary check_word: %s --> %d\n", word, result));
	return result;
}

static void
impl_gnome_spell_dictionary_add_word_to_session (PortableServer_Servant servant, const CORBA_char *word, CORBA_Environment *ev)
{
	GNOMESpellDictionary *dict = GNOME_SPELL_DICTIONARY (bonobo_object_from_servant (servant));
	GSList *l;

	g_return_if_fail (word);

	update_engines (dict, ev);
	DICT_DEBUG (printf ("Dictionary add_word_to_session: %s\n", word));
	for (l = dict->engines; l; l = l->next) {
		if (((SpellEngine *) l->data)->speller)
			aspell_speller_add_to_session (((SpellEngine *) l->data)->speller, word, strlen (word));
	}
}

static void
impl_gnome_spell_dictionary_add_word_to_personal (PortableServer_Servant servant,
						  const CORBA_char *word, const CORBA_char *language, CORBA_Environment *ev)
{
	GNOMESpellDictionary *dict = GNOME_SPELL_DICTIONARY (bonobo_object_from_servant (servant));
	SpellEngine *se;

	g_return_if_fail (word);

	update_engines (dict, ev);
	DICT_DEBUG (printf ("Dictionary add_word_to_personal: %s (%s)\n", word, language));
	se = (SpellEngine *) g_hash_table_lookup (dict->languages, language);

	if (se && se->speller) {
		aspell_speller_add_to_personal (se->speller, word, strlen (word));
		aspell_speller_save_all_word_lists (se->speller);
		DICT_DEBUG (printf ("Added and saved.\n"));
	}
}

static void
impl_gnome_spell_dictionary_set_correction (PortableServer_Servant servant,
					    const CORBA_char *word, const CORBA_char *replacement, const CORBA_char *language, CORBA_Environment *ev)
{
	GNOMESpellDictionary *dict = GNOME_SPELL_DICTIONARY (bonobo_object_from_servant (servant));
	SpellEngine *se;

	g_return_if_fail (word);
	g_return_if_fail (replacement);

	update_engines (dict, ev);
	DICT_DEBUG (printf ("Dictionary correction: %s <-- %s\n", word, replacement));
	se = (SpellEngine *) g_hash_table_lookup (dict->languages, language);

	if (se && se->speller) {
		aspell_speller_store_replacement (se->speller, word, strlen (word),
						  replacement, strlen (replacement));
		aspell_speller_save_all_word_lists (se->speller);
		DICT_DEBUG (printf ("Set and saved.\n"));
	}
}

static GNOME_Spell_StringSeq *
impl_gnome_spell_dictionary_get_suggestions (PortableServer_Servant servant,
					     const CORBA_char *word, CORBA_Environment *ev)
{
	GNOMESpellDictionary  *dict = GNOME_SPELL_DICTIONARY (bonobo_object_from_servant (servant));
	const AspellWordList  *suggestions;
	AspellStringEnumeration *elements;
	GNOME_Spell_StringSeq *seq = NULL;
	GSList *l, *suggestion_list = NULL;
	gint i, len, pos;

	g_return_val_if_fail (word, NULL);

	DICT_DEBUG (printf ("Dictionary correction: %s\n", word));
	update_engines (dict, ev);

	len = 0;
	for (l = dict->engines; l; l = l->next) {
		SpellEngine *se = (SpellEngine *) l->data;

		if (se->speller) {
			suggestions  = aspell_speller_suggest (se->speller, word, strlen (word));
			suggestion_list = g_slist_prepend (suggestion_list, (gpointer) suggestions);
			len += 2*aspell_word_list_size (suggestions);
			suggestion_list = g_slist_prepend (suggestion_list, engine_to_language (dict, se));
		}
	}

	seq          = GNOME_Spell_StringSeq__alloc ();
	seq->_length = len;

	if (seq->_length == 0)
		return seq;

	seq->_buffer = CORBA_sequence_CORBA_string_allocbuf (seq->_length);

	pos = 0;
	for (l = suggestion_list; l; l = l->next) {
		gint list_len;
		gchar *language;

		language = (gchar *) l->data;
		l = l->next;
		suggestions = (const AspellWordList  *) l->data;
		elements = aspell_word_list_elements (suggestions);
		list_len = aspell_word_list_size (suggestions);
		for (i = 0; i < list_len; i ++, pos ++) {
			seq->_buffer [pos] = CORBA_string_dup (aspell_string_enumeration_next (elements));
			pos ++;
			seq->_buffer [pos] = CORBA_string_dup (language);
		}
		delete_aspell_string_enumeration (elements);
	}
	CORBA_sequence_set_release (seq, CORBA_TRUE);
	g_slist_free (suggestion_list);

	return seq;
}

/*
 * If you want users to derive classes from your implementation
 * you need to support this method.
 */
POA_GNOME_Spell_Dictionary__epv *
gnome_spell_dictionary_get_epv (void)
{
	POA_GNOME_Spell_Dictionary__epv *epv;

	epv = g_new0 (POA_GNOME_Spell_Dictionary__epv, 1);


	return epv;
}

static void
gnome_spell_dictionary_class_init (GNOMESpellDictionaryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	POA_GNOME_Spell_Dictionary__epv *epv = &klass->epv;

	dictionary_parent_class   = g_type_class_ref (BONOBO_TYPE_OBJECT);
	object_class->finalize    = dictionary_finalize;

	epv->getLanguages         = impl_gnome_spell_dictionary_get_languages;
	epv->setLanguage          = impl_gnome_spell_dictionary_set_language;
	epv->checkWord            = impl_gnome_spell_dictionary_check_word;
	epv->addWordToSession     = impl_gnome_spell_dictionary_add_word_to_session;
	epv->addWordToPersonal    = impl_gnome_spell_dictionary_add_word_to_personal;
	epv->getSuggestions       = impl_gnome_spell_dictionary_get_suggestions;
	epv->setCorrection        = impl_gnome_spell_dictionary_set_correction;
}

BONOBO_TYPE_FUNC_FULL (
	GNOMESpellDictionary,          /* Glib class name */
	GNOME_Spell_Dictionary,        /* CORBA interface name */
	BONOBO_TYPE_OBJECT,            /* parent type */
	gnome_spell_dictionary);       /* local prefix ie. 'echo'_class_init */

BonoboObject *
gnome_spell_dictionary_new (void)
{
	return g_object_new (GNOME_SPELL_DICTIONARY_TYPE, NULL);
}
