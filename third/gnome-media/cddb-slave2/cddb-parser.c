/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Authors: Iain Holmes <iain@ximian.com>
 *
 *  Copyright 2002 , Iain Holmes.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include "cddb-slave.h"

gboolean
cddb_entry_parse_file (CDDBEntry *entry,
		       const char *filename)
{
	FILE *handle;
	char line[4096];
	char *prev_vector[2] = {NULL, NULL};
	
	handle = fopen (filename, "r");
	if (handle == NULL) {
		return FALSE;
	}

	while (fgets (line, 4096, handle)) {
		char *end;
		char **vector;
		GString *string;

		if (*line == '#') {
			/* Save comments to rebuild the file later. */
			entry->comments = g_list_append (entry->comments,
							 g_strdup (line));
			continue;
		}
		
		if (*line == 0 || g_ascii_isdigit (*line)) {
			continue;
		}

		if (*line == '.') {
			break;
		}

		/* Strip newlines */
		line[strlen (line) - 1] = 0;
		/* Check for \r */
		end = strchr (line, '\r');
		if (end != NULL) {
			*end = 0;
		}

		vector = g_strsplit (line, "=", 2);
		if (vector == NULL) {
			continue;
		}

                if (*line == '\0'|| vector[1] == NULL) {
			vector[1] = g_strjoin (NULL, "\n", vector[0], NULL);
			g_free (vector[0]);
                        vector[0] = g_strdup (prev_vector[0]);
                }
	
		if (vector [0] != NULL)	{
			if (prev_vector [0] != NULL)
				g_free (prev_vector[0]);
			prev_vector[0] = g_strdup (vector[0]);
		}
		if (vector [1] != NULL)	{
			if (prev_vector [1] != NULL)
				g_free (prev_vector[1]);
			prev_vector[1] = g_strdup (vector[1]);
		}


		/* See if we have this ID */
		string = g_hash_table_lookup (entry->fields, vector[0]);
		if (string == NULL) {
			string = g_string_new (vector[1]);
			g_hash_table_insert (entry->fields, g_strdup (vector[0]), string);
		} else {
			g_string_append (string, vector[1]);
		}

		g_free (vector[0]);
		g_free (vector[1]);
	}

	g_free (prev_vector[0]);
	g_free (prev_vector[1]);
	return TRUE;
}

#define XMCD_INDICATOR "xmcd"
#define XMCD_INDICATOR_LEN 4

#define TRACK_OFFSET_INDICATOR "Track frame offsets:"
#define TRACK_OFFSET_INDICATOR_LEN 20

#define DISC_LENGTH_INDICATOR "Disc length:"
#define DISC_LENGTH_INDICATOR_LEN 12

#define REVISION_INDICATOR "Revision:"
#define REVISION_INDICATOR_LEN 9

static void
parse_comments (CDDBEntry *entry)
{
	GList *comments;
	gboolean parsing_offsets = FALSE;
	int trackno = 0;
	
	for (comments = entry->comments; comments; comments = comments->next) {
		char *line = comments->data;
		
		line++; /* Move past the # */
		while (g_ascii_isspace (*line)) {
			line++; /* Move past the white space */
		}

		if (parsing_offsets == TRUE) {
			g_print ("Found (%d) %s\n", trackno, line);
			entry->offsets[trackno] = atoi (line);
			trackno++;

			if (trackno >= entry->ntrks) {
				parsing_offsets = FALSE;
			}
		}
			
		if (strncmp (XMCD_INDICATOR, line, XMCD_INDICATOR_LEN) == 0) {
			g_print ("Is xmcd file\n");
			parsing_offsets = FALSE;
			continue;
		}
		
		if (strncmp (TRACK_OFFSET_INDICATOR, line, TRACK_OFFSET_INDICATOR_LEN) == 0) {
			g_print ("Found track offsets\n");
			parsing_offsets = TRUE;
			trackno = 0;
			continue;
		}

		if (strncmp (DISC_LENGTH_INDICATOR, line, DISC_LENGTH_INDICATOR_LEN) == 0) {
			parsing_offsets = FALSE;
			line += (DISC_LENGTH_INDICATOR_LEN + 1); /* Past white space */
			entry->disc_length = atoi (line);
			g_print ("Found disc length: %d\n", entry->disc_length);
			continue;
		}

		if (strncmp (REVISION_INDICATOR, line, REVISION_INDICATOR_LEN) == 0) {
			parsing_offsets = FALSE;
			line += (REVISION_INDICATOR_LEN + 1); /* Past white space */
			entry->revision = atoi (line);
			g_print ("Found revision: %d\n", entry->revision);
			continue;
		}
	}
}

#define CD_FRAMES 75

static void
calculate_lengths (CDDBEntry *entry)
{
	int i;
	
	for (i = 0; i < entry->ntrks; i++) {
		int start, finish;
		int length;

		start = entry->offsets[i];
		if (i == entry->ntrks - 1) {
			finish = entry->disc_length * CD_FRAMES;
		} else {
			finish = entry->offsets[i + 1];
		}

		length = finish - start;

		entry->lengths[i] = (length / CD_FRAMES) - 1;
	}
}

CDDBEntry *
cddb_entry_new (const char *discid,
		int ntrks,
		const char *offsets,
		int nsecs)
{
	CDDBEntry *entry;
	char **offset_vector;
	int i;

	entry = g_new0 (CDDBEntry, 1);

	entry->discid = g_strdup (discid);
	entry->realdiscid = g_strdup (discid);
	entry->ntrks = ntrks;
  	entry->offsets = g_new (int, entry->ntrks);
	entry->lengths = g_new (int, entry->ntrks);
	offset_vector = g_strsplit (offsets, " ", entry->ntrks);
	for (i = 0; i < entry->ntrks; i++) {
		entry->offsets[i] = atoi (offset_vector[i]);
	}
	g_strfreev (offset_vector);
	
	entry->disc_length = nsecs;

	calculate_lengths (entry);
	entry->fields = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	return entry;
}

static void
is_track (gpointer key,
	  gpointer value,
	  gpointer data)
{
	int *t = (int *) data;

	if (strncasecmp (key, "TTITLE", 6) == 0) {
		(*t)++;
	}
}

static int
count_tracks (CDDBEntry *entry)
{
	int ntrks = 0;

	g_hash_table_foreach (entry->fields, is_track, &ntrks);
	return ntrks;
}

CDDBEntry *
cddb_entry_new_from_file (const char *filename)
{
	CDDBEntry *entry;
	GString *did;

	entry = g_new0 (CDDBEntry, 1);

	entry->fields = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	if (cddb_entry_parse_file (entry, filename) == FALSE) {
		g_hash_table_destroy (entry->fields);
		g_free (entry);
		return FALSE;
	}

	/* The data in the file may be for a different id than the file */
	entry->realdiscid = g_path_get_basename (filename);
	
	did = g_hash_table_lookup (entry->fields, "DISCID");
	if (did == NULL) {
		entry->discid = g_path_get_basename (filename);
	} else {
		entry->discid = g_strndup (did->str, 8);
		g_print ("**** Entry->discid = \"%s\"\n", entry->discid);
	}
	
	entry->ntrks = count_tracks (entry);
	entry->offsets = g_new (int, entry->ntrks);
	entry->lengths = g_new (int, entry->ntrks);
	
	parse_comments (entry);

	calculate_lengths (entry);
	
	return entry;
}

/* Handle splitting the line into 80 char lines
   The spec says <= 256 is the right length,
   but everyone seems to split on 80 so that it looks good
   in a terminal */
static void
write_line (FILE *handle,
	    const char *key,
	    const char *value)
{
	int key_len, val_len, line_len;
	char *tv = (char *) value;
	char *str;

	if (key == NULL) {
		str = g_strdup_printf ("%s\r\n", value);
		g_print ("Writing: %s\n", str);
		fputs (str, handle);
	} else {
		key_len = strlen (key) + 1 + 2; /* Length of "KEY=" + \r\n */
		val_len = strlen (value);
		line_len = 80 - key_len;

		while (val_len > line_len) {
			str = g_strdup_printf ("%s=%s\r\n", key, tv);
			g_print ("Writing: %s\n", str);
			fputs (str, handle);
			g_free (str);
			
			tv += line_len;
			val_len -= line_len;
		}
		
		/* Last line */
		str = g_strdup_printf ("%s=%s\r\n", key, tv);
		g_print ("Writing: %s\n", str);
		fputs (str, handle);
		g_free (str);
	}
}

static void
write_offsets (CDDBEntry *entry,
	       FILE *handle)
{
	int i;

	g_print ("Writing offsets\n");
	for (i = 0; i < entry->ntrks; i++) {
		char *str;
		
		str = g_strdup_printf ("#\t%d", entry->offsets[i]);
		write_line (handle, NULL, str);
		g_free (str);
	}
}

static void
write_disc_length (CDDBEntry *entry,
		   FILE *handle)
{
	char *str;

	g_print ("Writing disc length\n");
	str = g_strdup_printf ("# Disc length: %d seconds", entry->disc_length);
	write_line (handle, NULL, str);
	g_free (str);
}

static void
write_revision (CDDBEntry *entry,
		FILE *handle)
{
	char *str;

	g_print ("Writing revision\n");
	str = g_strdup_printf ("# Revision: %d", entry->revision);
	write_line (handle, NULL, str);
	g_free (str);
}

static void
write_version (CDDBEntry *entry,
	       FILE *handle)
{
	char *str;

	g_print ("Writing version\n");
	str = g_strdup_printf ("# Submitted via: CDDBSlave2 %s", VERSION);
	write_line (handle, NULL, str);
	g_free (str);
}

static void
write_headers (CDDBEntry *entry,
	       FILE *handle)
{
	g_print ("Writing headers\n");
	write_line (handle, NULL, "# xmcd");
	write_line (handle, NULL, "# Track frame offsets:");
	
	write_offsets (entry, handle);
	
	write_line (handle, NULL, "#");

	write_disc_length (entry, handle);

	write_revision (entry, handle);
	write_version (entry, handle);

	write_line (handle, NULL, "#");
}

static void
write_field (CDDBEntry *entry,
	     const char *key,
	     FILE *handle)
{
	GString *value;

	value = g_hash_table_lookup (entry->fields, key);
	if (value != NULL) {
		write_line (handle, key, value->str);
	}
}

static void
write_body (CDDBEntry *entry,
	    FILE *handle)
{
	int i;

	g_print ("Writing body\n");
	write_line (handle, "DISCID", entry->realdiscid);

	write_field (entry, "DTITLE", handle);
	write_field (entry, "DGENRE", handle);
	write_field (entry, "DYEAR", handle);
	/* Track titles */
	for (i = 0; i < entry->ntrks; i++) {
		char *key;

		key = g_strdup_printf ("TTITLE%d", i);
		write_field (entry, key, handle);
		g_free (key);
	}
	write_field (entry, "EXTD", handle);
	for (i = 0; i < entry->ntrks; i++) {
		char *key;

		key = g_strdup_printf ("EXTT%d", i);
		write_field (entry, key, handle);
		g_free (key);
	}

	write_field (entry, "PLAYORDER", handle);
}

gboolean
cddb_entry_write_to_file (CDDBEntry *entry)
{
	char *filename;
	FILE *handle;
	
	g_return_val_if_fail (entry != NULL, FALSE);

	filename = g_build_filename (g_get_home_dir (),
				     ".cddbslave",
				     entry->realdiscid, NULL);
	g_print ("Writing file %s\n", filename);
	handle = fopen (filename, "w");
	g_free (filename);
	
	if (handle == NULL) {
		return FALSE;
	}

	/* Update the revision */
	entry->revision++;
	
	write_headers (entry, handle);
	write_body (entry, handle);

	fclose (handle);
	
	return TRUE;
}
