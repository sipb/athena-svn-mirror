/*
 *
 */
#include "include.h"

#ifdef ATHENA
#ifdef RVD

#ifdef UNUSED
#undef UNUSED
#endif UNUSED

#include <machineio/vdreg.h>

char    *vd_control_name = "/dev/rvdcontrol";
int     vdcntrl;

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

   if((vdcntrl = open(vd_control_name, O_RDONLY, 0)) < 0) 
     {
       syslog(LOG_ERR, "lu_rvd: Cannot open rvd device");
       return(BUILD_ERR);
     }
  
    if(ioctl(vdcntrl, VDIOCGETSTAT, &stats)) 
      {
	syslog(LOG_ERR, "%s", perror("lu_rvd"));
	return(BUILD_ERR);
      }

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
#endif ATHENA
