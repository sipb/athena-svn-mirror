/*
 * This is the MIT supplement to the PSI/NYSERNet implementation of SNMP.
 * This file describes the Kerberos portion of the mib.
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
 *    $Source: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/krb_grp.c,v $
 *    $Author: tom $
 *    $Locker:  $
 *    $Log: not supported by cvs2svn $
 * Revision 1.3  90/04/26  17:05:46  tom
 * added rcsid
 * 
 * Revision 1.2  90/04/26  16:51:07  tom
 * *** empty log message ***
 * 
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/krb_grp.c,v 1.4 1990-05-26 13:38:18 tom Exp $";
#endif

#include "include.h"
#include <mit-copyright.h>

#ifdef MIT
#ifdef KERBEROS

#include <krb.h>

#define DATA_LEN 64
#define SRVTAB_FILE "/etc/srvtab"

static char *read_service_key();

/*
 * Function:    lu_kerberos()
 * Description: Top level callback for kerberos. The realm and key version
 *              variables should be split into two functions.
 */
 
int
lu_kerberos(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  char buf[DATA_LEN];
  char key[SNMPMXSID];
  int  nflag = 0;
  char *pal;
  char *ch;
  int len;
  int cnt;
  int num;

  bzero(buf, sizeof(buf));
  bzero(key, sizeof(key));

  if (varnode->flags & NOT_AVAIL || varnode->offset <= 0)
    return (BUILD_ERR);

  if((instptr == (objident *) NULL) || (instptr->ncmp == 0))
    num = 1;
  else
    num = instptr->cmp[0];

  if((reqflg == NXT) && (instptr != (objident *) NULL) &&
     (instptr->ncmp != 0))
    num++;

  /*
   * Build reply
   */

  bcopy ((char *)varnode->var_code, (char *) &repl->name, sizeof(repl->name));
  
  switch(varnode->offset)
    {
    case N_KRBCREALM:
      bzero(buf, sizeof(buf));      
      if(krb_get_lrealm(buf, num) == KFAILURE)
	return(BUILD_ERR);
      if(*buf == '\0')
	return(BUILD_ERR);
      repl->name.cmp[repl->name.ncmp] = num;
      repl->name.ncmp++;
      repl->val.type = STR;  
      repl->val.value.str.len = strlen(buf);
      repl->val.value.str.str = (char *) malloc(sizeof(char) * 
				repl->val.value.str.len);
      strcpy(repl->val.value.str.str, buf);
      return(BUILD_SUCCESS);

    case N_KRBCKEY:
      repl->val.type = INT; 
      if((instptr == (objident *) NULL) || (instptr->ncmp == 0))
	key[0] = '\0';
       else
	 {
	   cnt = 0;
	   while((cnt < instptr->ncmp) && (cnt < SNMPMXSID))
	     {
	       key[cnt] = instptr->cmp[cnt];
	       ++cnt;
	     }
	   key[cnt] = '\0';
	 }

      if((reqflg == NXT) && (instptr != (objident *) NULL) &&
	 (instptr->ncmp != 0))
	nflag = 1;

      if((pal = read_service_key(key, nflag, &(repl->val.value.intgr), 
				 SRVTAB_FILE)) == (char *) NULL)
	return(BUILD_ERR);

       /*
	*  fill in object instance and return value
	*/

      cnt = 0;
      ch = pal;
      len = strlen(pal);
      if(len > SNMPMXSID)
	len = SNMPMXSID;
      while(cnt < len) 
	{
	  repl->name.cmp[repl->name.ncmp] = *ch & 0xff;
	  repl->name.ncmp++;
	  cnt++;
	  ch++;
	  if(*ch == '.') /* we don't have enough room to store everything */
	    break;
        }
      
      return(BUILD_SUCCESS);
    default:
      syslog (LOG_WARNING, "lu_kerberos: bad offset: %d", varnode->offset);
      return(BUILD_ERR);
    }
}


/*
 * stolen from kerberos library- 
 * takes a principal and returns the version number of that key
 * in kvno. If principal is null, then returns the version of the nth
 * key. In either case if a key is found, a pointer to the principal is
 * returned. 
 * The read() for the key has been replaced with lseek().
 * Cuurrently only allows instance to be specified.
 */

static char *
read_service_key(sir, next, kvno, file)
     char *sir;              /* Service/Instance/Realm */
     int   next;             /* whether we want next one */
     int  *kvno;             /* Key version number */
     char *file;             /* Filename */
{
  static char princ[SNAME_SZ+INST_SZ+REALM_SZ];
  char serv[SNAME_SZ];
  char inst[INST_SZ];
  char rlm[REALM_SZ];
  unsigned char vno;        
  int wcard;
  int stab;
  int i = 0;
  int found = 0;

  if ((stab = open(file, 0, 0)) < NULL)
    return((char *) NULL);

  while(getst(stab, serv, SNAME_SZ) > 0) /* Read sname */
    {  
      (void) getst(stab, inst, INST_SZ); /* Instance */
      (void) getst(stab, rlm, REALM_SZ); /* Realm */

      /* Vers number */
      if (read(stab, (char *)&vno, 1) != 1) 
	{
	  (void) close(stab);
	  return((char *) NULL);
        }
      
      if((sir == (char *) NULL) || (strncmp(serv, sir, strlen(sir)) == 0) ||
	 (next && found))
	{ /* found it */
	  if(next && !found)
	    {
	      found = 1;
	      goto seek;
	    }
	  *kvno = vno;
	  sprintf(princ, "%s.%s@%s", serv, inst, rlm);
	  (void) close(stab);
	  return(princ);
	}

seek:	  
        if(lseek(stab, 8, L_INCR) < 0)  /* Skip Key */
	  {
            (void) close(stab);
            return((char *) NULL);
	  }
    }
  
  /* Can't find the requested service */
  (void) close(stab);
  return((char *) NULL);
}


/*
 * getst() takes a file descriptor, a string and a count.  It reads
 * from the file until either it has read "count" characters, or until
 * it reads a null byte.  When finished, what has been read exists in
 * the given string "s".  If "count" characters were actually read, the
 * last is changed to a null, so the returned string is always null-
 * terminated.  getst() returns the number of characters read, including
 * the null terminator.
 */

int
getst(fd, s, n)
     int fd;
     register char *s;
{
  register count = n;
  while (read(fd, s, 1) > 0 && --count)
    if (*s++ == '\0')
      return (n - count);
  *s = '\0';
  return (n - count);
}

#endif KERBEROS
#endif MIT
