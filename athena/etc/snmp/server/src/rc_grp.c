/*
 * This is the MIT supplement to the PSI/NYSERNet implementation of SNMP.
 * This file describes the Athena configuration portion of the mib.
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
 *    $Source: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/rc_grp.c,v $
 *    $Author: tom $
 *    $Locker:  $
 *    $Log: not supported by cvs2svn $
 * Revision 1.3  90/05/26  13:40:23  tom
 * athena release 7.0e
 * 
 * Revision 1.2  90/04/26  17:53:39  tom
 * *** empty log message ***
 * 
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/rc_grp.c,v 1.4 1990-07-15 18:11:49 tom Exp $";
#endif


#include "include.h"
#include <mit-copyright.h>

#ifdef MIT
#ifdef ATHENA

static int get_variable();

/*
 * Function:    lu_rcvar()
 * Description: Top level callback. Supports variables in rc.conf.
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */


int
lu_rcvar(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  if (varnode->flags & NOT_AVAIL ||
      varnode->offset <= 0 ||
      ((varnode->flags & NULL_OBJINST) && (reqflg == NXT)))
    return (BUILD_ERR);

  
  /*
   * Build reply
   */

  bcopy ((char *)varnode->var_code, (char *) &repl->name, sizeof(repl->name));
  repl->name.ncmp++;			/* include the "0" instance */

  repl->val.type = STR;  /* True of all the replies */

  switch (varnode->offset) 
    {
    case N_RCHOST:
      return(get_variable("HOST",      &(repl->val.value.str)));
    case N_RCADDR:
      return(get_variable("ADDR",      &(repl->val.value.str)));
    case N_RCMACHINE:
      return(get_variable("MACHINE",   &(repl->val.value.str)));
    case N_RCSYSTEM:
      return(get_variable("SYSTEM",    &(repl->val.value.str)));
    case N_RCWS:
      return(get_variable("WS",        &(repl->val.value.str)));
    case N_RCTOEHOLD:
      return(get_variable("TOEHOLD",   &(repl->val.value.str)));
    case N_RCPUBLIC:
      return(get_variable("PUBLIC",    &(repl->val.value.str)));
    case N_RCERRHALT:
      return(get_variable("ERRHALT",   &(repl->val.value.str)));
    case N_RCLPD:
      return(get_variable("LPD",       &(repl->val.value.str)));
    case N_RCRVDSRV:
      return(get_variable("RVDSRV",    &(repl->val.value.str)));
    case N_RCRVDCLIENT:
      return(get_variable("RVDCLIENT", &(repl->val.value.str)));
    case N_RCNFSSRV:
      return(get_variable("NFSSRV",    &(repl->val.value.str)));
    case N_RCNFSCLIENT:
      return(get_variable("NFSCLIENT", &(repl->val.value.str)));
    case N_RCAFSSRV:
      return(get_variable("AFSCLIENT", &(repl->val.value.str)));
    case N_RCRPC:
      return(get_variable("RPC",       &(repl->val.value.str)));
    case N_RCSAVECORE:
      return(get_variable("SAVECORE",  &(repl->val.value.str)));
    case N_RCPOP:
      return(get_variable("POP",       &(repl->val.value.str)));
    case N_RCSENDMAIL:
      return(get_variable("SENDMAIL",  &(repl->val.value.str)));
    case N_RCQUOTAS:
      return(get_variable("QUOTAS",    &(repl->val.value.str)));
    case N_RCACCOUNT:
      return(get_variable("ACCOUNT",   &(repl->val.value.str)));
    case N_RCOLC:
      return(get_variable("OLC",       &(repl->val.value.str)));
    case N_RCTIMESRV:
      return(get_variable("TIMESRV",   &(repl->val.value.str)));
    case N_RCPCNAMED:
      return(get_variable("PCNAMED",   &(repl->val.value.str)));
    case N_RCNEWMAILCF:
      return(get_variable("NEWMAILCF", &(repl->val.value.str)));
    case N_RCKNETD:
      return(get_variable("KNETD",     &(repl->val.value.str)));
    case N_RCTIMEHUB:
      return(get_variable("TIMEHUB",   &(repl->val.value.str)));
    case N_RCZCLIENT:
      return(get_variable("ZCLIENT",   &(repl->val.value.str)));
    case N_RCZSERVER:
      return(get_variable("ZSERVER",   &(repl->val.value.str)));
    case N_RCSMSUPDATE:
      return(get_variable("SMSUPDATE", &(repl->val.value.str)));
    case N_RCINETD:
      return(get_variable("INETD",     &(repl->val.value.str)));
    case N_RCNOCREATE:
      return(get_variable("NOCREATE",  &(repl->val.value.str)));
    case N_RCNOATTACH:
      return(get_variable("NOATTACH",  &(repl->val.value.str)));
    default:
      syslog (LOG_ERR, "lu_rc: bad rc offset: %d", varnode->offset);
      return(BUILD_ERR);
    }
}



/*
 * Function:    return(get_variable()
 * Description: gets var and puts size of it into string. 
 *               getenv() is a lousy copout.
 */

static int 
return(get_variable(var, str)
  char *var;
  str *str;    
{
  char *ptr;

  ptr = (char *) getenv(var);
  if((str.str = (char *) malloc(strlen(ptr) * sizeof(char))) == (char *) NULL)
	return(BUILD_ERR);
  
  strcpy(str.str, ptr);      
  str.len = strlen(ptr);
  return(BUILD_SUCCESS);
}


#endif ATHENA
#endif MIT
