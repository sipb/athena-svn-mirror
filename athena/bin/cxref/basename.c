/*
** basename.c
**
** return the last portion of a path name.
** included by all the cxref component programs.
**
** Arnold Robbins, Information and Computer Science, Georgia Tech
**	gatech!arnold
** Copyright (c) 1984 by Arnold Robbins.
** All rights reserved.
** This program may not be sold, but may be distributed
** provided this header is included.
*/

#include <string.h>

char *basename(char *str);

char *basename(char *str)
{
	char *cp;

	if((cp = strrchr(str, '/')) == NULL)
		return(str);
	else
		return(++cp);
}
