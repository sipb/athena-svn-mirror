/* movemail foo bar -- move file foo to file bar,
   locking file foo the way /bin/mail respects.
   Copyright (C) 1986, 1992 Free Software Foundation, Inc.

This file is part of GNU Emacs.

GNU Emacs is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GNU Emacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Emacs; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* Important notice: defining MAIL_USE_FLOCK *will cause loss of mail*
   if you do it on a system that does not normally use flock as its way of
   interlocking access to inbox files.	The setting of MAIL_USE_FLOCK
   *must agree* with the system's own conventions.
   It is not a choice that is up to you.

   So, if your system uses lock files rather than flock, then the only way
   you can get proper operation is to enable movemail to write lockfiles there.
   This means you must either give that directory access modes
   that permit everyone to write lockfiles in it, or you must make movemail
   a setuid or setgid program.	*/

/*
 * Modified January, 1986 by Michael R. Gretzinger (Project Athena)
 *
 * Added POP (Post Office Protocol) service.  When compiled -DPOP
 * movemail will accept input filename arguments of the form
 * "po:username".  This will cause movemail to open a connection to
 * a pop server running on $MAILHOST (environment variable).  Movemail
 * must be setuid to root in order to work with POP.
 *
 * New module: popmail.c
 * Modified routines:
 *	main - added code within #ifdef MAIL_USE_POP; added setuid (getuid ())
 *		after POP code.
 * New routines in movemail.c:
 *	get_errmsg - return pointer to system error message
 *
 * Modified November, 1990 by Jonathan I. Kamens (Project Athena)
 *
 * Added KPOP (Kerberized POP) service to POP code.  If KERBEROS is
 * defined, then:
 *
 * 1. The "kpop" service is used instead of the "pop" service.
 * 2. Kerberos authorization data is sent to the server upon start-up.
 * 3. Instead of sending USER and RPOP, USER and PASS are sent, both
 *    containing the username of the user retrieving mail.
 *
 * Added HESIOD support.  If HESIOD is defined, then an attempt will
 * be made to look up the user's mailhost in the hesiod nameserver
 * database if the MAILHOST environment variable is not set.
 *
 * Modified June, 1992 by David Vinayak Henkel-Wallace (gumby@cygnus.com)
 * o - merged kerb code w/Emacs 19 version
 * o - cleaned up mode and setuid code -- now opens files with the
 *     correct owner, so that the output file isn't world-readable.
 *     setuid works even if you assemble the pop code.  You can make this
 *     setgid if your system can handle it.
 * o - POP client handles long lines correctly (but the implementation is ugly)
 * o - cleaned up style to FSF style, added some pointless casts.
 * o - Added (!) error-handling code for when POP msg deletion failed.
 * o - Fixed error-handling in some POP cases so you didn't end up with a
 *     partial inbox.
 * o - Corrected the format of the inbox file written by the POP code
 *     so that it won't confuse RMAIL.
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#define NO_SHORTNAMES	/* Tell config not to load remap.h */
/* #include "../src/config.h" */

#ifdef MAIL_USE_POP
/* moved up here so osconf.h gets included */
#ifdef KERBEROS
#include <krb.h>
#include <des.h>
#endif
#endif

#ifdef __SCO__
#define USG
#endif
#ifdef __svr4__
#define USG
#endif
#ifdef linux
#define USG
#endif
#ifdef USG
#include <fcntl.h>
#include <unistd.h>
#ifndef F_OK
#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4
#endif
#endif /* USG */

#ifdef XENIX
#include <sys/locking.h>
#endif

#ifdef MAIL_USE_MMDF
extern int lk_open (), lk_close ();
#endif

int open_ofile ();
int uid, gid, euid, egid;	/* sigh */

/* Cancel substitutions made by config.h for Emacs.  */
#undef open
#undef read
#undef write
#undef close

char *malloc ();
char *concat ();
char *xmalloc ();
#ifndef errno
extern int errno;
#endif

#ifdef hpux
#define seteuid(e) setresuid(-1,e,-1)
#define setegid(e) setresgid(-1,e,-1)
#endif
/* Nonzero means this is name of a lock file to delete on fatal error.	*/
char *delete_lockname;

main (argc, argv)
     int argc;
     char **argv;
{
  char *inname, *outname;
  int indesc, outdesc;
  int nread;

#ifndef MAIL_USE_FLOCK
  struct stat st;
  long now;
  int tem;
  char *lockname, *p;
  char *tempname;
  int desc;
#endif /* not MAIL_USE_FLOCK */

  delete_lockname = 0;

  if (argc < 3)
    fatal ("two arguments required");

  inname = argv[1];
  outname = argv[2];

  uid = getuid ();
  gid = getgid ();
  euid = geteuid ();
  egid = getegid ();

#ifdef MAIL_USE_MMDF
  mmdf_init (argv[0]);
#endif

#ifdef MAIL_USE_POP
  if (!memcmp (inname, "po:", 3))
    {
      int status; char *user;

      user = strrchr (inname, ':') + 1;
      status = popmail (user, outname);
      exit (status);
    }

#endif /* MAIL_USE_POP */

  /* Check access to input file.  */
  seteuid (uid);
  setegid (gid);
  if (access (inname, R_OK | W_OK) != 0)
    pfatal_with_name (inname);
  seteuid (euid);
  setegid (egid);

#ifndef MAIL_USE_MMDF
#ifndef MAIL_USE_FLOCK
  /* Use a lock file named /usr/spool/mail/$USER.lock:
     If it exists, the mail file is locked.

     Note: this locking mechanism is *required* by the mailer
     (on systems which use it) to prevent loss of mail.

     On systems that use a lock file, extracting the mail without locking
     WILL occasionally cause loss of mail due to timing errors!

     So, if creation of the lock file fails
     due to access permission on /usr/spool/mail,
     you simply MUST change the permission
     and/or make movemail a setgid program
     so it can create lock files properly.

     You might also wish to verify that your system is one
     which uses lock files for this purpose.  Some systems use other methods.

     If your system uses the `flock' system call for mail locking,
     define MAIL_USE_FLOCK in config.h or the s-*.h file
     and recompile movemail.  If the s- file for your system
     should define MAIL_USE_FLOCK but does not, send a bug report
     to bug-gnu-emacs@prep.ai.mit.edu so we can fix it.	 */

  lockname = concat (inname, ".lock", "");
  tempname = (char *) strcpy (xmalloc (strlen (inname)+1), inname);
  p = tempname + strlen (tempname);
  while (p != tempname && p[-1] != '/')
    p--;
  *p = 0;
  strcpy (p, "EXXXXXX");
  mktemp (tempname);
  (void) unlink (tempname);	/* why is this necessary? */

  while (1)
    {
      /* This looks pretty marginal to me */

      /* Create the lock file, but not under the lock file name.  */
      /* Give up if cannot do that.  */
      desc = open (tempname, O_WRONLY | O_CREAT, 0666);
      if (desc < 0)
	pfatal_with_name ("lock file");
      (void) close (desc);

      tem = link (tempname, lockname);
      (void) unlink (tempname);
      if (tem >= 0)
	break;
      sleep (1);

      /* If lock file is a minute old, unlock it.  */
      if (stat (lockname, &st) >= 0)
	{
	  now = time (0);
	  if (st.st_ctime < now - 60)
	    (void) unlink (lockname);
	}
    }

  delete_lockname = lockname;
#endif /* not MAIL_USE_FLOCK */

#ifdef MAIL_USE_FLOCK
  indesc = open (inname, O_RDWR);
#else /* if not MAIL_USE_FLOCK */
  indesc = open (inname, O_RDONLY);
#endif /* not MAIL_USE_FLOCK */
#else /* MAIL_USE_MMDF */
  indesc = lk_open (inname, O_RDONLY, 0, 0, 10);
#endif /* MAIL_USE_MMDF */
  if (indesc < 0)
    pfatal_with_name (inname);

  outdesc = open_ofile (outname);
  if (outdesc < 0)
    pfatal_with_name (outname);

#ifdef MAIL_USE_FLOCK
#ifdef XENIX
  if (locking (indesc, LK_RLCK, 0L) < 0)
    pfatal_with_name (inname);
#else
  if (flock (indesc, LOCK_EX) < 0)
    pfatal_with_name (inname);
#endif
#endif /* MAIL_USE_FLOCK */

  {
    char buf[1024];

    while (1)
      {
	nread = read (indesc, buf, sizeof buf);
	if (nread != write (outdesc, buf, nread))
	  {
	    int saved_errno = errno;
	    (void) unlink (outname);
	    errno = saved_errno;
	    pfatal_with_name (outname);
	  }
	if (nread < sizeof buf)
	  break;
      }
  }

#ifndef NO_FSYNC
#ifdef BSD
  /* Mainly for AFS users -- see below in POP code */
  if (fsync (outdesc) < 0)
    pfatal_and_delete (outname);
#endif
#endif

  /* Check to make sure no errors before we zap the inbox.  */
  if (close (outdesc) != 0)
    pfatal_and_delete (outname);

#ifdef MAIL_USE_FLOCK
#if defined (STRIDE) || defined (XENIX)
  /* Stride, xenix have file locking, but no ftruncate.	 This mess will do. */
  /* Won't this change the mode of the file? */
  (void) close (open (inname, O_CREAT | O_TRUNC | O_RDWR, 0666));
#else
  (void) ftruncate (indesc, 0L);
#endif /* STRIDE or XENIX */
#endif /* MAIL_USE_FLOCK */

#ifdef MAIL_USE_MMDF
  lk_close (indesc, 0, 0, 0);
#else
  (void) close (indesc);
#endif

#ifndef MAIL_USE_FLOCK
  /* Delete the input file; if we can't, at least get rid of its contents.  */
#ifdef MAIL_UNLINK_SPOOL
  /* This is generally bad to do, because it destroys the permissions
     that were set on the file.	 Better to just empty the file.	 */
  if (unlink (inname) < 0 && errno != ENOENT)
#endif /* MAIL_UNLINK_SPOOL */
    creat (inname, 0600);
#ifndef MAIL_USE_MMDF
  (void) unlink (lockname);
#endif /* not MAIL_USE_MMDF */
#endif /* not MAIL_USE_FLOCK */
  exit (0);
}

int
open_ofile (outname)
     char *outname;
{
  int outdesc;

  /* Unfortunately, even though movemail is setuid to root, it may
     not be possible to write the output file into his (her) homedir
     if it's NFS mounted w/o root access.  So to get this right we
     change back (!) before opening this file.
   */

  seteuid (uid);
  setegid (gid);

  outdesc = open (outname, O_WRONLY | O_CREAT | O_EXCL, 0600);

  seteuid (euid);
  setegid (egid);

  return outdesc;		/* caller should check if it's < 0 */
}

/* Print error message and exit.  */

fatal (s1, s2)
     char *s1, *s2;
{
  if (delete_lockname)
    (void) unlink (delete_lockname);
  error (s1, s2);
  exit (1);
}

/* Print error message.	 `s1' is printf control string, `s2' is arg for it. */

error (s1, s2, s3)
     char *s1, *s2, *s3;
{
  printf ("movemail: ");
  printf (s1, s2, s3);
  printf ("\n");
}

pfatal_with_name (name)
     char *name;
{
  extern int errno, sys_nerr;
#ifndef HAVE_SYS_ERRLIST_DECL
  extern char *sys_errlist[];
#endif
  char *s;

  if (errno < sys_nerr)
    s = concat ("", sys_errlist[errno], " for %s");
  else
    s = "cannot open %s";
  fatal (s, name);
}

pfatal_and_delete (name)
     char *name;
{
  extern int errno, sys_nerr;
#ifndef HAVE_SYS_ERRLIST_DECL
  extern char *sys_errlist[];
#endif
  char *s;

  if (errno < sys_nerr)
    s = concat ("", sys_errlist[errno], " for %s");
  else
    s = "cannot open %s";

  (void) unlink (name);
  fatal (s, name);
}

/* Return a newly-allocated string whose contents concatenate those of s1, s2, s3.  */

char *
concat (s1, s2, s3)
     char *s1, *s2, *s3;
{
  int len1 = strlen (s1), len2 = strlen (s2), len3 = strlen (s3);
  char *result = (char *) xmalloc (len1 + len2 + len3 + 1);

  strcpy (result, s1);
  strcpy (result + len1, s2);
  strcpy (result + len1 + len2, s3);
  *(result + len1 + len2 + len3) = 0;

  return result;
}

/* Like malloc but get fatal error if memory is exhausted.  */

char *
xmalloc (size)
     unsigned size;
{
  char *result = malloc (size);
  if (!result)
    fatal ("virtual memory exhausted", 0);
  return result;
}

/* This is the guts of the interface to the Post Office Protocol.  */

#ifdef MAIL_USE_POP

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pwd.h>
#ifdef KERBEROS
#include <krbports.h>
#endif
#ifdef HESIOD
#include <hesiod.h>
#endif

#ifdef USG
#include <fcntl.h>
/* Cancel substitutions made by config.h for Emacs.  */
#undef open
#undef read
#undef write
#undef close
#endif /* USG */

#define NOTOK (-1)
#define OK 0
#define DONE 1

char *progname;
FILE *sfi;
FILE *sfo;
char Errmsg[80];

/* Yes, this is ugly, but the abstraction in this code is atrocious! */
static int getline_saw_newline = 0;
static int debug = 0;

char *get_errmsg ();
char *getenv ();

popmail (user, outfile)
     char *user;
     char *outfile;
{
  char *host;
  int nmsgs, nbytes;
  char response[128];
  register int i;
  int mbfi;
  FILE *mbf;
#ifdef HESIOD
  struct hes_postoffice *p;
#endif
  struct passwd *pw = (struct passwd *) getpwuid (uid);
  if (pw == NULL)
    fatal ("cannot determine user name");

  host = getenv ("MAILHOST");
#ifdef HESIOD
  if (host == NULL) {
    p = hes_getmailhost (user);
    if (p != NULL && strcmp (p->po_type, "POP") == 0)
      host = p->po_host;
    else
      fatal ("no POP server listed in Hesiod");
  }
#endif				/* HESIOD */
  if (host == NULL)
    fatal ("no MAILHOST defined");

  if (pop_init (host) == NOTOK)
    fatal (Errmsg);

  if ((getline (response, sizeof response, sfi) != OK) || (*response != '+'))
    fatal (response);

#ifdef KERBEROS
  if (pop_command ("USER %s", user) == NOTOK
      || pop_command ("PASS %s", user) == NOTOK)
#else
    if (pop_command ("USER %s", user) == NOTOK
	|| pop_command ("RPOP %s", user) == NOTOK)
#endif
    {
      pop_command ("QUIT");
      fatal (Errmsg);
    }

  if (pop_stat (&nmsgs, &nbytes) == NOTOK)
    {
      pop_command ("QUIT");
      fatal (Errmsg);
    }

  if (!nmsgs)
    {
      pop_command ("QUIT");
      return 0;
    }

  mbfi = open_ofile (outfile);
  if (mbfi < 0)
    {
      pop_command ("QUIT");
      pfatal_and_delete (outfile);
    }

  if ((mbf = fdopen (mbfi, "w")) == NULL)
    {
      pop_command ("QUIT");
      pfatal_and_delete (outfile);
    }

  for (i = 1; i <= nmsgs; i++)
    {
      mbx_delimit_begin (mbf);
      if (pop_retr (i, mbf) != OK)
	{
	  (void) close (mbfi);
	  (void) unlink (outfile);
	  pop_command ("QUIT");
	  fatal (Errmsg);
	}
      mbx_delimit_end (mbf);

      fflush (mbf);
      if (ferror (mbf))
	{
	  pop_command ("QUIT");
	  pfatal_and_delete (outfile);
	}
    }

  /* On AFS, a call to write() only modifies the file in the local
     workstation's AFS cache.  The changes are not written to the server
     until a call to fsync () or close () is made.  Users with AFS home
     directories have lost mail when over quota because these checks were
     not made in previous versions of movemail. */

#ifndef NO_FSYNC
  if (fsync (mbfi) < 0)
    {
      pop_command ("QUIT");
      pfatal_and_delete (outfile);
    }
#endif
  
  if (close (mbfi) == -1)
    {
      pop_command ("QUIT");
      pfatal_and_delete (outfile);
    }

  for (i = 1; i <= nmsgs; i++)
    {
      if (pop_command ("DELE %d", i) == NOTOK)
	/* Holy shit!  Quick, put everything back! */
	{
	  char * err_dup = Errmsg; /* don't let it get clobbered */
	
	  pop_command ("RSET");
	  pop_command ("QUIT");
	  (void) unlink (outfile);
	  fatal (err_dup);
	}
    }

  pop_command ("QUIT");
  return (0);
}

pop_init (host)
     char *host;
{
  register struct hostent *hp;
  register struct servent *sp;
  struct sockaddr_in sin;
  register int s;
  char *host_save;
#ifdef KERBEROS
  KTEXT ticket;
  MSG_DAT msg_data;
  CREDENTIALS cred;
  Key_schedule schedule;
  int rem;
#else
  int lport = IPPORT_RESERVED - 1;
#endif

  hp = gethostbyname (host);
  if (hp == NULL)
    {
      sprintf (Errmsg, "MAILHOST unknown: %s", host);
      return NOTOK;
    }

#ifdef KERBEROS
  sp = getservbyname ("kpop", "tcp");
#else
  sp = getservbyname ("pop", "tcp");
#endif
  if (sp == 0)
    {
#ifdef KERBEROS
      sin.sin_port = htons(KPOP_PORT); /* kpop/tcp */
#else
      sin.sin_port = htons(POP3_PORT); /* pop/tcp -- POP3 (POP2 is 109) */
#endif
      return NOTOK;
    }
  else
    {
      sin.sin_port = sp->s_port;
    }

  sin.sin_family = hp->h_addrtype;
  memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
#ifdef KERBEROS
  s = socket (AF_INET, SOCK_STREAM, 0);
#else
  s = rresvport (&lport);
#endif

  if (s < 0)
    {
      sprintf (Errmsg, "error creating socket: %s", get_errmsg ());
      return NOTOK;
    }

  if (connect (s, (struct sockaddr *)&sin, sizeof sin) < 0)
    {
      sprintf (Errmsg, "error during connect: %s", get_errmsg ());
      (void) close (s);
      return NOTOK;
    }

#ifdef KERBEROS
  ticket = (KTEXT) malloc (sizeof(KTEXT_ST));
  host_save = malloc(strlen(hp->h_name)+1);
  strcpy(host_save, hp->h_name);
  rem = krb_sendauth (0L, s, ticket, "pop", host_save,
#ifdef _AUX_SOURCE		/* krb_realmofhost is broken on AUX */
		     KRB_REALM,
#else
		     (char *) krb_realmofhost (host_save),
#endif				/* _AUX_SOURCE */
		     (unsigned long)0, &msg_data, &cred, schedule,
		     (struct sockaddr_in *)0,
		     (struct sockaddr_in *)0,
		     "KPOPV0.1");
  free(host_save);
  if (rem != KSUCCESS) {
    sprintf (Errmsg, "kerberos error: %s", krb_get_err_text(rem));
    (void) close (s);
    return (NOTOK);
  }
#endif				/* KERBEROS */

  sfi = fdopen (s, "r");
  sfo = fdopen (s, "w");
  if (sfi == NULL || sfo == NULL)
    {
      sprintf (Errmsg, "error in fdopen: %s", get_errmsg ());
      (void) close (s);
      return NOTOK;
    }

  return OK;
}

pop_command (fmt, a, b, c, d)
     char *fmt;
     unsigned long a, b, c, d;
{
  char buf[128];

  sprintf (buf, fmt, a, b, c, d);

  if (debug) fprintf (stderr, "---> %s\n", buf);
  if (putline (buf, Errmsg, sfo) == NOTOK)
    return NOTOK;

  if (getline (buf, sizeof buf, sfi) != OK)
    {
      strcpy (Errmsg, buf);
      return NOTOK;
    }

  if (debug)
    fprintf (stderr, "<--- %s\n", buf);
  if (*buf != '+')
    {
      strcpy (Errmsg, buf);
      return NOTOK;
    }
  else
    {
      return OK;
    }
}

pop_stat (nmsgs, nbytes)
     int *nmsgs, *nbytes;
{
  char buf[128];

  if (debug)
    fprintf (stderr, "---> STAT\n");
  if (putline ("STAT", Errmsg, sfo) == NOTOK)
    return NOTOK;

  if (getline (buf, sizeof buf, sfi) != OK)
    {
      strcpy (Errmsg, buf);
      return NOTOK;
    }

  if (debug) fprintf (stderr, "<--- %s\n", buf);
  if (*buf != '+')
    {
      strcpy (Errmsg, buf);
      return NOTOK;
    }
  else
    {
      sscanf (buf, "+OK %d %d", nmsgs, nbytes);
      return OK;
    }
}

pop_retr (msgno, mbf)
     int msgno;
     FILE *mbf;
{
  char buf[128];

  sprintf (buf, "RETR %d", msgno);
  if (debug)
    fprintf (stderr, "%s\n", buf);
  if (putline (buf, Errmsg, sfo) == NOTOK)
    return NOTOK;

  if (getline (buf, sizeof buf, sfi) != OK)
    {
      strcpy (Errmsg, buf);
      return NOTOK;
    }

  while (1)
    {
      switch (multiline (buf, sizeof buf, sfi))
	{
	case OK:
	  if (fputs (buf, mbf) == EOF)
	    {
	      strcpy (Errmsg, get_errmsg ());
	      return (NOTOK);
	    }
	  if ((getline_saw_newline == 1) && (fputc ('\n', mbf) == EOF))
	    {
	      strcpy (Errmsg, get_errmsg ());
	      return (NOTOK);
	    }
	  break;
	case DONE:
	  return OK;
	case NOTOK:
	  strcpy (Errmsg, buf);
	  return NOTOK;
	}
    }
}

int
getline (buf, n, f)
     char *buf;
     register int n;
     FILE *f;
{
  register char *p;
  int c;

  getline_saw_newline = 0;

  p = buf;
  while (--n > 0 && (c = fgetc (f)) != EOF)
    if ((*p++ = c) == '\n')
      break;

  if (ferror (f))
    {
      strcpy (buf, "error on connection");
      return NOTOK;
    }

  if (c == EOF && p == buf)
    {
      strcpy (buf, "connection closed by foreign host");
      return DONE;
    }

  *p = 0;
  if (*--p == '\n')
    {
      getline_saw_newline = 1;
      *p = 0;
      /* smush cr _only_ if it's part of cr-nl */
      if (*--p == '\r')
	*p = 0;
    }

  return OK;
}

int
multiline (buf, n, f)
     char *buf;
     register int n;
     FILE *f;
{
  int had_newline = getline_saw_newline;
  if (getline (buf, n, f) != OK)
    return NOTOK;
  if (had_newline && *buf == '.')
    {
      if (*(buf+1) == 0)
	return DONE;
      else
	{
	  /* Don't use strcpy!  There's no guarantee about the order it
	     copies characters!  */
	  char *p = buf;
	  while (*p)
	    {
	      *p = p[1];
	      p++;
	    }
	}
    }
  return OK;
}

char *
get_errmsg ()
{
  extern int errno, sys_nerr;
#ifndef HAVE_SYS_ERRLIST_DECL
  extern char *sys_errlist[];
#endif
  char *s;

  if (errno < sys_nerr)
    s = sys_errlist[errno];
  else
    s = "unknown error";
  return s;
}

int
putline (buf, err, f)
     char *buf;
     char *err;
     FILE *f;
{
  fprintf (f, "%s\r\n", buf);
  fflush (f);
  if (ferror (f))
    {
      strcpy (err, "lost connection");
      return NOTOK;
    }
  return OK;
}

mbx_delimit_begin (mbf)
     FILE *mbf;
{
  fputs ("\f\n0, unseen,,\n*** EOOH ***\n", mbf);
}

mbx_delimit_end (mbf)
     FILE *mbf;
{
  putc ('\037', mbf);
}

#endif /* MAIL_USE_POP */
