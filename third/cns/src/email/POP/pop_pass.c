/*
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* 
 * authorization rules for use with kerberos (tom)
 * password server using kerberos (jon)
 */

#ifndef lint
static char copyright[] = "Copyright (c) 1990 Regents of the University of California.\nAll rights reserved.\n";
static char SccsId[] = "@(#)pop_pass.c  1.7 7/13/90";
#endif

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <pwd.h>
#include <netinet/in.h>
#include "popper.h"

#ifdef KERBEROS
#include <krb.h>
extern AUTH_DAT kdata;
#endif /* KERBEROS */

#ifdef KERBEROS_PASSWD_HACK
extern int fallback;
#endif /* KERBEROS_PASSWD_HACK */

#ifdef MH_COMPAT
struct passwd * our_getpwnam();
#endif /* MH_COMPAT */

/* 
 *  pass:   Obtain the user password from a POP client
 */

int pop_pass (p)
POP     *   p;
{
#ifdef KERBEROS
    char lrealm[REALM_SZ];
    int status; 
#else
    register struct passwd  *   pw;
    char *crypt();
#endif /* KERBEROS */


#ifdef KERBEROS
    if ((status = krb_get_lrealm(lrealm,1)) == KFAILURE) {
        pop_log(p, POP_WARNING, "%s: (%s.%s@%s) %s", p->client, kdata.pname, 
		kdata.pinst, kdata.prealm, krb_get_err_text(status));
        return(pop_msg(p,POP_FAILURE,
            "Kerberos error:  \"%s\".", krb_get_err_text(status)));
    }

    if (strcmp(kdata.prealm,lrealm))  {
         pop_log(p, POP_WARNING, "%s: (%s.%s@%s) realm not accepted.", 
		 p->client, kdata.pname, kdata.pinst, kdata.prealm);
	 return(pop_msg(p,POP_FAILURE,
		     "Kerberos realm \"%s\" not accepted.", kdata.prealm));
    }

    if (strcmp(kdata.pinst,"")) {
        pop_log(p, POP_WARNING, "%s: (%s.%s@%s) instance not accepted.", 
		 p->client, kdata.pname, kdata.pinst, kdata.prealm);
        return(pop_msg(p,POP_FAILURE,
	      "Must use null Kerberos(tm) instance -  \"%s.%s\" not accepted.",
	      kdata.pname, kdata.pinst));
    }

    /*  Build the name of the user's maildrop */
    (void)sprintf(p->drop_name,"%s/%s",POP_MAILDIR,p->user);
    
    /*  Make a temporary copy of the user's maildrop */
    if (pop_dropcopy(p) != POP_SUCCESS) return (POP_FAILURE);

#else /* KERBEROS */

#ifdef KERBEROS_PASSWD_HACK

    if(hack_pass(p) == POP_SUCCESS)
      return(POP_SUCCESS);

    if(!fallback) 
       return(POP_FAILURE);
     pop_log(p, POP_WARNING, "%s (%s): falling back to passwd file", p->user,
	     p->client);
#endif /* KERBEROS_PASSWD_HACK */

    
     /*  Look for the user in the password file */
#ifdef MH_COMPAT
    our_setpwent(POP_PASSFILE);

    if ((pw = our_getpwnam(p->user)) == NULL) {
#else /* MH_COMPAT */
    setpwfile(POP_PASSFILE);

    if ((pw = getpwnam(p->user)) == NULL) {
#endif /* MH_COMPAT */

         pop_log(p, POP_WARNING, "%s (%s): not in password file", p->user,
		 p->client);
	 return (pop_msg(p,POP_FAILURE,
			 "Password supplied for \"%s\" is incorrect.",
			 p->user));
    }
    
    /*  We don't accept connections from users with null passwords */
    if (pw->pw_passwd == NULL) {
        pop_log(p, POP_WARNING, "%s (%s): password is set to null.. rejecting",
		p->user, p->client);
        return (pop_msg(p,POP_FAILURE,
            "Password supplied for \"%s\" is incorrect.",p->user));
    }

    /*  Compare the supplied password with the password file entry */
    if (strcmp (crypt (p->pop_parm[1], pw->pw_passwd), pw->pw_passwd) != 0) {
        pop_log(p, POP_WARNING, "%s (%s): password incorrect", p->user,
		p->client);
        return (pop_msg(p,POP_FAILURE,
            "Password supplied for \"%s\" is incorrect.",p->user));
    }

#ifdef notdef
    /* Set the group and user id */
    (void)setgid(pw->pw_gid);
    (void)setuid(pw->pw_uid);
#endif 

    /*  Build the name of the user's maildrop */
    (void)sprintf(p->drop_name,"%s/%s",POP_MAILDIR,p->user);

    /*  Make a temporary copy of the user's maildrop */
    if (pop_dropcopy(p) != POP_SUCCESS) return (POP_FAILURE);

#ifdef DEBUG
    if(p->debug)pop_log(p,POP_DEBUG,"uid = %d, gid = %d",getuid(),getgid());
#endif

#endif /* KERBEROS */

    /*  Get information about the maildrop */
    if (pop_dropinfo(p) != POP_SUCCESS) {
      pop_log(p, POP_PRIORITY, "dropinfo failure");
      return(POP_FAILURE);
    }

    /*  Initialize the last-message-accessed number */
    p->last_msg = 0;

    /*  Authorization completed successfully */
    return (pop_msg (p,POP_SUCCESS,
        "%s has %d message(s) (%d octets).",
            p->user,p->msg_count,p->drop_size));
}




#ifdef KERBEROS_PASSWD_HACK
/*
 * Check to see if the user is in the passwd file, if not get a kerberos
 * ticket with the password
 */



#include <sys/file.h>
#include <netdb.h>
#include <krb.h>

int hack_pass (p)
POP     *   p;
{
    register struct passwd  *   pw;
    char *crypt();

    if(verify_passwd_hack_hack_hack(p) != POP_SUCCESS)
      return(POP_FAILURE);
    
    /*  Build the name of the user's maildrop */
    (void)sprintf(p->drop_name,"%s/%s",POP_MAILDIR,p->user);
    
    /*  Make a temporary copy of the user's maildrop */
    if (pop_dropcopy(p) != POP_SUCCESS) return (POP_FAILURE);

    
#ifdef DEBUG
    if(p->debug)pop_log(p,POP_DEBUG,"uid = %d, gid = %d",getuid(),getgid());
#endif

    /*  Get information about the maildrop */
    if (pop_dropinfo(p) != POP_SUCCESS) {
        pop_log(p, POP_PRIORITY, "dropinfo failure");
        return(POP_FAILURE);
    }

    /*  Initialize the last-message-accessed number */
    p->last_msg = 0;

    /*  Authorization completed successfully */
    return (pop_msg (p,POP_SUCCESS,
        "%s has %d message(s) (%d octets).",
            p->user,p->msg_count,p->drop_size));
}


/* for Ultrix and friends ... */
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif


int verify_passwd_hack_hack_hack(p)
     POP *p;
{
    char lrealm[REALM_SZ];
    int status, fd; 
    KTEXT_ST ticket;
    AUTH_DAT kdata;
    char savehost[MAXHOSTNAMELEN];
    char tkt_file_name[MAXPATHLEN];
    struct sockaddr_in sin;
    int len;

    extern int errno;
    extern int sp;

    /* Be sure ticket file is correct and exists */

    sprintf(tkt_file_name, "/tmp/tkt_pop.%s.%d", p->user, getpid());
    if (setenv("KRBTKFILE", tkt_file_name, 1 /* overwrite = yes */))
      return(pop_msg(p,POP_FAILURE, "Could not set ticket file name."));

    if ((fd = open(tkt_file_name, O_RDWR|O_CREAT, 0600)) == -1) {
        close(fd);
	pop_log(p, POP_ERROR, "%s: could not create ticket file, \"%s\": %s",
		p->user, tkt_file_name, sys_errlist[errno]);
	if(fallback)
	  return(POP_FAILURE);
	return(pop_msg(p,POP_FAILURE,"POP server error: %s while creating %s.",
		       sys_errlist[errno], tkt_file_name));
    }
    close(fd);

    /* 
     * Get an initial ticket using the given name/password and then use it.
     * This hack will allow us to stop maintaining a separate pop passwd 
     * on eagle. We can cut over to real Kerberos POP clients at any point. 
     */

    if ((status = krb_get_lrealm(lrealm,1)) == KFAILURE) {
        pop_log(p,POP_ERROR, "%s (%s):  error getting local realm:  \"%s\".", 
		p->user, p->client, krb_get_err_text(status));
	dest_tkt(); /* need to remove empty file */
	if(fallback)
	  return(POP_FAILURE);
	return(pop_msg(p,POP_FAILURE, "POP server error:  \"%s\".", 
		       krb_get_err_text(status)));
    }
    
    status = krb_get_pw_in_tkt(p->user, "", lrealm, "krbtgt", lrealm,
			       2 /* 10 minute lifetime */, p->pop_parm[1]);
    if (status != KSUCCESS) {
        pop_log(p,POP_WARNING, "%s (%s): error getting tgt: %s", 
		p->user, p->client, krb_get_err_text(status));
	dest_tkt();
	if(fallback)
	  return(POP_FAILURE);
	return(pop_msg(p,POP_FAILURE,
		       "Kerberos authentication error:  \"%s\".", 
		       krb_get_err_text(status)));
    }

    /*
     * Now use the ticket for something useful, to make sure
     * it is valid.
     */

    (void) strncpy(savehost, krb_get_phost(p->myhost), sizeof(savehost));
    savehost[sizeof(savehost)-1] = 0;

    status = krb_mk_req(&ticket, "pop", savehost, lrealm, 0);
    if (status == KDC_PR_UNKNOWN) {
          pop_log(p, POP_ERROR, "%s (%s): %s", p->user, p->client, 
		  krb_get_err_text(status));
	  dest_tkt();
	  if(fallback)
	    return(POP_FAILURE);
          return(pop_msg(p,POP_FAILURE,
		     "POP server configuration error:  \"%s\".", 
		     krb_get_err_text(status)));

    } 
      
    if (status != KSUCCESS) {
	  dest_tkt();
	  pop_log(p, POP_ERROR, "%s (%s): krb_mk_req error: %s", p->user, 
		  p->client, krb_get_err_text(status));
	  dest_tkt();
	  if(fallback)
	    return(POP_FAILURE);
	  return(pop_msg(p,POP_FAILURE,
	        "Kerberos authentication error (while getting ticket) \"%s\".",
			 krb_get_err_text(status)));
    }  

    len = sizeof(sin);
    getsockname(sp, &sin, &len); 
    if ((status = krb_rd_req(&ticket, "pop", savehost, sin.sin_addr.s_addr, 
			     &kdata, ""))
	!= KSUCCESS) {
         dest_tkt();
	 pop_log(p, POP_WARNING, "%s (%s): krb_rd_req error: %s", p->user, 
		 p->client, krb_get_err_text(status));
	 if(fallback)
	   return(POP_FAILURE);
	 return(pop_msg(p,POP_FAILURE, 
			"Kerberos authentication error:  \"%s\".", 
			krb_get_err_text(status)));
    }
    dest_tkt();
    return(POP_SUCCESS);
}

#endif /* KERBEROS_PASSWD_HACK */

#ifdef MH_COMPAT

/*
 * I'm getting myself deeper and deeper
 */

static char *pwfile = "/etc/passwd";

our_setpwent(file)
     char *file;
{
  if(file)
    {
      pwfile = (char *) malloc(strlen(file) + 1);
      if(pwfile)
	strcpy(pwfile, file);
      else
	return(-1);
      return(0);
    }
  return(-1);
}

struct passwd *
our_getpwnam(user)
     char *user;
{
  FILE *fp;
  char buf[BUFSIZ];
  register char *c;
  register char *d;
  static struct passwd p;

  if(!(fp = fopen(pwfile, "r")))
    return(NULL);

  memset(&p, 0, sizeof(p));
  while(fgets(buf, sizeof(buf), fp))
    {
      if(!(c = (char *) strchr(buf, ':')))
	continue;

      *c++ = '\0';
      if(strcmp(buf, user))
	continue;
      
      p.pw_name = buf;
      if(!((d = (char *) strchr(c, ':')) && (c = (char *) strchr(++d, ':')) &&
	 (d = (char *) strchr(++c, ':'))))
	return(&p);
      *d = '\0';
      p.pw_passwd = c;
      return(&p);
    }
  
  return(NULL);
}

#endif /* MH_COMPAT */
