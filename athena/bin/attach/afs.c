/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/attach/afs.c,v $
 *	Copyright (c) 1990 by the Massachusetts Institute of Technology.
 */

static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/afs.c,v 1.1 1990-07-16 07:26:48 jfc Exp $";

#include "config.h"

#ifdef AFS
#include "attach.h"

#include <sys/stat.h>
#include <sys/param.h>	/* MAXPATHLEN */
#include <sys/errno.h>

#include <afs/auth.h>	/* token structures */
#include <afs/venus.h>	/* pioctl codes */
#include <afs/param.h>	/* AFS_PIOCTL */

#include <krb.h>
extern char *krb_realmofhost();	/* <krb.h> doesn't declare this */

/* Configuration limits */
#define	CELLNAMELEN	64
#define	VOLNAMELEN	64
#define	MAXCELLS	4	/* max cells in path */

/* private version of pioctl, until the AFS lib version is fixed not
   to include rmtsys code */


int pioctl(a1,a2,a3,a4)
char *a1;
struct ViceIoctl *a3;
int a2,a4;
{
  int ret = syscall(AFS_PIOCTL, a1, a2, a3, a4);
  if(debug_flag)
    printf("pioctl(\"%s\", %x, %x, %d) = %d (%d)\n", 
	   a1 ? a1 : "(nil)", a2, a3, a4, ret, ret == -1 ? errno : 0);
  return ret;
}


static int afs_add_cell_to_list(), afs_auth_cell_list();

/*
 * The current implementation of AFS attaches is to make a symbolic
 * link.  This is NOT GUARANTEED to always be the case. 
 */

int afs_attach(at, opt, errorout)
	struct _attachtab *at;
	struct mntopts *opt;
	int errorout;
{
  struct stat	statbuf;
  int		err;
  struct ViceIoctl vio;

  /* Most of this code should be run as the user */
  setreuid(effective_uid, real_uid);

  /* getting tokens might take time...start a prefetch */
  if(at->mode != 'n')
    {
      vio.in = ".";
      vio.in_size = 2;
      vio.out = 0;
      vio.out_size = 0;

      if(pioctl(at->hostdir, VIOCPREFETCH, &vio, 1) == -1 && debug_flag)
	fprintf(stderr,
		"%s: pioctl(VIOCPREFETCH) failed for path \"%s/.\"\n\terror is %s\n",
		progname, at->hostdir, errstr(errno));
      
      if(afs_auth(at->hesiodname, at->hostdir) != SUCCESS)
	return FAILURE;	/* afs_auth sets error_status */
    }

  /* Verify that directory exists and is in AFS */
  vio.in = ".";
  vio.in_size = 2;
  vio.out = 0;
  vio.out_size = 0;
  if(pioctl(at->hostdir, VIOCGETFID, &vio, 0) == -1)
    if(errno == EINVAL)
      {
	fprintf(stderr,
		"%s: %s (mountpoint of %s) does not appear to be in AFS\n",
		progname, at->hostdir, at->hesiodname);
	error_status = ERR_ATTACHBADFILSYS;
	setreuid(real_uid, effective_uid);
	return FAILURE;
      } else if(errno == ENOENT) {
	fprintf(stderr, "%s: filesystem %s (directory %s) does not exist\n",
		progname, at->hesiodname, at->hostdir);
	error_status = ERR_ATTACHBADFILSYS;
	setreuid(real_uid, effective_uid);
	return FAILURE;
      }
  if(stat(at->hostdir, &statbuf) == -1)
    {
      if(errno == ENOENT)
	fprintf(stderr, "%s: filesystem %s (directory %s) does not exist\n",
		progname, at->hesiodname, at->hostdir);
      else
	fprintf(stderr, "%s: filesystem %s (directory %s): %s\n",
		progname, at->hesiodname, at->hostdir, errstr(errno));
      error_status = ERR_ATTACHBADFILSYS;
      setreuid(real_uid, effective_uid);
      return FAILURE;
    }
  if((statbuf.st_mode & S_IFMT) != S_IFDIR)
    {
      fprintf(stderr, "%s: %s (root of filesystem %s) is not a directory\n",
	      progname, at->hostdir, at->hesiodname);
      error_status = ERR_ATTACHBADFILSYS;
      setreuid(real_uid, effective_uid);
      return FAILURE;
    }

  /* Now create the mountpoint ... need to return to root for this. */
  setreuid(real_uid, effective_uid);

  err = 0;
  if(lstat(at->mntpt, &statbuf) == -1 && (err = errno) != ENOENT)
    {
      /* ??? */
      fprintf(stderr, "%s: error checking mountpoint %s of filesystem %s\n\terror is %s",
	      progname, at->mntpt, at->hesiodname, errstr(errno));
      error_status = ERR_ATTACHBADMNTPT;
      return FAILURE;
    }
  if(err == 0)
    {
      /* file/link exists */
      if((statbuf.st_mode & S_IFMT) == S_IFDIR)
	{
	  if(rmdir(at->mntpt) == -1 && errno != ENOENT)
	    {
	      if(errno == ENOTEMPTY)
		fprintf(stderr, "%s: mountpoint %s of filesystem %s is a non-empty directory\n",
			progname, at->mntpt, at->hesiodname);
	      else
		fprintf(stderr, "%s: error removing mountpoint %s of filesystem %s\n\terror is %s\n",
			progname, at->mntpt, at->hesiodname, errstr(errno));
	      error_status = ERR_ATTACHBADMNTPT;
	      return FAILURE;
	    }
	  /* reaching here means the mountpoint has been removed */
	} else if((statbuf.st_mode & S_IFMT) == S_IFLNK) {
	  char buf[MAXPATHLEN];

	  /* setting errno isn't guaranteed to work, but it does on
	     all systems I've seen */
	  errno = 0;
	  /* check to see if is already correct */
	  if(readlink(at->mntpt, &buf, sizeof(buf)) >= 0 &&
	     !strcmp(buf, at->hostdir))
	    {
	      if(verbose && !print_path)
		printf("%s: symlink from %s to %s for filesystem %s exists\n",
		       progname, at->mntpt, at->hostdir, at->hesiodname);
	      if(keep_mount)
		at->flags |= FLAG_PERMANENT;
	      return SUCCESS;
	    }
	  /* couldn't read the file, or pointed to the wrong place */
	  /* NOTE: add a check somewhere for duplicate mountpoints */
	  err = errno;	/* return of readlink */
	  if(keep_mount || unlink(at->mntpt) == -1)
	    {
	      if(err)
		fprintf(stderr, "%s: unable to attach %s: mountpoint exists\n",
			progname, at->hesiodname);
	      else
		fprintf(stderr, "%s: unable to attach %s: symlink from %s to %s exists\n",
			progname, at->hesiodname, at->mntpt, buf);
	      error_status = ERR_ATTACHINUSE;
	      return FAILURE;
	    }
	  /* reaching here means the mountpoint has been removed */
	} else {
	  fprintf(stderr, "%s: can't attach %s: mountpoint %s exists\n",
		  progname, at->hesiodname, at->mntpt);
	  error_status = ERR_ATTACHINUSE;
	  return FAILURE;
	}
    }
  if(symlink(at->hostdir, at->mntpt) == -1)
    {
      fprintf(stderr, "%s: can't attach %s: symlink from %s to %s failed\n\terror is %s\n",
	      progname, at->hesiodname, at->mntpt, at->hostdir, errstr(errno));
      error_status = ERR_ATTACHNOFILSYS;
      return FAILURE;
    }
  return SUCCESS;
}

/*
 * Strategy: STAT_MT_PT first; if not a mount point then FILE_CELL_NAME (the
 * mount point must be checked, else a mount point leading to a
 * non-world-readable directory in another cell can not be checked).
 *
 * Return value: cell name, 0 length string = EACCES, NULL string = error
 */

char *afs_cell_of_path(path, len)
const char *path;
int len;
{
  static char cell_name[CELLNAMELEN];	/* return */
  struct ViceIoctl vio;
  char *cp;

  if(debug_flag)
    printf("afs_cell_of_path(\"%s\", %d)\n", path, len);

  vio.in = (caddr_t)path;
  vio.in_size = len;
  vio.out = cell_name;
  vio.out_size = sizeof(cell_name);
  if(pioctl(path, VIOC_AFS_STAT_MT_PT, &vio, 1) >= 0)
    {
      if(cp = index(cell_name, ':'))
	{
	  *cp = 0;
	  return cell_name + 1;
	}
    }
  if(pioctl(path, VIOC_FILE_CELL_NAME, &vio, 1) >= 0)
    return cell_name;
  return (errno == EACCES) ? "" : 0;
}

/*
 * Generate a list of cells to authenticate to, given an AFS path.  Strategy:
 * get cell of dir.  If failed due to access check, auth to parent dir.  */

static int afs_auth_to_dir(dir)
const char *dir;
{
  static char path_buf[MAXPATHLEN];
  static int path_in_use = 0;
  char *cell_name;
  int path_len;
  
  if(path_in_use++ == 0)	/* top level */
    {
      (void) strcpy(path_buf, dir);
    }
 
  path_len = strlen(dir);
  cell_name = afs_cell_of_path(path_buf, path_len);
  if(cell_name == 0)	/* fatal error */
    {
      --path_in_use;
      return FAILURE;
    }
  if(*cell_name == '\0')	/* permission denied */
    {
      char *cp = rindex(path_buf, '/');
      if(cp == 0 || cp == path_buf)
	{
	  --path_in_use;
	  return FAILURE;
	}
      *cp = '\0';
      if(afs_auth_to_dir(path_buf) == SUCCESS)
	{
	  *cp = '/';
	  if((cell_name = afs_cell_of_path(path_buf, path_len)) &&
	     *cell_name && afs_add_cell_to_list(cell_name) == SUCCESS)
	    {
	      --path_in_use;
	      return SUCCESS;
	    }
	}
      --path_in_use;
      return FAILURE;
    }
  --path_in_use;
  return afs_add_cell_to_list(cell_name);
}

/*
 * As is typical of AFS code, authenticating to a cell is designed to be
 * complicated.  This version of the following function calls ktc_SetToken
 * to install the tokens.  ktc_SetToken is a compilcated function full of
 * magic constants and undocumented structures; otherwise this function
 * would do all the work of constructing a token.
 */

int afs_auth_to_cell(cell)
const char *cell;
{
  static char cell_list[MAXCELLS][CELLNAMELEN];
  static int ncells;

  char *realm;
  int i, err;
  CREDENTIALS cred;

  if(debug_flag)
    printf("Authenticate to cell \"%s\".\n", cell);

  for(i = 0;i < ncells;i++)
    if(!strcmp(cell, cell_list[i]))
      return SUCCESS;

  if(ncells >= MAXCELLS)
    return FAILURE;

  (void)strcpy(cell_list[ncells++], cell);

  /* Get realm of host.  Assume cell name is formatted like a host name. */

  realm = krb_realmofhost(cell);

  err = krb_get_cred("afs", cell, realm, &cred);
  if(err != KSUCCESS)
    {
      /* There's got to be a better way than this to get the ticket
	 into KRBTKFILE, but I can't find one documented. */
      KTEXT_ST authent;
      if(krb_mk_req(&authent, "afs", cell, realm, 0) == KSUCCESS)
	err = krb_get_cred("afs", cell, realm, &cred);
    }
  if(err)
    {
      fprintf(stderr, "%s: Unable to authenticate to cell %s\n\tKerberos error is %s\n",
	      progname, cell, krb_err_txt[err]);
      --ncells;
      return FAILURE;
    } else {
      struct ktc_principal as, ac;
      struct ktc_token at;
      int ret;

      at.kvno = cred.kvno;

      (void) strcpy(as.name, "afs");
      as.instance[0] = '\0';
      (void) strcpy(as.cell, cell);
      (void) strcpy(ac.name, cred.pname);
      (void) strcpy(ac.instance, cred.pinst);
      (void) strcpy(ac.cell, cred.realm);
      at.startTime = cred.issue_date;
      at.endTime = cred.issue_date + cred.lifetime * 5;
      at.ticketLen = cred.ticket_st.length;
      bcopy(cred.session, &at.sessionKey, 8);
      bcopy(cred.ticket_st.dat, at.ticket, at.ticketLen);
      if(ret = ktc_SetToken(&as, &at, &ac))
	{
	  fprintf(stderr, "%s: Unable to authenticate to cell %s (ktc error %x)\n", 
		  progname, cell, ret);
	  --ncells;
	  return FAILURE;
	}
    }
  return SUCCESS;
}

static char clist[MAXCELLS][CELLNAMELEN];
static int  cindex;

static int afs_auth_cell_list()
{
  int i;

  if(debug_flag)
    for(i = 0;clist[i][0] && i < MAXCELLS;i++)
      printf("cell %s\n", clist[i]);
  for(i = 0;clist[i][0] && i < MAXCELLS;i++)
    if(afs_auth_to_cell(clist[i]) == FAILURE)
      return FAILURE;
  return SUCCESS;
}

static int afs_add_cell_to_list(cell)
const char *cell;
{
  if(cindex >= MAXCELLS)
    return FAILURE;

  if(debug_flag)
    printf("afs_add_cell_to_list(%s)\n", cell);

  (void) strcpy(clist[cindex++], cell);

  return SUCCESS;
}

int afs_auth(hesname, afsdir)
{
  if(debug_flag)
    printf("afs_auth(%s, %s)\n", hesname, afsdir);
  if(afs_auth_to_dir(afsdir) == FAILURE || afs_auth_cell_list() == FAILURE)
    return FAILURE;
  return SUCCESS;
}

/*
 * Parsing of explicit AFS file types
 */
char **afs_explicit(name)
	char *name;
{
  char *dir;
  char newmntpt[BUFSIZ];
  extern char *exp_hesptr[2];

  if (*name != '/')
    {
      fprintf(stderr, "%s: Illegal explicit definition \"%s\" for type %s\n",
	      progname, name, filsys_type);
      return (0);
    }
	
  dir = rindex(name, '/');
  (void) strcpy(newmntpt, afs_mount_dir);
  (void) strcat(newmntpt, dir);
  
  sprintf(exp_hesline, "AFS %s %c %s", name, override_mode ?
	  override_mode : 'w', mntpt ? mntpt : newmntpt);
  exp_hesptr[0] = exp_hesline;
  exp_hesptr[1] = 0;
  return exp_hesptr;
}

int afs_detach(at)
	struct _attachtab *at;
{
  if(at->flags & FLAG_PERMANENT)
    return SUCCESS;
     
  if(unlink(at->mntpt) == -1)
    {
      if(errno == ENOENT)
	{
	  fprintf(stderr, "%s: filesystem %s already detached\n",
		  progname, at->hesiodname);
	  return SUCCESS;
	}
      fprintf(stderr, "%s: detach of filesystem %s failed, unable to remove mountpoint\n\terror is %s\n",
	      progname, at->hesiodname, errstr(errno));
      /* Set error_status? */
      return FAILURE;
    }
  return SUCCESS;
}

/* 
 * Current strategy: subscribe to afs:CELL:VOLUME and afs:CELL
 * unconditionally and hostname of sole replication site.
 */

int afs_zinit(hesname, afsdir)
const char *hesname, *afsdir;
{
  struct ViceIoctl vio;
  int len;
  /* need to fit "afs:" + cell name + '#' + cell + volume */
  char cell[4+CELLNAMELEN+CELLNAMELEN+VOLNAMELEN+1];

  /* 0: clear out the return value, not trusting the AFS
     kernel code to do it */
  bzero(cell, sizeof(cell));

  /* 1: afs:cell */
  (void) strcpy(cell, "afs:");
  vio.in = ".";
  vio.in_size = 2;
  vio.out = cell + 4;;
  vio.out_size = sizeof(cell) - 4;
  if(pioctl(afsdir, VIOC_FILE_CELL_NAME, &vio, 1) == -1)
    {
      fprintf(stderr,
	      "%s: unable to get cell name for AFS filesystem %s\n",
	      progname, hesname);
      return FAILURE;
    }
  zephyr_addsub(cell);

  /* 2: afs:cell:volume */
  {
    char tmpp[MAXPATHLEN], *cp;
    (void)strcpy(tmpp, afsdir);
    cp = rindex(tmpp, '/');
    if(cp)
      {
	*cp = '\0';
	vio.in = cp + 1;
	vio.in_size = strlen(cp + 1) + 1;
      }
    len = strlen(cell);
    vio.out = cell + len;
    vio.out_size = CELLNAMELEN + VOLNAMELEN + 1;
    if(pioctl(tmpp, VIOC_AFS_STAT_MT_PT, &vio, 1) >= 0)
      {
	cp = index(cell + len + 1, ':');
	if(cp)	/* cell:volume */
	  {
	    (void) strncpy(cell + len - 3, "afs:", 4);
	    zephyr_addsub(cell + len - 3);
	  } else {
	    cell[len] = ':';
	    zephyr_addsub(cell);
	  }
      }
  }

  /* 3: check replication sites */
  /* assume that cell is sufficiently aligned to cast to (long *) */
  vio.in = ".";
  vio.in_size = 2;
  vio.out = cell;
  vio.out_size = sizeof(cell);
  bzero(cell, sizeof(cell));
  if(pioctl(afsdir, VIOCWHEREIS, &vio, 1) >= 0)
    {
      long *addrl = (long *)cell;
      if(addrl[0] && !addrl[1])	/* exactly one site? */
	{
	  char *tmp = inaddr_to_name(addrl[0]);
	  if(tmp)
	    zephyr_addsub(tmp);
	}
    }
}
#endif	/* AFS */
