/* -*- mode: c; style: linux -*- */

/* preferences.c
 * Copyright (C) 2000 Helix Code, Inc.
 *
 * Written by Bradford Hovinen <hovinen@helixcode.com>,
 *            Martin Baulig <martin@home-of-linux.org>
 *
 * Based on gnome-core/desktop-properties/property-bell.c with
 * ideas from capplets/keyboard-properties/keyboard-properties.c.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>

#include <gnome.h>

#include <gdk/gdkx.h>
#include <X11/X.h>

#ifdef HAVE_X11_EXTENSIONS_XF86MISC_H
#include <X11/extensions/xf86misc.h>
#endif

#include "preferences.h"

static GtkObjectClass *parent_class;

static void preferences_init             (Preferences *prefs);
static void preferences_class_init       (PreferencesClass *class);

static gint       xml_read_int           (xmlNodePtr node,
					  gchar *propname);
static xmlNodePtr xml_write_int          (gchar *name, 
					  gchar *propname, 
					  gint number);

static gint apply_timeout_cb             (Preferences *prefs);

guint
preferences_get_type (void)
{
	static guint preferences_type = 0;

	if (!preferences_type) {
		GtkTypeInfo preferences_info = {
			"Preferences",
			sizeof (Preferences),
			sizeof (PreferencesClass),
			(GtkClassInitFunc) preferences_class_init,
			(GtkObjectInitFunc) preferences_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		preferences_type = 
			gtk_type_unique (gtk_object_get_type (), 
					 &preferences_info);
	}

	return preferences_type;
}

static void
preferences_init (Preferences *prefs)
{
	prefs->frozen = FALSE;

	/* Code to initialize preferences object to defaults */
}

static void
preferences_class_init (PreferencesClass *class) 
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *) class;
	object_class->destroy = preferences_destroy;

	parent_class = 
		GTK_OBJECT_CLASS (gtk_type_class (gtk_object_get_type ()));
}

GtkObject *
preferences_new (void) 
{
	GtkObject *object;

	object = gtk_type_new (preferences_get_type ());

	return object;
}

GtkObject *
preferences_clone (Preferences *prefs)
{
	GtkObject *object;
	Preferences *new_prefs;

	g_return_val_if_fail (prefs != NULL, NULL);
	g_return_val_if_fail (IS_PREFERENCES (prefs), NULL);

	object = preferences_new ();

	new_prefs = PREFERENCES (object);

	new_prefs->percent = prefs->percent;
	new_prefs->pitch = prefs->pitch;
	new_prefs->duration = prefs->duration;

	return object;
}

void
preferences_destroy (GtkObject *object) 
{
	Preferences *prefs;

	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_PREFERENCES (object));

	prefs = PREFERENCES (object);

	/* Code to free dynamically allocated data */

	parent_class->destroy (object);
}

void
preferences_load (Preferences *prefs) 
{
	XKeyboardState kbdstate;

	g_return_if_fail (prefs != NULL);
	g_return_if_fail (IS_PREFERENCES (prefs));

	prefs->percent = gnome_config_get_int("/Desktop/Bell/percent=-1");
	prefs->pitch = gnome_config_get_int("/Desktop/Bell/pitch=-1");
	prefs->duration = gnome_config_get_int("/Desktop/Bell/duration=-1");

	XGetKeyboardControl(GDK_DISPLAY(), &kbdstate);

	if (prefs->percent == -1)
		prefs->percent = kbdstate.bell_percent;

	if (prefs->pitch == -1)
	        prefs->pitch = kbdstate.bell_pitch;

	if (prefs->duration == -1)
	        prefs->duration = kbdstate.bell_duration;
}

void 
preferences_save (Preferences *prefs) 
{
	g_return_if_fail (prefs != NULL);
	g_return_if_fail (IS_PREFERENCES (prefs));

	gnome_config_set_int ("/Desktop/Bell/percent", prefs->percent);
	gnome_config_set_int ("/Desktop/Bell/pitch", prefs->pitch);
	gnome_config_set_int ("/Desktop/Bell/duration", prefs->duration);

	gnome_config_sync ();
}

void
preferences_changed (Preferences *prefs) 
{
	if (prefs->frozen) return;

	if (prefs->timeout_id)
		gtk_timeout_remove (prefs->timeout_id);

	preferences_apply_now (prefs);
}

void
preferences_apply_now (Preferences *prefs)
{
	XKeyboardControl kbdcontrol;

	g_return_if_fail (prefs != NULL);
	g_return_if_fail (IS_PREFERENCES (prefs));

	if (prefs->timeout_id)
		gtk_timeout_remove (prefs->timeout_id);

	prefs->timeout_id = 0;

	kbdcontrol.bell_percent = prefs->percent;
	kbdcontrol.bell_pitch = prefs->pitch;
	kbdcontrol.bell_duration = prefs->duration;

	XChangeKeyboardControl(GDK_DISPLAY(),
			       KBBellPercent | KBBellPitch | KBBellDuration, 
			       &kbdcontrol);
}

void preferences_freeze (Preferences *prefs) 
{
	prefs->frozen = TRUE;
}

void preferences_thaw (Preferences *prefs) 
{
	prefs->frozen = FALSE;
}

Preferences *
preferences_read_xml (xmlDocPtr xml_doc) 
{
	Preferences *prefs;
	xmlNodePtr root_node, node;

	prefs = PREFERENCES (preferences_new ());

	root_node = xmlDocGetRootElement (xml_doc);

	if (strcmp (root_node->name, "bell-properties"))
		return NULL;

	for (node = root_node->childs; node; node = node->next) {
                if (!strcmp (node->name, "percent"))
                        prefs->percent = xml_read_int (node, NULL);
                else if (!strcmp (node->name, "pitch"))
                        prefs->pitch = xml_read_int (node, NULL);
                else if (!strcmp (node->name, "duration"))
                        prefs->duration = xml_read_int (node, NULL);
	}

	return prefs;
}

xmlDocPtr 
preferences_write_xml (Preferences *prefs) 
{
	xmlDocPtr doc;
	xmlNodePtr node;
	char *tmp;

	doc = xmlNewDoc ("1.0");

	node = xmlNewDocNode (doc, NULL, "bell-properties", NULL);

        xmlAddChild (node, xml_write_int ("percent", NULL, prefs->percent));
        xmlAddChild (node, xml_write_int ("pitch", NULL, prefs->pitch));
        xmlAddChild (node, xml_write_int ("duration", NULL, prefs->duration));

	xmlDocSetRootElement (doc, node);

	return doc;
}

/* Read a numeric value from a node */

static gint
xml_read_int (xmlNodePtr node, char *propname) 
{
	char *text;

	if (propname == NULL)
		text = xmlNodeGetContent (node);
	else
		text = xmlGetProp (node, propname);

	if (text == NULL) 
		return 0;
	else
		return atoi (text);
}

/* Write out a numeric value in a node */

static xmlNodePtr
xml_write_int (gchar *name, gchar *propname, gint number) 
{
	xmlNodePtr node;
	gchar *str;

	g_return_val_if_fail (name != NULL, NULL);

	str = g_strdup_printf ("%d", number);

	node = xmlNewNode (NULL, name);

	if (propname == NULL)
		xmlNodeSetContent (node, str);
	else
		xmlSetProp (node, propname, str);

	g_free (str);

	return node;
}

static gint 
apply_timeout_cb (Preferences *prefs) 
{
	preferences_apply_now (prefs);

	return TRUE;
}
