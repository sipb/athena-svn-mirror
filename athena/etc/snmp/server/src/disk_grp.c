/*
 *
 */

#include "include.h"
#ifdef ATHENA

#ifdef BSD
#undef BSD
#endif BSD
#include <sys/param.h>
#include <errno.h>
#include <ufs/fs.h>
#include <sys/vfs.h>
#include <mntent.h>

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
  repl->name.ncmp++;                    /* include the "0" instance */

  repl->val.type = INT;  /* True of all the replies */
  
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





lu_disk(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  FILE *mtabp;
  struct mntent *mnt;
  int cnt = 0;
  int pnum = 0;
  struct thing df;
  struct thing di;
  struct stat statbuf;

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
  if(cnt != pnum)
    return(BUILD_ERR);

  
  /*
   * Build reply
   */

  bcopy ((char *)varnode->var_code, (char *) &repl->name, sizeof(repl->name));
  repl->name.cmp[repl->name.ncmp] = pnum;
  repl->name.ncmp++;                    /* include the "0" instance */
  repl->val.type = INT; 

  if((stat(mnt->mnt_fsname, &statbuf) >= 0) &&
     (((statbuf.st_mode & S_IFBLK) == S_IFBLK) ||
      ((statbuf.st_mode & S_IFCHR) == S_IFCHR)))
    dfreedev(mnt->mnt_fsname, &df, &di);
  else
    dfreemnt(mnt->mnt_dir, &df, &di);
	
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
  return(BUILD_SUCCESS);
}



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


#endif ATHENA
