/* $Id: util-gconf.c,v 1.1.1.1 2001-01-16 15:26:21 ghudson Exp $
 *
 * util-gconf: Utility functions for dealing with gconf
 *
 * Copyright (C) 2000  Eazel, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * Authors: Mike Fleming <mfleming@eazel.com>
 */

#include "util-gconf.h"
#include <glib.h>
#include <string.h>

#include "log.h"

/*
 * Global Variables
 */

GConfEngine *gl_gconf_engine 		= NULL;
static GList * s_watch_list 		= NULL;  /* list of UtilGConfWatchVariable *'s */

void
util_gconf_init (void)
{
	GError	  *error = NULL;

	if (!gconf_is_initialized ()) {
		char		  *argv[] = { "eazel-proxy", NULL };
		
		if (!gconf_init (1, argv, &error)) {
			g_assert (error != NULL);
			
			g_warning ("GConf init failed:\n  %s", error->message);
			
			g_error_free (error);
			
			error = NULL;
			
			return;
		}
	}

	g_assert ( NULL == gl_gconf_engine );
	/* FIXME: We never unref.  Is this bad? */
	gl_gconf_engine = gconf_engine_get_default ();

	if (NULL == gl_gconf_engine) {
		log ("Couldn't get default gconf engine..what the heck?");
		return;
	}
}

static void 
util_gconf_set_watched (UtilGConfWatchVariable *watch, const GConfValue *val)
{
	/* NULL means the variable has been unset */
	if( NULL == val ) {
		switch (watch->type) {
		case GCONF_VALUE_STRING:
			if (NULL != (watch->t.p_string)) {
				g_free (*(watch->t.p_string));
				*(watch->t.p_string) = NULL;
			}
			break;
		case GCONF_VALUE_INT:
			if (NULL != (watch->t.p_int)) {
				*(watch->t.p_int) = 0; /* is this right? */
			}
			break;
		case GCONF_VALUE_BOOL:
			if (NULL != (watch->t.p_boolean)) {
				*(watch->t.p_boolean) = FALSE; /* is this right? */
			}
			break;
		default:
			g_assert (FALSE);		
		}
	} else if (watch->type == val->type) {
		switch (watch->type) {
		case GCONF_VALUE_STRING:
			if (NULL != (watch->t.p_string)) {
				g_free (*(watch->t.p_string));
				*(watch->t.p_string) = g_strdup (gconf_value_get_string (val));
			}
			break;
		case GCONF_VALUE_INT:
			if (NULL != (watch->t.p_int)) {
				*(watch->t.p_int) = gconf_value_get_int (val);
			}
			break;
		case GCONF_VALUE_BOOL:
			if (NULL != (watch->t.p_boolean)) {
				*(watch->t.p_boolean) = gconf_value_get_bool (val);
			}
			break;
		default:
			g_assert (FALSE);
		
		}
	}

	/* Call the function even if its the wrong type */

	if (watch->func_cb) {
		watch->func_cb (watch, val);
	}
}

static void /* GConfNotifyFunc */
util_gconf_notify_cb (
	GConfEngine *conf, 
	guint cnxn_id,
	GConfEntry* entry,
	gpointer user_data
) {
	UtilGConfWatchVariable *watch;

	watch = (UtilGConfWatchVariable*) (user_data);

	if (NULL == watch) {
		return;
	}

	util_gconf_set_watched (watch, entry->value);
}

void
util_gconf_watch_variable (const UtilGConfWatchVariable *to_watch)
{
	UtilGConfWatchVariable *my_watch;
	GConfValue *val = NULL;
	GError *err_gconf = NULL;
	guint notify_id;

	g_return_if_fail (NULL != gl_gconf_engine);
	g_return_if_fail (NULL != to_watch);
	g_return_if_fail (
		GCONF_VALUE_STRING == to_watch->type
		|| GCONF_VALUE_INT == to_watch->type
		|| GCONF_VALUE_BOOL == to_watch->type
	);

	/* copy watch */	
	my_watch = g_new0(UtilGConfWatchVariable, 1);
	memcpy (my_watch, to_watch, sizeof (UtilGConfWatchVariable) );
	my_watch->key = g_strdup (to_watch->key);


	/* keep track of watch */
	s_watch_list = g_list_prepend (s_watch_list, my_watch);

	/* Get current value */
	val = gconf_engine_get (gl_gconf_engine, my_watch->key, &err_gconf);

	if ( NULL != val && NULL == err_gconf ) {
		util_gconf_set_watched (my_watch, val);	
		gconf_value_free (val);
	}

	notify_id = gconf_engine_notify_add (
				gl_gconf_engine,
				my_watch->key,
				util_gconf_notify_cb,
				(gpointer) my_watch,
				&err_gconf
			    );

	g_assert (0 != notify_id);

	g_clear_error (&err_gconf);
}
