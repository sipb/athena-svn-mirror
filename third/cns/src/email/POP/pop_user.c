/*
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char copyright[] = "Copyright (c) 1990 Regents of the University of California.\nAll rights reserved.\n";
static char SccsId[] = "@(#)pop_user.c  1.5 7/13/90";
#endif

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include "popper.h"

#ifdef KERBEROS
#include <krb.h>
extern AUTH_DAT kdata;
#endif /* KERBEROS */


/* 
 *  user:   Prompt for the user name at the start of a POP session
 */

int pop_user (p)
POP     *   p;
{
#ifndef KERBEROS
    /*  Save the user name */

#ifdef KERBEROS_PASSWD_HACK
    lower_case(p->pop_parm[1]);
#endif /* KERBEROS_PASSWD_HACK */

    (void)strcpy(p->user, p->pop_parm[1]);

#else /* KERBEROS */

    if(strcmp(p->pop_parm[1], p->user))
      {
	pop_log(p, POP_WARNING, "%s: auth failed: %s.%s@@%s vs %s",
		p->client, kdata.pname, kdata.pinst, kdata.prealm, 
		p->pop_parm[1]);
        return(pop_msg(p,POP_FAILURE,
		       "Wrong username supplied (%s vs. %s).\n", p->user,
		       p->pop_parm[1]));
      }

#endif /* KERBEROS */

    /*  Tell the user that the password is required */
    return (pop_msg(p,POP_SUCCESS,"Password required for %s.",p->user));
}


lower_case(s)
     char *s;
{
  while(s && *s)
    {
      if(isupper(*s))
	*s = tolower(*s);
      ++s;
    }
}
