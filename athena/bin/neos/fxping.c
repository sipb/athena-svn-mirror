/**********************************************************************
 * File Exchange fxping client
 *
 * $Id: fxping.c,v 1.2 2002-03-13 17:06:41 zacheiss Exp $
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef lint
static char rcsid_fxping_c[] = "$Id: fxping.c,v 1.2 2002-03-13 17:06:41 zacheiss Exp $";
#endif /* lint */

#include <stdio.h>
#include <string.h>
#include <fx/fxcl.h>		/* overrides #include <time.h> */

main(argc, argv)
  int argc;
  char *argv[];
{
  FX *fxp;
  long code;
  stringlist node;
  server_stats *stats;
  struct tm *tim;

  /* Initialization needed for com_err routines */
  initialize_fxcl_error_table();
  initialize_rpc_error_table();
  initialize_fxsv_error_table();
  initialize_krb_error_table();
  fxp = fx_open("", &code);
  if (code) 
    com_err(argv[0], code, "while connecting.");
  if (!fxp)
    exit(1);
  for (node = fx_host_list(); node != NULL; node = node->next) {
    strcpy(fxp->host, node->s);
    code = fx_connect(fxp);
    if (!code)
      fx_stat(fxp, &stats);
    else
      {
	printf("%16s down: %s\n", fxp->host, error_message(code));
	continue;
      }

    tim = (struct tm *)gmtime(&stats->uptime);
    printf("%16s up %d days, %02d:%02d:%02d, DB %ld/%ld\n", fxp->host,
	   tim->tm_yday, tim->tm_hour, tim->tm_min, tim->tm_sec,
	   stats->vers.synctime, stats->vers.commit);
  }
  fx_close(fxp);
  exit(0);
}
