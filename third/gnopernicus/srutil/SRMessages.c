/* SRMessages.h
 *
 * Copyright 2001, 2002 Sun Microsystems, Inc.,
 * Copyright 2001, 2002 BAUM Retec, A.G.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>
#include "SRMessages.h"

#ifdef SRU_PARANOIA
#define GNOPERNICUS_DEBUG "GNOPERNICUS_DEBUG"
#define GNOPERNICUS_DEBUG_STACK "GNOPERNICUS_DEBUG_STACK"

GLogLevelFlags sru_log_flags 		= 0;
GLogLevelFlags sru_log_stack_flags 	= 0;

static G_CONST_RETURN gchar*
sru_log_get_env_var (const gchar *env)
{
    G_CONST_RETURN gchar *val;

    sru_assert (env);
    sru_assert (strcmp (env, GNOPERNICUS_DEBUG) == 0 || 
		    strcmp (env, GNOPERNICUS_DEBUG_STACK) == 0);
    
    val = g_getenv (env);
    if (!val)
	val = "important";

    if (strcmp (val, "important") == 0)
    {
	if (strcmp (env, GNOPERNICUS_DEBUG) == 0)
	    val = "error:warning:message";
	else if (strcmp (env, GNOPERNICUS_DEBUG_STACK) == 0)
	    val = "";
    }
    else if (strcmp (val, "all") == 0)
	val = "error:critical:warning:message:info:debug";
    else if (strcmp (val, "none") == 0)
	val = "";

    return val;
}



static GLogLevelFlags
sru_log_get_flags_from_env_var (const gchar *env)
{
    G_CONST_RETURN gchar *val;
    gchar **vals;
    gint i;
    GLogLevelFlags flags = 0;
    static struct 
	{
	    GLogLevelFlags flag;
	    gchar *name;
	}flag_name[] = 
	    {
		{ G_LOG_LEVEL_ERROR, 	"error"},
		{ G_LOG_LEVEL_CRITICAL,	"critical"},
		{ G_LOG_LEVEL_WARNING,	"warning"},
		{ G_LOG_LEVEL_MESSAGE,	"message"},
		{ G_LOG_LEVEL_INFO,	"info"},
		{ G_LOG_LEVEL_DEBUG,	"debug"}
	    };

    sru_assert (env);

    val = sru_log_get_env_var (env);

    vals = g_strsplit (val, ":", 6);
    for (i = 0; vals[i]; i++)
    {
	gint j;
	for (j = 0; j < G_N_ELEMENTS (flag_name); j++)
	{
	    if (strcmp (flag_name[j].name, vals[i]) == 0)
	    {
		flags |= flag_name[j].flag;
		break;
	    }		
	}
	if (j == G_N_ELEMENTS (flag_name))
	    fprintf (stderr, "\"%s\" is unknown value for \"%s\" environment variable", vals[i], env);
    }

    g_strfreev (vals);
    
    return flags;
}


gboolean
sru_log_init ()
{
    sru_log_flags = sru_log_get_flags_from_env_var (GNOPERNICUS_DEBUG);
    sru_log_stack_flags = sru_log_get_flags_from_env_var (GNOPERNICUS_DEBUG_STACK);
    return TRUE;
}

gboolean
sru_log_terminate ()
{
    return TRUE;
}
#endif /* SRU_PARANOIA */
