/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-config.c: And frontend abstraction to whatever config system we eventually have
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors:
 *    Lauris Kaplinski <lauris@helixcode.com>
 *    Chema Celorio <chema@ximian.com>
 *
 *  Copyright 2001-2003 Ximian, Inc.
 */

#include <config.h>
#include <ctype.h>
#include <stdlib.h>
#include <locale.h>

#include <libgnomeprint/gpa/gpa-config.h>
#include <libgnomeprint/gnome-print-config.h>
#include <libgnomeprint/gnome-print-config-private.h>
#include <libgnomeprint/gnome-print-job.h>

typedef struct _GnomePrintConfigClass GnomePrintConfigClass;

struct _GnomePrintConfig {
	GObject parent;
	GPANode *node;
};

struct _GnomePrintConfigClass {
	GObjectClass parent_class;
};

static void gnome_print_config_class_init (GnomePrintConfigClass *klass);
static void gnome_print_config_init (GnomePrintConfig *pc);
static void gnome_print_config_finalize (GObject *object);

static GObjectClass *parent_class = NULL;

GType
gnome_print_config_get_type (void)
{
	static GType type;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomePrintConfigClass),
			NULL, NULL,
			(GClassInitFunc) gnome_print_config_class_init,
			NULL, NULL,
			sizeof (GnomePrintConfig),
			0,
			(GInstanceInitFunc) gnome_print_config_init
		};
		type = g_type_register_static (G_TYPE_OBJECT, "GnomePrintConfig", &info, 0);
	}
	return type;
}

static void
gnome_print_config_class_init (GnomePrintConfigClass *klass)
{
	GObjectClass *object_class;

	object_class = (GObjectClass*) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gnome_print_config_finalize;
}

static void
gnome_print_config_init (GnomePrintConfig *config)
{
	config->node = NULL;	
}

static void
gnome_print_config_finalize (GObject *object)
{
	GnomePrintConfig *config;

	config = GNOME_PRINT_CONFIG (object);

	config->node = gpa_node_unref (config->node);
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
}


GPANode *
gnome_print_config_get_node (GnomePrintConfig *config)
{
	g_return_val_if_fail (config != NULL, NULL);

	return config->node;
}

GnomePrintConfig *
gnome_print_config_default (void)
{
	GnomePrintConfig *config;

	config = g_object_new (GNOME_TYPE_PRINT_CONFIG, NULL);
	config->node = (GPANode *) gpa_config_new ();

	return config;
}

GnomePrintConfig *
gnome_print_config_ref (GnomePrintConfig *config)
{
	g_return_val_if_fail (config != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_CONFIG (config), NULL);

	g_object_ref (G_OBJECT (config));

	return config;
}

GnomePrintConfig *
gnome_print_config_unref (GnomePrintConfig *config)
{
	g_return_val_if_fail (config != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_CONFIG (config), NULL);

	g_object_unref (G_OBJECT (config));
	
	return NULL;
}

GnomePrintConfig *
gnome_print_config_dup (GnomePrintConfig *old_config)
{
	GnomePrintConfig *config = NULL;

	g_return_val_if_fail (old_config != NULL, NULL);

	config = g_object_new (GNOME_TYPE_PRINT_CONFIG, NULL);
	
	config->node = gpa_node_duplicate (old_config->node);

	return config;
}

guchar *
gnome_print_config_get (GnomePrintConfig *config, const guchar *key)
{
	guchar *val;

	g_return_val_if_fail (config != NULL, NULL);
	g_return_val_if_fail (key != NULL, NULL);
	g_return_val_if_fail (*key != '\0', NULL);

	val = gpa_node_get_path_value (config->node, key);

	return val;
}

gboolean
gnome_print_config_set (GnomePrintConfig *config, const guchar *key, const guchar *value)
{
	gboolean result;

	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);
	g_return_val_if_fail (value != NULL, FALSE);

	result = gpa_node_set_path_value (config->node, key, value);

	return result;
}

gboolean
gnome_print_config_get_boolean (GnomePrintConfig *config, const guchar *key, gboolean *val)
{
	guchar *v;
	
	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);
	g_return_val_if_fail (val != NULL, FALSE);

	v = gnome_print_config_get (config, key);

	if (v != NULL) {
		if (!g_ascii_strcasecmp (v, "true") ||
		    !g_ascii_strcasecmp (v, "yes") ||
		    !g_ascii_strcasecmp (v, "y") ||
		    !g_ascii_strcasecmp (v, "yes") ||
		    (atoi (v) != 0)) {
			*val = TRUE;
			return TRUE;
		}
		*val = FALSE;
		g_free (v);
		return TRUE;
	}
	
	return FALSE;
}

gboolean
gnome_print_config_get_int (GnomePrintConfig *config, const guchar *key, gint *val)
{
	guchar *v;
	
	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);
	g_return_val_if_fail (val != NULL, FALSE);

	v = gnome_print_config_get (config, key);

	if (v != NULL) {
		gchar *loc;
		loc = g_strdup (setlocale (LC_NUMERIC, NULL));
		setlocale (LC_NUMERIC, "C");
		*val = atoi (v);
		g_free (v);
		setlocale (LC_NUMERIC, loc);
		g_free (loc);
		return TRUE;
	}
	
	return FALSE;
}

gboolean
gnome_print_config_get_double (GnomePrintConfig *config, const guchar *key, gdouble *val)
{
	guchar *v;

	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);
	g_return_val_if_fail (val != NULL, FALSE);

	v = gnome_print_config_get (config, key);

	if (v != NULL) {
		gchar *loc;
		loc = g_strdup (setlocale (LC_NUMERIC, NULL));
		setlocale (LC_NUMERIC, "C");
		*val = atof (v);
		g_free (v);
		setlocale (LC_NUMERIC, loc);
		g_free (loc);
		return TRUE;
	}
	
	return FALSE;
}

gboolean
gnome_print_config_get_length (GnomePrintConfig *config, const guchar *key, gdouble *val, const GnomePrintUnit **unit)
{
	guchar *v;
	const GnomePrintUnit *c_unit;

	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);
	g_return_val_if_fail (val != NULL, FALSE);


	v = gnome_print_config_get (config, key);

	if (v != NULL) {
		guchar *loc, *e;
		loc = g_strdup (setlocale (LC_NUMERIC, NULL));
		setlocale (LC_NUMERIC, "C");
		*val = strtod (v, (gchar **) &e);
		setlocale (LC_NUMERIC, loc);
		g_free (loc);
		if (e == v) {
			g_free (v);
			return FALSE;
		}
		while (*e && !isalnum (*e))
			e++;
		if (!*e) {
			c_unit = GNOME_PRINT_PS_UNIT;
		} else {
			c_unit = gnome_print_unit_get_by_abbreviation (e);
			if (!c_unit)
				c_unit = gnome_print_unit_get_by_name (e);
		}
		g_free (v);
		if (c_unit == NULL)
			return FALSE;
		if (unit != NULL) {
			*unit = c_unit;
		} else {
			gnome_print_convert_distance (val, c_unit, GNOME_PRINT_PS_UNIT);
		}
		return TRUE;
	}
	
	return FALSE;
}

gboolean
gnome_print_config_set_boolean (GnomePrintConfig *config, const guchar *key, gboolean val)
{
	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);

	return gnome_print_config_set (config, key, (val) ? "true" : "false");
}

gboolean
gnome_print_config_set_int (GnomePrintConfig *config, const guchar *key, gint val)
{
	guchar c[128];
	gchar *loc;
	
	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);

	loc = g_strdup (setlocale (LC_NUMERIC, NULL));
	setlocale (LC_NUMERIC, "C");
	g_snprintf (c, 128, "%d", val);
	setlocale (LC_NUMERIC, loc);
	g_free (loc);

	return gnome_print_config_set (config, key, c);
}

gboolean
gnome_print_config_set_double (GnomePrintConfig *config, const guchar *key, gdouble val)
{
	guchar c[128];
	gchar *loc;
	
	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);

	loc = g_strdup (setlocale (LC_NUMERIC, NULL));
	setlocale (LC_NUMERIC, "C");
	g_snprintf (c, 128, "%g", val);
	setlocale (LC_NUMERIC, loc);
	g_free (loc);

	return gnome_print_config_set (config, key, c);
}

gboolean
gnome_print_config_set_length (GnomePrintConfig *config, const guchar *key, gdouble val, const GnomePrintUnit *unit)
{
	guchar c[128];
	gchar *loc;
	
	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);
	g_return_val_if_fail (unit != NULL, FALSE);

	loc = g_strdup (setlocale (LC_NUMERIC, NULL));
	setlocale (LC_NUMERIC, "C");
	g_snprintf (c, 128, "%g%s", val, unit->abbr);
	setlocale (LC_NUMERIC, loc);
	g_free (loc);

	return gnome_print_config_set (config, key, c);
}


gchar *
gnome_print_config_to_string (GnomePrintConfig *config, guint flags)
{
	g_return_val_if_fail (config != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_CONFIG (config), NULL);

	return gpa_config_to_string (GPA_CONFIG (config->node), flags);
}

GnomePrintConfig *
gnome_print_config_from_string (const gchar *str,
				guint flags)
{
	GnomePrintConfig *config;

	g_return_val_if_fail (str != NULL, NULL);

	config = g_object_new (GNOME_TYPE_PRINT_CONFIG, NULL);
	config->node = (GPANode *) gpa_config_from_string (str, flags);

	return config;
}

void
gnome_print_config_dump (GnomePrintConfig *config)
{
	g_return_if_fail (config != NULL);
	g_return_if_fail (GNOME_IS_PRINT_CONFIG (config));

	gpa_utils_dump_tree (config->node, 1);
}

/**
 * gnome_print_config_get_page_size
 * @config: 
 * @width: 
 * @height: 
 * 
 * Get imaging area size available to the application for printing
 * after margins and layouts are applied. Sizes are given in PS
 * points (GNOME_PRINT_PS_UNIT)
 * 
 * Return Value: TRUE on success, FALSE on error
 **/
gboolean
gnome_print_config_get_page_size (GnomePrintConfig *config, gdouble *width, gdouble *height)
{
	GnomePrintJob *job;

	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (GNOME_IS_PRINT_CONFIG (config), FALSE);
	g_return_val_if_fail (width != NULL, FALSE);
	g_return_val_if_fail (height != NULL, FALSE);

	job = gnome_print_job_new (config);

	gnome_print_job_get_page_size (job, width, height);

	g_object_unref (G_OBJECT (job));

	return TRUE;
}
