/* gok-glade-helpers.c 
* 
* Copyright 2004 Sun Microsystems, Inc.,
* Copyright 2004 University Of Toronto 
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

#include <gnome.h>
#include <gconf/gconf-client.h>
#include <glade/glade.h>
#include "gok-glade-helpers.h"
#include "gok-gconf-keys.h"
#include "gok-log.h"

gchar *build_glade_xml_path();


/**
* gok_glade_xml_new 
*
* @file: The name of the glade file to open. 
* @root: The root widget glade should load.
*
* returns: an xml pointer to the glade xml structure that has just
* been made and autoconnected.
**/
GladeXML *gok_glade_xml_new(const char *file, const char *root)
{
	GladeXML *xml;
	gchar *path = NULL;

	path = build_glade_xml_path ();
	xml = glade_xml_new (path, root, NULL);
	
	if (xml == NULL)
	{
		gok_log_x ("Error: Problem opening %s\n", path);
		return NULL;
	}
	g_free(path);
	
	/* connect the signals in the interface */
	glade_xml_signal_autoconnect(xml);
	
	return xml;
}

/**
* build_glade_xml_path
*
* Builds the absolute path to the glade xml file.
*
* returns: a gchar pointer that is the absolute path to 
* the glade xml file. This pointer must be freed.
**/
gchar *build_glade_xml_path ()
{
    GConfClient *gconf_client = NULL;
    GError *gconf_err = NULL;
    gchar *directory_name = NULL;
    gchar *complete_path = NULL;

    gconf_client = gconf_client_get_default ();
    
    directory_name = gconf_client_get_string (gconf_client,
					      GOK_GCONF_RESOURCE_DIRECTORY,
					      &gconf_err);
    if (directory_name == NULL)
    {
	gok_log_x ("Got NULL resource directory key from GConf");
    }
    else if (gconf_err != NULL)
    {
	gok_log_x ("Error getting resource directory key from GConf");
	g_error_free (gconf_err);
    }
    else
    {
        complete_path = g_build_filename (directory_name, "glade", "gok.glade2", NULL);
	g_free (directory_name);
	return complete_path;
    }
}
