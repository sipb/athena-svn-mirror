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
#include <locale.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <libintl.h>
#include <scrollkeeper.h>

typedef struct {
	char *filename;
	char *locale;
} LocaleTree;

/* check the first set of titles and create one output file for each locale;
   the file structure is:
   
   <ScrollKeeperContentsList>
   <sect>
    <title>...</title>
    <title xml:lang="hu">...</title>
    <title xml:lang="de">...</title>
    <title xml:lang="da">...</title>
   </sect>
   ...
   </ScrollkeeperContentsList>
   
   so we go down to the first <sect> node's children
*/

static int create_locale_files(xmlDocPtr main_tree, char *output_dir,
				LocaleTree **locale_tab, int *locale_num)
{
	xmlNodePtr node, start;
	char *locale, *name;
	int count, i, j;
	LocaleTree *tab;
	FILE *fid;
	
	if (main_tree == 0 || main_tree->children == NULL ||
	    main_tree->children->children == NULL) {
		printf(_("Invalid category file.\n"));
	    	return 0;
	}
	
	for(node = main_tree->children->children; 
	    node != NULL; node = node->next) {
		if (!xmlStrcmp(node->name, (xmlChar *)("sect"))) {
			break;
		}
	}
	
	if (node == NULL || node->children == NULL) {
		printf(_("Invalid category file.\n"));
		return 0;
	}
	
	start = node->children;
	
	for(node = start, count = 0; node != NULL; node = node->next) {
		if (!xmlStrcmp(node->name, (xmlChar *)("title"))) {
			count++;
		}
	}
	
	if (count == 0) {
		printf(_("Invalid category file.\n"));
		return 0;
	}
	
	tab = (LocaleTree *)calloc(count, sizeof(LocaleTree));
	check_ptr(tab, "scrollkeeper-tree-separate");
	
	for(node = start, i = 0; node != NULL; node = node->next) {
		if (!xmlStrcmp(node->name, (xmlChar *)("title"))) {
			locale = (char *)xmlGetNsProp(node, (xmlChar *)"lang", XML_XML_NAMESPACE);
			if (locale == NULL) {
				locale = "C";
			}
			tab[i].locale = locale;
			name = malloc(strlen(output_dir)+1+
				strlen(locale)+1+strlen("scrollkeeper_cl.xml")+1);
			check_ptr(name, "scrollkeeper-tree-separate");
			sprintf(name, "%s/%s", output_dir, locale);
			if (mkdir(name, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH|
					S_IXUSR|S_IXGRP|S_IXOTH) != 0) {
				if (errno != EEXIST) {
					printf("Could not create directory: %s\n",name);
					free(name);
					for(j = 0; j < i; j++) {
						free(tab[j].filename);
					}			
					free(tab);
					return 0;
				}
			}
			strcat(name, "/scrollkeeper_cl.xml");
			fid = fopen(name, "w");
			if (fid == NULL) {
				printf("Could not create file: %s\n",name);
				free(name);
				for(j = 0; j < i; j++) {
						free(tab[j].filename);
					}	
				free(tab);
				return 0;
			}
			fclose(fid);
			tab[i].filename = name;
			i++;
		}
	}

	*locale_tab = tab;
	*locale_num = count;

	return 1;
}

static int whitespace_only(xmlChar *str)
{
	do {
        	if (*str != '\n' && *str != ' ' && 
		    *str != '\t'&& *str != '\0') {
			printf("F\n");
			return 0;
		}
		
    } while (*str++ != 0);
    
    return 1;
}

static xmlChar *generate_uid(xmlChar ***path_tab)
{
	int i, len;
	xmlChar *result;
	
	if (path_tab == NULL || ((*path_tab)[0]) == NULL) {
		printf("G\n");
		return NULL;
	}
	
	for(i = 0, len = 0; ((*path_tab)[i]) != NULL; i++) {
		len += xmlStrlen((*path_tab)[i]);
	}
	
	len += 1;
	
	result = NULL;
		
	for(i = 0; ((*path_tab)[i]) != NULL; i++) {
		result = xmlStrcat(result, (*path_tab)[i]);
	}
				
	return result;
}

static void trim_obsolete_locales(xmlNodePtr tree_node, char *locale, xmlChar ***path_tab)
{
	xmlNodePtr node, next, w_node;
	char *lang;
	xmlChar *title;
	int i;
	xmlChar *id;
	xmlNsPtr ns;
		
	title = NULL;
		
	for(node = tree_node; node != NULL; node = node->next) {
		if (!xmlStrcmp(node->name, (xmlChar *)("title"))) {
			if (xmlGetNsProp(node, (xmlChar *)"lang", XML_XML_NAMESPACE) == NULL &&
			    node->children != NULL &&
			    node->children->content != NULL) {
				title = xmlStrdup(node->children->content);
				break;		
			}
		}
	}
	
	i = 0;
	if (title != NULL) {
		if (*path_tab == NULL) {
			i = 1;
			*path_tab = (xmlChar **)malloc(2*sizeof(xmlChar *));
			check_ptr(*path_tab, "scrollkeeper-tree-separate");
			(*path_tab)[0] = title;
			(*path_tab)[1] = NULL;
		}
		else {
			for(i = 0; ((*path_tab)[i]) != NULL; i++)
				;
				
			*path_tab = (xmlChar **)realloc(*path_tab, (i+2)*sizeof(xmlChar *));
			check_ptr(*path_tab, "scrollkeeper-tree-separate");
			(*path_tab)[i] = title;
			(*path_tab)[i+1] = NULL;
			i++;
		}
	}
	
	next = NULL;

	for(node = tree_node; node != NULL; node = next) {
		next = node->next;
	
		if (!xmlStrcmp(node->name, (xmlChar *)("title"))) {
			lang = (char *)xmlGetNsProp(node, (xmlChar *)"lang", XML_XML_NAMESPACE);
			if (lang == NULL) {
				lang = "C";
			}
			if (strcmp(lang, locale)) {
				xmlUnlinkNode(node);
				xmlFreeNode(node);
				w_node = next;
				if (w_node != NULL &&
				    w_node->type == XML_TEXT_NODE &&
				    !xmlStrcmp(w_node->name, (xmlChar *)("text")) &&
				    whitespace_only(w_node->content)) {
				    	next = w_node->next;
					xmlUnlinkNode(w_node);
					xmlFreeNode(w_node);
				}
				continue;
			}
			else {
				if (strcmp(lang, "C")) {
					ns = xmlSearchNsByHref(node->doc, node, XML_XML_NAMESPACE);
					if (ns != NULL) {
						xmlUnsetNsProp(node, ns, (xmlChar *)"lang");
					}
				}
				
				id = generate_uid(path_tab);
				check_ptr(id, "scrollkeeper-tree-separate");
				xmlNewProp(node->parent, (xmlChar *)("categorycode"), id);
				xmlFree(id);
			}
		}
		 
		if (!xmlStrcmp(node->name, (xmlChar *)("sect"))) {
			trim_obsolete_locales(node->children, locale, path_tab);
		}
	}
	
	if (i == 0) {
		return;
	}
	
	if (i == 1) {
		xmlFree((*path_tab)[0]);
		free(*path_tab);
		*path_tab = NULL;
	}
	else {
		xmlFree((*path_tab)[i-1]);
		(*path_tab)[i-1] = NULL;
	}
}

static xmlDocPtr create_locale_tree(xmlDocPtr main_tree, char *locale)
{
	xmlDocPtr locale_tree;
	xmlChar **path_tab;
	
	locale_tree = xmlCopyDoc(main_tree, 1);
	check_ptr(locale_tree, "scrollkeeper-tree-separate");

	path_tab = NULL;
	trim_obsolete_locales(locale_tree->children->children, locale, &path_tab);

	return locale_tree;
}

static void usage()
{
    	printf(_("Usage: scrollkeeper-tree-separate <tree file> <output directory>\n"));
}

int
main (int argc, char *argv[])
{
	xmlDocPtr main_tree, locale_tree;
	LocaleTree *locale_tab;
	int locale_num, i;
	int locale_tree_error=0;

    	setlocale (LC_ALL, "");
    	bindtextdomain (PACKAGE, SCROLLKEEPERLOCALEDIR);
    	textdomain (PACKAGE);
    
    	if (argc != 3) {
    		usage();
    		exit(EXIT_FAILURE);
	}
	
	main_tree = xmlParseFile(argv[1]);
	if (main_tree == NULL) {
		printf(_("File does not contain well-formed XML\n"));
		exit(EXIT_FAILURE);
	}
	
	if (!create_locale_files(main_tree, argv[2], &locale_tab, &locale_num)) {
		printf(_("tree separation failed\n"));
		xmlFreeDoc(main_tree);
		exit(EXIT_FAILURE);
	}
	
	for(i = 0; i < locale_num; i++) {
		locale_tree = create_locale_tree(main_tree, locale_tab[i].locale);
		if (locale_tree == NULL) {
			printf(_("Unable to create localized category file: %s\n"),locale_tab[i].filename);
			locale_tree_error=1;
		}
		else {
			printf(_("Creating localized category file: %s\n"),locale_tab[i].filename);
			xmlSaveFile(locale_tab[i].filename, locale_tree);
		}
	}
	
	for(i = 0; i < locale_num; i++) {
		free(locale_tab[i].filename);
	}
	free(locale_tab);
	
	if (locale_tree_error == 1) {
		printf( _("Unable to create all localized category files.\n"));
	}	
	
	xmlFreeDoc(main_tree);

    	return 0;
}
