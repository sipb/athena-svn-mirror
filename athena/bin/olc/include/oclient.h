/*
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/oclient.h,v 1.2 1991-01-08 14:44:00 lwvanels Exp $
 */

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
