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
 * Revision 2.0  92/04/22  02:05:13  tom
 * release 7.4
 * 	removed version stuff from this file
 * 
 * Revision 1.7  90/07/17  14:23:26  tom
 * undef EMPTY for decmips (recalared in utmp.h)
 * 
 * Revision 1.6  90/07/16  21:55:24  tom
 * maybe this is it... all TIMES which really represented dates have been 
 * changed to strings. This was done to avoid confusion with the sysUpTime
 * variable (more of a standard thing) which measures time ticks in hundreths
 * of a second. The kernel variables measuring times in other smaller units
 * are left as INTS. 
 * 
 * Revision 1.5  90/07/15  18:00:03  tom
 * changed file mod time vars to be of type TIME
 * 
 * Revision 1.4  90/06/05  15:24:35  tom
 * lbuf is static
 * 
 * Revision 1.3  90/05/26  13:41:00  tom
 * athena release 7.0e - reduced number of buffers
 * 
 * Revision 1.2  90/04/26  18:14:39  tom
 * *** empty log message ***
 * 
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/stat_grp.c,v 2.1 1993-02-19 15:14:50 tom Exp $";
#endif


#include "include.h"
#include <mit-copyright.h>

#ifdef MIT

#ifdef EMPTY
#undef EMPTY
#endif EMPTY

#include <utmp.h>

static int  get_rtload();
static int  get_time();

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
      return(get_time(&(repl->val.value.intgr)));
    case N_STATLOAD:
      return(get_rtload(&(repl->val.value.intgr)));
    case N_STATLOGIN:
      return(get_inuse(&(repl->val.value.intgr)));
#ifdef RSPOS 
    case N_LINIT:
      repl->val.type = TIME;
      return(get_rs6k_load(&(repl->val.value.time)));
#endif /* RSPOS */
    default:
      syslog(LOG_ERR, "lu_status: bad offset: %d", varnode->offset);
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


  /* 
   * Files should be defined in snmpd.conf
   */

  switch(varnode->offset)
    {
    case N_MAILALIAS:
      return(make_str(&(repl->val), stattime(mail_alias_file)));
    case N_MAILALIASPAG:
      sprintf(lbuf, "%s.pag", mail_alias_file);
      return(make_str(&(repl->val), stattime(lbuf)));
    case N_MAILALIASDIR:
      sprintf(lbuf, "%s.dir", mail_alias_file);
      return(make_str(&(repl->val), stattime(lbuf)));
    case N_MAILALIASFILE:
      return(make_str(&(repl->val), mail_alias_file));
#ifdef RPC
    case N_RPCCRED:
      return(make_str(&(repl->val), stattime(rpc_cred_file)));
    case N_RPCCREDPAG:
      sprintf(lbuf, "%s.pag", rpc_cred_file);
      return(make_str(&(repl->val), stattime(lbuf)));
    case N_RPCCREDDIR:
      sprintf(lbuf, "%s.dir", rpc_cred_file);
      return(make_str(&(repl->val), stattime(lbuf)));
    case N_RPCCREDFILE:
      return(make_str(&(repl->val), rpc_cred_file));
#endif /* RPC */
    default:
      syslog (LOG_ERR, "lu_tuchtime: bad offset: %d", varnode->offset);
      return(BUILD_ERR);
    }
}






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
  return(BUILD_SUCCESS);
}


#ifdef ultrix
#include <sys/fixpoint.h>
#endif


/*
 * Function:    get_load()
 * Description: gets the load, places it in ret
 * Returns:     BUILD_SUCCESS on success
 *              BUILD_ERR on error
 */

static int
get_rtload(ret)
     int *ret;
{
  int load = 0;

#if defined(vax) || defined(ibm032)
  double avenrun[3];
#endif
#if defined(AIX) || defined(mips) || defined(RSPOS)
  int avenrun[3];
#endif

  if(nl[N_AVENRUN].n_value == 0)
    {
      syslog(LOG_ERR, "Couldn't find load average from name list.\n");
      return(BUILD_ERR);      
    }

#ifdef notdef	
  if(nl[N_HZ].n_value == 0 || nl[N_CPTIME].n_value == 0)
    {
      syslog(LOG_ERR, "Couldn't find cpu time from name list.\n");
      return(BUILD_ERR);
    }
#endif

  (void) lseek(kmem, nl[N_AVENRUN].n_value, L_SET);
  if(read(kmem, avenrun, sizeof(avenrun)) == -1)
    {
#if defined(decmips)
      syslog(LOG_ERR, "avenrun read(%d, %#x, %d) failed.\n",
	     kmem, avenrun, sizeof(avenrun));
#else
      syslog(LOG_ERR, "avenrun read(%d, %#x, %d) failed: %s\n",
	      kmem, avenrun, sizeof(avenrun), sys_errlist[errno]);
#endif
      return(BUILD_ERR);
    } 
  else 
    {

#if defined(vax) || defined(ibm032)
      load  = (int) (avenrun[0] * 48 + avenrun[1] * 32 + avenrun[2] * 16);
#endif

#if defined(AIX) || defined(RSPOS)
      load  = (avenrun[0] >> 11) +  (avenrun[1] >> 12) + (avenrun[2] >> 12);
#endif

#ifdef ultrix
       load = (int) (FIX_TO_DBL(avenrun[0]) * 100);
#endif
    }
  
  *ret = load;

  return(BUILD_SUCCESS);
}







/*
 * Function:    get_inuse()
 * Description: Decides if one is logegd in. If so, ret is set to # entries
 *              in utmp. 
 * Returns:     BUILD_SUCCESS on success
 *              BUILD_ERRon error
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

  fd = open(login_file, O_RDONLY);
  if(fd == NULL)
    return(BUILD_ERR);
  ucount = loop = 0;
  while(read(fd,lbuf,usize) > 0) 
    {
#ifdef RSPOS
      if(uptr->ut_type != USER_PROCESS)
	continue;
#endif /* RSPOS */
      ++loop;
      if(uptr->ut_name[0] != 0)
	++ucount;
    }

 *ret = ucount;
  close(fd);
  return(BUILD_SUCCESS);
}



#ifdef RSPOS

/*
 * Function:    get_rs6k_load()
 * Description: 
 * Returns:     BUILD_SUCCESS on success
 *              BUILD_ERRon error
 */

int
get_rs6k_load(ret)
        int *ret;
{
  struct utmp *uptr;
  int fd;
  long lseek();
  int usize;

  usize = sizeof(struct utmp);
  uptr = (struct utmp *) & lbuf[0];

  fd = open(login_file, O_RDONLY);
  if(fd == NULL)
    return(BUILD_ERR);

  while(read(fd,lbuf,usize) > 0) 
    {
      if(uptr->ut_type == BOOT_TIME)
	{
	  *ret = uptr->ut_time * 100;
	  return(BUILD_SUCCESS);
	}
    }

  return(BUILD_ERR);
}

#endif /* RSPOS */

#endif MIT










