
/*
 * print_help.c -- print a help message, and possibly the
 *              -- profile/context entries for this command
 *
 * $Id: print_help.c,v 1.1.1.1 1999-02-07 18:14:09 danw Exp $
 */

#include <h/mh.h>


void
print_help (char *str, struct swit *swp, int print_context)
{
    char *s;

    /* print Usage string */
    printf ("Usage: %s\n", str);

    /* print all the switches */
    printf ("  switches are:\n");
    print_sw (ALL, swp, "-");

    /*
     * check if we should print any profile entries
     */
    if (print_context && (s = context_find (invo_name)))
	printf ("\nProfile: %s\n", s);
}