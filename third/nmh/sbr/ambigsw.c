
/*
 * ambigsw.c -- report an ambiguous switch
 *
 * $Id: ambigsw.c,v 1.1.1.1 1999-02-07 18:14:07 danw Exp $
 */

#include <h/mh.h>


void
ambigsw (char *arg, struct swit *swp)
{
    advise (NULL, "-%s ambiguous.  It matches", arg);
    print_sw (arg, swp, "-");
}
