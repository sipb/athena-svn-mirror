/*
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/oclient.h,v 1.3 1991-04-18 22:01:59 lwvanels Exp $
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <hesiod.h>
#include <strings.h>
#include <ctype.h>

#include "system.h"

#include "requests.h"

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

#ifdef KERBEROS
#define K_SERVICE "olc"

#endif /* KERBEROS */

/* oreplay.c */
void usage P((void));
void punt P((int fd, char *filename));

/* io.c */
int sread P((int fd , void *buf , int nbytes ));
int swrite P((int fd , void *buf , int nbytes ));
void expand_hostname P((char *hostname , char *instance , char *realm ));

#undef P
