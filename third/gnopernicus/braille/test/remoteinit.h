/* remoteinit.h
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

#ifndef __GNOPIINIT_MODULE__
#define __GNOPIINIT_MODULE__

#include <glib.h>
#include <stdio.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

gboolean brl_set_int                 (gint        val,
				     const gchar  *key);

gboolean brl_set_string		     (const gchar *val,
                                     const gchar  *key);
gboolean brl_gconf_client_init	     ();

gint     brl_get_int_with_default    (const gchar *key,
                                     gint         default_value);
gchar*   brl_get_string_with_default (const gchar *key,
				     gchar        *default_value);

#endif
