/* gok-gconf.h
*
* Copyright 2001,2002 Sun Microsystems, Inc.,
* Copyright 2001,2002 University Of Toronto
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

gboolean gok_gconf_get_double (GConfClient *client, const gchar *key,
			       gdouble *dest);

gboolean gok_gconf_set_double (GConfClient *client, const gchar *key,
			       gdouble value);

gboolean gok_gconf_get_int (GConfClient *client, const gchar *key,
                            gint *dest);

gboolean gok_gconf_set_int (GConfClient *client, const gchar *key,
                            gint value);

gboolean gok_gconf_get_bool (GConfClient *client, const gchar *key,
                             gboolean *dest);

gboolean gok_gconf_set_bool (GConfClient *client, const gchar *key,
                             gboolean value);

gboolean gok_gconf_get_string (GConfClient *client, const gchar *key,
                               gchar **dest);

gboolean gok_gconf_set_string (GConfClient *client, const gchar *key,
                               const gchar *value);
