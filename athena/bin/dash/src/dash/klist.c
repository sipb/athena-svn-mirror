/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/dash/klist.c,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#ifndef	lint
static char rcsid[] =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/dash/klist.c,v 1.1 1991-09-03 11:15:30 vanharen Exp $";
#endif	lint

#include "mit-copyright.h"
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <krb.h>
#include "Jets.h"
#include "Button.h"
#include "warn.h"


#if  defined(ultrix) || defined(_AIX) || defined(_AUX_SOURCE) || defined(sun)
extern int errno;
extern char *sys_errlist[];
extern int sys_nerr;
#endif


char   *tkt_string();
char   *getenv();

extern char *krb_err_txt[];

#ifndef TICKET_GRANTING_TICKET
#define TICKET_GRANTING_TICKET	"krbtgt"
#endif

static Warning *old_warn = NULL;
static int ok();
extern char line1[], line2[];

int checkTkts()
{
  char pname[ANAME_SZ];
  char pinst[INST_SZ];
  char prealm[REALM_SZ];
  int k_errno;
  CREDENTIALS c;
  char *file;
  int ret = 0;
  static int old_ret = 0;
  static time_t mod_time = 0;
  static long exp_time;
  struct stat statbuf;
  int diff;
  unsigned int timeout = 5*60*1000;	/* 5 minutes... */

  line1[0] = line2[0] = '\0';

  if ((file = getenv("KRBTKFILE")) == NULL)
    file = TKT_FILE;


  if (stat(file, &statbuf))
    {
      sprintf(line1, "Could not stat `%s':", file);
      if (errno == 0 || errno > sys_nerr)
	sprintf(line2, "Error %d", errno);
      else
	sprintf(line2, "%s", sys_errlist[errno]);
      ret = 1;
      goto done;
    }

  if (statbuf.st_mtime != mod_time)
    mod_time = statbuf.st_mtime;
  else  if (exp_time - time(0) > 15 * 60)
    {
      ret = 0;
      goto done;
    }

  /* 
   * Since krb_get_tf_realm will return a ticket_file error, 
   * we will call tf_init and tf_close first to filter out
   * things like no ticket file.  Otherwise, the error that 
   * the user would see would be 
   * klist: can't find realm of ticket file: No ticket file (tf_util)
   * instead of
   * klist: No ticket file (tf_util)
   */

  /* Open ticket file */
  if (k_errno = tf_init(file, R_TKT_FIL))
    {
      sprintf(line1, "%s", krb_err_txt[k_errno]);
      ret = 1;
      goto done;
    }
  /* Close ticket file */
  (void) tf_close();

  /* 
   * We must find the realm of the ticket file here before calling
   * tf_init because since the realm of the ticket file is not
   * really stored in the principal section of the file, the
   * routine we use must itself call tf_init and tf_close.
   */
  if ((k_errno = krb_get_tf_realm(file, prealm)) != KSUCCESS)
    {
      strcpy(line1, "can't find realm of ticket file:");
      sprintf(line2, "%s", krb_err_txt[k_errno]);
      ret = 1;
      goto done;
    }

  /* Open ticket file, get principal name and instance */
  if ((k_errno = tf_init(file, R_TKT_FIL)) ||
      (k_errno = tf_get_pname(pname)) ||
      (k_errno = tf_get_pinst(pinst)))
    {
      sprintf(line1, "%s", krb_err_txt[k_errno]);
      ret = 1;
      goto done;
    }

  /* 
   * You may think that this is the obvious place to get the
   * realm of the ticket file, but it can't be done here as the
   * routine to do this must open the ticket file.  This is why 
   * it was done before tf_init.
   */
       
  while ((k_errno = tf_get_cred(&c)) == KSUCCESS)
    {
      if (!strcmp(c.service, TICKET_GRANTING_TICKET) &&
	  !strcmp(c.instance, prealm))
	{
	  exp_time = c.issue_date + ((unsigned char) c.lifetime) * 5 * 60;
	  diff = exp_time - time(0);

	  if (diff < 0)
	    {
	      strcpy(line1, "Your authentication has expired.");
	      strcpy(line2, "Type `renew' to re-authenticate.");
	      ret = 3;			/* has expired */
	      goto done;
	    }
	  if (diff < 5 * 60)		/* inside of 5 minutes? */
	    {
	      strcpy(line1,
		     "Your authentication will expire in less than 5 minutes.");
	      strcpy(line2, "Type `renew' to re-authenticate.");
	      timeout = 60*1000;	/* set timeout to 1 minute... */
	      ret = 2;
	      goto done;
	    }
	  else if (diff < 15 * 60)	/* inside of 15 minutes? */
	    {
	      strcpy(line1,
		     "Your authentication will expire in less than 15 minutes.");
	      strcpy(line2, "Type `renew' to re-authenticate.");
	      timeout = 60*1000;	/* set timeout to 1 minute... */
	      ret = 1;
	      goto done;
	    }

	  ret = 0;			/* tgt hasn't expired */
	  goto done;
	}
      continue;			/* not a tgt */
    }

  strcpy(line1, "You have no authentication.");
  strcpy(line2, "Type `renew' to re-authenticate.");
  ret = 1;			/* no tgt found */



 done:
  if (ret  &&  (old_ret != ret))
    {
      Warning *w;

      /* Destroy last warning if user hasn't clicked it away already */
      if (old_warn != NULL)
	XjCallCallbacks((caddr_t) old_warn,
			old_warn->button->button.activateProc, NULL);

      w = (Warning *)XjMalloc((unsigned) sizeof(Warning));

      w->me.next = NULL;
      w->me.argType = argInt;
      w->me.passInt = (int)w;
      w->me.proc = ok;

      w->l1 = XjNewString(line1);
      w->l2 = XjNewString(line2);

      old_warn = UserWarning(w, True);
    }
  old_ret = ret;
  (void) tf_close();
  XjAddWakeup(checkTkts, NULL, timeout);
  return ret;
}

static int ok(who, w, data)
     Jet who;
     Warning *w;
     caddr_t data;
{
  Display *dpy;

  dpy = w->top->core.display;	/* save off the display before */
				/* destroying the Jet */
  XjDestroyJet(w->top);
  XFlush(dpy);

  XjFree(w->l1);
  XjFree(w->l2);
  XjFree((char *) w);
  old_warn = NULL;
  return 0;
}
