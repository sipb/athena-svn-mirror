/* copyright (C) 2000 Sun Microsystems, Inc.*/

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
#include <locale.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <libintl.h>
#include <dirent.h>
#include <scrollkeeper.h>

#define _(String) gettext (String)

#define SEP		"|"
#define PATHLEN		256

#ifdef SOLARIS
/*extern char *strtok_r(char *, const char *, char **);*/
#endif

static int get_unique_doc_id(char *);
static void add_doc_to_scrollkeeper_docs(char *, char *, char *, int, char *);
static void add_doc_to_content_list(xmlNodePtr, char *, char **, char *, char *,
				    char *, char *, char *, int, int, char, char **);
static char *get_doc_property(xmlNodePtr, char *, char *);
static char *get_doc_parameter_value(xmlNodePtr, char *);
static char* remove_leading_and_trailing_white_spaces(char *);

static int get_best_locale_dir(char *locale_dir, char *locale_name, 
				char *scrollkeeper_dir, char *locale)
{
    char *loc, *dest_dir, *ptr;
        
    dest_dir = malloc (strlen (scrollkeeper_dir) + strlen (locale) + 2);
    check_ptr(dest_dir, "scrollkeeper-install");
    snprintf(dest_dir, PATHLEN, "%s/%s", scrollkeeper_dir, locale);
    
    if (is_dir(dest_dir))
    {
        strncpy(locale_dir, dest_dir, PATHLEN);
	strncpy(locale_name, locale, PATHLEN);
	free(dest_dir);
	return 1;
    }
    
    loc = strdup(locale);
    check_ptr(loc, "scrollkeeper-install");

    ptr = strrchr(loc, '.');
    if (ptr != NULL)
    {
        *ptr = '\0';
	snprintf(dest_dir, PATHLEN, "%s/%s", scrollkeeper_dir, loc);
	if (is_dir(dest_dir))
    	{
            strncpy(locale_dir, dest_dir, PATHLEN);
	    strncpy(locale_name, loc, PATHLEN);
	    free(dest_dir);
	    free(loc);
	    return 1;
    	}
    } 
    
    ptr = strrchr(loc, '_');
    if (ptr != NULL)
    {
        *ptr = '\0';
	snprintf(dest_dir, PATHLEN, "%s/%s", scrollkeeper_dir, loc);
	if (is_dir(dest_dir))
    	{
            strncpy(locale_dir, dest_dir, PATHLEN);
	    strncpy(locale_name, loc, PATHLEN);
	    free(dest_dir);
	    free(loc);
	    return 1;
    	}
    } 
    
    free(dest_dir);
    free(loc);
    return 0;
}

static xmlNodePtr create_toc_tree(char *docpath, char outputprefs)
{
    xmlDocPtr toc_doc;
    FILE *config_fid;
    char command[1024], tocpath[PATHLEN];
    xmlNodePtr toc_tree;
    errorSAXFunc xml_error_handler;
    warningSAXFunc xml_warning_handler;
    fatalErrorSAXFunc xml_fatal_error_handler;
    
    snprintf(command, 1024, "scrollkeeper-get-toc-from-docpath %s", docpath);
    config_fid = popen(command, "r");

    if (config_fid == NULL)
	return NULL;

    fscanf(config_fid, "%s", tocpath);
    if (pclose(config_fid))
        return NULL;
   
    xml_error_handler = xmlDefaultSAXHandler.error;
    xmlDefaultSAXHandler.error = NULL;
    xml_warning_handler = xmlDefaultSAXHandler.warning;
    xmlDefaultSAXHandler.warning = NULL;
    xml_fatal_error_handler = xmlDefaultSAXHandler.fatalError;
    xmlDefaultSAXHandler.fatalError = NULL;
    toc_doc = xmlParseFile(tocpath);
    xmlDefaultSAXHandler.error = xml_error_handler;
    xmlDefaultSAXHandler.warning = xml_warning_handler;
    xmlDefaultSAXHandler.fatalError = xml_fatal_error_handler;
    if (toc_doc == NULL)
    {
        sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "(install)", _("TOC file does not exist, is not readable, or is not well-formed XML: %s\n"), tocpath);
        return NULL;
    }
    toc_tree = toc_doc->children;
		      	          
    
    /* XXX this is not documented, but xmlUnlinkNode() does not work for the toplevel node
       while this one works according to the current libxml source
    */
    toc_doc->children = NULL;
    
    xmlFreeDoc(toc_doc);
    return toc_tree;
}


/* 
 * FIXME: This function should be rewritten to provide warnings/errors for all
 * cases in which it doesn't install an OMF entry. Currently it may fail
 *  silently for a variety of reasons. 
 */
int install(char *omf_name, char *scrollkeeper_dir, char *data_dir, char outputprefs)
{    
    xmlDocPtr omf_doc;
    xmlDtdPtr dtd;
    xmlNodePtr node, omf_node, s_node;
    char *docpath, *title, *format, str[1024];
    char cl_filename[PATHLEN], cl_ext_filename[PATHLEN];
    char locale_dir[PATHLEN], locale_name[PATHLEN], *locale, *ptr;
    int unique_id;
    xmlDocPtr cl_doc, cl_ext_doc;
    char scrollkeeper_docs[PATHLEN];
    char **stylesheets=NULL, **output_files=NULL, *toc_name;
    char *toc_stylesheet_name, *index_name, *index_stylesheet_name, *uid;

    /*
     * Read in OMF file
     */
    omf_doc = xmlParseFile(omf_name);
    if (omf_doc == NULL)
    {
        sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "(install)", _("OMF file does not exist, is not readable, or is not well-formed XML: %s\n"), omf_name);
        return 0;
    }

    /*
     * Validate OMF file against ScrollKeeper-OMF DTD
     */
    dtd = xmlParseDTD(NULL, (const xmlChar *)SCROLLKEEPER_OMF_DTD);
    if (dtd == NULL) {
        sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "(install)", _("Could not parse ScrollKeeper-OMF DTD: %s\n"), SCROLLKEEPER_OMF_DTD); 
        return 0;
    } else {
        xmlValidCtxt cvp;
        cvp.userData = (char *) &outputprefs;
        cvp.error    = (xmlValidityErrorFunc) sk_dtd_validation_message;
        cvp.warning  = (xmlValidityWarningFunc) sk_dtd_validation_message;
        if (!xmlValidateDtd(&cvp, omf_doc, dtd)) {
            sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "(install)", _("OMF file [%s] does not validate against ScrollKeeper-OMF DTD: %s\n"), omf_name, SCROLLKEEPER_OMF_DTD); 
            return 0;
        }
    }
    xmlFreeDtd(dtd);
		      	          
    snprintf(scrollkeeper_docs, PATHLEN, "%s/scrollkeeper_docs", scrollkeeper_dir);
    
    /* We assume that this file is a concatenation of "resource" tags so
     * they should start from the top node's children.
     */

    /* Well, sort of. Let's do the right thing when comments are involved. */
    for(omf_node = omf_doc->children; (omf_node != NULL) && (omf_node->type != XML_ELEMENT_NODE); omf_node = omf_node->next) ;;

    if (!omf_node) {
        /* This should not happen */
        sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "(install)", _("Failed to locate an <OMF> element.\n"));
        return 0;
    }
    if (!omf_node->children) {
        /* This should not happen */
        sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "(install)", _("<OMF> node has no children.\n"));
        return 0;
    }

    for(node = omf_node->children; node != NULL; node = node->next)
    {
        if (!xmlStrcmp(node->name, (xmlChar *)"resource"))
	{
	    /* create full content list path names and read trees */
    	    locale = get_doc_property(node, "language", "code");
	    if (locale == NULL)
		continue;
	    if (!get_best_locale_dir(locale_dir, locale_name, scrollkeeper_dir, locale)) {
		xmlFree(locale);
	        continue;
	    }
	    xmlFree(locale);
    	    snprintf(cl_filename, PATHLEN, "%s/scrollkeeper_cl.xml", locale_dir);
	    snprintf(cl_ext_filename, PATHLEN, "%s/scrollkeeper_extended_cl.xml", locale_dir);
	    
	    if (!is_file(cl_filename))
	        continue;
		
	    if (!is_file(cl_ext_filename))
	        continue;
	    
	    cl_doc = xmlParseFile(cl_filename);
    	    if (cl_doc == NULL)
    	    {
        	sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "(install)", _("Content list file does not exist, is not readable, or is not well-formed XML: %s\n"), cl_filename);
        	continue;
    	    }
	    
	    cl_ext_doc = xmlParseFile(cl_ext_filename);
    	    if (cl_ext_doc == NULL)
    	    {
        	sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "(install)", _("Extended content list file does not exist, is not readable, or is not well-formed XML: %s\n"), cl_ext_filename);
        	continue;
    	    }

	    docpath = get_doc_property(node, "identifier", "url");
	    if (docpath == NULL)
		continue;
	    
	    /* add to scrollkeeper_docs */
	    unique_id = get_unique_doc_id(scrollkeeper_docs);
            add_doc_to_scrollkeeper_docs(scrollkeeper_docs, docpath, omf_name, unique_id,
	    					locale_name);
						
	    format = get_doc_property(node, "format", "mime");
	    if (format && !strcmp(format, "text/xml"))
	    {
	        /* create TOC file and index file */

		if (!strncmp("file:", docpath, 5))
		    ptr = docpath+5;
		else
		    ptr = docpath;
				
		toc_name = malloc((strlen(scrollkeeper_dir)+32)*sizeof(char));
		check_ptr(toc_name, "scrollkeeper-install");
		sprintf(toc_name, "%s/TOC/%d", scrollkeeper_dir, unique_id);
		
		toc_stylesheet_name = malloc(
			(strlen(data_dir)+strlen("/stylesheets/toc.xsl")+1)*sizeof(char));
		check_ptr(toc_stylesheet_name, "scrollkeeper-install");
		sprintf(toc_stylesheet_name, "%s/stylesheets/toc.xsl", data_dir);
		
		index_name = malloc((strlen(scrollkeeper_dir)+32)*sizeof(char));
		check_ptr(index_name, "scrollkeeper-install");
		sprintf(index_name, "%s/index/%d", scrollkeeper_dir, unique_id);
		
		index_stylesheet_name = malloc(
			(strlen(data_dir)+strlen("/stylesheets/index.xsl")+1)*sizeof(char));
		check_ptr(index_stylesheet_name, "scrollkeeper-install");
		sprintf(index_stylesheet_name, "%s/stylesheets/index.xsl", data_dir);
                /* create stylesheets table and output_files table as
                   parameters to apply_stylesheets                     */
       
                stylesheets=(char **)calloc(2, sizeof(char *));
                check_ptr(stylesheets, "scrollkeeper-install");
                output_files=(char **)calloc(2, sizeof(char *));
                check_ptr(output_files, "scrollkeeper-install");

                stylesheets[0] = toc_stylesheet_name;
                stylesheets[1] = index_stylesheet_name;

                output_files[0] = toc_name;
                output_files[1] = index_name;

		apply_stylesheets(ptr, format+5, 2, 
					stylesheets, output_files, outputprefs);
		
		free(toc_name);
		free(toc_stylesheet_name);
		free(index_name);
		free(index_stylesheet_name);
                free(stylesheets);
                free(output_files);
	    }
	    
	    uid = get_doc_property(node, "relation", "seriesid");
	    if (uid == NULL)
		continue;
	    
	    title = get_doc_parameter_value(node, "title");
    	    strncpy(str, title, 1024);
	    title = remove_leading_and_trailing_white_spaces(str);
	    	    
	    /* add the doc to the content list */
	    for(s_node = node->children; s_node != NULL; s_node = s_node->next)
	    {
	        /* look for subject nodes */
	        if (!xmlStrcmp(s_node->name, (xmlChar *)"subject"))
		{
		    char *category, *token, *rest, *complete_cat_token = NULL;

		    category = (char *)xmlGetProp(s_node, (xmlChar *)"category");
		    if (category == NULL)
			continue;
		    token = strtok_r(category, SEP, &rest);
		    add_doc_to_content_list(cl_doc->children->children, token, &rest,
		    docpath, omf_name, title, format, uid, unique_id, 0, outputprefs,
			&complete_cat_token);
		    xmlFree(category);
		    free((void *)complete_cat_token);
		    category = (char *)xmlGetProp(s_node, (xmlChar *)"category");
		    token = strtok_r(category, SEP, &rest);
			complete_cat_token = NULL;
		    add_doc_to_content_list(cl_ext_doc->children->children, token, &rest,
		    docpath, omf_name, title, format, uid, unique_id, 1, outputprefs,
			&complete_cat_token);
		    xmlFree(category);
		    free((void *)complete_cat_token);
		}
	    }
	    
	    xmlSaveFile(cl_filename, cl_doc);
	    xmlFreeDoc(cl_doc);
	    xmlSaveFile(cl_ext_filename, cl_ext_doc);
	    xmlFreeDoc(cl_ext_doc);
        xmlFree(docpath);
	xmlFree(format);
	xmlFree(uid);
	}
    }
    
    return 1;
}

static char* remove_leading_and_trailing_white_spaces(char *str)
{
    int i, len;
    
    len = strlen(str);
    
    for(i = len-1; i >= 0; i--)
    {
        if (str[i] == ' ' || str[i] == '\t' ||
	    str[i] == '\n' || str[i] == '\r')
	    str[i] = '\0';
	else
	    break;
    }
        
    while (*str == ' ' || *str == '\t' ||
	   *str == '\n' || *str == '\r')
	   str++;
	   
    return str;
}

static void add_doc_to_scrollkeeper_docs(char *filename, char *doc_name, char *omf_name, 
						int unique_id, char *locale)
{
    FILE *fid;
    struct stat buf;
    
    fid = fopen(filename, "a");
    if (fid == NULL)
        fid = fopen(filename, "w");
    if (fid == NULL) {
	perror (filename);
	exit (EXIT_FAILURE);
    }
	
    stat(omf_name, &buf);
    
    fprintf(fid, "%s\t%d\t%s\t%ld\t%s\n", omf_name, unique_id, doc_name, buf.st_mtime,
    		locale);
    
    fclose(fid);
}

static int get_unique_doc_id(char *filename)
{
/* TODO implement a method that returns the first unused doc id, rather than incrementing the 
   highest used one
*/

    FILE *fid;
    int id = 1, unique_id = 0;
    
    fid = fopen(filename, "r");
    
    /* this is the first doc added so just return */
    if (fid == NULL)
        return unique_id;

    while (1)
    {
        fscanf(fid, "%*s%d%*s%*d%*s", &id);
	if (feof(fid))
	    break;
	    
	if (id > unique_id)
	    unique_id = id;
    }

   fclose (fid);
   return unique_id + 1;
}

/* do not modify the return value of this routine */
static char *get_doc_property(xmlNodePtr omf_node, char *tag, char *prop)
{
    xmlNodePtr node;

    if (omf_node == NULL)
        return NULL;
    
    for(node = omf_node->children; node != NULL; node = node->next)
    {    
        if (node->type == XML_ELEMENT_NODE &&
	    !xmlStrcmp(node->name, (xmlChar *)tag))
	    return (char *)xmlGetProp(node, (xmlChar *)prop); 
    }
    
    return NULL;
}

/* do not modify the return value of this routine */
static char *get_doc_parameter_value(xmlNodePtr omf_node, char *tag)
{
    xmlNodePtr node;

    if (omf_node == NULL)
        return NULL;

    for(node = omf_node->children; node != NULL; node = node->next)
    {    
        if (node->type == XML_ELEMENT_NODE &&
	    !xmlStrcmp(node->name, (xmlChar *)tag) &&
	    node->children != NULL)
	    return (char *)node->children->content;
    }
    
    return NULL;
}

static xmlNodePtr create_new_doc_node(xmlDocPtr cl_doc, char *docpath, char *omf_name,
					char *title, char *format, char *uid, int id)
{
    char str[32];
    xmlNodePtr node;

    node = xmlNewDocNode(cl_doc, NULL, (xmlChar *)"doc", NULL);
    snprintf(str, 32, "%d", id);
    xmlSetProp(node, (xmlChar *)"docid", (xmlChar *)str);
    xmlNewChild(node, NULL, (xmlChar *)"doctitle", (xmlChar *)title);
    xmlNewChild(node, NULL, (xmlChar *)"docomf", (xmlChar *)omf_name);
    xmlNewChild(node, NULL, (xmlChar *)"docsource", (xmlChar *)docpath);
    xmlNewChild(node, NULL, (xmlChar *)"docformat", (xmlChar *)format);
    if (uid != NULL) {
   	xmlNewChild(node, NULL, (xmlChar *)"docseriesid", (xmlChar *)uid);
    }
    
    return node;
}

static void add_doc_to_content_list(xmlNodePtr sect_node, char *cat_token, char **rest,
				    char *docpath, char *omf_name,
				    char *title, char *format, char *uid,
				    int id, int add_toc, char outputprefs, char **complete_cat_token)
{
    xmlNodePtr node, new_node, s_node;

    if (sect_node == NULL ||
        cat_token == NULL)
        return;

	if (*complete_cat_token == NULL)
		*complete_cat_token = strdup(cat_token);
	else {
		char *ptr;
		ptr = malloc(strlen(*complete_cat_token) + strlen(cat_token) + 2);
		sprintf(ptr, "%s%s", *complete_cat_token, cat_token);
		free(*complete_cat_token);
		*complete_cat_token = ptr;
	}

    /* these should all be <sect> nodes */	    
    for(node = sect_node; node != NULL; node = node->next)
    {
    	if (xmlStrcmp(node->name, (xmlChar *)"sect"))
	    continue;
	
	xmlChar *categorycode;
	
	categorycode = xmlGetProp(node, (xmlChar *)"categorycode");
	if (categorycode == NULL)
	    continue;
	
	/* these should be the actual titles */
	if (!xmlStrcmp((xmlChar *)(*complete_cat_token), categorycode))
	{
	        cat_token = strtok_r(NULL, SEP, rest);
		if (cat_token == NULL)
		{
		    /* we have a match so create a node */
		     
	    	    new_node = create_new_doc_node(node->children->doc, docpath, omf_name, title, 
					format, uid, id);
					
		    if (add_toc)
		    {
		    	xmlNodePtr toc_tree;
		    
		   	toc_tree = create_toc_tree(docpath, outputprefs);
			if (toc_tree != NULL)
		   	    xmlAddChild(new_node, toc_tree);
		    }

	            xmlAddChild(node, new_node);
		    
		    return;
	    	}
		else
		{
		    /* partial match, continue on this branch only if there are
		       any more <sect> nodes
		    */
		    
		    for(s_node = node->children; s_node != NULL; s_node = s_node->next)
		    {
		    	if (s_node->type == XML_ELEMENT_NODE &&
	    		    !xmlStrcmp(s_node->name, (xmlChar *)"sect"))
		    	    break;
		    }
		    
		    if (s_node != NULL)
		        add_doc_to_content_list(s_node, cat_token, rest, 
						docpath, omf_name, title, format,
						uid, id, add_toc, outputprefs, complete_cat_token);
		    return;
		}
	}	
	
	xmlFree(categorycode);
    }
}
