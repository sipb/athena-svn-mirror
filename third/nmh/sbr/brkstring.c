
/*
 * brkstring.c -- (destructively) split a string into
 *             -- an array of substrings
 *
 * $Id: brkstring.c,v 1.1.1.1 1999-02-07 18:14:07 danw Exp $
 */

#include <h/mh.h>

/* allocate this number of pointers at a time */
#define NUMBROKEN 256

static char **broken = NULL;	/* array of substring start addresses */
static int len = 0;		/* current size of "broken"           */

/*
 * static prototypes
 */
static int brkany (char, char *);


char **
brkstring (char *str, char *brksep, char *brkterm)
{
    int i;
    char c, *s;

    /* allocate initial space for pointers on first call */
    if (!broken) {
	len = NUMBROKEN;
	if (!(broken = (char **) malloc ((size_t) (len * sizeof(*broken)))))
	    adios (NULL, "unable to malloc array in brkstring");
    }

    /*
     * scan string, replacing separators with zeroes
     * and enter start addresses in "broken".
     */
    s = str;

    for (i = 0;; i++) {

	/* enlarge pointer array, if necessary */
	if (i >= len) {
	    len += NUMBROKEN;
	    if (!(broken = realloc (broken, (size_t) (len * sizeof(*broken)))))
		adios (NULL, "unable to realloc array in brkstring");
	}

	while (brkany (c = *s, brksep))
	    *s++ = '\0';

	/*
	 * we are either at the end of the string, or the
	 * terminator found has been found, so finish up.
	 */
	if (!c || brkany (c, brkterm)) {
	    *s = '\0';
	    broken[i] = NULL;
	    return broken;
	}

	/* set next start addr */
	broken[i] = s;

	while ((c = *++s) && !brkany (c, brksep) && !brkany (c, brkterm))
	    ;	/* empty body */
    }

    return broken;	/* NOT REACHED */
}


/*
 * If the character is in the string,
 * return 1, else return 0.
 */

static int
brkany (char c, char *str)
{
    char *s;

    if (str) {
	for (s = str; *s; s++)
	    if (c == *s)
		return 1;
    }
    return 0;
}
