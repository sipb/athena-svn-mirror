/*
 * This is the MIT supplement to the PSI/NYSERNet implementation of SNMP.
 * This file containms miscellaneous utilities.                        
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
 * Revision 1.4  90/05/26  13:41:43  tom
 * athena release 7.0e
 * 
 * Revision 1.3  90/04/26  18:38:59  tom
 * corrected template header
 * 
 * Revision 1.2  90/04/26  18:35:53  tom
 * *** empty log message ***
 * 
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/utils.c,v 2.0 1992-04-22 01:59:26 tom Exp $";
#endif

#include "include.h"
#include <mit-copyright.h>

#ifdef MIT




/*
 * Function:    stattime()
 * Description: Returns modification time on given file.
 * Returns:     time if success
 *              0 if error
 */


char *
stattime(file)
     char *file;
{
  struct stat statbuf;
  char *c, *t;

  bzero(&statbuf, sizeof(struct stat));

  if(!file)
    return(file);

  if(stat(file, &statbuf) < 0)
    return((char *) NULL);
  
  t   = ctime(&(statbuf.st_mtime));
  c   = rindex(t, '\n');
  *c  = '\0';
  return(t);
}




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
  return(0);
}
  


/*
 * Function:     make_str()
 * Description:  create strng object for given string 
 */


int 
make_str(v, s)
     objval *v;
     char   *s;
{
  if(!s)
    return(BUILD_ERR);

  v->type = STR;
  
  if(!(v->value.str.str = malloc((strlen(s) + 1) * sizeof(char))))
    {
      syslog(LOG_ERR, "unable to allocate %d bytes for string.",
	     (strlen(s) + 1) * sizeof(char));
      return(BUILD_ERR);
    }

  strcpy(v->value.str.str, s);
  v->value.str.len = strlen(v->value.str.str);
  return(BUILD_SUCCESS);
}

#endif MIT
