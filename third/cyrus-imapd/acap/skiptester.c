/* $Id: skiptester.c,v 1.1.1.1 2002-10-13 18:01:16 ghudson Exp $ */

#include <stdio.h>
#include <string.h>

#include "skip-list.h"

int cf(const void *v1, const void *v2)
{
    return strcmp((const char *) v1, (const char *) v2);
}

void printone(const void *v)
{
    printf("%s ", (const char *) v);
}

int main(void)
{
    skiplist *S;
    char buf[8192];

    S = skiplist_new(10, 0.5, &cf);
    for (;;) {
	char *p;

	if (!fgets(buf, sizeof(buf), stdin)) {
	    break;
	}
	p = strchr(buf, ' ');
	if (p) *p = '\0';
	p = strchr(buf, '\n');
	if (p) *p = '\0';
	switch (buf[0]) {
	case '-':
	    sdelete(S, buf + 1);
	    break;
	case '\0':
	    printf("noop\n");
	    break;
	default:
	    sinsert(S, strdup(buf));
	    break;
	}

	/* print out the list */
	printf(": ");
	sforeach(S, &printone);
	printf("\n");
    }

    printf("final %d: ", skiplist_items(S));
    sforeach(S, &printone);
    printf("\n");
}
