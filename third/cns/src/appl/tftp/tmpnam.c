/* tmpnam.c */

/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	"notice.h"


/* Generate a unique temporary name.  Stolen from stdio, but changed to
 * generate shorter variable names so extensions can be added.
 */

char *tmpnam(s)
char *s;
{
	static seed;

	sprintf(s, "t%d.%d", getpid(), seed++);
	return(s);
}
