/*
 * Copyright (C) 2001, 2002 Anders Carlsson <andersca@gnu.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include "gconf-util.h"

#include <string.h>

/* We are defining this here because is internal, but there is no other way to
 * know if an user can write into default/mandatory gconf values */
GConfEngine *gconf_engine_get_local               (const char  *address,
                                                   GError     **err);


GType
gconf_value_get_type (void)
{
	static GType type = 0;

	if (type == 0) {
		type = g_boxed_type_register_static ("GConfValue",
						     (GBoxedCopyFunc)gconf_value_copy,
						     (GBoxedFreeFunc)gconf_value_free);
	}

	return type;
}

gchar *
gconf_get_key_name_from_path (const gchar *path)
{
	const gchar *ptr;

	/* FIXME:  VALIDATE KEY */
	
	ptr = path + strlen (path);

	while (ptr[-1] != '/')
		ptr--;

	return g_strdup (ptr);
}

gchar *
gconf_value_type_to_string (GConfValueType value_type)
{
	switch (value_type) {
	case GCONF_VALUE_STRING:
		return g_strdup ("String");
		break;
	case GCONF_VALUE_INT:
		return g_strdup ("Integer");
		break;
	case GCONF_VALUE_BOOL:
		return g_strdup ("Boolean");
		break;
	case GCONF_VALUE_LIST:
		return g_strdup ("List");
		break;
	default:
		return g_strdup_printf ("UNKNOWN, %d", value_type);
	}
}

GConfSchema *
gconf_client_get_schema_for_key (GConfClient *client, const char *key)
{
	GConfEngine *engine;
	GConfEntry *entry;
	const char *schema_name;
	GConfSchema *schema;

	engine = gconf_engine_get_for_address (GCONF_DEFAULTS_SOURCE, NULL);
	entry = gconf_engine_get_entry (engine, key, NULL, TRUE, NULL);
	schema_name = gconf_entry_get_schema_name (entry);

	if (schema_name == NULL)
		return NULL;

	schema = gconf_client_get_schema (client, schema_name, NULL);

	return schema;
}

gboolean
gconf_client_can_edit_defaults (void)
{
	GConfEngine *defaults_engine = NULL;
	GError *error = NULL;

	defaults_engine = gconf_engine_get_local (GCONF_DEFAULTS_SOURCE, NULL);
	if (error) {
		return FALSE;
	}

	gconf_engine_set_bool (defaults_engine, "/apps/gconf-editor/can_edit_defaults", FALSE, &error);
	if (error) {
		gconf_engine_unref (defaults_engine);
		return FALSE;
	}
	gconf_engine_suggest_sync (defaults_engine, &error);
	if (error) {
		gconf_engine_unref (defaults_engine);
		return FALSE;
	}
	gconf_engine_unref (defaults_engine);
	return TRUE;
}

gboolean
gconf_client_can_edit_mandatory (void)
{
	GConfEngine *defaults_engine = NULL;
	GError *error = NULL;

	defaults_engine = gconf_engine_get_local (GCONF_MANDATORY_SOURCE, NULL);
	if (error) {
		return FALSE;
	}

	gconf_engine_set_bool (defaults_engine, "/apps/gconf-editor/can_edit_mandatory", FALSE, &error);
	if (error) {
		gconf_engine_unref (defaults_engine);
		return FALSE;
	}
	gconf_engine_suggest_sync (defaults_engine, &error);
	if (error) {
		gconf_engine_unref (defaults_engine);
		return FALSE;
	}	
	gconf_engine_unref (defaults_engine);
	return TRUE;
}

	

