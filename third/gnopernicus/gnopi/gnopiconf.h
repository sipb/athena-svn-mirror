/* gnopiconf.h
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

#ifndef __GNOPIINIT__MODULE
#define __GNOPIINIT__MODULE

#include <glib.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>



gboolean	gnopiconf_gconf_client_init (void);
gboolean	gnopiconf_client_add_dir (const gchar *path);
void		gnopiconf_value_change (const gchar *key, 
					const gchar *val);

gboolean	gnopiconf_set_int (gint val,
				    const gchar *key);
gboolean	gnopiconf_set_int_in_section (gint val, 
					      const gchar *section, 
					      const gchar *key);
gboolean	gnopiconf_set_double (gdouble val,
				      const gchar *key);
gboolean	gnopiconf_set_double_in_section (gdouble val, 
						 const gchar *section, 
						 const gchar *key);
gboolean	gnopiconf_set_string (const gchar *val, 
				      const gchar *key);
gboolean	gnopiconf_set_string_in_section (const gchar *val, 
						 const gchar *section, 
						 const gchar *key);
gboolean	gnopiconf_set_bool (gboolean val, 
				    const gchar *key);
gboolean	gnopiconf_set_bool_in_section (gboolean    val, 
					       const gchar *section, 
					       const gchar *key);
gboolean	gnopiconf_get_bool (const gchar* key,
				    GError **error);
gboolean	gnopiconf_set_list (GSList *list, 
				    GConfValueType list_type, 
				    const gchar *key);
gint 		gnopiconf_get_int_with_default (const gchar *key, 
					        gint default_value);
gint 		gnopiconf_get_int_from_section_with_default 
					    (const gchar *section,
					     const gchar *key, 
					     gint default_value);
gdouble		gnopiconf_get_double_from_section_with_default (const gchar *section,
					     const gchar *key, 
					     gdouble default_value);
gdouble		gnopiconf_get_double_with_default (const gchar *key, 
					     gdouble default_value);
gchar* 		gnopiconf_get_string_with_default (const gchar *key, 
					           const gchar *default_value);
gchar*		gnopiconf_get_string_from_section_with_default 
					    (const gchar *section,
					    const gchar *key, 
					    const gchar *default_value);
gboolean	gnopiconf_get_bool_with_default (const gchar* key,
					         gboolean default_value);
gboolean	gnopiconf_get_bool_from_section_with_default 
					    (const gchar *section,
					    const gchar *key, 
					    gboolean default_value);
GSList*		gnopiconf_get_list_with_default (const gchar* key, 
				    		GSList *default_list, 
						GConfValueType *type);
gboolean	gnopiconf_unset_key (const gchar *key);
gboolean	gnopiconf_remove_dir (gchar *dir);
GSList*		gnopiconf_get_all_entries (const gchar *path);

#endif

