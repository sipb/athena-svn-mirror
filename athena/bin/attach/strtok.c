#ifdef NEED_STRTOK
/* strtok.c -- return tokens from a string, NULL if no token left */
/* LINTLIBRARY */

/*
 * Get next token from string s1 (NULL on 2nd, 3rd, etc. calls),
 * where tokens are nonempty strings separated by runs of
 * chars from s2.  Writes NULs into s1 to end tokens.  s2 need not
 * remain constant from call to call.
 *
 * Written by reading the System V Interface Definition, not the code.
 *
 * Totally public domain.
 *
 */

#define	NULL	0

char *
strtok(s1, s2)
char *s1;
register char *s2;
{
	register char *scan;
	char *tok;
	register char *scan2;
	static char *scanpoint = (char *)NULL;

	if (s1 == (char *)NULL && scanpoint == (char *)NULL)
		return((char *)NULL);
	if (s1 != (char *)NULL)
		scan = s1;
	else
		scan = scanpoint;

	/*
	 * Scan leading delimiters.
	 */
	for (; *scan != '\0'; scan++) {
		for (scan2 = s2; *scan2 != '\0'; scan2++)
			if (*scan == *scan2)
				break;
		if (*scan2 == '\0')
			break;
	}
	if (*scan == '\0') {
		scanpoint = (char *)NULL;
		return((char *)NULL);
	}

	tok = scan;

	/*
	 * Scan token.
	 */
	for (; *scan != '\0'; scan++) {
		for (scan2 = s2; *scan2 != '\0';)	/* ++ moved down. */
			if (*scan == *scan2++) {
				scanpoint = scan+1;
				*scan = '\0';
				return(tok);
			}
	}

	/*
	 * Reached end of string.
	 */
	scanpoint = (char *)NULL;
	return(tok);
}

/* strtok.c ends here */
#endif /* NEED_STRTOK */
