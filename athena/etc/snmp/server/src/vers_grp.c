/*
 * This is the MIT supplement to the PSI/NYSERNet implementation of SNMP.
 * This file describes the version portion of the mib.
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Tom Coppeto
 * MIT Network Services
 * 15 April 1992
 *
 *    $Source: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/vers_grp.c,v $
 *    $Author: tom $
 *    $Locker:  $
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/vers_grp.c,v 2.0 1992-04-22 02:04:16 tom Exp $";
#endif


#include "include.h"
#include <mit-copyright.h>

#ifdef MIT
#include "compile_time.h"

#ifdef ATHENA

char *get_ws_version();
char *get_snmp_version_str();


/*
 * Function:    lu_relvers()
 * Description: Top level callback for release version. 
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

lu_relvers(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  char *version_str;
  char *version;
  char *type;
  char *date;

  if (varnode->flags & NOT_AVAIL ||
      varnode->offset < 0 ||       /* not expecting offset here */
      ((varnode->flags & NULL_OBJINST) && (reqflg == NXT)))
    return (BUILD_ERR);

  if(!(version_str = get_ws_version(version_file)))
    return(BUILD_ERR);
  
  /*
   * Build reply
   */

  bcopy ((char *)varnode->var_code, (char *) &repl->name, sizeof(repl->name));
  repl->name.ncmp++;			/* include the "0" instance */

  switch(varnode->offset)
    {
    case N_RELVERSSTR:
      return(make_str(&(repl->val), version_str));
    case N_RELVERSION:
      parse_ws_version(version_str, &version, &type, &date);
      return(make_str(&(repl->val), version));
    case N_RELVERSTYPE:
      parse_ws_version(version_str, &version, &type, &date);
      return(make_str(&(repl->val), type));
    case N_RELVERSDATE:
      parse_ws_version(version_str, &version, &type, &date);
      return(make_str(&(repl->val), date));
    default:
      syslog (LOG_ERR, "lu_relvers: bad offset: %d", varnode->offset);
      return(BUILD_ERR);
    }
}






/*
 * Function:    lu_spnum()
 * Description: Top level callback. Supports:
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */


int
lu_spnum(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  if (varnode->flags & NOT_AVAIL || varnode->offset <= 0 ||
      ((varnode->flags & NULL_OBJINST) && (reqflg == NXT)))
    return (BUILD_ERR);

  
  /*
   * Build reply
   */

  bcopy ((char *)varnode->var_code, (char *) &repl->name, sizeof(repl->name));
  repl->name.ncmp++;			/* include the "0" instance */

  switch(varnode->offset)
    {
    case N_SYSPACKNUM:
      repl->val.type = INT;  
      repl->val.value.intgr = 1;
      return(BUILD_SUCCESS);
    default:
       syslog (LOG_ERR, "lu_spnum: bad offset: %d", varnode->offset);
      return(BUILD_ERR);
    }
}





lu_spvers(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  char *version_str;
  char *version;
  char *type;
  char *date;
  int num;

  if (varnode->flags & NOT_AVAIL || varnode->offset < 0)
    return (BUILD_ERR);

  if(!(version_str = get_ws_version(syspack_file)))
    return(BUILD_ERR);
  
  /*
   * Build reply
   */

   if(!instptr || (instptr->ncmp == 0))
     num = 1;
   else
     num = instptr->cmp[0];

  if((reqflg & NXT) && instptr && instptr->ncmp)
    num++;

  if(num > 1)
    return(BUILD_ERR);

  bcopy ((char *)varnode->var_code, (char *) &repl->name, sizeof(repl->name));
  repl->name.cmp[repl->name.ncmp] = num;
  repl->name.ncmp++;			/* include the "0" instance */

  switch(varnode->offset)
    {
    case N_SYSPACKNAME:
      return(make_str(&(repl->val), "srvd"));
    case N_SYSPACKVERS:
      parse_ws_version(version_str, &version, &type, &date);
      return(make_str(&(repl->val), version));
    case N_SYSPACKTYPE:
      parse_ws_version(version_str, &version, &type, &date);
      return(make_str(&(repl->val), type));
    case N_SYSPACKDATE:
      parse_ws_version(version_str, &version, &type, &date);
      return(make_str(&(repl->val), date));
    default:
      syslog (LOG_ERR, "lu_packvers: bad offset: %d", varnode->offset);
      return(BUILD_ERR);
    }
}

#endif /* ATHENA */

/*
 * Function:    lu_rsnmpvers()
 * Description: Top level callback for snmp version. 
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

lu_snmpvers(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  char *version_str;
  char *version;
  char *type;
  char *date;

  if (varnode->flags & NOT_AVAIL ||
      varnode->offset < 0 ||       /* not expecting offset here */
      ((varnode->flags & NULL_OBJINST) && (reqflg == NXT)))
    return (BUILD_ERR);
  
  /*
   * Build reply
   */

  bcopy ((char *)varnode->var_code, (char *) &repl->name, sizeof(repl->name));
  repl->name.ncmp++;			/* include the "0" instance */

  switch(varnode->offset)
    {
    case N_SNMPVERS:
      return(make_str(&(repl->val), MIT_VERSION));
    case N_SNMPBUILD:
      return(make_str(&(repl->val), COMPILE_TIME));
    default:
      syslog (LOG_ERR, "lu_snmpvers: bad offset: %d", varnode->offset);
      return(BUILD_ERR);
    }
}



#ifdef ATHENA

/*
 * Function:    get_ws_version()
 * Description: gets last line out of /etc/version and returns it in string
 * Returns:     version string
 */

char *
get_ws_version(file)
     char *file;
{
  FILE *fp;
  static char string[BUFSIZ];

  fp = fopen(file, "r");
  if(fp == NULL)
    {
      syslog(LOG_ERR,"get_version: cannot open %s", version_file);
      return((char *) NULL);
    }

  bzero(string, sizeof(string));
  while(fgets(lbuf, sizeof(lbuf) - 1, fp) != NULL)
    if(*lbuf != '\n')
      strncpy(string, lbuf, sizeof(string));
	
  string[strlen(string) - 1] = '\0';
  (void) fclose(fp);

  return(string);
}


parse_ws_version(string, version, type, date)
     char *string;
     char **type;
     char **version;
     char **date;
{
  char *c;
  char *d;

  c = string;
  while (isspace(*c))
    ++c;
  d = c;
  while (!isspace(*c))
    ++c;
  while (isspace(*c))
    ++c;
  while (!isspace(*c))
    ++c;
  *c++ = '\0';
  *type = d;
  while (isspace(*c))
    ++c;
  while (!isspace(*c))
    ++c;
  while (isspace(*c))
    ++c;
  while (!isspace(*c))
    ++c;
  while (isspace(*c))
    ++c;
  d = c;
  while (!isspace(*c))
    ++c;
  *c++ = '\0';
  *version = d;
  d = c;
  while(*c && (*c != '\n'))
    ++c;
  *c = '\0';
  *date = d;
  return;
}

#endif ATHENA


/*
 * Function:    get_snmp_version_str()
 * Description: 
 * Returns:     version string
 */

char *
get_snmp_version_str()
{
  sprintf(lbuf, "SNMP PSI Version %s  MIT MIB Version %s compiled %s.",
	  VERSION, MIT_VERSION, COMPILE_TIME);
  return(lbuf);
}



#endif MIT










