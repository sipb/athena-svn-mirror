/*
 * This is the MIT supplement to the PSI/NYSERNet implementation of SNMP.
 * This file describes the kernel stats (vm) portion of the mib.
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
 *    $Source: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/vm_grp.c,v $
 *    $Author: tom $
 *    $Locker:  $
 *    $Log: not supported by cvs2svn $
 * Revision 1.3  90/05/26  13:41:59  tom
 * athena release 7.0e
 * 
 * Revision 1.2  90/04/26  18:42:50  tom
 * *** empty log message ***
 * 
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/vm_grp.c,v 2.0 1992-04-22 01:58:04 tom Exp $";
#endif


#include "include.h"
#include <mit-copyright.h>

#ifdef MIT

#ifndef RSPOS

#include <sys/vm.h>
#include <sys/text.h>
#include <sys/dk.h>

#ifdef decmips
#include <sys/dir.h>
#include <sys/namei.h>
#else  decmips
#include <machine/machparam.h>
#endif /* decmips */

#ifdef VFS
struct  ncstats {
        long    ncs_goodhits;           /* hits that we can reall use */
        long    ncs_badhits;            /* hits we must drop */
        long    ncs_falsehits;          /* hits with id mismatch */
        long    ncs_miss;               /* misses */
        long    ncs_long;               /* long names that ignore cache */
        long    ncs_pass2;              /* names found with passes == 2 */
        long    ncs_2passes;            /* number of times we attempt it */
};
#endif VFS

#define  pgtok(a) ((a) * NBPG/1024)
#if defined(vax)
#define INTS(x) ((x) - (hz + phz))
#else  
#define INTS(x) (x)
#endif 

int hz = 0;
int phz = 0;

static int vmcpu();
static int vmproctotal();
static int vmpgrates();
static int vmsumtotal();
static int vmforkstats();
static int vmtimestats();
static int vmncstats();

#ifndef decmips
static int vmxstats();
#endif

int
lu_vmstat(varnode, repl, instptr, reqflg)
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
  repl->name.ncmp++;                    /* include the "0" instance */

  switch (varnode->offset)
    {
    case N_VMPROCR:
    case N_VMPROCB:
    case N_VMPROCW:
    case N_VMMEMAVM:
    case N_VMMEMFRE:
      repl->val.type = INT;  
      return(vmproctotal(varnode->offset, &(repl->val.value.intgr)));

    case N_VMPAGERE:
    case N_VMPAGEAT:
    case N_VMPAGEPI:
    case N_VMPAGEPO:
    case N_VMPAGEFR:
    case N_VMPAGEDE:
    case N_VMPAGESR:
    case N_VMMISCIN:
    case N_VMMISCSY:
    case N_VMMISCCS:
      repl->val.type = GAUGE;
      return(vmpgrates(varnode->offset, &(repl->val.value.gauge)));

    case N_VMCPUUS:
    case N_VMCPUNI:
    case N_VMCPUSY:
    case N_VMCPUID:
      repl->val.type = INT;
      return(vmcpu(varnode->offset, &(repl->val.value.gauge)));

    case N_VMSWAPIN:
    case N_VMSWAPOUT:
    case N_VMPGSWAPIN:
    case N_VMPGSWAPOUT:
    case N_VMATFAULTS:
    case N_VMPGSEQFREE:
    case N_VMPGREC:
    case N_VMPGFASTREC:
    case N_VMFLRECLAIM:
    case N_VMITBLKPGFAULT:
    case N_VMZFPGCREATE:
    case N_VMZFPGFAULT:
    case N_VMEFPGCREATE:
    case N_VMEFPGFAULT:
    case N_VMSTPGFRE:
    case N_VMITPGFRE:
    case N_VMFFPGCREATE:
    case N_VMFFPGFAULT:
    case N_VMPGSCAN:
    case N_VMCLKREV:
    case N_VMCLKFREE:
    case N_VMCSWITCH:
    case N_VMDINTR:
    case N_VMSINTR:
    case N_VMTRAP:
    case N_VMSYSCALL:
#if defined(vax)
    case N_VMPDMAINTR:
#endif
      repl->val.type = CNTR;  
      return(vmsumtotal(varnode->offset, &(repl->val.value.cntr)));

#ifndef decmips
    case N_VMXACALL:
    case N_VMXAHIT:
    case N_VMXASTICK:
    case N_VMXAFLUSH:
    case N_VMXAUNUSE:
    case N_VMXFRECALL:
    case N_VMXFREINUSE:
    case N_VMXFRECACHE:
    case N_VMXFRESWP:
      repl->val.type = CNTR;  
      return(vmxstats(varnode->offset, &(repl->val.value.cntr)));
#endif

    case N_VMFORK:
    case N_VMFKPAGE:
    case N_VMVFORK:
    case N_VMVFKPAGE:
      repl->val.type = CNTR;  
      return(vmforkstats(varnode->offset, &(repl->val.value.cntr)));

    case N_VMRECTIME:
    case N_VMPGINTIME:
      repl->val.type = INT;
      return(vmtimestats(varnode->offset, &(repl->val.value.intgr)));

#ifdef VFS
    case N_VMNCSGOOD:
    case N_VMNCSBAD:
    case N_VMNCSFALSE:
    case N_VMNCSMISS:
    case N_VMNCSLONG:
    case N_VMNCSTOTAL:
#else  VFS
    case N_VMNCHGOOD:
    case N_VMNCHBAD:
    case N_VMNCHFALSE:
    case N_VMNCHMISS:
    case N_VMNCHLONG:
    case N_VMNCHTOTAL:
#endif VFS
      repl->val.type = CNTR; 
      return(vmncstats(varnode->offset, &(repl->val.value.cntr)));

    default:
      syslog (LOG_ERR, "lu_vmstat: bad offset: %d", varnode->offset);
      return(BUILD_ERR);
    }
}


static int
vmcpu(offset, value)
     int offset;
     unsigned int *value;
{
  long time[CPUSTATES];

  if(lseek(kmem, (long) nl[N_CPTIME].n_value, L_SET) != nl[N_CPTIME].n_value)
    {
     syslog(LOG_ERR, "lu_cpu: can't lseek");
     return(BUILD_ERR);
    }

  if(read(kmem, time, sizeof(time)) != sizeof(time))
    {
      syslog(LOG_ERR, "lu_cpu: can't read time");
      return(BUILD_ERR);
    }
  
  switch(offset)
    {
    case N_VMCPUUS:
      *value = time[0];
      return(BUILD_SUCCESS);
    case N_VMCPUNI:
      *value = time[1];
      return(BUILD_SUCCESS);      
    case N_VMCPUSY:
      *value = time[2];
      return(BUILD_SUCCESS);
    case N_VMCPUID:
      *value = time[3];
      return(BUILD_SUCCESS);      
    default:
      syslog (LOG_ERR, "lu_cpu: bad offset: %d", offset);
      return(BUILD_ERR);
    }
}


static int
vmproctotal(offset, value)
     int offset;
     int *value;
{
  struct vmtotal total;

  if(lseek(kmem, (long) nl[N_TOTAL].n_value, L_SET) != nl[N_TOTAL].n_value)
    {
     syslog(LOG_ERR, "lu_vmstat: can't lseek");
     return(BUILD_ERR);
    }

  if(read(kmem, &total, sizeof(total)) != sizeof(total))
    {
      syslog(LOG_ERR, "lu_vmstat: can't read total");
      return(BUILD_ERR);
    }
  
  switch(offset)
    {
    case N_VMPROCR:
      *value = total.t_rq;
      return(BUILD_SUCCESS);
    case N_VMPROCB:
      *value = total.t_dw + total.t_pw;
      return(BUILD_SUCCESS);
    case N_VMPROCW:
      *value = total.t_sw;
      return(BUILD_SUCCESS);
    case N_VMMEMAVM:
      *value = pgtok(total.t_avm);
      return(BUILD_SUCCESS);
    case N_VMMEMFRE:
      *value = pgtok(total.t_free);
      return(BUILD_SUCCESS);
    default:
      syslog (LOG_ERR, "lu_vmstat: bad offset: %d", offset);
      return(BUILD_ERR);
    }
}


static int
vmpgrates(offset, value)
     int offset;
     unsigned int *value;
{
  struct vmmeter rate;
  int nintv = 0;
  int btime = 0;
  int deficit;
  int hztemp = hz;

  if(lseek(kmem, (long) nl[N_HZ].n_value, L_SET) != nl[N_HZ].n_value)
    {
     syslog(LOG_ERR, "lu_vmstat: can't lseek to hz");
     return(BUILD_ERR);
    }

  if(read(kmem, &hztemp, sizeof(hz)) != sizeof(hz))
    {
      syslog(LOG_ERR, "lu_vmstat: can't read hz");
      return(BUILD_ERR);
    }

  if (nl[N_PHZ].n_value != 0)
    {
      if(lseek(kmem, (long) nl[N_PHZ].n_value, L_SET) != nl[N_PHZ].n_value)
	{
	  syslog(LOG_ERR, "lu_vmstat: can't lseek to phz");
	  return(BUILD_ERR);
	}

      if(read(kmem, &phz, sizeof(phz)) != sizeof(phz))
	{
	  syslog(LOG_ERR, "lu_vmstat: can't read phz");
	  return(BUILD_ERR);
	}
    }
  
  if(lseek(kmem, (long) nl[N_BOOT].n_value, L_SET) != nl[N_BOOT].n_value)
    {
      syslog(LOG_ERR, "lu_vmstat: can't lseek to boottime");
      return(BUILD_ERR);
    }

  if(read(kmem, &btime, sizeof(btime)) != sizeof(btime))
    {
      syslog(LOG_ERR, "lu_vmstat: can't read boottime");
      return(BUILD_ERR);
    }

  nintv = time(0) - btime;
  if((nintv <= 0) || nintv > 60 * 60 * 24 * 365 * 10)
    {
      syslog(LOG_WARNING, "lu_vmstat: time is funny %d (ha ha)", nintv);
      return(BUILD_ERR);
    }

  if(lseek(kmem, (long) nl[N_RATE].n_value, L_SET) != nl[N_RATE].n_value)
    {
     syslog(LOG_ERR, "lu_vmstat: can't lseek");
     return(BUILD_ERR);
    }

  if(read(kmem, &rate, sizeof(rate)) != sizeof(rate))
    {
      syslog(LOG_ERR, "lu_vmstat: can't read rate");
      return(BUILD_ERR);
    }
  
  if(lseek(kmem, (long) nl[N_DEFICIT].n_value, L_SET) != nl[N_DEFICIT].n_value)
    {
     syslog(LOG_ERR, "lu_vmstat: can't lseek");
     return(BUILD_ERR);
    }

  if(read(kmem, &deficit, sizeof(deficit)) != sizeof(deficit))
    {
      syslog(LOG_ERR, "lu_vmstat: can't read deficit");
      return(BUILD_ERR);
    }

  switch(offset)
    {
    case N_VMPAGERE:
      *value = (rate.v_pgrec - (rate.v_xsfrec+rate.v_xifrec))/nintv;
      return(BUILD_SUCCESS);
    case N_VMPAGEAT:
      *value = (rate.v_xsfrec+rate.v_xifrec)/nintv;
      return(BUILD_SUCCESS);
    case N_VMPAGEPI:
      *value = pgtok(rate.v_pgin)/nintv;
      return(BUILD_SUCCESS);
    case N_VMPAGEPO:
      *value = pgtok(rate.v_pgpgout)/nintv;
      return(BUILD_SUCCESS);
    case N_VMPAGEFR:
      *value = pgtok(rate.v_dfree)/nintv;
      return(BUILD_SUCCESS);
    case N_VMPAGEDE:
      *value = pgtok(deficit);
      return(BUILD_SUCCESS);
    case N_VMPAGESR:
      *value = rate.v_scan/nintv;
      return(BUILD_SUCCESS);
    case N_VMMISCIN:
      *value = INTS(rate.v_intr/nintv);
      return(BUILD_SUCCESS);
    case N_VMMISCSY:      
      *value = rate.v_syscall/nintv;
      return(BUILD_SUCCESS);
    case N_VMMISCCS:    
      *value = rate.v_swtch/nintv;
      return(BUILD_SUCCESS);
    default:
      syslog (LOG_ERR, "lu_vmstat: bad offset: %d", offset);
      return(BUILD_ERR);
    }
}


static int
vmsumtotal(offset, value)
     int offset;
     unsigned int *value;
{
  struct vmmeter sum;

  if(lseek(kmem, (long) nl[N_SUM].n_value, L_SET) != nl[N_SUM].n_value)
    {
     syslog(LOG_ERR, "lu_vmstat: can't lseek");
     return(BUILD_ERR);
    }

  if(read(kmem, &sum, sizeof(sum)) != sizeof(sum))
    {
      syslog(LOG_ERR, "lu_vmstat: can't read sum");
      return(BUILD_ERR);
    }
  
  switch(offset)
    {
    case N_VMSWAPIN:
      *value = sum.v_swpin;
      return(BUILD_SUCCESS);
    case N_VMSWAPOUT:
      *value = sum.v_swpout;
      return(BUILD_SUCCESS);
    case N_VMPGSWAPIN:
      *value = sum.v_pswpin / CLSIZE;
      return(BUILD_SUCCESS);
    case N_VMPGSWAPOUT:
      *value = sum.v_pswpout / CLSIZE;
      return(BUILD_SUCCESS);
    case N_VMATFAULTS:
      *value = sum.v_faults;
      return(BUILD_SUCCESS);
    case N_VMPGSEQFREE:
      *value = sum.v_seqfree;
      return(BUILD_SUCCESS);
    case N_VMPGREC:
      *value = sum.v_pgrec;
      return(BUILD_SUCCESS);
    case N_VMPGFASTREC:
      *value = sum.v_fastpgrec;
      return(BUILD_SUCCESS);
    case N_VMFLRECLAIM:
      *value = sum.v_pgrec;
      return(BUILD_SUCCESS);
    case N_VMITBLKPGFAULT:
      *value = sum.v_intrans;
      return(BUILD_SUCCESS);
    case N_VMZFPGCREATE:
      *value = sum.v_nzfod / CLSIZE;
      return(BUILD_SUCCESS);
    case N_VMZFPGFAULT:
      *value = sum.v_zfod / CLSIZE;
      return(BUILD_SUCCESS);
    case N_VMEFPGCREATE:
      *value = sum.v_nexfod / CLSIZE;
      return(BUILD_SUCCESS);
    case N_VMEFPGFAULT:
      *value = sum.v_exfod / CLSIZE;
      return(BUILD_SUCCESS);
    case N_VMSTPGFRE:
      *value = sum.v_xsfrec;
      return(BUILD_SUCCESS);
    case N_VMITPGFRE:
      *value = sum.v_xifrec;
      return(BUILD_SUCCESS);
    case N_VMFFPGCREATE:
      *value = sum.v_nvrfod / CLSIZE;
      return(BUILD_SUCCESS);
    case N_VMFFPGFAULT:
      *value = sum.v_vrfod / CLSIZE;
      return(BUILD_SUCCESS);
    case N_VMPGSCAN:
      *value = sum.v_scan;
      return(BUILD_SUCCESS);
    case N_VMCLKREV:
      *value = sum.v_rev;
      return(BUILD_SUCCESS);
    case N_VMCLKFREE:
      *value = sum.v_dfree;
      return(BUILD_SUCCESS);
    case N_VMCSWITCH:
      *value = sum.v_swtch;
      return(BUILD_SUCCESS);
    case N_VMDINTR:
      *value = sum.v_intr;
      return(BUILD_SUCCESS);
    case N_VMSINTR:
      *value = sum.v_soft;
      return(BUILD_SUCCESS);
    case N_VMTRAP:
      *value = sum.v_trap;
      return(BUILD_SUCCESS);
    case N_VMSYSCALL:
      *value = sum.v_syscall;
      return(BUILD_SUCCESS);
#if defined(vax)
    case N_VMPDMAINTR:
      *value = sum.v_pdma;
      return(BUILD_SUCCESS);
#endif
    default:
      syslog (LOG_ERR, "lu_vmstat: bad offset: %d", offset);
      return(BUILD_ERR);
    }
}


#ifndef decmips

static int
vmxstats(offset, value)
     int offset;
     unsigned int *value;
{
  struct xstats xstats;

  if(lseek(kmem, (long) nl[N_XSTATS].n_value, L_SET) != nl[N_XSTATS].n_value)
    {
     syslog(LOG_ERR, "lu_vmstat: can't lseek to xstats");
     return(BUILD_ERR);
    }

  if(read(kmem, &xstats, sizeof(xstats)) != sizeof(xstats))
    {
      syslog(LOG_ERR, "lu_vmstat: can't read xstats");
      return(BUILD_ERR);
    }
  
  switch(offset)
    {
    case N_VMXACALL:
      *value = xstats.alloc;
      return(BUILD_SUCCESS);
    case N_VMXAHIT:
      *value = xstats.alloc_cachehit;
      return(BUILD_SUCCESS);
    case N_VMXASTICK:
      *value = xstats.alloc_inuse;
      return(BUILD_SUCCESS);
    case N_VMXAFLUSH:
      *value = xstats.alloc_cacheflush;
      return(BUILD_SUCCESS);
    case N_VMXAUNUSE:
      *value = xstats.alloc_unused;
      return(BUILD_SUCCESS);
    case N_VMXFRECALL:
      *value = xstats.free;
      return(BUILD_SUCCESS);
    case N_VMXFREINUSE:
      *value = xstats.free_inuse;
      return(BUILD_SUCCESS);
    case N_VMXFRECACHE:
      *value = xstats.free_cache;
      return(BUILD_SUCCESS);
    case N_VMXFRESWP:
      *value = xstats.free_cacheswap;
      return(BUILD_SUCCESS);
    default:
      syslog (LOG_ERR, "lu_vmstat: bad offset: %d", offset);
      return(BUILD_ERR);
    }
}


#endif


static int
vmforkstats(offset, value)
     int offset;
     unsigned int *value;
{
  struct forkstat forkstat;

  if(lseek(kmem, (long) nl[N_FORKSTAT].n_value, L_SET) !=
     nl[N_FORKSTAT].n_value)
    {
      syslog(LOG_ERR, "lu_vmstat: can't lseek to fork stats");
      return(BUILD_ERR);
    }

  if(read(kmem, &forkstat, sizeof(forkstat)) != sizeof(forkstat))
    {
      syslog(LOG_ERR, "lu_vmstat: can't read forkstats");
      return(BUILD_ERR);
    }
  
  switch(offset)
    {
    case N_VMFORK:
      *value = forkstat.cntfork;
      return(BUILD_SUCCESS);
    case N_VMFKPAGE:
      *value = forkstat.sizfork;
      return(BUILD_SUCCESS);
    case N_VMVFORK:
      *value = forkstat.cntvfork;
      return(BUILD_SUCCESS);
    case N_VMVFKPAGE:
      *value = forkstat.sizvfork;
      return(BUILD_SUCCESS);
    default:
      syslog (LOG_ERR, "lu_vmstat: bad offset: %d", offset);
      return(BUILD_ERR);
    }
}



static int
vmtimestats(offset, value)
     int offset;
     unsigned int *value;
{
  int rectime;
  int pgintime;
  
  switch(offset)
    {
    case N_VMRECTIME:      
      if(lseek(kmem, (long) nl[N_REC].n_value, L_SET) != nl[N_REC].n_value)
	{
	  syslog(LOG_ERR, "lu_vmstat: can't lseek to rectime");
	  return(BUILD_ERR);
	}
      
      if(read(kmem, &rectime, sizeof(rectime)) != sizeof(rectime))
	{
	  syslog(LOG_ERR, "lu_vmstat: can't read rectime");
	  return(BUILD_ERR);
	}
      
      *value = rectime;
      return(BUILD_SUCCESS);

    case N_VMPGINTIME:
      if(lseek(kmem, (long) nl[N_PGIN].n_value, L_SET) != nl[N_PGIN].n_value)
	{
	  syslog(LOG_ERR, "lu_vmstat: can't lseek to pgintime");
	  return(BUILD_ERR);
	}
      
      if(read(kmem, &pgintime, sizeof(pgintime)) != sizeof(pgintime))
	{
	  syslog(LOG_ERR, "lu_vmstat: can't read pgintime");
	  return(BUILD_ERR);
	}
      
      *value = pgintime;
      return(BUILD_SUCCESS);
      
    default:
      syslog (LOG_ERR, "lu_vmstat: bad offset: %d", offset);
      return(BUILD_ERR);
    }
} 


static int
vmncstats(offset, value)
     int offset;
     unsigned int *value;
{ 
#ifdef VFS
  struct ncstats ncstats;
#else  VFS
  struct nchstats nchstats;
#endif VFS

#ifdef VFS
  if(lseek(kmem, (long) nl[N_NCSTATS].n_value, L_SET) != nl[N_NCSTATS].n_value)
    {
     syslog(LOG_ERR, "lu_vmstat: can't lseek to ncstats");
     return(BUILD_ERR);
    }

  if(read(kmem, &ncstats, sizeof(ncstats)) != sizeof(ncstats))
    {
      syslog(LOG_ERR, "lu_vmstat: can't read ncstats");
      return(BUILD_ERR);
    }
  
  switch(offset)
    {
    case N_VMNCSGOOD:
      *value = ncstats.ncs_goodhits;
      return(BUILD_SUCCESS);
    case N_VMNCSBAD:
      *value = ncstats.ncs_badhits;
      return(BUILD_SUCCESS);
    case N_VMNCSFALSE:
      *value = ncstats.ncs_falsehits;
      return(BUILD_SUCCESS);
    case N_VMNCSMISS:
      *value = ncstats.ncs_miss;
      return(BUILD_SUCCESS);
    case N_VMNCSLONG:
      *value = ncstats.ncs_long;
      return(BUILD_SUCCESS);
    case N_VMNCSTOTAL:
      *value = ncstats.ncs_goodhits + ncstats.ncs_long + 
	ncstats.ncs_falsehits + ncstats.ncs_miss + ncstats.ncs_badhits;
      return(BUILD_SUCCESS);
    default:
      syslog (LOG_ERR, "lu_vmstat: bad offset: %d", offset);
      return(BUILD_ERR);
    }

#else  VFS

  if(lseek(kmem, (long) nl[N_NCHSTATS].n_value, L_SET) != 
     nl[N_NCHSTATS].n_value)
    {
     syslog(LOG_ERR, "lu_vmstat: can't lseek to nchstats");
     return(BUILD_ERR);
    }

  if(read(kmem, &nchstats, sizeof(nchstats)) != sizeof(nchstats))
    {
      syslog(LOG_ERR, "lu_vmstat: can't read nchstats");
      return(BUILD_ERR);
    }
  
  switch(offset)
    {
    case N_VMNCHGOOD:
      *value = nchstats.ncs_goodhits;
      return(BUILD_SUCCESS);
    case N_VMNCHBAD:
      *value = nchstats.ncs_badhits;
      return(BUILD_SUCCESS);
    case N_VMNCHFALSE:
      *value = nchstats.ncs_falsehits;
      return(BUILD_SUCCESS);
    case N_VMNCHMISS:
      *value = nchstats.ncs_miss;
      return(BUILD_SUCCESS);
    case N_VMNCHLONG:
      *value = nchstats.ncs_long;
      return(BUILD_SUCCESS);
    case N_VMNCHTOTAL:
      *value = nchstats.ncs_goodhits + nchstats.ncs_long + 
	nchstats.ncs_falsehits + nchstats.ncs_miss + nchstats.ncs_badhits;
      return(BUILD_SUCCESS);
    default:
      syslog (LOG_ERR, "lu_vmstat: bad offset: %d", offset);
      return(BUILD_ERR);
    }
#endif VFS
}

#endif /* RSPOS */
#endif MIT


