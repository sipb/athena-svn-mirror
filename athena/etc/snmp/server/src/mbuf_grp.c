/*
 * This is the MIT supplement to the PSI/NYSERNet implementation of SNMP.
 * This file describes the unix mbuf portion of the mib as specified
 * by the unix mib by Rose & Sklower.
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
 *    $Source: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/mbuf_grp.c,v $
 *    $Author: ghudson $
 *    $Locker:  $
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/mbuf_grp.c,v 1.2 1997-02-27 06:47:36 ghudson Exp $";
#endif


#include "include.h"
#include <mit-copyright.h>

#ifdef MIT

#define NTYPES 13

#include <sys/mbuf.h>

/*
 * Function:    lu_mbuf()
 * Description: Top level callback for the MBUF group. Supports the following:
 *                    N_MBUFS:    Number of mbufs
 *                    N_MBUSCLUS:
 *                    N_MBUFFREECLUS:
 *                    N_MBUFWAIT:
 *                    N_MBUFDRAIN:
 *                    N_MBUFFREE:
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

lu_mbuf(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  struct mbstat mb;
  int num;

  if (varnode->flags & NOT_AVAIL ||
      varnode->offset <= 0)
    return (BUILD_ERR);

  if((varnode->offset != N_MBUFTYPE) && (varnode->offset != N_MBUFALLOC))
    {
      if((varnode->flags & NULL_OBJINST) && (reqflg & NXT))
	return(BUILD_ERR);
      repl->name.ncmp++;                    /* include the "0" instance */
    }

  klseek(kmem, nl[N_MBUF], L_SET);
  if(read(kmem, (char *) &mb, sizeof(mb)) != sizeof(mb)) 
    {
      syslog(LOG_ERR, "lu_mbuf: can't lseek to n");
      return(BUILD_ERR);      
    }

  /*
   * Build reply
   */

  memcpy (&repl->name, varnode->var_code, sizeof(repl->name));

  repl->val.type = CNTR;  /* True of all the replies */
    
  switch(varnode->offset)
    {
    case N_MBUFS:
      repl->val.value.cntr = mb.m_mbufs;
      return(BUILD_SUCCESS);
    case N_MBUFCLUS:
      repl->val.value.cntr = mb.m_clusters;
      return(BUILD_SUCCESS);
    case N_MBUFFREECLUS:
      repl->val.value.cntr = mb.m_clfree;
      return(BUILD_SUCCESS);
    case N_MBUFDROP:
      repl->val.value.cntr = mb.m_drops;
      return(BUILD_SUCCESS);
    case N_MBUFWAIT:
      repl->val.value.cntr = mb.m_wait;
      return(BUILD_SUCCESS);
    case N_MBUFDRAIN:
      repl->val.value.cntr = mb.m_drain;
      return(BUILD_SUCCESS);
    case N_MBUFFREE:
      repl->val.value.cntr = mb.m_types[MT_FREE];
      return(BUILD_SUCCESS);
    case N_MBUFTYPE:
    case N_MBUFALLOC:
      if((instptr == (objident *) NULL) || (instptr->ncmp == 0))
	num = 1;
      else 
	num = instptr->cmp[0];
      
      if((reqflg & NXT) && (instptr != (objident *) NULL) && 
	 (instptr->ncmp != 0))
	num++;
      
      if(num > NTYPES)
	return(BUILD_ERR);

      repl->name.cmp[repl->name.ncmp] = num;
      repl->name.ncmp++;	

      if(varnode->offset == N_MBUFTYPE)
	{
	  repl->value.type = INT;
	  repl->value.val.intgr = num;
	}
      else
	repl->value.val.cntr = mb_m_mtypes[num];
      return(BUILD_SUCCESS);

    default:
      syslog (LOG_ERR, "lu_mbuf: bad offset: %d", varnode->offset);
      return(BUILD_ERR);
    }
}

#endif /* MIT */
