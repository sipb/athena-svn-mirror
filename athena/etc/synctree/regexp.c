/*
 * Copyright 1986,1989 by Richard P. Basch
 * All Rights Reserved.
 */

/*
 * Regular expression parser
 *
 * Function:
 *    int regexp(pattern, string)
 *
 * Synopsis:
 *    This routine will determine if the "string" matches the "pattern".
 *    The pattern is similar to that of UNIX /bin/csh filename globbing.
 *    The only differences between this and the csh parser are:
 *      - Double negation in ranges is allowed
 *        eg.  [a-z^m-q^n]  would represent any character
 *             between a and z (inclusive), except for the characters
 *             m through q, but still allowing n
 *      - {} has not yet been implemented.
 *
 * Return values:
 *    This routine returns 0 if the string did not match the pattern
 *    and non-zero if the string did match.
 */

#include "regexp.h"

int reg_matchrange(range,testc)
char *range, testc;
{
    /*
     * Arguments:
     *    range = range expression
     *    testc = character to test to see if it within the range
     * 
     * Variables:
     *    found     = was there a match (either negated or normal)
     *    r_start   = beginning of a range
     *    r_end     = end of a range
     *    not       = toggle for whether to negate the expression
     *    reg_chars = non-zero if there are non-negated characters/ranges
     *    c         = temporary character
     *    r_state   = 0 for first char of range, 1 for last char of range
     */
    
    int found=0, not=0, reg_chars=0, r_state = 0, literal = 0;
    unsigned char r_start, r_end, c;

    while(c = *range++) {
	literal = 0;
	
	switch(c) {
	case NOTCHAR:
	case LITERAL:
	    if (c == NOTCHAR && *range) {
		not = (! not);
		break;
	    }
	    if (c == LITERAL && *range) {
		c = *range++;
		literal++;
	    }
	default:
	    reg_chars += !not;

	    if (!r_state) {		/* This is the beginning of a range */
		if (c == RANGETO && !literal && *range) {
		    r_start = r_end =  '\0';
		    range--;
		} else
		    r_start = r_end = c;

		if (*range == RANGETO) {
		    range++;		/* Skip the RANGETO character */

		    switch (*range) {	/* What comes next? */
		    case NOTCHAR:	/* If NOTCHAR or no more characters */
		    case '\0':		/* set the range-end and process    */
			r_end = '\377';
			break;
		    default:		/* Ok, to process range-end normally */
			r_state++;	/* Set the flag */
			break;
		    }

		    if (r_state)
			break;		/* Go ahead and get the range-end */
		}
	    } else
		r_end = c;

	    if (testc >= r_start && testc <= r_end)
		found = not ? -1 : 1;
	    break;
	}
    }
    return((found > 0) || (!reg_chars && !found));
}

int regexp(pattern, string)		/* returns 1 if string fits */
char *pattern, *string;			/* pattern, 0 otherwise.    */
{
    int good = 1;
    char range[128], *temp, c, lastc;

    while(*pattern && good) {
	switch(*pattern) {
	case WILDCHAR:
	    pattern++;
	    good = (*string++ != '\0');
	    break;
	case LITERAL:
	    pattern++;
	    good = (*pattern++ == *string++);
	    break;
	case WILDCARD:
	    pattern++;
	    while ((!regexp(pattern, string)) && *string != '\0')
		string++;
	    break;
	case RANGEBEG:
	    lastc = *pattern++;
	    for (temp = range;
		 (c = *pattern++) && !(c == RANGEEND && lastc != LITERAL);
		 lastc = *temp++ = c) ;
	    *temp = '\0';
	    temp = string++;
	    while (!(good = regexp(pattern, string)) && *string != '\0')
		string++;
	    for ( ; temp<string && good; good=reg_matchrange(range, *temp++)) ;
	    break;
	default:
	    good = (*pattern++ == *string++);
	    break;
	}
    }
    return(good && *pattern == '\0' && *string == '\0');
}




#if 0
char *strindex(string1,string2)		/* find first occurrence */
char *string1, *string2;		/* of string2 in string1 */
{
    char *t1, *t2, *t3;

    for (t1=string1; *t1 != '\0'; t1++) {
	for (t2=string2, t3=t1; *t2 != '\0' && *t2 == *t3; t2++, t3++) ;
	if (*t2 == '\0') return (t1);
    }
    return(NULL);
}

char *bstrindex(string1, string2)	/* Find last occurrence  */
char *string1, *string2;		/* of string2 in string1 */
{
    char *t1, *t2;

    t1 = strindex(string1, string2);
    if (t1 != NULL)
	while ((t2 = strindex(t1+1, string2)) != NULL) t1=t2;
    return(t1);
}

stoupper(string)                        /* Convert string to uppercase */
char *string;
{
   while(*string != NULL)
      *string++ = toupper(*string);
}
#endif


#ifdef TEST
#include <stdio.h>
#include <signal.h>

cleanup()
{
    exit(1);
}
main()
{
    char s1[80], s2[80];
    int s3;

    signal(SIGINT,cleanup);

loop:
    printf("Pattern ==> ");
    if (!gets(s1))
	exit (0);
    if (! strcmp(s1,"quit"))
	exit(0);

    printf("String  ==> ");
    if (!gets(s2))
	exit(0);

    s3 = regexp(s1,s2);
    printf("%s\n\n", s3 ? "Pattern matched" : "Pattern failed");
    goto loop;
}
#endif
