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
#include <scrollkeeper.h>
#include <string.h>

/* the returned value must not be modified outside this routine
*/
static xmlChar *get_doc_uid(xmlNodePtr doc_node)
{
	xmlNodePtr node;
	
	for(node = doc_node->children; node != NULL; node = node->next) {
		if (!xmlStrcmp(node->name, (xmlChar *)"docseriesid") &&
		    node->children != NULL &&
		    node->children->type == XML_TEXT_NODE &&
		    node->children->content != NULL) {
		    return node->children->content;
		}
	}
	
	return NULL;
}

static int find_uid_in_sect(xmlNodePtr sect_node, xmlChar *orig_uid)
{
	xmlNodePtr node;
	xmlChar *uid;
	
	for(node = sect_node->children; node != NULL; node = node->next) {
		if (xmlStrcmp(node->name, (xmlChar *)"doc")) {
			continue;
		}
	
		uid = get_doc_uid(node);
		if (uid != NULL) {
			if (!xmlStrcmp(orig_uid, uid)) {
				return 1;
			}
		}
	}
	
	return 0;
}

static void merge_two_sections(xmlNodePtr sect_node, xmlNodePtr other_sect_node)
{
	xmlNodePtr node, new_node;
	xmlChar *uid;
	
	for(node = other_sect_node->children; node != NULL; node = node->next) {
		if (xmlStrcmp(node->name, (xmlChar *)"doc")) {
			continue;
		}
	
		uid = get_doc_uid(node);
		if (uid != NULL) {
			if (!find_uid_in_sect(sect_node, uid)) {
				new_node = xmlCopyNode(node, 1);
				check_ptr(new_node, "");
				xmlAddChild(sect_node, new_node);
			}
		}
	}
}

static int find_sect_with_code(xmlNodePtr tree, xmlChar *sect_code, 
				xmlNodePtr *sect_return)
{
	xmlNodePtr sect_node;
	xmlChar *code;
	
	for(sect_node = tree; sect_node != NULL; sect_node = sect_node->next) {
		if (!xmlStrcmp(sect_node->name, (xmlChar *)"sect")) {
			code = xmlGetProp(sect_node, (xmlChar *)"categorycode");
			if (code != NULL) {
				if (!xmlStrcmp(sect_code, code)) {
					*sect_return = sect_node;
					xmlFree(code);
					return 1;
				}
				xmlFree(code);
			}
			
			if (find_sect_with_code(sect_node->children, 
						sect_code, sect_return)) {
				return 1;
			}
		}
	}

	return 0;
}

static void merge_sections(xmlNodePtr sect_node, xmlChar *code, 
				xmlDocPtr *tree_tab, int tree_num)
{
	int i;
	xmlNodePtr other_sect_node;
	
	for(i = 0; i < tree_num; i++) {
		if (tree_tab[i] == NULL) {
			continue;
		}
	
		if (find_sect_with_code(tree_tab[i]->children->children, 
					code, &other_sect_node)) {
			merge_two_sections(sect_node, other_sect_node);
		}
	}
}

static void merge_trees(xmlNodePtr tree, xmlDocPtr *tree_tab, int tree_num)
{
	xmlNodePtr sect_node;
	xmlChar *code;
    
    	for(sect_node = tree; sect_node != NULL; sect_node = sect_node->next) {
        	if (!xmlStrcmp(sect_node->name, (xmlChar *)"sect"))
		{
			code = xmlGetProp(sect_node, (xmlChar *)"categorycode");
			if (code != NULL) {
				merge_sections(sect_node, code, tree_tab, tree_num);
				xmlFree(code);
			}
			
			merge_trees(sect_node->children, tree_tab, tree_num);
		}
    	}
}

static xmlDocPtr merge_locale_trees_in_first(xmlDocPtr *tree_tab, int tree_num)
{
	xmlDocPtr new_tree;
	int i;

	if (tree_tab == NULL || tree_num == 0)
		return NULL;
		
	for(i = 0; i < tree_num; i++) {
		if (tree_tab[i] != NULL) {
			break;
		}
	}
	
	if (i == tree_num) {
		return NULL;
	}
		
	new_tree = xmlCopyDoc(tree_tab[i], 1);
	check_ptr(new_tree, "");
	
	if (tree_num > 0) {
		merge_trees(new_tree->children->children, &(tree_tab[i+1]), tree_num-i-1);
	}

	return new_tree;
}

static char *create_content_list_file_path(char *scrollkeeper_dir, char *locale, 
						char *base)
{
	char *str;
		
	str = malloc(sizeof(char)*
			(strlen(scrollkeeper_dir)+1+strlen(locale)+1+strlen(base)+1));	
	check_ptr(str, "");
	
	sprintf(str, "%s/%s/%s", scrollkeeper_dir, locale, base);
	
	return str;
}

xmlDocPtr merge_locale_trees(char *scrollkeeper_dir, char *base_locale, char *basename)
{
	char **lang_tab, *path;
	int i, lang_num, count;
	xmlDocPtr merged_tree, *tree_tab;
	
	lang_tab = sk_get_language_list();
	
	if (lang_tab == NULL) {
		return NULL;
	}
	
	for(i = 0, lang_num = 0; lang_tab[i] != NULL; i++) {
		lang_num++;
	}
	
	tree_tab = (xmlDocPtr *)malloc(sizeof(xmlDocPtr)*(lang_num+1));
	
	path = create_content_list_file_path(scrollkeeper_dir,
				base_locale, basename);
	tree_tab[0] = xmlParseFile(path);
	free(path);
	
	for(i = 0, count = 1; i < lang_num; i++) {
		if (!strcmp(base_locale, lang_tab[i])) {
			continue;
		}
		
		path = create_content_list_file_path(scrollkeeper_dir,
				lang_tab[i], basename);
		tree_tab[count] = xmlParseFile(path);
		free(path);
		count++;
	}
 
	merged_tree = merge_locale_trees_in_first(tree_tab, count);
	
	for(i = 0; lang_tab[i] != NULL; i++) {
		free(lang_tab[i]);
	}
	
	for(i = 0; i < count; i++) {
		if (tree_tab[i] != NULL) {
			xmlFreeDoc(tree_tab[i]);
		}
	}
	
	free(lang_tab);
	free(tree_tab);
	
	return merged_tree;
}
