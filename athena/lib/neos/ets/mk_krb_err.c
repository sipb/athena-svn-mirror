/**********************************************************************
 * Error table generator for kerberos
 *
 * $Author: ghudson $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/ets/mk_krb_err.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/ets/mk_krb_err.c,v 1.2 1996-08-10 05:40:15 ghudson Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>
#include "krb.h"

main()
{
  int i;
  const char *msg;

  printf("############################################################\n");
  printf("# File Exchange kerberos error table\n");
  printf("# This file is automatically generated.  Do not edit it.\n");
  printf("############################################################\n\n");

  printf("error_table\tkrb\n\n");

  for (i=0; i < MAX_KRB_ERRORS; i++) {
    msg = krb_err_txt[i];
    if (!msg) msg = "";
    printf("error_code\tERR_KRB_%d,\n\t\t\"%s\"\n\n", i, msg);
  }
  printf("end\n");
  exit(0);
}
