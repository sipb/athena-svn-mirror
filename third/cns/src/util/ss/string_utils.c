/*
 * Copyright 1987 by MIT Student Information Processing Board
 *
 * For copyright information, see mit-sipb-copyright.h.
 */
#include "mit-sipb-copyright.h"
#include <strings.h>

char *malloc();
extern long gensym_n;

char *
gensym(name)
	char *name;
{
	char *symbol;

	symbol = malloc((strlen(name)+6) * sizeof(char));
	gensym_n++;
	sprintf(symbol, "%s%05ld", name, gensym_n);
	return(symbol);
}

/* concatenate three strings and return the result */
char *str_concat3(a, b, c)
	register char *a, *b, *c;
{
	char *result;
	int size_a = strlen(a);
	int size_b = strlen(b);
	int size_c = strlen(c);

	result = malloc((size_a + size_b + size_c + 2)*sizeof(char));
	strcpy(result, a);
	strcpy(&result[size_a], c);
	strcpy(&result[size_a+size_c], b);
	return(result);
}

/* return copy of string enclosed in double-quotes */
char *quote(string)
	register char *string;
{
	register char *result;
	int len;
	len = strlen(string)+1;
	result = malloc(len+2);
	result[0] = '"';
	bcopy(string, &result[1], len-1);
	result[len] = '"';
	result[len+1] = '\0';
	return(result);
}

/* make duplicate of string and return pointer */
char *ds(s)
	register char *s;
{
	register int len = strlen(s) + 1;
	register char *new;
	new = malloc(len);
	bcopy(s, new, len);
	return(new);
}
