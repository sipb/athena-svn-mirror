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
 *    $Author: ghudson $
 *    $Locker:  $
 *    $Log: not supported by cvs2svn $
 *    Revision 2.0  1992/04/22 02:05:03  tom
 *    release 7.4
 *    	new rc variables
 *
 * Revision 1.5  90/07/17  14:21:28  tom
 * do not return error if variable does not exist... get-next will discontinue
 * this branch if an error occurrs... instead set string to null.
 * 
 * Revision 1.4  90/07/15  18:11:49  tom
 * malloc only what we need for variable size
 * 
 * Revision 1.3  90/05/26  13:40:23  tom
 * athena release 7.0e
 * 
 * Revision 1.2  90/04/26  17:53:39  tom
 * *** empty log message ***
 * 
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/rc_grp.c,v 2.1 1997-02-27 06:47:44 ghudson Exp $";
#endif


#include "include.h"
#include <mit-copyright.h>

#ifdef MIT
#ifdef ATHENA

static int get_rc_variable();
static int get_srv_number();
static char *get_service();

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

  memcpy (&repl->name, varnode->var_code, sizeof(repl->name));
  repl->name.ncmp++;			/* include the "0" instance */

  repl->val.type = STR;  /* True of all the replies */

  switch (varnode->offset) 
    {
    case N_RCFILE:
      repl->val.value.str.str = malloc(strlen(rc_file) + 1);
      if(repl->val.value.str.str)
	{
	  strcpy(repl->val.value.str.str, rc_file);
	  repl->val.value.str.len = strlen(repl->val.value.str.str);
	  return(BUILD_SUCCESS);
	}
      else
	return(BUILD_ERR);
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
      return(get_variable("AFSSRV", &(repl->val.value.str)));
    case N_RCAFSCLIENT:
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
    case N_AFSADJUST:
      return(get_variable("AFSADJUST", &(repl->val.value.str)));
    case N_SNMP:
      return(get_variable("SNMP",      &(repl->val.value.str)));
    case N_AUTOUPDATE:
      return(get_variable("AUTOUPDATE",&(repl->val.value.str)));
    default:
      syslog (LOG_ERR, "lu_rc: bad rc offset: %d", varnode->offset);
      return(BUILD_ERR);
    }
}




/*
 * Function:    lu_service()
 * Description: Top level callback. Suports the following:
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

int
lu_service(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  char s[SNMPMXID];
  char *ch;
  int  cnt;

  if((varnode->flags & NOT_AVAIL) || (varnode->offset <= 0))
    return (BUILD_ERR);
  
  if((instptr == (objident *) NULL) || (instptr->ncmp == 0))
    s[0] = '\0';
  else
    {
      cnt = 0;
      while((cnt < instptr->ncmp) && (cnt < SNMPMXID))
        {
          s[cnt] = instptr->cmp[cnt];
          ++cnt;
        }
      s[cnt] = '\0';
    }
  
  if((ch = get_service(s, reqflg)) == (char *) NULL)
    return(BUILD_ERR);
  
  memcpy (&repl->name, varnode->var_code, sizeof(repl->name));
  repl->val.type = STR;
  if((repl->val.value.str.str = malloc(strlen(ch) + 1)) == (char *) NULL)
    {
      syslog(LOG_ERR, "malloc failed");
      return(BUILD_ERR);
    }
  strcpy(repl->val.value.str.str, ch);
  repl->val.value.str.len = strlen(ch);
  

  /*
   *  fill in object instance name
   */

  cnt = 0;
  len = strlen(ch);
  if((len + repl->name.ncmp) > SNMPMXID)
    len = SNMPMXID - repl->name.ncmp;

  while (cnt < len)
    {
      repl->name.cmp[repl->name.ncmp] = *ch & 0xff;
      repl->name.ncmp++;
      cnt++;
      ch++;
    }
  return(BUILD_SUCCESS);
}




/*
 * Function:    get_variable()
 * Description: gets var and puts size of it into string. 
 *               getenv() is a lousy copout.
 */

static int 
get_rc_variable(var, s)
  char  *var;
  strng *s;    
{
  FILE *fp;
  char *a;
  char *b;
  
  if((fp = fopen(rc_file, "r")) == (FILE *) NULL)
    {
      syslog(LOG_ERR, "unable to open rc config file \"%s\".", rc_file);
      return(BUILD_ERR);
    }

  while(fgets(lbuf, sizeof(lbuf)-1, fp) != (char *) NULL)
    {
      a = var;
      b = lbuf;
      while(*a++ == *b++);
      while(((*b == ' ') || (*b == '\t')) && (*b != '\0'))
	++b;
      if(*b++ != '=')
	continue;
      c = b;
      while((*c != '\0') && (*c != ';'))
	++c;
      if((*c == '\0') || (*c == '\n'))
	continue;
      *c = '\0';
      fclose(fp);

      if((s->str = (char *) malloc(strlen(b) * sizeof(char))) == 
	 (char *) NULL)
	return(BUILD_ERR);
	
      strcpy(s->str, b);      
      s->len = strlen(b);
      return(BUILD_SUCCESS);
    }

  fclose(fp);
  s->str = (char *) NULL;
  s->len = 0;
  return(BUILD_SUCCESS);
}


static int
get_srv_number(n)
     int *n;
{
  FILE *fp;
  char *c;
  int i = 0;

  if((fp = fopen(srv_file, "r")) == (FILE *) NULL)
    return(BUILD_ERR);
    
  while(fgets(lbuf, sizeof(lbuf)-1, fp) != (char *) NULL)
    {
      c = lbuf;
      while((*c != '\0') && ((*c == ' ') || (*c == '\t')))
	*c++;
      if((*c == '\n') || (*c == '\0'))
	continue;
      ++i;
    }
  fclose(fp);
  *n = i;
  return(BUILD_SUCCESS);
}



static char *
get_service(name, req)
     char *name;
     int  req;
{
  FILE *fp;
  char *c;
  int onemore = 0;

  if((fp = fopen(srv_file, "r")) == (FILE *) NULL)
    return(BUILD_ERR);
    
  while(fgets(lbuf, sizeof(lbuf)-1, fp) != (char *) NULL)
    {
      c = lbuf;
      while((*c != '\0') && ((*c == ' ') || (*c == '\t')))
	*c++;
      if((*c == '\n') || (*c == '\0'))
	continue;
      if(*name && (strcmp(name, c) == 0))
	{
	  if((req == NXT) && !onemore)
	    {
	      onemore = 1;
	      continue;
	    }
	  break;
	}
      if((*name == '\0') && (req == NXT))
	break;
    }

  fclose(fp);
  return(c);
}



#endif ATHENA
#endif MIT
