/*
 * This is the MIT supplement to the PSI/NYSERNet implementation of SNMP.
 * This file describes the workstation status portion of the mib.
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
 *    $Source: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/stat_grp.c,v $
 *    $Author: tom $
 *    $Locker:  $
 *    $Log: not supported by cvs2svn $
 * Revision 1.2  90/04/26  18:14:39  tom
 * *** empty log message ***
 * 
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/stat_grp.c,v 1.3 1990-05-26 13:41:00 tom Exp $";
#endif


#include "include.h"
#include <mit-copyright.h>

#ifdef MIT
#include <utmp.h>

static int stattime();
static int get_load();
static int get_time();
char lbuf[BUFSIZ];

#ifdef ATHENA
static int get_ws_version();

#define MAX_REPLY_SIZE 256
#define VERSION_FILE "/etc/version"



/*
 * Function:    lu_relvers()
 * Description: Top level callback for workstation version. 
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

lu_relvers(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  if (varnode->flags & NOT_AVAIL ||
      varnode->offset < 0 ||       /* not expecting offset here */
      ((varnode->flags & NULL_OBJINST) && (reqflg == NXT)))
    return (BUILD_ERR);

  
  /*
   * Build reply
   */

  bcopy ((char *)varnode->var_code, (char *) &repl->name, sizeof(repl->name));
  repl->name.ncmp++;			/* include the "0" instance */

  repl->val.type = STR;  /* True of all the replies */
  repl->val.value.str.str = (char *) malloc(sizeof(char) * MAX_REPLY_SIZE);
  repl->val.value.str.len = MAX_REPLY_SIZE;

  return(get_ws_version(repl->val.value.str.str,MAX_REPLY_SIZE));
}

#endif ATHENA


/*
 * Function:    lu_status()
 * Description: Top level callback for workstation status. Supports:
 *                    N_STATIME:   (INT)  time on agent
 *                    N_STATLOAD:  (INT)  load on agent
 *                    N_STATLOGIN: (INT)  1 if in use
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

int
lu_status(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  if (varnode->flags & NOT_AVAIL ||
      varnode->offset < 0 ||       /* not expecting offset here */
      ((varnode->flags & NULL_OBJINST) && (reqflg == NXT)))
    return (BUILD_ERR);

  
  /*
   * Build reply
   */

  bcopy ((char *)varnode->var_code, (char *) &repl->name, sizeof(repl->name));
  repl->name.ncmp++;			/* include the "0" instance */

  repl->val.type = INT;  /* True of all the replies */

  switch (varnode->offset)
    {
    case N_STATTIME:
      if(get_time(&(repl->val.value.intgr)))
	return(BUILD_ERR);
      else
	return(BUILD_SUCCESS);
    case N_STATLOAD:
      if(get_load(&(repl->val.value.intgr)))
	return(BUILD_ERR);
      else
	return(BUILD_SUCCESS);
#ifdef LOGIN
    case N_STATLOGIN:
      if(get_inuse(&(repl->val.value.intgr)))
	return(BUILD_ERR);
      else
	return(BUILD_SUCCESS);
#endif LOGIN
    default:
      return(BUILD_ERR);
    }
}





/*
 * Function:    lu_tuchtime()
 * Description: Top level callback for mod time on files. Supports:
 *                     aliases file
 *                     credentials file
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

int
lu_tuchtime(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  if (varnode->flags & NOT_AVAIL ||
      varnode->offset < 0 ||       /* not expecting offset here */
      ((varnode->flags & NULL_OBJINST) && (reqflg == NXT)))
    return (BUILD_ERR);

  
  /*
   * Build reply
   */

  bcopy ((char *)varnode->var_code, (char *) &repl->name, sizeof(repl->name));
  repl->name.ncmp++;			/* include the "0" instance */

  repl->val.type = INT;  /* True of all the replies */

  /* 
   * Files should be defined in snmpd.conf
   */

  switch(varnode->offset)
    {
    case N_MAILALIAS:
      repl->val.value.intgr = stattime("/usr/lib/aliases");
      return(BUILD_SUCCESS);
    case N_MAILALIASPAG:
      repl->val.value.intgr = stattime("/usr/lib/aliases.pag");
      return(BUILD_SUCCESS);
    case N_MAILALIASDIR:
      repl->val.value.intgr = stattime("/usr/lib/aliases.dir");
      return(BUILD_SUCCESS);
    case N_RPCCRED:
      repl->val.value.intgr = stattime("/usr/etc/credentials");
      return(BUILD_SUCCESS);
    case N_RPCCREDPAG:
      repl->val.value.intgr = stattime("/usr/etc/credentials.pag");
      return(BUILD_SUCCESS);
    case N_RPCCREDDIR:
      repl->val.value.intgr = stattime("/usr/etc/credentials.dir");
      return(BUILD_SUCCESS);
    default:
      return(BUILD_ERR);
    }
}




/*
 * Function:    stattime()
 * Description: Returns modification time on given file.
 * Returns:     time if success
 *              0 if error
 */

static int
stattime(file)
     char *file;
{
  struct stat statbuf;
  bzero(&statbuf, sizeof(struct stat));

  if(file == (char *) NULL)
    return(0);

  if(stat(file, &statbuf) < 0)
    return(0);

  return(statbuf.st_mtime);
}


#ifdef ATHENA
/*
 * Function:    get_ws_version()
 * Description: gets last line out of /etc/version and returns it in string
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

static int
get_ws_version(string, size)
     char *string;
     int size;
{
  FILE *fp;

  fp = fopen(VERSION_FILE, "r");
  if(fp == NULL)
    {
      syslog(LOG_ERR,"get_version: cannot open %s", VERSION_FILE);
      return(BUILD_ERR);
    }

  while(fgets(lbuf, sizeof(lbuf), fp) != NULL)
    strncpy(string, lbuf, size);

  string[strlen(string)-1] = '\0';
  (void) fclose(fp);
  return(BUILD_SUCCESS);
}
#endif ATHENA


/*
 * Function:    get_time()
 * Description: gets the time
 * Returns:     0
 */

static int
get_time(ret)
     int *ret;
{
  *ret = (int) time(0);
  return(0);
}



/*
 * Function:    get_load()
 * Description: gets the load, places it in ret
 * Returns:     0 on success
 *              4 on error?
 */

static int
get_load(ret)
     int *ret;
{
  int load;

#if defined(vax) || defined(ibm032)
  double avenrun[3];
#endif
#if defined(AIX) || defined(mips)
  int avenrun[3];
#endif

#if !defined (vax) && !defined(ibm032) && !defined(AIX) && !defined(mips)
  return(parse_load_from_uptime(ret));
#endif
  
  if(nl[N_AVENRUN].n_value == 0)
    {
      syslog(LOG_ERR, "Couldn't find load average from name list.\n");
      return(4);      
    }
	
  if(nl[N_HZ].n_value == 0 || nl[N_CPTIME].n_value == 0)
    {
      syslog(LOG_ERR, "Couldn't find cpu time from name list.\n");
      return(4);
    }

  (void) lseek(kmem, nl[N_AVENRUN].n_value, L_SET);
  if(read(kmem, avenrun, sizeof(avenrun)) == -1)
    {
      syslog(LOG_ERR, "avenrun read(%d, %#x, %d) failed: %s\n",
	      kmem, avenrun, sizeof(avenrun), sys_errlist[errno]);
      return(4);
    } 
  else 
    {

#if defined(vax) || defined(ibm032)
      load  = (int) (avenrun[0] * 48 + avenrun[1] * 32 + avenrun[2] * 16);
#endif

#if defined(AIX)
      load  = (avenrun[0] >> 11) +  (avenrun[1] >> 12) + (avenrun[2] >> 12);
#endif

    }
  
  *ret = load;
  return(0);
}




#ifdef LOGIN

/*
 * Function:    get_inuse()
 * Description: Decides if one is logegd in. If so, ret is set to # entries
 *              in utmp. 
 * Returns:     0 on success
 *              4 on error
 */

int
get_inuse(ret)
        int *ret;
{
  struct utmp *uptr;
  int fd;
  long lseek();
  int usize,ucount,loop;

  usize = sizeof(struct utmp);
  uptr = (struct utmp *) & lbuf[0];

  fd = open(LOGIN_FILE, O_RDONLY);
  if(fd == NULL)
          return(4);
  ucount = loop = 0;
  while(read(fd,lbuf,usize) > 0) {
        ++loop;
        if(uptr->ut_name[0] != 0)
                ++ucount;
        }

 *ret = ucount;
  close(fd);
  return(0);
}

#endif LOGIN
#endif MIT
