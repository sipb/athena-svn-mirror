
/*
 * seq_print.c -- Routines to print sequence information.
 *
 * $Id: seq_print.c,v 1.1.1.1 1999-02-07 18:14:10 danw Exp $
 */

#include <h/mh.h>

#define empty(s) ((s) ? (s) : "")

/*
 * Print all the sequences in a folder
 */
void
seq_printall (struct msgs *mp)
{
    int i;
    char *list;

    for (i = 0; mp->msgattrs[i]; i++) {
	list = seq_list (mp, mp->msgattrs[i]);
	printf ("%s%s: %s\n", mp->msgattrs[i],
	    is_seq_private (mp, i) ? " (private)" : "", empty(list));
    }
}


/*
 * Print a particular sequence in a folder
 */
void
seq_print (struct msgs *mp, char *seqname)
{
    int i;
    char *list;

    /* get the index of sequence */
    i = seq_getnum (mp, seqname);

    /* get sequence information */
    list = seq_list (mp, seqname);

    printf ("%s%s: %s\n", seqname,
	(i == -1) ? "" : is_seq_private(mp, i) ? " (private)" : "", empty(list));
}