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
 *    Andreas J. Guelzow <aguelzow@taliesin.ca>
 *
 *  Copyright 2001-2003 Ximian, Inc.
 */

#define GNOME_PRINT_UNSTABLE_API

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <libart_lgpl/art_affine.h>
#include <libgnomeprint/gpa/gpa-config.h>
#include <libgnomeprint/gpa/gpa-key.h>
#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gnome-print-config.h>
#include <libgnomeprint/gnome-print-config-private.h>
#include <libgnomeprint/gnome-print-job.h>

extern int errno;

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

/**
 * gnome_print_config_default
 *
 * Creates a #GnomePrintConfig object with the default printer and settings.
 *
 * Returns: A pointer to a #GnomePrintConfig object with the default settings
 *
 **/
GnomePrintConfig *
gnome_print_config_default (void)
{
	GnomePrintConfig *config;

	config = g_object_new (GNOME_TYPE_PRINT_CONFIG, NULL);
	config->node = (GPANode *) gpa_config_new ();

	return config;
}

/**
 * gnome_print_config_ref
 * @config: The #GnomePrintConfig object to have its reference count increased
 *
 * Increase the reference count on the #GnomePrintConfig object by one.
 *
 * Returns: A pointer to the #GnomePrintConfig object or %NULL on failure
 *
 **/
GnomePrintConfig *
gnome_print_config_ref (GnomePrintConfig *config)
{
	g_return_val_if_fail (config != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_CONFIG (config), NULL);

	g_object_ref (G_OBJECT (config));

	return config;
}

/**
 * gnome_print_config_unref
 * @config: The #GnomePrintConfig object to have its reference count decreased
 *
 * Decrease the reference count on the #GnomePrintConfig object by one.
 *
 * Returns: A pointer to the #GnomePrintConfig object or %NULL on failure.
 *
 **/
GnomePrintConfig *
gnome_print_config_unref (GnomePrintConfig *config)
{
	g_return_val_if_fail (config != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_CONFIG (config), NULL);

	g_object_unref (G_OBJECT (config));
	
	return NULL;
}

/**
 * gnome_print_config_dup
 * @config: The config to be copied
 *
 * Does a deep copy of the config @config.  You should unref the returned 
 * #GnomePrintConfig using #gnome_print_config_unref when you are finished
 * using it.
 *
 * Returns: A copy of config
 *
 **/
GnomePrintConfig *
gnome_print_config_dup (GnomePrintConfig *old_config)
{
	GnomePrintConfig *config = NULL;

	g_return_val_if_fail (old_config != NULL, NULL);

	config = g_object_new (GNOME_TYPE_PRINT_CONFIG, NULL);
	
	config->node = gpa_node_duplicate (old_config->node);

	return config;
}

/**
 * gnome_print_config_keys_compat:
 * @key: 
 * 
 * Used for backward compat when keys change. Known to need love.
 * 
 * Return Value: 
 **/
static gchar *
gnome_print_config_keys_compat (const gchar *key)
{
	const gchar *old [] = {
		"Settings.Transport.Backend.FileName",
		NULL
	};
	const gchar *new [] = {
		GNOME_PRINT_KEY_OUTPUT_FILENAME,
		NULL
	};
	gint i = 0;

	while (old[i]) {
		if (strcmp (old[i], new[i]) == 0) {
			g_print ("Replace %s with %s\n",
				 old[i], new[i]);
			return g_strdup (new[i]);
		}
		i++;
	}

	return g_strdup (key);
}

/**
 * gnome_print_config_get
 * @config: Pointer to a #GnomePrintConfig object
 * @key: String containing the path of key whose value is to be obtained
 *
 * Gets the value of string @key from the #GnomePrintConfig object.  The returned string
 * should be freed with #g_free when you are finished with it.
 *
 * Returns: The value of the key, %NULL indicates failure.
 *
 **/
guchar *
gnome_print_config_get (GnomePrintConfig *config, const guchar *key)
{
	guchar *val, *real_key;

	g_return_val_if_fail (config != NULL, NULL);
	g_return_val_if_fail (key != NULL, NULL);
	g_return_val_if_fail (*key != '\0', NULL);

	real_key = gnome_print_config_keys_compat (key);

	val = gpa_node_get_path_value (config->node, real_key);

	g_free (real_key);

	return val;
}

/**
 * gnome_print_config_set
 * @config: Pointer to a #GnomePrintConfig object
 * @key: String containing the path of key whose value is to be set
 * @value: String containing the value to set
 *
 * Sets the value of string @key in the #GnomePrintConfig object to value @value.
 *
 * Returns: %TRUE on success, %FALSE on failure
 *
 **/
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

/**
 * gnome_print_config_get_boolean
 * @config: Pointer to a #GnomePrintConfig object
 * @key: String containing the path of key whose value is to be obtained
 * @val: Pointer to a boolean variable to store the value in.  Should initially be %NULL
 *
 * Gets the value of key @key from the #GnomePrintConfig object.  Converts values such
 * as "true", "y", "yes", and their opposites, to their boolean equivalent.  The
 * boolean value will be stored in the variable @val.
 *
 * Returns: #TRUE if a value was retrieved, #FALSE on failure
 *
 **/
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

/**
 * gnome_print_config_get_int
 * @config: Pointer to a #GnomePrintConfig object
 * @key: String containing the path of key whose value is to be obtained
 * @val: Pointer to a integer variable to store the value in.  Should initially be %NULL
 *
 * Gets the value of key @key from the #GnomePrintConfig object.  Converts values
 * to their integer equivalent.  The integer value will be stored in the variable 
 * @val.
 *
 * Returns: #TRUE if a value was retrieved, #FALSE on failure
 *
 **/
gboolean
gnome_print_config_get_int (GnomePrintConfig *config, const guchar *key, gint *val)
{
	guchar *v;
	
	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);
	g_return_val_if_fail (val != NULL, FALSE);

	v = gnome_print_config_get (config, key);

	if (v == NULL) 
		return FALSE;
	
	*val = atoi (v);
	g_free (v);
	return TRUE;
}

/**
 * gnome_print_config_get_double
 * @config: Pointer to a #GnomePrintConfig object
 * @key: String containing the path of key whose value is to be obtained
 * @val: Pointer to a double variable to store the value in.  Should initially be %NULL
 *
 * Gets the value of key @key from the #GnomePrintConfig object.  Converts values
 * to their double equivalent.  The double value will be stored in the variable 
 * @val. 
 *
 * Returns: #TRUE if a value was retrieved, #FALSE on failure
 *
 **/
gboolean
gnome_print_config_get_double (GnomePrintConfig *config, const guchar *key, gdouble *val)
{
	guchar *v;
	gboolean result = TRUE;

	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);
	g_return_val_if_fail (val != NULL, FALSE);

	v = gnome_print_config_get (config, key);

	if (v == NULL) 
		return FALSE;

	*val = g_ascii_strtod (v, NULL);
	if (errno != 0) {
		result = FALSE;
		g_warning ("g_ascii_strtod error: %i", errno);
	}
	g_free (v);

	return result;
}

/**
 * gnome_print_config_get_length
 * @config: Pointer to a #GnomePrintConfig object
 * @key: String containing the path of key whose value is to be obtained
 * @val: Pointer to a double variable to store the value in.  Should initially be %NULL
 * @unit: Pointer to an already allocated #GnomePrintUnit struct
 *
 * Gets the value of key @key from the #GnomePrintConfig object.  Converts values
 * to their double equivalent.  The double value will be stored in the variable 
 * @val and the units will be stored in @unit.  You should allocate the storage 
 * for @unit before calling this function.
 *
 * Returns: #TRUE if a value was retrieved, #FALSE on failure
 *
 **/
gboolean
gnome_print_config_get_length (GnomePrintConfig *config, const guchar *key, gdouble *val, const GnomePrintUnit **unit)
{
	guchar *v, *e;
	const GnomePrintUnit *c_unit = NULL;

	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);
	g_return_val_if_fail (val != NULL, FALSE);

	v = gnome_print_config_get (config, key);

	if (v == NULL)
		return FALSE;

	*val = g_ascii_strtod (v, (gchar **) &e);

	if ((errno != 0) || (e == v)) {
		g_free (v);
		return FALSE;
	}
	while (*e && !g_ascii_isalnum (*e))
		e++;
	if (*e != '\0') {
		c_unit = gnome_print_unit_get_by_abbreviation (e);
		if (!c_unit)
			c_unit = gnome_print_unit_get_by_name (e);
	}
	if (c_unit == NULL)
		c_unit = GNOME_PRINT_PS_UNIT;
	g_free (v);

	if (unit != NULL) {
		*unit = c_unit;
	} else {
		gnome_print_convert_distance (val, c_unit, GNOME_PRINT_PS_UNIT);
	}
	return TRUE;

}

/**
 * gnome_print_config_set_boolean
 * @config: Pointer to a #GnomePrintConfig object
 * @key: String containing the path of key whose value is to be set
 * @val: Boolean containing the value to set
 *
 * Set a @boolean value in the #GnomePrintConfig object.
 *
 * Returns: %TRUE on success, %FALSE on failure
 *
 **/
gboolean
gnome_print_config_set_boolean (GnomePrintConfig *config, const guchar *key, gboolean val)
{
	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);

	return gnome_print_config_set (config, key, (val) ? "true" : "false");
}

/**
 * gnome_print_config_set_int
 * @config: Pointer to a #GnomePrintConfig object
 * @key: String containing the path of key whose value is to be set
 * @val: Integer containing the value to set
 *
 * Set an @integer value in the #GnomePrintConfig object.
 *
 * Returns: %TRUE on success, %FALSE on failure
 *
 **/
gboolean
gnome_print_config_set_int (GnomePrintConfig *config, const guchar *key, gint val)
{
	guchar c[G_ASCII_DTOSTR_BUF_SIZE];
	
	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);

	g_snprintf (c, G_ASCII_DTOSTR_BUF_SIZE, "%d", val);

	return gnome_print_config_set (config, key, c);
}

/**
 * gnome_print_config_set_double
 * @config: Pointer to a #GnomePrintConfig object
 * @key: String containing the path of key whose value is to be set
 * @val: Double containing the value to set
 *
 * Set a @double value in the #GnomePrintConfig object.
 *
 * Returns: %TRUE on success, %FALSE on failure
 *
 **/
gboolean
gnome_print_config_set_double (GnomePrintConfig *config, const guchar *key, gdouble val)
{
	guchar c[G_ASCII_DTOSTR_BUF_SIZE];
	
	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);

	g_ascii_dtostr (c, sizeof (c), val);

	return gnome_print_config_set (config, key, c);
}

/**
 * gnome_print_config_set_length
 * @config: Pointer to a #GnomePrintConfig object
 * @key: String containing the path of key whose value is to be set
 * @val: Double containing the value to set
 * @unit: Units to use when setting value
 *
 * Sets a double value and the units it is using.  This should be used in 
 * conjunction with #gnome_print_config_get_length.
 *
 * Returns: %TRUE on success, %FALSE on failure
 *
 **/
gboolean
gnome_print_config_set_length (GnomePrintConfig *config, const guchar *key, gdouble val, const GnomePrintUnit *unit)
{
	guchar c[G_ASCII_DTOSTR_BUF_SIZE];
	gboolean result = FALSE;
	gchar *c_abbr;
	
	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);
	g_return_val_if_fail (unit != NULL, FALSE);

	g_ascii_dtostr (c, sizeof (c), val);
	c_abbr = g_strconcat (c, unit->abbr, NULL);

#if 0
	g_warning ("gnome_print_config_set_length %s %s", key, unit->abbr);
#endif

	result = gnome_print_config_set (config, key, c_abbr);
	g_free (c_abbr);

	return result;
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

/**
 * gnome_print_config_dump
 * @gpc: The #GnomePrintConfig to output
 *
 * Print out the tree structure representing the GnomePrintConfig.  Output
 * is to %STDOUT and is limited to a depth of 20.
 *
 **/
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


gboolean
gnome_print_config_insert_boolean (GnomePrintConfig *config, const guchar *key, gboolean def)
{
	GPANode *app;

	g_return_val_if_fail (GNOME_IS_PRINT_CONFIG (config), FALSE);
	g_return_val_if_fail (key != NULL, FALSE);

	if (strncmp (key, "Settings.Application.", strlen ("Settings.Application."))) {
		g_warning ("Applications can only append nodes inside the \"Settings.Application\"\n"
			   "subtree. Node \"%s\" not could not be apppended.", key);
		return FALSE;
	}

	app = gpa_node_lookup (config->node, "Settings.Application");
	if (!app) {
		g_warning ("Could not find Settings.Application");
		return FALSE;
	}

	key += strlen ("Settings.Application.");
	gpa_key_insert (app, key, def ? "true" : "false");

	return TRUE;
}

gboolean
gnome_print_config_insert_options (GnomePrintConfig *config, const guchar *key,
				   GnomePrintConfigOption *options, const gchar *def)
{
	GPANode *app;

	g_return_val_if_fail (GNOME_IS_PRINT_CONFIG (config), FALSE);
	g_return_val_if_fail (options != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);

	if (strncmp (key, "Settings.Application.", strlen ("Settings.Application."))) {
		g_warning ("Applications can only append nodes inside the \"Settings.Application\"\n"
			   "subtree. Node \"%s\" not could not be apppended.", key);
		return FALSE;
	}

	app = gpa_node_lookup (config->node, "Settings.Application");
	if (!app) {
		g_warning ("Could not find Settings.Application");
		return FALSE;
	}

	key += strlen ("Settings.Application.");
	gpa_key_insert (app, key, def);

	return TRUE;
}

gboolean
gnome_print_config_get_option (GnomePrintConfig *config, const guchar *key,
			       GnomePrintConfigOption *options, gint *index)
{
	GnomePrintConfigOption option;
	gchar *value;
	gint i = 0;

	g_return_val_if_fail (GNOME_IS_PRINT_CONFIG (config), FALSE);
	g_return_val_if_fail (options != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (index != NULL, FALSE);
	
	*index = 0;

	value = gnome_print_config_get (config, key);

	option = options[i++];
	while (option.description) {
		if (strcmp (option.id, value) == 0) {
			*index = option.index;
			break;
		}
		option = options[i++];
	}

	return TRUE;
}

gboolean
gnome_print_config_get_transform (GnomePrintConfig *config, const guchar *key, gdouble *transform)
{
	guchar *v;
	gdouble t[6];
	gboolean ret;

	g_return_val_if_fail (config != NULL, FALSE);
	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (*key != '\0', FALSE);
	g_return_val_if_fail (config != NULL, FALSE);

	v = gnome_print_config_get (config, key);
	if (!v) {
		return FALSE;
	}

	ret = gnome_print_parse_transform (v, t);
	g_free (v);
	if (ret) {
		memcpy (transform, t, 6 * sizeof (gdouble));
	}

	return ret;
}

/* All keys can be NULL */
GnomePrintLayoutData *
gnome_print_config_get_layout_data (GnomePrintConfig *config,
				    const guchar *pagekey,
				    const guchar *porientkey,
				    const guchar *lorientkey,
				    const guchar *layoutkey)
{
	GnomePrintLayoutData *lyd;
	guchar key[1024];
	const GnomePrintUnit *unit;
	GPANode *layout;

	/* Local data */
	GnomePrintLayoutPageData *pages;
	gdouble porient[6], lorient[6];
	gdouble pw, ph, lyw, lyh;
	gint num_pages;
	gint numlp;
	
	g_return_val_if_fail (config != NULL, NULL);

	if (!pagekey)
		pagekey = GNOME_PRINT_KEY_PAPER_SIZE;
	if (!porientkey)
		porientkey = GNOME_PRINT_KEY_PAPER_ORIENTATION;
	if (!lorientkey)
		lorientkey = GNOME_PRINT_KEY_PAGE_ORIENTATION;
	if (!layoutkey)
		layoutkey = GNOME_PRINT_KEY_LAYOUT;


	/* Initialize */
	pw = 210 * 72.0 / 25.4;
	ph = 297 * 72.0 / 25.4;
	art_affine_identity (porient);
	art_affine_identity (lorient);
	lyw = 1.0;
	lyh = 1.0;
	num_pages = 0;
	pages = NULL;

	/* Now the fun part */

	/* Physical size */
	g_snprintf (key, 1024, "%s.Width", pagekey);
	if (gnome_print_config_get_length (config, key, &pw, &unit)) {
		gnome_print_convert_distance (&pw, unit, GNOME_PRINT_PS_UNIT);
	}
	g_snprintf (key, 1024, "%s.Height", pagekey);
	if (gnome_print_config_get_length (config, key, &ph, &unit)) {
		gnome_print_convert_distance (&ph, unit, GNOME_PRINT_PS_UNIT);
	}
	/* Physical orientation */
	g_snprintf (key, 1024, "%s.Paper2PrinterTransform", porientkey);
	gnome_print_config_get_transform (config, key, porient);
	/* Logical orientation */
	g_snprintf (key, 1024, "%s.Page2LayoutTransform", lorientkey);
	gnome_print_config_get_transform (config, key, lorient);
	/* Layout size */
	g_snprintf (key, 1024, "%s.Width", layoutkey);
	gnome_print_config_get_double (config, key, &lyw);
	g_snprintf (key, 1024, "%s.Height", layoutkey);
	gnome_print_config_get_double (config, key, &lyh);

	/* Get the layout tree from the config database */
	layout = gpa_node_get_child_from_path (GNOME_PRINT_CONFIG_NODE (config), layoutkey);
	if (!layout) {
		layout = gpa_node_get_child_from_path (NULL, "Globals.Document.Page.Layout.Plain");
		if (!layout) {
			g_warning ("Could not get Globals.Document.Page.Layout.Plain");
			return NULL;
		}
	}

	/* Now come the affines */
	numlp = 0;
	if (gpa_node_get_int_path_value (layout, "LogicalPages", &numlp) && (numlp > 0)) {
		GPANode *pnodes;
		pnodes = gpa_node_get_child_from_path (layout, "Pages");
		if (pnodes) {
			GPANode *page;
			gint pagenum;
			pages = g_new (GnomePrintLayoutPageData, numlp);
			pagenum = 0;
			for (page = gpa_node_get_child (pnodes, NULL); page != NULL; page = gpa_node_get_child (pnodes, page)) {
				guchar *transform;
				transform = gpa_node_get_value (page);
				gpa_node_unref (page);
				if (!transform)
					break;
				gnome_print_parse_transform (transform, pages[pagenum].matrix);
				g_free (transform);
				pagenum += 1;
				if (pagenum >= numlp)
					break;
			}
			gpa_node_unref (pnodes);
			if (pagenum == numlp) {
				num_pages = numlp;
			} else {
				g_free (pages);
			}
		}
	}
	gpa_node_unref (layout);

	if (num_pages == 0) {
		g_warning ("Could not get_layout_data\n");
		return NULL;
	}

	/* Success */

	lyd = g_new (GnomePrintLayoutData, 1);
	lyd->pw = pw;
	lyd->ph = ph;
	memcpy (lyd->porient, porient, 6 * sizeof (gdouble));
	memcpy (lyd->lorient, lorient, 6 * sizeof (gdouble));
	lyd->lyw = lyw;
	lyd->lyh = lyh;
	lyd->num_pages = num_pages;
	lyd->pages = pages;

	return lyd;
}




