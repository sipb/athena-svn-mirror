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
#include <stdlib.h>
#include <scrollkeeper.h>
#include <string.h>
#include <locale.h>

/* this routine returns a char** pointer with an array of
    dynamically allocated strings, the last string being NULL;
    if C is not in the list, it will be added to the end;
    the array and the strings themselves should be freed by the caller
    when no longer needed
*/
char **sk_get_language_list()
{
	char *lang, *str, *token, sep[2];
	int count, i;
	char **tab;
	int c_found;
	
	lang = getenv("LANGUAGE");
	if (lang == NULL || lang[0] == '\0') {
		lang = setlocale(LC_MESSAGES, NULL);
	}
		
	if (lang == NULL || lang[0] == '\0') {
		return NULL;
	}
	
	sep[0] = ':';
	sep[1] = '\0';
		
	str = strdup(lang);
	check_ptr(str, "");
	
	count = 0;
	c_found = 0;
	token = strtok(str, sep);
	while (token != NULL) {
		if (!strcmp(token, "C")) {
			c_found = 1;	
		}

		count++;
		token = strtok(NULL, sep);
	}
	
	free(str);
	
	if (!c_found) {
		count++;
	}
	
	tab = (char **)malloc(sizeof(char *) * (count + 1));
	
	str = strdup(lang);
	check_ptr(str, "");
		
	i = 0;
	
	token = strtok(str, sep);
	while (token != NULL) {
		tab[i] = strdup(token);
		check_ptr(tab[i], "");
		i++;
		token = strtok(NULL, sep);
	}
	
	if (!c_found) {
		tab[i] = strdup("C");
		check_ptr(tab[i], "");
		i++;
	}
	
	tab[i] = NULL;
	
	free(str);
	
	return tab;	
}
