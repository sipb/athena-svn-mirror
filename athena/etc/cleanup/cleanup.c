/* $Header: /afs/dev.mit.edu/source/repository/athena/etc/cleanup/cleanup.c,v 1.1 1990-05-01 18:47:02 mar Exp $
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
#include <sys/errno.h>
#include <utmp.h>
#include <pwd.h>
#include <nlist.h>
#include <hesiod.h>

extern void make_passwd(int,uid_t *);
struct nlist nl[] =
{
#define PROC 0
  { "_proc" },
#define NPROC 1
  { "_nproc" },
  { ""}
};

static char *nologin_msg = "Try again in a few seconds...\n";
static char *nologin_fn  = "/etc/nologin";

int main(int argc,char *argv[])

{
  int fd, r, i, nuid;
  struct utmp u;
  uid_t uids[64];

  fd = open(nologin_fn ,O_RDWR | O_EXCL | O_CREAT, 0664);
  if(fd < 0)
    if(errno == EEXIST)
      {
	if(getuid())
	  fprintf(stderr, "%s exists.  Failing.\n", nologin_fn);
	exit(2);
      } else {
	fprintf(stderr, "Can't create %s, %s.\n", nologin_fn, 
		sys_errlist[errno]);
	exit(3);
      }
  (void)write(fd, nologin_msg, strlen(nologin_msg));
  (void)close(fd);

  fd = open("/etc/utmp", O_RDONLY);

  if(fd < 0)
    {
      fprintf(stderr, "Couldn't open /etc/utmp, %s.\n", sys_errlist[errno]);
      exit(4);
    }
  uids[0] = 0;	/* Don't kill off root... */
  uids[1] = 1;  /* ...or daemon. */
  for(i=2;(r = read(fd, &u, sizeof (u))) > 0 && i < 64;i++)
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
	  fprintf(stderr, "Warning...could not get uid for user \"%s\".\n", buf);
	  i--;
	} else {
	  uids[i] = p->pw_uid;
	}
    }

  nuid = i;
  while(--i>=0)
      printf("uids[%d] = %d\n", i, uids[i]);

  /* Now: loop through all processes, getting the uid.
   * First pass: send HUP
   * (sleep ...)
   * Second pass: send KILL
   */
  kill_uids(nuid,uids, 0, 1, 1);
  sleep(2);
  kill_uids(nuid,uids, 0, 0, 9);

  make_passwd(nuid,uids);
  
  if(unlink(nologin_fn) < 0)
    fprintf(stderr, "Warning: unable to unlink /etc/nologin.\n");

  return 0;
}

#include <sys/time.h>
#include <sys/proc.h>

int kill_uids(int nuid,uid_t *uids,int except,int debug,int sig)

{
  char *kernel = "/vmunix";
  struct proc p[16];
  int i,j,k,nproc,procp;
  int kmem;
  int self = getpid();
  static int n_done = 0;

  printf("nuid = %d\n", nuid);
  if(n_done == 0 && nlist(kernel, nl) != 0)
    {
      fprintf(stderr, "Error: can't get kernel name list.\n");
      exit(5);
    }
  n_done = 1;
  if((kmem = open("/dev/kmem", O_RDONLY)) < 0)
    {
      fprintf(stderr, "Error: can't open kmem (%s).\n", sys_errlist[errno]);
      exit(6);
    }
  lseek(kmem, nl[NPROC].n_value, L_SET);
  read(kmem, &nproc, sizeof(nproc));

  lseek(kmem, nl[PROC].n_value, L_SET);
  read(kmem, &procp, sizeof(procp));
  lseek(kmem, procp, L_SET);
  for(i=0;i<nproc; i+=16)
    {
      read(kmem, &p, sizeof(p));
      for(j=i;j<(i+16);j++)
	{
	  if(j >= nproc)
	    goto done;
	  for(k=0;k<nuid;k++)
	    if(p[j-i].p_uid == uids[k])
	      goto out;
	  if(p[j-i].p_pid == except || p[j-i].p_pid == self)
	    goto out;
	  if(debug)
	    printf("killing proc %d, uid %d\n", p[j-i].p_pid, p[j-i].p_uid);
	  kill(p[j-i].p_pid,sig);
	out:
	  continue;
	}
      putchar('.');
    }
 done:
  (void) close(kmem);
  return 0;
}

void make_passwd(int nuid,uid_t *uids)

{
  int fd = open("/etc/passwd.new",O_RDWR | O_CREAT | O_TRUNC, 0664);
  int proto,r,i;
  char buf[1024],**ret;
  if(fd == -1)
    {
      fprintf(stderr,"Couldn't open \"/etc/passwd.new\", %s.\n",
	      sys_errlist[errno]);
      return;
    }
  if((proto = open("/etc/passwd.local", O_RDONLY)) == -1)
    {
      fprintf(stderr,"Couldn't open \"/etc/passwd.local\", %s.\n",
	      sys_errlist[errno]);
      close(fd);
      return;
    }
  while((r = read(proto,buf,1024)) > 0)
    write(fd,buf,r);
  if(r < 0)
    {
      fprintf(stderr,"Couldn't open \"/etc/passwd.local\", %s.\n",
	      sys_errlist[errno]);
      close(fd);
      close(r);
      return;
    }
  for(i=0;i<nuid;i++)
    {
      char uname[8];
      if(uids[i] == 0 || uids[i] == 1)
	continue;
      sprintf(uname,"%d",uids[i]);
      ret = hes_resolve(uname,"uid");
      if(ret == NULL)
	{
	  fprintf(stderr,"Couldn't get hesinfo for uid %d, error %d.\n",
		  uids[i],hes_error());
	} else {
	  if((write(fd,*ret,strlen(*ret)) < 0)/*||(write(fd,"\n",1) < 0)*/)
	    fprintf(stderr,"Error writing \"/etc/passwd.new\": %s.\n",
		    sys_errlist[errno]);
	  else
	    printf(">>>%s\n", *ret);
	  write(fd,"\n",1);
	}
    }
  if(unlink("/etc/passwd") == -1)
    perror("unlink");
  if(rename("/etc/passwd.new", "/etc/passwd") == -1)
    perror("rename");
}
