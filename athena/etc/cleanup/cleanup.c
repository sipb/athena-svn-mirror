/* $Header: /afs/dev.mit.edu/source/repository/athena/etc/cleanup/cleanup.c,v 1.9 1991-03-07 17:40:14 epeisach Exp $
 *
 * Cleanup script for dialup.
 *
 * Author: John Carr <jfc@athena.mit.edu>
 *
 * 1. Create /etc/nologin.  If it exists or can't be created, fail.
 *
 * 2. Find logged in users.  This should be the union of those users in
 *     /usr/adm/utmp and a file to be named later containing the list of
 *     background process owners (latter feature not implemented *
 *
 * 3. Kill processes owned by users who are not logged in.
 *
 * 4. Re-create /etc/passwd from user list and hesiod (since $HOME is
 *     inherited, this should not break anything).
 *
 *  5. Remove /etc/nologin
 */

#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <signal.h>
#include <strings.h>
#include <utmp.h>
#include <pwd.h>
#ifdef _BSD
#undef _BSD
#endif
#include <nlist.h>
#include <hesiod.h>

#ifndef __STDC__
#define const
extern void make_passwd();
extern void make_group();
#else
extern void make_passwd(int,uid_t *, char (*)[16]);
extern void make_group(int, uid_t *);
#endif

#if defined(AIX) && (AIXV<30)
extern char     *sys_errlist[];
extern int      sys_nerr;
#endif

const char *version = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/cleanup/cleanup.c,v 1.9 1991-03-07 17:40:14 epeisach Exp $";

#ifdef ultrix
extern char *sys_errlist[];
#endif

struct nlist nl[] =
{
#ifdef _AIX
#define PROC 0
  { "proc" },
#define NPROC 1
  { "nproc" },
  { ""}
#else
#define PROC 0
  { "_proc" },
#define NPROC 1
  { "_nproc" },
  { ""}
#endif
};

#define MAXUSERS 1024
#define ROOTUID 0
#define DAEMONUID 1

#define LOGGED_IN 1
#define PASSWD 2

/* static const char *nologin_msg = "Try again in a few seconds...\n"; */
static const char *nologin_msg = "This machine is down for cleanup; try again in a few seconds.\n";
static const char *nologin_fn  = "/etc/nologin";

#ifdef __STDC__
int main(int argc,char *argv[])
#else
int main(argc,argv)
int argc;
char *argv[];
#endif
{
  int fd, r, i, nuid;
  int status = 0;
  int mode = LOGGED_IN;
  struct utmp u;
  struct passwd *pwd;
  uid_t uids[MAXUSERS];
  char plist[MAXUSERS][16];
  FILE *lockf;

  if (argc == 1)
    mode = LOGGED_IN;
  else if (argc == 2 && !strcmp(argv[1], "-loggedin"))
    mode = LOGGED_IN;
  else if (argc == 2 && !strcmp(argv[1], "-passwd"))
    mode = PASSWD;
  else {
      fprintf(stderr, "usage: %s [-loggedin | -passwd]\n", argv[0]);
      exit(1);
  }

  /* First create /etc/nologin */
  fd = open(nologin_fn ,O_RDWR | O_EXCL | O_CREAT, 0664);
  if(fd < 0) {
    if (errno == EEXIST) {
	if (getuid())
	  fprintf(stderr, "cleanup: %s exists.  Failing.\n", nologin_fn);
	else
	  fprintf(stderr, "cleanup: %s already exists, not performing cleanup.\n",
		  nologin_fn);
	exit(2);
    } else {
	fprintf(stderr, "cleanup: Can't create %s, %s.\n", nologin_fn, sys_errlist[errno]);
	exit(3);
    }
  }
  (void)write(fd, nologin_msg, strlen(nologin_msg));
  (void)close(fd);

  /* Lock /etc/passwd */
  i = 10;
  while (!access("/etc/ptmp", 0) && --i)
    sleep(1);
  if ((lockf = fopen("/etc/ptmp", "w")) != NULL)
    fprintf(lockf, "%d\n", getpid());
  fclose(lockf);
  /* Also lock /etc/group */
  i = 10;
  while (!access("/etc/gtmp", 0) && --i)
    sleep(1);
  if ((lockf = fopen("/etc/gtmp", "w")) != NULL)
    fprintf(lockf, "%d\n", getpid());
  fclose(lockf);

  if (mode == LOGGED_IN) {
      /* Get the list of current users */
      fd = open("/etc/utmp", O_RDONLY);
      if(fd < 0)
	{
	    fprintf(stderr, "cleanup: Couldn't open /etc/utmp, %s.\n",
		    sys_errlist[errno]);
	    status = 4;
	    goto done;
	}
      
      uids[0] = ROOTUID;	/* Always count root... */
      uids[1] = DAEMONUID;  /* ...and daemon. */
      for(i=2;(r = read(fd, &u, sizeof (u))) > 0 && i < MAXUSERS;i++)
	{
	    register struct passwd *p;
	    char buf[9];
	    if(u.ut_name[0] == '\0')
	      {
		  i--;
		  continue;
	      }
	    bcopy(u.ut_name, buf, 8);
	    buf[8] = '\0';
	    p = getpwnam(buf);
	    if(p == 0)
	      {
		  fprintf(stderr, "cleanup: Warning, could not get uid for user \"%s\".\n", buf);
		  i--;
	      } else {
		  int j;
		  for(j = 0;j < i;j++)
		    if(p->pw_uid == uids[j])
		      break;
		  if(i != j) {
		      i--;
		      continue;	/* duplicate */
		  }
		  (void) strncpy(plist[i], p->pw_passwd, sizeof(plist[i]));
		  uids[i] = p->pw_uid;
	      }
	}
      nuid = i;
  } else /* mode == PASSWD */ {
      /* Get the list of users in /etc/passwd */
      setpwent();
      nuid = 0;
      while ((pwd = getpwent()) != NULL)
	uids[nuid++] = pwd->pw_uid;
      if (nuid > MAXUSERS) {
	  fprintf(stderr, "cleanup: Too many users in /etc/passwd for cleanup\n");
	  status = 2;
	  goto done;
      }
  }

#ifdef DEBUG
  i = nuid;
  while(--i>=0)
      printf("uids[%d] = %d\n", i, uids[i]);
#endif

  /* Now: kill any processes owned by a user not in uids[].
   * First pass: send HUP
   * (sleep ...)
   * Second pass: send KILL
   */
  if (status = kill_uids(nuid, uids, 1, SIGHUP))
    goto done;
  sleep(5);
  if (status = kill_uids(nuid, uids, 0, SIGKILL))
    goto done;

  if (mode == LOGGED_IN) {
      make_passwd(nuid, uids, plist);
      make_group(nuid, uids);
  }
  
 done:
  if(unlink(nologin_fn) < 0)
    fprintf(stderr, "cleanup: Warning: unable to unlink /etc/nologin.\n");
  if(unlink("/etc/ptmp") < 0)
    fprintf(stderr, "cleanup: Warning: unable to unlink /etc/ptmp.\n");
  if(unlink("/etc/gtmp") < 0)
    fprintf(stderr, "cleanup: Warning: unable to unlink /etc/gtmp.\n");

  return (status);
}


#define NPROCS 16

#ifdef __STDC__
int kill_uids(int nuid,uid_t *uids,int debug,int sig)
#else
int kill_uids(nuid, uids, debug, sig)
int nuid;
uid_t *uids;
int debug;
int sig;
#endif
{
#ifdef _AIX
  char *kernel = "/unix";
#else
  char *kernel = "/vmunix";
#endif
  struct proc p[NPROCS];
  int i,j,k,nproc,procp;
  int kmem;
  int self = getpid();
  static int n_done = 0;

#ifdef DEBUG
  printf("nuid = %d\n", nuid);
#endif
  if(n_done == 0 && nlist(kernel, nl) != 0)
    {
      fprintf(stderr, "Cleanup: can't get kernel name list.\n");
      return(5);
    }
  n_done = 1;
  if((kmem = open("/dev/kmem", O_RDONLY)) < 0)
    {
      fprintf(stderr, "Cleanup: can't open kmem (%s).\n", sys_errlist[errno]);
      return(6);
    }
  lseek(kmem, nl[NPROC].n_value, L_SET);
  read(kmem, &nproc, sizeof(nproc));

  lseek(kmem, nl[PROC].n_value, L_SET);
  read(kmem, &procp, sizeof(procp));
  lseek(kmem, procp, L_SET);
  for(i=0; i<nproc; i+=NPROCS)
    {
      read(kmem, p, sizeof(p));
      for(j=i; j < (i+NPROCS); j++)
	{
	  if(j >= nproc)
	    goto done;
	  for(k=0;k<nuid;k++)
	    if(p[j-i].p_uid == uids[k])
	      goto out;
	  if(p[j-i].p_pid == ROOTUID || p[j-i].p_pid == DAEMONUID ||
	     p[j-i].p_pid == self)
	    goto out;
#ifdef DEBUG
	  if(debug)
	    printf("killing proc %d, uid %d\n", p[j-i].p_pid, p[j-i].p_uid);
#endif
	  kill(p[j-i].p_pid, sig);
	out:
	  continue;
	}
#ifdef DEBUG
      putchar('.');
#endif
    }
 done:
#ifdef DEBUG
  putchar('\n');
#endif
  (void) close(kmem);
  return 0;
}


#ifdef __STDC__
void make_passwd(int nuid, uid_t *uids, char (*plist)[16])
#else
void make_passwd(nuid, uids, plist)
int nuid;
uid_t *uids;
char (*plist)[16];
#endif
{
  int fd;
  int proto,r,i;
  char buf[BUFSIZ],**ret;

  if(access("/etc/passwd.local", R_OK) < 0) return;
  fd = open("/etc/passwd.new",O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(fd == -1)
    {
      fprintf(stderr,"cleanup: Couldn't open \"/etc/passwd.new\", %s.\n",
	      sys_errlist[errno]);
      return;
    }
  if((proto = open("/etc/passwd.local", O_RDONLY)) == -1)
    {
      fprintf(stderr,"cleanup: Couldn't open \"/etc/passwd.local\", %s.\n",
	      sys_errlist[errno]);
      close(fd);
      return;
    }
  while((r = read(proto,buf,BUFSIZ)) > 0)
    if(write(fd,buf,r) != r)
      {
	fprintf(stderr, "cleanup: Error copying /etc/passwd.local...aborting.\n");
	(void) close(fd);
	(void) close(proto);
	(void) unlink("/etc/passwd.new");
	return;
      }
  if(r < 0)
    {
      fprintf(stderr,"cleanup: Couldn't open \"/etc/passwd.local\", %s.\n",
	      sys_errlist[errno]);
      close(fd);
      close(proto);
      return;
    }
  for(i=0;i<nuid;i++)
    {
      char uname[8];
      char *c1ptr, *c2ptr;
      if(uids[i] == 0 || uids[i] == 1)
	continue;
      sprintf(uname,"%d",uids[i]);
      ret = hes_resolve(uname,"uid");
      if(ret == NULL || *ret == NULL)
	{
	  fprintf(stderr, "cleanup: Couldn't get hesinfo for uid %d, error %d.\n",
		  uids[i],hes_error());
	} else if((c1ptr = index(*ret, ':')) == 0 || 
		  (c2ptr = index(++c1ptr, ':')) == 0) {
	  fprintf(stderr, "cleanup: Corrupt password entry for uid %d: \"%s\".\n",
		  uids[i], *ret);
	} else {
	  *c1ptr = '\0';
	  if(write(fd, *ret,strlen(*ret)) < 0 ||
	     write(fd, plist[i], strlen(plist[i])) < 0 ||
	     write(fd, c2ptr, strlen(c2ptr)) < 0)
	    fprintf(stderr,"cleanup: Error writing \"/etc/passwd.new\": %s.\n",
		    sys_errlist[errno]);
#ifdef DEBUG
	  else
	    printf(">>>%s %s %s\n", *ret, plist[i], c2ptr);
#endif
	  write(fd,"\n",1);
	}
    }
  if(unlink("/etc/passwd") == -1)
    perror("unlink");
  if(rename("/etc/passwd.new", "/etc/passwd") == -1)
    perror("rename");
}


/* Rebuild the group file, keeping any group which has a member who is 
 * in the passwd file.
 */

#ifdef __STDC__
void make_group(int nuid, uid_t *uids)
#else
void make_group(nuid, uids)
int nuid;
uid_t *uids;
#endif
{
    int r, i, n, match;
    char buf[10240], *p, *p1, **ret;
    char llist[MAXUSERS][9];
    struct passwd *pw;
    FILE *new, *old;

    /* build list of users in the passwd file */
    for (i = 0; i < nuid; i++) {
	sprintf(buf, "%d", uids[i]);
	ret = hes_resolve(buf, "uid");
	if (ret == NULL || *ret == NULL) {
	    pw = getpwuid(uids[i]);
	    if (pw != NULL)
	      strcpy(llist[i], pw->pw_name);
	} else if ((p = index(*ret, ':')) == NULL) {
	    fprintf(stderr, "cleanup: Corrupt password entry for uid %d: \"%s\".\n",
		    uids[i], *ret);
	} else {
	    *p = 0;
	    strcpy(llist[i], *ret);
	}
    }

    if ((new = fopen("/etc/group.new", "w")) == NULL) {
	fprintf(stderr,"cleanup: Couldn't open \"/etc/group.new\", %s.\n",
		sys_errlist[errno]);
	return;
    }
    if (chmod("/etc/group.new", 0644)) {
	fprintf(stderr, "cleanup: Couldn't change mode of \"/etc/group.new\", %s\n",
		sys_errlist[errno]);
	return;
    }
    if ((old = fopen("/etc/group", "r")) == NULL) {
	fprintf(stderr,"cleanup: Couldn't open \"/etc/group\", %s.\n",
		sys_errlist[errno]);
	fclose(new);
	return;
    }

    /* loop over each line in the group file */
    while (fgets(buf, sizeof(buf), old)) {
	/* take out tailing \n */
	n = strlen(buf);
	if (n) {
	    if (buf[n - 1] == '\n')
	      buf[n - 1] = 0;
	    else
	      fprintf(stderr, "cleanup: warning, too long group entry truncated\n");
	}
	if ((p = index(buf, ':')) == 0 ||
	    (p = index(p+1, ':')) == 0 ||
	    (p = index(p+1, ':')) == 0) {
	    fprintf(stderr, "cleanup: Corrupt group entry \"%s\".\n", buf);
	    continue;
	}
	match = 0;
	if (p) p++;

	/* loop over each member of the group */
	while (p != NULL) {
	    p1 = index(p, ',');
	    if (p1)
	      n = p1 - p;
	    else
	      n = strlen(p);
	    /* compare against each user in the passwd file */
	    for (i = 0; i < nuid; i++) {
		if (!strncmp(p, llist[i], n)) {
		    match = 1;
		    break;
		}
	    }
	    if (match)
	      break;
	    if (p1)
	      p = p1 + 1;
	    else
	      p = NULL;
	}
	if (match)
	  fprintf(new, "%s\n", buf);
#ifdef DEBUG
	else
	  printf("<<<%s\n", buf);
#endif
    }
    if (fclose(new) || fclose(old)) {
	fprintf(stderr, "cleannup: Error closing group file: sys_errlist[errno]\n");
	return;
    }
    if (unlink("/etc/group") == -1)
      perror("unlink");
    if (rename("/etc/group.new", "/etc/group") == -1)
      perror("rename");
}
