/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* gnome-vfs-mime-info.c - GNOME mime-information implementation.

   Copyright (C) 1998 Miguel de Icaza
   Copyright (C) 2000, 2001 Eazel, Inc.
   All rights reserved.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors:
   Miguel De Icaza <miguel@helixcode.com>
   Mathieu Lacage <mathieu@eazel.com>
*/

#include <config.h>
#include "gnome-vfs-mime-info.h"

#include "gnome-vfs-mime-monitor.h"
#include "gnome-vfs-mime-private.h"
#include "gnome-vfs-mime.h"
#include "gnome-vfs-private-utils.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifdef NEED_GNOMESUPPORT_H
#include "gnomesupport.h"
#endif

#define FAST_FILE_EOF -1
#define FAST_FILE_BUFFER_SIZE (1024 * 16)

typedef struct {
	guchar *ptr;
	guchar *buffer;
	int     length;
	FILE   *fh;
} FastFile;

static void
fast_file_close (FastFile *ffh)
{
	fclose (ffh->fh);
	g_free (ffh->buffer);
}

static gboolean
fast_file_open (FastFile *ffh, const char *filename)
{
	memset (ffh, 0, sizeof (FastFile));

	ffh->fh = fopen (filename, "r");
	if (ffh->fh == NULL) {
		return FALSE;
	}

	ffh->buffer = g_malloc (FAST_FILE_BUFFER_SIZE);
	ffh->ptr = ffh->buffer;

	ffh->length = fread (ffh->buffer, 1, FAST_FILE_BUFFER_SIZE, ffh->fh);

	if (ffh->length < 0) {
		fast_file_close (ffh);
		return FALSE;
	}

	return TRUE;
}

static inline int
fast_file_getc (FastFile *ffh)
{
	if (ffh->ptr < ffh->buffer + ffh->length)
		return *(ffh->ptr++);
	else {
		ffh->length = fread (ffh->buffer, 1, FAST_FILE_BUFFER_SIZE, ffh->fh);

		if (!ffh->length)
			return FAST_FILE_EOF;
		else {
			ffh->ptr = ffh->buffer;
			return *(ffh->ptr++);
		}
	}
}

typedef struct {
	char       *mime_type;
	GHashTable *keys;
} GnomeMimeContext;

/* Describes the directories we scan for information */
typedef struct {
	char *dirname;
	struct stat s;
	unsigned int valid : 1;
	unsigned int system_dir : 1;
	unsigned int force_reload : 1;
} mime_dir_source_t;


#define DELETED_KEY "deleted"
#define DELETED_VALUE "moilegrandvizir"

/* These ones are used to automatically reload mime info on demand */
static mime_dir_source_t gnome_mime_dir, user_mime_dir;
static time_t last_checked;

/* To initialize the module automatically */
static gboolean gnome_vfs_mime_inited = FALSE;

/* you will write back or reload the file if and only if this var' value is 0 */
static int gnome_vfs_is_frozen = 0;

static GList *current_lang = NULL;
/* we want to replace the previous key if the current key has a higher
   language level */
static int previous_key_lang_level = -1;

/*
 * A hash table containing all of the Mime records for specific
 * mime types (full description, like image/png)
 * It also contains a the generic types like image/
 * extracted from .keys files
 */
static GHashTable *specific_types;
/* user specific data */
static GHashTable *specific_types_user;


/*
 * A hash table containing all of the Mime records for all registered
 * mime types
 * extracted from .mime files
 */
static GHashTable *registered_types;
/* user specific data */
static GHashTable *registered_types_user;



/* Prototypes */
static GnomeVFSResult write_back_mime_user_file (void);
static GnomeVFSResult write_back_keys_user_file (void);
static const char *   gnome_vfs_mime_get_registered_mime_type_key (const char *mime_type,
								   const char *key);

void
_gnome_vfs_mime_info_mark_gnome_mime_dir_dirty (void)
{
	gnome_mime_dir.force_reload = TRUE;
}

void
_gnome_vfs_mime_info_mark_user_mime_dir_dirty (void)
{
	user_mime_dir.force_reload = TRUE;
}

static gboolean
does_string_contain_caps (const char *string)
{
	const char *temp_c;

	temp_c = string;
	while (*temp_c != '\0') {
		if (g_ascii_isupper (*temp_c)) {
			return TRUE;
		}
		temp_c++;
	}

	return FALSE;
}

static GnomeMimeContext *
context_new (GHashTable *hash_table, GString *str)
{
	GnomeMimeContext *context;
	char *mime_type;
	char last_char;

	mime_type = g_strdup (str->str);

	last_char = mime_type[strlen (mime_type) - 1];
	if (last_char == '*') {
		mime_type[strlen (mime_type) - 1] = '\0';
	}

	context = g_hash_table_lookup (hash_table, mime_type);

	if (context != NULL) {
		g_free (mime_type);
		return context;
	}

/*	fprintf (stderr, "New context: '%s'\n", mime_type); */

	context = g_new (GnomeMimeContext, 1);
	context->mime_type = mime_type;
	context->keys = g_hash_table_new_full (
		g_str_hash, g_str_equal,
		(GDestroyNotify) g_free,
		(GDestroyNotify) g_free);

	g_hash_table_insert (hash_table, context->mime_type, context);

	return context;
}

static void
context_destroy (GnomeMimeContext *context)
{
	g_hash_table_destroy (context->keys);
	g_free (context->mime_type);
	g_free (context);
}

static void
context_destroy_and_unlink (GnomeMimeContext *context)
{
	/*
	 * Remove the context from our hash tables, we dont know
	 * where it is: so just remove it from both (it can
	 * only be in one).
	 */
	g_hash_table_remove (specific_types,        context->mime_type);
	g_hash_table_remove (registered_types,      context->mime_type);
	g_hash_table_remove (specific_types_user,   context->mime_type);
	g_hash_table_remove (registered_types_user, context->mime_type);

	context_destroy (context);
}

/* this gives us a number of the language in the current language list,
   the higher the number the "better" the translation */
static int
language_level (const char *langage)
{
	int i;
	GList *li;

	if (langage == NULL)
		return 0;

	for (i = 1, li = current_lang; li != NULL; i++, li = g_list_next (li)) {
		if (strcmp ((const char *) li->data, langage) == 0)
			return i;
	}

	return -1;
}


static void
context_add_key (GnomeMimeContext *context, char *key, char *lang, char *value)
{
	int lang_level;

	lang_level = language_level (lang);
	/* wrong language completely */
	if (lang_level < 0)
		return;

	/* if a previous key in the hash had a better lang_level don't do anything */
	if (lang_level > 0 &&
	    previous_key_lang_level > lang_level) {
		return;
	}

/*	fprintf (stderr, "Add key: '%s' '%s' '%s' %d\n", key, lang, value, lang_level); */

	g_hash_table_replace (context->keys, g_strdup (key), g_strdup (value));

	previous_key_lang_level = lang_level;
}

typedef enum {
	STATE_NONE,
	STATE_LANG,
	STATE_LOOKING_FOR_KEY,
	STATE_ON_MIME_TYPE,
	STATE_ON_KEY,
	STATE_ON_VALUE
} ParserState;

/* #define APPEND_CHAR(gstr,c) g_string_insert_c ((gstr), -1, (c)) */

#define APPEND_CHAR(gstr,c) \
	G_STMT_START {                                     \
		if (gstr->len + 1 < gstr->allocated_len) { \
			gstr->str [gstr->len++] = c;       \
			gstr->str [gstr->len] = '\0';      \
		} else {                                   \
			g_string_insert_c (gstr, -1, c);   \
		}                                          \
	} G_STMT_END

typedef enum {
	FORMAT_MIME,
	FORMAT_KEYS
} Format;

#define load_mime_type_info_from(a,b) load_type_info_from ((a), (b), FORMAT_MIME)
#define load_mime_list_info_from(a,b) load_type_info_from ((a), (b), FORMAT_KEYS)

static void
load_type_info_from (const char *filename,
		     GHashTable *hash_table,
		     Format      format)
{
	GString *line;
	int column, c;
	ParserState state;
	FastFile mime_file;
	gboolean skip_line;
	GnomeMimeContext *context;
	int key, lang, last_str_end; /* offsets */

	if (!fast_file_open (&mime_file, filename)) {
		return;
	}

	skip_line = FALSE;
	column = -1;
	context = NULL;
	line = g_string_sized_new (120);
	key = lang = last_str_end = 0;
	state = STATE_NONE;

	while ((c = fast_file_getc (&mime_file)) != EOF) {
	handle_char:
		column++;
		if (c == '\r')
			continue;

		if (c == '#' && column == 0) {
			skip_line = TRUE;
			continue;
		}

		if (skip_line) {
			if (c == '\n') {
				skip_line = FALSE;
				column = -1;
				g_string_truncate (line, 0);
				key = lang = last_str_end = 0;
			}
			continue;
		}

		if (c == '\n') {
			skip_line = FALSE;
			column = -1;
			if (state == STATE_ON_MIME_TYPE) {
				/* setup for a new key */
				previous_key_lang_level = -1;
				context = context_new (hash_table, line);

			} else if (state == STATE_ON_VALUE) {
				APPEND_CHAR (line, '\0');
				context_add_key (context,
						 line->str + key,
						 lang ? line->str + lang : NULL,
						 line->str + last_str_end);
				key = lang = 0;
			}
			g_string_truncate (line, 0);
			last_str_end = 0;
			state = STATE_LOOKING_FOR_KEY;
			continue;
		}

		switch (state) {
		case STATE_NONE:
			if (c == ' ' || c == '\t') {
				break;
			} else if (c == ':') {
				skip_line = TRUE;
				break;
			} else {
				state = STATE_ON_MIME_TYPE;
			}
			/* fall down */

		case STATE_ON_MIME_TYPE:
			if (c == ':') {
				skip_line = TRUE;
				/* setup for a new key */
				previous_key_lang_level = -1;
				context = context_new (hash_table, line);
				state = STATE_LOOKING_FOR_KEY;
				break;
			}
			APPEND_CHAR (line, c);
			break;

		case STATE_LOOKING_FOR_KEY:
			if (c == '\t' || c == ' ') {
				break;
			}

			if (c == '[') {
				state = STATE_LANG;
				break;
			}

			if (column == 0) {
				state = STATE_ON_MIME_TYPE;
				APPEND_CHAR (line, c);
				break;
			}
			state = STATE_ON_KEY;
			/* falldown */

		case STATE_ON_KEY:
			if (c == '\\') {
				c = fast_file_getc (&mime_file);
				if (c == EOF) {
					break;
				}
			}
			if (c == '=') {
				key = last_str_end;
				APPEND_CHAR (line, '\0');
				last_str_end = line->len;
				state = STATE_ON_VALUE;
				break;
			}

			if (format == FORMAT_KEYS && c == ':') {
				key = last_str_end;
				APPEND_CHAR (line, '\0');
				last_str_end = line->len;

				/* Skip space after colon.  There should be one
				 * there.  That is how the file is defined. */
				c = fast_file_getc (&mime_file);
				if (c != ' ') {
					if (c == EOF) {
						break;
					} else {
						goto handle_char;
					}
				} else {
					column++;
				}

				state = STATE_ON_VALUE;
				break;
			}

			APPEND_CHAR (line, c);
			break;

		case STATE_ON_VALUE:
			APPEND_CHAR (line, c);
			break;

		case STATE_LANG:
			if (c == ']') {
				state = STATE_ON_KEY;

				lang = last_str_end;
				APPEND_CHAR (line, '\0');
				last_str_end = line->len;

				if (!line->str [0] ||
				    language_level (line->str + lang) < 0) {
					skip_line = TRUE;
					key = lang = last_str_end = 0;
					g_string_truncate (line, 0);
					state = STATE_LOOKING_FOR_KEY;
				}
			} else {
				APPEND_CHAR (line, c);
			}
			break;
		}
	}

	if (context != NULL) {
		if (key && line->str [0]) {
			APPEND_CHAR (line, '\0');
			context_add_key (context,
					 line->str + key,
					 lang ? line->str + lang : NULL,
					 line->str + last_str_end);
		} else {
			if (g_hash_table_size (context->keys) < 1) {
				context_destroy_and_unlink (context);
			}
		}
	}

	g_string_free (line, TRUE);

	previous_key_lang_level = -1;

	fast_file_close (&mime_file);
}

static void
mime_info_load (mime_dir_source_t *source)
{
	DIR *dir;
	struct dirent *dent;
	const int extlen = sizeof (".keys") - 1;
	char *filename;

	if (stat (source->dirname, &source->s) != -1)
		source->valid = TRUE;
	else
		source->valid = FALSE;

	dir = opendir (source->dirname);
	if (!dir) {
		source->valid = FALSE;
		return;
	}
	if (source->system_dir) {
		filename = g_strconcat (source->dirname, "/gnome-vfs.keys", NULL);
		load_mime_type_info_from (filename, specific_types);
		g_free (filename);
	}

	while ((dent = readdir (dir)) != NULL) {

		int len = strlen (dent->d_name);

		if (len <= extlen)
			continue;
		if (strcmp (dent->d_name + len - extlen, ".keys"))
			continue;
		if (source->system_dir && !strcmp (dent->d_name, "gnome-vfs.keys"))
			continue;

		if (source->system_dir && !strcmp (dent->d_name, "gnome.keys")) {
			/* Ignore the obsolete "official" one so it doesn't override
			 * the new official one.
			 */
			continue;
		}

		if (!source->system_dir && !strcmp (dent->d_name, "user.keys"))
			continue;

		filename = g_strconcat (source->dirname, "/", dent->d_name, NULL);
		load_mime_type_info_from (filename, specific_types);
		g_free (filename);
	}
	if (!source->system_dir) {
		filename = g_strconcat (source->dirname, "/user.keys", NULL);
		load_mime_type_info_from (filename, specific_types_user);
		g_free (filename);
	}
	closedir (dir);
}

static void
mime_list_load (mime_dir_source_t *source)
{
	DIR *dir;
	struct dirent *dent;
	const int extlen = sizeof (".mime") - 1;
	char *filename;

	if (stat (source->dirname, &source->s) != -1)
		source->valid = TRUE;
	else
		source->valid = FALSE;

	dir = opendir (source->dirname);
	if (!dir) {
		source->valid = FALSE;
		return;
	}
	if (source->system_dir) {
		filename = g_strconcat (source->dirname, "/gnome-vfs.mime", NULL);
		load_mime_list_info_from (filename, registered_types);
		g_free (filename);
	}

	while ((dent = readdir (dir)) != NULL) {

		int len = strlen (dent->d_name);

		if (len <= extlen)
			continue;
		if (strcmp (dent->d_name + len - extlen, ".mime"))
			continue;
		if (source->system_dir && !strcmp (dent->d_name, "gnome-vfs.mime"))
			continue;

		if (source->system_dir && !strcmp (dent->d_name, "gnome.mime")) {
			/* Ignore the obsolete "official" one so it doesn't override
			 * the new official one.
			 */
			continue;
		}

		if (!source->system_dir && !strcmp (dent->d_name, "user.mime"))
			continue;

		filename = g_strconcat (source->dirname, "/", dent->d_name, NULL);
		load_mime_list_info_from (filename, registered_types);
		g_free (filename);
	}
	if (!source->system_dir) {
		filename = g_strconcat (source->dirname, "/user.mime", NULL);
		load_mime_list_info_from (filename, registered_types_user);
		g_free (filename);
	}
	closedir (dir);
}

static void
load_mime_type_info (void)
{
	mime_info_load (&gnome_mime_dir);
	mime_info_load (&user_mime_dir);
	mime_list_load (&gnome_mime_dir);
	mime_list_load (&user_mime_dir);
}

static void
gnome_vfs_mime_init (void)
{
	/*
	 * The hash tables that store the mime keys.
	 */
	specific_types = g_hash_table_new (g_str_hash, g_str_equal);
	registered_types  = g_hash_table_new (g_str_hash, g_str_equal);

	specific_types_user = g_hash_table_new (g_str_hash, g_str_equal);
	registered_types_user  = g_hash_table_new (g_str_hash, g_str_equal);

	current_lang = gnome_vfs_i18n_get_language_list ("LC_MESSAGES");

	/*
	 * Setup the descriptors for the information loading
	 */
	gnome_mime_dir.dirname = g_strdup (DATADIR "/mime-info");
	gnome_mime_dir.system_dir = TRUE;

	user_mime_dir.dirname  = g_strconcat
		(g_get_home_dir (), "/.gnome/mime-info", NULL);
	user_mime_dir.system_dir = FALSE;

	/*
	 * Load
	 */
	load_mime_type_info ();

	last_checked = time (NULL);
	gnome_vfs_mime_inited = TRUE;
}

static gboolean
remove_keys (gpointer key, gpointer value, gpointer user_data)
{
	GnomeMimeContext *context = value;

	context_destroy (context);

	return TRUE;
}

static void
reload_if_needed (void)
{
	time_t now = time (NULL);
	gboolean need_reload = FALSE;
	struct stat s;

	if (gnome_vfs_is_frozen > 0)
		return;

	if (gnome_mime_dir.force_reload || user_mime_dir.force_reload)
		need_reload = TRUE;
	else if (now > last_checked + 5) {
		if (stat (gnome_mime_dir.dirname, &s) != -1 &&
		    s.st_mtime != gnome_mime_dir.s.st_mtime)
			need_reload = TRUE;
		else if (stat (user_mime_dir.dirname, &s) != -1 &&
			 s.st_mtime != user_mime_dir.s.st_mtime)
			need_reload = TRUE;
	}

	last_checked = now;

	if (need_reload) {
	        gnome_vfs_mime_info_reload ();
	}
}

static void
gnome_vfs_mime_info_clear (void)
{
	if (specific_types != NULL) {
		g_hash_table_foreach_remove (specific_types, remove_keys, NULL);
	}
	if (registered_types != NULL) {
		g_hash_table_foreach_remove (registered_types, remove_keys, NULL);
	}
	if (specific_types_user != NULL) {
		g_hash_table_foreach_remove (specific_types_user, remove_keys, NULL);
	}
	if (registered_types_user != NULL) {
		g_hash_table_foreach_remove (registered_types_user, remove_keys, NULL);
	}
}

/**
 * _gnome_vfs_mime_info_shutdown:
 * 
 * Remove the MIME database from memory.
 **/
void
_gnome_vfs_mime_info_shutdown (void)
{
	gnome_vfs_mime_info_clear ();

	if (specific_types != NULL) {
		g_hash_table_destroy (specific_types);
		specific_types = NULL;
	}
	if (registered_types != NULL) {
		g_hash_table_destroy (registered_types);
		registered_types = NULL;
	}
	if (specific_types_user != NULL) {
		g_hash_table_destroy (specific_types_user);
		specific_types_user = NULL;
	}
	if (registered_types_user != NULL) {
		g_hash_table_destroy (registered_types_user);
		registered_types_user = NULL;
	}
}

/**
 * gnome_vfs_mime_info_reload:
 *
 * Reload the MIME database from disk and notify any listeners
 * holding active #GnomeVFSMIMEMonitor objects.
 **/
void
gnome_vfs_mime_info_reload (void)
{
	if (!gnome_vfs_mime_inited) {
		gnome_vfs_mime_init ();
	}

	/* 1. Clean */
	gnome_vfs_mime_info_clear ();

	/* 2. Reload */
	load_mime_type_info ();

	/* 3. clear our force flags */
	gnome_mime_dir.force_reload = FALSE;
	user_mime_dir.force_reload = FALSE;

	/* 3. Tell anyone who cares */
	_gnome_vfs_mime_monitor_emit_data_changed (gnome_vfs_mime_monitor_get ());
}


/**
 * gnome_vfs_mime_freeze:
 *
 * Freezes the mime data so that you can do multiple
 * updates to the dat in one batch without needing
 * to back the files to disk or readind them
 */
void
gnome_vfs_mime_freeze (void)
{
	gnome_vfs_is_frozen++;
}



/**
 * gnome_vfs_mime_thaw:
 *
 * UnFreezes the mime data so that you can do multiple
 * updates to the dat in one batch without needing
 * to back the files to disk or readind them
 */
void
gnome_vfs_mime_thaw (void)
{
	gnome_vfs_is_frozen--;

	if (gnome_vfs_is_frozen == 0) {
		write_back_mime_user_file ();
		write_back_keys_user_file ();
	}
}


static GnomeVFSResult
set_value_real (const char *mime_type, const char *key, const char *value,
		GHashTable *user_hash_table)
{
	GnomeMimeContext *context;

	if (mime_type == NULL
	    || key == NULL
	    || value == NULL) {
		return gnome_vfs_result_from_errno ();
	}

	g_return_val_if_fail (!does_string_contain_caps (mime_type),
			      gnome_vfs_result_from_errno ());

	if (!gnome_vfs_mime_inited) {
		gnome_vfs_mime_init ();
	}

	context = g_hash_table_lookup (user_hash_table, mime_type);
	if (context == NULL) {
		GString *string;

		string = g_string_new (mime_type);

		/* create the mime type context */
		context = context_new (user_hash_table, string);
		/* add the info to the mime type context */
		g_hash_table_insert (context->keys, g_strdup (key), g_strdup (value));
	} else
		g_hash_table_replace (context->keys, g_strdup (key), g_strdup (value));

	return GNOME_VFS_OK;
}

/**
 * gnome_vfs_mime_set_value
 * @mime_type: a mime type.
 * @key: a key to store the value in.
 * @value: the value to store in the key.
 *
 * This function is going to set the value
 * associated to the key and it will save it
 * to the user' file if necessary.
 * You should not free the key/values passed to
 * this function. They are used internally.
 *
 * Returns: GNOME_VFS_OK if the operation succeeded, otherwise an error code
 *
 */
GnomeVFSResult
gnome_vfs_mime_set_value (const char *mime_type, const char *key, const char *value)
{
	GnomeVFSResult retval;

	retval = set_value_real (mime_type, key, value, specific_types_user);

	if (gnome_vfs_is_frozen == 0) {
		return write_back_keys_user_file ();
	}

	return retval;
}


static gboolean
is_mime_type_deleted (const char *mime_type)
{
	const char *deleted_key;

	deleted_key = gnome_vfs_mime_get_registered_mime_type_key (mime_type, DELETED_KEY);
	return deleted_key != NULL && strcmp (deleted_key, DELETED_VALUE) == 0;
}

static const char *
get_value_from_hash_table (GHashTable *hash_table, const char *mime_type, const char *key)
{
	GnomeMimeContext *context;
	char *value;

	value = NULL;
	context = g_hash_table_lookup (hash_table, mime_type);
	if (context != NULL) {
		value = g_hash_table_lookup (context->keys, key);
	}
	return value;
}

static const char *
get_value_real (const char *mime_type,
		const char *key,
		GHashTable *user_hash_table,
		GHashTable *system_hash_table)
{
	const char *value;
	char *generic_type, *p;

	g_return_val_if_fail (key != NULL, NULL);
	g_assert (user_hash_table != NULL);
	g_assert (system_hash_table != NULL);

	if (mime_type == NULL) {
		return NULL;
	}

	g_return_val_if_fail (!does_string_contain_caps (mime_type),
			      NULL);

	reload_if_needed ();

	if (strcmp (key, DELETED_KEY) != 0 && is_mime_type_deleted (mime_type)) {
		return NULL;
	}

	value = get_value_from_hash_table (user_hash_table, mime_type, key);
	if (value != NULL) {
		return value;
	}

	value = get_value_from_hash_table (system_hash_table, mime_type, key);
	if (value != NULL) {
		return value;
	}

	generic_type = g_strdup (mime_type);
	p = strchr (generic_type, '/');
	if (p != NULL)
		*(p+1) = '\0';

	value = get_value_from_hash_table (user_hash_table, generic_type, key);
	if (value != NULL) {
		g_free (generic_type);
		return value;
	}

	value = get_value_from_hash_table (system_hash_table, generic_type, key);
	g_free (generic_type);
	if (value != NULL) {
		return value;
	}

	return NULL;
}


/**
 * gnome_vfs_mime_get_value:
 * @mime_type: a mime type.
 * @key: A key to lookup for the given mime-type
 *
 * This function retrieves the value associated with @key in
 * the given GnomeMimeContext.  The string is private, you
 * should not free the result.
 *
 * Returns: GNOME_VFS_OK if the operation succeeded, otherwise an error code
 */
const char *
gnome_vfs_mime_get_value (const char *mime_type, const char *key)
{
	if (!gnome_vfs_mime_inited)
		gnome_vfs_mime_init ();

	return get_value_real (mime_type, key, specific_types_user, specific_types);
}

/**
 * gnome_vfs_mime_type_is_known:
 * @mime_type: a mime type.
 *
 * This function returns TRUE if @mime_type is in the MIME database at all.
 *
 * Returns: TRUE if anything is known about @mime_type, otherwise FALSE
 */
gboolean
gnome_vfs_mime_type_is_known (const char *mime_type)
{
	if (mime_type == NULL) {
		return FALSE;
	}

	g_return_val_if_fail (!does_string_contain_caps (mime_type),
			      FALSE);

	if (!gnome_vfs_mime_inited)
		gnome_vfs_mime_init ();

	reload_if_needed ();

	if (g_hash_table_lookup (specific_types, mime_type)) {
		return TRUE;
	}

	if (g_hash_table_lookup (specific_types_user, mime_type)) {
		return TRUE;
	}

	if (g_hash_table_lookup (registered_types, mime_type)) {
		return TRUE;
	}

	if (g_hash_table_lookup (registered_types_user, mime_type)) {
		return TRUE;
	}

	return FALSE;
}


/**
 * gnome_vfs_mime_keys_list_free:
 * @mime_type_list: A mime type list to free.
 *
 * Frees the mime type list.
 */
void
gnome_vfs_mime_keys_list_free (GList *mime_type_list)
{
	/* we do not need to free the data in the list since
	   the data was stolen from the internal hash table
	   This function is there so that people do not need
	   to know this particuliar implementation detail.
	*/

	g_list_free (mime_type_list);
}

static void
assemble_list (gpointer key, gpointer value, gpointer user_data)
{
	GList **listp = user_data;

	(*listp) = g_list_prepend ((*listp), key);
}

/**
 * gnome_vfs_mime_get_key_list:
 * @mime_type: the MIME type to lookup
 *
 * Gets a list of all keys associated with @mime_type.
 *
 * Return value: a GList of const char * representing keys associated
 * with @mime_type
 **/
GList *
gnome_vfs_mime_get_key_list (const char *mime_type)
{
	char *p, *generic_type;
	GnomeMimeContext *context;
	GList *list = NULL, *l;

	if (mime_type == NULL) {
		return NULL;
	}
	g_return_val_if_fail (!does_string_contain_caps (mime_type),
			      NULL);

	if (!gnome_vfs_mime_inited)
		gnome_vfs_mime_init ();

	reload_if_needed ();

	generic_type = g_strdup (mime_type);
	p = strchr (generic_type, '/');
	if (p != NULL)
		*(p+1) = 0;

	context = g_hash_table_lookup (specific_types_user, generic_type);
	if (context != NULL) {
		g_hash_table_foreach (
			context->keys, assemble_list, &list);
	}

	context = g_hash_table_lookup (specific_types, generic_type);
	if (context != NULL) {
		g_hash_table_foreach (
			context->keys, assemble_list, &list);
	}

	g_free (generic_type);
	for (l = list; l;) {
		if (l->next) {
			void *this = l->data;
			GList *m;

			for (m = l->next; m; m = m->next) {
				if (strcmp ((char*) this, (char*) m->data) != 0)
					continue;
				list = g_list_remove (list, m->data);
				break;
			}
		}
		l = l->next;
	}

	return list;
}

gint
str_cmp_callback  (gconstpointer a,
		   gconstpointer b);
gint
str_cmp_callback  (gconstpointer a,
		   gconstpointer b)
{
	return (strcmp ((char *)a, (char *)b));
}

/**
 * gnome_vfs_mime_set_extensions_list:
 * @mime_type: the mime type.
 * @extensions_list: a whitespace-separated list of the
 *                   extensions to set for this mime type.
 *
 * Sets the extensions for a given mime type. Overrides
 * the previously set extensions.
 *
 * Return value: GNOME_VFS_OK if the operation succeeded, otherwise an error code.
 */
GnomeVFSResult
gnome_vfs_mime_set_extensions_list (const char *mime_type,
				    const char *extensions_list)
{
	return gnome_vfs_mime_set_registered_type_key (mime_type, "ext", extensions_list);
}


/**
 * gnome_vfs_mime_get_extensions_list:
 * @mime_type: type to get the extensions of
 *
 * Get the file extensions associated with mime type @mime_type.
 *
 * Return value: a GList of char *s
 **/
GList *
gnome_vfs_mime_get_extensions_list (const char *mime_type)
{
	GList *list;
	const char *extensions_system, *extensions_user;
	char *extensions;
	gchar **elements;
	int index;
	GnomeMimeContext *context;

	if (mime_type == NULL) {
		return NULL;
	}
	g_return_val_if_fail (!does_string_contain_caps (mime_type),
			      NULL);


	if (!gnome_vfs_mime_inited) {
		gnome_vfs_mime_init ();
	}

	reload_if_needed ();

	extensions_system = NULL;
	extensions_user = NULL;

	context = g_hash_table_lookup (registered_types_user, mime_type);
	if (context != NULL) {
		extensions_user = g_hash_table_lookup (context->keys, "ext");
	}

	context = g_hash_table_lookup (registered_types, mime_type);
	if (context != NULL) {
		extensions_system = g_hash_table_lookup (context->keys, "ext");
	}


	extensions = NULL;
	if (extensions_user != NULL) {
		extensions = g_strdup (extensions_user);
	} else if (extensions_system != NULL) {
		extensions = g_strdup (extensions_system);
	}

	/* build a GList from the string */
	list = NULL;
	if (extensions != NULL) {
		/* Parse the extensions and add to list */
		elements = g_strsplit (extensions, " ", 0);
		if (elements != NULL) {
			index = 0;

			while (elements[index] != NULL) {
				if (strcmp (elements[index], "") != 0) {
					list = g_list_append (list, g_strdup (elements[index]));
				}
				index++;
			}
			g_strfreev (elements);
		}
	}

	g_free (extensions);

	return list;
}


/**
 * gnome_vfs_mime_get_extensions_string:
 * @mime_type: the mime type
 *
 * Retrieves the extensions associated with @mime_type as a single
 * space seperated string.
 *
 * Return value: a string containing space seperated extensions for @mime_type
 */
char *
gnome_vfs_mime_get_extensions_string (const char *mime_type)
{
	GList *extensions_list, *temp_list;
	char *extensions;

	if (mime_type == NULL) {
		return NULL;
	}
	g_return_val_if_fail (!does_string_contain_caps (mime_type),
			      NULL);


	/* it might seem overkill to use gnome_vfs_mime_get_extensions_list
	   here but it has the advantage that this function returns
	   a list of unique extensions */

	extensions_list = gnome_vfs_mime_get_extensions_list (mime_type);
	if (extensions_list == NULL) {
		return NULL;
	}
	extensions = NULL;

	for (temp_list = extensions_list; temp_list != NULL; temp_list = temp_list->next) {
		char *temp_string;
		temp_string = g_strconcat (temp_list->data, " ", extensions, NULL);
		g_free (extensions);
		extensions = temp_string;
	}

	extensions[strlen (extensions) - 1] = '\0';
	return extensions;
}

/**
 * gnome_vfs_mime_get_extensions_pretty_string:
 * @mime_type: the mime type
 *
 * Returns the supported extensions for @mime_type as a comma-seperated list.
 *
 * Return value: a string containing comma seperated extensions for this mime-type
 **/
char *
gnome_vfs_mime_get_extensions_pretty_string (const char *mime_type)
{
	GList *extensions, *element;
	char *ext_str, *tmp_str;

	if (mime_type == NULL) {
		return NULL;
	}

	ext_str = NULL;
	tmp_str = NULL;

	if (!gnome_vfs_mime_inited) {
		gnome_vfs_mime_init ();
	}

	reload_if_needed ();

	extensions = gnome_vfs_mime_get_extensions_list (mime_type);
	if (extensions == NULL) {
		return NULL;
	}

	for (element = extensions; element != NULL; element = element->next) {
		if (ext_str != NULL) {
			tmp_str = ext_str;

			if (element->next == NULL) {
				ext_str = g_strconcat (tmp_str, ".", (char *)element->data, NULL);
			} else {
				ext_str = g_strconcat (tmp_str, ".", (char *)element->data, ", ", NULL);
			}
			g_free (tmp_str);
		} else {
			if (g_list_length (extensions) == 1) {
				ext_str = g_strconcat (".", (char *)element->data, NULL);
			} else {
				ext_str = g_strconcat (".", (char *)element->data, ", ", NULL);
			}
		}
	}
	gnome_vfs_mime_extensions_list_free (extensions);

	return ext_str;
}


/**
 * gnome_vfs_mime_extensions_list_free:
 * @list: the extensions list
 *
 * Call this function on the list returned by gnome_vfs_mime_extensions
 * to free the list and all of its elements.
 **/
void
gnome_vfs_mime_extensions_list_free (GList *list)
{
	if (list == NULL) {
		return;
	}
	g_list_foreach (list, (GFunc) g_free, NULL);
	g_list_free (list);
}



static gint
mime_list_sort (gconstpointer a, gconstpointer b)
{
  return (strcmp ((const char *) a, (const char *) b));
}

/**
 *  get_key_name
 *
 *  Hash table function that adds the name of the mime type
 *  to the supplied GList.
 *
 */
static void
get_key_name (gpointer key, gpointer value, gpointer user_data)
{
	GnomeMimeContext *context;
	char *name;
	GList **list = user_data;
	GList *duplicate;

	if (value == NULL || key == NULL) {
		return;
	}

	context = (GnomeMimeContext *) value;

	if (context->mime_type[0] == '#') {
		return;
	}

	if (is_mime_type_deleted (context->mime_type)) {
		return;
	}

	/* Get name from key and exit if key is NULL or string is empty */
	name = (char *)key;
	if (name == NULL || strlen (name) == 0) {
		return;
	}

	duplicate = NULL;
	duplicate = g_list_find_custom ((*list), context->mime_type, (GCompareFunc)strcmp);
	if (duplicate == NULL) {
		(*list) = g_list_insert_sorted ((*list), g_strdup(context->mime_type), mime_list_sort);
	}

}

/**
 * gnome_vfs_mime_reset
 *
 * resets the user's mime database to the system defaults.
 */
void
gnome_vfs_mime_reset (void)
{
	char *filename;

	filename = g_strconcat (user_mime_dir.dirname, "/user.keys", NULL);
	unlink (filename);
	g_free (filename);

	filename = g_strconcat (user_mime_dir.dirname, "/user.mime", NULL);
	unlink (filename);
	g_free (filename);
}


/**
 * gnome_vfs_mime_registered_mime_type_delete:
 * @mime_type: string representing the existing type to delete
 *
 * Delete a mime type for the user which runs this command.
 * You can undo this only by calling gnome_vfs_mime_reset
 */

void
gnome_vfs_mime_registered_mime_type_delete (const char *mime_type)
{
	gnome_vfs_mime_set_registered_type_key (mime_type,
						DELETED_KEY,
						DELETED_VALUE);

}

/*
 * gnome_vfs_get_registered_mime_types
 *
 *  Return the list containing the name of all
 *  registrered mime types.
 *  This function is costly in terms of speed.
 */
GList *
gnome_vfs_get_registered_mime_types (void)
{
	GList *type_list = NULL;

	if (!gnome_vfs_mime_inited) {
		gnome_vfs_mime_init ();
	}

	reload_if_needed ();

	/* Extract mime type names */
	g_hash_table_foreach (registered_types_user, get_key_name, &type_list);
	g_hash_table_foreach (registered_types, get_key_name, &type_list);

	return type_list;
}

/**
 * gnome_vfs_mime_registered_mime_type_list_free:
 * @list: the extensions list
 *
 * Call this function on the list returned by gnome_vfs_get_registered_mime_types
 * to free the list and all of its elements.
 */
void
gnome_vfs_mime_registered_mime_type_list_free (GList *list)
{
	if (list == NULL) {
		return;
	}

	g_list_foreach (list, (GFunc) g_free, NULL);
	g_list_free (list);
}

/**
 * gnome_vfs_mime_set_registered_type_key:
 * @mime_type: 	Mime type to set key for
 * @key: 	The key to set
 * @data: 	The data to set for the key
 *
 * This function sets the key data for the registered mime
 * type's hash table.
 *
 * Returns: GNOME_VFS_OK if the operation succeeded, otherwise an error code
 */
GnomeVFSResult
gnome_vfs_mime_set_registered_type_key (const char *mime_type, const char *key, const char *value)
{
	GnomeVFSResult result;

	result = set_value_real (mime_type, key, value, registered_types_user);

	if (gnome_vfs_is_frozen == 0) {
		result = write_back_mime_user_file ();
	}

	return result;
}

/**
 * gnome_vfs_mime_get_registered_mime_type_key
 * @mime_type: a mime type.
 * @key: A key to lookup for the given mime-type
 *
 * This function retrieves the value associated with @key in
 * the given GnomeMimeContext.  The string is private, you
 * should not free the result.
 */
static const char *
gnome_vfs_mime_get_registered_mime_type_key (const char *mime_type, const char *key)
{
	if (!gnome_vfs_mime_inited)
		gnome_vfs_mime_init ();

	return get_value_real (mime_type, key,
			       registered_types_user,
			       registered_types);
}


static gboolean
ensure_user_directory_exist (void)
{
	DIR *dir;
	gboolean retval;

	if (stat (user_mime_dir.dirname, &user_mime_dir.s) != -1)
		user_mime_dir.valid = TRUE;
	else
		user_mime_dir.valid = FALSE;

	dir = NULL;
	dir = opendir (user_mime_dir.dirname);
	if (dir == NULL) {
		int result;

		result = mkdir (user_mime_dir.dirname, S_IRWXU );
		if (result != 0) {
			user_mime_dir.valid = FALSE;
			return FALSE;
		}
		dir = opendir (user_mime_dir.dirname);
		if (dir == NULL) {
			user_mime_dir.valid = FALSE;
		}
	}

	retval = (dir != NULL);
	if (retval) 
		closedir (dir);
	return retval;
}


static void
write_back_mime_user_file_context_callback (char *key_data,
					    char *value_data,
					    FILE *file) 
{
	fprintf (file, "\t%s: %s\n", key_data, value_data);
}
static void
write_back_mime_user_file_callback (char		*mime_type,
				    GnomeMimeContext	*context,
				    FILE		*file)
{
	fprintf (file, "%s\n", mime_type);
	g_hash_table_foreach (context->keys,
		(GHFunc) write_back_mime_user_file_context_callback,
		file);
	fprintf (file, "\n");
}

static GnomeVFSResult
write_back_mime_user_file (void)
{
	FILE *file;
	char *filename;

	if (!ensure_user_directory_exist ()) {
		return gnome_vfs_result_from_errno ();
	}

	if (!user_mime_dir.system_dir) {
		filename = g_strconcat (user_mime_dir.dirname, "/user.mime", NULL);

        	remove (filename);
		file = fopen (filename, "w");
		if (file == NULL)
			return gnome_vfs_result_from_errno ();

		fprintf (file,
			 "# This file was autogenerated by gnome-vfs-mime-info.\n"
			 "# Do not edit by hand.\n");

		g_hash_table_foreach (registered_types_user,
			(GHFunc) write_back_mime_user_file_callback,
			file);

		/* Cleanup file */
		fclose (file);
		g_free (filename);
	}

	return GNOME_VFS_OK;
}

static void
write_back_keys_user_file_context_callback (char *key_data,
					    char *value_data,
					    FILE *file)
{
	fprintf (file, "\t%s=%s\n", key_data, value_data);
}
static void
write_back_keys_user_file_callback (char	     *mime_type,
				    GnomeMimeContext *context,
				    FILE	     *file)

{
	fprintf (file, "%s\n", mime_type);
	g_hash_table_foreach (context->keys,
		(GHFunc) write_back_keys_user_file_context_callback,
		file);
	fprintf (file, "\n");
}

static GnomeVFSResult
write_back_keys_user_file (void)
{
	FILE *file;
	char *filename;

	if (!ensure_user_directory_exist ()) {
		return gnome_vfs_result_from_errno ();
	}

	if (!user_mime_dir.system_dir) {
		filename = g_strconcat (user_mime_dir.dirname, "/user.keys", NULL);

        	remove (filename);
		file = fopen (filename, "w");
		if (file == NULL) {
			return gnome_vfs_result_from_errno ();
		}


		fprintf (file, "# this file was autogenerated by gnome-vfs-mime-info.\n"
			 "# DO NOT EDIT BY HAND\n");

		g_hash_table_foreach (specific_types_user,
			(GHFunc) write_back_keys_user_file_callback,
			file);

		/* Cleanup file */
		fclose (file);
		g_free (filename);
	}

	return GNOME_VFS_OK;
}
