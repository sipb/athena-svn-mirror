
/*
 * seq_bits.c -- return the snprintb() string for a sequence
 *
 * $Id: seq_bits.c,v 1.1.1.1 1999-02-07 18:14:10 danw Exp $
 */

#include <h/mh.h>


char *
seq_bits (struct msgs *mp)
{
    int i;
    size_t len;
    static char buffer[BUFSIZ];

    strncpy (buffer, MBITS, sizeof(buffer));

    for (i = 0; mp->msgattrs[i]; i++) {
	len = strlen (buffer);
	snprintf (buffer + len, sizeof(buffer) - len,
		"%c%s", FFATTRSLOT + 1 + i, mp->msgattrs[i]);
    }

    return buffer;
}
