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
 *    $Source: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/afs_grp.c,v $
 *    $Author: tom $
 *    $Locker:  $
 *    $Log: not supported by cvs2svn $
 * Revision 1.3  90/04/26  15:24:09  tom
 * *** empty log message ***
 * 
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/afs_grp.c,v 1.4 1990-05-26 13:34:28 tom Exp $";
#endif

#include "include.h"
#include <mit-copyright.h>

#ifdef MIT
#ifdef AFS
char lbuf[BUFSIZ];


/*
 * This file contains the cache size and directory.
 */

char *cache_file = "/usr/vice/etc/cacheinfo";

static int crock_cachesize();

/*
 * Function:    lu_afs()
 * Description: Top level callback. Supports the following:
 *                  N_AFSCACHESIZE- (INT) afs cache size
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

int
lu_afs(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{

  if (varnode->flags & NOT_AVAIL ||
      varnode->offset <= 0 ||     /* not expecting offset here */
      ((varnode->flags & NULL_OBJINST) && (reqflg == NXT)))
    return (BUILD_ERR);

  /*
   * Build reply
   */

  bcopy ((char *)varnode->var_code, (char *) &repl->name, sizeof(repl->name));
  repl->name.ncmp++;                    /* include the "0" instance */

  switch(varnode->offset)
    {
    case N_AFSCCACHESIZE:
      repl->val.type = INT;
      repl->val.value.intgr = crock_cachesize();
      return(BUILD_SUCCESS);
    default:
      syslog (LOG_ERR, "lu_afs: bad offset: %d", varnode->offset);
      return(BUILD_ERR);
    }
}



/*
 * Function:    crock_cachesize()
 * Description: Reads the cachefile, and returns the cache size.
 *              The cache description line better be the first line and
 *              the size after the last semi-colon.
 * Returns:     size of cache if successful
 *              0 if error
 */

static int
crock_cachesize()
{
  FILE *fp;
  char *cp;

  fp = fopen(cache_file, "r");
  if(fp == (FILE *) NULL)
    {
      syslog(LOG_ERR, "lu_afs: cannot open %s", cache_file);
      return(BUILD_ERR);
    }

  if(fgets(lbuf, sizeof(lbuf), fp) == (char *) NULL)
    {
      fclose(fp);
      return(0);
    }

  fclose(fp);

  cp = rindex(lbuf, ':');
  if(cp++ == (char) NULL)
    return(0);
    
  return(atoi(cp));
}

#endif AFS
#endif MIT
