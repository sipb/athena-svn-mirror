/* copyright (C) 2001 Sun Microsystems, Inc.*/

/*    
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <config.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libintl.h>
#include <scrollkeeper.h>

#define _(String) gettext (String)

int update_doc_url_in_omf_file(char *omf_name, char *url, char *omf_new_name)
{
    xmlNodePtr node;
    xmlDocPtr omf_doc;
        

    /* Parse file and make sure it is well-formed */
    omf_doc = xmlParseFile(omf_name);
    if (omf_doc == NULL || omf_doc->children == NULL) {
	printf(_("OMF file was not well-formed.\n"));
        return 0;
    }


    /* Look for root element and make sure it is <omf> */
    node = xmlDocGetRootElement(omf_doc);
    if (node == NULL) {
        printf(_("Could not find root element of OMF file.\n"));
        return 0;
    }
    if (xmlStrcmp(node->name, (xmlChar *)"omf")) {
        printf(_("Root element of OMF file is not <omf>.\n"));
        return 0;
    }


    /* Look for <resource> */    
    for(node = node->children; node != NULL; node = node->next) {
	if (!xmlStrcmp(node->name, (xmlChar *)"resource")) {
		break;
	}
    }
    if (node == NULL) {
        printf(_("OMF file does not have <resource> element.\n"));
    	return 0;
    }    	
    

    /* Modify <url> element */
    for(node = node->children; node != NULL; node = node->next)
    {    
        if (node->type == XML_ELEMENT_NODE &&
	    !xmlStrcmp(node->name, (xmlChar *)"identifier"))
	{
	    xmlSetProp(node, (xmlChar *)"url", (xmlChar *)url);
	    break;
	}
    }
    
    /* Save the modified XML tree to file */
    xmlSaveFile(omf_new_name, omf_doc);
    xmlFreeDoc(omf_doc);

    if (node != NULL) {
   	return 1;
    }
  
    return 0;
}
