/* $Id: util-gconf.h,v 1.1.1.1 2001-01-16 15:25:58 ghudson Exp $
 *
 * utils-gconf: Utility functions for dealing with gconf
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

#ifndef _UTIL_GCONF_H_
#define _UTIL_GCONF_H_

#include <gconf/gconf.h>

/*
 * Global Variables
 */
 
extern GConfEngine *gl_gconf_engine;

/*
 * Types
 */
typedef struct UtilGConfWatchVariable UtilGConfWatchVariable;


typedef void (*UtilGConfCb)(const UtilGConfWatchVariable *watched, const GConfValue *new_value);

struct UtilGConfWatchVariable {
	gchar *key;
	GConfValueType type;
	union {
		gchar **	p_string;	/* GNOME_VALUE_STRING */
		gint *		p_int;		/* GCONF_VALUE_INT */
		gboolean *	p_boolean;	/* GCONF_VALUE_BOOL */
	}t;
	UtilGConfCb func_cb;
};

/*
 * Functions
 */

void util_gconf_init (void);
void util_gconf_watch_variable (const UtilGConfWatchVariable *to_watch);

#endif /* _UTIL_GCONF_H_ */
