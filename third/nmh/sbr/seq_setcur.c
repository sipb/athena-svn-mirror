
/*
 * seq_setcur.c -- set the current message ("cur" sequence) for a folder
 *
 * $Id: seq_setcur.c,v 1.1.1.1 1999-02-07 18:14:10 danw Exp $
 */

#include <h/mh.h>


void
seq_setcur (struct msgs *mp, int msgnum)
{
    /*
     * Just call seq_addmsg() to update the
     * "cur" sequence.
     */
    seq_addmsg (mp, current, msgnum, -1, 1);
}
