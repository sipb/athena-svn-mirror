/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * Copyright (C) 1998 Miguel de Icaza
 * Copyright (C) 1997 Paolo Molaro
 * Copyright (C) 2000 Eazel
 * All rights reserved.
 *
 * This file is part of the Gnome Library.
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include "gnome-vfs.h"
#include "gnome-vfs-mime.h"
#include "gnome-vfs-mime-info.h"
#include "gnome-vfs-mime-sniff-buffer.h"
#include "gnome-vfs-mime-private.h"
#include "gnome-vfs-module-shared.h"

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <regex.h>
#include <string.h>
#include <ctype.h>

static gboolean module_inited = FALSE;

static GHashTable *mime_extensions [2] = { NULL, NULL };
static GList      *mime_regexs     [2] = { NULL, NULL };

typedef struct {
	char *mime_type;
	regex_t regex;
} RegexMimePair;

typedef struct {
	char *dirname;
	struct stat s;
	unsigned int valid : 1;
	unsigned int system_dir : 1;
} mime_dir_source_t;

/* chached localhostname */
static char localhostname[1024];
static gboolean got_localhostname = FALSE;

/* These ones are used to automatically reload mime-types on demand */
static mime_dir_source_t gnome_mime_dir, user_mime_dir;
static time_t last_checked;

#ifdef G_THREADS_ENABLED

/* We lock this mutex whenever we modify global state in this module.  */
G_LOCK_DEFINE_STATIC (mime_mutex);

#endif /* G_LOCK_DEFINE_STATIC */



static char *
get_priority (char *def, int *priority)
{
	*priority = 0;

	if (*def == ','){
		def++;
		if (*def == '1'){
			*priority = 0;
			def++;
		} else if (*def == '2'){
			*priority = 1;
			def++;
		}
	}

	while (*def && *def == ':')
		def++;

	return def;
}

static gint
list_find_type (gconstpointer value, gconstpointer type)
{
	return (g_strcasecmp( (const gchar*) value, (const gchar*) type ));
}

static void
add_to_key (char *mime_type, char *def)
{
	int priority = 1;
	char *s, *p, *ext;
	GList *list = NULL;

	if (strncmp (def, "ext", 3) == 0){
		char *tokp;

		def += 3;
		def = get_priority (def, &priority);
		s = p = g_strdup (def);

		while ((ext = strtok_r (s, " \t\n\r,", &tokp)) != NULL){
			list = (GList*) g_hash_table_lookup (mime_extensions [priority], ext);
		    if (!g_list_find_custom (list, (gpointer) mime_type, list_find_type)) {
				list = g_list_prepend (list, g_strdup (mime_type));
				g_hash_table_insert (mime_extensions [priority], g_strdup (ext), list);
			}
			s = NULL;
		}
		g_free (p);
	}

	if (strncmp (def, "regex", 5) == 0){
		RegexMimePair *mp;
		def += 5;
		def = get_priority (def, &priority);

		while (*def && isspace ((unsigned char)*def))
			def++;

		if (!*def)
			return;

		/* This was g_new instead of g_new0, but there seems
		 * to be a bug in the Solaris? version of regcomp that
		 * requires an initialized regex or it will crash.
		 */
		mp = g_new0 (RegexMimePair, 1);
		if (regcomp (&mp->regex, def, REG_EXTENDED | REG_NOSUB)){
			g_free (mp);
			return;
		}
		mp->mime_type = g_strdup (mime_type);

		mime_regexs [priority] = g_list_prepend (mime_regexs [priority], mp);
	}
}

static void
mime_fill_from_file (const char *filename)
{
	FILE *f;
	char buf [1024];
	char *current_key;

	g_assert (filename != NULL);

	f = fopen (filename, "r");

	if (!f)
		return;

	current_key = NULL;
	while (fgets (buf, sizeof (buf), f)){
		char *p;

		if (buf [0] == '#')
			continue;

		/* Trim trailing spaces */
		for (p = buf + strlen (buf) - 1; p >= buf; p--){
			if (isspace ((unsigned char)*p) || *p == '\n')
				*p = 0;
			else
				break;
		}

		if (!buf [0])
			continue;

		if (buf [0] == '\t' || buf [0] == ' '){
			if (current_key){
				char *p = buf;

				while (*p && isspace ((unsigned char)*p))
					p++;

				if (*p == 0)
					continue;

				add_to_key (current_key, p);
			}
		} else {
			g_free (current_key);

			current_key = g_strdup (buf);
			if (current_key [strlen (current_key)-1] == ':')
				current_key [strlen (current_key)-1] = 0;
		}
	}

	g_free (current_key);

	fclose (f);
}

static void
mime_load (mime_dir_source_t *source)
{
	DIR *dir;
	struct dirent *dent;
	const int extlen = sizeof (".mime") - 1;
	char *filename;

	g_return_if_fail (source != NULL);
	g_return_if_fail (source->dirname != NULL);

	if (stat (source->dirname, &source->s) != -1)
		source->valid = TRUE;
	else
		source->valid = FALSE;

	dir = opendir (source->dirname);
	if (!dir){
		source->valid = FALSE;
		return;
	}

	if (source->system_dir){
		filename = g_strconcat (source->dirname, "/gnome-vfs.mime", NULL);
		mime_fill_from_file (filename);
		g_free (filename);
	}

	while ((dent = readdir (dir)) != NULL){

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

		mime_fill_from_file (filename);
		g_free (filename);
	}
	closedir (dir);

	if (!source->system_dir) {
		filename = g_strconcat (source->dirname, "/user.mime", NULL);
		mime_fill_from_file (filename);
		g_free (filename);
	}
}

static gboolean
remove_one_mime_hash_entry (gpointer key, gpointer value, gpointer user_data)
{
	g_free (key);
	g_list_foreach (value, (GFunc) g_free, NULL);
	g_list_free (value);

	return TRUE;
}

static void
mime_extensions_empty (void)
{
	GList *p;
	int i;
	for (i = 0; i < 2; i++) {
		if (mime_extensions [i] != NULL) {
			g_hash_table_foreach_remove (mime_extensions [i], 
						     remove_one_mime_hash_entry, NULL);
		}

		for (p = mime_regexs [i]; p != NULL; p = p->next){
			RegexMimePair *mp = p->data;

			g_free (mp->mime_type);
			regfree (&mp->regex);
			g_free (mp);
		}
		g_list_free (mime_regexs [i]);
		mime_regexs [i] = NULL;
	}
}

static void
maybe_reload (void)
{
	time_t now = time (NULL);
	gboolean need_reload = FALSE;
	struct stat s;

	if (last_checked + 5 >= now)
		return;

	if (stat (gnome_mime_dir.dirname, &s) != -1)
		if (s.st_mtime != gnome_mime_dir.s.st_mtime)
			need_reload = TRUE;

	if (stat (user_mime_dir.dirname, &s) != -1)
		if (s.st_mtime != user_mime_dir.s.st_mtime)
			need_reload = TRUE;

	last_checked = now;

	if (!need_reload)
		return;

	mime_extensions_empty ();

	mime_load (&gnome_mime_dir);
	mime_load (&user_mime_dir);
	last_checked = time (NULL);
}

static void
mime_init (void)
{
	mime_extensions [0] = g_hash_table_new (g_str_hash, g_str_equal);
	mime_extensions [1] = g_hash_table_new (g_str_hash, g_str_equal);
	
	gnome_mime_dir.dirname = g_strconcat (GNOME_VFS_DATADIR, "/mime-info", NULL);
	gnome_mime_dir.system_dir = TRUE;

	user_mime_dir.dirname = g_strconcat (g_get_home_dir (), "/.gnome/mime-info", NULL);
	user_mime_dir.system_dir = FALSE;

	mime_load (&gnome_mime_dir);
	mime_load (&user_mime_dir);
	last_checked = time (NULL);

	module_inited = TRUE;
}

void
gnome_vfs_mime_shutdown (void)
{
	if (!module_inited)
		return;

	gnome_vfs_mime_info_shutdown ();
	gnome_vfs_mime_clear_magic_table ();

	mime_extensions_empty ();
	
	g_hash_table_destroy (mime_extensions[0]);
	g_hash_table_destroy (mime_extensions[1]);
	
	g_free (gnome_mime_dir.dirname);
	g_free (user_mime_dir.dirname);
}

/**
 * gnome_vfs_mime_type_from_name_or_default:
 * @filename: A filename (the file does not necesarily exist).
 * @defaultv: A default value to be returned if no match is found
 *
 * This routine tries to determine the mime-type of the filename
 * only by looking at the filename from the GNOME database of mime-types.
 *
 * Returns the mime-type of the @filename.  If no value could be
 * determined, it will return @defaultv.
 */
const char *
gnome_vfs_mime_type_from_name_or_default (const gchar *filename, const gchar *defaultv)
{
	const gchar *ext;
	gchar *upext;
	int priority;
	const gchar *result = defaultv;

	G_LOCK (mime_mutex);

	if (!filename)
		goto done;
	ext = strrchr (filename, '.');
	if (ext)
		++ext;

	if (!module_inited)
		mime_init ();

	maybe_reload ();

	for (priority = 1; priority >= 0; priority--){
		GList *l;
		GList *list = NULL ;
		
		if (ext){
			
			list = g_hash_table_lookup (mime_extensions [priority], ext);
			if (list) {
				list = g_list_first( list );
				result = (gchar *) list->data;
				goto done;
			}

			/* Search for UPPER case extension */
			upext = g_strdup (ext);
			g_strup (upext);
			list = g_hash_table_lookup (mime_extensions [priority], upext);
			if (list) {
				g_free (upext);
				list = g_list_first (list);
				result = (gchar *) list->data;
				goto done;
			}

			/* Final check for lower case */
			g_strdown (upext);
			list = g_hash_table_lookup (mime_extensions [priority], upext);
 			g_free (upext);
			if (list) {
				list = g_list_first (list);
				result = (gchar *) list->data;
				goto done;
			}
		}

		for (l = mime_regexs [priority]; l; l = l->next){
			RegexMimePair *mp = l->data;

			if (regexec (&mp->regex, filename, 0, 0, 0) == 0) {
				result = mp->mime_type;
				goto done;
			}
		}
	}

 done:
	G_UNLOCK (mime_mutex);
	return result;
}

/**
 * gnome_vfs_mime_type_from_name:
 * @filename: A filename (the file does not necessarily exist).
 *
 * Determined the mime type for @filename.
 *
 * Returns the mime-type for this filename.
 */
const char *
gnome_vfs_mime_type_from_name (const gchar * filename)
{
	return gnome_vfs_mime_type_from_name_or_default (filename, "application/octet-stream");
}

static const char *
gnome_vfs_get_mime_type_from_uri_internal (GnomeVFSURI *uri)
{
	const char *base_name;

	/* Return a mime type based on the file extension or NULL if no match. */
	base_name = gnome_vfs_uri_get_basename (uri);
	if (base_name == NULL)
		return NULL;

	return gnome_vfs_mime_type_from_name_or_default (base_name, NULL);
}

/**
 * gnome_vfs_get_mime_type:
 * @uri: a real file or a non-existent uri.
 * @data_size: Size of the data.
 *
 * Tries to guess the mime type of the file represented by @uir.
 * Favors using the file data to the @uri extension.
 * Handles passing @uri of a non-existent file by falling back
 * on returning a type based on the extension.
 *
 * Returns the mime-type for this uri.
 */
const char *
gnome_vfs_get_mime_type (GnomeVFSURI *uri)
{
	const char *result;
	GnomeVFSMimeSniffBuffer *buffer;
	GnomeVFSHandle *handle;
	GnomeVFSResult error;

	/* Check for special stat-defined file types first. */
	result = gnome_vfs_get_special_mime_type (uri);
	if (result != NULL) {
		return result;
	}

	error = gnome_vfs_open_uri (&handle, uri, GNOME_VFS_OPEN_READ);

	if (error != GNOME_VFS_OK) {
		/* file may not exist, return type based on name only */
		return gnome_vfs_get_mime_type_from_uri_internal (uri);
	}
	
	buffer = gnome_vfs_mime_sniff_buffer_new_from_handle (handle);

	/* check the type from the file data */
	result = gnome_vfs_get_mime_type_for_buffer (buffer);

	if (result == NULL) {
		if (gnome_vfs_sniff_buffer_looks_like_text (buffer)) {
			/* Text file -- treat extensions as a more accurate source
			 * of type information.
			 */
			result = gnome_vfs_get_mime_type_from_uri_internal (uri);
			if (result == NULL) {
				/* Didn't find an extension match, assume plain text. */
				result = "text/plain";
			}
		} else {
			/* No type recognized -- fall back on extensions. */
			result = gnome_vfs_get_mime_type_from_uri_internal (uri);
		}
	}

	if (result == NULL) {
		/* no type detected, return a generic file type */
		result = "application/octet-stream";
	}
	
	gnome_vfs_mime_sniff_buffer_free (buffer);
	gnome_vfs_close (handle);

	return result;
}

static GnomeVFSResult
file_seek_binder (gpointer context, GnomeVFSSeekPosition whence, 
	GnomeVFSFileOffset offset)
{
	FILE *file = (FILE *)context;
	int result;
	result = fseek (file, offset, whence);
	if (result < 0) {
		return gnome_vfs_result_from_errno ();
	}
	return GNOME_VFS_OK;
}

static GnomeVFSResult
file_read_binder (gpointer context, gpointer buffer, 
	GnomeVFSFileSize bytes, GnomeVFSFileSize *bytes_read)
{
	FILE *file = (FILE *)context;	
	*bytes_read = fread (buffer, 1, bytes, file);
	if (*bytes_read < 0) {
		*bytes_read = 0;
		return gnome_vfs_result_from_errno ();
	}

	return GNOME_VFS_OK;
}


/**
 * gnome_vfs_get_file_mime_type:
 * @path: a path of a file.
 * @stat_info: optional stat buffer.
 * @suffix_only: whether or not to do a magic-based lookup.
 *
 * Tries to guess the mime type of the file represented by @path.
 * If @suffix_only is false, uses the mime-magic based lookup first.
 * Handles passing @path of a non-existent file by falling back
 * on returning a type based on the extension.
 *
 * Returns the mime-type for this path.
 */
const char *
gnome_vfs_get_file_mime_type (const char *path, const struct stat *stat_info,
	gboolean suffix_only)
{
	const char *result;
	GnomeVFSMimeSniffBuffer *buffer;
	struct stat tmp_stat_buffer;
	FILE *file;

	buffer = NULL;
	result = NULL;

	/* get the stat info if needed */
	if (stat_info == NULL && stat (path, &tmp_stat_buffer) == 0) {
		stat_info = &tmp_stat_buffer;
	}

	/* single out special file types */
	if (stat_info && !S_ISREG(stat_info->st_mode)) {
		if (S_ISDIR(stat_info->st_mode))
			return "x-directory/normal";
		else if (S_ISCHR(stat_info->st_mode))
			return "x-special/device-char";
		else if (S_ISBLK(stat_info->st_mode))
			return "x-special/device-block";
		else if (S_ISFIFO(stat_info->st_mode))
			return "x-special/fifo";
		else if (S_ISSOCK(stat_info->st_mode))
			return "x-special/socket";
		else
			/* unknown entry type, return generic file type */
			return "application/octet-stream";
	}

	if (!suffix_only) {
		file = fopen(path, "r");
		if (file != NULL) {
			buffer = gnome_vfs_mime_sniff_buffer_new_generic
				(file_seek_binder, file_read_binder, file);

			result = gnome_vfs_get_mime_type_for_buffer (buffer);
			if (result == NULL && gnome_vfs_sniff_buffer_looks_like_text (buffer)) {
				/* Text file -- treat extensions as a more accurate source
				 * of type information.
				 */
				result = gnome_vfs_mime_type_from_name_or_default (path, NULL);
				if (result == NULL) {
					/* Didn't find an extension match, assume plain text. */
					result = "text/plain";
				}
			}
			fclose (file);
		}
	}

	if (result == NULL) {
		result = gnome_vfs_mime_type_from_name_or_default (path, NULL);	
	}
	if (result == NULL) {
		result = "application/octet-stream";
	}
	if (buffer) {
		gnome_vfs_mime_sniff_buffer_free (buffer);
	}

	return result;
}

/**
 * gnome_vfs_get_mime_type_from_uri:
 * @uri: A file uri.
 *
 * Tries to guess the mime type of the file @uri by
 * checking the file name extension. Works on non-existent
 * files.
 *
 * Returns the mime-type for this filename.
 */
const char *
gnome_vfs_get_mime_type_from_uri (GnomeVFSURI *uri)
{
	const char *result;

	result = gnome_vfs_get_mime_type_from_uri_internal (uri);
	if (result == NULL) {
		/* no type, return generic file type */
		result = "application/octet-stream";
	}

	return result;
}

/**
 * gnome_vfs_get_mime_type_from_file_data:
 * @uri: A file uri.
 *
 * Tries to guess the mime type of the file @uri by
 * checking the file data using the magic patterns. Does not handle text files properly
 *
 * Returns the mime-type for this filename.
 */
const char *
gnome_vfs_get_mime_type_from_file_data (GnomeVFSURI *uri)
{
	const char *result;
	GnomeVFSMimeSniffBuffer *buffer;
	GnomeVFSHandle *handle;
	GnomeVFSResult error;

	error = gnome_vfs_open_uri (&handle, uri, GNOME_VFS_OPEN_READ);

	if (error != GNOME_VFS_OK) {
		return NULL;
	}
	
	buffer = gnome_vfs_mime_sniff_buffer_new_from_handle (handle);

	/* check the type from the file data */
	result = gnome_vfs_get_mime_type_for_buffer (buffer);

	if (result == NULL && gnome_vfs_sniff_buffer_looks_like_text (buffer)) {
		result = "text/plain";
	}

	if (result == NULL) {
		/* no type detected, return a generic file type */
		result = "application/octet-stream";
	}
	
	gnome_vfs_mime_sniff_buffer_free (buffer);
	gnome_vfs_close (handle);

	return result;
}

/**
 * gnome_vfs_get_mime_type_for_data:
 * @data: A pointer to data in memory.
 * @data_size: Size of the data.
 *
 * Tries to guess the mime type of the data in @data
 * using the magic patterns.
 *
 * Returns the mime-type for this filename.
 */
const char *
gnome_vfs_get_mime_type_for_data (gconstpointer data, int data_size)
{
	const char *result;
	GnomeVFSMimeSniffBuffer *buffer;

	buffer = gnome_vfs_mime_sniff_buffer_new_from_existing_data
		(data, data_size);

	result = gnome_vfs_get_mime_type_for_buffer (buffer);
	gnome_vfs_mime_sniff_buffer_free (buffer);

	return result;
}

/**
 * gnome_uri_list_extract_uris:
 * @uri_list: an uri-list in the standard format.
 *
 * Extract URIs from a @uri-list and return a list
 *
 * Returns: a GList containing strings allocated with g_malloc.
 * You should use #gnome_uri_list_free_strings to free the
 * returned list
 */
GList*
gnome_uri_list_extract_uris (const gchar* uri_list)
{
	const guchar *p, *q;
	gchar *retval;
	GList *result = NULL;

	g_return_val_if_fail (uri_list != NULL, NULL);

	p = (const guchar *)uri_list;

	/* We don't actually try to validate the URI according to RFC
	 * 2396, or even check for allowed characters - we just ignore
	 * comments and trim whitespace off the ends.  We also
	 * allow LF delimination as well as the specified CRLF.
	 */
	while (p) {
		if (*p != '#') {
			while (isspace(*p))
				p++;

			q = p;
			while (*q && (*q != '\n') && (*q != '\r'))
				q++;

			if (q > p) {
			        q--;
				while (q > p && isspace(*q))
					q--;

				retval = g_malloc (q - p + 2);
				strncpy (retval, p, q - p + 1);
				retval[q - p + 1] = '\0';

				result = g_list_prepend (result, retval);
			}
		}
		p = strchr (p, '\n');
		if (p)
			p++;
	}

	return g_list_reverse (result);
}

/**
 * gnome_uri_list_extract_filenames:
 * @uri_list: an uri-list in the standard format
 *
 * Extract local files from a @uri-list and return a list.  Note
 * that unlike the #gnome_uri_list_extract_uris function, this
 * will only return local files and not any other urls.
 *
 * Returns: a GList containing strings allocated with g_malloc.
 * You should use #gnome_uri_list_free_strings to free the
 * returned list.
 */
GList*
gnome_uri_list_extract_filenames (const gchar* uri_list)
{
	GList *tmp_list, *node, *result;

	g_return_val_if_fail (uri_list != NULL, NULL);

	result = gnome_uri_list_extract_uris (uri_list);

	tmp_list = result;
	while (tmp_list) {
		gchar *s = tmp_list->data;

		node = tmp_list;
		tmp_list = tmp_list->next;

		node->data = gnome_uri_extract_filename(s);

		/* if we didn't get anything, just remove the element */
		if (!node->data) {
			result = g_list_remove_link(result, node);
			g_list_free_1 (node);
		}
		g_free (s);
	}
	return result;
}

/**
 * gnome_uri_extract_filename:
 * @uri: a single URI
 *
 * If the @uri is a local file, return the local filename only
 * without the file:[//hostname] prefix.
 *
 * Returns: a newly allocated string if the @uri, or %NULL
 * if the @uri was not a local file
 */
gchar *
gnome_uri_extract_filename (const gchar* uri)
{
	/* file uri with a hostname */
	if (strncmp(uri, "file://", strlen("file://"))==0) {
		char *hostname = g_strdup(&uri[strlen("file://")]);
		char *p = strchr(hostname,'/');
		char *path;
		/* if we can't find the '/' this uri is bad */
		if(!p) {
			g_free(hostname);
			return NULL;
		}
		/* if no hostname */
		if(p==hostname)
			return hostname;

		path = g_strdup(p);
		*p = '\0';

		/* gel local host name and cache it */
		if(!got_localhostname) {
			G_LOCK (mime_mutex);
			if(gethostname(localhostname,
				       sizeof(localhostname)) < 0) {
				strcpy(localhostname,"");
			}
			got_localhostname = TRUE;
			G_UNLOCK (mime_mutex);
		}

		/* if really local */
		if((localhostname[0] &&
		    g_strcasecmp(hostname,localhostname)==0) ||
		   g_strcasecmp(hostname,"localhost")==0) {
			g_free(hostname);
			return path;
		}
		
		g_free(hostname);
		g_free(path);
		return NULL;

	/* if the file doesn't have the //, we take it containing 
	   a local path */
	} else if (strncmp(uri, "file:", strlen("file:"))==0) {
		const char *path = &uri[strlen("file:")];
		/* if empty bad */
		if(!*path) return NULL;
		return g_strdup(path);
	}
	return NULL;
}

/**
 * gnome_uri_list_free_strings:
 * @list: A GList returned by gnome_uri_list_extract_uris() or gnome_uri_list_extract_filenames()
 *
 * Releases all of the resources allocated by @list.
 */
void
gnome_uri_list_free_strings (GList *list)
{
	g_list_foreach (list, (GFunc) g_free, NULL);
	g_list_free (list);
}
