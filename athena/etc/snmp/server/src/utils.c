/*
 * This is the MIT supplement to the PSI/NYSERNet implementation of SNMP.
 * This file describes the AFS (Andrew File System) portion of the mib.
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Tom Coppeto
 * MIT Network Services
 * 15 April 1990
 *
 *    $Source: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/utils.c,v $
 *    $Author: tom $
 *    $Locker:  $
 *    $Log: not supported by cvs2svn $
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/utils.c,v 1.2 1990-04-26 18:35:53 tom Exp $";
#endif

#include "include.h"
#include <mit-copyright.h>

#ifdef MIT

/*
 * Function:     call_program()
 * Description:  forks a program and sets ret to whatthe programs prints out
 *               up to size characters.
 */

int
call_program(program, arg, ret, size)
     char *program;
     char *arg;
     char *ret;
     int size;
{
  int f[2];

  pipe(f);
  switch(fork()) 
    {
    case 0:             /* child */
      (void) close(f[0]);
      (void) close(0);
      (void) dup2(f[1], 1);
      execl(program, program, arg, 0);
      perror("exec");
      _exit(1);
      break;
    case -1:            /* error */
      perror("Can't fork");
      return(-1);
    default:            /* parent */
      (void) close(f[1]);
  }

  read(f[0],ret,size);
  wait(0);
  (void) close(f[0]);
  ret[size-1] = '\0';
}
  

#endif MIT
