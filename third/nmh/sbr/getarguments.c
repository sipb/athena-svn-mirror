
/*
 * getarguments.c -- Get the argument vector ready to go.
 *
 * $Id: getarguments.c,v 1.1.1.1 1999-02-07 18:14:08 danw Exp $
 */

#include <h/mh.h>

char **
getarguments (char *invo_name, int argc, char **argv, int check_context)
{
    char *cp, **ap, **bp, **arguments;
    int n = 0;

    /*
     * Check if profile/context specifies any arguments
     */
    if (check_context && (cp = context_find (invo_name))) {
	cp = getcpy (cp);		/* make copy    */
	ap = brkstring (cp, " ", "\n");	/* split string */

	/* Count number of arguments split */
	bp = ap;
	while (*bp++)
	    n++;
    }

    if (!(arguments = (char **) malloc ((argc + n) * sizeof(*arguments))))
	adios (NULL, "unable to malloc argument storage");
    bp = arguments;

    /* Copy any arguments from profile/context */
    if (n > 0) {
	while (*ap)
	    *bp++ = *ap++;
     }

    /* Copy arguments from command line */
    argv++;
    while (*argv)
	*bp++ = *argv++;

    /* Now NULL terminate the array */
    *bp = NULL;

    return arguments;
}
