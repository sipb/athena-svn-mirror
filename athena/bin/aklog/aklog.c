/* 
 * $Id: aklog.c,v 1.3 1991-07-16 06:25:13 probe Exp $
 *
 * Copyright 1990,1991 by the Massachusetts Institute of Technology
 * For distribution and copying rights, see the file "mit-copyright.h"
 */

#if !defined(lint) && !defined(SABER)
static char *rcsid = "$Id: aklog.c,v 1.3 1991-07-16 06:25:13 probe Exp $";
#endif /* lint || SABER */

#include "aklog.h"

#ifdef __STDC__
main(int argc, char *argv[])
#else
main(argc, argv)
  int argc;
  char *argv[];
#endif /* __STDC__ */
{
    aklog_params params;

    aklog_init_params(&params);
    aklog(argc, argv, &params);
}
