
/*
 * vfgets.c -- virtual fgets
 *
 * $Id: vfgets.c,v 1.1.1.1 1999-02-07 18:14:10 danw Exp $
 */

#include <h/mh.h>

#define	QUOTE	'\\'


int
vfgets (FILE *in, char **bp)
{
    int toggle;
    char *cp, *dp, *ep, *fp;
    static int len = 0;
    static char *pp = NULL;

    if (pp == NULL)
	if (!(pp = malloc ((size_t) (len = BUFSIZ))))
	    adios (NULL, "unable to allocate string storage");

    for (ep = (cp = pp) + len - 1;;) {
	if (fgets (cp, ep - cp + 1, in) == NULL) {
	    if (cp != pp) {
		*bp = pp;
		return 0;
	    }
	    return (ferror (in) && !feof (in) ? -1 : 1);
	}

	if ((dp = cp + strlen (cp) - 2) < cp || *dp != QUOTE) {
wrong_guess:
	    if (cp > ++dp)
		adios (NULL, "vfgets() botch -- you lose big");
	    if (*dp == '\n') {
		*bp = pp;
		return 0;
	    } else {
		cp = ++dp;
	    }
	} else {
	    for (fp = dp - 1, toggle = 0; fp >= cp; fp--) {
		if (*fp != QUOTE)
		    break;
		else
		    toggle = !toggle;
	    }
	    if (toggle)
		goto wrong_guess;

	    if (*++dp == '\n') {
		*--dp = 0;
		cp = dp;
	    } else {
		cp = ++dp;
	    }
	}

	if (cp >= ep) {
	    int curlen = cp - pp;

	    if (!(dp = realloc (pp, (size_t) (len += BUFSIZ)))) {
		adios (NULL, "unable to allocate string storage");
	    } else {
		cp = dp + curlen;
		ep = (pp = dp) + len - 1;
	    }
	}
    }
}
