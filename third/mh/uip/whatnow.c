/* whatnow.c - the MH WhatNow? shell */
#ifndef	lint
static char ident[] = "@(#)$Id: whatnow.c,v 1.1.1.1 1996-10-07 07:14:26 ghudson Exp $";
#endif	/* lint */

#ifdef LOCALE
#include	<locale.h>
#endif

main (argc, argv)
int	argc;
char  **argv;
{
#ifdef LOCALE
    setlocale(LC_ALL, "");
#endif
    WhatNow (argc, argv);
}
