/*
 * This is the MIT supplement to the PSI/NYSERNet implementation of SNMP.
 * This file describes the BSD process stats portion of the mib.
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
 *    $Source: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/ps_grp.c,v $
 *    $Author: ghudson $
 *    $Locker:  $
 *    $Log: not supported by cvs2svn $
 *    Revision 2.1  1993/06/18 14:35:40  root
 *    first cut at solaris port
 *
 * Revision 2.0  92/04/22  01:58:31  tom
 * release 7.4
 * 	support for decmips
 * 
 * Revision 1.3  90/05/26  13:40:18  tom
 * athena release 7.0e
 * 
 * Revision 1.2  90/04/26  17:46:22  tom
 * added function declarations
 * 
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/ps_grp.c,v 2.2 1997-02-27 06:47:40 ghudson Exp $";
#endif


#include "include.h"
#include <mit-copyright.h>

#ifdef MIT
#ifndef SOLARIS
#ifndef RSPOS 

#include <sys/proc.h>
#include <sys/text.h>

#ifdef VFS
#include <sys/vnode.h>
#include <ufs/inode.h>
#else  VFS
#include <sys/inode.h>
#endif VFS

#include <sys/map.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/conf.h>
#include <sys/vm.h>
#include <machine/pte.h>

#ifdef decmips
#define inode gnode
#define i_count g_count
#endif decmips

#ifdef decmips
struct  file 
{
  int     f_flag;         
  short   f_type;         
  short   f_count;        
  short   f_msgcount;     
  struct  fileops 
    {
      int     (*fo_rw)();
      int     (*fo_ioctl)();
      int     (*fo_select)();
      int     (*fo_close)();
    } *f_ops;
  caddr_t f_data;         
  off_t  f_offset;
  struct ucred *f_cred;
};

#else  decmips
#define KERNEL
#include <sys/file.h>
#undef  KERNEL
#endif  decmips

#define btok(x) ((x) / (1024 / DEV_BSIZE))

int dmmax; /* for up */
int dmmin;

static int psswap();
static int psfile();
static int pstext();
static int psinode();
static int psproc();
static int up();
static int vusize();


/*
 * Function:    lu_nfscl()
 * Description: Top level callback for NFS. Supports the following:
 *                    N_PSTOTAL: (INT) total swap space configured
 *                    N_PSUSED:  (INT) swap used
 *                    N_PSTUEED: (INT) swap ued for text
 *                    N_PSFREE:  (INT) free swap
 *                    N_PSWASTED:(INT) wasted swap
 *                    N_MISSING: (INT) missing swap
 *                    N_PFTOTAL: (INT) max files
 *                    N_PFUSED:  (INT) files used
 *                    N_PITOTAL: (INT) max inodes
 *                    N_PIUSED:  (INT) used inodes
 *                    N_PPTOTAL: (INT) max procs
 *                    N_PPUSED:  (INT) used procs
 *                    N_PTTOTAL: (INT) total texts
 *                    N_PTUSED:  (INT) texts used
 *                    N_PTACTIVE:(INT) active texts
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

lu_pstat(varnode, repl, instptr, reqflg)
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

  memcpy (&repl->name, varnode->var_code, sizeof(repl->name));
  repl->name.ncmp++;                    /* include the "0" instance */

  repl->val.type = INT;  /* True of all the replies */

  switch(varnode->offset)
    {
    case N_PSTOTAL:
    case N_PSUSED:
    case N_PSTUSED:
    case N_PSFREE:
    case N_PSWASTED:
    case N_PSMISSING:
      return(psswap(varnode->offset, &(repl->val.value.intgr)));
      
    case N_PFTOTAL:
    case N_PFUSED:
      return(psfile(varnode->offset, &(repl->val.value.intgr)));
      
    case N_PITOTAL:
    case N_PIUSED:
      return(psinode(varnode->offset, &(repl->val.value.intgr)));
      
    case N_PPTOTAL:
    case N_PPUSED:
      return(psproc(varnode->offset, &(repl->val.value.intgr)));
      
    case N_PTTOTAL:
    case N_PTACTIVE:
    case N_PTUSED:
      return(pstext(varnode->offset, &(repl->val.value.intgr)));
      
    default:
      syslog (LOG_ERR, "lu_pstat: bad offset: %d", varnode->offset);
      return(BUILD_ERR);
    }
}




/*
 * Function:    psinode
 * Description: collect inode information
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

static int
psinode(offset, value)
     int offset;
     int *value;
{
  struct inode *in;
  register struct inode *ip;
  register int nin = 0;
  int ai;
  int n = 0;
  
  if(lseek(kmem, (long) nl[N_NINODE].n_value, L_SET) !=
     nl[N_NINODE].n_value)
    {
      syslog(LOG_ERR, "lu_pstat: can't lseek to n");
      return(BUILD_ERR);
    }
  
  if(read(kmem, &n, sizeof(n)) != sizeof(n))
    {
      syslog(LOG_ERR, "lu_pstat: can't read n");
      return(BUILD_ERR);
    }


  switch(offset)
    {
    case N_PITOTAL:
      *value = n;
      return(BUILD_SUCCESS);
      
    case N_PIUSED:      
      if((n < 0) || (n > 10000))
	{
	  syslog(LOG_ERR, "lu_pstat: n out of bounds (%d)", n);
	  return(BUILD_ERR);
	}
     
      in = (struct inode *) calloc(n, sizeof(struct inode));
      if(in == (struct inode *) 0)
	{
	  syslog(LOG_ERR, "lu_pstat: calloc failed");
	  return(BUILD_ERR);
	}
         
      if(lseek(kmem, (long) nl[N_INODE].n_value, L_SET) != nl[N_INODE].n_value)
	{
	  syslog(LOG_ERR, "lu_pstat: can't lseek to inode");
	  return(BUILD_ERR);
	}     
      read(kmem, &ai, sizeof(long));

      if(lseek(kmem, (long) ai, L_SET) != ai)
	{
	  syslog(LOG_ERR, "lu_pstat: can't lseek to ainode");
	  return(BUILD_ERR);
	} 
      read(kmem, in, n * sizeof(struct inode));

      for(ip = in; ip < &in[n]; ip++)
	{

#ifdef VFS
	  if(ip->i_vnode.v_count)
#else  VFS
	  if(ip->i_count)
#endif VFS
	    ++nin;
	}

      *value = nin;
      free(in);
      return(BUILD_SUCCESS);
 
    default:
      syslog (LOG_ERR, "lu_pstat: bad offset: %d", offset);
      return(BUILD_ERR);
    }
}



/*
 * Function:    pstext
 * Description: collect text information
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

static int
pstext(offset, value)
     int offset;
     int *value;
{
  struct text *tt;
  register struct text *tp;
  register int ntx  = 0;
  register int ntxa = 0;
  long atext;
  int n = 0;
  
  if(lseek(kmem, (long) nl[N_NTEXT].n_value, L_SET) !=  nl[N_NTEXT].n_value)
    {
      syslog(LOG_ERR, "lu_pstext: can't lseek to n");
      return(BUILD_ERR);
    }
  
  if(read(kmem, &n, sizeof(n)) != sizeof(n))
    {
      syslog(LOG_ERR, "lu_pstat: can't read n");
      return(BUILD_ERR);
    }


  switch(offset)
    {
    case N_PTTOTAL:
      *value = n;
      return(BUILD_SUCCESS);
      
    case N_PTUSED:
    case N_PTACTIVE:
      if((n < 0) || (n > 10000))
	{
	  syslog(LOG_ERR, "lu_pstat: n out of bounds (%d)", n);
	  return(BUILD_ERR);
	}
     
      tt = (struct text *) calloc(n, sizeof(struct text));
      if(tt == (struct text *) 0)
	{
	  syslog(LOG_ERR, "lu_pstat: calloc failed");
	  return(BUILD_ERR);
	}
        
      if(lseek(kmem, (long) nl[N_XTEXT].n_value, L_SET) != nl[N_XTEXT].n_value)
	{
	  syslog(LOG_ERR, "lu_pstext: can't lseek to text");
	  return(BUILD_ERR);
	}
      read(kmem, &atext, sizeof(long));

      if(lseek(kmem, (long) atext, L_SET) != atext)
	{
	  syslog(LOG_ERR, "lu_pstext: can't lseek to atext");
	  return(BUILD_ERR);
	}
      read(kmem, tt, n * sizeof(struct text));

      for(tp = tt; tp < &tt[n]; tp++)
	{
#ifdef VFS 
	  if(tp->x_vptr != NULL)
#else  VFS
	  if(tp->x_iptr != NULL)
#endif VFS
	    ++ntxa;
	  if(tp->x_count != 0)
	    ++ntx;
	}

      if(offset == N_PTUSED)
	*value = ntxa;
      else
	*value = ntx;

      free(tt);
      return(BUILD_SUCCESS);
 
    default:
      syslog (LOG_ERR, "lu_pstat: bad offset: %d", offset);
      return(BUILD_ERR);
    }
}




/*
 * Function:    psproc
 * Description: collect proc information
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

static int
psproc(offset, value)
     int offset;
     int *value;
{
  struct proc *xproc;
  register struct proc *pp;
  int n = 0;
  int nproc = 0;
  long aproc;

  if(lseek(kmem, (long) nl[N_NPROC].n_value, L_SET) != nl[N_NPROC].n_value)
    {
      syslog(LOG_ERR, "lu_psstat: can't lseek to n");
      return(BUILD_ERR);
    }
  
  if(read(kmem, &n, sizeof(n)) != sizeof(n))
    {
      syslog(LOG_ERR, "lu_pstat: can't read n");
      return(BUILD_ERR);
    }


  switch(offset)
    {
    case N_PPTOTAL:
      *value = n;
      return(BUILD_SUCCESS);
      
    case N_PPUSED:
      if((n < 0) || (n > 10000))
	{
	  syslog(LOG_ERR, "lu_pstat: n out of bounds (%d)", n);
	  return(BUILD_ERR);
	}
     
      xproc = (struct proc *) calloc(n, sizeof(struct proc));
      if(xproc == (struct proc *) 0)
	{
	  syslog(LOG_ERR, "lu_pstat: calloc failed");
	  return(BUILD_ERR);
	}

      if(lseek(kmem, (long) nl[N_PROC].n_value, L_SET) != nl[N_PROC].n_value)
	{
	  syslog(LOG_ERR, "lu_psstat: can't lseek to proc");
	  return(BUILD_ERR);
	}      
      read(kmem, &aproc, sizeof(long));

      if(lseek(kmem, (long) aproc, L_SET) != aproc)
	{
	  syslog(LOG_ERR, "lu_psstat: can't lseek to aproc");
	  return(BUILD_ERR);
	}    
      read(kmem, xproc, n * sizeof(struct proc));

      for(pp = xproc; pp < &xproc[n]; pp++)
	{
	  if(pp->p_stat)
	    ++nproc;
	}

      *value = nproc;
      free(xproc);
      return(BUILD_SUCCESS);
 
    default:
      syslog (LOG_ERR, "lu_pstat: bad offset: %d", offset);
      return(BUILD_ERR);
    }
}

  


/*
 * Function:    psfile
 * Description: collect file information
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

static int
psfile(offset, value)
     int offset;
     int *value;
{
  register struct file *fp;
  struct file *xfile;
  int n = 0;
  int nfile = 0;
  long afile;

  if(lseek(kmem, (long) nl[N_NFILE].n_value, L_SET) != nl[N_NFILE].n_value)
    {
      syslog(LOG_ERR, "lu_pstat: can't lseek to n");
      return(BUILD_ERR);
    }
  
  if(read(kmem, &n, sizeof(n)) != sizeof(n))
    {
      syslog(LOG_ERR, "lu_pstat: can't read n");
      return(BUILD_ERR);
    }


  switch(offset)
    {
    case N_PFTOTAL:
      *value = n;
      return(BUILD_SUCCESS);
      
    case N_PFUSED:
      if((n < 0) || (n > 10000))
	{
	  syslog(LOG_ERR, "lu_pstat: n out of bounds (%d)", n);
	  return(BUILD_ERR);
	}
     
      xfile = (struct file *) calloc(n, sizeof(struct file));
      if(xfile == (struct file *) 0)
	{
	  syslog(LOG_ERR, "lu_pstat: calloc failed");
	  return(BUILD_ERR);
	}
      
      if(lseek(kmem, (long) nl[N_FILE].n_value, L_SET) != nl[N_FILE].n_value)
	{
	  syslog(LOG_ERR, "lu_pstat: can't lseek to file addr");
	  return(BUILD_ERR);
	}  
      read(kmem, &afile, sizeof(long));

      if(lseek(kmem, (long) afile, L_SET) != afile)
	{
	  syslog(LOG_ERR, "lu_pstat: can't lseek to file");
	  return(BUILD_ERR);
	} 
      read(kmem, xfile, n * sizeof(struct file));

      for(fp = xfile; fp < &xfile[n]; fp++)
	{
	  if(fp->f_count)
	    ++nfile;
	}

      *value = nfile;
      free(xfile);
      return(BUILD_SUCCESS);
 
    default:
      syslog (LOG_ERR, "lu_pstat: bad offset: %d", offset);
      return(BUILD_ERR);
    }
}

  



/*
 * Function:    psswap
 * Description: collect swap information
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

static int
psswap(offset, value)
     int offset;
     int *value;
{
  struct proc *xproc;
  int nproc;
  struct text *xtext;
  int ntext;
  struct map *swapmap;
  int nswapmap;
  struct swdevt *swdevt;
  int nswdev;
  register struct proc *pp;
  register struct mapent *me;
  register struct text *xp;
  register struct swdevt *sw;
  int sb = 0;
  int db = 0;
  int nswap = 0;
  int used  = 0;
  int tused = 0;
  int fre = 0;
  int waste = 0;
  long addr;

  /*
   * dmmin & dmmax 
   */

  if(lseek(kmem, (long) nl[N_DMIN].n_value, L_SET) != nl[N_DMIN].n_value)
    {
      syslog(LOG_ERR, "lu_pstat: can't lseek to dmmin");
      return(BUILD_ERR);
    }
  
  if(read(kmem, &dmmin, sizeof(dmmin)) != sizeof(dmmin))
    {
      syslog(LOG_ERR, "lu_pstat: can't read dmmin");
      return(BUILD_ERR);
    }

  if(lseek(kmem, (long) nl[N_DMAX].n_value, L_SET) != nl[N_DMAX].n_value)
    {
      syslog(LOG_ERR, "lu_pstat: can't lseek to dmmax");
      return(BUILD_ERR);
    }
  
  if(read(kmem, &dmmax, sizeof(dmmax)) != sizeof(dmmax))
    {
      syslog(LOG_ERR, "lu_pstat: can't read dmmax");
      return(BUILD_ERR);
    }


  /* 
   * sizes
   */

  if(lseek(kmem, (long) nl[N_NPROC].n_value, L_SET) != nl[N_NPROC].n_value)
    {
      syslog(LOG_ERR, "lu_pstat: can't lseek to nproc");
      return(BUILD_ERR);
    }
  
  if(read(kmem, &nproc, sizeof(nproc)) != sizeof(nproc))
    {
      syslog(LOG_ERR, "lu_pstat: can't read nproc");
      return(BUILD_ERR);
    }

  if(lseek(kmem, (long) nl[N_NTEXT].n_value, L_SET) != nl[N_NTEXT].n_value)
    {
      syslog(LOG_ERR, "lu_pstat: can't lseek to ntext");
      return(BUILD_ERR);
    }
  
  if(read(kmem, &ntext, sizeof(ntext)) != sizeof(ntext))
    {
      syslog(LOG_ERR, "lu_pstat: can't read ntext");
      return(BUILD_ERR);
    }

  if((nproc < 0) || (nproc > 10000) || (ntext < 0) || (ntext > 10000))
    {
      syslog(LOG_ERR, 
	     "lu_pstats: preposterous number of procs or texts (%d %d)",
	     nproc, ntext);
      return(BUILD_ERR);
    }

  if(lseek(kmem, (long) nl[N_NSWDEV].n_value, L_SET) != 
     nl[N_NSWDEV].n_value)
    {
      syslog(LOG_ERR, "lu_pstat: can't lseek to nswdev");
      return(BUILD_ERR);
    }
  
  if(read(kmem, &nswdev, sizeof(nswdev)) != sizeof(nswdev))
    {
      syslog(LOG_ERR, "lu_pstat: can't read nswdev");
      return(BUILD_ERR);
    }

  if(lseek(kmem, (long) nl[N_NSWAPMAP].n_value, L_SET) != 
      nl[N_NSWAPMAP].n_value)
    {
      syslog(LOG_ERR, "lu_pstat: can't lseek to swapmap");
      return(BUILD_ERR);
    }
  
  if(read(kmem, &nswapmap, sizeof(nswapmap)) != sizeof(nswapmap))
    {
      syslog(LOG_ERR, "lu_pstat: can't read swapmap");
      return(BUILD_ERR);
    }

  /*
   * allocation 
   */

  xproc = (struct proc *) calloc(nproc, sizeof(struct proc));
  if(xproc == (struct proc *) NULL)
    {
      syslog(LOG_ERR, "lu_pstat: can't allocate memory for proc table");
      return(BUILD_ERR);
    }

  xtext = (struct text *) calloc(ntext, sizeof(struct text));
  if(xtext == (struct text *) NULL)
    {
      syslog(LOG_ERR, "lu_pstat: can't allocate memory for text table");
      return(BUILD_ERR);
    }

  swapmap = (struct map *) calloc(nswapmap, sizeof(struct map));
  if(swapmap == (struct map *) NULL)
    {
      syslog(LOG_ERR, "lu_pstat: can't allocate memory for swap table");
      return(BUILD_ERR);
    }

  swdevt = (struct swdevt *) calloc(nswdev, sizeof(struct swdevt));
  if(swdevt == (struct swdevt *) NULL)
    {
      syslog(LOG_ERR, "lu_pstat: can't allocate memory for sdev table");
      return(BUILD_ERR);
    }


  /*
   * reads- beware: swdev diving differs from the rest!
   */

  if(lseek(kmem, nl[N_SWDEVT].n_value, L_SET) != nl[N_SWDEVT].n_value)
    {
      syslog(LOG_ERR, "lu_pstat: can't lseek to swdevt addr");
      return(BUILD_ERR);
    }

  if(lseek(kmem, nl[N_SWDEVT].n_value, L_SET) != nl[N_SWDEVT].n_value)
    {
      syslog(LOG_ERR, "lu_pstat: can't lseek to swdevt");
      return(BUILD_ERR);
    }

  if(read(kmem, swdevt, nswdev * sizeof(struct swdevt)) != 
     sizeof(struct swdevt) * nswdev)
    {
      syslog(LOG_ERR, "lu_pstat: can't read swdevt");
      return(BUILD_ERR);
    }

  
  if(lseek(kmem, nl[N_PROC].n_value, L_SET) != nl[N_PROC].n_value)
    {
      syslog(LOG_ERR, "lu_pstat: can't lseek to proc addr");
      return(BUILD_ERR);
    }
  
  if(read(kmem, &addr, sizeof(addr)) != sizeof(addr))
    {
      syslog(LOG_ERR, "lu_pstat: can't read proc addr");
      return(BUILD_ERR);
    }

  if(lseek(kmem, addr, L_SET) != addr)
    {
      syslog(LOG_ERR, "lu_pstat: can't lseek to proc");
      return(BUILD_ERR);
    }

  if(read(kmem, xproc, (nproc * sizeof(struct proc))) != 
     sizeof(struct proc) * nproc)
    {
      syslog(LOG_ERR, "lu_pstat: can't read proc");
      return(BUILD_ERR);
    }


  if(lseek(kmem, nl[N_XTEXT].n_value, L_SET)  != nl[N_XTEXT].n_value)
    {
      syslog(LOG_ERR, "lu_pstat: can't lseek to text addr");
      return(BUILD_ERR);
    }

  if(read(kmem, &addr, sizeof(addr)) != sizeof(addr))
    {
      syslog(LOG_ERR, "lu_pstat: can't read text addr");
      return(BUILD_ERR);
    }

  if(lseek(kmem, addr, L_SET)  != addr)
    {
      syslog(LOG_ERR, "lu_pstat: can't lseek to text");
      return(BUILD_ERR);
    }

  if(read(kmem, xtext, ntext * sizeof(struct text)) != 
     sizeof(struct text) * ntext)
    {
      syslog(LOG_ERR, "lu_pstat: can't read text");
      return(BUILD_ERR);
    }

  
  if(lseek(kmem, nl[N_SWAPMAP].n_value, L_SET) != nl[N_SWAPMAP].n_value)
    {
      syslog(LOG_ERR, "lu_pstat: can't lseek to swapmap addr");
      return(BUILD_ERR);
    }
  
  if(read(kmem, &addr, sizeof(addr)) != sizeof(addr))
    {
      syslog(LOG_ERR, "lu_pstat: can't read swpmap addr");
      return(BUILD_ERR);
    }

  if(lseek(kmem, addr, L_SET)  != addr)
    {
      syslog(LOG_ERR, "lu_pstat: can't lseek to swpmap");
      return(BUILD_ERR);
    }

  if(read(kmem, swapmap, nswapmap * sizeof(swapmap)) != 
     sizeof(swapmap) * nswapmap)
    {
      syslog(LOG_ERR, "lu_pstat: can't read swapmap");
      return(BUILD_ERR);
    }

  /*
   * calculations
   */

  nswap = 0;
  for(sw = swdevt;  sw < &swdevt[nswdev]; sw++)
    if(sw->sw_freed)
      nswap += sw->sw_nblks;

  fre = 0;
  for(me = (struct mapent *) (swapmap + 1); 
      me < (struct mapent *) &swapmap[nswapmap]; me++)
    fre += me->m_size;

  tused = 0;
  for(xp = xtext; xp < &xtext[ntext]; xp++)
    {
#ifdef VFS
      if(xp->x_vptr != NULL)
	{
	  tused += ctod(clrnd(xp->x_size));
	  if(xp->x_flag & XPAGV)
	    tused += ctod(clrnd(ctopt(xp->x_size)));
	}
#else  VFS
      if(xp->x_iptr != NULL)
	{
	  tused += ctod(clrnd(xp->x_size));
	  if(xp->x_flag & XPAGI)
	    tused += ctod(clrnd(ctopt(xp->x_size)));
	}
#endif VFS
    }

  used = tused;
  waste = 0;

  for(pp = xproc; pp < &xproc[nproc]; pp++)
    {
      if((pp->p_stat == 0) || (pp->p_stat == SZOMB))
	continue;
      if(pp->p_flag & SSYS)
	continue;
      db = ctod(pp->p_dsize);
      sb = up(db);
      used  += db;
      waste += sb - db;
      db = ctod(pp->p_ssize);
      sb = up(db);
      used  += sb;
      waste += sb - db;
      if((pp->p_flag & SLOAD) == 0)
	used += ctod(vusize(pp));
    }

  free(xproc);
  free(xtext);
  free(swapmap);
  free(swdevt);

  /*
   * for consistency with other variables, convert to bytes 
   */

  switch(offset)
    {
    case N_PSTOTAL: 
      *value = btok(nswap) * 1024;
      return(BUILD_SUCCESS);
    case N_PSUSED:
      *value = btok(used)  * 1024;
      return(BUILD_SUCCESS);
    case N_PSTUSED:
      *value = btok(tused) * 1024;
      return(BUILD_SUCCESS);
    case N_PSFREE:
      *value = btok(fre)   * 1024;
      return(BUILD_SUCCESS);
    case N_PSWASTED:
      *value = btok(waste) * 1024;
      return(BUILD_SUCCESS);
    case N_PSMISSING:
      *value = btok(nswap - dmmax/2 - (used + fre)) * 1024;
      return(BUILD_SUCCESS);
    default:
      syslog (LOG_ERR, "lu_pstat: bad offset: %d", offset);
      return(BUILD_ERR);
    }
}

	
static int
up(size)
     register int size;
{
  register int i, block;

  i = 0;
  block = dmmin;
  while (i < size) 
    {
      i += block;
      if (block < dmmax)
	block *= 2;
    }
  return (i);
}


/*
 * Function:    vusize()
 * Description: Compute number of pages to be allocated to the u. area
 *              and data and stack area page tables, which are stored on the
 *              disk immediately after the u. area.
 */

static int
vusize(p)
     register struct proc *p;
{
  register int tsz = p->p_tsize / NPTEPG;

  /*
   * We do not need page table space on the disk for page
   * table pages wholly containing text.
   */
  return (clrnd(UPAGES +
		clrnd(ctopt(p->p_tsize+p->p_dsize+p->p_ssize+UPAGES)) - tsz));
}


#endif /* RSPOS */
#endif /* SOLARIS */

#endif MIT
