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

/* Mask components used in locale spec. Ordering is from least significant to
 * most significant.
 */
enum {
	CODESET = 1 << 0,
	TERRITORY = 1 << 1,
	MODIFIER = 1 << 2
};

/*
 * The explode_locale and compute_locale_variants functions are variants on the
 * functions of the same names from libgnome. Consequently, they are
 *    Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation
 *    All rights reserved.
 */

/* Support function for compute_locale_variants. */
static int explode_locale(const char *locale, char **language,
		char **territory, char **codeset, char **modifier)
{
	const char *uscore_pos;
	const char *dot_pos;
	const char *at_pos;
	int mask = 0;

	uscore_pos = strchr(locale, '_');
	dot_pos = strchr(uscore_pos ? uscore_pos : locale, '.');
	at_pos = strchr(dot_pos ? dot_pos : (uscore_pos ? uscore_pos : locale), '@');

	if (at_pos) {
		mask |= MODIFIER;
		*modifier = strdup(at_pos);
		check_ptr(modifier, "");
	} else {
		at_pos = locale + strlen (locale);
		*modifier = strdup("");
	}

	if (dot_pos) {
		mask |= CODESET;
		*codeset = (char*) malloc(at_pos - dot_pos + 1);
		strncpy(*codeset, dot_pos, at_pos - dot_pos);
		(*codeset)[at_pos - dot_pos] = '\0';
	} else {
		dot_pos = at_pos;
		*codeset = strdup("");
	}

	if (uscore_pos) {
		mask |= TERRITORY;
		*territory = (char*) malloc(dot_pos - uscore_pos + 1);
		strncpy(*territory, uscore_pos, dot_pos - uscore_pos);
		(*territory)[dot_pos - uscore_pos] = '\0';
	} else {
		uscore_pos = dot_pos;
		*territory = strdup("");
	}

	*language = (char*) malloc(uscore_pos - locale + 1);
	strncpy(*language, locale, uscore_pos - locale);
	(*language)[uscore_pos - locale] = '\0';

	return mask;
}

/* Compute variants of the given locale name and return an array with the
 * choices ordered from 'most interesting' to 'least interesting'.  For
 * example, passing a locale of 'sv_SE.ISO8859-1' will return the list:
 * sv_SE.ISO8859-1, sv_SE and sv (in that order).
 *
 * It is assumed that the locale is in the X/Open format:
 * 	language[_territory][.codeset][@modifier]
 *
 * Returns a NULL-terminated array of strings.
 */
static char **compute_locale_variants(const char *locale)
{
	char **progress, **retval, *language, *territory, *codeset, *modifier;
	int mask, i, pos, count = 0;

	if (locale == NULL) {
		return NULL;
	}

	mask = explode_locale(locale, &language, &territory, &codeset,
			&modifier);
	progress = (char **) malloc(sizeof(char *) * (mask + 1));
	check_ptr(progress, "");

	/* Iterate through all possible combinations, from most attractive
	 * to least attractive.
	 */
	for (i = mask; i >= 0; i--) {
		if ((i & ~mask) == 0) {
			int length = strlen(language) + strlen(territory) 
				+ strlen(codeset) + strlen(modifier);
			char *var = (char *) malloc(sizeof(char) * length);
			check_ptr(var, "");

			strcpy(var, language);
			if (i & TERRITORY) {
				strcat(var, territory);
			}
			if (i & CODESET) {
				strcat(var, codeset);
			}
			if (i & MODIFIER) {
				strcat(var, modifier);
			}
			progress[mask - i] = var;
			++count;
		} else {
			progress[mask - i] = NULL;
		}
	}

	/* Return only the non-nul elements from 'progress'. */
	retval = (char **) malloc(sizeof(char *) * (count + 1));
	check_ptr(retval, "");
	pos = 0;
	for (i = 0; i <= mask; i++) {
		if (progress[i] != NULL) {
			retval[pos] = progress[i];
			++pos;
		}
	}
	retval[count] = NULL;

	free(progress);
	free(language);
	free(codeset);
	free(territory);
	free(modifier);

	return retval;

}

/* this routine returns a char** pointer with an array of
    dynamically allocated strings, the last string being NULL;
    if C is not in the list, it will be added to the end;
    the array and the strings themselves should be freed by the caller
    when no longer needed
*/
char **sk_get_language_list()
{
	char *lang, *str, *token, sep[2];
	int count, total, pos, i, j, k;
	char ***tab, **retval;
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
	
	tab = (char ***)malloc(sizeof(char **) * count);
	
	str = strdup(lang);
	check_ptr(str, "");
		
	i = 0;
	total = 0;

	token = strtok(str, sep);
	while (token != NULL) {
		char **variants = compute_locale_variants(token);
		j = 0;
		while (variants[j] != NULL) {
			++total;
			++j;
		}
		tab[i] = variants;
		++i;
		token = strtok(NULL, sep);
	}

	if (!c_found) {
		tab[i] = (char **) malloc(sizeof(char *) * 2);
		check_ptr(tab[i], "");
		tab[i][0] = strdup("C");
		tab[i][1] = NULL;
		i++;
		total++;
	}
	
	tab[i] = NULL;
	
	/* Flatten the 'tab' array of arrays into an array of strings. */
	retval = (char **) malloc(sizeof(char *) * (total + 1));
	check_ptr(retval, "");
	pos = 0;
	j = 0;
	while (tab[j] != NULL) {
		k = 0;
		while (tab[j][k] != NULL) {
			retval[pos] = tab[j][k];
			++pos;
			++k;
		}
		free(tab[j]);
		++j;
	}
	free(tab);
	retval[pos] = NULL;

	free(str);
	
	return retval;	
}
