/*
 * This is the MIT supplement to the PSI/NYSERNet implementation of SNMP.
 * This file describes the RVD (Remote Virtual Disk) portion of the mib.
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
 *    $Source: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/rvd_grp.c,v $
 *    $Author: vrt $
 *    $Locker:  $
 *    $Log: not supported by cvs2svn $
 * Revision 2.1  93/06/18  14:35:46  root
 * first cut at solaris port
 * 
 * Revision 2.0  92/04/22  01:56:13  tom
 * release 7.4
 * 	no changes
 * 
 * Revision 1.3  90/05/26  13:40:30  tom
 * athena release 7.0e
 * 
 * Revision 1.2  90/04/26  17:59:06  tom
 * *** empty log message ***
 * 
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/rvd_grp.c,v 2.2 1994-03-07 15:15:06 vrt Exp $";
#endif

#include "include.h"

#ifdef MIT
#ifdef RVD

/* 
 * this gets set again
 */

#ifdef UNUSED
#undef UNUSED
#endif UNUSED

#ifndef SOLARIS
#include <machineio/vdreg.h>
#endif

char    *vd_control_name = "/dev/rvdcontrol";
int     vdcntrl;

/*
 * Function:    lu_rvdcl()
 * Description: Top level callback for RVD. Supports client side RVD stats.
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

int
lu_rvdcl(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  struct  vd_longstat     vd_longstat;
  struct  vd_longstat     *stats = &vd_longstat;

  if (varnode->flags & NOT_AVAIL ||
      varnode->offset <= 0 ||     /* not expecting offset here */
      ((varnode->flags & NULL_OBJINST) && (reqflg == NXT)))
    return (BUILD_ERR);

  /* 
   * Get RVD data
   */

   if((vdcntrl = open(vd_control_name, O_RDONLY, 0)) < 0) 
     {
       syslog(LOG_ERR, "lu_rvd: Cannot open rvd device");
       return(BUILD_ERR);
     }
  
    if(ioctl(vdcntrl, VDIOCGETSTAT, &stats)) 
      {
	syslog(LOG_ERR, "%s", perror("lu_rvd"));
	close(vdcntrl);
	return(BUILD_ERR);
      }
  close(vdcntrl);

  /*
   * Build reply
   */

  bcopy ((char *)varnode->var_code, (char *) &repl->name, sizeof(repl->name));
  repl->name.ncmp++;                    /* include the "0" instance */

  repl->val.type = CNTR;  /* True of all the replies */

  switch (varnode->offset) 
    {
    case N_RVDCBADBLOCK:
      repl->val.value.cntr = vd_longstat.vdstat.bad_blk;
      return (BUILD_SUCCESS);
    case N_RVDCBADCKSUM:
      repl->val.value.cntr = vd_longstat.vdstat.bad_cksum;
      return (BUILD_SUCCESS);
    case N_RVDCBADTYPE:
      repl->val.value.cntr = vd_longstat.vdstat.bad_type;
      return (BUILD_SUCCESS);
    case N_RVDCBADSTATE:
      repl->val.value.cntr = vd_longstat.vdstat.bad_state;
      return (BUILD_SUCCESS);
    case N_RVDCBADFORMAT:
      repl->val.value.cntr = vd_longstat.vdstat.bad_frmt;
      return (BUILD_SUCCESS);
    case N_RVDCTIMEOUT:
      repl->val.value.cntr = vd_longstat.vdstat.timeout;
      return (BUILD_SUCCESS);
    case N_RVDCBADNONCE:
      repl->val.value.cntr = vd_longstat.vdstat.bad_nonce;
      return (BUILD_SUCCESS);
    case N_RVDCERRORRECV:
      repl->val.value.cntr = vd_longstat.vdstat.err_rcv;
      return (BUILD_SUCCESS);
    case N_RVDCBADDATA:
      repl->val.value.cntr = vd_longstat.vdstat.bad_data;
      return (BUILD_SUCCESS);
    case N_RVDCBADVERS:
      repl->val.value.cntr = vd_longstat.vdstat.bad_vers;
      return (BUILD_SUCCESS);
    case N_RVDCPKTREJ:
      repl->val.value.cntr = vd_longstat.vdstat.pkt_rej;
      return (BUILD_SUCCESS);
    case N_RVDCPUSH:
      repl->val.value.cntr = vd_longstat.vdstat.pushes;
      return (BUILD_SUCCESS);
    case N_RVDCPKTSENT:
      repl->val.value.cntr = vd_longstat.vdstat.pkts_sent;
      return (BUILD_SUCCESS);
    case N_RVDCPKTRECV:
      repl->val.value.cntr = vd_longstat.vdstat.pkts_rcvd;
      return (BUILD_SUCCESS);
    case N_RVDCQKRETRANS:
      repl->val.value.cntr = vd_longstat.vdstat.rxmts;
      return (BUILD_SUCCESS);
    case N_RVDCLGRETRANS:
      repl->val.value.cntr = vd_longstat.long_rxmts;
      return (BUILD_SUCCESS);
    case N_RVDCBLOCKRD:
      repl->val.value.cntr = vd_longstat.vdstat.blk_rqs;
      return (BUILD_SUCCESS);
    case N_RVDCBLOCKWR:
      repl->val.value.cntr = vd_longstat.vdstat.blk_wrt;
      return (BUILD_SUCCESS);
    default:
      syslog (LOG_ERR, "lurvdv: bad rvd offset: %d", varnode->offset);
      return(BUILD_ERR);
  }
}


#endif RVD
#endif MIT
