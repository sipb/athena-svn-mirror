/*
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 *
 *      Modified for jwgc by Daniel Henninger.
 */

#include "mit-copyright.h"

#include "character_class.h"

/*
 * It may look like we are passing the cache by value, but since it's really
 * an array we are passing by reference.  C strikes again....
 */

static character_class cache;

/* character_class */
char *
string_to_character_class(str)
	string str;
{
	int i;

	(void) memset(cache, 0, sizeof(cache));

	for (i = 0; i < strlen(str); i++)
		cache[(int) str[i]] = 1;

	return (cache);
}
