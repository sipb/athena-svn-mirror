/*
 * cddb-slave-private.c: Internal functions (debugging)
 *
 * Copyright (C) 2004 Thomas Vander Stichele
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "cddb-slave-private.h"

/* debugging mode; set through CDDB_SLAVE_DEBUG env var */
static gboolean cddb_slave2_debug = FALSE;

void
cs_set_debug (gboolean active)
{
  cddb_slave2_debug = active;
}

/* if cddb_slave2_debug is TRUE
 * (caused by env var CDDB_SLAVE_DEBUG being set)
 * g_print the given message */
void
cs_debug (const gchar *format,
          ...)
{
        va_list args;
        gchar *string;

        if (cddb_slave2_debug == FALSE) {
                return;
        }
        va_start (args, format);
        string = g_strdup_vprintf (format, args);
        va_end (args);

        g_print ("DEBUG: %s\n", string);
        g_free (string);
}
