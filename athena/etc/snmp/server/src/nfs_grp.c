/*
 *
 */
#include "include.h"

#ifdef ATHENA
#ifdef NFS

struct 
{
  int calls;
  int badcalls;
  int nclget;
  int nclsleep;
  int reqs[32];
} ncl;


struct 
{
  int calls;
  int badcalls;
  int reqs[32];
} nsv;


lu_nfscl(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{

  if (varnode->flags & NOT_AVAIL ||
      varnode->offset <= 0 ||     /* not expecting offset here */
      ((varnode->flags & NULL_OBJINST) && (reqflg == NXT)))
    return (BUILD_ERR);

  if(nl[N_CLSTAT].n_value == 0 )
    {
      syslog(LOG_WARNING, "lu_nfscl: can't get namelists for nfs");
      return(BUILD_ERR);
    }
  
  if(lseek(kmem, (long) nl[N_CLSTAT].n_value, L_SET) != nl[N_CLSTAT].n_value)
    {
     syslog(LOG_WARNING, "lu_nfscl: can't lseek");
     return(BUILD_ERR);
    }
  
  if(read(kmem, &ncl, sizeof(ncl)) != sizeof(ncl))
    {
      syslog(LOG_WARNING, "lu_nfscl: can't read cl");
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
    case N_NFSCCALL:
      repl->val.value.cntr = ncl.calls;
      return (BUILD_SUCCESS);
    case N_NFSCBADCALL:
      repl->val.value.cntr = ncl.badcalls;
      return (BUILD_SUCCESS);
    case N_NFSCNULL:
      repl->val.value.cntr = ncl.reqs[0];
      return (BUILD_SUCCESS);
    case N_NFSCGETADDR:
      repl->val.value.cntr = ncl.reqs[1];
      return (BUILD_SUCCESS);
    case N_NFSCSETADDR:
      repl->val.value.cntr = ncl.reqs[2];
      return (BUILD_SUCCESS);
    case N_NFSCROOT:
      repl->val.value.cntr = ncl.reqs[3];
      return (BUILD_SUCCESS);
    case N_NFSCLOOKUP:
      repl->val.value.cntr = ncl.reqs[4];
      return (BUILD_SUCCESS);
    case N_NFSCREADLINK:
      repl->val.value.cntr = ncl.reqs[5];
      return (BUILD_SUCCESS);
    case N_NFSCREAD:
      repl->val.value.cntr = ncl.reqs[6];
      return (BUILD_SUCCESS);
    case N_NFSCWRCACHE:
      repl->val.value.cntr = ncl.reqs[7];
      return (BUILD_SUCCESS);
    case N_NFSCWRITE:
      repl->val.value.cntr = ncl.reqs[8];
      return (BUILD_SUCCESS);
    case N_NFSCCREATE:
      repl->val.value.cntr = ncl.reqs[9];
      return (BUILD_SUCCESS);
    case N_NFSCREMOVE:
      repl->val.value.cntr = ncl.reqs[10];
      return (BUILD_SUCCESS);
    case N_NFSCRENAME:
      repl->val.value.cntr = ncl.reqs[11];
      return (BUILD_SUCCESS);
    case N_NFSCLINK:
      repl->val.value.cntr = ncl.reqs[12];
      return (BUILD_SUCCESS);
    case N_NFSCSYMLINK:
      repl->val.value.cntr = ncl.reqs[13];
      return (BUILD_SUCCESS);
    case N_NFSCMKDIR:
      repl->val.value.cntr = ncl.reqs[14];
      return (BUILD_SUCCESS);
    case N_NFSCRMDIR:
      repl->val.value.cntr = ncl.reqs[15];
      return (BUILD_SUCCESS);
    case N_NFSCREADDIR:
      repl->val.value.cntr = ncl.reqs[16];
      return (BUILD_SUCCESS);
    case N_NFSCFSSTAT:
      repl->val.value.cntr = ncl.reqs[17];
      return (BUILD_SUCCESS);
    case N_NFSCNCLGET:
      repl->val.value.cntr = ncl.nclget;
      return (BUILD_SUCCESS);
    case N_NFSCNCLSLEEP:
      repl->val.value.cntr = ncl.nclsleep;
      return (BUILD_SUCCESS);
    default:
      syslog (LOG_ERR, "lu_nfscl: bad nfs offset: %d", varnode->offset);
      return(BUILD_ERR);
  }
}






lu_nfssv(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{

  if (varnode->flags & NOT_AVAIL ||
      varnode->offset <= 0 ||     /* not expecting offset here */
      ((varnode->flags & NULL_OBJINST) && (reqflg == NXT)))
    return (BUILD_ERR);

  if(nl[N_SVSTAT].n_value == 0)
    {
      syslog(LOG_WARNING, "lu_nfssv: can't get namelists for nfs");
      return(BUILD_ERR);
    }
  
  if(lseek(kmem, (long) nl[N_SVSTAT].n_value, L_SET) != nl[N_SVSTAT].n_value)
    {
     syslog(LOG_WARNING, "lu_nfssv: can't lseek");
     return(BUILD_ERR);
    }
  
  if(read(kmem, &nsv, sizeof(nsv)) != sizeof(nsv))
    {
      syslog(LOG_WARNING, "lu_nfssv: can't read cl");
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
    case N_NFSSCALL:
      repl->val.value.cntr = nsv.calls;
      return (BUILD_SUCCESS);
    case N_NFSSBADCALL:
      repl->val.value.cntr = nsv.badcalls;
      return (BUILD_SUCCESS);
    case N_NFSSNULL:
      repl->val.value.cntr = nsv.reqs[0];
      return (BUILD_SUCCESS);
    case N_NFSSGETADDR:
      repl->val.value.cntr = nsv.reqs[1];
      return (BUILD_SUCCESS);
    case N_NFSSSETADDR:
      repl->val.value.cntr = nsv.reqs[2];
      return (BUILD_SUCCESS);
    case N_NFSSROOT:
      repl->val.value.cntr = nsv.reqs[3];
      return (BUILD_SUCCESS);
    case N_NFSSLOOKUP:
      repl->val.value.cntr = nsv.reqs[4];
      return (BUILD_SUCCESS);
    case N_NFSSREADLINK:
      repl->val.value.cntr = nsv.reqs[5];
      return (BUILD_SUCCESS);
    case N_NFSSREAD:
      repl->val.value.cntr = nsv.reqs[6];
      return (BUILD_SUCCESS);
    case N_NFSSWRCACHE:
      repl->val.value.cntr = nsv.reqs[7];
      return (BUILD_SUCCESS);
    case N_NFSSWRITE:
      repl->val.value.cntr = nsv.reqs[8];
      return (BUILD_SUCCESS);
    case N_NFSSCREATE:
      repl->val.value.cntr = nsv.reqs[9];
      return (BUILD_SUCCESS);
    case N_NFSSREMOVE:
      repl->val.value.cntr = nsv.reqs[10];
      return (BUILD_SUCCESS);
    case N_NFSSRENAME:
      repl->val.value.cntr = nsv.reqs[11];
      return (BUILD_SUCCESS);
    case N_NFSSLINK:
      repl->val.value.cntr = nsv.reqs[12];
      return (BUILD_SUCCESS);
    case N_NFSSSYMLINK:
      repl->val.value.cntr = nsv.reqs[13];
      return (BUILD_SUCCESS);
    case N_NFSSMKDIR:
      repl->val.value.cntr = nsv.reqs[14];
      return (BUILD_SUCCESS);
    case N_NFSSRMDIR:
      repl->val.value.cntr = nsv.reqs[15];
      return (BUILD_SUCCESS);
    case N_NFSSREADDIR:
      repl->val.value.cntr = nsv.reqs[16];
      return (BUILD_SUCCESS);
    case N_NFSSFSSTAT:
      repl->val.value.cntr = nsv.reqs[17];
      return (BUILD_SUCCESS);
    default:
      syslog (LOG_ERR, "lu_nfssv: bad nfs offset: %d", varnode->offset);
      return(BUILD_ERR);
  }
}

#endif NFS
#endif ATHENA
