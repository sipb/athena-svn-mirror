/*
 * This is for A/UX.  It is a wrapper around the C library regex functions.
 *
 * $Id: emul_re.c,v 1.2 1991-07-06 14:56:34 probe Exp $
 */

#ifdef _AUX_SOURCE

static char *re;
int Error;

char *re_comp(s)
    char *s;
{
    if (!s)
	return 0;
    if (re)
	free(re);

    if (!(re = regcmp(s, (char *)0)))
	return "Bad argument to re_comp";

    return 0;
}

int re_exec(s)
    char *s;
{
    return regex(re, s) != 0;
}

#endif
