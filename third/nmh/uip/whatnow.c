
/*
 * whatnow.c -- the nmh `WhatNow' shell
 *
 * $Id: whatnow.c,v 1.1.1.1 1999-02-07 18:14:17 danw Exp $
 */

#include <h/mh.h>


int
main (int argc, char **argv)
{
#ifdef LOCALE
    setlocale(LC_ALL, "");
#endif
    WhatNow (argc, argv);
}
