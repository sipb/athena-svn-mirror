/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* gnome-vfs-mime-info.c - GNOME mime-information implementation.

   Copyright (C) 1998 Miguel de Icaza
   Copyright (C) 2000 Eazel, Inc
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
   Boston, MA 02111-1307, USA.  */
/*
 * Authors: George Lebl
 * 	Based on original mime-info database code by Miguel de Icaza
 */

#include "config.h"

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>
#include <glib.h>

#include "gnome-vfs-types.h"
#include "gnome-vfs-result.h"
#include "gnome-vfs-mime-handlers.h"
#include "gnome-vfs-application-registry.h"
#include "gnome-vfs-private.h"

#if !defined getc_unlocked && !defined HAVE_GETC_UNLOCKED
# define getc_unlocked(fp) getc (fp)
#endif

typedef struct _Application Application;
struct _Application {
	char       *app_id;
	int         ref_count;
	/* The following is true if this was found in the
	 * home directory or if the user changed any keys
	 * here.  It means that it will be saved into a user
	 * file */
	gboolean    user_owned;
	GHashTable *keys;
	GList      *mime_types;
	/* The user_owned version of this if this is a system
	 * version */
	Application *user_application;
};

/* Describes the directories we scan for information */
typedef struct {
	char *dirname;
	struct stat s;
	unsigned int valid : 1;
	unsigned int system_dir : 1;
} ApplicationRegistryDir;

/* These ones are used to automatically reload mime info on demand */
static ApplicationRegistryDir gnome_registry_dir;
static ApplicationRegistryDir user_registry_dir;
static time_t last_checked;

/* To initialize the module automatically */
static gboolean gnome_vfs_application_registry_initialized = FALSE;


static GList *current_lang = NULL;
/* we want to replace the previous key if the current key has a higher
   language level */
static char *previous_key = NULL;
static int previous_key_lang_level = -1;

/*
 * A hash table containing application registry record (Application)
 * keyed by application ids.
 */
static GHashTable *applications = NULL;
/*
 * A hash table containing GList's of application registry records (Application)
 * keyed by the mime types
 */
/* specific mime_types (e.g. image/png) */
static GHashTable *specific_mime_types = NULL;
/* generic mime_types (e.g. image/<star>) */
static GHashTable *generic_mime_types = NULL;

/*
 * Dirty flag, just to make sure we don't sync needlessly
 */
static gboolean user_file_dirty = FALSE;

/*
 * Some local prototypes
 */
static void gnome_vfs_application_registry_init (void);
static void application_clear_mime_types (Application *application);

static Application *
application_ref (Application *application)
{
	g_return_val_if_fail(application != NULL, NULL);

	application->ref_count ++;

	return application;
}

static void
hash_foreach_free_key_value(gpointer key, gpointer value, gpointer user_data)
{
	g_free(key);
	g_free(value);
}

static void
application_unref (Application *application)
{
	g_return_if_fail(application != NULL);

	application->ref_count --;

	if (application->ref_count == 0) {
		application_clear_mime_types (application);

		if (application->keys != NULL) {
			g_hash_table_foreach(application->keys, hash_foreach_free_key_value, NULL);
			g_hash_table_destroy(application->keys);
			application->keys = NULL;
		}

		g_free(application->app_id);
		application->app_id = NULL;

		if (application->user_application != NULL) {
			application_unref (application->user_application);
			application->user_application = NULL;
		}

		g_free(application);
	}
}

static Application *
application_new (const char *app_id, gboolean user_owned)
{
	Application *application;

	application = g_new0 (Application, 1);
	application->app_id = g_strdup(app_id);
	application->ref_count = 1;
	application->user_owned = user_owned;
	application->keys = NULL;
	application->mime_types = NULL;
	application->user_application = NULL;

	return application;
}

static Application *
application_lookup_or_create (const char *app_id, gboolean user_owned)
{
	Application *application;

	g_return_val_if_fail(app_id != NULL, NULL);

	application = g_hash_table_lookup (applications, app_id);
	if (application != NULL) {
		if ( ! user_owned) {
			/* if we find only a user app, do magic */
			if (application->user_owned) {
				Application *new_application;
				new_application = application_new (app_id, FALSE/*user_owned*/);
				new_application->user_application = application;
				/* override the user application */
				g_hash_table_insert (applications, new_application->app_id,
						     new_application);
				return new_application;
			} else {
				return application;
			}
		} else {
			if (application->user_owned) {
				return application;
			} if (application->user_application != NULL) {
				return application->user_application;
			} else {
				Application *new_application;
				new_application = application_new (app_id, TRUE/*user_owned*/);
				application->user_application = new_application;
				return new_application;
			}
		}
	}

	application = application_new (app_id, user_owned);

	g_hash_table_insert (applications, application->app_id, application);

	return application;
}

static Application *
application_lookup (const char *app_id)
{
	g_return_val_if_fail(app_id != NULL, NULL);

	if (applications == NULL)
		return NULL;

	return g_hash_table_lookup (applications, app_id);
}

static const char *
peek_value (const Application *application, const char *key)
{
	g_return_val_if_fail(application != NULL, NULL);
	g_return_val_if_fail(key != NULL, NULL);

	if (application->keys == NULL)
		return NULL;

	return g_hash_table_lookup (application->keys, key);
}

static void
set_value (Application *application, const char *key, const char *value)
{
	char *old_value, *old_key;

	g_return_if_fail(application != NULL);
	g_return_if_fail(key != NULL);
	g_return_if_fail(value != NULL);

	if (application->keys == NULL)
		application->keys = g_hash_table_new (g_str_hash, g_str_equal);

	if (g_hash_table_lookup_extended (application->keys, key,
					  (gpointer *)&old_key,
					  (gpointer *)&old_value)) {
		g_hash_table_insert (application->keys,
				     old_key, g_strdup (value));
		g_free (old_value);
	} else {
		g_hash_table_insert (application->keys,
				     g_strdup (key), g_strdup (value));
	}
}

static void
unset_key (Application *application, const char *key)
{
	char *old_value, *old_key;

	g_return_if_fail(application != NULL);
	g_return_if_fail(key != NULL);

	if (application->keys == NULL)
		return;

	if (g_hash_table_lookup_extended (application->keys, key,
					  (gpointer *)&old_key,
					  (gpointer *)&old_value)) {
		g_hash_table_remove (application->keys, old_key);
		g_free (old_key);
		g_free (old_value);
	}
}

static gboolean
get_bool_value (const Application *application, const char *key,
		gboolean *got_key)
{
	const char *value = peek_value (application, key);
	if (got_key) {
		if (value != NULL)
			*got_key = TRUE;
		else
			*got_key = FALSE;
	}
	if (value &&
	    (value[0] == 'T' ||
	     value[0] == 't' ||
	     value[0] == 'Y' ||
	     value[0] == 'y' ||
	     atoi (value) != 0))
		return TRUE;
	else
		return FALSE;
}

static void
set_bool_value (Application *application, const char *key,
		gboolean value)
{
	set_value (application, key, value ? "true" : "false");
}

static int
application_compare (Application *application1,
		     Application *application2)
{
	return strcmp (application1->app_id, application2->app_id);
}

static void
add_mime_type (Application *application, const char *mime_type)
{
	GList *app_list;
	char *old_key;
	GHashTable *table;

	g_return_if_fail(application != NULL);
	g_return_if_fail(mime_type != NULL);

	/* if this exists already, just return */
	if (g_list_find_custom (application->mime_types,
				/*glib is const incorrect*/(gpointer)mime_type,
				(GCompareFunc) strcmp) != NULL)
		return;

	if (strstr (mime_type, "/*") != NULL)
		table = generic_mime_types;
	else
		table = specific_mime_types;

	g_assert (table != NULL);

	application->mime_types =
		g_list_insert_sorted (application->mime_types,
				      g_strdup (mime_type),
				      (GCompareFunc) strcmp);

	if (g_hash_table_lookup_extended (table, mime_type,
					  (gpointer *)&old_key,
					  (gpointer *)&app_list)) {
		/* Sorted order is important as we can then easily
		 * uniquify the results */
		app_list = g_list_insert_sorted
			(app_list,
			 application_ref (application),
			 (GCompareFunc) application_compare);
		g_hash_table_insert (table, old_key, app_list);
	} else {
		app_list = g_list_prepend (NULL,
					   application_ref (application));
		g_hash_table_insert (table, g_strdup (mime_type), app_list);
	}
}

static void
remove_mime_type (Application *application, const char *mime_type)
{
	GList *app_list, *entry;
	char *old_key;
	GHashTable *table;

	g_return_if_fail(application != NULL);
	g_return_if_fail(mime_type != NULL);

	entry = g_list_find_custom
		(application->mime_types,
		 /*glib is const incorrect*/(gpointer)mime_type,
		 (GCompareFunc) strcmp);

	/* if this doesn't exist, just return */
	if (entry == NULL)
		return;

	if (strstr (mime_type, "/*") != NULL)
		table = generic_mime_types;
	else
		table = specific_mime_types;

	g_assert (table != NULL);

	application->mime_types =
		g_list_remove_link (application->mime_types, entry);
	g_free (entry->data);
	entry->data = NULL;
	g_list_free_1 (entry);

	if (g_hash_table_lookup_extended (table, mime_type,
					  (gpointer *)&old_key,
					  (gpointer *)&app_list)) {
		entry = g_list_find (app_list, application);

		/* if this fails we're in deep doodoo I guess */
		g_assert (entry != NULL);

		app_list = g_list_remove_link (app_list, entry);
		entry->data = NULL;
		application_unref (application);

		if (app_list != NULL) {
			g_hash_table_insert (table, old_key, app_list);
		} else {
			g_hash_table_remove (table, old_key);
			g_free(old_key);
		}
	} else
		g_assert_not_reached ();
}

static void
application_clear_mime_types (Application *application)
{
	g_return_if_fail (application != NULL);

	while (application->mime_types)
		remove_mime_type (application, application->mime_types->data);
}

static void
application_remove (Application *application)
{
	Application *main_application;

	g_return_if_fail (application != NULL);

	if (applications == NULL)
		return;

	main_application = application_lookup (application->app_id);
	if (main_application == NULL)
		return;

	/* We make sure the mime types are killed even if the application
	 * entry lives after unreffing it */
	application_clear_mime_types (application);

	if (main_application == application) {
		if (application->user_application)
			application_clear_mime_types (application->user_application);

		g_hash_table_remove (applications, application->app_id);
	} else {
		/* This must be a user application */
		g_assert (main_application->user_application == application);

		main_application->user_application = NULL;
	}

	application_unref (application);

}

static void
sync_key (gpointer key, gpointer value, gpointer user_data)
{
	char *key_string = key;
	char *value_string = value;
	FILE *fp = user_data;

	fprintf (fp, "\t%s=%s\n", key_string, value_string);
}

/* write an application to a file */
static void
application_sync (Application *application, FILE *fp)
{
	GList *li;

	g_return_if_fail (application != NULL);
	g_return_if_fail (fp != NULL);

	fprintf (fp, "%s\n", application->app_id);

	if (application->keys != NULL)
		g_hash_table_foreach (application->keys, sync_key, fp);

	if (application->mime_types != NULL) {
		char *separator;
		fprintf (fp, "\tmime_types=");
		separator = "";
		for (li = application->mime_types; li != NULL; li = li->next) {
			char *mime_type = li->data;
			fprintf (fp, "%s%s", separator, mime_type);
			separator = ",";
		}
		fprintf (fp, "\n");
	}
	fprintf (fp, "\n");
}


/* this gives us a number of the language in the current language list,
   the higher the number the "better" the translation */
static int
language_level (const char *lang)
{
	int i;
	GList *li;

	if (lang == NULL)
		return 0;

	for (i = 1, li = current_lang; li != NULL; i++, li = g_list_next (li)) {
		if (strcmp ((const char *) li->data, lang) == 0)
			return i;
	}

	return -1;
}


static void
application_add_key (Application *application, const char *key,
		     const char *lang, const char *value)
{
	int lang_level;

	/* special case */
	if (strcmp (key, "mime_types") == 0) {
		char *tmp = g_strdup (value);
		char *p;

		if (lang != NULL)
			g_warning ("Trying to \"translate\" mime types, bad!");

		p = strtok (tmp, ", \t");
		while (p) {
			add_mime_type(application, p);
			p = strtok (NULL, ", \t");
		}

		g_free(tmp);

		return;
	}

	lang_level = language_level (lang);
	/* wrong language completely */
	if (lang_level < 0)
		return;

	/* if we have some language defined and
	   if there was a previous_key */
	if (lang_level > 0 &&
	    previous_key &&
	    /* our language level really sucks and the previous
	       translation was of better language quality so just
	       ignore us */
	    previous_key_lang_level > lang_level) {
		return;
	}

	set_value (application, key, value);

	/* set this as the previous key */
	g_free(previous_key);
	previous_key = g_strdup(key);
	previous_key_lang_level = lang_level;
}

typedef enum {
	STATE_NONE,
	STATE_LANG,
	STATE_LOOKING_FOR_KEY,
	STATE_ON_APPLICATION,
	STATE_ON_KEY,
	STATE_ON_VALUE
} ParserState;

static void
load_application_info_from (const char *filename, gboolean user_owned)
{
	FILE *fp;
	gboolean in_comment, app_used;
	GString *line;
	int column, c;
	ParserState state;
	Application *application;
	char *key;
	char *lang;
	
	fp = fopen (filename, "r");
	if (fp == NULL)
		return;

	in_comment = FALSE;
	app_used = FALSE;
	column = -1;
	application = NULL;
	key = NULL;
	lang = NULL;
	line = g_string_sized_new (120);
	state = STATE_NONE;
	
	while ((c = getc_unlocked (fp)) != EOF){
		column++;
		if (c == '\r')
			continue;

		if (c == '#' && column == 0){		
			in_comment = TRUE;
			continue;
		}
		
		if (c == '\n'){
			in_comment = FALSE;
			column = -1;
			if (state == STATE_ON_APPLICATION){

				/* set previous key to nothing
				   for this mime type */
				g_free(previous_key);
				previous_key = NULL;
				previous_key_lang_level = -1;

				application = application_lookup_or_create (line->str, user_owned);
				app_used = FALSE;
				g_string_assign (line, "");
				state = STATE_LOOKING_FOR_KEY;
				continue;
			}
			if (state == STATE_ON_VALUE){
				app_used = TRUE;
				application_add_key (application, key, lang, line->str);
				g_string_assign (line, "");
				g_free (key);
				key = NULL;
				g_free (lang);
				lang = NULL;
				state = STATE_LOOKING_FOR_KEY;
				continue;
			}
			continue;
		}

		if (in_comment)
			continue;

		switch (state){
		case STATE_NONE:
			if (c != ' ' && c != '\t')
				state = STATE_ON_APPLICATION;
			else
				break;
			/* fall down */
			
		case STATE_ON_APPLICATION:
			if (c == ':'){
				in_comment = TRUE;
				break;
			}
			g_string_append_c (line, c);
			break;

		case STATE_LOOKING_FOR_KEY:
			if (c == '\t' || c == ' ')
				break;

			if (c == '['){
				state = STATE_LANG;
				break;
			}

			if (column == 0){
				state = STATE_ON_APPLICATION;
				g_string_append_c (line, c);
				break;
			}
			state = STATE_ON_KEY;
			/* falldown */

		case STATE_ON_KEY:
			if (c == '\\'){
				c = getc (fp);
				if (c == EOF)
					break;
			}
			if (c == '='){
				key = g_strdup (line->str);
				g_string_assign (line, "");
				state = STATE_ON_VALUE;
				break;
			}
			g_string_append_c (line, c);
			break;

		case STATE_ON_VALUE:
			g_string_append_c (line, c);
			break;
			
		case STATE_LANG:
			if (c == ']'){
				state = STATE_ON_KEY;      
				if (line->str [0]){
					g_free(lang);
					lang = g_strdup(line->str);
				} else {
					in_comment = TRUE;
					state = STATE_LOOKING_FOR_KEY;
				}
				g_string_assign (line, "");
				break;
			}
			g_string_append_c (line, c);
			break;
		}
	}

	if (application){
		if (key && line->str [0])
			application_add_key (application, key, lang, line->str);
		else
			if ( ! app_used)
				application_remove (application);
	}

	g_string_free (line, TRUE);
	g_free (key);
	g_free (lang);

	/* free the previous_key stuff */
	g_free(previous_key);
	previous_key = NULL;
	previous_key_lang_level = -1;

	fclose (fp);
}

static void
application_info_load (ApplicationRegistryDir *source)
{
	DIR *dir;
	struct dirent *dent;
	const int extlen = sizeof (".applications") - 1;
	char *filename;

	if (stat (source->dirname, &source->s) != -1)
		source->valid = TRUE;
	else
		source->valid = FALSE;
	
	dir = opendir (source->dirname);
	if (dir == NULL) {
		source->valid = FALSE;
		return;
	}
	if (source->system_dir) {
		filename = g_strconcat (source->dirname, "/gnome-vfs.applications", NULL);
		load_application_info_from (filename, FALSE /*user_owned*/);
		g_free (filename);
	}

	while ((dent = readdir (dir)) != NULL){
		
		int len = strlen (dent->d_name);

		if (len <= extlen)
			continue;
		if (strcmp (dent->d_name + len - extlen, ".applications"))
			continue;
		if (source->system_dir && strcmp (dent->d_name, "gnome-vfs.applications") == 0)
			continue;
		if ( ! source->system_dir && strcmp (dent->d_name, "user.applications") == 0)
			continue;
		filename = g_strconcat (source->dirname, "/", dent->d_name, NULL);
		load_application_info_from (filename, FALSE /*user_owned*/);
		g_free (filename);
	}

	if ( ! source->system_dir) {
		filename = g_strconcat (source->dirname, "/user.applications", NULL);
		/* Currently this is the only file that is "user owned".  It actually makes
		 * sense.  Editting of other files from the API would be too complex */
		load_application_info_from (filename, TRUE /*user_owned*/);
		g_free (filename);
	}
	closedir (dir);
}

static void
load_application_info (void)
{
	application_info_load (&gnome_registry_dir);
	application_info_load (&user_registry_dir);
}

static void
gnome_vfs_application_registry_init (void)
{
	if (gnome_vfs_application_registry_initialized)
		return;

	/*
	 * The hash tables that store the mime keys.
	 */
	applications = g_hash_table_new (g_str_hash, g_str_equal);
	generic_mime_types  = g_hash_table_new (g_str_hash, g_str_equal);
	specific_mime_types  = g_hash_table_new (g_str_hash, g_str_equal);
	
	current_lang = gnome_vfs_i18n_get_language_list ("LC_MESSAGES");

	/*
	 * Setup the descriptors for the information loading
	 */

	gnome_registry_dir.dirname = g_strconcat (GNOME_VFS_DATADIR, "/application-registry", NULL);
	gnome_registry_dir.system_dir = TRUE;
	
	user_registry_dir.dirname = g_strconcat (g_get_home_dir(), "/.gnome/application-info", NULL);
	user_registry_dir.system_dir = FALSE;

	/* Make sure user directory exists */
	if (mkdir (user_registry_dir.dirname, 0700) &&
	    errno != EEXIST) {
		g_warning("Could not create per-user Gnome application-registry directory: %s",
			  user_registry_dir.dirname);
	}

	/*
	 * Load
	 */
	load_application_info ();

	last_checked = time (NULL);

	gnome_vfs_application_registry_initialized = TRUE;
}

static void
maybe_reload (void)
{
	time_t now = time (NULL);
	gboolean need_reload = FALSE;
	struct stat s;

	gnome_vfs_application_registry_init ();
	
	if (last_checked + 5 >= now)
		return;

	if (stat (gnome_registry_dir.dirname, &s) != -1)
		if (s.st_mtime != gnome_registry_dir.s.st_mtime)
			need_reload = TRUE;

	if (stat (user_registry_dir.dirname, &s) != -1)
		if (s.st_mtime != user_registry_dir.s.st_mtime)
			need_reload = TRUE;

	last_checked = now;
	
	if (need_reload) {
	        gnome_vfs_application_registry_reload ();
	}
}

static gboolean
remove_apps (gpointer key, gpointer value, gpointer user_data)
{
	Application *application = value;

	application_clear_mime_types (application);

	application_unref (application);
	
	return TRUE;
}

static void
gnome_vfs_application_registry_clear (void)
{
	if (applications != NULL)
		g_hash_table_foreach_remove (applications, remove_apps, NULL);
}

void
gnome_vfs_application_registry_shutdown (void)
{
	gnome_vfs_application_registry_clear ();

	if (applications != NULL) {
		g_hash_table_destroy (applications);
		applications = NULL;
	}

	if(generic_mime_types != NULL) {
		g_hash_table_destroy (generic_mime_types);
		generic_mime_types = NULL;
	}

	if(specific_mime_types != NULL) {
		g_hash_table_destroy (specific_mime_types);
		specific_mime_types = NULL;
	}

	g_free(gnome_registry_dir.dirname);
	gnome_registry_dir.dirname = NULL;
	g_free(user_registry_dir.dirname);
	user_registry_dir.dirname = NULL;

	g_list_free(current_lang);
	current_lang = NULL;

	gnome_vfs_application_registry_initialized = FALSE;
}

void
gnome_vfs_application_registry_reload (void)
{
	if ( ! gnome_vfs_application_registry_initialized) {
		/* If not initialized, initialization will do a "reload" */
		gnome_vfs_application_registry_init ();
	} else {
		gnome_vfs_application_registry_clear ();
		load_application_info ();
	}
}

/*
 * Existance check
 */
gboolean
gnome_vfs_application_registry_exists (const char *app_id)
{
	g_return_val_if_fail (app_id != NULL, FALSE);

	maybe_reload ();

	if (application_lookup (app_id) != NULL)
		return TRUE;
	else
		return FALSE;
}


/*
 * Getting arbitrary keys
 */

static void
get_keys_foreach(gpointer key, gpointer value, gpointer user_data)
{
	GList **listp = user_data;

	/* make sure we only insert unique keys */
	if ( (*listp) && strcmp ((*listp)->data, key) == 0)
		return;

	(*listp) = g_list_insert_sorted ((*listp), key,
					 (GCompareFunc) strcmp);
}

GList *
gnome_vfs_application_registry_get_keys (const char *app_id)
{
	GList *retval;
	Application *application;

	g_return_val_if_fail (app_id != NULL, NULL);

	maybe_reload ();

	application = application_lookup (app_id);
	if (application == NULL)
		return NULL;

	retval = NULL;

	if (application->keys != NULL)
		g_hash_table_foreach (application->keys, get_keys_foreach,
				      &retval);

	if (application->user_application != NULL &&
	    application->user_application->keys)
		g_hash_table_foreach (application->user_application->keys,
				      get_keys_foreach, &retval);

	return retval;
}

static const char *
real_peek_value (const Application *application, const char *key)
{
	const char *retval;

	g_return_val_if_fail (application != NULL, NULL);
	g_return_val_if_fail (key != NULL, NULL);

	retval = NULL;

	if (application->user_application)
		retval = peek_value (application->user_application, key);

	if (retval == NULL)
		retval = peek_value (application, key);

	return retval;
}

static gboolean
real_get_bool_value (const Application *application, const char *key, gboolean *got_key)
{
	gboolean sub_got_key, retval;

	g_return_val_if_fail (application != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);

	sub_got_key = FALSE;
	retval = FALSE;
	if (application->user_application)
		retval = get_bool_value (application->user_application, key,
					 &sub_got_key);

	if ( ! sub_got_key)
		retval = get_bool_value (application, key, &sub_got_key);

	if (got_key != NULL)
		*got_key = sub_got_key;

	return retval;
}

const char *
gnome_vfs_application_registry_peek_value (const char *app_id, const char *key)
{
	Application *application;

	g_return_val_if_fail (app_id != NULL, NULL);
	g_return_val_if_fail (key != NULL, NULL);

	maybe_reload ();

	application = application_lookup (app_id);
	if (application == NULL)
		return NULL;

	return real_peek_value (application, key);
}

gboolean
gnome_vfs_application_registry_get_bool_value (const char *app_id, const char *key,
					       gboolean *got_key)
{
	Application *application;

	g_return_val_if_fail (app_id != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);

	maybe_reload ();

	application = application_lookup (app_id);
	if (application == NULL)
		return FALSE;

	return real_get_bool_value (application, key, got_key);
}

/*
 * Setting stuff
 */

void
gnome_vfs_application_registry_remove_application (const char *app_id)
{
	Application *application;

	g_return_if_fail (app_id != NULL);

	maybe_reload ();

	application = application_lookup (app_id);
	if (application == NULL)
		return;

	/* Only remove the user_owned stuff */
	if (application->user_owned) {
		application_remove (application);
		user_file_dirty = TRUE;
	} else if (application->user_application != NULL) {
		application_remove (application->user_application);
		user_file_dirty = TRUE;
	}
}

void
gnome_vfs_application_registry_set_value (const char *app_id,
					  const char *key,
					  const char *value)
{
	Application *application;

	g_return_if_fail (app_id != NULL);
	g_return_if_fail (key != NULL);
	g_return_if_fail (value != NULL);

	maybe_reload ();

	application = application_lookup_or_create (app_id, TRUE/*user_owned*/);

	set_value (application, key, value);

	user_file_dirty = TRUE;
}

void
gnome_vfs_application_registry_set_bool_value (const char *app_id,
					       const char *key,
					       gboolean value)
{
	Application *application;

	g_return_if_fail (app_id != NULL);
	g_return_if_fail (key != NULL);

	maybe_reload ();

	application = application_lookup_or_create (app_id, TRUE/*user_owned*/);

	set_bool_value (application, key, value);

	user_file_dirty = TRUE;
}

void
gnome_vfs_application_registry_unset_key (const char *app_id,
					  const char *key)
{
	Application *application;

	g_return_if_fail (app_id != NULL);
	g_return_if_fail (key != NULL);

	maybe_reload ();

	application = application_lookup_or_create (app_id, TRUE/*user_owned*/);

	unset_key (application, key);

	user_file_dirty = TRUE;
}

/*
 * Query functions
 */

GList *
gnome_vfs_application_registry_get_applications (const char *mime_type)
{
	GList *app_list, *app_list2, *retval, *li;

	g_return_val_if_fail (mime_type != NULL, NULL);

	maybe_reload ();

	if (strstr (mime_type, "/*") != NULL) {
		app_list = g_hash_table_lookup (generic_mime_types, mime_type);
		app_list2 = NULL;
	} else {
		char buf[256] = "";

		app_list = g_hash_table_lookup (specific_mime_types, mime_type);

		/* yes this is safe, the 253 is 255 - strlen("/<star>") */
		sscanf (mime_type, "%253s/", buf);
		strcat (buf, "/*");

		app_list2 = g_hash_table_lookup (generic_mime_types, mime_type);
	}

	retval = NULL;
	for (li = app_list; li != NULL; li = li->next) {
		Application *application = li->data;
		/* Note that this list is sorted so to kill duplicates
		 * in app_list we only need to check the first entry */
		if (retval == NULL ||
		    strcmp (retval->data, application->app_id) != 0)
			retval = g_list_prepend (retval, application->app_id);
	}

	for (li = app_list2; li != NULL; li = li->next) {
		Application *application = li->data;
		if (g_list_find_custom (retval, application->app_id,
					(GCompareFunc) strcmp) == NULL)
			retval = g_list_prepend (retval, application->app_id);
	}

	return retval;
}

GList *
gnome_vfs_application_registry_get_mime_types (const char *app_id)
{
	Application *application;
	GList *retval;

	g_return_val_if_fail (app_id != NULL, NULL);

	maybe_reload ();

	application = application_lookup (app_id);
	if (application == NULL)
		return NULL;

	retval = g_list_copy (application->mime_types);

	/* merge in the mime types from the user_application,
	 * if it exists */
	if (application->user_application) {
		GList *li;
		for (li = application->user_application->mime_types;
		     li != NULL;
		     li = li->next) {
			Application *application = li->data;
			if (g_list_find_custom (retval, application->app_id,
						(GCompareFunc) strcmp) == NULL)
				retval = g_list_prepend (retval,
							 application->app_id);
		}
	}

	return retval;
}

gboolean
gnome_vfs_application_registry_supports_mime_type (const char *app_id,
						   const char *mime_type)
{
	Application *application;

	g_return_val_if_fail (app_id != NULL, FALSE);
	g_return_val_if_fail (mime_type != NULL, FALSE);

	maybe_reload ();

	application = application_lookup (app_id);
	if (application == NULL)
		return FALSE;

	/* check both the application and the user application
	 * mime_types lists */
	if ((g_list_find_custom (application->mime_types,
				 /*glib is const incorrect*/(gpointer)mime_type,
				(GCompareFunc) strcmp) != NULL) ||
	    (application->user_application &&
	     g_list_find_custom (application->user_application->mime_types,
				 /*glib is const incorrect*/(gpointer)mime_type,
				 (GCompareFunc) strcmp) != NULL))
		return TRUE;
	else
		return FALSE;
}

/*
 * Mime type functions
 * Note that mime_type can be a specific (image/png) or generic (image/<star>) type
 */

void
gnome_vfs_application_registry_clear_mime_types (const char *app_id)
{
	Application *application;

	g_return_if_fail (app_id != NULL);

	maybe_reload ();

	application = application_lookup_or_create (app_id, TRUE/*user_owned*/);

	application_clear_mime_types (application);

	user_file_dirty = TRUE;
}

void
gnome_vfs_application_registry_add_mime_type (const char *app_id,
					      const char *mime_type)
{
	Application *application;

	g_return_if_fail (app_id != NULL);

	maybe_reload ();

	application = application_lookup_or_create (app_id, TRUE/*user_owned*/);

	add_mime_type (application, mime_type);

	user_file_dirty = TRUE;
}

void
gnome_vfs_application_registry_remove_mime_type (const char *app_id,
						 const char *mime_type)
{
	Application *application;

	g_return_if_fail (app_id != NULL);

	maybe_reload ();

	application = application_lookup_or_create (app_id, TRUE/*user_owned*/);

	remove_mime_type (application, mime_type);

	user_file_dirty = TRUE;
}

/*
 * Syncing to disk
 */

static void
application_sync_foreach (gpointer key, gpointer value, gpointer user_data)
{
	Application *application = value;
	FILE *fp = user_data;

	/* Only sync things that are user owned */
	if (application->user_owned)
		application_sync (application, fp);
	else if (application->user_application)
		application_sync (application->user_application, fp);
}

GnomeVFSResult
gnome_vfs_application_registry_sync (void)
{
	FILE *fp;
	char *file;
	time_t curtime;

	if ( ! user_file_dirty)
		return GNOME_VFS_OK;

	maybe_reload ();

	file = g_strconcat (user_registry_dir.dirname, "/user.applications", NULL);
	fp = fopen (file, "w");

	if ( ! fp) {
		g_warning ("Cannot open '%s' for writing", file);
		g_free (file);
		return gnome_vfs_result_from_errno ();
	}

	g_free (file);

	time(&curtime);

	fprintf (fp, "# This file is automatically generated by gnome-vfs "
		 "application registry\n"
		 "# Do NOT edit by hand\n# Generated: %s\n",
		 ctime (&curtime));

	if (applications != NULL)
		g_hash_table_foreach (applications, application_sync_foreach, fp);

	fclose (fp);

	user_file_dirty = FALSE;

	return GNOME_VFS_OK;
}

GnomeVFSMimeApplication *
gnome_vfs_application_registry_get_mime_application (const char *app_id)
{
	Application *i_application;
	GnomeVFSMimeApplication *application;

	g_return_val_if_fail (app_id != NULL, NULL);

	maybe_reload ();

	i_application = application_lookup (app_id);

	if (i_application == NULL)
		return NULL;

	application = g_new0 (GnomeVFSMimeApplication, 1);

	application->id = g_strdup(app_id);

	application->name =
		g_strdup (real_peek_value
			  (i_application,
			   GNOME_VFS_APPLICATION_REGISTRY_NAME));
	application->command =
		g_strdup (real_peek_value
			  (i_application,
			   GNOME_VFS_APPLICATION_REGISTRY_COMMAND));

	application->can_open_multiple_files =
		real_get_bool_value
			(i_application,
			 GNOME_VFS_APPLICATION_REGISTRY_CAN_OPEN_MULTIPLE_FILES,
			 NULL);
	application->can_open_uris =
		real_get_bool_value
			(i_application,
			 GNOME_VFS_APPLICATION_REGISTRY_CAN_OPEN_URIS,
			 NULL);
	application->requires_terminal =
		real_get_bool_value
			(i_application,
			 GNOME_VFS_APPLICATION_REGISTRY_REQUIRES_TERMINAL,
			 NULL);

	return application;
}

void
gnome_vfs_application_registry_save_mime_application (const GnomeVFSMimeApplication *application)
{
	Application *i_application;

	g_return_if_fail (application != NULL);

	/* make us a new user application */
	i_application = application_lookup_or_create (application->id, TRUE);

	application_ref (i_application);

	set_value (i_application, GNOME_VFS_APPLICATION_REGISTRY_NAME,
		   application->name);
	set_value (i_application, GNOME_VFS_APPLICATION_REGISTRY_COMMAND,
		   application->command);
	set_bool_value (i_application, GNOME_VFS_APPLICATION_REGISTRY_CAN_OPEN_MULTIPLE_FILES,
			application->can_open_multiple_files);
	set_bool_value (i_application, GNOME_VFS_APPLICATION_REGISTRY_CAN_OPEN_URIS,
			application->can_open_uris);
	set_bool_value (i_application, GNOME_VFS_APPLICATION_REGISTRY_REQUIRES_TERMINAL,
			application->requires_terminal);

	user_file_dirty = TRUE;
}
