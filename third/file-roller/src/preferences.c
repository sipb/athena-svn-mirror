/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  File-Roller
 *
 *  Copyright (C) 2001 The Free Software Foundation, Inc.
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
 */

#include <string.h>
#include <libgnome/libgnome.h>
#include <gconf/gconf-client.h>
#include "typedefs.h"
#include "preferences.h"
#include "main.h"
#include "file-utils.h"
#include "gconf-utils.h"
#include "window.h"


typedef struct {
	int   i_value;
	char *s_value;
} EnumStringTable;


static int
get_enum_from_string (EnumStringTable *table,
		      const char      *s_value)
{
	int i;

	/* return the first value if s_value is invalid */
	
	if (s_value == NULL)
		return table[0].i_value; 

	for (i = 0; table[i].s_value != NULL; i++)
		if (strcmp (s_value, table[i].s_value) == 0)
			return table[i].i_value;
	
	return table[0].i_value;
}


static char *
get_string_from_enum (EnumStringTable *table,
		      int              i_value)
{
	int i;

	for (i = 0; table[i].s_value != NULL; i++)
		if (i_value == table[i].i_value)
			return table[i].s_value;
	
	/* return the first value if i_value is invalid */

	return table[0].s_value;
}


/* --------------- */


static EnumStringTable sort_method_table [] = {
	{ WINDOW_SORT_BY_NAME, "name" },
	{ WINDOW_SORT_BY_SIZE, "size" },
        { WINDOW_SORT_BY_TYPE, "type" },
        { WINDOW_SORT_BY_TIME, "time" },
        { WINDOW_SORT_BY_PATH, "path" },
	{ 0, NULL }
};

static EnumStringTable sort_type_table [] = {
	{ GTK_SORT_ASCENDING, "ascending" },
	{ GTK_SORT_DESCENDING, "descending" },
	{ 0, NULL }
};

static EnumStringTable list_mode_table [] = {
	{ WINDOW_LIST_MODE_FLAT, "all_files" },
	{ WINDOW_LIST_MODE_AS_DIR, "as_folder" },
	{ 0, NULL }
};

static EnumStringTable compression_level_table [] = {
	{ FR_COMPRESSION_VERY_FAST, "very_fast" },
	{ FR_COMPRESSION_FAST, "fast" },
	{ FR_COMPRESSION_NORMAL, "normal" },
	{ FR_COMPRESSION_MAXIMUM, "maximum" },
	{ 0, NULL }
};


/* --------------- */


WindowSortMethod
preferences_get_sort_method ()
{
	char *s_value;
	int   i_value;

	s_value = eel_gconf_get_string (PREF_LIST_SORT_METHOD);
	i_value = get_enum_from_string (sort_method_table, s_value);
	g_free (s_value);

	return (WindowSortMethod) i_value;
}


void
preferences_set_sort_method (WindowSortMethod i_value)
{
	char *s_value;

	s_value = get_string_from_enum (sort_method_table, i_value);
	eel_gconf_set_string (PREF_LIST_SORT_METHOD, s_value);
}


GtkSortType
preferences_get_sort_type ()
{
	char *s_value;
	int   i_value;

	s_value = eel_gconf_get_string (PREF_LIST_SORT_TYPE);
	i_value = get_enum_from_string (sort_type_table, s_value);
	g_free (s_value);

	return (GtkSortType) i_value;
}


void
preferences_set_sort_type (GtkSortType i_value)
{
	char *s_value;

	s_value = get_string_from_enum (sort_type_table, i_value);
	eel_gconf_set_string (PREF_LIST_SORT_TYPE, s_value);
}


WindowListMode
preferences_get_list_mode ()
{
	char *s_value;
	int   i_value;

	s_value = eel_gconf_get_string (PREF_LIST_MODE);
	i_value = get_enum_from_string (list_mode_table, s_value);
	g_free (s_value);

	return (WindowListMode) i_value;
}


void
preferences_set_list_mode (WindowListMode i_value)
{
	char *s_value;

	s_value = get_string_from_enum (list_mode_table, i_value);
	eel_gconf_set_string (PREF_LIST_MODE, s_value);
}


FRCompression
preferences_get_compression_level ()
{
	char *s_value;
	int   i_value;

	s_value = eel_gconf_get_string (PREF_ADD_COMPRESSION_LEVEL);
	i_value = get_enum_from_string (compression_level_table, s_value);
	g_free (s_value);

	return (FRCompression) i_value;
}


void
preferences_set_compression_level (FRCompression i_value)
{
	char *s_value;

	s_value = get_string_from_enum (compression_level_table, i_value);
	eel_gconf_set_string (PREF_ADD_COMPRESSION_LEVEL, s_value);
}
