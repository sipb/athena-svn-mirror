/* $Id: gdict-pref.c,v 1.1.1.3 2004-10-04 05:06:08 ghudson Exp $ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *  Mike Hughes <mfh@psilord.com>
 *  Bradford Hovinen <hovinen@udel.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  GDict preferences
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define GTK_ENABLE_BROKEN
#include <gnome.h>
#include "dict.h"
#include "gdict-pref.h"
#include "gdict-app.h"

static GConfClient *gdict_client = NULL;

GDictPref gdict_pref = { 
    NULL, 0, TRUE, NULL, NULL, TRUE
};

GConfClient *
gdict_get_gconf_client (void)
{
    if (!gdict_client)
        gdict_client = gconf_client_get_default ();

    return gdict_client;
}

static void
database_changed_cb (GConfClient *client, guint id, GConfEntry *entry, gpointer data)
{
    GConfValue *value = gconf_entry_get_value (entry);
 
    if (gdict_pref.database != NULL)
	g_free (gdict_pref.database);   
    gdict_pref.database = g_strdup (gconf_value_get_string (value));
}

static void
strat_changed_cb (GConfClient *client, guint id, GConfEntry *entry, gpointer data)
{
    GConfValue *value = gconf_entry_get_value (entry);
    
    if (gdict_pref.dfl_strat != NULL)
	g_free (gdict_pref.dfl_strat);   
    gdict_pref.dfl_strat = g_strdup (gconf_value_get_string (value));
}

static void
smart_changed_cb (GConfClient *client, guint id, GConfEntry *entry, gpointer data)
{
    GConfValue *value = gconf_entry_get_value (entry);
    
    gdict_pref.smart = gconf_value_get_bool (value);
}

/* gdict_pref_load
 *
 * Loads configuration from config file
 */

void 
gdict_pref_load (void)
{
    GError *error = NULL;

    gconf_client_add_dir(gdict_get_gconf_client (), "/apps/gnome-dictionary", GCONF_CLIENT_PRELOAD_NONE, NULL);
    
    if (gdict_pref.server != NULL)
        g_free(gdict_pref.server);
    
    /* FIXME: notification for the server stuff is really tricky
    ** Leaving it out for now */
    gdict_pref.server = gconf_client_get_string (gdict_get_gconf_client (), "/apps/gnome-dictionary/server", NULL);
    
    gdict_pref.port = gconf_client_get_int (gdict_get_gconf_client (), "/apps/gnome-dictionary/port", NULL);
    
    gdict_pref.smart = gconf_client_get_bool(gdict_get_gconf_client (),
"/apps/gnome-dictionary/smart", &error);
    if (error) {
        gdict_pref.smart = TRUE;
        g_error_free (error);
    }

    gconf_client_notify_add (gdict_get_gconf_client (), "/apps/gnome-dictionary/smart", smart_changed_cb, NULL, NULL, NULL);
    
    gdict_pref.database = gconf_client_get_string (gdict_get_gconf_client (), "/apps/gnome-dictionary/database", NULL);
    gconf_client_notify_add (gdict_get_gconf_client (), "/apps/gnome-dictionary/database", database_changed_cb, NULL, NULL, NULL);
    
    gdict_pref.dfl_strat = gconf_client_get_string (gdict_get_gconf_client (), "/apps/gnome-dictionary/strategy", NULL);
    gconf_client_notify_add (gdict_get_gconf_client (), "/apps/gnome-dictionary/strategy", strat_changed_cb, NULL, NULL, NULL);

    /* If things go bad and gconf doesn't return values, we shouldn't die so just return the 
       default schemas */
    if (!gdict_pref.server)
	gdict_pref.server = g_strdup ("dict.org");
    if (!gdict_pref.port)
	gdict_pref.port = 2628;
    if (!gdict_pref.database)
	gdict_pref.database = "!";
    if (!gdict_pref.dfl_strat)
	gdict_pref.dfl_strat = "lev";
}


