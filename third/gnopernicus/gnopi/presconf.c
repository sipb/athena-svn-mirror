/* presconf.c
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
#include "presconf.h"
#include "SRMessages.h"
#include "gnopiconf.h"
#include <string.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include "libsrconf.h"
#include "srintl.h"


extern GConfClient *gnopernicus_client;

gboolean 
presconf_gconf_init (void)
{
    if (!gnopiconf_client_add_dir (CONFIG_PATH PRESENTATION_PATH))
	return FALSE;

    return TRUE;
}

void		
presconf_set_defaults 		(void)
{
    presconf_active_setting_set (DEFAULT_PRESENTATION_ACTIVE_SETTING_NAME);
}

void 
presconf_gconf_terminate (void)
{    	
    if (gconf_client_dir_exists (gnopernicus_client, CONFIG_PATH PRESENTATION_PATH,NULL))
	gconf_client_remove_dir (gnopernicus_client, CONFIG_PATH PRESENTATION_PATH,NULL);	
	
}



GSList*
presconf_all_settings_get (void)
{
    GSList *list = NULL;
    GDir *dir ;

    dir = g_dir_open (PRESENTATION_DIR, 0, NULL);
    if (dir)
    {
	const gchar *file;
	for (file = g_dir_read_name (dir); file; file = g_dir_read_name (dir))
	{
	    gchar *tmp = g_strrstr(file, ".xml");
	    if (tmp && !tmp[4])
	    {
		gchar *name = g_strndup (file,  tmp - file);
		list = g_slist_append (list, name);
	    }
	    else
	    {
		sru_warning (_("incorrect file name. \"file.xml\" name is expected."));
	    }
	}	
	g_dir_close (dir);
    }
    return list;
}


gboolean
presconf_check_if_setting_in_list (GSList *list, const gchar *name, GSList **ret)
{
    GSList *tmp = NULL;

    if (!name)
    {
	*ret = list;
	return FALSE;
    }    
    
    *ret = NULL;
    for (tmp = list; tmp ; tmp = tmp->next)
    {
	if (!strcmp ((gchar*)tmp->data, name))
	{
	    *ret = tmp;
	    return TRUE;
	}
    }
    
    *ret = list;
    return FALSE;
}

gchar*
presconf_active_setting_get (void)
{
    gchar  *key = NULL;
    gchar  *setting = NULL;
    
    key = g_strdup_printf ("%s/%s", CONFIG_PATH PRESENTATION_PATH, PRESENTATION_ACTIVE_SETTING);
    setting = gnopiconf_get_string_with_default (key, DEFAULT_PRESENTATION_ACTIVE_SETTING_NAME);
    g_free (key);
    
    return setting;
}

static void
presconf_changes_end (void)
{
    static gboolean presentation_end = FALSE;
    presentation_end = !presentation_end;
    gnopiconf_set_bool (presentation_end,
			CONFIG_PATH PRESENTATION_PATH PRESENTATION_CHANGES_END);
}

void
presconf_active_setting_set (const gchar *setting_name)
{
    gchar  *key = NULL;
    key = g_strdup_printf ("%s/%s", CONFIG_PATH PRESENTATION_PATH, PRESENTATION_ACTIVE_SETTING);
    gnopiconf_set_string (setting_name, key);
    g_free (key);
    presconf_changes_end ();
}



