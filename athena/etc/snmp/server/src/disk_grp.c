/*
 * This is the MIT supplement to the PSI/NYSERNet implementation of SNMP.
 * This file describes the disk usage portion of the mib.
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
 *    $Source: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/disk_grp.c,v $
 *    $Author: tom $
 *    $Locker:  $
 *    $Log: not supported by cvs2svn $
 *
 */

#include "include.h"
#include <mit-copyright.h>

#ifdef MIT

/* 
 * something else sets this as well... arggh
 */

#ifdef BSD
#undef BSD
#endif BSD

#include <sys/param.h>
#include <errno.h>
#include <ufs/fs.h>
#include <sys/vfs.h>
#include <mntent.h>

/*
 * local utilities
 */

static int dfreedev();
static int dfreemnt();
static int bread();

/*
 * plastic shopping bags are 5 cents
 */

struct thing 
{
  int total;
  int free;
  int avail;
  int used;
};

union 
{
  struct fs iu_fs;
  char dummy[SBSIZE];
} sb;

#define sbk sb.iu_fs



/*
 * Function:    lu_ndparts()
 * Description: Top level callback. Retrieves the number of partitions 
 *              currently mounted (INT).
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

int
lu_ndparts(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  FILE *mtabp;
  struct mntent *mnt;
  int cnt = 0;

  if (varnode->flags & NOT_AVAIL ||
      varnode->offset <= 0 ||     /* not expecting offset here */
      ((varnode->flags & NULL_OBJINST) && (reqflg == NXT)))
    return (BUILD_ERR);

  /*
   * Build reply
   */

  bcopy ((char *)varnode->var_code, (char *) &repl->name, sizeof(repl->name));
  repl->name.ncmp++;      
  repl->val.type = INT;   
  
  /*
   * Open mtab and count 'em up. We only care about MNTTYPE_42 types.
   */

  if((mtabp = setmntent(MOUNTED, "r")) == 0)
    {
      syslog(LOG_ERR, "lu_ndparts: could not open mtab");
      return(BUILD_ERR);
    }

  while(mnt = getmntent(mtabp))
    {
      if(strcmp(mnt->mnt_type, MNTTYPE_42) != 0)
	continue;
      cnt++;
    }

  endmntent(mtabp);
  repl->val.value.intgr = cnt;
  return(BUILD_SUCCESS);
}




/*
 * Function:    lu_disk()
 * Description: Top level callback. Suports the following:
 *                 PTUSED:  (INT) used disk in bytes
 *                 PTAVAIL: (INT) available disk in bytes
 *                 PTFREE:  (INT) free disk in bytes
 *                 PTTOTAL: (INT) total disk in bytes
 *                 PIUSED:  (INT) used inodes
 *                 PIFREE:  (INT) free inodes
 *                 PITOTAL: (INT) total inodes
 *              Each partition is an instance starting at 1, proceeding in the
 *              order in which they were mounted.
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 * Problems:    get-next feature doesn't work with the available instances
 */

int          
lu_disk(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  struct stat statbuf;
  struct mntent *mnt;
  FILE *mtabp;
  int pnum = 0; 
  int cnt = 0;
  struct thing df;     
  struct thing di;

  if (varnode->flags & NOT_AVAIL ||
      varnode->offset <= 0 ||     /* not expecting offset here */
      ((varnode->flags & NULL_OBJINST) && (reqflg == NXT)))
    return (BUILD_ERR);

  if((instptr == (objident *) NULL) || (instptr->ncmp == 0))
    pnum = 1;
  else
    pnum = instptr->cmp[0];

  if((reqflg == NXT) && (instptr != (objident *) NULL) && (instptr->ncmp !=0))
     pnum++;

  /*
   * open mtab and get the partition associated with given instance 
   */

  if((mtabp = setmntent(MOUNTED, "r")) == 0)
    {
      syslog(LOG_ERR, "lu_ndparts: could not open mtab");
      return(BUILD_ERR);
    }

  while(mnt = getmntent(mtabp))
    {
      if(strcmp(mnt->mnt_type, MNTTYPE_42) != 0)
	continue;
      cnt++;
      if(cnt == pnum)
	break;
    }

  endmntent(mtabp);

  /*
   * instance too high?
   */

  if(cnt != pnum)
    return(BUILD_ERR);

  
  /*
   * Build reply
   */

  bcopy ((char *)varnode->var_code, (char *) &repl->name, sizeof(repl->name));
  repl->name.cmp[repl->name.ncmp] = pnum;
  repl->name.ncmp++;                
  repl->val.type = INT; 


  /*
   * find out what device has been selected and call appropriate df function
   */

  if((stat(mnt->mnt_fsname, &statbuf) >= 0) &&
     (((statbuf.st_mode & S_IFBLK) == S_IFBLK) ||
      ((statbuf.st_mode & S_IFCHR) == S_IFCHR)))
    dfreedev(mnt->mnt_fsname, &df, &di);
  else
    dfreemnt(mnt->mnt_dir, &df, &di);
	
  /*
   * return values
   */

  switch(varnode->offset)
    {
    case N_PTUSED:
      repl->val.value.intgr = df.used;
      return(BUILD_SUCCESS);
      
    case N_PTAVAIL:
      repl->val.value.intgr = df.avail;
      return(BUILD_SUCCESS);
      
    case N_PTFREE:
      repl->val.value.intgr = df.free;
      return(BUILD_SUCCESS);
      
    case N_PTTOTAL:
      repl->val.value.intgr = df.total;
      return(BUILD_SUCCESS);

    case N_PIUSED:
      repl->val.value.intgr = di.used;
      return(BUILD_SUCCESS);
      
    case N_PIFREE:
      repl->val.value.intgr = di.free;
      return(BUILD_SUCCESS);

    case N_PITOTAL:
      repl->val.value.intgr = di.total;
      return(BUILD_SUCCESS);
    }
}



/*
 * Function:    dfreedev()
 * Description: read disk stats out of given file, return stuff in things.
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

static int
dfreedev(file, df, di)
     char *file;
     struct thing *df;
     struct thing *di;
{
  int fd;
  int tavail;

  fd = open(file, 0);
  if(fd < 0)
    return(BUILD_ERR);

  if(bread(fd, SBLOCK, (char *) &sbk, SBSIZE) == 0)
    {
      (void) close(fd);
      return(BUILD_ERR);
    }
  
  di->total  = sbk.fs_ncg * sbk.fs_ipg;
  di->used   = di->total - sbk.fs_cstotal.cs_nifree;
  di->free   = sbk.fs_cstotal.cs_nifree;
  df->total  = sbk.fs_dsize;
  df->free   = sbk.fs_cstotal.cs_nbfree * sbk.fs_frag + 
    sbk.fs_cstotal.cs_nffree;
  df->used   = df->total - df->free;
  tavail     = df->total * (100 - sbk.fs_minfree) / 100;
  df->avail  = (tavail > df->used ? tavail - df->used : 0);

  df->total  = df->total * sbk.fs_fsize;
  df->used   = df->used  * sbk.fs_fsize;
  df->avail  = df->avail * sbk.fs_fsize;
  df->free   = df->free  * sbk.fs_fsize;

  (void) close(fd);
  return(BUILD_SUCCESS);
}



/*
 * Function:    dfreemnt()
 * Description: read disk stats out of given file, return stuff in things.
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

static int      
dfreemnt(file, df, di)
     char *file;
     struct thing *df;
     struct thing *di;
{
  struct statfs fs;
  
  if(statfs(file, &fs) < 0)
    return(BUILD_ERR);
  
  di->total  = fs.f_files;
  di->used   = di->total - fs.f_ffree;
  di->free   = fs.f_ffree;
  df->total  = fs.f_blocks;
  df->free   = fs.f_bfree;
  df->used   = (df->total - df->free);
  df->avail  = fs.f_bavail;

  df->total  = df->total * fs.f_bsize;
  df->used   = df->used  * fs.f_bsize;
  df->avail  = df->avail * fs.f_bsize;
  df->free   = df->free  * fs.f_bsize;
  
  return(BUILD_SUCCESS);
}



/*
 * Function:    bread()
 * Description: seek & read
 * Returns:     0 if error
 *              1 if success
 */

static int
bread(fi, bno, buf, cnt)
     int fi;
     daddr_t bno;
     char *buf;
{
  int n;
  extern errno;
  
  (void) lseek(fi, (long)(bno * DEV_BSIZE), 0);
  if ((n=read(fi, buf, cnt)) != cnt) 
    {
      /* probably a dismounted disk if errno == EIO */
      if (errno != EIO) 
	{
	  /* syslog here */
	}
      return (0);
    }
  return (1);
}


#endif MIT
