
/*
 * gans.c -- get an answer from the user
 *
 * $Id: gans.c,v 1.1.1.1 1999-02-07 18:14:08 danw Exp $
 */

#include <h/mh.h>


int
gans (char *prompt, struct swit *ansp)
{
    register int i;
    register char *cp;
    register struct swit *ap;
    char ansbuf[BUFSIZ];

    for (;;) {
	printf ("%s", prompt);
	fflush (stdout);
	cp = ansbuf;
	while ((i = getchar ()) != '\n') {
	    if (i == EOF)
		return 0;
	    if (cp < &ansbuf[sizeof ansbuf - 1]) {
#ifdef LOCALE
		i = (isalpha(i) && isupper(i)) ? tolower(i) : i;
#else
		if (i >= 'A' && i <= 'Z')
		    i += 'a' - 'A';
#endif
		*cp++ = i;
	    }
	}
	*cp = '\0';
	if (ansbuf[0] == '?' || cp == ansbuf) {
	    printf ("Options are:\n");
	    for (ap = ansp; ap->sw; ap++)
		printf ("  %s\n", ap->sw);
	    continue;
	}
	if ((i = smatch (ansbuf, ansp)) < 0) {
	    printf ("%s: %s.\n", ansbuf, i == -1 ? "unknown" : "ambiguous");
	    continue;
	}
	return i;
    }
}